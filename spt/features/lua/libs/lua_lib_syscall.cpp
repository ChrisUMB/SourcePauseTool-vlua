#include "stdafx.hpp"
#include "lua_lib_syscall.hpp"

#ifdef WIN32

#include <Windows.h>

#endif

LuaSyscallLibrary lua_syscall_library;

static int Syscall(lua_State* L)
{
	// Get the string from the function invocation
	auto command = std::string(luaL_checkstring(L, 1));

	unsigned long exit_code = 0x0;
	auto output = std::string();
	auto error = std::string();

#ifdef WIN32
	HANDLE hStdOutPipeRead = nullptr;
	HANDLE hStdOutPipeWrite = nullptr;
	HANDLE hStdErrPipeRead = nullptr;
	HANDLE hStdErrPipeWrite = nullptr;

	// Create two pipes.
	SECURITY_ATTRIBUTES sa = {sizeof(SECURITY_ATTRIBUTES), nullptr, TRUE};

	if (!CreatePipe(&hStdOutPipeRead, &hStdOutPipeWrite, &sa, 0))
	{
		return luaL_error(L, "CreatePipe hStdOutPipe failed");
	}

	if (!CreatePipe(&hStdErrPipeRead, &hStdErrPipeWrite, &sa, 0))
	{
		return luaL_error(L, "CreatePipe hStdErrPipe failed");
	}

	// Create the process.
	STARTUPINFO si = {};
	si.cb = sizeof(STARTUPINFO);
	si.dwFlags = STARTF_USESTDHANDLES;
	si.hStdError = hStdErrPipeWrite;
	si.hStdOutput = hStdOutPipeWrite;
	PROCESS_INFORMATION pi = {};
	std::string lpCommandLine = R"(C:\Windows\System32\cmd.exe /s /c )" + command;

	if (!CreateProcess(
	        nullptr, &lpCommandLine[0], nullptr, nullptr, TRUE, CREATE_NO_WINDOW, nullptr, nullptr, &si, &pi))
	{
		return luaL_error(L, "CreateProcess failed");
	}

	// Close pipes we do not need.
	CloseHandle(hStdOutPipeWrite);
	CloseHandle(hStdErrPipeWrite);

	char buf[1024];
	DWORD dwRead = 0;
	bool reading = true;
	while (reading)
	{
		reading = false;

		if (ReadFile(hStdOutPipeRead, buf, 1024, &dwRead, nullptr))
		{
			output.append(buf, dwRead);
			reading = true;
		}

		if (ReadFile(hStdErrPipeRead, buf, 1024, &dwRead, nullptr))
		{
			error.append(buf, dwRead);
			reading = true;
		}
	}

	// Clean up and exit.
	CloseHandle(hStdOutPipeRead);
	CloseHandle(hStdErrPipeRead);

	GetExitCodeProcess(pi.hProcess, &exit_code);

#else
	//Do later: Linux is very sad.
	system(command.c_str());

#endif

	lua_newtable(L);
	lua_pushnumber(L, exit_code);
	lua_setfield(L, -2, "exit_code");
	lua_pushstring(L, output.c_str());
	lua_setfield(L, -2, "output");
	lua_pushstring(L, error.c_str());
	lua_setfield(L, -2, "error");

	return 1;
}

LuaSyscallLibrary::LuaSyscallLibrary() : LuaLibrary("syscall") {}

void LuaSyscallLibrary::Load(lua_State* L)
{
	lua_pushcfunction(L, Syscall);
	lua_setglobal(L, "syscall");
}

const std::string& LuaSyscallLibrary::GetLuaSource()
{
	static std::string sources = R"""(
---@class syscall_result
---@field exit_code number Exit code of the command.
---@field output string Output of the command.
---@field error string Error of the command.

---@param command string Command to execute in the shell.
---@return syscall_result Result of the command.
function syscall(command)
end
)""";

	return sources;
}
