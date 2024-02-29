#pragma once

#include "../lua_feature.hpp"
#include <vector>

class LuaEventsLibrary : public LuaLibrary {
protected:
    std::vector<lua_State*> states;

public:
    explicit LuaEventsLibrary();

    void InvokeEvent(const std::string& event_name, const std::function<void(lua_State*)>& lambda);

    void Load(lua_State* L) override;

    void Unload(lua_State* L) override;

    const std::string& GetLuaSource() override;
};

extern LuaEventsLibrary lua_events_library;
