module;
#include <fcntl.h>
#include <spawn.h>
#include <util.h>

module terminus;
import hai;
import jute;

namespace terminus {
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
    return {};
  }

  posix_spawn_file_actions_t sfa{};
  posix_spawn_file_actions_init(&sfa);
  posix_spawn_file_actions_adddup2(&sfa, sec, 0);
  posix_spawn_file_actions_adddup2(&sfa, sec, 1);
  posix_spawn_file_actions_adddup2(&sfa, sec, 2);
  posix_spawnp(0, args[0], &sfa, 0, args.begin(), env);

  return {};
}
} // namespace terminus
