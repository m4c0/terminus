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

public:
  spawn_prog(const spawn_params &p) {
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
  void send(jute::view chars) override {}
  void recv(buffer *b) override {}
};

[[nodiscard]] hai::uptr<prog> spawn(const spawn_params &p) {
  return hai::uptr<prog>{new spawn_prog{p}};
}
} // namespace terminus
