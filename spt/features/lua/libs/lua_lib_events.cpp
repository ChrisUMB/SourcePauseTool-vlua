#include "stdafx.hpp"
#include "lua_lib_events.hpp"

LuaEventsLibrary lua_events_library;

LuaEventsLibrary::LuaEventsLibrary() : LuaLibrary("events") {}

void LuaEventsLibrary::InvokeEvent(const std::string &event_name, const std::function<void(lua_State*)> &lambda) {
    for (const auto &L: states) {
        lambda(L);
        lua_getglobal(L, "events");
        lua_getfield(L, -1, event_name.c_str());

        if (!lua_istable(L, -1)) {
            lua_pop(L, 2);
            Msg("event doesn't exist: %s\n", event_name.c_str());
            return;
        }

        lua_remove(L, -2);
        lua_getfield(L, -1, "call");
        lua_pushvalue(L, -2);
        lua_remove(L, -3);
        lua_pushvalue(L, -3);
        if (lua_pcall(L, 2, 0, 0)) {
            Warning("%s\n", lua_tostring(L, -1));
        }

        lua_pop(L, 1);
    }
}

void LuaEventsLibrary::Load(lua_State *L) {
    states.insert(L);
}

void LuaEventsLibrary::Unload(lua_State *L) {
    LuaLibrary::Unload(L);
    states.erase(L);
}

const std::string &LuaEventsLibrary::GetLuaSource() {
    static std::string sources = R"""(
---@class event

---@generic T : event
---@class event_type<T>
local event_type = {}
event_type.__index = event_type

function new_event_type()
    return setmetatable({}, event_type)
end

---@class events
events = {
    ---@class tick_event : event
    ---@field tick number The tick number
    ---@type event_type<tick_event>
    tick = new_event_type(),

    ---@class frame_event : event
    ---@field delta number The delta time since the last frame
    ---@type event_type<frame_event>
    frame = new_event_type(),

    ---@class level_init_event : event
    ---@field map_name string The name of the map
    ---@type event_type<level_init_event>
    level_init = new_event_type(),

    ---@class client_active_event : event
    ---@type event_type<client_active_event>
    client_active = new_event_type(),

    ---@class client_disconnect_event : event
    ---@type event_type<client_disconnect_event>
    client_disconnect = new_event_type(),

    ---@class player_jump_event : event
    ---@type event_type<player_jump_event>
    player_jump = new_event_type(),

    ---@class player_grounded_event : event
    ---@type event_type<player_grounded_event>
    player_grounded = new_event_type(),
}

local listeners = {}

---@generic T : event
---@param self event_type<T>
---@param event T The event to pass to the listeners
function event_type.call(self, event)
    if not event then
        event = {}
    end

    local event_listeners = listeners[self]
    if not event_listeners then
        return
    end

    listeners[self] = {}

    for _, l in pairs(event_listeners) do
        local s, e = pcall(function()
            l.callback(event, l)
        end)

        if not s then
            console.warning("Error in event listener: %s", e)
        end
    end
end

---@generic T : event
---@param self event_type<T>
---@param callback fun(event:T) The callback to call when the event is fired
function event_type.next(self, callback)
    local others = listeners[self] or {}
    local result = {
        event = self,
        callback = callback
    }
    table.insert(others, result)
    listeners[self] = others
end

---@generic T : event
---@param self event_type<T>
---@param count number|nil The number of events to wait for
---@return T
function event_type.wait(self, count)
    local co = coroutine.running()
    self:next(function(e)
        if co ~= nil then
            coroutine.resume(co, e)
        end
    end)

    local result = coroutine.yield()
    if count and count > 1 then
        return self:wait(count - 1)
    else
        return result
    end
end

---@generic T : event
---@param self event_type<T>
---@param callback fun(event:T, cancel:fun()) The callback to call when the event is fired
---@return fun() Cancels the listener
function event_type.listen(self, callback)
    local cancelled = false
    local cancel = function()
        cancelled = true
    end

    local wrapped_callback
    wrapped_callback = function(e)
        if not cancelled then
            callback(e, cancel)
            self:next(wrapped_callback)
        end
    end

    self:next(wrapped_callback)

    return cancel
end
)""";

    return sources;
}
