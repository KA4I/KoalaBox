#pragma once

#include <functional>
#include <regex>
#include <string>
#include <vector>

#ifndef KOALABOX_PARSER
#define KOALABOX_PARSER 0
#endif

#if KOALABOX_PARSER
#  if __has_include(<cpp-tree-sitter.h>)
#    include <cpp-tree-sitter.h>
#  else
#    undef KOALABOX_PARSER
#    define KOALABOX_PARSER 0
#  endif
#endif

namespace koalabox::parser {
#if KOALABOX_PARSER
    ts::Tree parse_source(const std::string_view& buffer);

    struct query_entry {
        std::string path;
        std::string_view value;
    };

    std::vector<query_entry> query(const std::string_view& source, const std::regex& selector);

    enum class visit_result {
        Continue,
        SkipChildren,
        Stop,
    };

    void walk(const ts::Node& root, const std::function<visit_result(const ts::Node&)>& visit);
#else
    struct query_entry { std::string path; std::string_view value; };
    enum class visit_result { Continue, SkipChildren, Stop };
    inline std::vector<query_entry> query(const std::string_view&, const std::regex&) { return {}; }
    template<typename... Args>
    inline void walk(Args&&...) {}
#endif
}
