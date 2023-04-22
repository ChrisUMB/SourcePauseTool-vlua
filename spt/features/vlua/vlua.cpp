#include "stdafx.hpp"
#include "vlua.hpp"
#include "signals.hpp"

VLuaFeature spt_vlua;

bool VLuaFeature::ShouldLoadFeature() {
    return true;
}

void VLuaFeature::InitHooks() {
}

void VLuaFeature::OnTick() {
    Msg("Tick has passed.\n");
}

void VLuaFeature::LoadFeature() {
    Msg("Vlua loaded.\n");

//    TickSignal.Connect(this, &VLuaFeature::OnTick);
//
//    lua_State *L = luaL_newstate();
//    luaL_openlibs(L);
//    luaL_loadstring(L, "return 5 + 5");
//    lua_pcall(L, 0, 1, 0);
//    int n = luaL_checkint(L, -1);
//    lua_close(L);
//
//    Msg("Lua returned %d.\n", n);
}

void VLuaFeature::UnloadFeature() {
    Msg("Vlua unloaded.\n");
}
