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
    kill(m_pid, SIGTERM);
    close(m_pri); // surprisingly important - otherwise child don't exit
    close(m_sec);
    waitpid(m_pid, nullptr, 0);
  }

  void send(jute::view chars) override {
    if (0 >= write(m_pri, chars.data(), chars.size())) {
      silog::log(silog::error, "write: %s", strerror(errno));
    }
  }

  hai::varray<char> recv() override {
    fd_set set;
    FD_ZERO(&set);
    FD_SET(m_pri, &set);

    timeval timeout{
        .tv_sec = 0,
        .tv_usec = 500000,
    };
    switch (select(m_pri + 1, &set, NULL, NULL, &timeout)) {
    case -1:
      silog::log(silog::error, "select: %s", strerror(errno));
      return {};
    case 0:
      return {};
    default: {
      hai::varray<char> res{1024};
      auto rd = read(m_pri, res.begin(), res.size());
      if (rd <= 0) {
        silog::log(silog::error, "read: %s", strerror(errno));
        return {};
      }

      res.truncate(rd);
      return res;
    }
    }
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
      .ws_row = 60,
      .ws_col = 120,
  };

  int pri;
  int sec;
  if (-1 == openpty(&pri, &sec, 0, 0, &sz)) {
    silog::log(silog::error, "openpty: %s", strerror(errno));
    return {};
  }

  posix_spawn_file_actions_t sfa{};
  posix_spawn_file_actions_init(&sfa);
  posix_spawn_file_actions_adddup2(&sfa, sec, 0);
  posix_spawn_file_actions_adddup2(&sfa, sec, 1);
  posix_spawn_file_actions_adddup2(&sfa, sec, 2);

  int pid;
  if (0 != posix_spawnp(&pid, args[0], &sfa, 0, args.begin(), env)) {
    silog::log(silog::error, "posix_spawnp: %s", strerror(errno));
    return {};
  }

  return hai::uptr<prog>{new spawn_prog{pri, sec, pid}};
}
} // namespace terminus
