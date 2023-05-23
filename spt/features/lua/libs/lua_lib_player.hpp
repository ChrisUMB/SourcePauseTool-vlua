#pragma once

#include "../lua_feature.hpp"

class LuaPlayerLibrary : public LuaLibrary
{
public:
	QAngle local_angle_offset;
	Vector local_position_offset;
	Vector local_position_origin;

	explicit LuaPlayerLibrary();

	void Load(lua_State* L) override;

	const std::string& GetLuaSource() override;

	Vector AsLocalPosition(const Vector& position);

	void UpdateLocals(const Vector& old_pos, const QAngle& old_ang, const Vector& new_pos, const QAngle& new_ang);
};

extern LuaPlayerLibrary lua_player_library;