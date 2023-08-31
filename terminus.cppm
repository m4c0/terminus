export module terminus;
import hai;
import jute;

namespace terminus {
export class prog {
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
export [[nodiscard]] hai::uptr<prog> spawn(auto... args) {
  jute::view a[sizeof...(args)]{args...};
  return spawn_p(a, sizeof...(args));
}
} // namespace terminus
