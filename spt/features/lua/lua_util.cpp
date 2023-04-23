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