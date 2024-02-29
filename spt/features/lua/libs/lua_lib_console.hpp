#pragma once

#include "../lua_feature.hpp"

class LuaConsoleLibrary : public LuaLibrary {
public:
    explicit LuaConsoleLibrary();

    void Load(lua_State* L) override;

    void Unload(lua_State* L) override;

    const std::string& GetLuaSource() override;
};

class LuaCommandCallback : public ICommandCallback {
private:
    lua_State *L;
    int lua_function_ref;

public:
    LuaCommandCallback(lua_State *L, int lua_function_ref);
    ~LuaCommandCallback();

    void CommandCallback(const CCommand &command);
};

struct LuaConCommand {
    ConCommand *concmd;
    LuaCommandCallback *callback;
    char *help_text;

    LuaConCommand(ConCommand *concmd,
                  LuaCommandCallback *callback,
                  char *help_text
    ) : concmd(concmd), callback(callback), help_text(help_text) {}

    ~LuaConCommand() {
        delete concmd;
        delete callback;
        delete[] help_text;
    }
};

extern LuaConsoleLibrary lua_console_library;
