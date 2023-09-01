export module terminus;
import hai;
import jute;
import silog;

namespace terminus {
export class spawn_failed {};
export class recv_failed {};
export class send_failed {};

export class buffer {
  static constexpr const auto buf_size = 1024;
  char m_data[buf_size];
  unsigned m_size{buf_size};

public:
  [[nodiscard]] constexpr auto *data() noexcept { return m_data; }
  [[nodiscard]] constexpr auto *data() const noexcept { return m_data; }

  [[nodiscard]] constexpr auto capacity() const noexcept { return buf_size; }
  [[nodiscard]] constexpr auto size() const noexcept { return m_size; }

  constexpr void set_size(unsigned s) noexcept {
    if (s < buf_size)
      m_size = s;
  }
};

export class prog {
public:
  prog() = default;
  virtual ~prog() = default;

  prog(const prog &) = delete;
  prog(prog &&) = delete;
  prog &operator=(const prog &) = delete;
  prog &operator=(prog &&) = delete;

  virtual void send(jute::view chars) = 0;
  virtual void recv(buffer *b) = 0;
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
