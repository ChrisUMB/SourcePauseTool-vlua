#pragma once

#include "../lua_feature.hpp"

class LuaGameLibrary : public LuaLibrary {
public:
    explicit LuaGameLibrary();

    void Load(lua_State* L) override;

    const std::string& GetLuaSource() override;
};

extern LuaGameLibrary lua_game_library;
