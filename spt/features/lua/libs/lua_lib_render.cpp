#include "stdafx.hpp"
#include "lua_lib_render.hpp"
#include "../../visualizations/renderer/mesh_renderer.hpp"
#include "lua_lib_math.hpp"
#include "lua_lib_events.hpp"
#include "spt/features/lua/lua_util.hpp"
#include <string>
#include <vector>
#include <map>

LuaRenderLibrary lua_render_library;

struct Mesh {
    lua_State *L;
    int ref;
    bool is_static;

    std::vector<MeshCreateFunc> appendFunctions;

    virtual ~Mesh() {
        lua_rawgeti(L, LUA_REGISTRYINDEX, ref);
        lua_pushboolean(L, false);
        lua_setfield(L, -2, "valid");
        lua_pop(L, 1);

        luaL_unref(L, LUA_REGISTRYINDEX, ref);
    }
};

struct OfflineDynamicMesh : public Mesh {

};

struct OfflineStaticMesh : public Mesh {

    ~OfflineStaticMesh() override {
        if (valid && mesh.Valid()) {
            mesh.Destroy();
        }
    }

    std::string name;
    StaticMesh mesh;
    bool valid;
    bool needsRebuild;
};

struct LuaStateRenderData {
    std::vector<OfflineDynamicMesh *> offlineDynamicMeshes;
    std::map<std::string, OfflineStaticMesh *> offlineStaticMeshes;
};

static std::map<lua_State *, LuaStateRenderData> luaStateRenderData;

static void MeshRender(MeshRendererDelegate &mr) {
    lua_events_library.InvokeEvent("render", [](lua_State *L) {
        lua_newtable(L);
    });

//    Msg("MeshRender\n");
    for (auto &entry: luaStateRenderData) {
        LuaStateRenderData &data = entry.second;
//        Msg("MeshRender, %d dynamic meshes\n", data.offlineDynamicMeshes.size());

        for (auto &meshEntry: data.offlineStaticMeshes) {
            OfflineStaticMesh *offlineMesh = meshEntry.second;

            if (offlineMesh->needsRebuild && !offlineMesh->mesh.Valid()) {
                if (offlineMesh->mesh.Valid()) {
                    offlineMesh->mesh.Destroy();
                }

                // This totally copies the mesh and deletes the original, but maybe that's fine?
                // The destructor supposedly deletes the mesh, but this is calling the destructor
                // immediately. This doesn't make any sense.
                offlineMesh->mesh = spt_meshBuilder.CreateStaticMesh([&](MeshBuilderDelegate &builder) {
                    for (auto &func: offlineMesh->appendFunctions) {
                        func(builder);
                    }
                });
                offlineMesh->valid = true;
            }

            mr.DrawMesh(offlineMesh->mesh); // TODO: Transform callback stuff
        }

        for (auto &offlineMesh: data.offlineDynamicMeshes) {
            DynamicMesh mesh = spt_meshBuilder.CreateDynamicMesh([&](MeshBuilderDelegate &builder) {
                for (auto &func: offlineMesh->appendFunctions) {
                    func(builder);
                }
            });

//            Msg("MeshRender, draw dynamic mesh\n");
            mr.DrawMesh(mesh); // TODO: Transform callback stuff
            delete offlineMesh;
        }

        data.offlineDynamicMeshes.clear();
    }
}

#define CLAMP255(x) (x < 0 ? 0 : (x > 255 ? 255 : x))

static color32 ToColor32(const Vector4D& vector) {
    return color32(
        static_cast<uint8_t>(CLAMP255(vector.x * 255)),
        static_cast<uint8_t>(CLAMP255(vector.y * 255)),
        static_cast<uint8_t>(CLAMP255(vector.z * 255)),
        static_cast<uint8_t>(CLAMP255(vector.w * 255))
    );
}

static int CreateDynamicMesh(lua_State *L) {
    if(lua_gettop(L) != 1) {
        return luaL_error(L, "Expected 1 argument, got %d", lua_gettop(L));
    }

    LuaStateRenderData &data = luaStateRenderData[L];

    if(!lua_istable(L, 1)) {
        return luaL_error(L, "Expected table as first argument, got %s", lua_typename(L, lua_type(L, 1)));
    }

    lua_pushvalue(L, 1);
    int ref = luaL_ref(L, LUA_REGISTRYINDEX);

    auto *offlineMesh = new OfflineDynamicMesh();
    offlineMesh->L = L;
    offlineMesh->ref = ref;
    offlineMesh->appendFunctions = {};
    offlineMesh->is_static = false;

    data.offlineDynamicMeshes.push_back(offlineMesh);

    lua_pushboolean(L, true);
    lua_setfield(L, 1, "valid");

    lua_pushlightuserdata(L, offlineMesh);
    return 1;
}

static int CreateStaticMesh(lua_State *L) {
    if(lua_gettop(L) != 2)
        return luaL_error(L, "Expected 2 arguments, got %d", lua_gettop(L));

    LuaStateRenderData &data = luaStateRenderData[L];
    std::string name = luaL_checkstring(L, 1);

    if(!lua_istable(L, 2)) {
        return luaL_error(L, "Expected table as second argument, got %s", lua_typename(L, lua_type(L, 2)));
    }

    if(data.offlineStaticMeshes.find(name) != data.offlineStaticMeshes.end()) {
        delete data.offlineStaticMeshes[name];
    }

    lua_pushvalue(L, 2);
    int ref = luaL_ref(L, LUA_REGISTRYINDEX);

    auto *offlineMesh = new OfflineStaticMesh();
    offlineMesh->L = L;
    offlineMesh->ref = ref;
    offlineMesh->appendFunctions = {};
    offlineMesh->is_static = true;

    offlineMesh->name = name;
    offlineMesh->needsRebuild = false; // Empty mesh, no need to rebuild, just don't draw it
    offlineMesh->valid = false;

    data.offlineStaticMeshes[name] = offlineMesh;

    lua_pushboolean(L, true);
    lua_setfield(L, 2, "valid");

    lua_pushlightuserdata(L, offlineMesh);
    return 1;
}

static int DeleteStaticMesh(lua_State *L) {
    // First parameter is string name
    if(lua_gettop(L) != 1)
        return luaL_error(L, "Expected 1 argument, got %d", lua_gettop(L));

    if(!lua_isstring(L, 1))
        return luaL_error(L, "Expected string, got %s", lua_typename(L, lua_type(L, 1)));

    if(luaStateRenderData.find(L) == luaStateRenderData.end()) {
        lua_pushboolean(L, false); // Did not delete anything
        return 1;
    }

    std::string name = luaL_checkstring(L, 1);
    LuaStateRenderData &data = luaStateRenderData[L];

    if(data.offlineStaticMeshes.find(name) == data.offlineStaticMeshes.end()) {
        lua_pushboolean(L, false); // Did not delete anything
        return 1;
    }

    OfflineStaticMesh *offlineMesh = data.offlineStaticMeshes[name];
    delete offlineMesh;

    data.offlineStaticMeshes.erase(name);

    lua_pushboolean(L, true); // Deleted something
    return 1;
}

static int DeleteAll(lua_State *L) {
    if(luaStateRenderData.find(L) == luaStateRenderData.end())
        return 0;

    LuaStateRenderData &data = luaStateRenderData[L];

    for (auto &entry: data.offlineStaticMeshes) {
        delete entry.second;
    }

    for (auto item: data.offlineDynamicMeshes) {
        delete item;
    }

    luaStateRenderData.erase(L);
    return 0;
}

static int VerifyMesh(lua_State *L, Mesh** result) {
    if(!lua_istable(L, 1) || !lua_getmetatable(L, 1)) {
        return luaL_error(L, "(1) Expected mesh as first argument, got %s", lua_typename(L, lua_type(L, 1)));
    }

    lua_getfield(L, 1, "static");
    if(!lua_isboolean(L, -1)) {
        lua_pop(L, 1);
        return luaL_error(L, "(2) Expected mesh as first argument, got %s", lua_typename(L, lua_type(L, 1)));
    }

    bool is_static = lua_toboolean(L, -1);
    lua_pop(L, 1);

    if(!LuaCheckClass(L, 1, is_static ? "static_mesh" : "dynamic_mesh")) {
        return luaL_error(L, "(3) Expected mesh as first argument, got %s", lua_typename(L, lua_type(L, 1)));
    }

    lua_getfield(L, 1, "valid");
    if(!lua_isboolean(L, -1)) {
        lua_pop(L, 1);
        return luaL_error(L, "(4) Expected mesh as first argument, got %s", lua_typename(L, lua_type(L, 1)));
    }

    bool valid = lua_toboolean(L, -1);
    lua_pop(L, 1);
    if(!valid) {
        return luaL_error(L, "(5) Mesh is not valid");
    }

    lua_getfield(L, 1, "data");
    if(!lua_islightuserdata(L, -1)) {
        lua_pop(L, 1);
        return luaL_error(L, "(6) Mesh is not valid");
    }

    Mesh *mesh = (Mesh *) lua_touserdata(L, -1);
    lua_pop(L, 1);

    if(mesh == nullptr) {
        return luaL_error(L, "(7) Mesh is not valid");
    }

    *result = mesh;
    return 0;
}

static int AddBox(lua_State *L) {
    // mesh:table, pos:vec3, min:vec3, max:vec3, face_color:vec4?, edge_color:vec4?

    if(lua_gettop(L) < 4) {
        return luaL_error(L, "Expected at least 4 arguments, got %d", lua_gettop(L));
    }

    Mesh *mesh;
    int result = VerifyMesh(L, &mesh);
    if(result != 0) {
        return result;
    }

    if(!LuaMathLibrary::LuaIsVector3D(L, 2)) {
        return luaL_error(L, "Expected vec3 as second argument (pos)");
    }

    if(!LuaMathLibrary::LuaIsVector3D(L, 3)) {
        return luaL_error(L, "Expected vec3 as third argument (min)");
    }

    if(!LuaMathLibrary::LuaIsVector3D(L, 4)) {
        return luaL_error(L, "Expected vec3 as fourth argument (max)");
    }

    Vector pos = LuaMathLibrary::LuaGetVector3D(L, 2);
    Vector min = LuaMathLibrary::LuaGetVector3D(L, 3);
    Vector max = LuaMathLibrary::LuaGetVector3D(L, 4);

    Vector4D face_color = Vector4D(1, 1, 1, 0.5);
    Vector4D edge_color = Vector4D(1, 1, 1, 1);

    if(lua_gettop(L) >= 5) {
        if(!LuaMathLibrary::LuaIsVector4D(L, 5)) {
            return luaL_error(L, "Expected vec4 as fifth argument (face_color)");
        }

        face_color = LuaMathLibrary::LuaGetVector4D(L, 5);

        if(lua_gettop(L) >= 6) {
            if(!LuaMathLibrary::LuaIsVector4D(L, 6)) {
                return luaL_error(L, "Expected vec4 as sixth argument (edge_color)");
            }

            edge_color = LuaMathLibrary::LuaGetVector4D(L, 6);
        }
    }

//    Msg("Queuing box to mesh\n");

    mesh->appendFunctions.push_back([pos, min, max, face_color, edge_color](MeshBuilderDelegate &builder) {
//        Msg("Adding box to mesh\n");
        builder.AddBox(pos, min, max, { 0.0f, 0.0f, 0.0f }, ShapeColor(
                ToColor32(face_color),
                ToColor32(edge_color),
                true, // todo
                true, // todo
                WD_CW // todo
        ));
    });

    if(mesh->is_static) {
        ((OfflineStaticMesh *) mesh)->needsRebuild = true;
    }

    return 0;
}

static const struct luaL_Reg render_class[] = {
        {"_static",    CreateStaticMesh},
        {"_dynamic",   CreateDynamicMesh},
        {"_add_box",   AddBox},
        {"delete",     DeleteStaticMesh},
        {"delete_all", DeleteAll},
        {nullptr,      nullptr}
};

LuaRenderLibrary::LuaRenderLibrary() : LuaLibrary("render") {
    spt_meshRenderer.signal.Connect(MeshRender);
}

void LuaRenderLibrary::Load(lua_State *L) {
    LuaLibrary::Load(L);

    luaStateRenderData[L] = LuaStateRenderData({}, {});

    luaL_register(L, this->name.c_str(), render_class);
    lua_pop(L, 1);
}

void LuaRenderLibrary::Unload(lua_State *L) {
    LuaLibrary::Unload(L);

    DeleteAll(L);

//    if(luaStateRenderData.find(L) == luaStateRenderData.end())
//        return;
//
//    LuaStateRenderData &data = luaStateRenderData[L];
//    luaStateRenderData.erase(L);
//
//    for (auto &entry: data.offlineStaticMeshes) {
//        if(entry.second->mesh.Valid()) {
//            entry.second->mesh.Destroy(); // TODO: Is this necessary?
//        }
//        delete entry.second;
//    }
//
//    for (auto item: data.offlineDynamicMeshes) {
//        delete item;
//    }
}

const std::string &LuaRenderLibrary::GetLuaSource() {
    static std::string sources = R"(
        ---@class render
        render = {}

        ---@class mesh
        ---@field auto_render boolean
        ---@field frame_callback fun():void
        ---@field box fun(pos:vec3, min:vec3, max:vec3, face_color:vec4, edge_color:vec4):mesh
        ---@field sphere fun(pos:vec3, radius:number, face_color:vec4, edge_color:vec4):mesh
        ---@field line fun(pos1:vec3, pos2:vec3, color:vec4):mesh
        ---@field triangle fun(pos1:vec3, pos2:vec3, pos3:vec3, color:vec4):mesh
        ---@field quad fun(pos1:vec3, pos2:vec3, pos3:vec3, pos4:vec3, color:vec4):mesh
        local mesh = {}
        function mesh:box(pos, min, max, face_color, edge_color)
            render._add_box(self, pos, min, max, face_color, edge_color)
            return self
        end

        ---@class static_mesh : mesh
        static_mesh = {}
        static_mesh.__index = mesh

        function static_mesh:delete()
            render.delete(self.name)
        end

        ---@class dynamic_mesh : mesh
        dynamic_mesh = {}
        dynamic_mesh.__index = mesh

        function render.static(name, frame_callback)
            ---@class static_mesh
            local m = {
                name = name,
                frame_callback = frame_callback,
                auto_render = true,
                static = true,
                valid = false
            }

            setmetatable(m, static_mesh)
            m.data = render._static(name, m)

            return m
        end

        function render.dynamic(frame_callback)
            ---@class dynamic_mesh
            local m = {
                frame_callback = frame_callback,
                auto_render = true,
                static = false,
                valid = false
            }

            setmetatable(m, dynamic_mesh)
            m.data = render._dynamic(m)

            return m
        end

        function render.delete(mesh_name)
        end

        function render.delete_all()
        end

        --- Can only be called during the render event
        ---@param mesh static_mesh|dynamic_mesh
        function render.draw(mesh)
        end

    )";

    return sources;
}
