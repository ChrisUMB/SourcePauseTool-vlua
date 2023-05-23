#pragma once

#include "../lua_feature.hpp"

class LuaRenderLibrary : public LuaLibrary
{
public:
	explicit LuaRenderLibrary();

	void Load(lua_State* L) override;

	void Unload(lua_State* L) override;

	const std::string& GetLuaSource() override;
};

extern LuaRenderLibrary lua_render_library;