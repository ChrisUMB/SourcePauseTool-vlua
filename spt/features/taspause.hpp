#pragma once
#include "../feature.hpp"

typedef void(__cdecl* _Host_AccumulateTime)(float dt);

class TASPause final : public FeatureWrapper<TASPause>
{
protected:
	bool ShouldLoadFeature() override;

	void InitHooks() override;

	void LoadFeature() override;

	void UnloadFeature() override;

private:
	float* pHost_Frametime = nullptr;
	float* pHost_Realtime = nullptr;
	uintptr_t ORIG__Host_RunFrame = 0;
	_Host_AccumulateTime ORIG_Host_AccumulateTime = nullptr;

	static void __cdecl HOOKED_Host_AccumulateTime(float dt);
public:
	[[nodiscard]] float GetHostFrametime() const { return pHost_Frametime ? *pHost_Frametime : 0; }
	[[nodiscard]] float GetHostRealtime() const { return pHost_Realtime ? *pHost_Realtime : 0; }
};

extern TASPause spt_taspause;