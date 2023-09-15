module;
#include <stdio.h>
#pragma leco tool

export module poc;
import hai;
import jute;
import terminus;

void recv(auto &p) {
  terminus::buffer b{};
  p->recv(&b);
  while (b.size() == 0)
    p->recv(&b);

  while (b.size() > 0) {
    fwrite(b.data(), b.size(), 1, stdout);
    fflush(stdout);
    p->recv(&b);
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
  p->send("iThis is fine!\n\e");
  recv(p);
  p->send(":qa!\n");
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
