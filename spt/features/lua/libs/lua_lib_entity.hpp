#pragma once

#include "../lua_feature.hpp"

class LuaEntityLibrary : public LuaLibrary {
public:
    explicit LuaEntityLibrary();

    void Load(lua_State *L) override;

    const std::string& GetLuaSource() override;

    static void *LuaCheckEntity(lua_State *L, int index);
};

extern LuaEntityLibrary lua_entity_library;