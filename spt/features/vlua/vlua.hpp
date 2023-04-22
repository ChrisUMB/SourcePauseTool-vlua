//
// Created by Chris on 4/21/2023.
//

#pragma once

#include "../../feature.hpp"
//extern "C" {
//#include "lua.h"
//#include "lualib.h"
//#include "lauxlib.h"
//}

class VLuaFeature : public FeatureWrapper<VLuaFeature> {

protected:
    virtual bool ShouldLoadFeature() override;

    virtual void InitHooks() override;

    virtual void LoadFeature() override;

    virtual void UnloadFeature() override;

private:
    void OnTick();
};

extern VLuaFeature spt_vlua;