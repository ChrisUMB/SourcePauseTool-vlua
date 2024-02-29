#include "stdafx.hpp"
#include "../lua_util.hpp"
#include "lua_lib_console.hpp"
#include "sdk/sdk2013/public/Color.h"
#include "interfaces.hpp"

LuaConsoleLibrary lua_console_library;

static int ConsolePrint(lua_State* L, void (*function)(const tchar* pMsg, ...)) {
    if (lua_gettop(L) < 1)
        return 0;

    if (lua_type(L, 1) == LUA_TSTRING) {
        lua_string_format(L);
        function("%s", luaL_checkstring(L, -1));
    } else {
        return luaL_error(L, "console.msg: invalid argument type, expected string");
    }

    return 0;
}

static int ConsoleMsg(lua_State* L) {
    int argc = lua_gettop(L);

    if (argc < 1) {
        return luaL_error(L, "console.msg_color: not enough arguments");
    }

    int type = lua_type(L, 1);
    if (type == LUA_TSTRING) {
        lua_string_format(L);
        ConMsg("%s", luaL_checkstring(L, -1));
        return 0;
    }

    int hex_or_red = luaL_checkinteger(L, 1);
    lua_remove(L, 1);

    if (argc >= 4 && lua_type(L, 1) == LUA_TNUMBER && lua_type(L, 2) == LUA_TNUMBER
        && lua_type(L, 3) == LUA_TSTRING) {
        int green = lua_tointeger(L, 1);
        lua_remove(L, 1);
        int blue = lua_tointeger(L, 1);
        lua_remove(L, 1);
        lua_string_format(L);

        ConColorMsg(Color(hex_or_red, green, blue, 255), "%s", luaL_checkstring(L, -1));
    } else {
        lua_string_format(L);
        Color color = Color(hex_or_red >> 16 & 0xFF, hex_or_red >> 8 & 0xFF, hex_or_red & 0xFF, 255);
        ConColorMsg(color, "%s", luaL_checkstring(L, -1));
    }
    return 0;
}

static int ConsoleDevMsg(lua_State* L) {
    return ConsolePrint(L, DevMsg);
}

static int ConsoleLog(lua_State* L) {
    return ConsolePrint(L, Log);
}

static int ConsoleWarning(lua_State* L) {
    return ConsolePrint(L, Warning);
}

static int ConsoleError(lua_State* L) {
    return ConsolePrint(L, Error);
}

static int ConsoleExec(lua_State* L) {
    lua_string_format(L);
    const char* string = luaL_checkstring(L, 1);
    interfaces::engine->ClientCmd(string);
    return 0;
}

static int ConsolePrintln(lua_State* L) {
    Msg("%s\n", luaL_checkstring(L, 1));
    return 0;
}

static int ConsoleVarFind(lua_State* L) {
    const char* string = luaL_checkstring(L, 1);

    ConVar* convar = interfaces::g_pCVar->FindVar(string);

    if (convar == nullptr) {
        lua_pushnil(L);
        return 1;
    }

    auto** convar_ptr = (ConVar**)lua_newuserdata(L, sizeof(ConVar*));
    *convar_ptr = convar;

    luaL_getmetatable(L, "convar");
    lua_setmetatable(L, -2);
    return 1;
}

static int ConsoleVarCreate(lua_State* L) {
    // const int flags = luaL_checkinteger(L, 3);
    const char* name = luaL_checkstring(L, 1);
    ConVar* cvar = g_pCVar->FindVar(name);

    if (!cvar) {
        name = strdup(name);
        const char* default_value = strdup(luaL_checkstring(L, 2));
        const char* help = strdup(luaL_checkstring(L, 3));
        const auto convar = new ConVar(name, default_value, FCVAR_UNREGISTERED, help);
        g_pCVar->RegisterConCommand(convar);
    }

    auto** convar_ptr = static_cast<ConVar**>(lua_newuserdata(L, sizeof(ConVar*)));
    *convar_ptr = cvar;

    luaL_getmetatable(L, "convar");
    lua_setmetatable(L, -2);
    return 1;
}

static const luaL_Reg console_class[] = {
    {"msg", ConsoleMsg},
    {"dev_msg", ConsoleDevMsg},
    {"log", ConsoleLog},
    {"warning", ConsoleWarning},
    {"error", ConsoleError},
    {"exec", ConsoleExec},
    {"var_find", ConsoleVarFind},
    {"var_create", ConsoleVarCreate},
    {nullptr, nullptr}
};

#define CONVAR_GET() \
	auto convar_ptrptr = (ConVar**)luaL_checkudata(L, 1, "convar"); \
	if (convar_ptrptr == nullptr) \
	{ \
		luaL_argcheck(L, convar_ptrptr != nullptr, 1, "convar expected"); \
		return 0; \
	} \
	auto convar_ptr = *convar_ptrptr; \
	do \
	{ \
	} while (0)

static int ConvarGetNumber(lua_State* L) {
    CONVAR_GET();

    lua_pushnumber(L, convar_ptr->GetFloat());
    return 1;
}

static int ConvarGetString(lua_State* L) {
    CONVAR_GET();

    lua_pushstring(L, convar_ptr->GetString());
    return 1;
}

static int ConvarGetBool(lua_State* L) {
    CONVAR_GET();

    lua_pushboolean(L, convar_ptr->GetBool());
    return 1;
}

static int ConvarSetNumber(lua_State* L) {
    CONVAR_GET();

    convar_ptr->SetValue((float)luaL_checknumber(L, 2));
    return 0;
}

static int ConvarSetString(lua_State* L) {
    CONVAR_GET();

    convar_ptr->SetValue(luaL_checkstring(L, 2));
    return 0;
}

static int ConvarSetBool(lua_State* L) {
    CONVAR_GET();

    convar_ptr->SetValue(lua_toboolean(L, 2));
    return 0;
}

static const luaL_Reg convar_class[] = {
    {"get_number", ConvarGetNumber},
    {"get_string", ConvarGetString},
    {"get_bool", ConvarGetBool},
    {"set_number", ConvarSetNumber},
    {"set_string", ConvarSetString},
    {"set_bool", ConvarSetBool},
    {nullptr, nullptr}
};

LuaConsoleLibrary::LuaConsoleLibrary() : LuaLibrary("console") {}

void LuaConsoleLibrary::Load(lua_State* L) {
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

    /*    luaL_newmetatable(L, "concmd");
        lua_pushstring(L, "__index");
        lua_pushvalue(L, -2);
        lua_settable(L, -3);

        luaL_register(L, nullptr, concmd_class);
        lua_pop(L, 1);*/
}

void LuaConsoleLibrary::Unload(lua_State* L) {}

const std::string& LuaConsoleLibrary::GetLuaSource() {
    static std::string sources = R"""(---@meta
---@class console
console = {}

---@param msg string Message to be printed, accepts C++ style string formatting
---@vararg any
---@overload fun(red: number, green: number, blue: number, msg: string, ...)
---@overload fun(color: number, msg: string, ...)
---Print a message to the console
function console.msg(msg, ...)
end

---@param msg string Message to be printed
---@vararg any
---Print a message to the console, only visible with `developer 1` enabled
function console.dev_msg(msg, ...)
end

---@param msg string Message to be printed
---@vararg any
---Print a message to the console with `log` functionality
function console.log(msg, ...)
end

---@param msg string Message to be printed
---@vararg any
---Print a message to the console as a warning, automatically red
function console.warning(msg, ...)
end

---@param msg string Message to be displayed in the error dialogue
---@vararg any
--- ## Warning
---Closes the game with an error dialogue menu
function console.error(msg, ...)
end

---@param command string Command to be executed
---@vararg any
---Executes the command to the console
---Accepts formatting.
function console.exec(command, ...)
end

---@param name string ConVar name to find
---@return convar ConVar object.
function console.var_find(name)
end

---@param name string Name of the ConVar to create
---@param default_value string Default value of the ConVar
---@param help_text string Help text of the ConVar
function console.var_create(name, default_value, help_text)
end

---@class convar
convar = {}

---@return number The numerical value of the ConVar
function convar:get_number()
    return 0
end

---@return string The string value of the ConVar
function convar:get_string()
    return ""
end

---@return boolean The boolean value of the ConVar
function convar:get_bool()
    return false
end

---@param value number The numerical value to set the ConVar to
function convar:set_number(value)
end

---@param value string The string value to set the ConVar to
function convar:set_string(value)
end

---@param value boolean The boolean value to set the ConVar to
function convar:set_bool(value)
end

---Unregisters the ConVar
function convar:unregister()
end

---@class concmd
concmd = {}

---@vararg string Arguments to be passed to the command
function concmd:exec(...)
end
    )""";

    return sources;
}
