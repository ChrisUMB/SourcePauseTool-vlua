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

	using _PortalNewLocation = void(__thiscall*)(void* thisptr, Vector& origin, QAngle& angles);
	_PortalNewLocation ORIG_PortalNewLocation = nullptr;
	static void __fastcall HOOKED_PortalNewLocation(void* thisptr, int _edx, Vector& origin, QAngle& angles);

	using _TriggerStartTouch = void(__thiscall*)(void* thisptr, void* other);
	_TriggerStartTouch ORIG_TriggerStartTouch = nullptr;
	static void __fastcall HOOKED_TriggerStartTouch(void* thisptr, int _edx, void* other);

	using _GetPortalCallQueue = bool (*)();
	_GetPortalCallQueue ORIG_GetPortalCallQueue = nullptr;

	using _NET_RunFrame = int(__cdecl*)(double);
	_NET_RunFrame ORIG_NET_RunFrame = nullptr;
	static int __cdecl HOOKED_NET_RunFrame(double time);

    using _ClientDLL_FrameStageNotify = void(*)(int);
    _ClientDLL_FrameStageNotify ORIG_ClientDLL_FrameStageNotify = nullptr;
    static void HOOKED_ClientDLL_FrameStageNotify(int stage);
};

extern LuaFeature spt_lua;


// TODO: All of this should be... somewhere else.
enum class HostState : int
{
	NEW_GAME = 0,
	LOAD_GAME,
	CHANGE_LEVEL_SP,
	CHANGE_LEVEL_MP,
	RUN,
	GAME_SHUTDOWN,
	SHUTDOWN,
	RESTART,
};

enum class ServerState : int
{
	DEAD = 0, // Dead
	LOADING, // Spawning
	ACTIVE, // Running
	PAUSED, // Running, but paused
};

enum class ClientSignonState : int
{
	NONE,
	CHALLENGE,
	CONNECTED,
	NEW,
	PRESPAWN,
	SPAWN,
	FULL,
	CHANGELEVEL
};

ServerState GetServerState();
ClientSignonState GetClientState();
HostState GetHostState();

bool IsGamePaused();
bool IsGameSimulating();