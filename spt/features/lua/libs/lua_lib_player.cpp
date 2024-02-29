#include "stdafx.hpp"
#include "lua_lib_player.hpp"
#include "lua_lib_math.hpp"
#include "../../playerio.hpp"
#include "spt/sptlib-wrapper.hpp"
#include "../libs/lua_lib_entity.hpp"
#include "spt/features/generic.hpp"
#include "interfaces.hpp"
#include "signals.hpp"
#include "portal_utils.hpp"

LuaPlayerLibrary lua_player_library;

static inline float FixAngle(float x) {
    return x > 180 ? x - 360 : x < -180 ? x + 360 : x;
}

static void ResetLocals(edict_t*) {
    lua_player_library.local_angle_offset = QAngle(0.0f, 0.0f, 0.0f);
    lua_player_library.local_position_origin = Vector(0.0f, 0.0f, 0.0f);
    lua_player_library.local_position_offset = Vector(0.0f, 0.0f, 0.0f);
}

LuaPlayerLibrary::LuaPlayerLibrary() : LuaLibrary("player") {
    this->local_angle_offset = QAngle(0.0f, 0.0f, 0.0f);
    this->local_position_origin = Vector(0.0f, 0.0f, 0.0f);
    this->local_position_offset = Vector(0.0f, 0.0f, 0.0f);
}

static void PlayerTeleport(const Vector* pos, const QAngle* ang, const Vector* vel) {
    LuaEntityLibrary::Teleport(spt_entprops.GetPlayer(true), pos, ang, vel);
}

static int PlayerGetPos(lua_State* L) {
    LuaMathLibrary::LuaPushVector3D(L, spt_playerio.m_vecAbsOrigin.GetValue());
    return 1;
}

static int PlayerSetPos(lua_State* L) {
    if (!LuaMathLibrary::LuaIsVector3D(L, 1)) {
        return luaL_error(L, "player.set_pos: argument is not a vector");
    }

    const Vector& pos = LuaMathLibrary::LuaGetVector3D(L, 1);
    PlayerTeleport(&pos, nullptr, nullptr);
    return 0;
}

static int PlayerGetAng(lua_State* L) {
    QAngle ang;
    EngineGetViewAngles(&ang.x);
    LuaMathLibrary::LuaPushAngle(L, ang);
    return 1;
}

static int PlayerSetAng(lua_State* L) {
    if (!LuaMathLibrary::LuaIsAngle(L, 1)) {
        return luaL_error(L, "player.set_ang: argument is not a vector");
    }

    QAngle ang = LuaMathLibrary::LuaGetAngle(L, 1);
    EngineSetViewAngles(&ang.x);
    return 0;
}

static int PlayerGetVel(lua_State* L) {
    LuaMathLibrary::LuaPushVector3D(L, spt_playerio.m_vecAbsVelocity.GetValue());
    return 1;
}

static int PlayerSetVel(lua_State* L) {
    if (!LuaMathLibrary::LuaIsVector3D(L, 1)) {
        return luaL_error(L, "player.set_vel: argument is not a vector");
    }

    const Vector& vel = LuaMathLibrary::LuaGetVector3D(L, 1);
    PlayerTeleport(nullptr, nullptr, &vel);
    return 0;
}

static int PlayerGetEyePos(lua_State* L) {
    LuaMathLibrary::LuaPushVector3D(L, spt_generic.GetCameraOrigin());
    return 1;
}

static int PlayerTeleport(lua_State* L) {
    Vector pos;
    QAngle ang;
    Vector vel;

    Vector* p_pos = nullptr;
    QAngle* p_ang = nullptr;
    Vector* p_vel = nullptr;

    if (LuaMathLibrary::LuaIsVector3D(L, 1)) {
        pos = LuaMathLibrary::LuaGetVector3D(L, 1);
        p_pos = &pos;
    }

    if (LuaMathLibrary::LuaIsAngle(L, 2)) {
        ang = LuaMathLibrary::LuaGetAngle(L, 2);
        p_ang = &ang;
    }

    if (LuaMathLibrary::LuaIsVector3D(L, 3)) {
        vel = LuaMathLibrary::LuaGetVector3D(L, 3);
        p_vel = &vel;
    }

    PlayerTeleport(p_pos, p_ang, p_vel);
    return 0;
}

static int PlayerIsGrounded(lua_State* L) {
    lua_pushboolean(L, spt_playerio.IsGroundEntitySet());
    return 1;
}

static int PlayerGetLocalAng(lua_State* L) {
    QAngle ang;
    EngineGetViewAngles(&ang.x);

    ang.x = FixAngle(ang.x - lua_player_library.local_angle_offset.x);
    ang.y = FixAngle(ang.y - lua_player_library.local_angle_offset.y);
    ang.z = FixAngle(ang.z - lua_player_library.local_angle_offset.z);

    LuaMathLibrary::LuaPushAngle(L, ang);
    return 1;
}

static int PlayerSetLocalAng(lua_State* L) {
    if (!LuaMathLibrary::LuaIsAngle(L, 1)) {
        return luaL_error(L, "player.set_local_ang: argument is not a vector");
    }

    QAngle ang = LuaMathLibrary::LuaGetAngle(L, 1);
    ang.x = FixAngle(ang.x + lua_player_library.local_angle_offset.x);
    ang.y = FixAngle(ang.y + lua_player_library.local_angle_offset.y);
    ang.z = FixAngle(ang.z + lua_player_library.local_angle_offset.z);
    EngineSetViewAngles(&ang.x);
    return 0;
}

static int PlayerGetLocalAngOffset(lua_State* L) {
    LuaMathLibrary::LuaPushAngle(L, lua_player_library.local_angle_offset);
    return 1;
}

static int PlayerGetLocalPos(lua_State* L) {
    LuaMathLibrary::LuaPushVector3D(L, lua_player_library.AsLocalPosition(spt_playerio.m_vecAbsOrigin.GetValue()));
    return 1;
}

static int PlayerGetLocalPosOffset(lua_State* L) {
    LuaMathLibrary::LuaPushVector3D(L, lua_player_library.local_position_offset);
    return 1;
}

static int PlayerGetLocalPosOrigin(lua_State* L) {
    LuaMathLibrary::LuaPushVector3D(L, lua_player_library.local_position_origin);
    return 1;
}

static int PlayerGetSGPos(lua_State* L) {
    Vector pos;
    QAngle ang;
    calculateSGPosition(pos, ang);
    LuaMathLibrary::LuaPushVector3D(L, pos);
    return 1;
}

static int PlayerGetSGAng(lua_State* L) {
    Vector pos;
    QAngle ang;
    calculateSGPosition(pos, ang);
    LuaMathLibrary::LuaPushAngle(L, ang);
    return 1;
}

static int PlayerTrace(lua_State* L) {
    QAngle eyeAngles;
    EngineGetViewAngles(reinterpret_cast<float*>(&eyeAngles));

    const auto cl_player = spt_entprops.GetPlayer(false);

    if (cl_player == nullptr) {
        lua_pushnil(L);
        return 1;
    }

    //TODO: Patterns or something.
    const float eye_pos_x = *(static_cast<float*>(cl_player) + 1334);
    const float eye_pos_y = *(static_cast<float*>(cl_player) + 1335);
    const float eye_pos_z = *(static_cast<float*>(cl_player) + 1336);

    const Vector eyePosition(eye_pos_x, eye_pos_y, eye_pos_z);

    Vector forward;
    AngleVectors(eyeAngles, &forward);

    Vector start = eyePosition; // + forward * 24.0f;
    Vector end = start + forward * 8192.0f;
    Ray_t ray;
    ray.Init(start, end);

    auto server_dll = (DWORD)GetModuleHandle("server.dll");
    int data[4];
    //235D10
    typedef void* (__thiscall *CTraceFilterSimpleClassnameList_t)(void*, void*, int);
    auto CTraceFilterSimpleClassnameList = (CTraceFilterSimpleClassnameList_t)(server_dll + 0x235D10);
    CTraceFilterSimpleClassnameList(data, nullptr, COLLISION_GROUP_NONE);

    //235D40
    typedef void (__thiscall *AddClassnameToIgnore_t)(void*, const char*);
    auto AddClassnameToIgnore = (AddClassnameToIgnore_t)(server_dll + 0x235D40);
    AddClassnameToIgnore(data, "prop_physics");
    AddClassnameToIgnore(data, "func_physbox");
    AddClassnameToIgnore(data, "npc_portal_turret_floor");
    AddClassnameToIgnore(data, "prop_energy_ball");
    AddClassnameToIgnore(data, "npc_security_camera");
    AddClassnameToIgnore(data, "player");
    AddClassnameToIgnore(data, "simple_physics_prop");
    AddClassnameToIgnore(data, "simple_physics_brush");
    AddClassnameToIgnore(data, "prop_ragdoll");
    AddClassnameToIgnore(data, "prop_glados_core");
    AddClassnameToIgnore(data, "prop_portal");

    //    int clonesFilter[100];
    //    clonesFilter[0] = 0x5E91A8;
    //    clonesFilter[1] = reinterpret_cast<int>(data);

    trace_t tr;
    interfaces::engineTraceServer->TraceRay(ray, MASK_SHOT_PORTAL, reinterpret_cast<ITraceFilter*>(data), &tr);
    //    interfaces::engineTraceClient->TraceRay(ray, MASK_SHOT_PORTAL, nullptr, &tr);
    //    interfaces::engineTraceClient->TraceRay(ray, (CONTENTS_SOLID|CONTENTS_MOVEABLE|CONTENTS_WINDOW), nullptr, &tr);

    LuaMathLibrary::LuaPushVector3D(L, tr.endpos);
    return 1;
}

static const struct luaL_Reg player_class[] = {
    {"get_pos", PlayerGetPos},
    {"set_pos", PlayerSetPos},
    {"get_ang", PlayerGetAng},
    {"_set_ang", PlayerSetAng},
    {"get_vel", PlayerGetVel},
    {"set_vel", PlayerSetVel},
    {"get_eye_pos", PlayerGetEyePos},
    {"get_sg_pos", PlayerGetSGPos},
    {"get_sg_ang", PlayerGetSGAng},

    {"get_local_ang", PlayerGetLocalAng},
    {"_set_local_ang", PlayerSetLocalAng},
    {"get_local_ang_offset", PlayerGetLocalAngOffset},
    {"get_local_pos", PlayerGetLocalPos},
    {"get_local_pos_offset", PlayerGetLocalPosOffset},
    {"get_local_pos_origin", PlayerGetLocalPosOrigin},
    {"teleport", PlayerTeleport},
    {"is_grounded", PlayerIsGrounded},
    {"trace", PlayerTrace},
    {nullptr, nullptr}
};

void LuaPlayerLibrary::Load(lua_State* L) {
    static bool first_load = true;
    if (first_load) {
        ClientActiveSignal.Connect(ResetLocals);
        first_load = false;
    }

    luaL_register(L, "player", player_class);
    lua_pop(L, 1);
}

const std::string& LuaPlayerLibrary::GetLuaSource() {
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

Vector LuaPlayerLibrary::AsLocalPosition(const Vector& position) {
    Vector forward, right, up;
    AngleVectors(local_angle_offset, &forward, &right, &up);

    Vector origin_pos = position - local_position_origin;
    Vector local_pos = (forward * origin_pos.x) + (right * origin_pos.y) + (up * origin_pos.z);
    return local_pos + local_position_offset;
}

void LuaPlayerLibrary::UpdateLocals(const Vector& old_pos,
                                    const QAngle& old_ang,
                                    const Vector& new_pos,
                                    const QAngle& new_ang) {
    Vector new_offset = AsLocalPosition(old_pos);
    local_position_origin = new_pos;
    local_position_offset = new_offset;

    local_angle_offset.x = FixAngle(new_ang.x - old_ang.x + local_angle_offset.x);
    local_angle_offset.y = FixAngle(new_ang.y - old_ang.y + local_angle_offset.y);
    local_angle_offset.z = FixAngle(new_ang.z - old_ang.z + local_angle_offset.z);
}
