#pragma once

#include "../../feature.hpp"
#include <string>
#include <vector>
#include "lua.hpp"

class LuaLibrary {

protected:
    const std::string name;

public:
    explicit LuaLibrary(std::string name);

    virtual void Load(lua_State *L) = 0;

    virtual void Unload(lua_State *L);

    virtual const std::string& GetLuaSource();
};

class LuaFeature : public FeatureWrapper<LuaFeature> {

public:
    lua_State * GetLuaState();

    void ResetLuaState();

    void RegisterLibrary(LuaLibrary *library);

    void LoadLibraries(lua_State *state);

    void UnloadLibraries(lua_State *state);

    void Execute(const std::string &code);

    void ExecuteFile(const std::string &path);

protected:
    bool ShouldLoadFeature() override;

    void InitHooks() override;

    void LoadFeature() override;

    void UnloadFeature() override;

private:
    lua_State *L = nullptr;

    std::vector<LuaLibrary *> libraries;

    static void InitDirectory();

    void OnTick();
};

extern LuaFeature spt_lua;