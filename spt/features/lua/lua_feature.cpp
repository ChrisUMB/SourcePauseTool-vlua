#include "stdafx.hpp"
#include <filesystem>
#include <fstream>
#include <utility>
#include "lua_feature.hpp"
#include "interfaces.hpp"
#include "lua_util.hpp"
#include "lua_commands.hpp"
#include "libs/lua_lib_console.hpp"
#include "libs/lua_lib_syscall.hpp"
#include "libs/lua_lib_events.hpp"
#include "libs/lua_lib_input.hpp"
#include "libs/lua_lib_game.hpp"
#include "libs/lua_lib_math.hpp"
#include "libs/lua_lib_player.hpp"
#include "libs/lua_lib_render.hpp"
#include "signals.hpp"
#include "../visualizations/renderer/mesh_renderer.hpp"

LuaFeature spt_lua;

bool LuaFeature::ShouldLoadFeature() {
    return true;
}

void LuaFeature::LoadFeature() {
    InitConcommandBase(spt_lua_run_command);
    InitConcommandBase(spt_lua_reset_command);

    InitDirectory();

    RegisterLibrary(&lua_console_library);
    RegisterLibrary(&lua_syscall_library);
    RegisterLibrary(&lua_events_library);
    RegisterLibrary(&lua_input_library);
    RegisterLibrary(&lua_game_library);
    RegisterLibrary(&lua_math_library);
    RegisterLibrary(&lua_player_library);
    RegisterLibrary(&lua_render_library);

    void (*tick)() = []() {
        static int ticks = 0;

        lua_events_library.InvokeEvent("tick", [](lua_State *L) {
            lua_newtable(L);
            lua_pushinteger(L, ++ticks);
            lua_setfield(L, -2, "tick");
        });
    };

    TickSignal.Connect(*tick);

    void (*level_init)(const char *) = [](char const *map) {
        lua_events_library.InvokeEvent("level_init", [&](lua_State *L) {
            lua_newtable(L);
            lua_pushstring(L, map);
            lua_setfield(L, -2, "map");
        });
    };

    LevelInitSignal.Connect(*level_init);

    void (*client_active)(edict_t *entity) = [](edict_t *entity) {
        lua_events_library.InvokeEvent("client_active", [](lua_State *L) {
            lua_newtable(L);
        });
    };

    ClientActiveSignal.Connect(*client_active);

//    static const StaticMesh &mesh = spt_meshBuilder.CreateStaticMesh([](MeshBuilderDelegate &mb) {
//        mb.AddBox({0, 0, 0}, {0, 0, 0}, {64, 64, 64}, {0, 0, 0}, {C_OUTLINE(0, 85, 255, 100)});
//    });
//
//    void (*rendering)(MeshRendererDelegate &mr) = [](MeshRendererDelegate &mr) {
////        const DynamicMesh &mesh = spt_meshBuilder.CreateDynamicMesh([](MeshBuilderDelegate &mb) {
////            mb.AddBox({0, 0, 0}, {0, 0, 0}, {64, 64, 64}, {0, 0, 0}, {C_OUTLINE(0, 85, 255, 100)});
////        });
//
////        if(infoIn.cvs.origin.z > 0) {
////            infoOut.colorModulate = {0, 15, 255, 255};
////        }
//
//        for (int i = 0; i < 10; ++i) {
//            const int offset = i;
//            mr.DrawMesh(mesh, [offset](const CallbackInfoIn &infoIn, CallbackInfoOut &infoOut) {
//                PositionMatrix(Vector(0, 0, (float) offset * 96), infoOut.mat);
//            });
//        }
//    };
//
//    spt_meshRenderer.signal.Connect(*rendering);
}

void LuaFeature::UnloadFeature() {}

void LuaFeature::InitHooks() {
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
    if (global_state == nullptr) {
        global_state = luaL_newstate();
        luaL_openlibs(global_state);
        InitLuaState(global_state);
    }

    return global_state;
}

void LuaFeature::ResetLuaState() {
    if (global_state == nullptr) {
        return;
    }

    UnloadLibraries(global_state);
    lua_close(global_state);
    global_state = nullptr;
}

void LuaFeature::InitLuaState(lua_State *L) {
    if (!L) {
        return;
    }

    lua_pushstring(L, "portal/lua/libraries/");
    lua_setglobal(L, "LIB_DIR");

    LoadLibraries(L);
    lua_getglobal(L, "package");
    lua_getfield(L, -1, "cpath");
    std::string current_cpath = lua_tostring(L, -1);
    auto new_cpath = current_cpath + ";" + "./portal/lua/libraries/?.dll";
    lua_pop(L, 1);
    lua_pushstring(L, new_cpath.c_str());
    lua_setfield(L, -2, "cpath");
    lua_pop(L, 1);

    lua_getglobal(L, "package");
    lua_getfield(L, -1, "path");
    std::string current_path = lua_tostring(L, -1);
    auto new_path = current_path + ";" + "./portal/lua/libraries/?.lua;./portal/lua/libraries/?.luac;";

    for (const auto &entry: std::filesystem::directory_iterator("./portal/lua/libraries")) {
        if (entry.is_directory()) {
            auto path = entry.path().string();
            auto file_name = entry.path().filename().string();
            new_path += path + "/" + file_name + ".lua;" + path + "/" + file_name + ".luac;";
        }
    }

    lua_pop(L, 1);
    lua_pushstring(L, new_path.c_str());
    lua_setfield(L, -2, "path");
    lua_pop(L, 1);
}

void LuaFeature::RegisterLibrary(LuaLibrary *library) {
    this->libraries.push_back(library);
    ResetLuaState(); // We do this so if a feature asks for the lua state before other libraries are loaded, those other libraries will be loaded into that lua state.
}

void LuaFeature::LoadLibraries(lua_State *L) {
    for (const auto &item: this->libraries) {
        const std::string &source = item->GetLuaSource();
        if (!source.empty()) {
            luaL_loadstring(L, source.c_str());
            if (!lua_pcall(L, 0, 0, 0)) {
                const char *error = lua_tostring(L, -1);
                if (error != nullptr) {
                    Warning("Lua error loading library \"%s\": %s", item->name.c_str(), error);
                    continue;
                }
            }
        }

        item->Load(L);
    }
}

void LuaFeature::UnloadLibraries(lua_State *L) {
    for (const auto &item: this->libraries) {
        item->Unload(L);
    }
}

LuaLibrary::LuaLibrary(std::string name) : name(std::move(name)) {}

void LuaLibrary::Load(lua_State *L) {}

void LuaLibrary::Unload(lua_State *L) {
    lua_pushnil(L);
    lua_setglobal(L, this->name.c_str());
}

const std::string &LuaLibrary::GetLuaSource() {
    static std::string default_sources;
    return default_sources;
}
