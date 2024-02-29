#pragma once

#include "lua.hpp"
#include <filesystem>

struct file_path_t {
    std::string path; // The full path
    std::string prefix; // Everything before the last `/` or `\`
    std::string suffix; // Everything after the last `/` or `\`
};

void lua_string_format(lua_State* L);

void lua_new_class(lua_State* L, const char* name, const luaL_Reg* functions);

std::string RemoveFunctionBodies(const std::string& lua_source);

void DebugLuaStack(lua_State* L);

bool LuaIsClass(lua_State* L, int index, const char* class_name);

bool LuaCheckClass(lua_State* L, int index, const char* class_name);

void GetFilePath(file_path_t& file_path, const std::string& partial, const char* default_path);

std::vector<std::string> GetFileSuggestions(file_path_t& file_path, const std::string& file_suffix);
