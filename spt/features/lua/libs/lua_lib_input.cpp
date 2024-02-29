#include "stdafx.hpp"
#include "lua_lib_input.hpp"

LuaInputLibrary lua_input_library;

LuaInputLibrary::LuaInputLibrary() : LuaLibrary("input") {}

void LuaInputLibrary::Load(lua_State* L) {}

const std::string& LuaInputLibrary::GetLuaSource() {
    static std::string sources = R"""(---@meta
---@class input_key
---@field cmd string The name of the command for the input
local input_key = {}
input_key.__index = input_key

--- Release the input key.
function input_key:release()
    console.exec("-" .. self.cmd)
end

--- Hold the input key down until it is released.
---@param ticks number|nil The number of ticks to hold the key down for. If nil, the key will be held down until it is released.
function input_key:hold(ticks)
    console.exec("+" .. self.cmd)

    if ticks then
        game.async(function()
            events.sim_tick:wait(ticks)
            self:release()
        end)
    end
end

--- Hold the input key down for one tick.
function input_key:tap()
    self:hold()

    game.async(function()
        events.sim_tick:wait()
        self:release()
    end)
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
    forward = make_input("forward"),
    back = make_input("back"),
    moveleft = make_input("moveleft"),
    moveright = make_input("moveright"),
    moveup = make_input("moveup"),
    movedown = make_input("movedown"),
    left = make_input("left"),
    right = make_input("right"),
    lookup = make_input("lookup"),
    lookdown = make_input("lookdown"),
    jump = make_input("jump"),
    duck = make_input("duck"),
    attack = make_input("attack"),
    attack2 = make_input("attack2"),
    strafe = make_input("strafe"),
    speed = make_input("speed"),
    walk = make_input("walk"),
    use = make_input("use"),
    klook = make_input("klook"),
    jlook = make_input("jlook"),
    reload = make_input("reload"),
    alt1 = make_input("alt1"),
    alt2 = make_input("alt2"),
    score = make_input("score"),
    showscores = make_input("showscores"),
    graph = make_input("graph"),
    zoom = make_input("zoom"),
    grenade1 = make_input("grenade1"),
    grenade2 = make_input("grenade2")
}

--- Reset all input keys to released.
function input.reset()
    for _, key in pairs(input) do
        if type(key) == "table" then
            key:release()
        end
    end
end

--- Creates an `input_key` with the given name. The name is also used as the command for the input.
---@param name string The name of the input key
---@return input_key The created input key
function input.create(name)
    local key = make_input(name)
    return key
end
)""";

    return sources;
}
