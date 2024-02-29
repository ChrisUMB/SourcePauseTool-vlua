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
    lua_State* L;
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

struct OfflineDynamicMesh : public Mesh {};

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
    std::vector<OfflineDynamicMesh*> offlineDynamicMeshes;
    std::map<std::string, OfflineStaticMesh*> offlineStaticMeshes;
};

static std::map<lua_State*, LuaStateRenderData> luaStateRenderData;

static void MeshRender(MeshRendererDelegate& mr) {
    lua_events_library.InvokeEvent("render", [](lua_State* L) { lua_newtable(L); });

    //    Msg("MeshRender\n");
    for (auto& entry : luaStateRenderData) {
        LuaStateRenderData& data = entry.second;
        //        Msg("MeshRender, %d dynamic meshes\n", data.offlineDynamicMeshes.size());

        for (auto& meshEntry : data.offlineStaticMeshes) {
            OfflineStaticMesh* offlineMesh = meshEntry.second;
            if (offlineMesh->appendFunctions.empty()) {
                continue;
            }

            if (offlineMesh->needsRebuild) {
                if (offlineMesh->mesh.Valid()) {
                    offlineMesh->mesh.Destroy();
                }

                // This totally copies the mesh and deletes the original, but maybe that's fine?
                // The destructor supposedly deletes the mesh, but this is calling the destructor
                // immediately. This doesn't make any sense.
                offlineMesh->mesh = spt_meshBuilder.CreateStaticMesh(
                    [&](MeshBuilderDelegate& builder) {
                        for (auto& func : offlineMesh->appendFunctions) {
                            func(builder);
                        }
                    });
                offlineMesh->valid = true;
                offlineMesh->needsRebuild = false;
            }

            if (!offlineMesh->valid || !offlineMesh->mesh.Valid()) {
                continue;
            }

            mr.DrawMesh(offlineMesh->mesh); // TODO: Transform callback stuff
        }

        for (auto& offlineMesh : data.offlineDynamicMeshes) {
            DynamicMesh mesh = spt_meshBuilder.CreateDynamicMesh(
                [&](MeshBuilderDelegate& builder) {
                    for (auto& func : offlineMesh->appendFunctions) {
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
    return color32(static_cast<uint8_t>(CLAMP255(vector.x * 255)),
                   static_cast<uint8_t>(CLAMP255(vector.y * 255)),
                   static_cast<uint8_t>(CLAMP255(vector.z * 255)),
                   static_cast<uint8_t>(CLAMP255(vector.w * 255)));
}

static int CreateDynamicMesh(lua_State* L) {
    if (lua_gettop(L) != 1) {
        return luaL_error(L, "Expected 1 argument, got %d", lua_gettop(L));
    }

    LuaStateRenderData& data = luaStateRenderData[L];

    if (!lua_istable(L, 1)) {
        return luaL_error(L, "Expected table as first argument, got %s", lua_typename(L, lua_type(L, 1)));
    }

    lua_pushvalue(L, 1);
    int ref = luaL_ref(L, LUA_REGISTRYINDEX);

    auto* offlineMesh = new OfflineDynamicMesh();
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

static int CreateStaticMesh(lua_State* L) {
    if (lua_gettop(L) != 2)
        return luaL_error(L, "Expected 2 arguments, got %d", lua_gettop(L));

    LuaStateRenderData& data = luaStateRenderData[L];
    std::string name = luaL_checkstring(L, 1);

    if (!lua_istable(L, 2)) {
        return luaL_error(L, "Expected table as second argument, got %s", lua_typename(L, lua_type(L, 2)));
    }

    if (data.offlineStaticMeshes.find(name) != data.offlineStaticMeshes.end()) {
        delete data.offlineStaticMeshes[name];
    }

    lua_pushvalue(L, 2);
    int ref = luaL_ref(L, LUA_REGISTRYINDEX);

    auto* offlineMesh = new OfflineStaticMesh();
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

static int DeleteStaticMesh(lua_State* L) {
    // First parameter is string name
    if (lua_gettop(L) != 1)
        return luaL_error(L, "Expected 1 argument, got %d", lua_gettop(L));

    if (!lua_isstring(L, 1))
        return luaL_error(L, "Expected string, got %s", lua_typename(L, lua_type(L, 1)));

    if (luaStateRenderData.find(L) == luaStateRenderData.end()) {
        lua_pushboolean(L, false); // Did not delete anything
        return 1;
    }

    std::string name = luaL_checkstring(L, 1);
    LuaStateRenderData& data = luaStateRenderData[L];

    if (data.offlineStaticMeshes.find(name) == data.offlineStaticMeshes.end()) {
        lua_pushboolean(L, false); // Did not delete anything
        return 1;
    }

    OfflineStaticMesh* offlineMesh = data.offlineStaticMeshes[name];
    delete offlineMesh;

    data.offlineStaticMeshes.erase(name);

    lua_pushboolean(L, true); // Deleted something
    return 1;
}

static int DeleteAll(lua_State* L) {
    if (luaStateRenderData.find(L) == luaStateRenderData.end())
        return 0;

    LuaStateRenderData& data = luaStateRenderData[L];

    for (auto& entry : data.offlineStaticMeshes) {
        delete entry.second;
    }

    for (auto item : data.offlineDynamicMeshes) {
        delete item;
    }

    luaStateRenderData.erase(L);
    return 0;
}

static int VerifyMesh(lua_State* L, Mesh** result) {
    if (!lua_istable(L, 1) || !lua_getmetatable(L, 1)) {
        return luaL_error(L, "(1) Expected mesh as first argument, got %s", lua_typename(L, lua_type(L, 1)));
    }

    lua_getfield(L, 1, "static");
    if (!lua_isboolean(L, -1)) {
        lua_pop(L, 1);
        return luaL_error(L, "(2) Expected mesh as first argument, got %s", lua_typename(L, lua_type(L, 1)));
    }

    bool is_static = lua_toboolean(L, -1);
    lua_pop(L, 1);

    if (!LuaCheckClass(L, 1, is_static ? "static_mesh" : "dynamic_mesh")) {
        return luaL_error(L, "(3) Expected mesh as first argument, got %s", lua_typename(L, lua_type(L, 1)));
    }

    lua_getfield(L, 1, "valid");
    if (!lua_isboolean(L, -1)) {
        lua_pop(L, 1);
        return luaL_error(L, "(4) Expected mesh as first argument, got %s", lua_typename(L, lua_type(L, 1)));
    }

    bool valid = lua_toboolean(L, -1);
    lua_pop(L, 1);
    if (!valid) {
        return luaL_error(L, "(5) Mesh is not valid");
    }

    lua_getfield(L, 1, "data");
    if (!lua_islightuserdata(L, -1)) {
        lua_pop(L, 1);
        return luaL_error(L, "(6) Mesh is not valid");
    }

    Mesh* mesh = (Mesh*)lua_touserdata(L, -1);
    lua_pop(L, 1);

    if (mesh == nullptr) {
        return luaL_error(L, "(7) Mesh is not valid");
    }

    *result = mesh;
    return 0;
}

static int ParseShapeColor(lua_State* L, int index, ShapeColor& result) {
    if (lua_gettop(L) < index) {
        return 0;
    }

    if (!lua_isnil(L, index)) {
        if (!LuaMathLibrary::LuaIsVector4D(L, index)) {
            return luaL_error(L, "Expected vec4 as argument %d (face_color)", index);
        }

        result.faceColor = ToColor32(LuaMathLibrary::LuaGetVector4D(L, index));
    }

    if (lua_gettop(L) < index + 1) {
        return 0;
    }

    if (!lua_isnil(L, index + 1)) {
        if (!LuaMathLibrary::LuaIsVector4D(L, index + 1)) {
            return luaL_error(L, "Expected vec4 as argument %d (line_color)", index + 1);
        }

        result.lineColor = ToColor32(LuaMathLibrary::LuaGetVector4D(L, index + 1));
    }

    if (lua_gettop(L) < index + 2) {
        return 0;
    }

    if (!lua_isnil(L, index + 2)) {
        if (!lua_isboolean(L, index + 2)) {
            return luaL_error(L, "Expected boolean as argument %d (depth_test_face)", index + 2);
        }

        result.zTestFaces = lua_toboolean(L, index + 2);
    }

    if (lua_gettop(L) < index + 3) {
        return 0;
    }

    if (!lua_isnil(L, index + 3)) {
        if (!lua_isboolean(L, index + 3)) {
            return luaL_error(L, "Expected boolean as argument %d (depth_test_line)", index + 3);
        }

        result.zTestLines = lua_toboolean(L, index + 3);
    }

    return 0;
}

static int ParseLineColor(lua_State* L, int index, LineColor& result) {
    if (lua_gettop(L) < index) {
        return 0;
    }

    if (!lua_isnil(L, index)) {
        if (!LuaMathLibrary::LuaIsVector4D(L, index)) {
            return luaL_error(L, "Expected vec4 as argument %d (line_color)", index);
        }

        result = ToColor32(LuaMathLibrary::LuaGetVector4D(L, index));
    }

    if (lua_gettop(L) < index + 1) {
        return 0;
    }

    if (!lua_isnil(L, index + 1)) {
        if (!lua_isboolean(L, index + 1)) {
            return luaL_error(L, "Expected boolean as argument %d (depth_test_line)", index + 1);
        }

        result.zTestLines = lua_toboolean(L, index + 1);
    }

    return 0;
}

static int AddBox(lua_State* L) {
    if (lua_gettop(L) < 4) {
        return luaL_error(L, "Expected at least 4 arguments, got %d", lua_gettop(L));
    }

    Mesh* mesh;
    int result = VerifyMesh(L, &mesh);
    if (result != 0) {
        return result;
    }

    if (!LuaMathLibrary::LuaIsVector3D(L, 2)) {
        return luaL_error(L, "Expected vec3 as second argument (pos)");
    }

    if (!LuaMathLibrary::LuaIsVector3D(L, 3)) {
        return luaL_error(L, "Expected vec3 as third argument (min)");
    }

    if (!LuaMathLibrary::LuaIsVector3D(L, 4)) {
        return luaL_error(L, "Expected vec3 as fourth argument (max)");
    }

    Vector pos = LuaMathLibrary::LuaGetVector3D(L, 2);
    Vector min = LuaMathLibrary::LuaGetVector3D(L, 3);
    Vector max = LuaMathLibrary::LuaGetVector3D(L, 4);

    ShapeColor shapeColor(color32(255, 255, 255, 128), color32(255, 255, 255, 255), true, true);

    ParseShapeColor(L, 5, shapeColor);

    mesh->appendFunctions.emplace_back(
        [pos, min, max, shapeColor](MeshBuilderDelegate& builder) {
            builder.AddBox(pos, min, max, {0.0f, 0.0f, 0.0f}, shapeColor);
        });

    if (mesh->is_static) {
        ((OfflineStaticMesh*)mesh)->needsRebuild = true;
    }

    return 0;
}

static int AddSphere(lua_State* L) {
    if (lua_gettop(L) < 4) {
        return luaL_error(L, "Expected at least 4 arguments, got %d", lua_gettop(L));
    }

    Mesh* mesh;
    int result = VerifyMesh(L, &mesh);
    if (result != 0) {
        return result;
    }

    if (!LuaMathLibrary::LuaIsVector3D(L, 2)) {
        return luaL_error(L, "Expected vec3 as second argument (pos)");
    }

    if (!lua_isnumber(L, 3)) {
        return luaL_error(L, "Expected number as third argument (radius)");
    }

    if (!lua_isnumber(L, 4)) {
        return luaL_error(L, "Expected number as fourth argument (segments)");
    }

    Vector pos = LuaMathLibrary::LuaGetVector3D(L, 2);
    float radius = (float)lua_tonumber(L, 3);
    int segments = lua_tointeger(L, 4);

    ShapeColor shapeColor(color32(255, 255, 255, 128), color32(255, 255, 255, 255), true, true);

    ParseShapeColor(L, 5, shapeColor);

    mesh->appendFunctions.emplace_back([pos, radius, segments, shapeColor](MeshBuilderDelegate& builder) {
        builder.AddSphere(pos, radius, segments, shapeColor);
    });

    if (mesh->is_static) {
        ((OfflineStaticMesh*)mesh)->needsRebuild = true;
    }

    return 0;
}

static int AddLine(lua_State* L) {
    if (lua_gettop(L) < 3) {
        return luaL_error(L, "Expected at least 3 arguments, got %d", lua_gettop(L));
    }

    Mesh* mesh;
    int result = VerifyMesh(L, &mesh);
    if (result != 0) {
        return result;
    }

    if (!LuaMathLibrary::LuaIsVector3D(L, 2)) {
        return luaL_error(L, "Expected vec3 as second argument (start)");
    }

    if (!LuaMathLibrary::LuaIsVector3D(L, 3)) {
        return luaL_error(L, "Expected vec3 as third argument (end)");
    }

    Vector start = LuaMathLibrary::LuaGetVector3D(L, 2);
    Vector end = LuaMathLibrary::LuaGetVector3D(L, 3);

    LineColor lineColor(color32(255, 255, 255, 255), true);

    ParseLineColor(L, 4, lineColor);

    mesh->appendFunctions.emplace_back([start, end, lineColor](MeshBuilderDelegate& builder) {
        builder.AddLine(start, end, lineColor);
    });

    if (mesh->is_static) {
        ((OfflineStaticMesh*)mesh)->needsRebuild = true;
    }

    return 0;
}

static int AddTriangle(lua_State* L) {
    if (lua_gettop(L) < 4) {
        return luaL_error(L, "Expected at least 4 arguments, got %d", lua_gettop(L));
    }

    Mesh* mesh;
    int result = VerifyMesh(L, &mesh);
    if (result != 0) {
        return result;
    }

    if (!LuaMathLibrary::LuaIsVector3D(L, 2)) {
        return luaL_error(L, "Expected vec3 as second argument (a)");
    }

    if (!LuaMathLibrary::LuaIsVector3D(L, 3)) {
        return luaL_error(L, "Expected vec3 as third argument (b)");
    }

    if (!LuaMathLibrary::LuaIsVector3D(L, 4)) {
        return luaL_error(L, "Expected vec3 as fourth argument (c)");
    }

    Vector a = LuaMathLibrary::LuaGetVector3D(L, 2);
    Vector b = LuaMathLibrary::LuaGetVector3D(L, 3);
    Vector c = LuaMathLibrary::LuaGetVector3D(L, 4);

    ShapeColor shapeColor(color32(255, 255, 255, 128), color32(255, 255, 255, 255), true, true);

    ParseShapeColor(L, 5, shapeColor);

    mesh->appendFunctions.emplace_back([a, b, c, shapeColor](MeshBuilderDelegate& builder) {
        builder.AddTri(a, b, c, shapeColor);
    });

    if (mesh->is_static) {
        ((OfflineStaticMesh*)mesh)->needsRebuild = true;
    }

    return 0;
}

static int AddQuad(lua_State* L) {
    if (lua_gettop(L) < 5) {
        return luaL_error(L, "Expected at least 5 arguments, got %d", lua_gettop(L));
    }

    Mesh* mesh;
    int result = VerifyMesh(L, &mesh);
    if (result != 0) {
        return result;
    }

    if (!LuaMathLibrary::LuaIsVector3D(L, 2)) {
        return luaL_error(L, "Expected vec3 as second argument (a)");
    }

    if (!LuaMathLibrary::LuaIsVector3D(L, 3)) {
        return luaL_error(L, "Expected vec3 as third argument (b)");
    }

    if (!LuaMathLibrary::LuaIsVector3D(L, 4)) {
        return luaL_error(L, "Expected vec3 as fourth argument (c)");
    }

    if (!LuaMathLibrary::LuaIsVector3D(L, 5)) {
        return luaL_error(L, "Expected vec3 as fifth argument (d)");
    }

    Vector a = LuaMathLibrary::LuaGetVector3D(L, 2);
    Vector b = LuaMathLibrary::LuaGetVector3D(L, 3);
    Vector c = LuaMathLibrary::LuaGetVector3D(L, 4);
    Vector d = LuaMathLibrary::LuaGetVector3D(L, 5);

    ShapeColor shapeColor(color32(255, 255, 255, 128), color32(255, 255, 255, 255), true, true);

    ParseShapeColor(L, 6, shapeColor);

    mesh->appendFunctions.emplace_back([a, b, c, d, shapeColor](MeshBuilderDelegate& builder) {
        builder.AddQuad(a, b, c, d, shapeColor);
    });

    if (mesh->is_static) {
        ((OfflineStaticMesh*)mesh)->needsRebuild = true;
    }

    return 0;
}

static int AddLineStrip(lua_State* L) {
    // vec3[] points, boolean loop, vec4 lineColor, boolean depthTest
    if (lua_gettop(L) < 2) {
        return luaL_error(L, "Expected at least 2 arguments, got %d", lua_gettop(L));
    }

    Mesh* mesh;
    int result = VerifyMesh(L, &mesh);
    if (result != 0) {
        return result;
    }

    if (!lua_istable(L, 2)) {
        return luaL_error(L, "Expected table as second argument (points)");
    }

    LineColor lineColor(color32(255, 255, 255, 255), true);

    bool loop = false;
    if (lua_gettop(L) >= 3) {
        if (!lua_isboolean(L, 3)) {
            return luaL_error(L, "Expected boolean as third argument (loop)");
        }
        loop = lua_toboolean(L, 3);

        ParseLineColor(L, 4, lineColor);
    }

    std::vector<Vector> points;
    //lua_rawlen not available in Lua 5.1
    int numPoints = (int)lua_objlen(L, 2);

    for (int i = 1; i <= numPoints; i++) {
        lua_rawgeti(L, 2, i);
        if (!LuaMathLibrary::LuaIsVector3D(L, -1)) {
            return luaL_error(L, "Expected vec3 as element %d of second argument (points)", i);
        }
        points.push_back(LuaMathLibrary::LuaGetVector3D(L, -1));
        lua_pop(L, 1);
    }

    mesh->appendFunctions.emplace_back([points, numPoints, loop, lineColor](MeshBuilderDelegate& builder) {
        builder.AddLineStrip(points.data(), numPoints, loop, lineColor);
    });

    if (mesh->is_static) {
        ((OfflineStaticMesh*)mesh)->needsRebuild = true;
    }

    return 0;
}

static Vector HSBtoRGB(Vector hsb) {
    float hue = hsb.x;
    float saturation = hsb.y;
    float brightness = hsb.z;

    if (saturation == 0) {
        return {brightness, brightness, brightness};
    } else {
        float hueSector = hue * 6.0f;
        int sectorNumber = (int)floor(hueSector);
        float fractionalSector = hueSector - (float)sectorNumber;
        float p = brightness * (1 - saturation);
        float q = brightness * (1 - saturation * fractionalSector);
        float t = brightness * (1 - saturation * (1 - fractionalSector));

        switch (sectorNumber) {
        case 0:
            return {brightness, t, p};
        case 1:
            return {q, brightness, p};
        case 2:
            return {p, brightness, t};
        case 3:
            return {p, q, brightness};
        case 4:
            return {t, p, brightness};
        case 5:
            return {brightness, p, q};
        default:
            return {0, 0, 0};
        }
    }
}

static int HSB(lua_State* L) {
    if (lua_gettop(L) == 1) {
        // expect vec3
        if (!LuaMathLibrary::LuaIsVector3D(L, 1)) {
            return luaL_error(L, "Expected vec3 as first argument");
        }

        Vector hsb = LuaMathLibrary::LuaGetVector3D(L, 1);
        Vector rgb = HSBtoRGB(hsb);
        LuaMathLibrary::LuaPushVector3D(L, rgb);
        return 1;
    } else if (lua_gettop(L) == 3) {
        // expect 3 floats, return vec3
        Vector hsb = {
            (float)luaL_checknumber(L, 1),
            (float)luaL_checknumber(L, 2),
            (float)luaL_checknumber(L, 3)
        };
        Vector rgb = HSBtoRGB(hsb);
        LuaMathLibrary::LuaPushVector3D(L, rgb);
        return 1;
    } else {
        return luaL_error(L, "Expected 1 (vec3) or 3 (number) arguments, got %d", lua_gettop(L));
    }
}

static const struct luaL_Reg render_class[] = {
    {"_static", CreateStaticMesh},
    {"_dynamic", CreateDynamicMesh},
    {"_add_box", AddBox},
    {"_add_sphere", AddSphere},
    {"_add_line", AddLine},
    {"_add_triangle", AddTriangle},
    {"_add_quad", AddQuad},
    {"_add_line_strip", AddLineStrip},
    {"hsb", HSB},
    {"delete", DeleteStaticMesh},
    {"delete_all", DeleteAll},
    {nullptr, nullptr}
};

LuaRenderLibrary::LuaRenderLibrary() : LuaLibrary("render") {
    spt_meshRenderer.signal.Connect(MeshRender);
}

void LuaRenderLibrary::Load(lua_State* L) {
    LuaLibrary::Load(L);

    luaStateRenderData[L] = LuaStateRenderData({}, {});
    luaL_register(L, this->name.c_str(), render_class);
    lua_pop(L, 1);
}

void LuaRenderLibrary::Unload(lua_State* L) {
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

const std::string& LuaRenderLibrary::GetLuaSource() {
    static std::string sources = R"(
        ---@class render
        render = {}

        ---@class mesh
        ---@field auto_render boolean
        ---@field frame_callback fun():void
        local mesh = {}

        ---@param pos vec3
        ---@param min vec3
        ---@param max vec3
        ---@param face_color vec4|nil
        ---@param edge_color vec4|nil
        ---@param depth_test_face boolean|nil
        ---@param depth_test_edge boolean|nil
        function mesh:box(pos, min, max, face_color, edge_color, depth_test_face, depth_test_edge)
            render._add_box(self, pos, min, max, face_color, edge_color, depth_test_face, depth_test_edge)
            return self
        end

        ---@param pos vec3
        ---@param radius number
        ---@param segments number
        ---@param face_color vec4|nil
        ---@param edge_color vec4|nil
        ---@param depth_test_face boolean|nil
        ---@param depth_test_edge boolean|nil
        function mesh:sphere(pos, radius, segments, face_color, edge_color, depth_test_face, depth_test_edge)
            render._add_sphere(self, pos, radius, segments, face_color, edge_color, depth_test_face, depth_test_edge)
            return self
        end

        ---@param pos1 vec3
        ---@param pos2 vec3
        ---@param color vec4|nil
        ---@param depth_test boolean|nil
        function mesh:line(pos1, pos2, color, depth_test)
            render._add_line(self, pos1, pos2, color, depth_test)
            return self
        end

        ---@param pos1 vec3
        ---@param pos2 vec3
        ---@param pos3 vec3
        ---@param face_color vec4|nil
        ---@param edge_color vec4|nil
        ---@param depth_test_face boolean|nil
        ---@param depth_test_edge boolean|nil
        function mesh:triangle(pos1, pos2, pos3, face_color, edge_color, depth_test_face, depth_test_edge)
            render._add_triangle(self, pos1, pos2, pos3, face_color, edge_color, depth_test_face, depth_test_edge)
            return self
        end

        ---@param pos1 vec3
        ---@param pos2 vec3
        ---@param pos3 vec3
        ---@param pos4 vec3
        ---@param face_color vec4|nil
        ---@param edge_color vec4|nil
        ---@param depth_test_face boolean|nil
        ---@param depth_test_edge boolean|nil
        function mesh:quad(pos1, pos2, pos3, pos4, face_color, edge_color, depth_test_face, depth_test_edge)
            render._add_quad(self, pos1, pos2, pos3, pos4, face_color, edge_color, depth_test_face, depth_test_edge)
            return self
        end

        ---@param positions vec3[]
        ---@param loop boolean|nil
        ---@param color vec4|nil
        ---@param depth_test boolean|nil
        function mesh:line_strip(positions, loop, color, depth_test)
            render._add_line_strip(self, positions, loop, color, depth_test)
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

        ---@overload fun(hsb: vec3): vec3
        ---@return vec3
        function render.hsb(h, s, b)
        end

        function render.hsba(...)
            local v = vec4(...)
            local hsb = render.hsb(v.x, v.y, v.z)
            return vec4(hsb.x, hsb.y, hsb.z, v.w)
        end

        --- Can only be called during the render event
        ---@param mesh static_mesh|dynamic_mesh
        function render.draw(mesh)
        end

    )";

    return sources;
}
