#include "stdafx.hpp"
#include "lua_lib_entity.hpp"
#include "interfaces.hpp"
#include "ent_utils.hpp"
#include "../../ent_props.hpp"
#include "lua_lib_math.hpp"

LuaEntityLibrary lua_entity_library;

#define LUA_GET_ENTITY() \
	LuaEntityLibrary::LuaCheckEntity(L, 1); \
	if (entity == nullptr) \
	{ \
		lua_pushnil(L); \
		return 1; \
	}

LuaEntityLibrary::LuaEntityLibrary() : LuaLibrary("entity") {}

void LuaEntityLibrary::Teleport(void* entity, const Vector* pos, const QAngle* ang, const Vector* vel)
{
	int* p_vtable = reinterpret_cast<int*>(entity);
	(*(void(__thiscall**)(void*, const Vector*, const QAngle*, const Vector*))(*p_vtable
	                                                                           + 420))(entity, pos, ang, vel);
}

static int EntityFromID(lua_State* L)
{
	int id = luaL_checkinteger(L, 1);

	void* entity = interfaces::server_tools->GetIServerEntity(interfaces::entList->GetClientEntity(id));

	if (entity == nullptr)
	{
		lua_pushnil(L);
		return 1;
	}

	lua_newtable(L);
	lua_getglobal(L, "entity");
	lua_setmetatable(L, -2);

	lua_pushlightuserdata(L, entity);
	lua_setfield(L, -2, "data");
	return 1;
}

static int EntityFromHammerID(lua_State* L)
{
	int hammer_id = luaL_checkinteger(L, 1);
	void* entity = interfaces::server_tools->FindEntityByHammerID(hammer_id);

	if (entity == nullptr)
	{
		lua_pushnil(L);
		return 1;
	}

	lua_newtable(L);
	lua_getglobal(L, "entity");
	lua_setmetatable(L, -2);

	lua_pushlightuserdata(L, entity);
	lua_setfield(L, -2, "data");
	return 1;
}

static int EntityList(lua_State* L)
{
	lua_newtable(L);

	int i = 1;
	for (int j = 0; j < MAX_EDICTS; ++j)
	{
		void* entity = interfaces::server_tools->GetIServerEntity(interfaces::entList->GetClientEntity(j));

		if (entity == nullptr)
		{
			continue;
		}

		lua_pushinteger(L, i++);
		lua_newtable(L);
		lua_getglobal(L, "entity");
		lua_setmetatable(L, -2);

		lua_pushlightuserdata(L, entity);
		lua_setfield(L, -2, "data");

		lua_settable(L, -3);
	}

	return 1;
}

static int EntityGetID(lua_State* L)
{
	void* entity = LUA_GET_ENTITY() lua_pushinteger(L, utils::GetIndex(entity));
	return 1;
}

static int EntityGetClassName(lua_State* L)
{
	void* entity = LUA_GET_ENTITY()
	    lua_pushstring(L,
	                   interfaces::entList->GetClientEntity(utils::GetIndex(entity))->GetClientClass()->GetName());
	return 1;
}

static int EntityGetModelName(lua_State* L)
{
	void* entity = LUA_GET_ENTITY()
	    lua_pushstring(L, utils::GetModelName(interfaces::entList->GetClientEntity(utils::GetIndex(entity))));
	return 1;
}

static int EntityGetPos(lua_State* L)
{
	void* entity = LUA_GET_ENTITY()

	    Vector* p_pos =
	        (Vector*)((uintptr_t)entity + spt_entprops.GetFieldOffset("CBaseEntity", "m_vecAbsOrigin", true));
	LuaMathLibrary::LuaPushVector3D(L, *p_pos);
	return 1;
}

static int EntitySetPos(lua_State* L)
{
	void* entity = LUA_GET_ENTITY()

	    if (!LuaMathLibrary::LuaIsVector3D(L, 2))
	{
		lua_pushnil(L);
		return 1;
	}

	Vector pos = LuaMathLibrary::LuaGetVector3D(L, 2);
	LuaEntityLibrary::Teleport(entity, &pos, nullptr, nullptr);
	return 0;
}

static int EntityGetRot(lua_State* L)
{
	void* entity = LUA_GET_ENTITY()

	    Vector* p_rot =
	        (Vector*)((uintptr_t)entity + spt_entprops.GetFieldOffset("CBaseEntity", "m_angAbsRotation", true));
	LuaMathLibrary::LuaPushVector3D(L, *p_rot);
	return 1;
}

static int EntitySetRot(lua_State* L)
{
	void* entity = LUA_GET_ENTITY()

	    if (!LuaMathLibrary::LuaIsAngle(L, 2))
	{
		lua_pushnil(L);
		return 1;
	}

	QAngle ang = LuaMathLibrary::LuaGetAngle(L, 2);
	LuaEntityLibrary::Teleport(entity, nullptr, &ang, nullptr);
	return 0;
}

static int EntityGetVel(lua_State* L)
{
	void* entity = LUA_GET_ENTITY()

	    Vector* p_vel =
	        (Vector*)((uintptr_t)entity + spt_entprops.GetFieldOffset("CBaseEntity", "m_vecAbsVelocity", true));

	LuaMathLibrary::LuaPushVector3D(L, *p_vel);
	return 1;
}

static int EntitySetVel(lua_State* L)
{
	void* entity = LUA_GET_ENTITY()

	    if (!LuaMathLibrary::LuaIsVector3D(L, 2))
	{
		lua_pushnil(L);
		return 1;
	}

	Vector vel = LuaMathLibrary::LuaGetVector3D(L, 2);
	LuaEntityLibrary::Teleport(entity, nullptr, nullptr, &vel);
	return 0;
}

static const struct luaL_Reg entity_class[] = {{"_list", EntityList},
                                               {"from_id", EntityFromID},
                                               {"from_hammer_id", EntityFromHammerID},
                                               {"get_id", EntityGetID},
                                               {"get_class_name", EntityGetClassName},
                                               {"get_model_name", EntityGetModelName},
                                               {"get_pos", EntityGetPos},
                                               {"set_pos", EntitySetPos},
                                               {"get_rot", EntityGetRot},
                                               {"set_rot", EntitySetRot},
                                               {"get_vel", EntityGetVel},
                                               {"set_vel", EntitySetVel},
                                               //        {"teleport",    EntityTeleport},
                                               {nullptr, nullptr}};

void* LuaEntityLibrary::LuaCheckEntity(lua_State* L, int index)
{
	if (!LuaIsClass(L, index, "entity"))
	{
		luaL_error(L, "entity expected");
		return nullptr;
	}

	lua_getfield(L, index, "data");
	void* entity_ptr = (void*)lua_touserdata(L, -1);
	lua_pop(L, 1);
	return entity_ptr;
}

void LuaEntityLibrary::Load(lua_State* L)
{
	lua_new_class(L, "entity", entity_class);
}

const std::string& LuaEntityLibrary::GetLuaSource()
{
	static const std::string sources = R"(---@meta
---@class entity
entity = {}

---@param id number Entity index
---@return entity # Entity instance
function entity.from_id(id)
end

---@param hammer_id number Entity hammer ID
---@return entity # Entity instance
function entity.from_hammer_id(hammer_id)
end

---@param filter fun(entity:entity):boolean|nil Filter function, or `nil` to not filter
---@overload fun():entity[]
---@return entity[] # List of all entities
function entity.list(filter)
    local list = entity._list()
    if not filter then return list end

    local filtered = {}
    for _, ent in ipairs(list) do
        if filter(ent) then
            table.insert(filtered, ent)
        end
    end

    return filtered
end

---@param filter (fun(entity:entity):boolean)|string|nil Filter function, or a `string` class name, or `nil` to not filter
---@overload fun():entity?
---@overload fun(class_name:string):entity?
---@return entity? # Closest entity, or `nil` if none found based on `filter`
function entity.closest(filter)
    local closest = nil
    local closest_dist = math.huge

    if type(filter) == "string" then
        local name = filter
        filter = function(e) return e:get_class_name() == name end
    end

    for _, ent in ipairs(entity.list(filter)) do
        local dist = ent:get_pos():distance_squared(player.get_pos())
        if dist < closest_dist then
            closest = ent
            closest_dist = dist
        end
    end

    return closest
end

---@return number # Entity ID
function entity:get_id()
end

---@return string # Entity class name
function entity:get_class_name()
end

---@return string # Entity model name
function entity:get_model_name()
end

---@return vec3 # Entity position
function entity:get_pos()
end

---@param pos vec3 Entity position
function entity:set_pos(pos)
end

---@return vec3 # Entity rotation
function entity:get_rot()
end

---@param rot vec3 Entity rotation
function entity:set_rot(rot)
end

---@return vec3 # Entity velocity
function entity:get_vel()
end

---@param vel vec3 Entity velocity
function entity:set_vel(vel)
end
)";

	return sources;
}
