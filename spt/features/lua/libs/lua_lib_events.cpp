#include "stdafx.hpp"
#include "lua_lib_events.hpp"

LuaEventsLibrary lua_events_library;

LuaEventsLibrary::LuaEventsLibrary() : LuaLibrary("events") {}

void LuaEventsLibrary::InvokeEvent(const std::string& event_name, const std::function<void(lua_State*)>& lambda) {
    for (const auto& L : states) {
        lambda(L);
        lua_getglobal(L, "events");
        lua_getfield(L, -1, event_name.c_str());

        if (!lua_istable(L, -1)) {
            lua_pop(L, 2);
            Warning("event doesn't exist: %s\n", event_name.c_str());
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

void LuaEventsLibrary::Load(lua_State* L) {
    states.push_back(L);
}

void LuaEventsLibrary::Unload(lua_State* L) {
    LuaLibrary::Unload(L);
    states.erase(std::remove(states.begin(), states.end(), L), states.end());
}

#define EVENT_TYPE_LUA(name) \
    "---@class " #name "_event_type : event_type<" #name "_event>\n" \
    "    ---@field wait fun(self, count:number|nil):" #name "_event\n" \
    "    ---@field listen fun(self, callback:fun(event:" #name "_event, cancel:fun())):fun()\n" \
    "    ---@field next fun(self, callback:fun(event:" #name "_event)):fun()"

const std::string& LuaEventsLibrary::GetLuaSource() {
    static std::string sources = R"(
---@meta
---@class event

---@generic T : event
---@class event_type<T>
---@field name string The name of the event
local event_type = {}
event_type.__index = event_type

local event_id = 0

function new_event_type()
    local id = event_id
    event_id = event_id + 1
    return setmetatable({id = id}, event_type)
end

---@class event_scope
local event_scope = {
    ---@class tick_event : event
    ---Any time the engine ticks, this will occur even in the main menu.
    )" EVENT_TYPE_LUA(tick) R"(
    tick = new_event_type(),

    ---@class sim_tick_event : event
    ---Any time the engine ticks and is simulating, this will only occur in-game and in a map, but not while paused (including tas_pause).
    )" EVENT_TYPE_LUA(sim_tick) R"(
    sim_tick = new_event_type(),

    ---@class render_event : event
    )" EVENT_TYPE_LUA(render) R"(
    render = new_event_type(),

    ---@class hud_event : event
    )" EVENT_TYPE_LUA(hud) R"(
    hud = new_event_type(),

    ---@class level_init_event : event
    ---@field map_name string The name of the map
    )" EVENT_TYPE_LUA(level_init) R"(
    level_init = new_event_type(),

    ---@class client_active_event : event
    )" EVENT_TYPE_LUA(client_active) R"(
    client_active = new_event_type(),

    ---@class client_disconnect_event : event
    )" EVENT_TYPE_LUA(client_disconnect) R"(
    client_disconnect = new_event_type(),

    ---@class player_jump_event : event
    )" EVENT_TYPE_LUA(player_jump) R"(
    player_jump = new_event_type(),

    ---@class entity_teleport_event : event
    ---@field entity entity The entity that was teleported
    ---@field old_pos vec3 The position of the entity prior to the teleport
    ---@field new_pos vec3 The position of the entity after the teleport
    ---@field old_rot vec3 The rotation of the entity prior to the teleport
    ---@field new_rot vec3 The rotation of the entity after the teleport
    ---@field old_ang vec3 The angles of the entity prior to the teleport
    ---@field new_ang vec3 The angles of the entity after the teleport
    )" EVENT_TYPE_LUA(entity_teleport) R"(
    entity_teleport = new_event_type(),

    ---@class player_teleport_event : entity_teleport_event
    ---@field entity entity The player that was teleported
    )" EVENT_TYPE_LUA(player_teleport) R"(
    player_teleport = new_event_type(),

    ---@class player_grounded_event : event
    )" EVENT_TYPE_LUA(player_grounded) R"(
    player_grounded = new_event_type(),

    ---@class player_ungrounded_event : event
    )" EVENT_TYPE_LUA(player_ungrounded) R"(
    player_ungrounded = new_event_type(),

    ---@class entity_touch_trigger_event : event
    ---@field trigger entity The trigger that was touched
    ---@field entity entity The entity that touched the trigger
    )" EVENT_TYPE_LUA(entity_touch_trigger) R"(
    entity_touch_trigger = new_event_type(),

    ---@class player_touch_trigger_event : entity_touch_trigger_event
    ---@field entity entity The player that touched the trigger
    )" EVENT_TYPE_LUA(player_touch_trigger) R"(
    player_touch_trigger = new_event_type(),

    ---@class portal_moved_event : event
    ---@field old_pos vec3 The position of the portal prior to the move
    ---@field new_pos vec3 The position of the portal after the move
    ---@field old_ang vec3 The angles of the portal prior to the move
    ---@field new_ang vec3 The angles of the portal after the move
    )" EVENT_TYPE_LUA(portal_moved) R"(
    portal_moved = new_event_type(),

    ---@class demo_start_event : event
    ---@field file_name string The name of the demo file
    ---@field demo_protocol number The protocol version of the demo
    ---@field network_protocol number The protocol version of the network
    ---@field playback_ticks number The number of ticks in the demo
    ---@field playback_frames number The number of frames in the demo
    ---@field playback_time number The time of the demo in seconds
    ---@field map_name string The name of the map
    )" EVENT_TYPE_LUA(demo_start) R"(
    demo_start = new_event_type(),

    ---@class demo_tick_event : event
    ---@field tick number The tick number
    )" EVENT_TYPE_LUA(demo_tick) R"(
    demo_tick = new_event_type(),

    ---@class demo_stop_event : event
    )" EVENT_TYPE_LUA(demo_stop) R"(
    demo_stop = new_event_type(),

    ---@class lua_reset_event : event
    )" EVENT_TYPE_LUA(lua_reset) R"(
    lua_reset = new_event_type(),
}

local scopes = {}

---@return event_scope
local function create_new_scope(name)
    local existing = scopes[name]
    if existing ~= nil then
        scopes[name] = nil
    end

    local scope = {name = name, listeners = {}, cache_listeners = {}}
    local meta = {
        __index = function(_, key)
            local scoped_event_type = {}
            scoped_event_type.scope = scope
            scoped_event_type.name = key
            setmetatable(scoped_event_type, {__index = event_scope[key]})
            return scoped_event_type
        end
    }
    scopes[name] = scope

    setmetatable(scope, meta)
    return scope
end

---@class events : event_scope
---@field scope fun(name:string):event_scope
events = create_new_scope("global")
events.scope = function(name)
    if name == "global" then
        return events
    end

    return create_new_scope(name)
end

--local listeners = {}

---@generic T : event
---@param self event_type<T>
---@param event T The event to pass to the listeners
function event_type.call(self, event)
    if not event then
        event = {}
    end

    if self.scope.name == "global" then
        for _, s in pairs(scopes) do
            s.cache_listeners[self.id] = s.listeners[self.id]
            s.listeners[self.id] = {}
        end
        for _, s in pairs(scopes) do
            if s.name ~= "global" then
                s[self.name]:call(event)
            end
        end
    end

    local event_listeners = self.scope.cache_listeners[self.id]
    if not event_listeners then
        return
    end

    self.scope.cache_listeners[self.id] = {}

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
    local others = self.scope.listeners[self.id] or {}
    local result = {
        event = self,
        callback = callback
    }
    table.insert(others, result)
    self.scope.listeners[self.id] = others
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

)";

    return sources;
}
