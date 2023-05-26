#include "stdafx.hpp"
#include "lua_lib_math.hpp"
#include "spt/features/lua/lua_util.hpp"

LuaMathLibrary lua_math_library;

LuaMathLibrary::LuaMathLibrary() : LuaLibrary("math") {}

void LuaMathLibrary::Load(lua_State* L) {}

#define LUA_GET(target, name) \
	lua_getfield(L, index, name); \
	target = (vec_t)lua_tonumber(L, -1); \
	lua_pop(L, 1);

#define LUA_SET(source, name) \
	lua_pushnumber(L, source); \
	lua_setfield(L, -2, name);

void LuaMathLibrary::LuaPushVector2D(lua_State* L, const Vector2D& vector)
{
	lua_newtable(L);
	LUA_SET(vector.x, "x")
	LUA_SET(vector.y, "y")
	lua_getglobal(L, "vec2");
	lua_setmetatable(L, -2);
}

Vector2D LuaMathLibrary::LuaGetVector2D(lua_State* L, int index)
{
	if (!LuaCheckClass(L, index, "vec2"))
	{
		Warning("Lua Error: LuaGetVector2D invoked without checking\n");
		return {0, 0};
	}

	Vector2D result;
	LUA_GET(result.x, "x")
	LUA_GET(result.y, "y")
	return result;
}

bool LuaMathLibrary::LuaIsVector2D(lua_State* L, int index)
{
	return LuaIsClass(L, index, "vec2");
}

void LuaMathLibrary::LuaPushVector3D(lua_State* L, const Vector& vector)
{
	lua_newtable(L);
	LUA_SET(vector.x, "x")
	LUA_SET(vector.y, "y")
	LUA_SET(vector.z, "z")
	lua_getglobal(L, "vec3");
	lua_setmetatable(L, -2);
}

Vector LuaMathLibrary::LuaGetVector3D(lua_State* L, int index)
{
	if (!LuaCheckClass(L, index, "vec3"))
	{
		Warning("Lua Error: LuaGetVector3D invoked without checking\n");
		return {0, 0, 0};
	}

	Vector result;
	LUA_GET(result.x, "x")
	LUA_GET(result.y, "y")
	LUA_GET(result.z, "z")
	return result;
}

bool LuaMathLibrary::LuaIsVector3D(lua_State* L, int index)
{
	return LuaIsClass(L, index, "vec3");
}

void LuaMathLibrary::LuaPushVector4D(lua_State* L, const Vector4D& vector)
{
	lua_newtable(L);
	LUA_SET(vector.x, "x")
	LUA_SET(vector.y, "y")
	LUA_SET(vector.z, "z")
	LUA_SET(vector.w, "w")
	lua_getglobal(L, "vec4");
	lua_setmetatable(L, -2);
}

bool LuaMathLibrary::LuaIsVector4D(lua_State* L, int index)
{
	return LuaIsClass(L, index, "vec4");
}

Vector4D LuaMathLibrary::LuaGetVector4D(lua_State* L, int index)
{
	if (!LuaCheckClass(L, index, "vec4"))
	{
		Warning("Lua Error: LuaGetVector4D invoked without checking\n");
		return {0, 0, 0, 0};
	}

	Vector4D result;
	LUA_GET(result.x, "x")
	LUA_GET(result.y, "y")
	LUA_GET(result.z, "z")
	LUA_GET(result.w, "w")
	return result;
}

void LuaMathLibrary::LuaPushQuaternion(lua_State* L, const Quaternion& quaternion)
{
	lua_newtable(L);
	LUA_SET(quaternion.x, "x")
	LUA_SET(quaternion.y, "y")
	LUA_SET(quaternion.z, "z")
	LUA_SET(quaternion.w, "w")
	lua_getglobal(L, "quat");
	lua_setmetatable(L, -2);
}

Quaternion LuaMathLibrary::LuaGetQuaternion(lua_State* L, int index)
{
	if (!LuaCheckClass(L, index, "quat"))
	{
		Warning("Lua Error: LuaGetQuaternion invoked without checking\n");
		return {0, 0, 0, 0};
	}

	Quaternion result;
	LUA_GET(result.x, "x")
	LUA_GET(result.y, "y")
	LUA_GET(result.z, "z")
	LUA_GET(result.w, "w")
	return result;
}

bool LuaMathLibrary::LuaIsQuaternion(lua_State* L, int index)
{
	return LuaIsClass(L, index, "quat");
}

void LuaMathLibrary::LuaPushMatrix(lua_State* L, const VMatrix& matrix)
{
	lua_newtable(L);

	// this is intentionally transposed
	LUA_SET(matrix[0][0], "m00")
	LUA_SET(matrix[1][0], "m01")
	LUA_SET(matrix[2][0], "m02")
	LUA_SET(matrix[3][0], "m03")
	LUA_SET(matrix[0][1], "m10")
	LUA_SET(matrix[1][1], "m11")
	LUA_SET(matrix[2][1], "m12")
	LUA_SET(matrix[3][1], "m13")
	LUA_SET(matrix[0][2], "m20")
	LUA_SET(matrix[1][2], "m21")
	LUA_SET(matrix[2][2], "m22")
	LUA_SET(matrix[3][2], "m23")
	LUA_SET(matrix[0][3], "m30")
	LUA_SET(matrix[1][3], "m31")
	LUA_SET(matrix[2][3], "m32")
	LUA_SET(matrix[3][3], "m33")
	lua_getglobal(L, "mat4");
	lua_setmetatable(L, -2);
}

VMatrix LuaMathLibrary::LuaGetMatrix(lua_State* L, int index)
{
	if (!LuaCheckClass(L, index, "mat4"))
	{
		Warning("Lua Error: LuaGetMatrix invoked without checking\n");
		return VMatrix();
	}

	VMatrix result;
	LUA_GET(result[0][0], "m00")
	LUA_GET(result[0][1], "m01")
	LUA_GET(result[0][2], "m02")
	LUA_GET(result[0][3], "m03")
	LUA_GET(result[1][0], "m10")
	LUA_GET(result[1][1], "m11")
	LUA_GET(result[1][2], "m12")
	LUA_GET(result[1][3], "m13")
	LUA_GET(result[2][0], "m20")
	LUA_GET(result[2][1], "m21")
	LUA_GET(result[2][2], "m22")
	LUA_GET(result[2][3], "m23")
	LUA_GET(result[3][0], "m30")
	LUA_GET(result[3][1], "m31")
	LUA_GET(result[3][2], "m32")
	LUA_GET(result[3][3], "m33")
	return result;
}

bool LuaMathLibrary::LuaIsMatrix(lua_State* L, int index)
{
	return LuaIsClass(L, index, "mat4");
}

void LuaMathLibrary::LuaPushAngle(lua_State* L, const QAngle& angle)
{
	lua_newtable(L);
	LUA_SET(angle.x, "x")
	LUA_SET(angle.y, "y")
	LUA_SET(angle.z, "z")
	lua_getglobal(L, "vec3");
	lua_setmetatable(L, -2);
}

QAngle LuaMathLibrary::LuaGetAngle(lua_State* L, int index)
{
	if (!LuaCheckClass(L, index, "vec3"))
	{
		Warning("Lua Error: LuaGetAngle invoked without checking\n");
		return {0, 0, 0};
	}

	QAngle result;
	LUA_GET(result.x, "x")
	LUA_GET(result.y, "y")
	LUA_GET(result.z, "z")
	return result;
}

bool LuaMathLibrary::LuaIsAngle(lua_State* L, int index)
{
	return LuaIsClass(L, index, "vec3");
}

const std::string& LuaMathLibrary::GetLuaSource()
{
	static const std::string sources = R"(---@meta
function table.shallow_copy(t)
    local t2 = {}
    for k, v in pairs(t) do
        t2[k] = v
    end
    return t2
end

function table.all_keys_present(t, ...)
    local keys = { ... }
    for i = 1, #keys do
        if rawget(t, keys[i]) == nil then
            return false
        end
    end

    return true
end

-- unpack() was moved to table.unpack in Lua 5.2, we do this so the math library works in versions below and above that.
if not unpack then
    unpack = table.unpack
end
)"
	                                   R"(
---@class vec2 2D vector, XY
---@operator call:vec2
---@operator unm:vec2
---@operator mul(vec2):vec2
---@operator div(vec2):vec2
---@operator mod(vec2):vec2
---@operator add(vec2):vec2
---@operator sub(vec2):vec2
---@field x number X component of the vector
---@field y number Y component of the vector
vec2 = {
    components = { "x", "y" }
}

---@param x number|number[]|vec2|table|nil
---@param y number|nil
---@return vec2
function vec2.new(x, y)
    if x == nil then
        return vec2(0, 0)
    end

    local result
    local xType = type(x)

    if xType == "table" then
        if getmetatable(x) == vec2 then
            result = table.shallow_copy(x)
        elseif table.all_keys_present(x, "x", "y") then
            result = {
                x = x.x,
                y = x.y
            }
        elseif table.all_keys_present(x, 1, 2) then
            result = {
                x = x[1],
                y = x[2]
            }
        elseif table.all_keys_present(x, "1", "2") then
            result = {
                x = x["1"],
                y = x["2"]
            }
        else
            return nil
        end
    else
        if xType == "number" then
            if y ~= nil then
                result = { x = x, y = y }
            else
                result = { x = x, y = x }
            end
        end
    end

    setmetatable(result, vec2)
    return result
end

---@param ... number|number[]|vec2|table
---@return vec2
function vec2:add(...)
    local other = vec2(...)
    return vec2(self.x + other.x, self.y + other.y)
end

---@param other number|number[]|vec2|table
---@return vec2
function vec2:__add(other)
    return self:add(other)
end

---@param ... number|number[]|vec2|table
---@return vec2
function vec2:sub(...)
    local other = vec2(...)
    return vec2(self.x - other.x, self.y - other.y)
end

---@param other number|number[]|vec2|table
---@return vec2
function vec2:__sub(other)
    return self:sub(other)
end

---@param ... number|number[]|vec2|table
---@return vec2
function vec2:mul(...)
    local other = vec2(...)
    return vec2(self.x * other.x, self.y * other.y)
end

---@param other number|number[]|vec2|table
---@return vec2
function vec2:__mul(other)
    return self:mul(other)
end

---@param ... number|number[]|vec2|table
---@return vec2
function vec2:div(...)
    local other = vec2(...)
    return vec2(self.x / other.x, self.y / other.y)
end

---@param other number|number[]|vec2|table
---@return vec2
function vec2:__div(other)
    return self:div(other)
end

---@param ... number|number[]|vec2|table
---@return vec2
function vec2:mod(...)
    local other = vec2(...)
    return vec2(self.x % other.x, self.y % other.y)
end

---@param other number|number[]|vec2|table
---@return vec2
function vec2:__mod(other)
    return self:mod(other)
end

---@rturn vec2
function vec2:negate()
    return vec2(-self.x, -self.y)
end

---@return vec2
function vec2:__unm()
    return self:negate()
end

---@param ... number|number[]|vec2|table
---@return boolean
function vec2:equals(...)
    local other = vec2(...)
    return self.x == other.x and self.y == other.y
end

---@param other number|number[]|vec2|table
---@return boolean
function vec2:__eq(other)
    return self:equals(other)
end

---@param other number|number[]|vec2|table
---@return boolean
function vec2:__lt(other)
    other = vec2(other)
    return self:length() < other:length()
end

---@param other number|number[]|vec2|table
---@return boolean
function vec2:__le(other)
    other = vec2(other)
    return self:length() <= other:length()
end

---@return number
function vec2:length_squared()
    return self.x * self.x + self.y * self.y
end

---@return number
function vec2:length()
    return math.sqrt(self:length_squared())
end

---@return number
function vec2:magnitude()
    return self:length()
end

---@return vec2
function vec2:normalize()
    return self / self:length()
end

---@param ... number|number[]|vec2|table
---@return number
function vec2:distance(...)
    local other = vec2(...)
    return (self - other):length()
end

---@param ... number|number[]|vec2|table
---@return number
function vec2:distance_squared(...)
    local diff = self - vec2(...)
    return diff:length_squared()
end

---@param ... number|number[]|vec2|table
---@return number
function vec2:cross(...)
    local other = vec2(...)
    return self.x * other.y - self.y * other.x
end

---@param ... number|number[]|vec2|table
---@return number
function vec2:dot(...)
    local other = vec2(...)
    return self.x * other.x + self.y * other.y
end

---@param ... number|number[]|vec2|table
---@return number
function vec2:angle_to(...)
    local other = vec2(...)
    local dot = self:dot(other)
    local det = self:cross(other)
    return math.atan2(det, dot)
end

---@return vec2
function vec2:floor()
    return vec2(math.floor(self.x), math.floor(self.y))
end

---@return vec2
function vec2:ceil()
    return vec2(math.ceil(self.x), math.ceil(self.y))
end

---@return number
function vec2:angle()
    return math.atan2(self.y, self.x)
end

---@return vec2
function vec2:round()
    return vec2(math.round(self.x), math.round(self.y))
end

---@return vec2
function vec2:degrees()
    return vec2(math.degrees(self.x), math.degrees(self.y))
end

---@param func fun(c:number):number
---@return vec2
function vec2:map(func)
    return vec2(func(self.x), func(self.y))
end

---@param other number|number[]|vec2|table
---@return vec2
function vec2:look_at(other)
    return (other - self):normalize()
end

---@return number, number
function vec2:unpack()
    return self.x, self.y
end

function vec2:__index(name)
    if type(name) == "number" then
        name = vec2.components[name]
    end

    return rawget(self, name) or rawget(vec2, name)
end

function vec2:__newindex(key, value)
    if type(key) == "number" then
        key = vec2.components[key]
    end

    rawset(self, key, value)
end

function vec2:__call(...)
    local args = { ... }
    return vec2.new(unpack(args))
end

function vec2:__tostring()
    return "{x=" .. self.x .. ", y=" .. self.y .. "}"
end

setmetatable(vec2, vec2)

)"
	                                   R"(
---@class vec3 3D vector, XYZ
---@operator call:vec3
---@operator unm:vec3
---@operator mul(vec3):vec3
---@operator div(vec3):vec3
---@operator mod(vec3):vec3
---@operator add(vec3):vec3
---@operator sub(vec3):vec3
---@field x number X component of the vector
---@field y number Y component of the vector
---@field z number Z component of the vector
vec3 = {
    components = { "x", "y", "z" }
}

---@param x number|number[]|vec3|table|nil
---@param y number|nil
---@param z number|nil
---@return vec3
function vec3.new(x, y, z)
    if x == nil then
        return vec3(0, 0, 0)
    end

    local result
    local xType = type(x)

    if xType == "table" then
        if getmetatable(x) == vec3 then
            result = table.shallow_copy(x)
        elseif table.all_keys_present(x, "x", "y", "z") then
            result = {
                x = x.x,
                y = x.y,
                z = x.z
            }
        elseif table.all_keys_present(x, 1, 2, 3) then
            result = {
                x = x[1],
                y = x[2],
                z = x[3]
            }
        elseif table.all_keys_present(x, "1", "2", "3") then
            result = {
                x = x["1"],
                y = x["2"],
                z = x["3"]
            }
        else
            return nil
        end
    else
        if xType == "number" then
            if y ~= nil and z ~= nil then
                result = { x = x, y = y, z = z }
            else
                result = { x = x, y = x, z = x }
            end
        end
    end

    setmetatable(result, vec3)
    return result
end

---@param ... number|number[]|vec3|table
---@return vec3
function vec3:add(...)
    local other = vec3(...)
    return vec3(self.x + other.x, self.y + other.y, self.z + other.z)
end

---@param other number|number[]|vec3|table
---@return vec3
function vec3:__add(other)
    return self:add(other)
end

---@param ... number|number[]|vec3|table
---@return vec3
function vec3:sub(...)
    local other = vec3(...)
    return vec3(self.x - other.x, self.y - other.y, self.z - other.z)
end

---@param other number|number[]|vec3|table
---@return vec3
function vec3:__sub(other)
    return self:sub(other)
end

---@param ... number|number[]|vec3|table
---@return vec3
function vec3:mul(...)
    local other = vec3(...)
    return vec3(self.x * other.x, self.y * other.y, self.z * other.z)
end

---@param other number|number[]|vec3|table
---@return vec3
function vec3:__mul(other)
    return self:mul(other)
end

---@param ... number|number[]|vec3|table
---@return vec3
function vec3:div(...)
    local other = vec3(...)
    return vec3(self.x / other.x, self.y / other.y, self.z / other.z)
end

---@param other number|number[]|vec3|table
---@return vec3
function vec3:__div(other)
    return self:div(other)
end

---@param ... number|number[]|vec3|table
---@return vec3
function vec3:mod(...)
    local other = vec3(...)
    return vec3(self.x % other.x, self.y % other.y, self.z % other.z)
end

---@param other number|number[]|vec3|table
---@return vec3
function vec3:__mod(other)
    return self:mod(other)
end

---@return vec3
function vec3:negate()
    return vec3(-self.x, -self.y, -self.z)
end

---@return vec3
function vec3:__unm()
    return self:negate()
end

---@param ... number|number[]|vec3|table
---@return boolean
function vec3:equals(...)
    local other = vec3(...)
    return self.x == other.x and self.y == other.y and self.z == other.z
end

---@param other number|number[]|vec3|table
---@return boolean
function vec3:__eq(other)
    return self:equals(other)
end

---@param ... number|number[]|vec3|table
---@return boolean
function vec3:__lt(other)
    other = vec3(other)
    return self:length() < other:length()
end

---@param ... number|number[]|vec3|table
---@return boolean
function vec3:__le(other)
    other = vec3(other)
    return self:length() <= other:length()
end

---@return number
function vec3:length_squared()
    return self.x * self.x + self.y * self.y + self.z * self.z
end

---@return number
function vec3:length()
    return math.sqrt(self:length_squared())
end

---@return number
function vec3:magnitude()
    return self:length()
end

---@return vec3
function vec3:normalize()
    return self / self:length()
end

---@param ... number|number[]|vec3|table
---@return number
function vec3:distance(...)
    local diff = self - vec3(...)
    return diff:length()
end

---@param ... number|number[]|vec3|table
---@return number
function vec3:distance_squared(...)
    local diff = self - vec3(...)
    return diff:length_squared()
end

---@param ... number|number[]|vec3|table
---@return vec3
function vec3:cross(...)
    local other = vec3(...)
    local x = self.y * other.z - self.z * other.y
    local y = self.z * other.x - self.x * other.z
    local z = self.x * other.y - self.y * other.x
    return vec3(x, y, z)
end

---@param ... number|number[]|vec3|table
---@return number
function vec3:dot(...)
    local other = vec3(...)
    return self.x * other.x + self.y * other.y + self.z * other.z
end

---@param ... number|number[]|vec3|table
---@return number
function vec3:angle_to(...)
    local other = vec3(...)
    local dot = self:dot(other)
    local det = self:cross(other)
    return math.atan2(det, dot)
end

---@return vec3
function vec3:floor()
    return vec3(math.floor(self.x), math.floor(self.y), math.floor(self.z))
end

---@return vec3
function vec3:ceil()
    return vec3(math.ceil(self.x), math.ceil(self.y), math.ceil(self.z))
end

---@return number
function vec3:angle()
    return math.atan2(self.y, self.x)
end

---@return vec3
function vec3:round()
    return vec3(math.round(self.x), math.round(self.y), math.round(self.z))
end

---@return vec3
function vec3:degrees()
    return vec3(math.deg(self.x), math.deg(self.y), math.deg(self.z))
end

---@return vec3
function vec3:map(func)
    return vec3(func(self.x), func(self.y), func(self.z))
end

---@param other number|number[]|vec3|table
---@return vec3
function vec3:look_at(other)
    return (other - self):normalize()
end

---@return number, number
function vec3:unpack()
    return self.x, self.y, self.z
end

function vec3:__index(name)
    if type(name) == "number" then
        name = vec3.components[name]
    end

    return rawget(self, name) or rawget(vec3, name)
end

function vec3:__newindex(key, value)
    if type(key) == "number" then
        key = vec3.components[key]
    end

    rawset(self, key, value)
end

function vec3:__call(...)
    local args = { ... }
    return vec3.new(unpack(args))
end

function vec3:__tostring()
    return "{x=" .. self.x .. ", y=" .. self.y .. ", z=" .. self.z .. "}"
end

setmetatable(vec3, vec3)

)"
	                                   R"(
---@class vec4 4D vector, XYZW
---@operator call:vec4
---@operator unm:vec4
---@operator mul(vec4):vec4
---@operator div(vec4):vec4
---@operator mod(vec4):vec4
---@operator add(vec4):vec4
---@operator sub(vec4):vec4
---@field x number
---@field y number
---@field z number
---@field w number
vec4 = {
    components = { "x", "y", "z", "w" }
}

---@param x number|number[]|vec4|table|nil
---@param y number|nil
---@param z number|nil
---@param w number|nil
---@return vec4
function vec4.new(x, y, z, w)
    if x == nil then
        return vec4(0, 0, 0, 0)
    end

    local result
    local xType = type(x)

    if xType == "table" then
        if getmetatable(x) == vec4 then
            result = table.shallow_copy(x)
        elseif getmetatable(x) == vec3 then
            result = vec4(x.x, x.y, x.z, y or 0)
        elseif table.all_keys_present(x, "x", "y", "z", "w") then
            result = {
                x = x.x,
                y = x.y,
                z = x.z,
                w = x.w
            }
        elseif table.all_keys_present(x, 1, 2, 3, 4) then
            result = {
                x = x[1],
                y = x[2],
                z = x[3],
                w = x[4]
            }
        elseif table.all_keys_present(x, "1", "2", "3", "4") then
            result = {
                x = x["1"],
                y = x["2"],
                z = x["3"],
                w = x["4"]
            }
        else
            return nil
        end
    else
        if xType == "number" then
            if y ~= nil and z ~= nil and w ~= nil then
                result = { x = x, y = y, z = z, w = w }
            else
                result = { x = x, y = x, z = x, w = x }
            end
        end
    end

    setmetatable(result, vec4)
    return result
end

---@param ... number|number[]|vec4|table
---@return vec4
function vec4:add(...)
    local other = vec4(...)
    return vec4(self.x + other.x, self.y + other.y, self.z + other.z, self.w + other.w)
end

---@param other number|number[]|vec4|table
---@return vec4
function vec4:__add(other)
    return self:add(other)
end

---@param ... number|number[]|vec4|table
---@return vec4
function vec4:sub(...)
    local other = vec4(...)
    return vec4(self.x - other.x, self.y - other.y, self.z - other.z, self.w + other.w)
end

---@param other number|number[]|vec4|table
---@return vec4
function vec4:__sub(other)
    return self:sub(other)
end

---@param ... number|number[]|vec4|table
---@return vec4
function vec4:mul(...)
    local other = vec4(...)
    return vec4(self.x * other.x, self.y * other.y, self.z * other.z, self.w * other.w)
end

---@param other number|number[]|vec4|table
---@return vec4
function vec4:__mul(other)
    return self:mul(other)
end

---@param ... number|number[]|vec4|table
---@return vec4
function vec4:div(...)
    local other = vec4(...)
    return vec4(self.x / other.x, self.y / other.y, self.z / other.z, self.w / other.w)
end

---@param other number|number[]|vec4|table
---@return vec4
function vec4:__div(other)
    return self:div(other)
end

---@param ... number|number[]|vec4|table
---@return vec4
function vec4:mod(...)
    local other = vec4(...)
    return vec4(self.x % other.x, self.y % other.y, self.z % other.z, self.w % other.w)
end

---@param other number|number[]|vec4|table
---@return vec4
function vec4:__mod(other)
    return self:mod(other)
end

---@return vec4
function vec4:negate()
    return vec4(-self.x, -self.y, -self.z, -self.w)
end

---@return vec4
function vec4:__unm()
    return self:negate()
end

---@param ... number|number[]|vec4|table
---@return vec4
function vec4:equals(...)
    local other = vec4(...)
    return self.x == other.x and self.y == other.y and self.z == other.z and self.w == other.w
end

---@param other number|number[]|vec4|table
---@return boolean
function vec4:__eq(other)
    return self:equals(other)
end

---@param ... number|number[]|vec4|table
---@return boolean
function vec4:__lt(other)
    other = vec4(other)
    return self:length() < other:length()
end

---@param other number|number[]|vec4|table
---@return boolean
function vec4:__le(other)
    other = vec4(other)
    return self:length() <= other:length()
end

---@return number
function vec4:length_squared()
    return self.x * self.x + self.y * self.y + self.z * self.z + self.w * self.w
end

---@return number
function vec4:length()
    return math.sqrt(self:length_squared())
end

---@return number
function vec4:magnitude()
    return self:length()
end

---@return number
function vec4:normalize()
    return self / self:length()
end

---@param ... number|number[]|vec4|table
---@return number
function vec4:distance(...)
    local other = vec4(...)
    return (self - other):length()
end

---@param ... number|number[]|vec4|table
---@return number
function vec4:distance_squared(...)
    local other = vec4(...)
    return (self - other):length_squared()
end

---@param ... number|number[]|vec4|table
---@return vec4
function vec4:cross(...)
    local other = vec4(...)
    return vec4(
        self.y * other.z - self.z * other.y,
        self.z * other.x - self.x * other.z,
        self.x * other.y - self.y * other.x,
        0
    )
end

---@param ... number|number[]|vec4|table
---@return number
function vec4:dot(...)
    local other = vec4(...)
    return self.x * other.x + self.y * other.y + self.z * other.z + self.w * other.w
end

---@param ... number|number[]|vec4|table
---@return number
function vec4:angle_to(...)
    local other = vec4(...)
    local dot = self:dot(other)
    local det = self:cross(other)
    return math.atan2(det, dot)
end

---@return vec4
function vec4:floor()
    return vec4(math.floor(self.x), math.floor(self.y), math.floor(self.z), math.floor(self.w))
end

---@return vec4
function vec4:ceil()
    return vec4(math.ceil(self.x), math.ceil(self.y), math.ceil(self.z), math.ceil(self.w))
end

---@return number
function vec4:angle()
    return math.atan2(self.y, self.x)
end

---@return vec4
function vec4:round()
    return vec4(math.round(self.x), math.round(self.y), math.round(self.z), math.round(self.w))
end

---@return vec4
function vec4:degrees()
    return vec4(math.deg(self.x), math.deg(self.y), math.deg(self.z), math.deg(w))
end

---@param func fun(c:number):number
---@return vec4
function vec4:map(func)
    return vec4(func(self.x), func(self.y), func(self.z), func(self.w))
end

---@param other number|number[]|vec4|table
---@return vec4
function vec4:look_at(other)
    return (other - self):normalize()
end

---@return number, number
function vec4:unpack()
    return self.x, self.y, self.z, self.w
end

function vec4:__index(name)
    if type(name) == "number" then
        name = vec4.components[name]
    end

    return rawget(self, name) or rawget(vec4, name)
end

function vec4:__newindex(key, value)
    if type(key) == "number" then
        key = vec4.components[key]
    end

    rawset(self, key, value)
end

function vec4:__call(...)
    local args = { ... }
    return vec4.new(unpack(args))
end

function vec4:__tostring()
    return "{x=" .. self.x .. ", y=" .. self.y .. ", z=" .. self.z .. ", w=" .. self.w .. "}"
end

setmetatable(vec4, vec4)
)"
	                                   R"(
---@class axis
---@field positive_y vec3
---@field negative_y vec3
---@field positive_x vec3
---@field negative_x vec3
---@field positive_z vec3
---@field negative_z vec3
axis = {}

local reference_axis = {
    positive_y = vec3(0, 1, 0),
    negative_y = vec3(0, -1, 0),
    positive_x = vec3(1, 0, 0),
    negative_x = vec3(-1, 0, 0),
    positive_z = vec3(0, 0, 1),
    negative_z = vec3(0, 0, -1)
}

reference_axis.positive_y.next = "negative_y"
reference_axis.negative_y.next = "positive_x"
reference_axis.positive_x.next = "negative_x"
reference_axis.negative_x.next = "positive_z"
reference_axis.positive_z.next = "negative_z"
reference_axis.negative_z.next = nil

function axis:__index(name)
    return vec3(rawget(reference_axis, name))
end

function axis:__call(table, i, x)
    if x == nil then
        return axis.positive_y
    end
    if x.next == nil then
        return nil
    end

    return axis[x.next]
end

setmetatable(axis, axis)
)"
	                                   R"(
---@class quat Quaternion, XYZW
---@operator call:quat
---@field x number X component of the quaternion.
---@field y number Y component of the quaternion.
---@field z number Z component of the quaternion.
---@field w number W component of the quaternion.
quat = {
    components = { "x", "y", "z", "w" }
}

---@param x number|table|quat|vec4
---@param y number|nil Y component of the quaternion.
---@param z number|nil Z component of the quaternion.
---@param w number|nil W component of the quaternion.
---@return quat
function quat.new(x, y, z, w)
    if x == nil then
        return quat(0, 0, 0, 1)
    end

    local result = nil
    local xType = type(x)
    local meta = getmetatable(x)

    if xType == "table" then
        if meta == quat then
            result = table.shallow_copy(x)
        elseif table.all_keys_present(x, "x", "y", "z", "w") then
            result = {
                x = x.x,
                y = x.y,
                z = x.z,
                w = x.w
            }
        elseif table.all_keys_present(x, 1, 2, 3, 4) then
            result = {
                x = x[1],
                y = x[2],
                z = x[3],
                w = x[4]
            }
        elseif table.all_keys_present(x, "1", "2", "3", "4") then
            result = {
                x = x["1"],
                y = x["2"],
                z = x["3"],
                w = x["4"]
            }
        else
            return nil
        end
    else
        if xType == "number" then
            if y ~= nil and z ~= nil and w ~= nil then
                result = { x = x, y = y, z = z, w = w }
            else
                result = { x = x, y = x, z = x, w = x }
            end
        end
    end

    setmetatable(result, quat)
    result:set_normalized()
    return result
end

---@param ... table|quat|vec4
---@return quat
function quat:add(...)
    other = quat(...)
    return quat(self.x + other.x, self.y + other.y, self.z + other.z, self.w + other.w)
end

---@param other quat
---@return quat
function quat:__add(other)
    return self:add(other)
end

---@param ... table|quat|vec4
---@return quat
function quat:sub(...)
    local other = quat(...)
    return quat(self.x - other.x, self.y - other.y, self.z - other.z, self.w + other.w)
end

---@param other table|quat|vec4
---@return quat
function quat:__sub(other)
    return self:sub(other)
end

---@param ... table|quat|vec4
---@return quat
function quat:mul(...)
    local other = quat(...)
    local nx = self.w * other.x + self.x * other.w + self.y * other.z - self.z * other.y
    local ny = self.w * other.y + self.y * other.w + self.z * other.x - self.x * other.z
    local nz = self.w * other.z + self.z * other.w + self.x * other.y - self.y * other.x
    local nw = self.w * other.w - self.x * other.x - self.y * other.y - self.z * other.z
    return quat(nx, ny, nz, nw)
end


math.fma = function(a, b, c)
    return a * b + c
end

---@param ... number[]|number|table|vec3
---@return vec3
function quat:transform(...)
    local other = vec3(...)
    local x = other.x
    local y = other.y
    local z = other.z
    local xx = self.x * self.x
    local yy = self.y * self.y
    local zz = self.z * self.z
    local ww = self.w * self.w
    local xy = self.x * self.y
    local xz = self.x * self.z
    local yz = self.y * self.z
    local xw = self.x * self.w
    local zw = self.z * self.w
    local yw = self.y * self.w
    local k = 1.0 / (xx + yy + zz + ww)

    return vec3(
            math.fma((xx - yy - zz + ww) * k, x, math.fma(2.0 * (xy - zw) * k, y, (2.0 * (xz + yw) * k) * z)),
            math.fma(2.0 * (xy + zw) * k, x, math.fma((yy - xx - zz + ww) * k, y, (2.0 * (yz - xw) * k) * z)),
            math.fma(2.0 * (xz - yw) * k, x, math.fma(2.0 * (yz + xw) * k, y, ((zz - xx - yy + ww) * k) * z))
    )
end

---@param ... table|quat|vec4
---@return quat
function quat:__mul(other)
    return self:mul(other)
end

---@param ... number|number[]|table|quat|vec4
---@return quat
function quat:div(...)
    local other = quat(...)
    return self:mul(other:conjugate())
end

---@param other table|quat|vec4
---@return quat
function quat:__div(other)
    return self:div(other)
end

---@param ... number|number[]|table|quat|vec4
---@return quat
function quat:mod(...)
    local other = quat(...)
    return quat(self.x % other.x, self.y % other.y, self.z % other.z, self.w % other.w)
end

---@param other table|quat|vec4
---@return quat
function quat:__mod(other)
    return self:mod(other)
end

---@return quat
function quat:conjugate()
    return quat(-self.x, -self.y, -self.z, self.w)
end

---@return number
function quat:length_squared()
    return self.x * self.x + self.y * self.y + self.z * self.z + self.w * self.w
end

---@return number
function quat:length()
    return math.sqrt(self:length_squared())
end

---@return quat
function quat:normalize()
    local q_length = self:length()

    return quat(self.x / q_length, self.y / q_length, self.z / q_length, self.w / q_length)
end

---@return quat
function quat:set_normalized()
    local q_length = self:length()
    self.x = self.x / q_length
    self.y = self.y / q_length
    self.z = self.z / q_length
    self.w = self.w / q_length
    return self
end

---@param angle number
---@param ... number[]|number|table|vec3
function quat:rotate(angle, ...)
    local axis = vec3(...)
    local hangle = angle / 2.0
    local sin_angle = math.sin(hangle)
    local inv_v_length = 1.0 / math.sqrt(axis.x * axis.x + axis.y * axis.y + axis.z * axis.z)
    local rx = axis.x * inv_v_length * sin_angle
    local ry = axis.y * inv_v_length * sin_angle
    local rz = axis.z * inv_v_length * sin_angle
    local rw = math.cos(hangle)
    return quat(
            math.fma(self.w, rx, math.fma(self.x, rw, math.fma(self.y, rz, -self.z * ry))),
            math.fma(self.w, ry, math.fma(-self.x, rz, math.fma(self.y, rw, self.z * rx))),
            math.fma(self.w, rz, math.fma(self.x, ry, math.fma(-self.y, rx, self.z * rw))),
            math.fma(self.w, rw, math.fma(-self.x, rx, math.fma(-self.y, ry, -self.z * rz)))
    )
end

---@return vec3
function quat:to_euler()
    local x = self.x
    local y = self.y
    local z = self.z
    local w = self.w

    local t0 = 2.0 * (w * x + y * z)
    local t1 = 1.0 - 2.0 * (x * x + y * y)
    local roll = math.atan(t0, t1)

    local t2 = 2.0 * (w * y - z * x)
    if t2 > 1.0 then
        t2 = 1.0
    elseif t2 < -1.0 then
        t2 = -1.0
    end

    local pitch = math.asin(t2)

    local t3 = 2.0 * (w * z + x * y)
    local t4 = 1.0 - 2.0 * (y * y + z * z)
    local yaw = math.atan(t3, t4)

    local dr = math.abs(roll - math.pi * 2)
    if dr < 0.001 then
        roll = 0
    end

    if math.abs(pitch - math.pi * 2) < 0.001 then
        pitch = 0
    end

    if math.abs(yaw - math.pi * 2) < 0.001 then
        yaw = 0
    end

    return vec3(roll, pitch, yaw)
end

---@param euler_angles table|vec3
---@return quat
function quat.from_euler(euler_angles)
    local yaw = euler_angles.z
    local pitch = euler_angles.y
    local roll = euler_angles.x

    local sr = math.sin(roll / 2)
    local cr = math.cos(roll / 2)
    local sp = math.sin(pitch / 2)
    local cp = math.cos(pitch / 2)
    local sy = math.sin(yaw / 2)
    local cy = math.cos(yaw / 2)

    local qx = sr * cp * cy - cr * sp * sy
    local qy = cr * sp * cy + sr * cp * sy
    local qz = cr * cp * sy - sr * sp * cy
    local qw = cr * cp * cy + sr * sp * sy

    return quat(qx, qy, qz, qw)
end

---@return quat # conjugate
function quat:__unm()
    return self:conjugate()
end

---@return boolean
function quat:equals(...)
    local other = quat(...)
    return self.x == other.x and self.y == other.y and self.z == other.z and self.w == other.w
end

---@return vec3
function quat:to_vec3()
    return vec3(self.x, self.y, self.z)
end

---@return vec4
function quat:to_vec4()
    return vec4(self.x, self.y, self.z)
end

---@return vec3
function quat:positive_z()
    return self:transform(0.0, 0.0, 1.0)
end

---@return vec3
function quat:negative_z()
    return self:transform(0.0, 0.0, -1.0)
end

---@return vec3
function quat:positive_y()
    return self:transform(0.0, 1.0, 0.0)
end

---@return vec3
function quat:negative_y()
    return self:transform(0.0, -1.0, 0.0)
end

---@return vec3
function quat:positive_x()
    return self:transform(1.0, 0.0, 0.0)
end

---@return vec3
function quat:negative_x()
    return self:transform(-1.0, 0.0, 0.0)
end

---@return string
function quat:__tostring()
    return "{x=" .. self.x .. ", y=" .. self.y .. ", z=" .. self.z .. ", w=" .. self.w .. "}"
end

---@param start vec3
---@param dest vec3
---@return quat
function quat.rotation_between_vectors(start, dest)
    local n_start = start:normalize()
    local n_dest = dest:normalize()

    local cos_theta = n_start:dot(n_dest)
    local axis = n_start:cross(n_dest)

    local s = math.sqrt((1.0 + cos_theta) * 2.0)
    local inv_s = 1.0 / s

    return quat(axis.x * inv_s, axis.y * inv_s, axis.z * inv_s, s * 0.5)
end

---@param dir vec3
---@param up vec3
---@return quat
function quat.look_at(dir, up)

    local dirn = dir:normalize()
    local left = up:cross(dirn):normalize()
    local upn = dirn:cross(left)

    local x, y, z, w = 0, 0, 0, 0

    local tr = left.x + upn.y + dirn.z

    if tr >= 0 then
        local t = math.sqrt(tr + 1)
        w = t * 0.5
        t = 0.5 / t
        x = (dirn.y - upn.z) * t
        y = (left.z - dirn.x) * t
        z = (upn.x - left.y) * t
    else
        if left.x > upn.y and left.x > dirn.z then
            local t = math.sqrt(1 + left.x - upn.y - dirn.z)
            x = t * 0.5
            t = 0.5 / t
            y = (left.y + upn.x) * t
            z = (dirn.x + left.z) * t
            w = (dirn.y - upn.z) * t
        elseif upn.y > dirn.z then
            local t = math.sqrt(1 + upn.y - left.x - dirn.z)
            y = t * 0.5
            t = 0.5 / t
            x = (left.y + upn.x) * t
            z = (upn.z + dirn.y) * t
            w = (left.z - dirn.x) * t
        else
            local t = math.sqrt(1 + dirn.z - left.x - upn.y)
            z = t * 0.5
            t = 0.5 / t
            x = (dirn.x + left.z) * t
            y = (upn.z + dirn.y) * t
            w = (upn.x - left.y) * t
        end
    end

    return quat(x, y, z, w)
end

---@param ... number|number[]|table|vec3
---@return quat
function quat.from_forward(...)
    local forward = vec3(...)

    if forward:length_squared() == 0.0 then
        return quat()
    end

    forward = forward:normalize()

    local right = vec3(0,0,1)
    if right:dot(forward) > 0.9 then
        right = vec3(1,0,0)
    end

    right = forward:cross(right):normalize()
    local up = forward:cross(right):normalize()

    local m = mat4()
    m.m00 = right.x
    m.m10 = right.y
    m.m20 = right.z
    m.m01 = up.x
    m.m11 = up.y
    m.m21 = up.z
    m.m02 = forward.x
    m.m12 = forward.y
    m.m22 = forward.z

    return m:to_quaternion()
end

function quat:__index(name)
    if type(name) == "number" then
        name = quat.components[name]
    end

    return rawget(self, name) or rawget(quat, name)
end

function quat:__newindex(key, value)
    if type(key) == "number" then
        key = quat.components[key]
    end

    rawset(self, key, value)
end

function quat:__call(...)
    local args = { ... }
    return quat.new(unpack(args))
end

function quat:__tostring()
    return "{x=" .. self.x .. ", y=" .. self.y .. ", z=" .. self.z .. ", w=" .. self.w .. "}"
end

setmetatable(quat, quat)
)"
	                                   R"(
---@class mat4
---@operator call:mat4
---@field m00 number
---@field m01 number
---@field m02 number
---@field m03 number
---@field m10 number
---@field m11 number
---@field m12 number
---@field m13 number
---@field m20 number
---@field m21 number
---@field m22 number
---@field m23 number
---@field m30 number
---@field m31 number
mat4 = {
    components = {
        "m00", "m01", "m02", "m03",
        "m10", "m11", "m12", "m13",
        "m20", "m21", "m22", "m23",
        "m30", "m31", "m32", "m33"
    }
}

---@return mat4
function mat4.new(
        m00, m01, m02, m03,
        m10, m11, m12, m13,
        m20, m21, m22, m23,
        m30, m31, m32, m33
)
    local result

    function is_vec4(t)
        if not t or type(t) ~= "table" then
            return false
        end
        return getmetatable(t) == vec4
    end

    local m00_type = type(m00)
    if m00_type == "table" then
        local meta = getmetatable(m00)
        -- If the first value passed is already a mat4, just return it.
        if meta == mat4 then
            local copy = table.shallow_copy(m00)
            setmetatable(copy, mat4)
            return copy
            -- Otherwise, if it's a vec4, the following 3 must be vec4's and need to be parsed.
        elseif meta == vec4 then
            if not is_vec4(m01) or not is_vec4(m02) or not is_vec4(m03) then
                return nil
            end

            local v = { m00, m01, m02, m03 }
            result = {}
            for x = 1, 4 do
                for y = 1, 4 do
                    result["m" .. y - 1 .. x - 1] = v[x][y]
                end
            end
        elseif meta == quat then
            local quat = m00
            result = {
                m03 = 0.0,
                m13 = 0.0,
                m23 = 0.0,
                m30 = 0.0,
                m31 = 0.0,
                m32 = 0.0,
                m33 = 1.0,
            }
            local w2 = quat.w * quat.w
            local x2 = quat.x * quat.x
            local y2 = quat.y * quat.y
            local z2 = quat.z * quat.z
            local zw = quat.z * quat.w
            local dzw = zw + zw
            local xy = quat.x * quat.y
            local dxy = xy + xy
            local xz = quat.x * quat.z
            local dxz = xz + xz
            local yw = quat.y * quat.w
            local dyw = yw + yw
            local yz = quat.y * quat.z
            local dyz = yz + yz
            local xw = quat.x * quat.w
            local dxw = xw + xw
            result.m00 = w2 + x2 - z2 - y2
            result.m01 = dxy + dzw
            result.m02 = dxz - dyw
            result.m10 = -dzw + dxy
            result.m11 = y2 - z2 + w2 - x2
            result.m12 = dyz + dxw
            result.m20 = dyw + dxz
            result.m21 = dyz - dxw
            result.m22 = z2 - y2 - x2 + w2
        else
            local tb_mat = m00

            if tb_mat:all_keys_present(
                    "r0", "r1", "r2", "r3",
                    "r4", "r5", "r6", "r7",
                    "r8", "r9", "r10", "r11",
                    "r12", "r13", "r14", "r15"
            ) then
                result = {}
                for i = 0, 15 do
                    local x = math.floor(i / 4)
                    local y = i % 4
                    local key = "m" .. x .. y

                    result[key] = tb_mat["r" .. i]
                end
            elseif tb_mat:all_keys_present(
                    1, 2, 3, 4,
                    5, 6, 7, 8,
                    9, 10, 11, 12,
                    13, 14, 15, 16
            ) then
                result = {}
                for i = 0, 15 do
                    local x = math.floor(i / 4)
                    local y = i % 4
                    local key = "m" .. x .. y
                    result[key] = tb_mat[i + 1]
                end
            else
                return nil
            end
        end
    elseif m00_type == "number" then
        result = {
            m00 = m00, m01 = m01, m02 = m02, m03 = m03,
            m10 = m10, m11 = m11, m12 = m12, m13 = m13,
            m20 = m20, m21 = m21, m22 = m22, m23 = m23,
            m30 = m30, m31 = m31, m32 = m32, m33 = m33
        }

        for _, v in pairs(result) do
            if type(v) ~= "number" then
                return nil
            end
        end
    elseif not m00 then
        -- Return identity matrix.
        result = {
            m00 = 1.0, m01 = 0.0, m02 = 0.0, m03 = 0.0,
            m10 = 0.0, m11 = 1.0, m12 = 0.0, m13 = 0.0,
            m20 = 0.0, m21 = 0.0, m22 = 1.0, m23 = 0.0,
            m30 = 0.0, m31 = 0.0, m32 = 0.0, m33 = 1.0
        }
    end

    setmetatable(result, mat4)
    return result
end

---@param ... any mat4 or anything that satisfies mat4 constructor
---@return mat4
function mat4:mul(...)
    local other = mat4(...)
    local nm00 = math.fma(self.m00, other.m00, math.fma(self.m10, other.m01, math.fma(self.m20, other.m02, self.m30 * other.m03)))
    local nm01 = math.fma(self.m01, other.m00, math.fma(self.m11, other.m01, math.fma(self.m21, other.m02, self.m31 * other.m03)))
    local nm02 = math.fma(self.m02, other.m00, math.fma(self.m12, other.m01, math.fma(self.m22, other.m02, self.m32 * other.m03)))
    local nm03 = math.fma(self.m03, other.m00, math.fma(self.m13, other.m01, math.fma(self.m23, other.m02, self.m33 * other.m03)))
    local nm10 = math.fma(self.m00, other.m10, math.fma(self.m10, other.m11, math.fma(self.m20, other.m12, self.m30 * other.m13)))
    local nm11 = math.fma(self.m01, other.m10, math.fma(self.m11, other.m11, math.fma(self.m21, other.m12, self.m31 * other.m13)))
    local nm12 = math.fma(self.m02, other.m10, math.fma(self.m12, other.m11, math.fma(self.m22, other.m12, self.m32 * other.m13)))
    local nm13 = math.fma(self.m03, other.m10, math.fma(self.m13, other.m11, math.fma(self.m23, other.m12, self.m33 * other.m13)))
    local nm20 = math.fma(self.m00, other.m20, math.fma(self.m10, other.m21, math.fma(self.m20, other.m22, self.m30 * other.m23)))
    local nm21 = math.fma(self.m01, other.m20, math.fma(self.m11, other.m21, math.fma(self.m21, other.m22, self.m31 * other.m23)))
    local nm22 = math.fma(self.m02, other.m20, math.fma(self.m12, other.m21, math.fma(self.m22, other.m22, self.m32 * other.m23)))
    local nm23 = math.fma(self.m03, other.m20, math.fma(self.m13, other.m21, math.fma(self.m23, other.m22, self.m33 * other.m23)))
    local nm30 = math.fma(self.m00, other.m30, math.fma(self.m10, other.m31, math.fma(self.m20, other.m32, self.m30 * other.m33)))
    local nm31 = math.fma(self.m01, other.m30, math.fma(self.m11, other.m31, math.fma(self.m21, other.m32, self.m31 * other.m33)))
    local nm32 = math.fma(self.m02, other.m30, math.fma(self.m12, other.m31, math.fma(self.m22, other.m32, self.m32 * other.m33)))
    local nm33 = math.fma(self.m03, other.m30, math.fma(self.m13, other.m31, math.fma(self.m23, other.m32, self.m33 * other.m33)))
    local result = mat4()
    result.m00 = nm00
    result.m01 = nm01
    result.m02 = nm02
    result.m03 = nm03
    result.m10 = nm10
    result.m11 = nm11
    result.m12 = nm12
    result.m13 = nm13
    result.m20 = nm20
    result.m21 = nm21
    result.m22 = nm22
    result.m23 = nm23
    result.m30 = nm30
    result.m31 = nm31
    result.m32 = nm32
    result.m33 = nm33
    return result
end

---@param other mat4
function mat4:__mul(other)
    return self:mul(other)
end

---@param other vec3
---@return vec3 # transformed vector
function mat4:transform(other)
    local x = other.x
    local y = other.y
    local z = other.z
    local nx = math.fma(self.m00, x, math.fma(self.m10, y, math.fma(self.m20, z, self.m30)))
    local ny = math.fma(self.m01, x, math.fma(self.m11, y, math.fma(self.m21, z, self.m31)))
    local nz = math.fma(self.m02, x, math.fma(self.m12, y, math.fma(self.m22, z, self.m32)))
    return vec3(nx, ny, nz)
end

---@param other vec3
---@return mat4
function mat4:translate(other)
    return self * mat4.translation(other)
end

---@param translation vec3
---@return mat4
function mat4.translation(translation)
    local m = mat4()
    m.m30 = translation.x
    m.m31 = translation.y
    m.m32 = translation.z
    return m
end

---@param scale vec3
---@return mat4
function mat4:scale(scale)
    self.m00 = self.m00 * scale.x
    self.m01 = self.m01 * scale.x
    self.m02 = self.m02 * scale.x
    self.m03 = self.m03 * scale.x
    self.m10 = self.m10 * scale.y
    self.m11 = self.m11 * scale.y
    self.m12 = self.m12 * scale.y
    self.m13 = self.m13 * scale.y
    self.m20 = self.m20 * scale.z
    self.m21 = self.m21 * scale.z
    self.m22 = self.m22 * scale.z
    self.m23 = self.m23 * scale.z
    return self
end

---@param scale vec3
---@return mat4
function mat4.scaling(scale)
        local m = mat4()
        m.m00 = scale.x
        m.m11 = scale.y
        m.m22 = scale.z
        return m
end

---@param rotation quat
---@return mat4
function mat4:rotate(rotation)
    return self * mat4.rotation(rotation)
end

---@param rotation quat
---@return mat4
function mat4.rotation(rotation)
    return mat4(rotation)
end

---@return mat4 # transposed matrix
function mat4:transposed()
    local result = mat4()
    result.m00 = self.m00
    result.m01 = self.m10
    result.m02 = self.m20
    result.m03 = self.m30
    result.m10 = self.m01
    result.m11 = self.m11
    result.m12 = self.m21
    result.m13 = self.m31
    result.m20 = self.m02
    result.m21 = self.m12
    result.m22 = self.m22
    result.m23 = self.m32
    result.m30 = self.m03
    result.m31 = self.m13
    result.m32 = self.m23
    result.m33 = self.m33
    return result
end

---@return quat
function mat4:to_quaternion()
    local tr = self.m00 + self.m11 + self.m22
    local quat = quat()

    if tr > 0 then
        local S = math.sqrt(tr + 1.0) * 2
        quat.w = 0.25 * S
        quat.x = (self.m21 - self.m12) / S
        quat.y = (self.m02 - self.m20) / S
        quat.z = (self.m10 - self.m01) / S
    elseif ((self.m00 > self.m11) and (self.m00 > self.m22)) then
        local S = math.sqrt(1.0 + self.m00 - self.m11 - self.m22) * 2
        quat.w = (self.m21 - self.m12) / S
        quat.x = 0.25 * S
        quat.y = (self.m01 + self.m10) / S
        quat.z = (self.m02 + self.m20) / S
    elseif self.m11 > self.m22 then
        local S = math.sqrt(1.0 + self.m11 - self.m00 - self.m22) * 2
        quat.w = (self.m02 - self.m20) / S
        quat.x = (self.m01 + self.m10) / S
        quat.y = 0.25 * S
        quat.z = (self.m12 + self.m21) / S
    else
        local S = math.sqrt(1.0 + self.m22 - self.m00 - self.m11) * 2
        quat.w = (self.m10 - self.m01) / S
        quat.x = (self.m02 + self.m20) / S
        quat.y = (self.m12 + self.m21) / S
        quat.z = 0.25 * S
    end

    return quat
end

function mat4:print(print_func)
    print_func = print_func or println
    for x = 0, 3 do
        local line = ""
        for y = 0, 3 do
            local key = "m" .. x .. y
            local value = self[key]
            line = line .. key .. "=" .. value .. " "
        end
        print_func(line)
    end
end

function mat4:__index(name)
    if type(name) == "number" then
        name = components[name]
    end

    return rawget(self, name) or rawget(mat4, name)
end

function mat4:__newindex(key, value)
    if type(key) == "number" then
        key = mat4.components[key]
    end

    rawset(self, key, value)
end

function mat4:__call(...)
    local args = { ... }
    return mat4.new(unpack(args))
end

function mat4:__tostring()
    local str = ""

    for x = 0, 3 do
        local line = ""
        for y = 0, 3 do
            local key = "m" .. x .. y
            local value = self[key]
            line = line .. key .. "=" .. value .. " "
        end

        str = str .. line .. "\n"
    end

    return str
end

setmetatable(mat4, mat4)
)";

	return sources;
}
