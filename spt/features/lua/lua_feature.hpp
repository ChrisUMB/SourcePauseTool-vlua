#pragma once

#include "../../feature.hpp"
#include "spt/features/lua/lua_util.hpp"
#include <string>
#include <vector>
#include "lua.hpp"

class LuaLibrary
{
public:
	const std::string name;

	explicit LuaLibrary(std::string name);

	virtual void Load(lua_State* L);

	virtual void Unload(lua_State* L);

	virtual const std::string& GetLuaSource();
};

class LuaFeature : public FeatureWrapper<LuaFeature>
{
public:
	lua_State* GetLuaState();

	void ResetLuaState();

	void InitLuaState(lua_State* L);

	void RegisterLibrary(LuaLibrary* library, bool write_docs = true);

	void LoadLibraries(lua_State* L);

	void UnloadLibraries(lua_State* L);

	//    void Execute(const std::string &code);

	//    void ExecuteFile(const std::string &path);

protected:
	bool ShouldLoadFeature() override;

	void InitHooks() override;

	void LoadFeature() override;

	void UnloadFeature() override;

private:
	lua_State* global_state = nullptr;

	std::vector<LuaLibrary*> libraries;

	static void InitDirectory();

	using _TeleportTouchingEntity = void(__thiscall*)(void* thisptr, void*);
	_TeleportTouchingEntity ORIG_TeleportTouchingEntity = nullptr;
	static void __fastcall HOOKED_TeleportTouchingEntity(void* thisptr, int _edx, void* other);

    using _PortalNewLocation = void(__thiscall*)(void* thisptr, Vector &origin, QAngle &angles);
    _PortalNewLocation ORIG_PortalNewLocation = nullptr;
    static void __fastcall HOOKED_PortalNewLocation(void* thisptr, int _edx, Vector &origin, QAngle &angles);

    using _TriggerStartTouch = void(__thiscall*)(void* thisptr, void* other);
    _TriggerStartTouch ORIG_TriggerStartTouch = nullptr;
    static void __fastcall HOOKED_TriggerStartTouch(void* thisptr, int _edx, void* other);

	using _GetPortalCallQueue = bool (*)();
	_GetPortalCallQueue ORIG_GetPortalCallQueue = nullptr;
};

extern LuaFeature spt_lua;