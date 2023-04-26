#include "stdafx.hpp"
#include "lua_lib_player.hpp"
#include "lua_lib_math.hpp"
#include "../../playerio.hpp"
#include "spt/sptlib-wrapper.hpp"
#include "../libs/lua_lib_entity.hpp"
#include "spt/features/generic.hpp"

LuaPlayerLibrary lua_player_library;

LuaPlayerLibrary::LuaPlayerLibrary() : LuaLibrary("player") {}

static void PlayerTeleport(const Vector *pos, const QAngle *ang, const Vector *vel) {
    LuaEntityLibrary::Teleport(spt_entprops.GetPlayer(true), pos, ang, vel);
}

static int PlayerGetPos(lua_State *L) {
    LuaMathLibrary::LuaPushVector3D(L, spt_playerio.m_vecAbsOrigin.GetValue());
    return 1;
}

static int PlayerSetPos(lua_State *L) {
    if (LuaMathLibrary::LuaIsVector3D(L, 1)) {
        const Vector &pos = LuaMathLibrary::LuaGetVector3D(L, 1);
        PlayerTeleport(&pos, nullptr, nullptr);
    } else {
        luaL_error(L, "player.set_pos: argument is not a vector");
    }

    return 0;
}

static int PlayerGetAng(lua_State *L) {
    QAngle ang;
    EngineGetViewAngles(reinterpret_cast<float *>(&ang));
    LuaMathLibrary::LuaPushAngle(L, ang);
    return 1;
}

static int PlayerSetAng(lua_State *L) {
    if (LuaMathLibrary::LuaIsAngle(L, 1)) {
        QAngle ang = LuaMathLibrary::LuaGetAngle(L, 1);
        EngineSetViewAngles(reinterpret_cast<float *>(&ang));
    } else {
        luaL_error(L, "player.set_ang: argument is not a vector");
    }

    return 0;
}

static int PlayerGetVel(lua_State *L) {
    LuaMathLibrary::LuaPushVector3D(L, spt_playerio.m_vecAbsVelocity.GetValue());
    return 1;
}

static int PlayerSetVel(lua_State *L) {
    if (LuaMathLibrary::LuaIsVector3D(L, 1)) {
        const Vector &vel = LuaMathLibrary::LuaGetVector3D(L, 1);
        PlayerTeleport(nullptr, nullptr, &vel);
    } else {
        luaL_error(L, "player.set_vel: argument is not a vector");
    }

    return 0;
}

static int PlayerGetEyePos(lua_State *L) {
    LuaMathLibrary::LuaPushVector3D(L, spt_generic.GetCameraOrigin());
    return 1;
}

static int PlayerTeleport(lua_State *L) {
    Vector pos;
    Vector *p_pos = &pos;
    QAngle ang;
    QAngle *p_ang = &ang;
    Vector vel;
    Vector *p_vel = &vel;
    if (LuaMathLibrary::LuaIsVector3D(L, 1)) {
        pos = LuaMathLibrary::LuaGetVector3D(L, 1);
    } else {
        p_pos = nullptr;
    }

    if (LuaMathLibrary::LuaIsAngle(L, 2)) {
        ang = LuaMathLibrary::LuaGetAngle(L, 2);
    } else {
        p_ang = nullptr;
    }

    if (LuaMathLibrary::LuaIsVector3D(L, 3)) {
        vel = LuaMathLibrary::LuaGetVector3D(L, 3);
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
        {"set_pos",     PlayerSetPos},
        {"get_ang",     PlayerGetAng},
        {"_set_ang",    PlayerSetAng},
        {"get_vel",     PlayerGetVel},
        {"set_vel",     PlayerSetVel},
        {"get_eye_pos", PlayerGetEyePos},

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

---@param pos vec3 Position vector
function player.set_pos(pos)
end

---@return vec3 player viewing angle
function player.get_ang()
end

---@param ang vec3|vec2 Angle vector, `vec2` will use the current `z` value
function player.set_ang(ang)
    if getmetatable(ang) == vec2 then
        player._set_ang(vec3(ang.x, ang.y, player.get_ang().z))
    else
        player._set_ang(ang)
    end
end

---@return vec3 player velocity
function player.get_vel()
    return nil
end

---@vararg vec3 Velocity vector
function player.set_vel(vel)
end

---@return vec3 player eye position
function player.get_eye_pos()
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
