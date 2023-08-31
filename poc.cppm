module;
#include <stdio.h>

export module poc;
import hai;
import jute;

namespace terminus {
class prog {
public:
  virtual ~prog() = 0;

  prog(const prog &) = delete;
  prog(prog &&) = delete;
  prog &operator=(const prog &) = delete;
  prog &operator=(prog &&) = delete;

  virtual void send(jute::view chars) = 0;
  [[nodiscard]] virtual jute::heap recv() = 0;
};

[[nodiscard]] hai::uptr<prog> spawn_p(jute::view *argv, unsigned argc);
[[nodiscard]] hai::uptr<prog> spawn(auto... args) {
  jute::view a[sizeof...(args)]{args...};
  return spawn_p(a, sizeof...(args));
}
} // namespace terminus

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
