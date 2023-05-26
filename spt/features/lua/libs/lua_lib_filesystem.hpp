#pragma once

#include "../lua_feature.hpp"

class LuaFileSystemLibrary : public LuaLibrary
{
public:
	explicit LuaFileSystemLibrary();

	void Load(lua_State* L) override;

	const std::string& GetLuaSource() override;

    void Unload(lua_State *L) override;
};

extern LuaFileSystemLibrary lua_filesystem_library;