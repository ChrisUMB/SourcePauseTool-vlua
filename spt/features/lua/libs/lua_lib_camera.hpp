#pragma once

#include "../lua_feature.hpp"

class LuaCameraLibrary : public LuaLibrary {
public:
    explicit LuaCameraLibrary();

    void Load(lua_State* L) override;

    void Unload(lua_State* L) override;

    const std::string& GetLuaSource() override;
};

extern LuaCameraLibrary lua_camera_library;
