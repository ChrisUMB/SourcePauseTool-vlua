#pragma once

#include "../lua_feature.hpp"

class LuaPortalLibrary : public LuaLibrary {
public:
    explicit LuaPortalLibrary();

    void Load(lua_State* L) override;

    const std::string& GetLuaSource() override;
};

extern LuaPortalLibrary lua_portal_library;
