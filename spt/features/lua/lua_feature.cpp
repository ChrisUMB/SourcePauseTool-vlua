#include "stdafx.hpp"
#include "lua_feature.hpp"
#include <filesystem>
#include <fstream>
#include <utility>
#include "../ent_props.hpp"
#include "interfaces.hpp"
#include "libs/lua_lib_console.hpp"
#include "libs/lua_lib_entity.hpp"
#include "libs/lua_lib_events.hpp"
#include "libs/lua_lib_game.hpp"
#include "libs/lua_lib_input.hpp"
#include "libs/lua_lib_math.hpp"
#include "libs/lua_lib_player.hpp"
#include "libs/lua_lib_render.hpp"
#include "libs/lua_lib_syscall.hpp"
#include "lua_commands.hpp"
#include "lua_util.hpp"
#include "signals.hpp"
#include "spt/sptlib-wrapper.hpp"

LuaFeature spt_lua;

namespace patterns {

    static constexpr auto ptn_TeleportTouchingEntity_1 = Pattern<count_bytes(
            "81 EC ?? ?? ?? ?? 55 8B E9 89 6C 24 14 E8 ?? ?? ?? ??")>(
            "81 EC ?? ?? ?? ?? 55 8B E9 89 6C 24 14 E8 ?? ?? ?? ??");

    constexpr auto TeleportTouchingEntity = make_pattern_array(
            PatternWrapper{"5135", ptn_TeleportTouchingEntity_1});

    static constexpr auto ptn_GetPortalCallQueue_1 = Pattern<count_bytes(
            "33 C0 39 05 ?? ?? ?? ?? 0F 9E C0 83 E8 01 25 ?? ?? ?? ?? C3")>(
            "33 C0 39 05 ?? ?? ?? ?? 0F 9E C0 83 E8 01 25 ?? ?? ?? ?? C3");

    constexpr auto GetPortalCallQueue = make_pattern_array(
            PatternWrapper{"5135", ptn_GetPortalCallQueue_1});

}

bool LuaFeature::ShouldLoadFeature() {
    return true;
}

extern "C" __declspec(noinline) void AMOGUS(const char *str) {
    Msg(str);
}

void LuaFeature::LoadFeature() {
    InitConcommandBase(spt_lua_run_command);
    InitConcommandBase(spt_lua_reset_command);

    RegisterLibrary(&lua_console_library);
    RegisterLibrary(&lua_syscall_library);
    RegisterLibrary(&lua_events_library);
    RegisterLibrary(&lua_input_library);
    RegisterLibrary(&lua_game_library);
    RegisterLibrary(&lua_math_library);
    RegisterLibrary(&lua_player_library);
    RegisterLibrary(&lua_entity_library);
    RegisterLibrary(&lua_render_library);

    void (*tick)() = []() {
        static int ticks = 0;

        lua_events_library.InvokeEvent("tick", [](lua_State *L) {
            lua_newtable(L);
            lua_pushinteger(L, ++ticks);
            lua_setfield(L, -2, "tick");
        });
    };

    TickSignal.Connect(tick);

    void (*level_init)(const char *) = [](char const *map) {
        lua_events_library.InvokeEvent("level_init", [&](lua_State *L) {
            lua_newtable(L);
            lua_pushstring(L, map);
            lua_setfield(L, -2, "map");
        });
    };

    LevelInitSignal.Connect(level_init);

    void (*client_active)(edict_t *) = [](edict_t *entity) {
        lua_events_library.InvokeEvent("client_active", [](lua_State *L) { lua_newtable(L); });
    };

    ClientActiveSignal.Connect(client_active);

    if (OngroundSignal.Works) {
        void (*on_ground)(bool) = [](bool has_ground_entity) {
            static bool was_on_ground = false;

            if (was_on_ground && !has_ground_entity) {
                lua_events_library.InvokeEvent("player_ungrounded", [](lua_State *L) { lua_newtable(L); });
            }

            if (!was_on_ground && has_ground_entity) {
                lua_events_library.InvokeEvent("player_grounded", [](lua_State *L) { lua_newtable(L); });
            }

            was_on_ground = has_ground_entity;
        };

        OngroundSignal.Connect(on_ground);
    }
}

void LuaFeature::UnloadFeature() {}

void LuaFeature::InitHooks() {
    AddPatternHook(patterns::TeleportTouchingEntity,
                   "server",
                   "TeleportTouchingEntity",
                   reinterpret_cast<void **>(&ORIG_TeleportTouchingEntity),
                   reinterpret_cast<void *>(HOOKED_TeleportTouchingEntity));

    AddPatternHook(patterns::GetPortalCallQueue,
                   "server",
                   "GetPortalCallQueue",
                   reinterpret_cast<void **>(&ORIG_GetPortalCallQueue),
                   nullptr);
}

void __fastcall LuaFeature::HOOKED_TeleportTouchingEntity(void *thisptr, int _edx, void *other) {
    if (spt_lua.ORIG_GetPortalCallQueue()) {
        spt_lua.ORIG_TeleportTouchingEntity(thisptr, other);
        return;
    }

    int hammer_id = (int) ((uintptr_t) other + spt_entprops.GetFieldOffset("CBaseEntity", "m_iHammerID", true));

    Vector *p_pos =
            (Vector *) ((uintptr_t) other + spt_entprops.GetFieldOffset("CBaseEntity", "m_vecAbsOrigin", true));

    Vector *p_rot =
            (Vector *) ((uintptr_t) other + spt_entprops.GetFieldOffset("CBaseEntity", "m_angAbsRotation", true));

    QAngle *p_ang = (QAngle *) ((uintptr_t) other + spt_entprops.GetFieldOffset("CBaseEntity", "m_angRotation", true));

    bool is_player = other == spt_entprops.GetPlayer(true);

    Vector old_pos = *p_pos;
    Vector old_rot = *p_rot;
    QAngle old_ang = *p_ang;

    spt_lua.ORIG_TeleportTouchingEntity(thisptr, other);

    Vector new_pos = *p_pos;
    Vector new_rot = *p_rot;
    QAngle new_ang = *p_ang;

    if(is_player) {
        lua_player_library.UpdateLocals(old_pos, old_ang, new_pos, new_ang);
    }

    auto event_invocation = [&](lua_State *L) {
        lua_newtable(L);

        lua_pushinteger(L, hammer_id);
        lua_setfield(L, -2, "hammer_id");

        LuaMathLibrary::LuaPushVector3D(L, old_pos);
        lua_setfield(L, -2, "old_pos");

        LuaMathLibrary::LuaPushVector3D(L, old_rot);
        lua_setfield(L, -2, "old_rot");

        LuaMathLibrary::LuaPushAngle(L, old_ang);
        lua_setfield(L, -2, "old_ang");

        LuaMathLibrary::LuaPushVector3D(L, new_pos);
        lua_setfield(L, -2, "new_pos");

        LuaMathLibrary::LuaPushVector3D(L, new_rot);
        lua_setfield(L, -2, "new_rot");

        LuaMathLibrary::LuaPushAngle(L, new_ang);
        lua_setfield(L, -2, "new_ang");
    };

    lua_events_library.InvokeEvent("entity_teleport", event_invocation);

    if (is_player) {
        lua_events_library.InvokeEvent("player_teleport", event_invocation);
    }
}

void LuaFeature::InitDirectory() {
    auto game_dir = std::string(interfaces::engine->GetGameDirectory());

    if (!fs::exists(game_dir + "\\lua")) {
        fs::create_directory(game_dir + "\\lua");
    }

    if (!fs::exists(game_dir + "\\lua\\libraries")) {
        fs::create_directory(game_dir + "\\lua\\libraries");
        //TODO: Perhaps install `vtas` as a default library?
    }

    if (!fs::exists(game_dir + "\\lua\\scripts")) {
        fs::create_directory(game_dir + "\\lua\\scripts");
    }

    if (!fs::exists(game_dir + "\\lua\\docs")) {
        fs::create_directory(game_dir + "\\lua\\docs");
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

    auto game_dir = std::string(interfaces::engine->GetGameDirectory());
    auto lib_dir_string = game_dir + "\\lua\\libraries\\";
    const char *lib_dir = lib_dir_string.c_str();

    //this needs to use the game directory and not just assume portal
    lua_pushstring(L, lib_dir);
    lua_setglobal(L, "LIB_DIR");

    LoadLibraries(L);
    lua_getglobal(L, "package");
    lua_getfield(L, -1, "cpath");
    std::string current_cpath = lua_tostring(L, -1);
    auto new_cpath = current_cpath + ";" + lib_dir + "?.dll";
    lua_pop(L, 1);
    lua_pushstring(L, new_cpath.c_str());
    lua_setfield(L, -2, "cpath");
    lua_pop(L, 1);

    lua_getglobal(L, "package");
    lua_getfield(L, -1, "path");
    std::string current_path = lua_tostring(L, -1);
    auto new_path = current_path + ";" + lib_dir + "\\?.lua;" + lib_dir + "?.luac;";

    for (const auto &entry: std::filesystem::directory_iterator(lib_dir)) {
        if (entry.is_directory()) {
            auto path = entry.path().string();
            auto file_name = entry.path().filename().string();
            new_path += path + "\\" + file_name + ".lua;" + path + "\\" + file_name + ".luac;";
        }
    }

    lua_pop(L, 1);
    lua_pushstring(L, new_path.c_str());
    lua_setfield(L, -2, "path");
    lua_pop(L, 1);
}

void LuaFeature::RegisterLibrary(LuaLibrary *library, bool write_docs) {
    this->libraries.push_back(library);

    const std::string &source = library->GetLuaSource();

    if (write_docs) {
        auto game_dir = std::string(interfaces::engine->GetGameDirectory());
        std::ofstream out(game_dir + "\\lua\\docs\\" + library->name + ".lua");
        out << RemoveFunctionBodies(source);
        out.close();
    }

    ResetLuaState();// We do this so if a feature asks for the lua state before other libraries are loaded, those other libraries will be loaded into that lua state.
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
