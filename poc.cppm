module;
#include <stdio.h>

export module poc;
import hai;
import jute;
import terminus;

void recv(auto &p) {
  auto str = p->recv();
  while ((*str).size() > 0) {
    printf("%*s", static_cast<int>((*str).size()), (*str).data());
    str = p->recv();
  }
}

extern "C" int main() {
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
