#include "stdafx.hpp"
#include "lua_lib_game.hpp"
#include "interfaces.hpp"
#include "lua_lib_math.hpp"
#include "ent_utils.hpp"
#include "../../demo.hpp"
#include "spt/features/taspause.hpp"

LuaGameLibrary lua_game_library;

static int GetClientTick(lua_State* L) {
    lua_pushinteger(L, interfaces::engine_tool->ClientTick());
    return 1;
}

static int GetServerTick(lua_State* L) {
    lua_pushinteger(L, interfaces::engine_tool->ServerTick());
    return 1;
}

extern ConVar tas_pause;

static int IsPaused(lua_State* L) {
    lua_pushboolean(L, IsGamePaused());
    return 1;
}

static int IsSimulating(lua_State* L) {
    lua_pushboolean(L, IsGameSimulating());
    return 1;
}

static int IsTasPaused(lua_State* L) {
    lua_pushboolean(L, tas_pause.GetBool());
    return 1;
}

static int GetGameDirectory(lua_State* L) {
    lua_pushstring(L, interfaces::engine->GetGameDirectory());
    return 1;
}

static int IsPlayingDemo(lua_State* L) {
    lua_pushboolean(L, spt_demostuff.Demo_IsPlayingBack());
    return 1;
}

static int GetEngineState(lua_State* L) {
    auto eng = reinterpret_cast<DWORD>(GetModuleHandle("engine.dll")) + 0x36E69C;
    eng = *(DWORD*)eng;

    typedef int (__thiscall *GetState_t)(void*);
    const auto GetState = reinterpret_cast<GetState_t>((*(int**)eng)[4]);
    lua_pushinteger(L, GetState((void*)eng));

    return 1;
}

static int Trace(lua_State* L) {
    // Usage: game.trace(pos: vec3, angle: vec3, mask: number, filter: string[]|nil)
    if (!LuaMathLibrary::LuaIsVector3D(L, 1)) {
        return luaL_error(L, "game.trace: argument 1 is not a vector (vec3) (expected ray start position)");
    }

    if (!LuaMathLibrary::LuaIsAngle(L, 2)) {
        return luaL_error(L, "game.trace: argument 2 is not an angle (vec3) (expected ray direction)");
    }

    if (!lua_isnumber(L, 3)) {
        return luaL_error(L, "game.trace: argument 3 is not a number (expected mask)");
    }

    if (lua_gettop(L) >= 4 && !lua_istable(L, 4) && !lua_isnil(L, 4)) {
        return luaL_error(L, "game.trace: argument 4 is not a table or nil (expected filter)");
    }

    Vector start = LuaMathLibrary::LuaGetVector3D(L, 1);
    QAngle angle = LuaMathLibrary::LuaGetAngle(L, 2);
    int mask = lua_tointeger(L, 3);
    int filter_raw[4];
    CTraceFilter* filter = reinterpret_cast<CTraceFilter*>(filter_raw);

    auto server_dll = (DWORD)GetModuleHandle("server.dll");
    //235D10
    typedef void* (__thiscall *CTraceFilterSimpleClassnameList_t)(void*, void*, int);
    auto CTraceFilterSimpleClassnameList = (CTraceFilterSimpleClassnameList_t)(server_dll + 0x235D10);
    CTraceFilterSimpleClassnameList(filter, nullptr, COLLISION_GROUP_NONE);

    //235D40
    typedef void (__thiscall *AddClassnameToIgnore_t)(void*, const char*);
    auto AddClassnameToIgnore = (AddClassnameToIgnore_t)(server_dll + 0x235D40);
    if (lua_istable(L, 4)) {
        lua_pushnil(L);
        while (lua_next(L, 4) != 0) {
            if (lua_isstring(L, -1)) {
                AddClassnameToIgnore(filter, lua_tostring(L, -1));
            }
            lua_pop(L, 1);
        }
    } else {
        AddClassnameToIgnore(filter, "prop_physics");
        AddClassnameToIgnore(filter, "func_physbox");
        AddClassnameToIgnore(filter, "npc_portal_turret_floor");
        AddClassnameToIgnore(filter, "prop_energy_ball");
        AddClassnameToIgnore(filter, "npc_security_camera");
        AddClassnameToIgnore(filter, "player");
        AddClassnameToIgnore(filter, "simple_physics_prop");
        AddClassnameToIgnore(filter, "simple_physics_brush");
        AddClassnameToIgnore(filter, "prop_ragdoll");
        AddClassnameToIgnore(filter, "prop_glados_core");
        AddClassnameToIgnore(filter, "prop_portal");
    }

    Vector forward;
    AngleVectors(angle, &forward);
    Vector end = start + forward * 8192.0f;

    Ray_t ray;
    ray.Init(start, end);

    trace_t tr;
    interfaces::engineTraceServer->TraceRay(
        ray,
        mask,
        filter,
        &tr
    );

    //    if(!tr.DidHit()) {
    //        lua_pushnil(L);
    //        return 1;
    //    }

    // Result is a table of
    // {endPos: vec3, normal: vec3, entityID: number}

    lua_newtable(L);
    LuaMathLibrary::LuaPushVector3D(L, tr.endpos);
    lua_setfield(L, -2, "endPos");
    LuaMathLibrary::LuaPushVector3D(L, tr.plane.normal);
    lua_setfield(L, -2, "normal");
    int entityIndex = tr.m_pEnt != nullptr ? utils::GetIndex(tr.m_pEnt) : -1;
    if (entityIndex == -1) {
        lua_pushnil(L);
    } else {
        lua_pushinteger(L, entityIndex);
    }
    lua_setfield(L, -2, "entityID");
    return 1;
}


static const luaL_Reg game_class[] = {
    {"is_paused", IsPaused},
    {"is_simulating", IsSimulating},
    {"get_engine_state", GetEngineState},
    {"is_tas_paused", IsTasPaused},
    {"get_client_tick", GetClientTick},
    {"get_server_tick", GetServerTick},
    {"get_directory", GetGameDirectory},
    {"is_playing_demo", IsPlayingDemo},
    {"trace", Trace},
    {nullptr, nullptr}
};

LuaGameLibrary::LuaGameLibrary() : LuaLibrary("game") {}

void LuaGameLibrary::Load(lua_State* L) {
    luaL_register(L, "game", game_class);
    lua_pop(L, 1);
}

const std::string& LuaGameLibrary::GetLuaSource() {
    static std::string sources = R"""(---@meta
---@class game
game = {}

---@param name string name of the save
function game.save(name)
    console.exec("save " .. name)
end

---@param name string name of the save
function game.load(name)
    console.exec("load " .. name)
end

---@param name string name of the demo
function game.start_recording(name)
    console.exec("record " .. name)
end

---Stop recording a demo
function game.stop_recording()
    console.exec("stop")
end

---Pause the game
function game.pause()
    console.exec("pause")
end

---Unpause the game
function game.unpause()
    console.exec("unpause")
end

---@return int state Current engine state of the game.
function game.get_engine_state()
end

---@return boolean is_paused Is the game paused.
--- Returns true if the game is paused, including being in the main menu, false otherwise.
function game.is_paused()
end

---@return boolean is_simulating Is the game simulating.
--- Returns true if the game is simulating, false otherwise. Importantly, this will return false when tas_pause is enabled.
function game.is_simulating()
end

---@return number The current client tick.
function game.get_client_tick()
end

---@return number The current server tick.
function game.get_server_tick()
end

---@return string The game directory.
function game.get_directory()
end

---@return bool Is the game playing a demo.
function game.is_playing_demo()
end

---@class trace
---@field endPos vec3 The end position of the trace.
---@field normal vec3 The normal of the trace.
---@field entityID number The entity ID of the trace or nil if no entity was hit.
local trace = {}

---@param pos vec3 The start position of the ray.
---@param angle vec3 The direction of the ray.
---@param mask number The mask to use for the trace.
---@param filter string[]|nil The list of entities to ignore.
---@return trace|nil The result of the trace.
function game.trace(pos, angle, mask, filter)
end

---@return thread|any The thread that is running the callback.
---Launches an asynchronous task that runs the callback in a new thread.
function game.async(callback)
    return coroutine.resume(coroutine.create(callback))
end
)""";

    return sources;
}
