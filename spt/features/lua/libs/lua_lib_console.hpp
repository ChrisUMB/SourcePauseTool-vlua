#pragma once

#include "../lua_feature.hpp"

class LuaConsoleLibrary : public LuaLibrary {
public:
    explicit LuaConsoleLibrary();

    void Load(lua_State* L) override;

    void Unload(lua_State* L) override;

    const std::string& GetLuaSource() override;
};

extern LuaConsoleLibrary lua_console_library;
