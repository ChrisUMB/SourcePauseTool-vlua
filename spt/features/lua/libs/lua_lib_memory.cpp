#include "stdafx.hpp"
#include "lua_lib_memory.hpp"
#include "SPTLib/memutils.hpp"

LuaMemoryLibrary::LuaMemoryLibrary() : LuaLibrary("memory") {}

static int MemoryHook(lua_State *L) {
    if(!lua_istable(L, 1)) {
        return 0;
    }

    lua_getfield(L, 1, "base");
    int module_base = lua_tointeger(L, -1);
    lua_pop(L, 1);

    lua_getfield(L, 1, "handle");
    int module_handle = lua_tointeger(L, -1);
    lua_pop(L, 1);

    lua_getfield(L, 1, "offset");
    int module_offset = lua_tointeger(L, -1);
    lua_pop(L, 1);

    if(!lua_isnumber(L, 2)) {
        return 0;
    }

    int hook_offset = lua_tointeger(L, 2);
    const char* hook_signature = lua_tostring(L, 3);
    return 1;
}

static int MemoryModule(lua_State *L) {

    const char *name = luaL_checkstring(L, 1);

    void *handle;
    void *start;
    size_t size;
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    if (!MemUtils::GetModuleInfo(converter.from_bytes(name), &handle, &start, &size)) {
        lua_pushnil(L);
        return 1;
    }

    lua_newtable(L);
    lua_pushnumber(L, (uintptr_t) handle);
    lua_setfield(L, -2, "handle");
    lua_pushnumber(L, (uintptr_t) start);
    lua_setfield(L, -2, "base");
    lua_pushnumber(L, size);
    lua_setfield(L, -2, "size");
    return 1;
}

static const struct luaL_Reg memory_class[] = {
        {"hook",   MemoryHook},
        {"module", MemoryModule},
        {nullptr,  nullptr}
};

void LuaMemoryLibrary::Load(lua_State *L) {
    luaL_register(L, "memory", memory_class);
    lua_pop(L, 1);
}

const std::string &LuaMemoryLibrary::GetLuaSource() {
    static const std::string sources = R"(
---@class memory
memory = {}

---@class pointer : number

---@class module
---@field handle pointer Pointer to the module
---@field base pointer Base address of the module
---@field size number Size of the module

---@param name string Name of the module
---@return module? Module with the given name, or `nil` if not found.
function memory.module(name) end

---@enum HookStatus
HookStatus = {
    --- Unknown error. Should not be returned.
    UNKNOWN = -1,
    --- Successful.
    OK = 0,
    --- MinHook is already initialized.
    ERROR_ALREADY_INITIALIZED = 1,
    --- MinHook is not initialized yet, or already uninitialized.
    ERROR_NOT_INITIALIZED = 2,
    --- The hook for the specified target function is already created.
    ERROR_ALREADY_CREATED = 3,
    --- The hook for the specified target function is not created yet.
    ERROR_NOT_CREATED = 4,
    --- The hook for the specified target function is already enabled.
    ERROR_ENABLED = 5,
    --- The hook for the specified target function is not enabled yet, or already disabled.
    ERROR_DISABLED = 6,
    --- The specified pointer is invalid. It points the address of non-allocated and/or non-executable region.
    ERROR_NOT_EXECUTABLE = 7,
    --- The specified target function cannot be hooked.
    ERROR_UNSUPPORTED_FUNCTION = 8,
    --- Failed to allocate memory.
    ERROR_MEMORY_ALLOC = 1-1,
    --- Failed to change the memory protection.
    ERROR_MEMORY_PROTECT = 10,
    --- The specified module is not loaded.
    ERROR_MODULE_NOT_FOUND = 11,
    --- The specified function is not found.
    ERROR_FUNCTION_NOT_FOUND = 12,
}

---@class hook
---@field module module Module that the hook is created in
---@field offset number Offset of the target function from the module base
---@field target pointer Address of the target function
---@field original pointer Address of the original function
---@field trampoline pointer Address of the trampoline function
---@field status HookStatus Status of the hook
hook = {}

--- Enables the hook.
---@return HookStatus Status of the hook.
function hook:enable() end

function hook:disable() end

function hook:remove() end

---@param module module Module to hook in
---@param offset number Offset of the target function from the module base
---@param signature string Signature of the target function
---@param callback fun(hook: hook, ...) Function to call when the target function is called
function memory.hook(module, offset, signature, callback)
    local hook
    hook = memory._hook(module, offset, signature, function(...)
        callback(hook, ...)
    end)
end

setmetatable(hook, hook)

---@class modules : table<string, module>
modules = {
    client = memory.module("client.dll"),
    server = memory.module("server.dll"),
    engine = memory.module("engine.dll")
}
)";

    return sources;
}
