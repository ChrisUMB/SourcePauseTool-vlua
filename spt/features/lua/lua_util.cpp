#include "stdafx.hpp"
#include "lua_util.hpp"
#include "interfaces.hpp"

void lua_string_format(lua_State *L) {
    int arg_count = lua_gettop(L);

    lua_getglobal(L, "string");
    lua_getfield(L, -1, "format");
    lua_insert(L, 1);
    lua_pop(L, 1);

    lua_call(L, arg_count, 1);
}

void lua_new_class(lua_State *L, const char *name, const luaL_Reg *functions) {
    lua_getglobal(L, name);
    lua_pushvalue(L, -1);
    lua_setfield(L, -1, "__index");

    luaL_register(L, nullptr, functions);
    lua_setfield(L, LUA_REGISTRYINDEX, name);
}

void DebugLuaStack(lua_State *L) {
    int top = lua_gettop(L);

    Msg("-- Lua Stack --\n");
    for (int i = 0; i < top; ++i) {
        Msg("%i : %s\n", i + 1, lua_typename(L, lua_type(L, i + 1)));
    }
    Msg("----------\n");
}

std::string RemoveFunctionBodies(const std::string &lua_source) {
    std::stringstream ss(lua_source);
    std::string line;
    std::string output;

    bool in_function = false;

    while (std::getline(ss, line)) {
        if (in_function) {
            in_function = line != "end";
            continue;
        }

        if(line == "\n" || line == "") {
            output += "\n";
            continue;
        }

        if (line.starts_with("function")) {
            in_function = true;
            output += line.substr(0, line.find_first_of(')') + 1) + " end\n";
            continue;
        }

        output += line + "\n";
    }

    //close buffer
    ss.clear();
    return output;
}

void GetFilePath(file_path_t &file_path, const std::string &partial, const char *default_path) {
    file_path.path = partial;
    file_path.prefix = partial;
    file_path.suffix = partial;

    // Responsible for extracting the folder partial is trying to point to.
    unsigned int index = file_path.prefix.find_last_of('/');
    if (index == std::string::npos) {
        index = file_path.prefix.find_last_of('\\');
    }

    if (index != std::string::npos) {
        file_path.prefix = file_path.prefix.substr(0, index + 1);
        file_path.suffix = file_path.suffix.substr(index + 1);
    } else {
        file_path.prefix = "";
    }

    // If partial is not an absolute path, then point it to game_directory/${default_path}/ to find files.
    if (default_path && file_path.prefix.find(':') == std::string::npos) {
        std::string game_directory(interfaces::engine->GetGameDirectory());

        file_path.path = game_directory + default_path + file_path.prefix;
    }
}

std::vector<std::string> GetFileSuggestions(file_path_t &file_path, const std::string &file_suffix) {
    std::vector<std::string> suggestions;

    if (!fs::exists(file_path.path) || !fs::is_directory(file_path.path)) {
        return suggestions;
    }

    int file_count = 0;
    auto iterator = fs::directory_iterator(
            file_path.path,
            fs::directory_options::skip_permission_denied |
            fs::directory_options::follow_directory_symlink
    );

    for (const auto &entry: iterator) {
        auto file_name = entry.path().filename().string();

        if (!file_name.starts_with(file_path.suffix)) {
            continue;
        }

        if (!entry.is_directory() && !(entry.is_regular_file() && file_name.ends_with(file_suffix))) {


            continue;
        }

        auto suggestion = file_path.prefix + file_name;
        if (entry.is_directory()) {
            suggestion += "/";
        }

        suggestions.push_back(suggestion);
        file_count++;

        if (file_count >= COMMAND_COMPLETION_MAXITEMS) {
            break;
        }
    }

    return suggestions;
}

bool LuaCheckClass(lua_State *L, int index, const char *class_name) {
    if (!LuaIsClass(L, index, class_name)) {
        luaL_error(L, "Expected %s", class_name);
        return false;
    }
    return true;
}

bool LuaIsClass(lua_State *L, int index, const char *class_name) {
    lua_getmetatable(L, index);
    lua_getglobal(L, class_name);
    bool is_class = lua_rawequal(L, -1, -2);
    lua_pop(L, 2);
    return is_class;
}
