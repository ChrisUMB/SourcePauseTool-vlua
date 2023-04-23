#pragma once

#include "../lua_feature.hpp"

class LuaSyscallLibrary : public LuaLibrary {
public:
    explicit LuaSyscallLibrary();

    void Load(lua_State *L) override;

    const std::string& GetLuaSource() override;
};

extern LuaSyscallLibrary lua_syscall_library;