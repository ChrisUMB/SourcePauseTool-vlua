#pragma once

#include "../lua_feature.hpp"

class LuaSystemLibrary : public LuaLibrary
{
public:
	explicit LuaSystemLibrary();

	void Load(lua_State* L) override;

	const std::string& GetLuaSource() override;
};

extern LuaSystemLibrary lua_system_library;