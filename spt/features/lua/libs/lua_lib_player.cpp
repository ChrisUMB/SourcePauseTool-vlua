#include "stdafx.hpp"
#include "lua_lib_player.hpp"
#include "lua_lib_math.hpp"
#include "../../playerio.hpp"
#include "spt/sptlib-wrapper.hpp"
#include "../libs/lua_lib_entity.hpp"
#include "spt/features/generic.hpp"
#include "interfaces.hpp"
#include "signals.hpp"

LuaPlayerLibrary lua_player_library;

// Fix angles to between -180 and 180 degrees
#define FIX_ANGLE(x) ((x) > 180 ? (x)-360 : (x) < -180 ? (x) + 360 : (x))

static void ResetLocals(edict_t *) {
    lua_player_library.local_angle_offset = QAngle(0.0f, 0.0f, 0.0f);
    lua_player_library.local_position_origin = Vector(0.0f, 0.0f, 0.0f);
    lua_player_library.local_position_offset = Vector(0.0f, 0.0f, 0.0f);
}

LuaPlayerLibrary::LuaPlayerLibrary() : LuaLibrary("player") {
    this->local_angle_offset = QAngle(0.0f, 0.0f, 0.0f);
    this->local_position_origin = Vector(0.0f, 0.0f, 0.0f);
    this->local_position_offset = Vector(0.0f, 0.0f, 0.0f);
}

static void PlayerTeleport(const Vector *pos, const QAngle *ang, const Vector *vel) {
    LuaEntityLibrary::Teleport(spt_entprops.GetPlayer(true), pos, ang, vel);
}

static int PlayerGetPos(lua_State *L) {
    LuaMathLibrary::LuaPushVector3D(L, spt_playerio.m_vecAbsOrigin.GetValue());
    return 1;
}

static int PlayerSetPos(lua_State *L) {
    if (!LuaMathLibrary::LuaIsVector3D(L, 1)) {
        return luaL_error(L, "player.set_pos: argument is not a vector");
    }

    const Vector &pos = LuaMathLibrary::LuaGetVector3D(L, 1);
    PlayerTeleport(&pos, nullptr, nullptr);
    return 0;
}

static int PlayerGetAng(lua_State *L) {
    QAngle ang;
    EngineGetViewAngles(reinterpret_cast<float *>(&ang));
    LuaMathLibrary::LuaPushAngle(L, ang);
    return 1;
}

static int PlayerSetAng(lua_State *L) {
    if (!LuaMathLibrary::LuaIsAngle(L, 1)) {
        return luaL_error(L, "player.set_ang: argument is not a vector");
    }

    QAngle ang = LuaMathLibrary::LuaGetAngle(L, 1);
    EngineSetViewAngles(reinterpret_cast<float *>(&ang));
    return 0;
}

static int PlayerGetVel(lua_State *L) {
    LuaMathLibrary::LuaPushVector3D(L, spt_playerio.m_vecAbsVelocity.GetValue());
    return 1;
}

static int PlayerSetVel(lua_State *L) {
    if (!LuaMathLibrary::LuaIsVector3D(L, 1)) {
        return luaL_error(L, "player.set_vel: argument is not a vector");
    }

    const Vector &vel = LuaMathLibrary::LuaGetVector3D(L, 1);
    PlayerTeleport(nullptr, nullptr, &vel);
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

static int PlayerGetLocalAng(lua_State *L) {
    QAngle ang;
    EngineGetViewAngles(reinterpret_cast<float *>(&ang));

    ang.x = FIX_ANGLE(ang.x - lua_player_library.local_angle_offset.x);
    ang.y = FIX_ANGLE(ang.y - lua_player_library.local_angle_offset.y);
    ang.z = FIX_ANGLE(ang.z - lua_player_library.local_angle_offset.z);

    LuaMathLibrary::LuaPushAngle(L, ang);
    return 1;
}

static int PlayerSetLocalAng(lua_State *L) {
    if (!LuaMathLibrary::LuaIsAngle(L, 1)) {
        return luaL_error(L, "player.set_local_ang: argument is not a vector");
    }

    QAngle ang = LuaMathLibrary::LuaGetAngle(L, 1);
    ang.x = FIX_ANGLE(ang.x + lua_player_library.local_angle_offset.x);
    ang.y = FIX_ANGLE(ang.y + lua_player_library.local_angle_offset.y);
    ang.z = FIX_ANGLE(ang.z + lua_player_library.local_angle_offset.z);
    EngineSetViewAngles(reinterpret_cast<float *>(&ang));
    return 0;
}

static int PlayerGetLocalAngOffset(lua_State *L) {
    LuaMathLibrary::LuaPushAngle(L, lua_player_library.local_angle_offset);
    return 1;
}

static int PlayerGetLocalPos(lua_State *L) {
    LuaMathLibrary::LuaPushVector3D(L, lua_player_library.AsLocalPosition(spt_playerio.m_vecAbsOrigin.GetValue()));
    return 1;
}

static int PlayerGetLocalPosOffset(lua_State *L) {
    LuaMathLibrary::LuaPushVector3D(L, lua_player_library.local_position_offset);
    return 1;
}

static int PlayerGetLocalPosOrigin(lua_State *L) {
    LuaMathLibrary::LuaPushVector3D(L, lua_player_library.local_position_origin);
    return 1;
}

//static int PlayerTrace(lua_State *L) {
//
//}

static const struct luaL_Reg player_class[] = {
        {"get_pos",              PlayerGetPos},
        {"set_pos",              PlayerSetPos},
        {"get_ang",              PlayerGetAng},
        {"_set_ang",             PlayerSetAng},
        {"get_vel",              PlayerGetVel},
        {"set_vel",              PlayerSetVel},
        {"get_eye_pos",          PlayerGetEyePos},

        {"get_local_ang",        PlayerGetLocalAng},
        {"_set_local_ang",       PlayerSetLocalAng},
        {"get_local_ang_offset", PlayerGetLocalAngOffset},
        {"get_local_pos",        PlayerGetLocalPos},
        {"get_local_pos_offset", PlayerGetLocalPosOffset},
        {"get_local_pos_origin", PlayerGetLocalPosOrigin},
        {"teleport",             PlayerTeleport},
        {"is_grounded",          PlayerIsGrounded},
        //        {"trace",       PlayerTrace},
        {nullptr,                nullptr}
};

void LuaPlayerLibrary::Load(lua_State *L) {
    static bool first_load = true;
    if (first_load) {
        ClientActiveSignal.Connect(ResetLocals);
        first_load = false;
    }

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

---@return vec3
function player.get_local_ang()
    return nil
end

---@param ang vec3
function player.set_local_ang(ang)
    if getmetatable(ang) == vec2 then
        player._set_local_ang(vec3(ang.x, ang.y, player.get_ang().z))
    else
        player._set_local_ang(ang)
    end
end

---@return vec3
function player.get_local_ang_offset()
    return nil
end

---@return vec3
function player.get_local_pos()
    return nil
end

---@return vec3
function player.get_local_pos_offset()
    return nil
end

---@return vec3
function player.get_local_pos_origin()
    return nil
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

Vector LuaPlayerLibrary::AsLocalPosition(const Vector &position) {
    Vector forward, right, up;
    AngleVectors(local_angle_offset, &forward, &right, &up);

    Vector origin_pos = position - local_position_origin;
    Vector local_pos = (forward * origin_pos.x) + (right * origin_pos.y) + (up * origin_pos.z);
    return local_pos + local_position_offset;
}

void LuaPlayerLibrary::UpdateLocals(const Vector &old_pos,
                                    const QAngle &old_ang,
                                    const Vector &new_pos,
                                    const QAngle &new_ang) {
    Vector new_offset = AsLocalPosition(old_pos);
    local_position_origin = new_pos;
    local_position_offset = new_offset;

    local_angle_offset.x = FIX_ANGLE(new_ang.x - old_ang.x + local_angle_offset.x);
    local_angle_offset.y = FIX_ANGLE(new_ang.y - old_ang.y + local_angle_offset.y);
    local_angle_offset.z = FIX_ANGLE(new_ang.z - old_ang.z + local_angle_offset.z);
}
