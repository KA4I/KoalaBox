#include <Windows.h>
#include <format>
#include <fmt/format.h>
#include <stdexcept>
#include <string>

#include "koalabox/module.hpp"
#include "koalabox/path.hpp"

namespace {
    PIMAGE_SECTION_HEADER get_pe_section_header_or_throw(
        const HMODULE& module_handle,
        const std::string& section_name
    ) {
        const auto* dos_header = reinterpret_cast<const IMAGE_DOS_HEADER*>(module_handle);
        if(!dos_header || dos_header->e_magic != IMAGE_DOS_SIGNATURE) {
            throw std::runtime_error("Invalid DOS header");
        }

        const auto* nt_headers = reinterpret_cast<const IMAGE_NT_HEADERS*>(
            reinterpret_cast<const uint8_t*>(module_handle) + dos_header->e_lfanew
        );
        if(!nt_headers || nt_headers->Signature != IMAGE_NT_SIGNATURE) {
            throw std::runtime_error("Invalid NT signature");
        }

        auto* section = IMAGE_FIRST_SECTION(const_cast<IMAGE_NT_HEADERS*>(nt_headers));
        for(int i = 0; i < nt_headers->FileHeader.NumberOfSections; i++, section++) {
            std::string name(reinterpret_cast<char*>(section->Name), 8);
            if(auto nul = name.find('\0'); nul != std::string::npos) {
                name = name.substr(0, nul);
            }
            if(name == section_name) {
                return section;
            }
        }
        throw std::runtime_error(fmt::format("Section '{}' not found", section_name));
    }
}

namespace koalabox::module {
    std::filesystem::path get_fs_path(const void* module_handle) {
        WCHAR buffer[MAX_PATH];
        const auto size = GetModuleFileNameW(
            reinterpret_cast<HMODULE>(const_cast<void*>(module_handle)),
            buffer,
            MAX_PATH
        );
        if(size == 0) {
            throw std::runtime_error("GetModuleFileNameW failed");
        }
        return path::from_wstr(buffer);
    }

    std::optional<void*> get_function_address(
        void* module_handle,
        const char* function_name
    ) {
        if(auto* address = GetProcAddress(reinterpret_cast<HMODULE>(module_handle), function_name)) {
            return address;
        }
        return {};
    }

    void* get_function_address_or_throw(void* module_handle, const char* function_name) {
        if(auto addr = get_function_address(module_handle, function_name)) {
            return *addr;
        }
        throw std::runtime_error(fmt::format("Function '{}' not found", function_name));
    }

    section_t get_section_or_throw(const void* module_handle, const std::string& section_name) {
        const auto section = get_pe_section_header_or_throw(
            reinterpret_cast<HMODULE>(const_cast<void*>(module_handle)), section_name
        );
        const auto base = reinterpret_cast<uintptr_t>(module_handle);
        const auto start = base + section->PointerToRawData;
        return section_t{
            .start_address = reinterpret_cast<void*>(start),
            .end_address = reinterpret_cast<void*>(start + section->SizeOfRawData),
            .size = section->SizeOfRawData,
        };
    }

    std::optional<section_t> get_section(const void* module_handle, const std::string& section_name) {
        try {
            return get_section_or_throw(module_handle, section_name);
        } catch(...) {
            return {};
        }
    }

    std::optional<void*> load_library(const std::filesystem::path& library_path) {
        if(auto* handle = LoadLibraryW(path::to_wstr(library_path).c_str())) {
            return reinterpret_cast<void*>(handle);
        }
        return {};
    }

    void* load_library_or_throw(const std::filesystem::path& library_path) {
        if(auto h = load_library(library_path)) {
            return *h;
        }
        throw std::runtime_error(fmt::format("Failed to load library: {}", path::to_str(library_path)));
    }

    void unload_library(void* library_handle) {
        if(library_handle) {
            FreeLibrary(reinterpret_cast<HMODULE>(library_handle));
        }
    }

    void* get_library_handle(const TCHAR* library_name) {
        return reinterpret_cast<void*>(GetModuleHandle(library_name));
    }
}
