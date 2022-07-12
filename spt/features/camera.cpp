#include "stdafx.h"
#include "camera.hpp"
#include "playerio.hpp"
#include "interfaces.hpp"
#include "..\sptlib-wrapper.hpp"
#include "..\cvars.hpp"

#ifdef OE
#include "..\game_shared\usercmd.h"
#else
#include "usercmd.h"
#endif

#include <chrono>

Camera spt_camera;

ConVar y_spt_cam_control(
    "y_spt_cam_control",
    "0",
    FCVAR_CHEAT,
    "Camera is separated and can be controlled by user input. (Requires sv_cheats 1 if not playing demo)\n");
ConVar y_spt_cam_drive("y_spt_cam_drive", "1", FCVAR_CHEAT, "Enables or disables camera drive mode.\n");

CON_COMMAND(y_spt_cam_setpos, "y_spt_cam_setpos <x> <y> <z> - Sets camera position (requires camera Drive Mode)\n")
{
	if (args.ArgC() != 4)
	{
		Msg("Usage: y_spt_cam_setpos <x> <y> <z>;\n");
		return;
	}
	if (!y_spt_cam_control.GetBool())
	{
		Msg("Requires camera Drive Mode.\n");
		return;
	}
	Vector pos = {0.0, 0.0, 0.0};
	for (int i = 0; i < args.ArgC() - 1; i++)
	{
		pos[i] = atof(args.Arg(i + 1));
	}
	spt_camera.cam_origin = pos;
}

CON_COMMAND(y_spt_cam_setang,
            "y_spt_cam_setang <pitch> <yaw> [roll] - Sets camera angles (requires camera Drive Mode)\n")
{
	if (args.ArgC() != 3 && args.ArgC() != 4)
	{
		Msg("Usage: y_spt_cam_setang <pitch> <yaw> [roll];\n");
		return;
	}
	if (!y_spt_cam_control.GetBool())
	{
		Msg("Requires camera Drive Mode.\n");
		return;
	}
	QAngle ang = {0.0, 0.0, 0.0};
	for (int i = 0; i < args.ArgC() - 1; i++)
	{
		ang[i] = atof(args.Arg(i + 1));
	}
	spt_camera.cam_angles = ang;
}

namespace patterns
{
	PATTERNS(ClientModeShared__OverrideView,
	         "5135",
	         "83 EC 58 E8 ?? ?? ?? ?? 85 C0 0F 84 ?? ?? ?? ?? 8B 10 56 8B 74 24 ?? 8B C8",
	         "3420",
	         "83 EC 40 E8 ?? ?? ?? ?? 85 C0 0F 84 ?? ?? ?? ?? 8B 10 56 8B 74 24 ?? 8B C8",
	         "1910503",
	         "55 8B EC 83 EC 58 E8 ?? ?? ?? ?? 85 C0 0F 84 ?? ?? ?? ?? 8B 10 56 8B 75 ?? 8B C8",
	         "7197370",
	         "55 8B EC 83 EC 58 E8 ?? ?? ?? ?? 85 C0 0F 84 ?? ?? ?? ?? 8B 10 8B C8 56");
	PATTERNS(ClientModeShared__CreateMove,
	         "5135",
	         "E8 ?? ?? ?? ?? 85 C0 75 ?? B0 01 C2 08 00 8B 4C 24 ?? D9 44 24 ?? 8B 10",
	         "1910503",
	         "55 8B EC E8 ?? ?? ?? ?? 85 C0 75 ?? B0 01 5D C2 08 00 8B 4D ?? 8B 10",
	         "7197370",
	         "55 8B EC E8 ?? ?? ?? ?? 8B C8 85 C9 75 ?? B0 01 5D C2 08 00 8B 01");
	PATTERNS(
	    C_BasePlayer__ShouldDrawLocalPlayer,
	    "5135",
	    "8B 0D ?? ?? ?? ?? 8B 01 8B 50 ?? FF D2 85 C0 75 ?? E8 ?? ?? ?? ?? 84 C0 74 ?? E8 ?? ?? ?? ?? 84 C0 75 ?? 33 C0",
	    "7197370",
	    "8B 0D ?? ?? ?? ?? 85 C9 74 ?? 8B 01 8B 40 ?? FF D0 84 C0 74 ?? A1 ?? ?? ?? ?? A8 01");
	PATTERNS(
	    CInput__MouseMove,
	    "5135",
	    "83 EC 14 56 8B F1 8B 0D ?? ?? ?? ?? 8B 01 8B 40 ?? 8D 54 24 ?? 52 FF D0 8B CE E8 ?? ?? ?? ?? 8B 0D ?? ?? ?? ?? 8B 11",
	    "7197370",
	    "55 8B EC 83 EC 1C 8D 55 ?? 56 8B F1 8B 0D ?? ?? ?? ?? 52 8B 01 FF 50 ?? 8B CE E8 ?? ?? ?? ?? 8B 0D ?? ?? ?? ?? 8B 01");
	PATTERNS(C_BasePlayer__ShouldDrawThisPlayer,
	         "7197370",
	         "E8 ?? ?? ?? ?? 84 C0 75 ?? B0 01 C3 8B 0D ?? ?? ?? ?? 85 C9 74 ?? 8B 01");

} // namespace patterns

void Camera::InitHooks()
{
	HOOK_FUNCTION(client, ClientModeShared__OverrideView);
	HOOK_FUNCTION(client, ClientModeShared__CreateMove);
	HOOK_FUNCTION(client, CInput__MouseMove);
	HOOK_FUNCTION(client, C_BasePlayer__ShouldDrawLocalPlayer);
#ifdef SSDK2013
	HOOK_FUNCTION(client, C_BasePlayer__ShouldDrawThisPlayer);
#endif
}

bool Camera::ShouldOverrideView() const
{
	return y_spt_cam_control.GetBool() && interfaces::engine_client->IsInGame()
	       && (_sv_cheats->GetBool() || interfaces::engine_client->IsPlayingDemo());
}

bool Camera::IsInDriveMode() const
{
	return ShouldOverrideView() && y_spt_cam_drive.GetBool() && !interfaces::engine_vgui->IsGameUIVisible();
}

void Camera::GetCurrentView()
{
	cam_origin = spt_playerio.GetPlayerEyePos();
	float ang[3];
	EngineGetViewAngles(ang);
	cam_angles = QAngle(ang[0], ang[1], 0);
}

void Camera::OverrideView(CViewSetup* view)
{
	if (ShouldOverrideView())
	{
		HandleInput(true);
		view->origin = cam_origin;
		view->angles = cam_angles;
	}
	else if (input_active)
	{
		HandleInput(false);
	}
}

static bool isBindDown(const char* bind)
{
#ifndef OE
	const char* key = interfaces::engine_client->Key_LookupBinding(bind);
	if (key)
	{
		ButtonCode_t code = interfaces::inputSystem->StringToButtonCode(key);
		return interfaces::inputSystem->IsButtonDown(code);
	}
#endif
	return false;
}

void Camera::HandleInput(bool active)
{
	static std::chrono::time_point<std::chrono::steady_clock> last_frame;
	auto now = std::chrono::steady_clock::now();

	if (input_active ^ active)
	{
		if (input_active && !active)
		{
			// On camera control end
			interfaces::vgui_input->SetCursorPos(old_cursor[0], old_cursor[1]);
		}
		else
		{
			// On camera control start
			last_frame = now;
			GetCurrentView();
			interfaces::vgui_input->GetCursorPos(old_cursor[0], old_cursor[1]);
		}
	}

	float real_frame_time = std::chrono::duration_cast<std::chrono::duration<float>>(now - last_frame).count();
	last_frame = now;

	if (active && IsInDriveMode())
	{
		float frame_time = real_frame_time;
		Vector movement(0.0f, 0.0f, 0.0f);

		float move_speed = isBindDown("+speed") ? 525.0f : (isBindDown("+duck") ? 60.0 : 175.0f);

		if (isBindDown("+forward"))
			movement.x += 1.0f;
		if (isBindDown("+back"))
			movement.x -= 1.0f;
		if (isBindDown("+moveleft"))
			movement.y -= 1.0f;
		if (isBindDown("+moveright"))
			movement.y += 1.0f;
		if (isBindDown("+moveup"))
			movement.z += 1.0f;
		if (isBindDown("+movedown"))
			movement.z -= 1.0f;

		int mx, my;
		int dx, dy;

		interfaces::vgui_input->GetCursorPos(mx, my);

		dx = mx - old_cursor[0];
		dy = my - old_cursor[1];

		interfaces::vgui_input->SetCursorPos(old_cursor[0], old_cursor[1]);

		// Convert to pitch/yaw
		float pitch = (float)dy * 0.022f * sensitivity->GetFloat();
		float yaw = -(float)dx * 0.022f * sensitivity->GetFloat();

		// Apply mouse
		cam_angles.x += pitch;
		cam_angles.x = clamp(cam_angles.x, -89.0f, 89.0f);
		cam_angles.y += yaw;
		if (cam_angles.y > 180.0f)
			cam_angles.y -= 360.0f;
		else if (cam_angles.y < -180.0f)
			cam_angles.y += 360.0f;

		// Now apply forward, side, up

		Vector fwd, side, up;

		AngleVectors(cam_angles, &fwd, &side, &up);

		VectorNormalize(movement);
		movement *= move_speed * frame_time;

		cam_origin += fwd * movement.x;
		cam_origin += side * movement.y;
		cam_origin += up * movement.z;
	}
	input_active = active;
}

void __fastcall Camera::HOOKED_ClientModeShared__OverrideView(void* thisptr, int edx, CViewSetup* view)
{
	spt_camera.OverrideView(view);
	spt_camera.ORIG_ClientModeShared__OverrideView(thisptr, edx, view);
}

bool __fastcall Camera::HOOKED_ClientModeShared__CreateMove(void* thisptr, int edx, float flInputSampleTime, void* cmd)
{
	if (spt_camera.IsInDriveMode())
	{
		// Block all inputs
		auto usercmd = reinterpret_cast<CUserCmd*>(cmd);
		usercmd->buttons = 0;
		usercmd->forwardmove = 0;
		usercmd->sidemove = 0;
		usercmd->upmove = 0;
	}
	return spt_camera.ORIG_ClientModeShared__CreateMove(thisptr, edx, flInputSampleTime, cmd);
}

bool __fastcall Camera::HOOKED_C_BasePlayer__ShouldDrawLocalPlayer(void* thisptr, int edx)
{
	if (spt_camera.ShouldOverrideView())
	{
		return true;
	}
	return spt_camera.ORIG_C_BasePlayer__ShouldDrawLocalPlayer(thisptr, edx);
}

#if defined(SSDK2013)
bool __fastcall Camera::HOOKED_C_BasePlayer__ShouldDrawThisPlayer(void* thisptr, int edx)
{
	// ShouldDrawLocalPlayer only decides draw view model or weapon model in steampipe
	// We need ShouldDrawThisPlayer to make player model draw
	if (spt_camera.ShouldOverrideView())
	{
		return true;
	}
	return spt_camera.ORIG_C_BasePlayer__ShouldDrawThisPlayer(thisptr, edx);
}
#endif

void __fastcall Camera::HOOKED_CInput__MouseMove(void* thisptr, int edx, void* cmd)
{
	// Block mouse inputs and stop the game from resetting cursor pos
	if (spt_camera.IsInDriveMode())
		return;
	spt_camera.ORIG_CInput__MouseMove(thisptr, edx, cmd);
}

bool Camera::ShouldLoadFeature()
{
	return interfaces::engine_client != nullptr && interfaces::engine_vgui != nullptr
	       && interfaces::vgui_input != nullptr && interfaces::inputSystem != nullptr;
}

void Camera::PreHook()
{
	loadingSuccessful = ORIG_ClientModeShared__OverrideView && ORIG_ClientModeShared__CreateMove
	                    && ORIG_CInput__MouseMove && ORIG_C_BasePlayer__ShouldDrawLocalPlayer;
#if defined(SSDK2013)
	loadingSuccessful &= !!ORIG_C_BasePlayer__ShouldDrawThisPlayer;
#endif
}

void Camera::LoadFeature()
{
	if (loadingSuccessful)
	{
		InitConcommandBase(y_spt_cam_control);
		InitConcommandBase(y_spt_cam_drive);
		InitCommand(y_spt_cam_setpos);
		InitCommand(y_spt_cam_setang);

		sensitivity = interfaces::g_pCVar->FindVar("sensitivity");
	}
}

void Camera::UnloadFeature() {}