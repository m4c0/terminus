module;
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <spawn.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include <util.h>

module terminus;
import hai;
import jute;
import silog;

namespace terminus {
class spawn_prog : public prog {
  int m_pri;
  int m_sec;
  int m_pid;

public:
  spawn_prog(int pri, int sec, int pid) : m_pri{pri}, m_sec{sec}, m_pid{pid} {}

  ~spawn_prog() {
    silog::log(silog::debug, "Stopping PID %d", m_pid);
    close(m_pri); // surprisingly important - otherwise child don't exit
    close(m_sec);
    kill(m_pid, SIGKILL); // TODO: find a better way
    waitpid(m_pid, nullptr, 0);
  }

  void send(jute::view chars) override {
    if (chars.size() >= write(m_pri, chars.data(), chars.size())) {
      silog::log(silog::error, "write: %s", strerror(errno));
      throw send_failed{};
    }
  }

  void recv(buffer *b) override {
    fd_set set;
    FD_ZERO(&set);
    FD_SET(m_pri, &set);

    timeval timeout{
        .tv_sec = 0,
        .tv_usec = 500000,
    };
    auto sel = select(m_pri + 1, &set, NULL, NULL, &timeout);
    if (sel == -1) {
      silog::log(silog::error, "select: %s", strerror(errno));
      throw recv_failed{};
    }
    if (sel == 0)
      return;

    auto rd = read(m_pri, b->data(), b->capacity());
    if (rd < 0) {
      silog::log(silog::error, "read: %s", strerror(errno));
      throw recv_failed{};
    } else if (rd == 0) {
      silog::log(silog::error, "read: eof");
      throw recv_failed{};
    }

    b->set_size(rd);
    return;
  }
};

[[nodiscard]] hai::uptr<prog> spawn(const spawn_params &p) {
  unsigned argc = p.args.size();

  hai::array<hai::cstr> strs{argc};
  hai::array<char *> args{argc + 1};
  for (auto i = 0; i < argc; i++) {
    strs[i] = p.args[i].cstr();
    args[i] = strs[i].data();
  }
  args[argc] = 0;

  // TODO: add TERM, LINES, COLUMNS, etc
  char *env[1];
  env[0] = 0;

  winsize sz{
      .ws_row = static_cast<unsigned short>(p.con_height),
      .ws_col = static_cast<unsigned short>(p.con_width),
  };

  int pri;
  int sec;
  if (-1 == openpty(&pri, &sec, 0, 0, &sz)) {
    silog::log(silog::error, "openpty: %s", strerror(errno));
    throw spawn_failed{};
  }

  posix_spawn_file_actions_t sfa{};
  posix_spawn_file_actions_init(&sfa);
  posix_spawn_file_actions_adddup2(&sfa, sec, 0);
  posix_spawn_file_actions_adddup2(&sfa, sec, 1);
  posix_spawn_file_actions_adddup2(&sfa, sec, 2);

  int pid;
  if (0 != posix_spawnp(&pid, args[0], &sfa, 0, args.begin(), env)) {
    silog::log(silog::error, "posix_spawnp: %s", strerror(errno));
    throw spawn_failed{};
  }

  return hai::uptr<prog>{new spawn_prog{pri, sec, pid}};
}
} // namespace terminus
