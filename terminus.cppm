export module terminus;
import hai;
import jute;
import silog;

namespace terminus {
export class prog {
public:
  prog() = default;
  virtual ~prog() = default;

  prog(const prog &) = delete;
  prog(prog &&) = delete;
  prog &operator=(const prog &) = delete;
  prog &operator=(prog &&) = delete;

  virtual void send(jute::view chars) = 0;
  [[nodiscard]] virtual hai::varray<char> recv() = 0;
};

export struct spawn_params {
  unsigned con_width;
  unsigned con_height;
  hai::array<jute::view> args;
};

export [[nodiscard]] hai::uptr<prog> spawn(const spawn_params &p);
} // namespace terminus

#ifdef __APPLE__
#pragma ecow add_impl spawn
#endif
