#pragma once

#include "../lua_feature.hpp"

class LuaPlayerLibrary : public LuaLibrary {
public:
    explicit LuaPlayerLibrary();

    void Load(lua_State *L) override;

    const std::string& GetLuaSource() override;
};

extern LuaPlayerLibrary lua_player_library;