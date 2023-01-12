#pragma once

#include <core/common.h>
#include <rsl/Expected.hpp>

#include <fmt/core.h>

namespace rsl {

bool FileDialogsSupported();

rsl::expected<std::filesystem::path, std::string>
OpenOneFile(std::string_view title, std::string_view default_path,
            std::vector<std::string> filters);

rsl::expected<std::vector<std::filesystem::path>, std::string>
OpenManyFiles(std::string_view title, std::string_view default_path,
              std::vector<std::string> filters);

struct File {
  std::filesystem::path path;
  std::vector<u8> data;
};

rsl::expected<File, std::string> ReadOneFile(std::filesystem::path path);
rsl::expected<File, std::string> ReadOneFile(std::string_view title,
                                             std::string_view default_path,
                                             std::vector<std::string> filters);

rsl::expected<std::vector<File>, std::string>
ReadManyFile(std::string_view title, std::string_view default_path,
             std::vector<std::string> filters);

rsl::expected<std::filesystem::path, std::string>
SaveOneFile(std::string_view title, std::string_view default_path,
            std::vector<std::string> filters);

// Part of the same underlying library (pfd) but perhaps belongs in a different
// header
void ErrorDialog(const std::string& msg);

template <typename... Args>
inline void ErrorDialogFmt(fmt::format_string<Args...> msg, Args&&... args) {
  auto formatted = fmt::format(msg, std::forward<Args>(args)...);
  ErrorDialog(formatted);
}

} // namespace rsl
