#include "stdafx.hpp"
#include "../lua_util.hpp"
#include "lua_lib_camera.hpp"
#include "lua_lib_math.hpp"
#include "interfaces.hpp"

LuaCameraLibrary lua_camera_library;

LuaCameraLibrary::LuaCameraLibrary() : LuaLibrary("camera") {}

static int SetControl(lua_State* L) {
    if (lua_gettop(L) != 1) {
        return luaL_error(L, "camera.set_control: takes 1 argument");
    }

    if (!lua_isnumber(L, 1)) {
        return luaL_error(L, "camera.set_control: argument is not a number");
    }

    auto command = std::format("spt_cam_control {}", lua_tointeger(L, 1));
    interfaces::engine->ClientCmd(command.c_str());
    return 1;
}

static int SetPos(lua_State* L) {
    if (lua_gettop(L) != 1) {
        return luaL_error(L, "camera.set_pos: takes 1 argument");
    }

    if (!lua_istable(L, 1)) {
        return luaL_error(L, "camera.set_pos: argument is not a table");
    }

    if (!LuaMathLibrary::LuaIsVector3D(L, 1)) {
        return luaL_error(L, "camera.set_pos: argument is not a vector");
    }

    auto pos = LuaMathLibrary::LuaGetVector3D(L, 1);

    auto command = std::format("spt_cam_setpos {} {} {}", pos.x, pos.y, pos.z);
    interfaces::engine->ClientCmd(command.c_str());
    return 1;
}

static int SetAng(lua_State* L) {
    if (lua_gettop(L) != 1) {
        return luaL_error(L, "camera.set_ang: takes 1 argument");
    }

    if (!lua_istable(L, 1)) {
        return luaL_error(L, "camera.set_ang: argument is not a table");
    }

    if (!LuaMathLibrary::LuaIsAngle(L, 1)) {
        return luaL_error(L, "camera.set_ang: argument is not an angle");
    }

    auto ang = LuaMathLibrary::LuaGetAngle(L, 1);

    auto command = std::format("spt_cam_setang {} {} {}", ang.x, ang.y, ang.z);
    interfaces::engine->ClientCmd(command.c_str());
    return 1;
}

static int AddKeyframe(lua_State* L) {
    if (lua_gettop(L) < 3) {
        return luaL_error(L, "camera.add_keyframe: takes 4 arguments");
    }

    if (!lua_isnumber(L, 1)) {
        return luaL_error(L, "camera.add_keyframe: argument 1 is not a number (tick)");
    }

    auto tick = lua_tointeger(L, 1);

    if (!LuaMathLibrary::LuaIsVector3D(L, 2)) {
        return luaL_error(L, "camera.add_keyframe: argument 2 is not a vector (pos)");
    }

    auto pos = LuaMathLibrary::LuaGetVector3D(L, 2);

    if (!LuaMathLibrary::LuaIsAngle(L, 3)) {
        return luaL_error(L, "camera.add_keyframe: argument 3 is not an angle (ang)");
    }

    auto ang = LuaMathLibrary::LuaGetAngle(L, 3);

    if (!lua_isnumber(L, 4)) {
        return luaL_error(L, "camera.add_keyframe: argument 4 is not a number (fov)");
    }

    auto fov = lua_tonumber(L, 4);

    auto command = std::format("spt_cam_path_setkf {} {} {} {} {} {} {} {}", tick, pos.x, pos.y, pos.z, ang.x, ang.y,
                               ang.z, fov);
    interfaces::engine->ClientCmd(command.c_str());
    return 1;
}

static int ClearKeyframes(lua_State* L) {
    auto command = std::format("spt_cam_path_clear");
    interfaces::engine->ClientCmd(command.c_str());
    return 1;
}

static int SetInterp(lua_State* L) {
    if (lua_gettop(L) != 1) {
        return luaL_error(L, "camera.set_interp: takes 1 argument");
    }

    if (!lua_isnumber(L, 1)) {
        return luaL_error(L, "camera.set_interp: argument is not a number");
    }

    auto interp = lua_tointeger(L, 1);

    auto command = std::format("spt_cam_path_interp {}", interp);
    interfaces::engine->ClientCmd(command.c_str());
    return 1;
}

static const struct luaL_Reg camera_class[] = {
    {"set_control", SetControl},
    {"set_pos", SetPos},
    {"set_ang", SetAng},
    {"add_keyframe", AddKeyframe},
    {"clear_keyframes", ClearKeyframes},
    {"set_interp", SetInterp},
    {nullptr, nullptr}
};

void LuaCameraLibrary::Load(lua_State* L) {
    lua_new_class(L, "camera", camera_class);
}

void LuaCameraLibrary::Unload(lua_State* L) {}

const std::string& LuaCameraLibrary::GetLuaSource() {
    static std::string sources = R"""(---@meta
---@class camera
camera = {}

---@class CameraControl : number
CameraControl = {
    DEFAULT = 0,
    DRIVE = 1,
    CINEMATIC = 2,
    ENTITY = 3
}

---@class CameraInterp : number
CameraInterp = {
    LINEAR = 0,
    CUBIC = 1,
    PCHIP = 2
}

---@param control CameraControl
function camera.set_control(control)
end

---@param pos vec3
function camera.set_pos(pos)
end

---@param ang vec2
function camera.set_ang(ang)
end

---@param tick number
---@param pos vec3
---@param ang vec2
---@param fov number
function camera.add_keyframe(tick, pos, ang, fov)
end

function camera.clear_keyframes()
end

---@param interp CameraInterp
function camera.set_interp(interp)
end
)""";
    return sources;
}
