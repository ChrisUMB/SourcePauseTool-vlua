#pragma once

#include "lua.hpp"
#include <filesystem>
namespace fs = std::filesystem;

struct file_path_t {
    std::string path;   // The full path
    std::string prefix; // Everything before the last `/` or `\`
    std::string suffix; // Everything after the last `/` or `\`
};

void lua_string_format(lua_State *L);

void GetFilePath(file_path_t &file_path, const std::string &partial, const char *default_path);

std::vector<std::string> GetFileSuggestions(file_path_t &file_path, const std::string &file_suffix);