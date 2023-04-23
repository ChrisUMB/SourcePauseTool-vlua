#include "stdafx.hpp"
#include "../lua_util.hpp"
#include "lua_lib_console.hpp"
#include "sdk/sdk2013/public/Color.h"
#include "interfaces.hpp"

LuaConsoleLibrary lua_console_library;

#define CON_PRINT(func) [](lua_State *L) { ConsolePrint(L, func); return 0; }

static void ConsolePrint(lua_State *L, void (*function)(const tchar *pMsg, ...)) {
    int argc = lua_gettop(L);
    if (argc < 1) {
        return;
    }

    int first_type = lua_type(L, 1);
    if (argc == 1 || first_type == LUA_TSTRING) {
        lua_string_format(L);
        function("%s", luaL_checkstring(L, -1));
    } else if (first_type == LUA_TNUMBER) {
        int hex_or_red = lua_tointeger(L, 1);
        lua_remove(L, 1);

        if (argc >= 4
            && lua_type(L, 1) == LUA_TNUMBER
            && lua_type(L, 2) == LUA_TNUMBER
            && lua_type(L, 3) == LUA_TSTRING
                ) {
            int green = lua_tointeger(L, 1);
            lua_remove(L, 1);
            int blue = lua_tointeger(L, 1);
            lua_remove(L, 1);
            lua_string_format(L);

            ConColorMsg(Color(hex_or_red, green, blue, 255), "%s", luaL_checkstring(L, -1));
        } else {
            lua_string_format(L);

            Color color = Color(
                    hex_or_red >> 16 & 0xFF,
                    hex_or_red >> 8 & 0xFF,
                    hex_or_red & 0xFF,
                    255
            );

            ConColorMsg(color, "%s", luaL_checkstring(L, -1));
        }
    } else {
        luaL_error(L, "console.msg: invalid argument type, expected string or number");
    }
}

static int ConsoleExec(lua_State *L) {
    lua_string_format(L);
    const char *string = luaL_checkstring(L, 1);
    interfaces::engine->ClientCmd(string);
    return 0;
}

/*
static int lua_console_clear(lua_State *L) {
    interfaces::engine_vgui->ClearConsole();
    return 0;
}

static int lua_console_close(lua_State *L) {
    IEngineVGui *vgui = interfaces::engine_vgui;

    if (vgui->IsConsoleVisible()) {
        vgui->HideConsole();
    }

    vgui->HideGameUI();
    return 0;
}

static int lua_console_hide(lua_State *L) {
    IEngineVGuiInternal *vgui = interfaces::engine_vgui;

    if (vgui->IsConsoleVisible()) {
        vgui->HideConsole();
    }

    return 0;
}

static int lua_console_open(lua_State *L) {
    IEngineVGuiInternal *vgui = interfaces::engine_vgui;

    if (!vgui->IsConsoleVisible()) {
        vgui->ShowConsole();
    }

    return 0;
}

static int lua_console_is_visible(lua_State *L) {
    lua_pushboolean(L, interfaces::engine_vgui->IsConsoleVisible());
    return 1;
}

*/
static int ConsolePrintln(lua_State *L) {
    Msg("%s\n", luaL_checkstring(L, 1));
    return 0;
}

static int ConsoleVarFind(lua_State *L) {
    const char *string = luaL_checkstring(L, 1);

    ConVar *convar = interfaces::g_pCVar->FindVar(string);

    if (convar == nullptr) {
        lua_pushnil(L);
        return 1;
    }

    auto **convar_ptr = (ConVar **) lua_newuserdata(L, sizeof(ConVar *));
    *convar_ptr = convar;

    luaL_getmetatable(L, "convar");
    lua_setmetatable(L, -2);
    return 1;
}

static const struct luaL_Reg console_class[] = {
        {"msg",     CON_PRINT(Msg)},
        {"dev_msg", CON_PRINT(DevMsg)},
        {"log",     CON_PRINT(Log)},
        {"warning", CON_PRINT(Warning)},
        {"error",   CON_PRINT(Error)},
        {"exec",     ConsoleExec},
//        {"clear",      lua_console_clear},
        {"var_find", ConsoleVarFind},
//        {"var_create", lua_console_var_create},
//        {"cmd_find",   lua_console_cmd_find},
//        {"cmd_create", lua_console_cmd_create},
//        {"close",      lua_console_close},
//        {"hide",       lua_console_hide},
//        {"open",       lua_console_open},
//        {"is_visible", lua_console_is_visible},
        {nullptr,    nullptr}
};

#define CONVAR_GET() ConVar **convar_ptr = (ConVar **) luaL_checkudata(L, 1, "convar");\
if (convar_ptr == nullptr) {\
luaL_argcheck(L, convar_ptr != nullptr, 1, "convar expected");\
return 0;\
}

static int ConvarGetNumber(lua_State *L) {
    CONVAR_GET();

    lua_pushnumber(L, (*convar_ptr)->GetFloat());
    return 1;
}

static int ConvarGetString(lua_State *L) {
    CONVAR_GET();

    lua_pushstring(L, (*convar_ptr)->GetString());
    return 1;
}

static int ConvarGetBool(lua_State *L) {
    CONVAR_GET();

    lua_pushboolean(L, (*convar_ptr)->GetBool());
    return 1;
}

static int ConvarSetNumber(lua_State *L) {
    CONVAR_GET();

    (*convar_ptr)->SetValue((float) luaL_checknumber(L, 2));
    return 0;
}

static int ConvarSetString(lua_State *L) {
    CONVAR_GET();

    (*convar_ptr)->SetValue(luaL_checkstring(L, 2));
    return 0;
}

static int ConvarSetBool(lua_State *L) {
    CONVAR_GET();

    (*convar_ptr)->SetValue(lua_toboolean(L, 2));
    return 0;
}

static const struct luaL_Reg convar_class[] = {
        {"get_number", ConvarGetNumber},
        {"get_string", ConvarGetString},
        {"get_bool",   ConvarGetBool},
        {"set_number", ConvarSetNumber},
        {"set_string", ConvarSetString},
        {"set_bool",   ConvarSetBool},
        {nullptr,      nullptr}
};

LuaConsoleLibrary::LuaConsoleLibrary() : LuaLibrary("console") {}

void LuaConsoleLibrary::Load(lua_State *L) {
    luaL_register(L, this->name.c_str(), console_class);
    lua_pop(L, 1);

    lua_pushcfunction(L, &ConsolePrintln);
    lua_setglobal(L, "print");

    luaL_newmetatable(L, "convar");
    lua_pushstring(L, "__index");
    lua_pushvalue(L, -2);
    lua_settable(L, -3);

    luaL_register(L, nullptr, convar_class);
    lua_pop(L, 1);

//    lua_register_concmd(L);
}

void LuaConsoleLibrary::Unload(lua_State *L) {}

const std::string &LuaConsoleLibrary::GetLuaSource() {
    static std::string sources = R"""(
---@class console
console = {}

---@param msg string Message to be printed, accepts C++ style string formatting.
---@vararg any
---@overload fun(color_hex: number, msg: string, ...)
---@overload fun(color_r: number, color_g: number, color_b: number, msg: string, ...)
---Print a message to the console.
function console.msg(msg, ...)
end

---@param msg string Message to be printed
---@vararg any
---Print a message to the console, only visible with `developer 1` enabled.
function console.dev_msg(msg, ...)
end

---@param msg string Message to be printed
---@vararg any
---Print a message to the console with `log` functionality.
function console.log(msg, ...)
end

---@param msg string Message to be printed
---@vararg any
---Print a message to the console as a warning, automatically red.
function console.warning(msg, ...)
end

---@param msg string Message to be displayed in the error dialogue.
---@vararg any
--- ## Warning
---Closes the game with an error dialogue menu.
function console.error(msg, ...)
end

---@param command string Command to be executed
---@vararg any
---Executes the command to the console.
---Accepts formatting.
function console.exec(command, ...)
end

---Clears the console text.
function console.clear()
end

---@param name string ConVar name to find.
---@return convar ConVar object.
function console.var_find(name)
    return nil
end

---@param name string                           Name of ConVar.
---@param default_value number|string|boolean   Default value of ConVar.
---@param flags number                          Flags of ConVar.
---@param help_text string                      Help text of ConVar.
---@param min number|nil                        Minimum value of ConVar.
---@param max number|nil                        Max value of ConVar.
---@return convar                               ConVar object.
---
---@overload fun(name:string, default_value:number|string|boolean):convar
---@overload fun(name:string, default_value:number|string|boolean, flags:number):convar
---@overload fun(name:string, default_value:number|string|boolean, flags:number, help_text:string):convar
---@overload fun(name:string, default_value:number|string|boolean, flags:number, help_text:string, min:number|nil):convar
---@overload fun(name:string, default_value:number|string|boolean, flags:number, help_text:string, min:number|nil, max:number|nil):convar
function console.var_create(name, default_value, flags, help_text, min, max)
    return nil
end

---@param name string ConCommand name to find.
---@return concmd ConCommand object.
function console.cmd_find(name)
    return nil
end

---@param name string Name of ConCommand to create.
---@param help_text string Help text of ConCommand.
---@param callback function Callback function.
---@return concmd ConCommand object.
---
---@overload fun(name:string, callback:function):concmd
function console.cmd_create(name, help_text, callback)
    return nil
end

---Closes the developer console as if you hit '`', which will unpause the game.
function console.close()
end

---Hides the developer console, but keeps VGUI elements alive.
function console.hide()
end

---Opens the developer console, which also pauses the game.
function console.open()
end

---@return boolean Whether the console is open
function console.is_visible()
    return nil
end

---@class convar
convar = {}

---@return number The numerical value of the ConVar.
function convar:get_number()
    return 0
end

---@return string The string value of the ConVar.
function convar:get_string()
    return ""
end

---@return boolean The boolean value of the ConVar.
function convar:get_bool()
    return false
end

---@param value number The numerical value to set the ConVar to.
function convar:set_number(value)
end

---@param value string The string value to set the ConVar to.
function convar:set_string(value)
end

---@param value boolean The boolean value to set the ConVar to.
function convar:set_bool(value)
end

---Unregisters the ConVar.
function convar:unregister()
end
    )""";

    return sources;
}
