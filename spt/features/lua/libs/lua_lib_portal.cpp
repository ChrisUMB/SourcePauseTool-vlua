#include "stdafx.hpp"
#include "lua_lib_portal.hpp"
#include "ent_utils.hpp"
#include "lua_lib_math.hpp"
#include "../../ent_props.hpp"
#include "basehandle.h"
#include "interfaces.hpp"

LuaPortalLibrary lua_portal_library;

// This is duplicated code from lua_lib_entity.cpp, wahh wahh wahh
static IServerEntity* LuaCheckPortalEntity(lua_State* L, int index) {
    if (!LuaIsClass(L, index, "portal")) {
        luaL_error(L, "entity expected but the metatable is not correct");
        return nullptr;
    }

    lua_getfield(L, index, "data");
    auto client_ent = (IClientEntity*)lua_touserdata(L, -1);
    lua_pop(L, 1);

    if (!client_ent) {
        luaL_error(L, "entity expected but pointer is nil");
        return nullptr;
    }

    auto server_ent = utils::GetServerEntity((IClientEntity*)client_ent);

    if (!server_ent) {
        luaL_error(L, "server entity expected, but the pointer is nil");
        return nullptr;
    }

    return server_ent;
}

LuaPortalLibrary::LuaPortalLibrary() : LuaLibrary("portal") {}

static int PortalList(lua_State* L) {
    lua_newtable(L);

    int portals = 1;

    for (int i = 0; i < MAX_EDICTS; ++i) {
        IClientEntity* ent = utils::GetClientEntity(i);

        if (!ent || strcmp(ent->GetClientClass()->GetName(), "CProp_Portal") != 0) {
            continue;
        }

        lua_pushinteger(L, portals);
        lua_newtable(L);
        lua_getglobal(L, "portal");
        lua_setmetatable(L, -2);

        lua_pushlightuserdata(L, ent);
        lua_setfield(L, -2, "data");

        lua_settable(L, -3);
        portals++;
    }

    return 1;
}

static int PortalFromEntity(lua_State* L) {
    if (!lua_istable(L, 1)) {
        lua_pushnil(L);
        return 1;
    }

    lua_getfield(L, 1, "data");
    auto* ent = static_cast<IClientEntity*>(lua_touserdata(L, -1));
    lua_pop(L, 1);

    if (!ent || !strcmp(ent->GetClientClass()->GetName(), "CProp_Portal")) {
        lua_pushnil(L);
        return 1;
    }

    lua_newtable(L);
    lua_getglobal(L, "portal");
    lua_setmetatable(L, -2);

    lua_pushlightuserdata(L, ent);
    lua_setfield(L, -2, "data");
    return 1;
}

static int PortalGetEntity(lua_State* L) {
    auto ent = LuaCheckPortalEntity(L, 1);

    lua_newtable(L);
    lua_getglobal(L, "entity");
    lua_setmetatable(L, -2);

    lua_pushlightuserdata(L, ent);
    lua_setfield(L, -2, "data");
    return 1;
}

static int PortalIsActive(lua_State* L) {
    auto portal_ent = LuaCheckPortalEntity(L, 1);
    static int offset = spt_entprops.GetFieldOffset("CProp_Portal", "m_bActivated", true);
    bool* is_activated = (bool*)((uintptr_t)portal_ent + offset);
    lua_pushboolean(L, *is_activated);
    return 1;
}

static int PortalGetMatrix(lua_State* L) {
    auto portal_ent = LuaCheckPortalEntity(L, 1);
    static int offset = spt_entprops.GetFieldOffset("CProp_Portal", "m_matrixThisToLinked", true);
    auto matrix = (VMatrix*)((uintptr_t)portal_ent + offset);
    if (!matrix) {
        lua_pushnil(L);
        return 1;
    }

    LuaMathLibrary::LuaPushMatrix(L, *matrix);
    return 1;
}

static int PortalGetLinked(lua_State* L) {
    auto portal_ent = LuaCheckPortalEntity(L, 1);
    static int offset = spt_entprops.GetFieldOffset("CProp_Portal", "m_hLinkedPortal", true);
    CBaseHandle linked_handle = *reinterpret_cast<CBaseHandle*>(reinterpret_cast<uintptr_t>(portal_ent) + offset);
    if (!linked_handle.IsValid()) {
        lua_pushnil(L);
        return 1;
    }

    edict_t* edict = interfaces::engine_server->PEntityOfEntIndex(linked_handle.GetEntryIndex());
    IServerEntity* ent = edict->GetIServerEntity();

    if (!ent) {
        lua_pushnil(L);
        return 1;
    }

    lua_newtable(L);
    lua_getglobal(L, "portal");
    lua_setmetatable(L, -2);

    lua_pushlightuserdata(L, ent);
    lua_setfield(L, -2, "data");
    return 1;
}

static int PortalGetLinkageID(lua_State* L) {
    auto portal_ent = LuaCheckPortalEntity(L, 1);
    static int offset = spt_entprops.GetFieldOffset("CProp_Portal", "m_iLinkageGroupID", true);
    auto linkage_id = reinterpret_cast<int*>(reinterpret_cast<uintptr_t>(portal_ent) + offset);

    if (!linkage_id) {
        lua_pushnil(L);
        return 1;
    }

    lua_pushinteger(L, *linkage_id);
    return 1;
}

static int PortalIsOrange(lua_State* L) {
    auto portal_ent = LuaCheckPortalEntity(L, 1);
    static int offset = spt_entprops.GetFieldOffset("CProp_Portal", "m_bIsPortal2", true);
    auto is_orange = reinterpret_cast<bool*>(reinterpret_cast<uintptr_t>(portal_ent) + offset);

    if (!is_orange) {
        lua_pushnil(L);
        return 1;
    }

    lua_pushboolean(L, *is_orange);
    return 1;
}

static const struct luaL_Reg portal_class[] = {
    {"list", PortalList},
    {"from_entity", PortalFromEntity},
    {"get_entity", PortalGetEntity},
    {"is_active", PortalIsActive},
    {"get_matrix", PortalGetMatrix},
    {"get_linked", PortalGetLinked},
    {"get_linkage_id", PortalGetLinkageID},
    {"is_orange", PortalIsOrange},
    {nullptr, nullptr}
};

void LuaPortalLibrary::Load(lua_State* L) {
    lua_new_class(L, "portal", portal_class);
}

const std::string& LuaPortalLibrary::GetLuaSource() {
    static const std::string source = R"(---@meta
---@class portal
portal = {}

---@return portal[] # A list of all portals.
function portal.list()
end

---@return portal? # The closest portal, or `nil` if there isn't one.
function portal.closest()
    local closest = entity.closest(function(e) return e:get_class_name() == "CProp_Portal" end)
    return portal.from_entity(closest)
end

---@param entity entity The portal's entity.
---@return portal? # The portal linked to this entity, or `nil` if it's not a portal.
function portal.from_entity(entity)
end

---@param entity_id number The portal's entity ID.
---@return portal? # The portal linked to the entity with this ID, or `nil` if it doesn't exist or isn't a portal.
function portal.from_id(entity_id)
end

---@return portal? # The blue portal. If there are multiple blue portals, it returns the first one found.
function portal.blue()
    local portals = portal.list()

    for _, portal in ipairs(portals) do
        --if not portal:get_entity():get_model_name():match("/portal2.mdl") then
        if portal:is_blue() then
            return portal
        end
    end

    return nil
end

---@return portal? # The orange portal. If there are multiple orange portals, it returns the first one found.
function portal.orange()
    local portals = portal.list()

    for _, portal in ipairs(portals) do
        --if portal:get_entity():get_model_name():match("/portal2.mdl") then
        if portal:is_orange() then
            return portal
        end
    end

    return nil
end

---@return number # The portal's linkage ID.
function portal:get_linkage_id()
end

---@return portal? # The portal's linked portal.
function portal:get_linked()
end

---@return mat4? # The portal's transformation matrix, or `nil` if the portal isn't linked.
function portal:get_matrix()
end

---@return boolean # Whether the portal is active.
function portal:is_active()
end

---@return boolean # Whether the portal is orange.
function portal:is_orange()
end

---@return boolean # Whether the portal is blue.
function portal:is_blue()
    return self:is_orange() == false
end

---@return entity # The portal's entity.
function portal:get_entity()
end
)";

    return source;
}
