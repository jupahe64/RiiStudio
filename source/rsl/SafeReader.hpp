#pragma once

#include <expected>
#include <format>
#include <oishii/reader/binary_reader.hxx>
#include <rsl/Ranges.hpp>
#include <sstream>
#include <vendor/magic_enum/magic_enum.hpp>

namespace rsl {

inline std::string EnumError(u32 bad, auto&& good) {
  auto values_printed =
      rsl::join(good | std::views::transform([](auto x) {
                  auto [id, name] = x;
                  auto id_raw = static_cast<u32>(id);
                  return std::format("{}={}=0x{:x}", name, id_raw, id_raw);
                }),
                ", ");
  return std::format(
      "Invalid enum value. Expected one of ({}). Instead saw {} (0x{:x}).",
      values_printed, bad, bad);
}
class SafeReader {
public:
  template <typename T> using Result = std::expected<T, std::string>;

  SafeReader(oishii::BinaryReader& reader) : mReader(reader) {}

  // Slicing shouldn't fail, python approach. Just returns a null span.
  auto slice(u32 ofs = 0) -> std::span<const u8> {
    auto pos = mReader.tell();
    if (pos >= mReader.endpos()) {
      return {};
    }
    if (pos < 0) {
      return {};
    }
    return mReader.slice().subspan(ofs);
  }

  // Doesn't ever fail
  void seekSet(u32 pos) { mReader.seekSet(pos); }
  auto tell() const -> u32 { return mReader.tell(); }

  auto F32() -> Result<f32> { return mReader.tryRead<f32>(); }
  auto U32() -> Result<u32> { return mReader.tryRead<u32>(); }
  auto S32() -> Result<s32> { return mReader.tryRead<s32>(); }
  auto U16() -> Result<u16> { return mReader.tryRead<u16>(); }
  auto S16() -> Result<s16> { return mReader.tryRead<s16>(); }
  auto U8() -> Result<u8> { return mReader.tryRead<u8>(); }
  auto S8() -> Result<s8> { return mReader.tryRead<s8>(); }

  template <typename T, typename E> auto Enum() -> Result<E> {
    static_assert(std::is_integral_v<T>);
    static_assert(sizeof(T) <= sizeof(u32));

    auto u = TRY(mReader.tryRead<T>());
    auto as_enum = magic_enum::enum_cast<E>(u);
    if (!as_enum.has_value()) {
      auto values = magic_enum::enum_entries<E>();
      auto msg = EnumError(u, values);
      mReader.warnAt(msg.c_str(), mReader.tell() - sizeof(T), mReader.tell());
      return std::unexpected(msg);
    }
    return static_cast<E>(u);
  }

  template <typename E> auto Enum32() -> Result<E> { return Enum<u32, E>(); }
  template <typename E> auto Enum8() -> Result<E> { return Enum<u8, E>(); }

  template <size_t size> auto U8Buffer() -> Result<std::array<u8, size>> {
    auto buf = TRY(mReader.tryReadBuffer<u8>(size));
    return buf | rsl::ToArray<size>();
  }
  template <size_t size> auto CharBuffer() -> Result<std::array<char, size>> {
    auto buf = TRY(mReader.tryReadBuffer<char>(size));
    return buf | rsl::ToArray<size>();
  }

  auto Magic(std::string_view ident) -> Result<std::string_view> {
    auto buf = TRY(mReader.tryReadBuffer<char>(ident.size()));
    if (!std::ranges::equal(buf, ident)) {
      auto buf_s = std::string(buf.begin(), buf.end());
      auto msg =
          std::format("Expected magic identifier {} at {}. Instead saw {}.",
                      ident, mReader.tell() - ident.size(), buf_s);
      mReader.warnAt(msg.c_str(), mReader.tell() - ident.size(),
                     mReader.tell());
      return std::unexpected(msg);
    }
    return ident;
  }

  inline Result<std::string> StringOfs32(u32 relative);

  auto scoped(std::string&& name) {
    return mReader.createScoped(std::move(name));
  }

private:
  oishii::BinaryReader& mReader;
};

SafeReader::Result<std::string> SafeReader::StringOfs32(u32 relative) {
  auto ofs = TRY(mReader.tryRead<s32>());

  // semi-likely
  if (ofs == 0) {
    return "";
  }

  [[unlikely]] if (relative + ofs >= mReader.endpos() || relative + ofs < 0) {
    return std::unexpected(
        std::format("Invalid string offset {}. Out of file bounds [0, {})",
                    ofs + relative, mReader.endpos()));
  }

  auto span = mReader.slice().subspan(relative + ofs);
  bool terminated = false;
  for (size_t i = 0; i < span.size(); ++i) {
    if (span[i] == 0) {
      span = span.subspan(0, i);
      terminated = true;
      break;
    }
  }

  // very unlikely
  [[unlikely]] if (!terminated) {
    return std::unexpected("File has been truncated. String does not contain "
                           "a final null terminator");
  }

  return std::string{reinterpret_cast<const char*>(span.data()),
                     reinterpret_cast<const char*>(span.data() + span.size())};
}

} // namespace rsl