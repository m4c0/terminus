module;
#include <stdio.h>

export module poc;
import hai;
import jute;
import terminus;

void recv(auto &p) {
  auto str = p->recv();
  while (str.size() == 0) {
    str = p->recv();
  }
  while (str.size() > 0) {
    fwrite(str.begin(), str.size(), 1, stdout);
    fflush(stdout);
    str = p->recv();
  }
}

void try_main() {
  using namespace jute::literals;

  terminus::spawn_params sp{
      .con_width = 40,
      .con_height = 24,
      .args = hai::array<jute::view>::make("vim"_s, "-T"_s, "builtin_dumb"_s),
  };

  auto p = terminus::spawn(sp);
  recv(p);
  p->send(":q");
  recv(p);
}
extern "C" int main() {
  try {
    try_main();
    return 0;
  } catch (...) {
    return 1;
  }
}
