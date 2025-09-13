#pragma once

#ifndef KOALABOX_ZIP
#define KOALABOX_ZIP 1
#endif

#include <filesystem>
#include <functional>
#include <string>

namespace koalabox::zip {
    namespace fs = std::filesystem;

#if KOALABOX_ZIP
    void extract_files(
        const fs::path& zip_path,
        const std::function<fs::path(const std::string & name, bool is_dir)>& predicate
    );
#else
    template<typename F>
    inline void extract_files(const fs::path&, F&&) {}
#endif
}