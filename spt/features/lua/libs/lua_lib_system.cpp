#include "stdafx.hpp"
#include "lua_lib_system.hpp"
#include <Windows.h>


LuaSystemLibrary lua_system_library;

static int System(lua_State* L) {
    // Get the string from the function invocation
    const char* command = luaL_checkstring(L, 1);

    unsigned long exit_code = 0x0;
    std::string output;
    std::string error;

    HANDLE hStdOutPipeRead;
    HANDLE hStdOutPipeWrite;
    HANDLE hStdErrPipeRead;
    HANDLE hStdErrPipeWrite;

    // Create two pipes.
    SECURITY_ATTRIBUTES sa = {sizeof(sa), nullptr, TRUE};

    if (!CreatePipe(&hStdOutPipeRead, &hStdOutPipeWrite, &sa, 0)) {
        return luaL_error(L, "CreatePipe hStdOutPipe failed");
    }

    if (!CreatePipe(&hStdErrPipeRead, &hStdErrPipeWrite, &sa, 0)) {
        CloseHandle(hStdOutPipeRead);
        CloseHandle(hStdOutPipeWrite);
        return luaL_error(L, "CreatePipe hStdErrPipe failed");
    }

    // Create the process.
    STARTUPINFO si = {
        .cb = sizeof(si),
        .dwFlags = STARTF_USESTDHANDLES,
        .hStdOutput = hStdOutPipeWrite,
        .hStdError = hStdErrPipeWrite
    };

    PROCESS_INFORMATION pi;

    std::string cmd = std::format("cmd /s /c \"{}\"", command);

    if (!CreateProcess(
        R"(C:\Windows\System32\cmd.exe)", &cmd[0], nullptr, nullptr, TRUE, CREATE_NO_WINDOW, nullptr, nullptr, &si,
        &pi)) {
        CloseHandle(hStdOutPipeRead);
        CloseHandle(hStdOutPipeWrite);
        CloseHandle(hStdErrPipeRead);
        CloseHandle(hStdErrPipeWrite);
        return luaL_error(L, "CreateProcess failed");
    }

    // Close pipes we do not need.
    CloseHandle(hStdOutPipeWrite);
    CloseHandle(hStdErrPipeWrite);

    char buf[65536];
    DWORD dwRead = 0;
    bool reading = true;
    while (reading) {
        reading = false;

        if (ReadFile(hStdOutPipeRead, buf, 1024, &dwRead, nullptr)) {
            output.append(buf, dwRead);
            reading = true;
        }

        if (ReadFile(hStdErrPipeRead, buf, 1024, &dwRead, nullptr)) {
            error.append(buf, dwRead);
            reading = true;
        }
    }

    // Clean up and exit.
    CloseHandle(hStdOutPipeRead);
    CloseHandle(hStdErrPipeRead);

    WaitForSingleObject(pi.hProcess, 30000);
    GetExitCodeProcess(pi.hProcess, &exit_code);

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    lua_newtable(L);
    lua_pushnumber(L, exit_code);
    lua_setfield(L, -2, "exit_code");
    lua_pushstring(L, output.c_str());
    lua_setfield(L, -2, "output");
    lua_pushstring(L, error.c_str());
    lua_setfield(L, -2, "error");

    return 1;
}

LuaSystemLibrary::LuaSystemLibrary() : LuaLibrary("system") {}

void LuaSystemLibrary::Load(lua_State* L) {
    lua_pushcfunction(L, System);
    lua_setglobal(L, "system");
}

const std::string& LuaSystemLibrary::GetLuaSource() {
    static std::string sources = R"""(
---@class system_result
---@field exit_code number Exit code of the command.
---@field output string Output of the command.
---@field error string Error of the command.

---@param command string Command to execute in the shell.
---@return system_result Result of the command.
function system(command)
end
)""";

    return sources;
}
