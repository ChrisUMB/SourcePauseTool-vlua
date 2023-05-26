#include "stdafx.hpp"
#include "lua_lib_filesystem.hpp"
#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;

LuaFileSystemLibrary lua_filesystem_library;

LuaFileSystemLibrary::LuaFileSystemLibrary() : LuaLibrary("filesystem") {}

fs::path CheckPath(lua_State *L, int index) {
    if (!LuaIsClass(L, index, "fs.path")) {
        luaL_error(L, "Expected path");
    }

    lua_getfield(L, index, "path");
    return fs::path(luaL_checkstring(L, -1));
}

static const char *TryGetPath(lua_State *L, int index) {
    if (!LuaIsClass(L, index, "fs.path")) {
        return luaL_checkstring(L, index);
    }

    lua_getfield(L, index, "path");
    DebugLuaStack(L);
    return luaL_checkstring(L, -1);
}

static int PathFileName(lua_State *L) {
    fs::path p = CheckPath(L, 1);
    lua_pushstring(L, p.filename().string().c_str());
    return 1;
}

static int PathExtension(lua_State *L) {
    if (!LuaIsClass(L, 1, "fs.path")) {
        return luaL_error(L, "path:extension(): takes 1 argument");
    }

    lua_getfield(L, 1, "path");
    fs::path p(luaL_checkstring(L, -1));

    lua_pushstring(L, p.extension().string().c_str());
    return 1;
}

static int PathParentPath(lua_State *L) {
    if (!LuaIsClass(L, 1, "fs.path")) {
        return luaL_error(L, "path:parent_path(): takes 1 argument");
    }

    lua_getfield(L, 1, "path");
    fs::path p(luaL_checkstring(L, -1));

    lua_pushstring(L, p.parent_path().string().c_str());
    return 1;
}

static int PathIsDirectory(lua_State *L) {
    if (!LuaIsClass(L, 1, "fs.path")) {
        return luaL_error(L, "path:is_directory(): takes 1 argument");
    }

    lua_getfield(L, 1, "path");
    fs::path p(luaL_checkstring(L, -1));

    lua_pushboolean(L, fs::is_directory(p));
    return 1;
}

static const struct luaL_Reg filesystem_path_class[] = {
        {"file_name",    PathFileName},
        {"extension",    PathExtension},
        {"parent_path",  PathParentPath},
        {"is_directory", PathIsDirectory},
        {nullptr,        nullptr}
};

std::fstream *CheckFile(lua_State *L, int index) {
    if (!LuaIsClass(L, index, "fs.file")) {
        luaL_error(L, "Expected file");
    }

    lua_getfield(L, index, "file");
    return static_cast<std::fstream *>(lua_touserdata(L, -1));
}

static int FileIsOpen(lua_State *L) {
    std::fstream *file = CheckFile(L, 1);
    lua_pushboolean(L, file->is_open());
    return 1;
}

static int FileClose(lua_State *L) {
    std::fstream *file = CheckFile(L, 1);
    file->close();
    return 0;
}

static int FileReadLine(lua_State *L) {
    std::fstream *file = CheckFile(L, 1);

    std::string line;
    std::getline(*file, line);

    lua_pushstring(L, line.c_str());
    return 1;
}

static int FileReadAll(lua_State *L) {
    std::fstream *file = CheckFile(L, 1);

    file->seekg(0, std::ios::end);
    std::streampos fileSize = file->tellg();
    file->seekg(0, std::ios::beg);

    std::vector<char> buffer(fileSize);
    file->read(buffer.data(), fileSize);

    std::string data = std::string(buffer.begin(), buffer.end());
    lua_pushstring(L, data.c_str());
    return 1;
}

static int FileReadBytes(lua_State *L) {
    std::fstream *file = CheckFile(L, 1);
    unsigned int count = luaL_checkinteger(L, 2);

    std::vector<char> buffer(count);
    file->read(buffer.data(), count);

    std::string data = std::string(buffer.begin(), buffer.end());
    lua_pushstring(L, data.c_str());
    return 1;
}

static int FileWrite(lua_State *L) {
    std::fstream *file = CheckFile(L, 1);
    const char *data = luaL_checkstring(L, 2);

    file->write(data, strlen(data));
    return 0;
}

static int FileEOF(lua_State *L) {
    std::fstream *file = CheckFile(L, 1);
    lua_pushboolean(L, file->eof());
    return 1;
}

static const struct luaL_Reg filesystem_file_class[] = {
        {"is_open", FileIsOpen},
        {"close", FileClose},
        {"read_line", FileReadLine},
        {"read_all", FileReadAll},
        {"read", FileReadBytes},
        {"write", FileWrite},
        {"eof", FileEOF},
        {nullptr, nullptr}
};

static int FSFileCreate(lua_State *L) {
    const char *path = TryGetPath(L, 1);
    unsigned int mode = luaL_checkinteger(L, 2);

    std::fstream *file = new std::fstream(path, mode);

    if (!file->is_open()) {
        delete file;
        return luaL_error(L, "Failed to open file");
    }

    lua_pushlightuserdata(L, file);
    return 1;
}

static int FSFileSize(lua_State *L) {
    lua_pushinteger(L, fs::file_size(TryGetPath(L, 1)));
    return 1;
}

static int FSExists(lua_State *L) {
    lua_pushboolean(L, fs::exists(TryGetPath(L, 1)));
    return 1;
}

static int FSRename(lua_State *L) {
    fs::rename(TryGetPath(L, 1), TryGetPath(L, 2));
    return 0;
}

static int FSCreateDirectory(lua_State *L) {
    fs::create_directory(TryGetPath(L, 1));
    return 0;
}

static int FSRemove(lua_State *L) {
    fs::remove(TryGetPath(L, 1));
    return 0;
}

static int FSList(lua_State *L) {
    std::vector<std::string> files;

    const char *path = TryGetPath(L, 1);

    if (lua_isboolean(L, 2) && lua_toboolean(L, 2)) {
        for (const auto &entry: fs::recursive_directory_iterator(path)) {
            files.push_back(entry.path().string());
        }
    } else {
        for (const auto &entry: fs::directory_iterator(path)) {
            files.push_back(entry.path().string());
        }
    }

    lua_newtable(L);

    for (int i = 0; i < files.size(); i++) {
        lua_pushstring(L, files[i].c_str());
        lua_rawseti(L, -2, i + 1);
    }

    return 1;
}

static const struct luaL_Reg filesystem_class[] = {
        {"_file",            FSFileCreate},
        {"file_size",        FSFileSize},
        {"exists",           FSExists},
        {"rename",           FSRename},
        {"create_directory", FSCreateDirectory},
        {"remove",           FSRemove},
        {"list",             FSList},
        {nullptr,            nullptr}
};

void LuaFileSystemLibrary::Load(lua_State *L) {
    lua_new_class(L, "fs", filesystem_class);
    lua_new_class(L, "fs.path", filesystem_path_class);
    lua_new_class(L, "fs.file", filesystem_file_class);
}

void LuaFileSystemLibrary::Unload(lua_State *L) {
    lua_pushnil(L);
    lua_setglobal(L, "fs");
}

const std::string &LuaFileSystemLibrary::GetLuaSource() {
    static std::string sources = R"""(
---@meta
local bit = require("bit")
---@class fs
fs = {}

---@class fs.path
---@operator call(string): fs.path
fs.path = {}

function fs.path.__call(self, path)
    local p = {
        path = path
    }

    return setmetatable(p, fs.path)
end

setmetatable(fs.path, fs.path)

---@return string # The string representation of the parent folders path.
function fs.path:parent_path()
end

---@return string # The file name.
function fs.path:file_name()
end

---@return string # The file extension.
function fs.path:extension()
end

---@class fs.mode : number
fs.mode = {
    read = 0x01,
    write = 0x02,
    at_end = 0x04,
    append = 0x08,
    truncate = 0x10,
    binary = 0x20
}

---@class fs.file
---@operator call(string|fs.path): fs.file
fs.file = {}

function fs.file.__call(self, path, ...)
    local args = {...}

    if #args == 0 then
        args = {fs.mode.read}
    end

    local m = bit.bor(...)

    local f = {
        path = path,
        mode = m,
        file = fs._file(path, m)
    }

    return setmetatable(f, fs.file)
end

setmetatable(fs.file, fs.file)

---@return boolean # Whether the file is open.
function fs.file:is_open()
end

--- Closes the file.
function fs.file:close()
end

---@param str string # The string to write to the file.
function fs.file:write(str)
end

---@return string # The next line of the file.
function fs.file:read_line()
end

---@return string # The contents of the file.
function fs.file:read_all()
end

---@param n number # The number of bytes to read.
---@return string # The contents of the file read up to `n` bytes.
function fs.file:read(n)
end

---@return boolean # Whether the file is at the end.
function fs.file:eof()
end

---@param path fs.path|string # The path to the file.
---@return number # The size of the file in bytes.
function fs.file_size(path)
end

---@param path fs.path|string # The path to the file.
---@return boolean # Whether the file exists.
function fs.exists(path)
end

---@param path fs.path|string # The path of the directory to create.
function fs.create_directory(path)
end

---@param old_path fs.path|string # The path of the file to rename.
---@param new_path fs.path|string # The new path of the file.
function fs.rename(old_path, new_path)
end

---@param path fs.path|string # The path of the file or directory to remove.
function fs.remove(path)
end

---@param path fs.path|string # The path of the file or directory to remove.
---@param recurse boolean? # Whether to recurse into subdirectories, defaults to false.
---@return string[] # A list of files and directories.
function fs.list(path, recurse)
end
)""";

    return sources;
}
