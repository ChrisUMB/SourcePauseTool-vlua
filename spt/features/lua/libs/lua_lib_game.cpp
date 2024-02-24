#include "stdafx.hpp"
#include "lua_lib_game.hpp"
#include "interfaces.hpp"
#include "../../demo.hpp"
#include "spt/features/taspause.hpp"

LuaGameLibrary lua_game_library;

static int GetClientTick(lua_State* L)
{
	lua_pushinteger(L, interfaces::engine_tool->ClientTick());
	return 1;
}

static int GetServerTick(lua_State* L)
{
	lua_pushinteger(L, interfaces::engine_tool->ServerTick());
	return 1;
}

extern ConVar tas_pause;

static bool IsPaused()
{
	const auto engDLL = reinterpret_cast<DWORD>(GetModuleHandle("engine.dll"));
	const int server_state = *reinterpret_cast<int*>(engDLL + 0x53050C);
	const int client_state = *reinterpret_cast<int*>(engDLL + 0x3D1D80);
	const int host_state = *reinterpret_cast<int*>(engDLL + 0x3954C4);
	return server_state != 2 || client_state != 6 || host_state != 4;
}

static int IsPaused(lua_State* L)
{
	lua_pushboolean(L, IsPaused());
	return 1;
}

static int IsSimulating(lua_State* L)
{
	lua_pushboolean(L, !IsPaused() && spt_taspause.GetHostFrametime() != 0);
	return 1;
}

static int IsTasPaused(lua_State* L)
{
	lua_pushboolean(L, tas_pause.GetBool());
	return 1;
}

static int GetGameDirectory(lua_State* L)
{
	lua_pushstring(L, interfaces::engine->GetGameDirectory());
	return 1;
}

static int IsPlayingDemo(lua_State* L)
{
	lua_pushboolean(L, spt_demostuff.Demo_IsPlayingBack());
	return 1;
}

static int GetEngineState(lua_State* L)
{
	auto eng = reinterpret_cast<DWORD>(GetModuleHandle("engine.dll")) + 0x36E69C;
	eng = *(DWORD *)eng;

	typedef int (__thiscall *GetState_t)(void *);
	const auto GetState = reinterpret_cast<GetState_t>((*(int **)eng)[4]);
	lua_pushinteger(L, GetState((void*)eng));

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
	{nullptr, nullptr}};

LuaGameLibrary::LuaGameLibrary() : LuaLibrary("game")
{
}

void LuaGameLibrary::Load(lua_State* L)
{
	luaL_register(L, "game", game_class);
	lua_pop(L, 1);
}

const std::string& LuaGameLibrary::GetLuaSource()
{
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

---@return thread|any The thread that is running the callback.
---Launches an asynchronous task that runs the callback in a new thread.
function game.async(callback)
    return coroutine.resume(coroutine.create(callback))
end
)""";

	return sources;
}