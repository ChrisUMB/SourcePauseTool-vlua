#include "stdafx.hpp"
#include "lua_lib_player.hpp"
#include "lua_lib_math.hpp"
#include "../../playerio.hpp"
#include "../../ent_props.hpp"
#include "spt/sptlib-wrapper.hpp"

LuaPlayerLibrary lua_player_library;

LuaPlayerLibrary::LuaPlayerLibrary() : LuaLibrary("player") {}

static void PlayerTeleport(const Vector *pos, const QAngle *ang, const Vector * vel) {
    void *player = spt_entprops.GetPlayer(true);
    int *p_vtable = reinterpret_cast<int *>(player);
    (*(void (__thiscall **)(void *, const Vector *, const QAngle *, const Vector *)) (*p_vtable + 420))(player, pos, ang, vel);
}

static int PlayerGetPos(lua_State *L) {
    lua_math_library.LuaPushVector3D(L, spt_playerio.m_vecAbsOrigin.GetValue());
    return 1;
}

static int PlayerSetPos(lua_State *L) {
    if (lua_math_library.LuaIsVector3D(L, 1)) {
        const Vector &pos = lua_math_library.LuaGetVector3D(L, 1);
        PlayerTeleport(&pos, nullptr, nullptr);
    } else {
        luaL_error(L, "player.set_pos: argument is not a vector");
    }

    return 0;
}

static int PlayerGetAng(lua_State *L) {
    QAngle ang;
    EngineGetViewAngles(reinterpret_cast<float*>(&ang));
    lua_math_library.LuaPushAngle(L, ang);
    return 1;
}

static int PlayerSetAng(lua_State *L) {
    if (lua_math_library.LuaIsAngle(L, 1)) {
        QAngle ang = lua_math_library.LuaGetAngle(L, 1);
        EngineSetViewAngles(reinterpret_cast<float*>(&ang));
    } else {
        luaL_error(L, "player.set_ang: argument is not a vector");
    }

    return 0;
}

static int PlayerGetVel(lua_State *L) {
    lua_math_library.LuaPushVector3D(L, spt_playerio.m_vecAbsVelocity.GetValue());
    return 1;
}

static int PlayerSetVel(lua_State *L) {
    if (lua_math_library.LuaIsVector3D(L, 1)) {
        const Vector &vel = lua_math_library.LuaGetVector3D(L, 1);
        PlayerTeleport(nullptr, nullptr, &vel);
    } else {
        luaL_error(L, "player.set_vel: argument is not a vector");
    }

    return 0;
}

static int PlayerTeleport(lua_State *L) {
    Vector pos;
    Vector *p_pos = &pos;
    QAngle ang;
    QAngle *p_ang = &ang;
    Vector vel;
    Vector *p_vel = &vel;
    if(lua_math_library.LuaIsVector3D(L, 1)) {
        pos = lua_math_library.LuaGetVector3D(L, 1);
    } else {
        p_pos = nullptr;
    }


    if(lua_math_library.LuaIsVector3D(L, 2)) {
        ang = lua_math_library.LuaGetAngle(L, 2);
    } else {
       p_ang = nullptr;
    };

    if(lua_math_library.LuaIsVector3D(L, 3)) {
        vel = lua_math_library.LuaGetVector3D(L, 3);
    } else {
        p_vel = nullptr;
    }

    PlayerTeleport(p_pos, p_ang, p_vel);

    return 0;
}

static int PlayerIsGrounded(lua_State *L) {
    lua_pushboolean(L, spt_playerio.IsGroundEntitySet());
    return 1;
}

//static int PlayerTrace(lua_State *L) {
//
//}

static const struct luaL_Reg player_class[] = {
        {"get_pos",     PlayerGetPos},
        {"_set_pos",     PlayerSetPos},
        {"get_ang",     PlayerGetAng},
        {"_set_ang",     PlayerSetAng},
        {"get_vel",     PlayerGetVel},
        {"_set_vel",     PlayerSetVel}, // setters are done in lua so they can just take numbers/tables, too

//        {"get_eye_pos",          lua_player_get_eye_pos},
//        {"get_local_ang",        lua_player_get_local_ang},
//        {"set_local_ang",        lua_player_set_local_ang},
//        {"get_local_ang_offset", lua_player_get_local_ang_offset},
//        {"get_local_pos",        lua_player_get_local_pos},
//        {"get_local_pos_offset", lua_player_get_local_pos_offset},
        {"teleport",    PlayerTeleport},
        {"is_grounded", PlayerIsGrounded},
//        {"trace",       PlayerTrace},
        {nullptr,       nullptr}
};

void LuaPlayerLibrary::Load(lua_State *L) {
    luaL_register(L, "player", player_class);
    lua_pop(L, 1);
}

const std::string &LuaPlayerLibrary::GetLuaSource() {
    static std::string sources = R"""(
---@class player
player = {}

---@return vec3 player position
function player.get_pos()
    return nil
end

---@vararg vec3|number|table Position vector
function player.set_pos(...)
    player._set_pos(vec3(...))
end

---@return vec3 player viewing angle
function player.get_ang()
end

---@vararg vec3|vec2|number|table Angle vector
function player.set_ang(...)
    local args = {...}
    local t = getmetatable(args[1])
    if t == vec3 then
        player._set_ang(args[1])
    elseif t == vec2 then
        player._set_ang(vec3(args[1].x, args[1].y, 0))
    else
        local r = player.get_ang()

        if #args == 1 then
            r.x = args[1]
        elseif #args == 2 then
            r.x = args[1]
            r.y = args[2]
        elseif #args == 3 then
            r.x = args[1]
            r.y = args[2]
            r.z = args[3]
        end

        player._set_ang(r)
    end

end

---@return vec3 player velocity
function player.get_vel()
    return nil
end

---@vararg vec3|number|table Position vector
function player.set_vel(...)
    player._set_vel(vec3(...))
end

---@param pos vec3 player position
---@param ang vec3 player viewing angle
---@param vel vec3 player velocity
function player.teleport(pos, ang, vel)
end

---@return boolean is the player grounded.
function player.is_grounded()
    return false
end

---@return vec3 the traced result of the player's view
function player.trace()
    return nil
end
)""";

    return sources;
}
