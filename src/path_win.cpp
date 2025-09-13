// Removed dependency on utf8cpp; rely on internal wide<->utf8 helpers
#include <Windows.h>
#include "koalabox/path.hpp"
#include "koalabox/str.hpp"

namespace koalabox::path {
    fs::path from_wstr(const std::wstring& wstr) {
        // Convert wide string to UTF-8 multibyte then to u8string for construction
        const auto utf8 = str::to_str(wstr);
        const auto u8 = std::u8string(utf8.begin(), utf8.end());
        return {u8};
    }

    std::wstring to_wstr(const fs::path& path) {
        // Get UTF-8 representation of the path and widen it
        const auto u8str = path.generic_u8string();
        const std::string narrow(u8str.begin(), u8str.end());
        return str::to_wstr(narrow);
    }

    string to_kb_str(const fs::path& path) {
        return to_wstr(path); // platform specific string alias (wstring on Windows)
    }
}
