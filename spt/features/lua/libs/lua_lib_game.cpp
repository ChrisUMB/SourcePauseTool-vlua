#include "stdafx.hpp"
#include "lua_lib_game.hpp"
#include "interfaces.hpp"

LuaGameLibrary lua_game_library;

static int GetClientTick(lua_State *L) {
    lua_pushinteger(L, interfaces::engine_tool->ClientTick());
    return 1;
}

static int GetServerTick(lua_State *L) {
    lua_pushinteger(L, interfaces::engine_tool->ServerTick());
    return 1;
}

//static int IsGamePaused(lua_State *L) {
//
//
//
//    lua_pushboolean(L, *((int *) (modules::engine.base + 0x53050C)) !=
//                       2); // checking if it's anything aside from active, consider it paused.
//    return 1;
//}

static int GetGameDirectory(lua_State *L) {
    lua_pushstring(L, interfaces::engine->GetGameDirectory());
    return 1;
}

static const struct luaL_Reg game_class[] = {
//        {"is_paused", IsGamePaused},
        {"get_client_tick", GetClientTick},
        {"get_server_tick", GetServerTick},
        {"get_directory", GetGameDirectory},
        {nullptr, nullptr}
};

LuaGameLibrary::LuaGameLibrary() : LuaLibrary("game") {}

void LuaGameLibrary::Load(lua_State *L) {
    luaL_register(L, "game", game_class);
    lua_pop(L, 1);
}

const std::string &LuaGameLibrary::GetLuaSource() {
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

---@return boolean Is the game paused.
function game.is_paused()
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

---@return thread|any The thread that is running the callback.
---Launches an asynchronous task that runs the callback in a new thread.
function game.async(callback)
    return coroutine.resume(coroutine.create(callback))
end
)""";

    return sources;
}
