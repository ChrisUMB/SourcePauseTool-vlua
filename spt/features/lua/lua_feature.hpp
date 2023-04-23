#pragma once

#include "../../feature.hpp"
#include <string>
#include <vector>
#include "lua.hpp"

class LuaLibrary {

public:
    const std::string name;

    explicit LuaLibrary(std::string name);

    virtual void Load(lua_State *L) = 0;

    virtual void Unload(lua_State *L);

    virtual const std::string& GetLuaSource();
};

class LuaFeature : public FeatureWrapper<LuaFeature> {

public:
    lua_State * GetLuaState();

    void ResetLuaState();

    void InitLuaState(lua_State* L);

    void RegisterLibrary(LuaLibrary *library);

    void LoadLibraries(lua_State *L);

    void UnloadLibraries(lua_State *L);

    void Execute(const std::string &code);

    void ExecuteFile(const std::string &path);

protected:
    bool ShouldLoadFeature() override;

    void InitHooks() override;

    void LoadFeature() override;

    void UnloadFeature() override;

private:
    lua_State *global_state = nullptr;

    std::vector<LuaLibrary *> libraries;

    static void InitDirectory();
};

extern LuaFeature spt_lua;