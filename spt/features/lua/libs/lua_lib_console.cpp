#include "stdafx.hpp"
#include "../lua_util.hpp"
#include "lua_lib_console.hpp"

#include <ranges>

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
    const char* name = luaL_checkstring(L, 1);
    ConVar* cvar = g_pCVar->FindVar(name);

    if (!cvar) {
        name = strdup(name);
        const char* default_value = strdup(luaL_checkstring(L, 2));
        const char* help = strdup(luaL_checkstring(L, 3));
        cvar = new ConVar(name, default_value, FCVAR_UNREGISTERED, help);
        g_pCVar->RegisterConCommand(cvar);
    }

    auto** convar_ptr = static_cast<ConVar**>(lua_newuserdata(L, sizeof(ConVar*)));
    *convar_ptr = cvar;

    luaL_getmetatable(L, "convar");
    lua_setmetatable(L, -2);
    return 1;
}

LuaCommandCallback::LuaCommandCallback(lua_State* L, int lua_function_ref) {
    this->L = L;
    this->lua_function_ref = lua_function_ref;
}

void LuaCommandCallback::CommandCallback(const CCommand& command) {
    lua_rawgeti(L, LUA_REGISTRYINDEX, lua_function_ref);

    int arg_count = command.ArgC();
    for (int i = 1; i < arg_count; ++i) {
        lua_pushstring(L, command.Arg(i));
    }

    if (lua_pcall(L, arg_count - 1, 0, 0)) {
        Warning("%s\n", lua_tostring(L, -1));
    }
}

LuaCommandCallback::~LuaCommandCallback() {
    luaL_unref(L, LUA_REGISTRYINDEX, lua_function_ref);
}

static std::map<std::string, LuaConCommand*> lua_con_commands;

/*
 * Issues:
 * 1. Command arguments do not seem to work properly.
 * 2. The code is fucking ugly.
 * 3. There might be a followthrough where function_ref is not initialized.
 */
static int ConsoleCmdCreate(lua_State* L) {
    const char* name = luaL_checkstring(L, 1);

    if (const auto old = g_pCVar->FindCommand(name); old != nullptr) {
        g_pCVar->UnregisterConCommand(old);
        if (const auto it = lua_con_commands.find(name); it != lua_con_commands.end()) {
            delete it->second;
            lua_con_commands.erase(it);
        }
    }

    luaL_checkany(L, 2);
    int type = lua_type(L, 2);

    int function_ref;
    char* help_text = nullptr;

    if (type == LUA_TSTRING) {
        size_t length;
        const char* help_text_temp = luaL_checklstring(L, 2, &length);
        help_text = new char[length + 1];
        strcpy(help_text, help_text_temp);
    } else if (type == LUA_TFUNCTION) {
        function_ref = luaL_ref(L, LUA_REGISTRYINDEX);
    } else {
        luaL_error(L, "Invalid type for second argument");
        return 0;
    }

    if (help_text != nullptr) {
        type = lua_type(L, 3);
        if (type != LUA_TFUNCTION) {
            luaL_error(L, "Invalid type for third argument");
            return 0;
        }

        function_ref = luaL_ref(L, LUA_REGISTRYINDEX);
    }

    auto* callback = new LuaCommandCallback(L, function_ref);
    const char* name_duped = strdup(name);
    auto* command = new ConCommand(name_duped, callback, help_text);
    g_pCVar->RegisterConCommand(command);

    lua_con_commands[name_duped] = new LuaConCommand(command, callback, help_text);
    return 0;
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
    {"cmd_create", ConsoleCmdCreate},
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

static int ConcmdExec(lua_State* L) {
    const auto name = luaL_checkstring(L, 1);

    ConCommand* cmd = g_pCVar->FindCommand(name);

    if (cmd == nullptr) {
        return luaL_error(L, "ConCommand not found with name %s", name);
    }

    const int argc = lua_gettop(L);
    const auto argv = new char const*[argc];

    for (int i = 0; i <= argc; ++i) {
        argv[i] = luaL_checkstring(L, i);
    }

    const CCommand command(argc, argv);
    cmd->Dispatch(command);

    delete[] argv;
    return 0;
}

static const luaL_Reg concmd_class[] = {
    {"exec", ConcmdExec},
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

    luaL_newmetatable(L, "concmd");
    lua_pushstring(L, "__index");
    lua_pushvalue(L, -2);
    lua_settable(L, -3);

    luaL_register(L, nullptr, concmd_class);
    lua_pop(L, 1);
}

void LuaConsoleLibrary::Unload(lua_State* L) {
    for (const auto cmd : lua_con_commands | std::views::values) {
        g_pCVar->UnregisterConCommand(cmd->concmd);
        delete cmd;
    }

    lua_con_commands.clear();
}

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
---@return convar ConVar object.
function console.var_create(name, default_value, help_text)
end

---@param name string Name of ConCommand to create.
---@param help_text string Help text of ConCommand.
---@param callback function Callback function.
---@return concmd ConCommand object.
---@overload fun(name:string, callback:function):concmd
function console.cmd_create(name, help_text, callback)
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
