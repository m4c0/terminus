module;
#include <stdio.h>

export module poc;
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
  auto p = terminus::spawn("vim"_s, "-T"_s, "builtin_dumb"_s);
  recv(p);
  p->send(":q");
  recv(p);
}
