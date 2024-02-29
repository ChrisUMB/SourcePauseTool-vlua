#include "stdafx.hpp"
#include <filesystem>
#include <fstream>
#include "interfaces.hpp"
#include "lua_util.hpp"
#include "lua_feature.hpp"
#include "lua_commands.hpp"

static int LuaRunCommandAutoComplete(char const* partial_cstr,
                                     char commands[COMMAND_COMPLETION_MAXITEMS][COMMAND_COMPLETION_ITEM_LENGTH]) {
    std::string partial(partial_cstr);

    while (partial[0] == ' ') {
        partial = partial.substr(1);
    }

    partial = partial.substr(12);
    file_path_t file_path;
    GetFilePath(file_path, partial, R"(\lua\scripts\)");

    std::vector<std::string> suggestions = GetFileSuggestions(file_path, ".lua");
    if (suggestions.empty()) {
        GetFilePath(file_path, partial, "\\"); // base game directory
        suggestions = GetFileSuggestions(file_path, ".lua");
    }

    for (int i = 0; i < suggestions.size(); ++i) {
        std::string string = "spt_lua_run " + suggestions[i];
        strcpy(commands[i], string.c_str());
    }

    return (int)suggestions.size();
}

static void LuaRunCommand(const CCommand& args) {
    if (args.ArgC() < 1 || strcmp(args.Arg(1), "") == 0) {
        Warning("You must provide a lua file name.");
        return;
    }

    std::string file_name = std::string(args.Arg(1));

    while (file_name[0] == ' ') {
        file_name = file_name.substr(1);
    }

    std::string script_absolute_path = file_name;

    if (file_name.find(':') == std::string::npos) {
        std::string game_directory(interfaces::engine->GetGameDirectory());

        script_absolute_path = game_directory + R"(\lua\scripts\)" + file_name;
    }

    if (script_absolute_path.substr(script_absolute_path.size() - 4) != ".lua") {
        script_absolute_path += ".lua";
    }

    if (!std::filesystem::exists(script_absolute_path)) {
        // try the base game directory
        if (file_name.find(':') != std::string::npos) {
            Warning("Couldn't find file: %s\n", script_absolute_path.c_str());
            return;
        } else {
            std::string previous_script_absolute_path = script_absolute_path;
            script_absolute_path = std::string(interfaces::engine->GetGameDirectory()) + "\\" + file_name;

            if (script_absolute_path.substr(script_absolute_path.size() - 4) != ".lua") {
                script_absolute_path += ".lua";
            }

            if (!std::filesystem::exists(script_absolute_path)) {
                Warning("Couldn't find file:\n%s\n%s\n",
                        previous_script_absolute_path.c_str(),
                        script_absolute_path.c_str());
                return;
            }
        }
    }

    DevMsg("Running script: %s\n", script_absolute_path.c_str());
    int status = luaL_loadfile(spt_lua.GetLuaState(), script_absolute_path.c_str());

    if (status) {
        Warning("Couldn't load file: %s\n", script_absolute_path.c_str());
        Warning("Lua Error\n%s\n", lua_tostring(spt_lua.GetLuaState(), -1));
        return;
    }

    for (int i = 2; i < args.ArgC(); ++i) {
        lua_pushstring(spt_lua.GetLuaState(), args.Arg(i));
    }

    int result = lua_pcall(spt_lua.GetLuaState(), MAX(args.ArgC() - 2, 0), 0, 0);

    if (result) {
        Warning("Something went wrong when running file: %s\n", lua_tostring(spt_lua.GetLuaState(), -1));
        return;
    }
}

ConCommand spt_lua_run_command =
    ConCommand("spt_lua_run", LuaRunCommand, "Loads and executes a lua file.", 0, LuaRunCommandAutoComplete);

ConCommand spt_lua_reset_command = ConCommand(
    "spt_lua_reset",
    [](const CCommand& args) {
        spt_lua.ResetLuaState();
    },
    "Resets the lua state.",
    0);
