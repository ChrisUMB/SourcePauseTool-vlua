#pragma once

#include "../lua_feature.hpp"

class LuaInputLibrary : public LuaLibrary {
public:
    explicit LuaInputLibrary();

    void Load(lua_State *L) override;

    const std::string &GetLuaSource() override;
};

extern LuaInputLibrary lua_input_library;