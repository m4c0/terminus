module;
#include <windows.h>

module terminus;
import hai;

namespace terminus {
struct hpcon_del {
  void operator()(HPCON c) { ClosePseudoConsole(c); }
};
struct handle_del {
  void operator()(HANDLE h) { CloseHandle(h); }
};
using handle = hai::value_holder<HANDLE, handle_del>;
using hpcon = hai::value_holder<HPCON, hpcon_del>;

class spawn_prog : public prog {
  handle m_ors;
  handle m_iws;
  hpcon m_hpcon;

  void create_pty(const spawn_params &p) {
    handle irs;
    handle ows;

    if (!CreatePipe(&*irs, &*m_iws, nullptr, 0)) {
      // TODO: GetLastError()
      silog::log(silog::error, "failed to create input pipe");
      throw spawn_failed{};
    }

    if (!CreatePipe(&*m_ors, &*ows, nullptr, 0)) {
      // TODO: GetLastError()
      silog::log(silog::error, "failed to create output pipe");
      throw spawn_failed{};
    }

    COORD sz{
        .X = static_cast<short>(p.con_width),
        .Y = static_cast<short>(p.con_height),
    };

    HPCON pc;
    if (FAILED(CreatePseudoConsole(sz, *irs, *ows, 0, &pc))) {
      silog::log(silog::error, "failed to create pty");
      throw spawn_failed{};
    }
    m_hpcon = hpcon{pc};
  }
  auto create_startupinfo() {
    // TODO: check for errors
    STARTUPINFOEX si{};
    si.StartupInfo.cb = sizeof(STARTUPINFOEX);

    size_t req;
    InitializeProcThreadAttributeList(nullptr, 1, 0, &req);
    si.lpAttributeList = static_cast<PPROC_THREAD_ATTRIBUTE_LIST>(
        HeapAlloc(GetProcessHeap(), 0, req));
    InitializeProcThreadAttributeList(si.lpAttributeList, 1, 0, &req);

    UpdateProcThreadAttribute(si.lpAttributeList, 0,
                              PROC_THREAD_ATTRIBUTE_PSEUDOCONSOLE, *m_hpcon,
                              sizeof(*m_hpcon), nullptr, nullptr);
    return si;
  }

public:
  explicit spawn_prog(const spawn_params &p) {
    create_pty(p);

    auto si = create_startupinfo();

    jute::heap args{};
    for (auto arg : p.args) {
      if ((*args).size() > 0)
        args = args + " ";
      args = args + arg;
    }

    auto args_sz = (*args).size() + 1;
    auto cli = (*args).cstr();

    PROCESS_INFORMATION pi{};
    if (!CreateProcess(nullptr, cli.data(), nullptr, nullptr, false,
                       EXTENDED_STARTUPINFO_PRESENT, nullptr, nullptr,
                       &si.StartupInfo, &pi)) {
      silog::log(silog::error, "failed to create process");
      throw spawn_failed{};
    }
  }

  void send(jute::view chars) override {
    DWORD wr{};
    WriteFile(*m_iws, chars.data(), chars.size(), &wr, nullptr);
    if (wr != chars.size()) {
      silog::log(silog::error, "write: truncated output of %ld bytes", wr);
      throw send_failed{};
    }
  }
  void recv(buffer *b) override {
    DWORD rd{};
    PeekNamedPipe(*m_ors, nullptr, 0, nullptr, &rd, nullptr);
    if (rd == 0) {
      b->set_size(0);
      return;
    }
    auto sz = (rd < b->capacity()) ? rd : b->capacity();

    // TODO: check errors
    ReadFile(*m_ors, b->data(), sz, &rd, nullptr);
    b->set_size(sz);
  }
};

[[nodiscard]] hai::uptr<prog> spawn(const spawn_params &p) {
  return hai::uptr<prog>{new spawn_prog{p}};
}
} // namespace terminus
