#include "stdafx.hpp"
#include "lua_lib_input.hpp"

LuaInputLibrary lua_input_library;

LuaInputLibrary::LuaInputLibrary() : LuaLibrary("input") {}

void LuaInputLibrary::Load(lua_State *L) {}

const std::string &LuaInputLibrary::GetLuaSource() {
    static std::string sources = R"""(
---@class input_key
---@field cmd string The name of the command for the input
local input_key = {}
input_key.__index = input_key

--- Release the input key.
function input_key:release()
    console.exec("-" .. self.cmd)
end

--- Hold the input key down until it is released.
function input_key:hold()
    console.exec("+" .. self.cmd)
end

--- Hold the input key down for one tick.
function input_key:tap()
    self:hold()

    coroutine.resume(coroutine.create(function()
        events.tick:wait()
        self:release()
    end))
end

---@return input_key
local function make_input(cmd)
    local input = {}
    input.cmd = cmd
    setmetatable(input, input_key)
    return input
end

---@class input : table<string, input_key>
input = {
    move_forward = make_input("forward"),
    move_back = make_input("back"),
    move_left = make_input("moveleft"),
    move_right = make_input("moveright"),
    move_up = make_input("moveup"),
    move_down = make_input("movedown"),
    look_left = make_input("left"),
    look_right = make_input("right"),
    look_up = make_input("lookup"),
    look_down = make_input("lookdown"),
    jump = make_input("jump"),
    duck = make_input("duck"),
    attack_primary = make_input("attack"),
    attack_secondary = make_input("attack2"),
    strafe = make_input("strafe"),
    speed = make_input("speed"),
    walk = make_input("walk"),
    use = make_input("use"),
    klook = make_input("klook"),
    jlook = make_input("jlook"),
    reload = make_input("reload"),
    alt_primary = make_input("alt1"),
    alt_secondary = make_input("alt2"),
    score = make_input("score"),
    show_scores = make_input("showscores"),
    graph = make_input("graph"),
    zoom = make_input("zoom"),
    grenade_primary = make_input("grenade1"),
    grenade_secondary = make_input("grenade2")
}

--- Reset all input keys to released.
function input.reset()
    for _, key in pairs(input) do
        if type(key) == "table" then
            key:release()
        end
    end
end

)""";

    return sources;
}
