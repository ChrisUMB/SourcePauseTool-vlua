#include "stdafx.hpp"
#include "lua_lib_hud.hpp"
#include "spt/features/lua/lua_util.hpp"
#include "spt/features/lua/libs/lua_lib_events.hpp"
#include "spt/features/lua/libs/lua_lib_math.hpp"
#include "spt/features/hud.hpp"
#include "interfaces.hpp"

LuaHudLibrary lua_hud_library;
static int hudR, hudG, hudB, hudA;
vgui::HFont hudFont;

static void RenderLuaHud() {
//    ConMsg("RenderLuaHud\n");
//    IMatSystemSurface *surface = interfaces::surface;
    interfaces::surface->DrawSetColor(hudR = 255, hudG = 255, hudB = 255, hudA = 255);
    spt_hud_feat.GetFont(FONT_Trebuchet20, hudFont);
    interfaces::surface->DrawSetTextFont(hudFont);
    interfaces::surface->DrawSetTextColor(hudR, hudG, hudB, hudA);
    lua_events_library.InvokeEvent("hud", [](lua_State* L) { lua_newtable(L); });
}

static int HudGetWidth(lua_State* L) {
    lua_pushinteger(L, spt_hud_feat.renderView->width);
    return 1;
}

static int HudGetHeight(lua_State* L) {
    lua_pushinteger(L, spt_hud_feat.renderView->height);
    return 1;
}

static int HudGetSize(lua_State* L) {
    LuaMathLibrary::LuaPushVector2D(L, Vector2D((float) spt_hud_feat.renderView->width, (float) spt_hud_feat.renderView->height));
    return 2;
}

static int HudSetColor(lua_State* L) {
    if(lua_gettop(L) != 1) {
        return luaL_error(L, "hud.color: expected 1 argument, got %d", lua_gettop(L));
    }

    if (lua_isnumber(L, 1)) {
        // Expect hex ARGB (apparently can't use lua_tointeger here because it discards some data)
        auto color = static_cast<unsigned int>(lua_tonumber(L, 1));
        hudR = (int) ((color >> 24u) & 0xFFu);
        hudG = (int) ((color >> 16u) & 0xFFu);
        hudB = (int) ((color >> 8u) & 0xFFu);
        hudA = (int) ((color) & 0xFFu);
    } else if (LuaMathLibrary::LuaIsVector4D(L, 1)) {
        const Vector4D &color = LuaMathLibrary::LuaGetVector4D(L, 1);
        hudR = ((int) (color.x * 255)) & 0xFF;
        hudG = ((int) (color.y * 255)) & 0xFF;
        hudB = ((int) (color.z * 255)) & 0xFF;
        hudA = ((int) (color.w * 255)) & 0xFF;
    } else if (LuaMathLibrary::LuaIsVector3D(L, 1)) {
        const Vector &color = LuaMathLibrary::LuaGetVector3D(L, 1);
        hudR = ((int) (color.x * 255)) & 0xFF;
        hudG = ((int) (color.y * 255)) & 0xFF;
        hudB = ((int) (color.z * 255)) & 0xFF;
        hudA = 255;
    } else {
        return luaL_error(L, "hud.color: argument 1 is not a number, vec3 or vec4");
    }

    interfaces::surface->DrawSetColor(hudR, hudG, hudB, hudA);
    return 0;
}

static int HudFillRect(lua_State* L) {
    if(lua_gettop(L) != 4) {
        return luaL_error(L, "hud.fillRect: expected 4 arguments, got %d", lua_gettop(L));
    }

    if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2) || !lua_isnumber(L, 3) || !lua_isnumber(L, 4)) {
        return luaL_error(L, "hud.fillRect: arguments 1-4 are not numbers");
    }

    int minX = lua_tointeger(L, 1);
    int minY = lua_tointeger(L, 2);
    int maxX = lua_tointeger(L, 3);
    int maxY = lua_tointeger(L, 4);

    interfaces::surface->DrawFilledRect(minX, minY, maxX, maxY);
    return 0;
}

static int HudOutlineRect(lua_State* L) {
    if(lua_gettop(L) != 4) {
        return luaL_error(L, "hud.outlineRect: expected 4 arguments, got %d", lua_gettop(L));
    }

    if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2) || !lua_isnumber(L, 3) || !lua_isnumber(L, 4)) {
        return luaL_error(L, "hud.outlineRect: arguments 1-4 are not numbers");
    }

    int minX = lua_tointeger(L, 1);
    int minY = lua_tointeger(L, 2);
    int maxX = lua_tointeger(L, 3);
    int maxY = lua_tointeger(L, 4);

    interfaces::surface->DrawOutlinedRect(minX, minY, maxX, maxY);
    return 0;
}

static int HudDrawLine(lua_State* L) {
    if (lua_gettop(L) != 4) {
        return luaL_error(L, "hud.drawLine: expected 4 arguments, got %d", lua_gettop(L));
    }

    if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2) || !lua_isnumber(L, 3) || !lua_isnumber(L, 4)) {
        return luaL_error(L, "hud.drawLine: arguments 1-4 are not numbers");
    }

    int startX = lua_tointeger(L, 1);
    int startY = lua_tointeger(L, 2);
    int endX = lua_tointeger(L, 3);
    int endY = lua_tointeger(L, 4);

    interfaces::surface->DrawLine(startX, startY, endX, endY);
    return 0;
}

// x, y, radius
//static int HudFillCircle(lua_State* L) {
//    if (lua_gettop(L) != 3) {
//        return luaL_error(L, "hud.fillCircle: expected 3 arguments, got %d", lua_gettop(L));
//    }
//
//    if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2) || !lua_isnumber(L, 3)) {
//        return luaL_error(L, "hud.fillCircle: arguments 1-3 are not numbers");
//    }
//
//    int x = lua_tointeger(L, 1);
//    int y = lua_tointeger(L, 2);
//    auto radius = (float) lua_tonumber(L, 3);
//
//    interfaces::surface->DrawColoredCircle(x, y, radius, hudR, hudG, hudB, hudA);
//    return 0;
//}

static int HudOutlineCircle(lua_State* L) {
    if (lua_gettop(L) != 4) {
        return luaL_error(L, "hud.outlineCircle: expected 4 arguments, got %d", lua_gettop(L));
    }

    if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2) || !lua_isnumber(L, 3) || !lua_isnumber(L, 4)) {
        return luaL_error(L, "hud.outlineCircle: arguments 1-4 are not numbers");
    }

    int x = lua_tointeger(L, 1);
    int y = lua_tointeger(L, 2);
    int radius = lua_tointeger(L, 3);
    int segments = lua_tointeger(L, 4);

    interfaces::surface->DrawOutlinedCircle(x, y, radius, segments);
    return 0;
}

static int HudDrawText(lua_State* L) {
    if (lua_gettop(L) < 3) {
        return luaL_error(L, "hud.drawText: expected 3 or more arguments, got %d", lua_gettop(L));
    }

    if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2) || !lua_isstring(L, 3)) {
        return luaL_error(L, "hud.drawText: arguments 1-2 are not numbers, argument 3 is not a string (hint: use formatting specifiers)");
    }

    int x = lua_tointeger(L, 1);
    int y = lua_tointeger(L, 2);

    const char* text = luaL_checkstring(L, 3);
//    lua_remove(L, 1);
//    lua_remove(L, 1);
//    lua_string_format(L);

    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    std::wstring wide = converter.from_bytes(text);
    interfaces::surface->DrawSetTextPos(x, y);
    interfaces::surface->DrawSetTextColor(hudR, hudG, hudB, hudA);
    interfaces::surface->DrawPrintText(wide.c_str(), (int) wide.length());
//    char fmt[3] = "%s";
//    interfaces::surface->DrawColoredText(hudFont, x, y, hudR, hudG, hudB, hudA, fmt, text);
    return 0;
}

static int HudMeasureText(lua_State* L) {
    if (lua_gettop(L) < 1) {
        return luaL_error(L, "hud.measureText: expected 1 or more arguments, got %d", lua_gettop(L));
    }

    if (!lua_isstring(L, 1)) {
        return luaL_error(L, "hud.measureText: argument 1 is not a string (hint: use formatting specifiers)");
    }

    const char* text = luaL_checkstring(L, 1);

    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    std::wstring wide = converter.from_bytes(text);
    int width, height;
    interfaces::surface->GetTextSize(hudFont, wide.c_str(), width, height);
    LuaMathLibrary::LuaPushVector2D(L, Vector2D((float) width, (float) height));
    return 1;
}

static const struct luaL_Reg hud_class[] = {
        {"width", HudGetWidth},
        {"height", HudGetHeight},
        {"size", HudGetSize},
        {"color", HudSetColor},
        {"fillRect", HudFillRect},
        {"outlineRect", HudOutlineRect},
        {"drawLine", HudDrawLine},
//        {"fillCircle", HudFillCircle},
        {"outlineCircle", HudOutlineCircle},
        {"drawText", HudDrawText},
        {"measureText", HudMeasureText},
        {nullptr, nullptr}
};

LuaHudLibrary::LuaHudLibrary() : LuaLibrary("hud")
{

}

void LuaHudLibrary::Init() {
    bool result = spt_hud_feat.AddHudDefaultGroup(HudCallback(
            [](const auto& unknown) { return RenderLuaHud(); },
            []() { return true; },
            false
    ));

    if(result) {
        ConMsg("SPT LuaHudLibrary: Added hud group\n");
    } else {
        ConMsg("SPT LuaHudLibrary: Failed to add hud group\n");
    }
}

void LuaHudLibrary::Load(lua_State* L)
{
    LuaLibrary::Load(L);

    luaL_register(L, this->name.c_str(), hud_class);
    lua_pop(L, 1);
}

void LuaHudLibrary::Unload(lua_State* L) {
    LuaLibrary::Unload(L);
}

const std::string& LuaHudLibrary::GetLuaSource()
{
    static std::string sources = R"(
        ---@class hud
        hud = {}

        ---@return number
        function hud.width() end

        ---@return number
        function hud.height() end

        ---@return vec2
        function hud.size() end

        ---@param color number|vec3|vec4
        function hud.color(color) end

        ---@param minX number
        ---@param minY number
        ---@param maxX number
        ---@param maxY number
        function hud.fillRect(minX, minY, maxX, maxY) end

        ---@param minX number
        ---@param minY number
        ---@param maxX number
        ---@param maxY number
        function hud.outlineRect(minX, minY, maxX, maxY) end

        ---@param startX number
        ---@param startY number
        ---@param endX number
        ---@param endY number
        function hud.drawLine(startX, startY, endX, endY) end

        ---@param x number
        ---@param y number
        ---@param radius number
        ---@param segments number
        function hud.outlineCircle(x, y, radius, segments) end
    )";

    return sources;
}