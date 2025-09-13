#include <regex>
#include <map>
#include <Windows.h>
#include <Psapi.h>
#include <nlohmann/json.hpp>

#include "koalabox/logger.hpp"
#include "koalabox/loader.hpp"
#include "koalabox/win.hpp"
#include "koalabox/util.hpp"

namespace koalabox::loader {
    /**
    * Key is undecorated name, value is decorated name, if `undecorate` is set
    */
    std::map<std::string, std::string> get_export_map(void* library, const bool undecorate) {
        // Adapted from: https://github.com/mborne/dll2def/blob/master/dll2def.cpp

        auto exported_functions = std::map<std::string, std::string>();

    // Base address helpers
    const auto* base = reinterpret_cast<const BYTE*>(library);

    // DOS header
    const auto* dos_header = reinterpret_cast<const IMAGE_DOS_HEADER*>(library);

        if(dos_header->e_magic != IMAGE_DOS_SIGNATURE) {
            util::panic("e_magic  != IMAGE_DOS_SIGNATURE");
        }

    // NT headers (architecture specific)
#ifdef _WIN64
    const auto* nt_headers = reinterpret_cast<const IMAGE_NT_HEADERS64*>(base + dos_header->e_lfanew);
#else
    const auto* nt_headers = reinterpret_cast<const IMAGE_NT_HEADERS32*>(base + dos_header->e_lfanew);
#endif

        if(nt_headers->Signature != IMAGE_NT_SIGNATURE) {
            util::panic("header->Signature != IMAGE_NT_SIGNATURE");
        }

        if(nt_headers->OptionalHeader.NumberOfRvaAndSizes == 0) {
            util::panic("header->OptionalHeader.NumberOfRvaAndSizes <= 0");
        }

    const auto& data_dir = nt_headers->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT];
        const auto* exports = reinterpret_cast<const IMAGE_EXPORT_DIRECTORY*>(base + data_dir.VirtualAddress);
        const auto* names_rva_array = reinterpret_cast<const DWORD*>(base + exports->AddressOfNames);

        // Iterate over the names and add them to the vector
        for(unsigned int i = 0; i < exports->NumberOfNames; i++) {
            std::string exported_name = reinterpret_cast<const char*>(base + names_rva_array[i]);

            if(undecorate) {
                std::string undecorated_function = exported_name; // fallback value

                // Extract function name from decorated name
                static const std::regex expression(R"((?:^_)?(\w+)(?:@\d+$)?)");

                if(std::smatch matches; std::regex_match(exported_name, matches, expression)) {
                    if(matches.size() == 2) {
                        undecorated_function = matches[1];
                    } else {
                        LOG_WARN("Exported function regex size != 2: {}", exported_name);
                    }
                } else {
                    LOG_WARN("Exported function regex failed: {}", exported_name);
                }

                exported_functions.insert({undecorated_function, exported_name});
            } else {
                exported_functions.insert({exported_name, exported_name});
            }
        }

        return exported_functions;
    }

    // Match public header signature (void*). Internally cast to HMODULE
    std::string get_decorated_function(void* library, const std::string& function_name) {
#ifdef _WIN64
        return function_name;
#else
        static std::map<HMODULE, std::map<std::string, std::string>> undecorated_function_maps;

        const auto hmod = static_cast<HMODULE>(library);

        if(not undecorated_function_maps.contains(hmod)) {
            undecorated_function_maps[hmod] = get_export_map(library, true);
            LOG_DEBUG(
                "Populated export map of {} with {} entries",
                library, undecorated_function_maps.at(hmod).size()
            );
            LOG_TRACE("Export map:\n{}", nlohmann::json(undecorated_function_maps[hmod]).dump(2));
        }

        try {
            return undecorated_function_maps.at(hmod).at(function_name);
        } catch(const std::exception&) {
            LOG_WARN("Function '{}' not found in export map of {}", function_name, library);
            return function_name;
        }
#endif
    }
}
