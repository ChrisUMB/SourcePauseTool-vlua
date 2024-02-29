#pragma once

#include "../lua_feature.hpp"

class LuaMemoryLibrary : public LuaLibrary {
public:
    explicit LuaMemoryLibrary();

    void Load(lua_State* L) override;

    const std::string& GetLuaSource() override;
};

extern LuaMemoryLibrary lua_memory_library;
