#pragma once

#include "../lua_feature.hpp"

class LuaEntityLibrary : public LuaLibrary {
public:
    explicit LuaEntityLibrary();

    void Load(lua_State* L) override;

    const std::string& GetLuaSource() override;

    static void* LuaCheckEntity(lua_State* L, int index);

    static void LuaPushEntity(lua_State* L, void* entity);

    // This should be elevated to a separate part of SPT or something, it's not really a lua specific thing
    static void Teleport(void* entity, const Vector* pos, const QAngle* ang, const Vector* vel);
};

extern LuaEntityLibrary lua_entity_library;
