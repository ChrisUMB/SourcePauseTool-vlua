#pragma once

#include "../lua_feature.hpp"
#include "mathlib/vector4d.h"
#include "mathlib/vector.h"
#include "mathlib/vmatrix.h"

class LuaMathLibrary : public LuaLibrary {
public:
    explicit LuaMathLibrary();

    void Load(lua_State *L) override;

    const std::string& GetLuaSource() override;


    static void LuaPushVector2D(lua_State *L, const Vector2D& vector);
    static Vector2D LuaGetVector2D(lua_State *L, int index);
    static bool LuaIsVector2D(lua_State *L, int index);

    static void LuaPushVector3D(lua_State *L, const Vector& vector);
    static Vector LuaGetVector3D(lua_State *L, int index);
    static bool LuaIsVector3D(lua_State *L, int index);

    static void LuaPushVector4D(lua_State *L, const Vector4D& vector);
    static Vector4D LuaGetVector4D(lua_State *L, int index);
    static bool LuaIsVector4D(lua_State *L, int index);

    static void LuaPushQuaternion(lua_State *L, const Quaternion& quaternion);
    static Quaternion LuaGetQuaternion(lua_State *L, int index);
    static bool LuaIsQuaternion(lua_State *L, int index);

    static void LuaPushMatrix(lua_State *L, const VMatrix& matrix);
    static VMatrix LuaGetMatrix(lua_State *L, int index);
    static bool LuaIsMatrix(lua_State *L, int index);

    static void LuaPushAngle(lua_State *L, const QAngle& angle);
    static QAngle LuaGetAngle(lua_State *L, int index);
    static bool LuaIsAngle(lua_State *L, int index);
};

extern LuaMathLibrary lua_math_library;