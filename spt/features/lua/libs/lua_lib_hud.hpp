#pragma once

#include "../lua_feature.hpp"

class LuaHudLibrary : public LuaLibrary
{
public:
    explicit LuaHudLibrary();

    void Init();

    void Load(lua_State* L) override;

    void Unload(lua_State* L) override;

    const std::string& GetLuaSource() override;
};

extern LuaHudLibrary lua_hud_library;
