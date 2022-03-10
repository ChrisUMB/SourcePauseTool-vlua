#pragma once

#if defined(SSDK2007)
#include "..\feature.hpp"
#include "hud.hpp"
#include "basehandle.h"
#include "Color.h"

#ifdef OE
#include "..\game_shared\usercmd.h"
#else
#include "usercmd.h"
#endif

typedef void(__fastcall* _DecodeUserCmdFromBuffer)(void* thisptr, int edx, bf_read& buf, int sequence_number);

// Input HUD
class InputHud : public FeatureWrapper<InputHud>
{
public:
	void DrawInputHud();
	void SetInputInfo(int button, Vector movement);
	bool ModifySetting(const char* element, const char* param, const char* value);

	struct Button
	{
		bool enabled;
		std::wstring text;
		vgui::HFont font;
		int x;
		int y;
		int width;
		int hight;
		Color background;
		Color highlight;
		Color textcolor;
		Color texthighlight;
		int mask;
	};
	bool tasPreset = false;
	std::map<std::string, Button> buttonSetting;
	Button anglesSetting;

protected:
	virtual bool ShouldLoadFeature() override;

	virtual void InitHooks() override;

	virtual void LoadFeature() override;

	virtual void PreHook() override;

	virtual void UnloadFeature() override;

private:
	static void __fastcall HOOKED_DecodeUserCmdFromBuffer(void* thisptr,
	                                                      int edx,
	                                                      bf_read& buf,
	                                                      int sequence_number);
	_DecodeUserCmdFromBuffer ORIG_DecodeUserCmdFromBuffer = nullptr;
	void CreateMove(uintptr_t pCmd);
	void DrawRectAndCenterTxt(Color buttonColor,
	                          int x0,
	                          int y0,
	                          int x1,
	                          int y1,
	                          vgui::HFont font,
	                          Color textColor,
	                          const wchar_t* text);
	void DrawButton(Button button);
	Color StringToColor(const char* hex);

	IMatSystemSurface* surface;

	int xOffset;
	int yOffset;
	int gridSize;
	int padding;

	Vector inputMovement;
	Vector currentAng;
	Vector previousAng;
	int buttonBits;

	bool awaitingFrameDraw;

	bool loadingSuccessful = false;
};

extern InputHud spt_ihud;

#endif