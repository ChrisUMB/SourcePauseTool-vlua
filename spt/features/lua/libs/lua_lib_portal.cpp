#include "stdafx.hpp"
#include "lua_lib_portal.hpp"
#include "../features/ent_props.hpp"
#include "ent_utils.hpp"
#include "lua_lib_math.hpp"

LuaPortalLibrary::LuaPortalLibrary() : LuaLibrary("portal") {}

static int PortalGet(lua_State *L) {
    int index = lua_tointeger(L, 1);

    if (index != 1 && index != 2) {
        lua_pushnil(L);
        return 1;
    }

    IClientEntity *portal_ent = nullptr;

    for (int i = 0; i < MAX_EDICTS; ++i) {
        IClientEntity *ent = utils::GetClientEntity(i);

        if (!strcmp(ent->GetClientClass()->GetName(), "CProp_Portal")) {
            continue;
        }

        const char *model_name = utils::GetModelName(ent);

        if ((index == 2) == (strstr(model_name, "portal2") != nullptr)) {
            portal_ent = ent;
            break;
        }
    }

    int matrix_offset = spt_entprops.GetFieldOffset("CProp_Portal", "m_matrixThisToLinked", true);
    int linkage_offset = spt_entprops.GetFieldOffset("CProp_Portal", "m_iLinkageGroupID", true);

    auto matrix = *reinterpret_cast<VMatrix *>(portal_ent + matrix_offset);
    auto linkage_id = *reinterpret_cast<char *>(portal_ent + linkage_offset);

    LuaMathLibrary::LuaPushMatrix(L, matrix);
    lua_setfield(L, 2, "matrix");

    lua_pushinteger(L, linkage_id);
    lua_setfield(L, 2, "linkage_id");

}

static int PortalGetEntity(lua_State *L) {

}

static const struct luaL_Reg portals_class[] = {
        {"_get",        PortalGet},
        {"_get_entity", PortalGetEntity},
        {nullptr,       nullptr}
};

void LuaPortalLibrary::Load(lua_State *L) {
    luaL_register(L, "portals", portals_class);
    lua_pop(L, 1);
}

const std::string &LuaPortalLibrary::GetLuaSource() {
    static const std::string source = R"(
---@class portals
portals = {}

---@class portal
---@field linkage_id number The portal's linkage ID.
---@field matrix mat4? The portal's transformation matrix, or `nil` if the portal isn't linked.
local portal = {}
portal.__index = portal

---@param index number The index of the portal to get.
---@return portal? The portal instance, or `nil` if it's not placed.
function portals.get(index)
    local p = { index = index }
    setmetatable(p, portal)
    return portals._get(index, p)
end

---@return portal? The blue portal, or `nil` if it isn't placed.
function portals.blue()
    return portals.get(1)
end

---@return portal? The orange portal, or `nil` if it isn't placed.
function portals.orange()
    return portals.get(2)
end

---@return entity The entity that the portal is linked to.
function portal:get_entity()
    return portals._get_entity(self)
end

)";

    return source;
}
