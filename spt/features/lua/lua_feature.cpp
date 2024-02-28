#include "stdafx.hpp"
#include "lua_feature.hpp"
#include <filesystem>
#include <fstream>
#include <utility>
#include "../ent_props.hpp"
#include "../demo.hpp"
#include "interfaces.hpp"
#include "libs/lua_lib_console.hpp"
#include "libs/lua_lib_entity.hpp"
#include "libs/lua_lib_events.hpp"
#include "libs/lua_lib_game.hpp"
#include "libs/lua_lib_input.hpp"
#include "libs/lua_lib_math.hpp"
#include "libs/lua_lib_player.hpp"
#include "libs/lua_lib_render.hpp"
#include "libs/lua_lib_system.hpp"
#include "libs/lua_lib_filesystem.hpp"
#include "libs/lua_lib_portal.hpp"
#include "libs/lua_lib_hud.hpp"
#include "libs/lua_lib_camera.hpp"
#include "lua_commands.hpp"
#include "lua_util.hpp"
#include "signals.hpp"
#include "spt/sptlib-wrapper.hpp"
#include "spt/utils/game_detection.hpp"
#include "demofile/demoformat.h"

namespace fs = std::filesystem;

LuaFeature spt_lua;

bool LuaFeature::ShouldLoadFeature()
{
	return utils::DoesGameLookLikePortal() && utils::GetBuildNumber() >= 5135;
}

void LuaFeature::LoadFeature()
{
	InitDirectory();
	InitConcommandBase(spt_lua_run_command);
	InitConcommandBase(spt_lua_reset_command);

	RegisterLibrary(&lua_console_library);
	RegisterLibrary(&lua_system_library);
	RegisterLibrary(&lua_filesystem_library);
	RegisterLibrary(&lua_events_library);
	RegisterLibrary(&lua_input_library);
	RegisterLibrary(&lua_game_library);
	RegisterLibrary(&lua_math_library);
	RegisterLibrary(&lua_player_library);
	RegisterLibrary(&lua_entity_library);
	RegisterLibrary(&lua_render_library);
	RegisterLibrary(&lua_hud_library);
    lua_hud_library.Init();
	//    RegisterLibrary(&lua_camera_library);

	if (utils::DoesGameLookLikePortal())
	{
		RegisterLibrary(&lua_portal_library);
	}

	void (*tick)() = []()
	{
		static int ticks = 0;

		lua_events_library.InvokeEvent("tick",
		                               [](lua_State* L)
		                               {
			                               lua_newtable(L);
			                               lua_pushinteger(L, ++ticks);
			                               lua_setfield(L, -2, "tick");
		                               });
	};

	TickSignal.Connect(tick);

	void (*level_init)(const char*) = [](char const* map)
	{
		lua_events_library.InvokeEvent("level_init",
		                               [&](lua_State* L)
		                               {
			                               lua_newtable(L);
			                               lua_pushstring(L, map);
			                               lua_setfield(L, -2, "map");
		                               });
	};

	LevelInitSignal.Connect(level_init);

	void (*client_active)(edict_t*) = [](edict_t* entity)
	{ lua_events_library.InvokeEvent("client_active", [](lua_State* L) { lua_newtable(L); }); };

	ClientActiveSignal.Connect(client_active);

	if (OngroundSignal.Works)
	{
		void (*on_ground)(bool) = [](bool has_ground_entity)
		{
			static bool was_on_ground = false;

			if (was_on_ground && !has_ground_entity)
			{
				lua_events_library.InvokeEvent("player_ungrounded",
				                               [](lua_State* L) { lua_newtable(L); });
			}

			if (!was_on_ground && has_ground_entity)
			{
				lua_events_library.InvokeEvent("player_grounded",
				                               [](lua_State* L) { lua_newtable(L); });
			}

			was_on_ground = has_ground_entity;
		};

		OngroundSignal.Connect(on_ground);
	}

	if (DemoStartPlaybackSignal.Works)
	{
		void (*demo_start_playback)(const char*, bool as_time_demo) =
		    [](const char* filename, bool as_time_demo)
		{
			lua_events_library.InvokeEvent("demo_start",
			                               [&](lua_State* L)
			                               {
				                               lua_newtable(L);

				                               void** demoplayer = spt_demostuff.pDemoplayer;
				                               int* demo_file = ((int*)*demoplayer) + 1;
				                               demoheader_t demo_header =
				                                   *(demoheader_t*)(demo_file + 65);

				                               lua_pushstring(L, filename);
				                               lua_setfield(L, -2, "file_name");

				                               lua_pushinteger(L, demo_header.demoprotocol);
				                               lua_setfield(L, -2, "demo_protocol");

				                               lua_pushinteger(L, demo_header.networkprotocol);
				                               lua_setfield(L, -2, "network_protocol");

				                               lua_pushinteger(L, demo_header.playback_ticks);
				                               lua_setfield(L, -2, "playback_ticks");

				                               lua_pushinteger(L, demo_header.playback_frames);
				                               lua_setfield(L, -2, "playback_frames");

				                               lua_pushnumber(L, demo_header.playback_time);
				                               lua_setfield(L, -2, "playback_time");

				                               lua_pushstring(L, demo_header.mapname);
				                               lua_setfield(L, -2, "map_name");
			                               });
		};

		DemoStartPlaybackSignal.Connect(demo_start_playback);
	}

	if (DemoStopPlaybackSignal.Works)
	{
		void (*demo_stop_playback)() = []()
		{ lua_events_library.InvokeEvent("demo_stop", [](lua_State* L) { lua_newtable(L); }); };

		DemoStopPlaybackSignal.Connect(demo_stop_playback);
	}

	if (DemoUpdateSignal.Works)
	{
		void (*demo_stop_playback)(bool, int, float) = [](bool is_new_frame, int demo_tick, float demo_time)
		{
			lua_events_library.InvokeEvent("demo_tick",
			                               [&](lua_State* L)
			                               {
				                               lua_newtable(L);
				                               lua_pushinteger(L, demo_tick);
				                               lua_setfield(L, -2, "tick");
				                               lua_pushnumber(L, demo_time);
				                               lua_setfield(L, -2, "time");
			                               });
		};

		DemoUpdateSignal.Connect(demo_stop_playback);
	}
}

void LuaFeature::UnloadFeature() {}

void LuaFeature::InitDirectory()
{
	auto game_dir = std::string(interfaces::engine->GetGameDirectory());

	if (!fs::exists(game_dir + "\\lua"))
	{
		fs::create_directory(game_dir + "\\lua");
	}

	if (!fs::exists(game_dir + "\\lua\\libraries"))
	{
		fs::create_directory(game_dir + "\\lua\\libraries");
		//Perhaps install `vtas` as a default library?
	}

	if (!fs::exists(game_dir + "\\lua\\scripts"))
	{
		fs::create_directory(game_dir + "\\lua\\scripts");
	}

	if (!fs::exists(game_dir + "\\lua\\docs"))
	{
		fs::create_directory(game_dir + "\\lua\\docs");
	}
}

lua_State* LuaFeature::GetLuaState()
{
	if (global_state == nullptr)
	{
		global_state = luaL_newstate();

		if (!global_state)
		{
			Warning("Failed to create lua state\n");
			return nullptr;
		}

		luaL_openlibs(global_state);
		InitLuaState(global_state);
	}

	return global_state;
}

void LuaFeature::ResetLuaState()
{
	if (global_state == nullptr)
	{
		return;
	}

	UnloadLibraries(global_state);
	lua_close(global_state);
	global_state = nullptr;
}

void LuaFeature::InitLuaState(lua_State* L)
{
	if (!L)
	{
		return;
	}

	auto game_dir = std::string(interfaces::engine->GetGameDirectory());
	auto lib_dir_string = game_dir + "\\lua\\libraries\\";
	const char* lib_dir = lib_dir_string.c_str();

	//this needs to use the game directory and not just assume portal
	lua_pushstring(L, lib_dir);
	lua_setglobal(L, "LIB_DIR");

	LoadLibraries(L);
	lua_getglobal(L, "package");
	lua_getfield(L, -1, "cpath");
	std::string current_cpath = lua_tostring(L, -1);
	auto new_cpath = current_cpath + ";" + lib_dir + "?.dll";
	lua_pop(L, 1);
	lua_pushstring(L, new_cpath.c_str());
	lua_setfield(L, -2, "cpath");
	lua_pop(L, 1);

	lua_getglobal(L, "package");
	lua_getfield(L, -1, "path");
	std::string current_path = lua_tostring(L, -1);
	auto new_path = current_path + ";" + lib_dir + "\\?.lua;" + lib_dir + "?.luac;";

	for (const auto& entry : std::filesystem::directory_iterator(lib_dir))
	{
		if (entry.is_directory())
		{
			auto path = entry.path().string();
			auto file_name = entry.path().filename().string();
			new_path += path + "\\" + file_name + ".lua;" + path + "\\" + file_name + ".luac;";
		}
	}

	lua_pop(L, 1);
	lua_pushstring(L, new_path.c_str());
	lua_setfield(L, -2, "path");
	lua_pop(L, 1);
}

void LuaFeature::RegisterLibrary(LuaLibrary* library, bool write_docs)
{
	this->libraries.push_back(library);

	const std::string& source = library->GetLuaSource();

	if (write_docs)
	{
		auto game_dir = std::string(interfaces::engine->GetGameDirectory());
		std::string file_path = game_dir + "\\lua\\docs\\" + library->name + ".lua";
		std::ofstream out(file_path);
		out << RemoveFunctionBodies(source);
		out.close();
	}

	// We do this so if a feature asks for the lua state before other libraries are loaded, those other libraries will be loaded into that lua state.
	ResetLuaState();
}

void LuaFeature::LoadLibraries(lua_State* L)
{
	for (const auto& item : this->libraries)
	{
		int top_start = lua_gettop(L);

		const std::string& source = item->GetLuaSource();
		if (!source.empty())
		{
			luaL_loadstring(L, source.c_str());
			if (lua_pcall(L, 0, 0, 0) != 0)
			{
				const char* error = lua_tostring(L, -1);
				if (error != nullptr)
				{
					Warning("Lua error loading library \"%s\": %s", item->name.c_str(), error);
					continue;
				}
				else
				{
					Warning("Lua error loading library \"%s\": Unknown error", item->name.c_str());
					continue;
				}
			}
		}

		item->Load(L);
		int top_end = lua_gettop(L);

		if (top_start != top_end)
		{
			Warning("Lua library \"%s\" did not clean up the stack properly! (start: %d, end: %d)",
			        item->name.c_str(),
			        top_start,
			        top_end);
		}
	}
}

void LuaFeature::UnloadLibraries(lua_State* L)
{
	for (const auto& item : this->libraries)
	{
		item->Unload(L);
	}
}

LuaLibrary::LuaLibrary(std::string name) : name(std::move(name)) {}

void LuaLibrary::Load(lua_State* L) {}

void LuaLibrary::Unload(lua_State* L)
{
	lua_pushnil(L);
	lua_setglobal(L, this->name.c_str());
}

const std::string& LuaLibrary::GetLuaSource()
{
	static std::string default_sources;
	return default_sources;
}

//Hooks

namespace patterns
{
	PATTERNS(TeleportTouchingEntity, "5135", "81 EC ?? ?? ?? ?? 55 8B E9 89 6C 24 14 E8 ?? ?? ?? ??");
	PATTERNS(PortalNewLocation,
	         "5135",
	         "83 EC 2C 53 55 56 8B F1 57"); //8D 8E ?? ?? ?? ?? E8 ?? ?? ?? ?? 8B 06 8B 90");
	PATTERNS(TriggerStartTouch, "5135", "55 8B 6C 24 08 56 8B F1 8B 06 8B 90");
	PATTERNS(GetPortalCallQueue, "5135", "33 C0 39 05 ?? ?? ?? ?? 0F 9E C0 83 E8 01 25 ?? ?? ?? ?? C3");

    PATTERNS(NET_RunFrame, "5135", "DD 44 24 04 DD 05 ?? ?? ?? ?? D8 E9 D9 C9 DD 1D ?? ?? ?? ?? D9 E8 D9 C9 DB F1 77 0C DD D9 D9 EE DB F1 76 04 DD D9 EB 02 DD D8 A1 ?? ?? ?? ?? D8 48 2C DC 05 ?? ?? ?? ?? DD 1D ?? ?? ?? ?? E8");
    PATTERNS(ClientDLL_FrameStageNotify, "5135", "8B 0D ?? ?? ?? ?? 85 C9 74 0F 8B 01 8B 54")

} // namespace patterns

void LuaFeature::InitHooks()
{
	FIND_PATTERN(server, GetPortalCallQueue);

	HOOK_FUNCTION(server, TeleportTouchingEntity);
	HOOK_FUNCTION(server, PortalNewLocation);
	HOOK_FUNCTION(server, TriggerStartTouch);
    HOOK_FUNCTION(engine, NET_RunFrame);
    HOOK_FUNCTION(engine, ClientDLL_FrameStageNotify);
}

void LuaFeature::HOOKED_TriggerStartTouch(void* thisptr, int _edx, void* other)
{
	spt_lua.ORIG_TriggerStartTouch(thisptr, other);

	if (other == nullptr)
	{
		return;
	}

	bool is_player = other == spt_entprops.GetPlayer(true);

	auto event_invocation = [&](lua_State* L)
	{
		lua_newtable(L);

		LuaEntityLibrary::LuaPushEntity(L, thisptr);
		lua_setfield(L, -2, "trigger");

		LuaEntityLibrary::LuaPushEntity(L, other);
		lua_setfield(L, -2, "entity");
	};

	lua_events_library.InvokeEvent("entity_touch_trigger", event_invocation);

	if (is_player)
	{
		lua_events_library.InvokeEvent("player_touch_trigger", event_invocation);
	}
}

void __fastcall LuaFeature::HOOKED_PortalNewLocation(void* thisptr, int _edx, Vector& origin, QAngle& angles)
{
	Vector* p_pos =
	    (Vector*)((uintptr_t)thisptr + spt_entprops.GetFieldOffset("CBaseEntity", "m_vecAbsOrigin", true));

	QAngle* p_ang =
	    (QAngle*)((uintptr_t)thisptr + spt_entprops.GetFieldOffset("CBaseEntity", "m_angAbsRotation", true));

	Vector& old_pos = *p_pos;
	QAngle& old_ang = *p_ang;

	spt_lua.ORIG_PortalNewLocation(thisptr, origin, angles);
	lua_events_library.InvokeEvent("portal_moved",
	                               [&](lua_State* L)
	                               {
		                               lua_newtable(L);

		                               LuaMathLibrary::LuaPushVector3D(L, old_pos);
		                               lua_setfield(L, -2, "old_pos");

		                               LuaMathLibrary::LuaPushAngle(L, old_ang);
		                               lua_setfield(L, -2, "old_ang");

		                               LuaMathLibrary::LuaPushVector3D(L, origin);
		                               lua_setfield(L, -2, "new_pos");

		                               LuaMathLibrary::LuaPushAngle(L, angles);
		                               lua_setfield(L, -2, "new_ang");
	                               });
}

void __fastcall LuaFeature::HOOKED_TeleportTouchingEntity(void* thisptr, int _edx, void* other)
{
	if (spt_lua.ORIG_GetPortalCallQueue())
	{
		spt_lua.ORIG_TeleportTouchingEntity(thisptr, other);
		return;
	}

	int hammer_id = (int)((uintptr_t)other + spt_entprops.GetFieldOffset("CBaseEntity", "m_iHammerID", true));

	Vector* p_pos =
	    (Vector*)((uintptr_t)other + spt_entprops.GetFieldOffset("CBaseEntity", "m_vecAbsOrigin", true));

	Vector* p_rot =
	    (Vector*)((uintptr_t)other + spt_entprops.GetFieldOffset("CBaseEntity", "m_angAbsRotation", true));

	QAngle* p_ang = (QAngle*)((uintptr_t)other + spt_entprops.GetFieldOffset("CBaseEntity", "m_angRotation", true));

	bool is_player = other == spt_entprops.GetPlayer(true);

	Vector old_pos = *p_pos;
	Vector old_rot = *p_rot;
	QAngle old_ang = *p_ang;

	spt_lua.ORIG_TeleportTouchingEntity(thisptr, other);

	Vector new_pos = *p_pos;
	Vector new_rot = *p_rot;
	QAngle new_ang = *p_ang;

	if (is_player)
	{
		lua_player_library.UpdateLocals(old_pos, old_ang, new_pos, new_ang);
	}

	auto event_invocation = [&](lua_State* L)
	{
		lua_newtable(L);

		LuaEntityLibrary::LuaPushEntity(L, other);
		lua_setfield(L, -2, "entity");

		LuaMathLibrary::LuaPushVector3D(L, old_pos);
		lua_setfield(L, -2, "old_pos");

		LuaMathLibrary::LuaPushVector3D(L, old_rot);
		lua_setfield(L, -2, "old_rot");

		LuaMathLibrary::LuaPushAngle(L, old_ang);
		lua_setfield(L, -2, "old_ang");

		LuaMathLibrary::LuaPushVector3D(L, new_pos);
		lua_setfield(L, -2, "new_pos");

		LuaMathLibrary::LuaPushVector3D(L, new_rot);
		lua_setfield(L, -2, "new_rot");

		LuaMathLibrary::LuaPushAngle(L, new_ang);
		lua_setfield(L, -2, "new_ang");
	};

	lua_events_library.InvokeEvent("entity_teleport", event_invocation);

	if (is_player)
	{
		lua_events_library.InvokeEvent("player_teleport", event_invocation);
	}
}

int LuaFeature::HOOKED_NET_RunFrame(double time) {
    lua_events_library.InvokeEvent("net_runframe", [](lua_State* L) {
        lua_newtable(L);
    });

    spt_lua.ORIG_NET_RunFrame(time);
}

void LuaFeature::HOOKED_ClientDLL_FrameStageNotify(int stage) {
    if(stage != 6) {
        spt_lua.ORIG_ClientDLL_FrameStageNotify(stage);
        return;
    }

    static int lastWidth = 0;
    static int lastHeight = 0;
    static uint8_t* pImage = nullptr;

    auto engine_dll = (DWORD)GetModuleHandle("engine.dll");
    //3B2A18
    auto pCVideoMode = *(DWORD**)(engine_dll + 0x3B2A18);
    auto pCVideoModeVTable = *(DWORD**)pCVideoMode;

    // 56 -> GetModeWidth()
    // 60 -> GetModeHeight()
    // 108 -> ReadScreenPixels()
    typedef int(__thiscall* pCVideoMode_GetModeWidth_t)(void*);
    typedef int(__thiscall* pCVideoMode_GetModeHeight_t)(void*);
    typedef void(__thiscall* pCVideoMode_ReadScreenPixels_t)(void*, int, int, int, int, void*, int);

    auto pCVideoMode_GetModeWidth = (pCVideoMode_GetModeWidth_t)pCVideoModeVTable[14];
    auto pCVideoMode_GetModeHeight = (pCVideoMode_GetModeHeight_t)pCVideoModeVTable[15];
    auto pCVideoMode_ReadScreenPixels = (pCVideoMode_ReadScreenPixels_t)pCVideoModeVTable[27];

    auto width = pCVideoMode_GetModeWidth(pCVideoMode);
    auto height = pCVideoMode_GetModeHeight(pCVideoMode);

    if(pImage == nullptr || lastWidth != width || lastHeight != height) {
        delete[] pImage;
        pImage = new uint8_t[width * height * 4];
        lastWidth = width;
        lastHeight = height;
    }

//    auto* pImage = new uint8_t[width * height * 3];
//    pCVideoMode_ReadScreenPixels(pCVideoMode, 0, 0, width, height, pImage, 2 /* IMAGE_FORMAT_RGB888 */);
    pCVideoMode_ReadScreenPixels(pCVideoMode, 0, 0, width, height, pImage, 12 /* IMAGE_FORMAT_BGRA8888 */);
    ImageFormat backBufferFormat = interfaces::materialSystem->GetBackBufferFormat();
    ConMsg("ReadScreenPixels: %d %d %d (%d)\n", pImage[0], pImage[1], pImage[2], backBufferFormat);
//    delete[] pImage;

    spt_lua.ORIG_ClientDLL_FrameStageNotify(stage);
//    lua_State *state = spt_lua.global_state;
//    lua_pushinteger(state, pImage[0]);
//    lua_setglobal(state, "pixel_r");
//    lua_pushinteger(state, pImage[1]);
//    lua_setglobal(state, "pixel_g");
//    lua_pushinteger(state, pImage[2]);
//    lua_setglobal(state, "pixel_b");
}