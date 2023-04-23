#include "stdafx.hpp"
#include <filesystem>
#include <fstream>
#include <utility>
#include "lua_feature.hpp"
#include "interfaces.hpp"
#include "lua_util.hpp"
#include "lua_commands.hpp"
#include "libs/lua_lib_console.hpp"

LuaFeature spt_lua;

bool LuaFeature::ShouldLoadFeature() {
    return true;
}

void LuaFeature::LoadFeature() {
    InitConcommandBase(lua_run_command);

    InitDirectory();

    RegisterLibrary(&lua_console_library);
//    TickSignal.Connect(this, &LuaFeature::OnTick);
}

void LuaFeature::UnloadFeature() {}

void LuaFeature::InitHooks() {
}

void LuaFeature::OnTick() {

}

void LuaFeature::InitDirectory() {
    auto game_dir = std::string(interfaces::engine->GetGameDirectory());

    if (!fs::exists(game_dir + "\\lua")) {
        fs::create_directory(game_dir + "\\lua");
    }

    if (!fs::exists(game_dir + "\\lua\\configs")) {
        fs::create_directory(game_dir + "\\lua\\configs");
    }

    if (!fs::exists(game_dir + "\\lua\\libraries")) {
        fs::create_directory(game_dir + "\\lua\\libraries");
        //TODO: Perhaps install `vtas` as a default library?
    }

    if (!fs::exists(game_dir + "\\lua\\scripts")) {
        fs::create_directory(game_dir + "\\lua\\scripts");
    }

    const auto &docs_dir = game_dir + "\\lua\\docs";

    if (!fs::exists(docs_dir)) {
        fs::create_directory(docs_dir);
        //TODO: Write documentation to the docs folder.

//        for (const auto &[filename, source]: LUA_DOCS) {
//            const auto file_path = docs_dir + "\\" + filename;
//            std::filesystem::create_directories(std::filesystem::path(file_path).parent_path());
//            std::ofstream out(file_path);
//            out << source;
//            out.close();
//        }
    }
}

lua_State *LuaFeature::GetLuaState() {
    if (L == nullptr) {
        L = luaL_newstate();
        luaL_openlibs(L);
        LoadLibraries(L);
    }

    return L;
}

void LuaFeature::ResetLuaState() {
    if (L == nullptr) {
        return;
    }

    UnloadLibraries(L);
    lua_close(L);
    L = nullptr;
}

void LuaFeature::RegisterLibrary(LuaLibrary *library) {
    this->libraries.push_back(library);
    ResetLuaState(); // We do this so if a feature asks for the lua state before other libraries are loaded, those other libraries will be loaded into that lua state.
}

void LuaFeature::LoadLibraries(lua_State *state) {
    for (const auto &item: this->libraries) {
        item->Load(state);
    }
}

void LuaFeature::UnloadLibraries(lua_State *state) {
    for (const auto &item: this->libraries) {
        item->Unload(state);
    }
}

LuaLibrary::LuaLibrary(std::string name) : name(std::move(name)) {}

void LuaLibrary::Unload(lua_State *L) {
    lua_pushnil(L);
    lua_setglobal(L, this->name.c_str());
}

const std::string &LuaLibrary::GetLuaSource() {
    static std::string default_sources;
    return default_sources;
}
