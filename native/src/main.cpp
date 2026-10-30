// LÖVE 11.5 standalone host, derived from the zlib-licensed LÖVE main
// program and structured after Webmon's MIT host. Before an owner import it
// runs the bundled no-assets setup screen and its private-LAN pairing portal.
#include "Platform.h"
#include "PortalServer.h"

#include "common/runtime.h"
#include "common/version.h"
#include "modules/love/love.h"

extern "C" {
#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>
}

#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <string>
#include <unistd.h>
#include <vector>

namespace {
void preload(lua_State *state, lua_CFunction function, const char *name) {
    lua_getglobal(state, "package");
    lua_getfield(state, -1, "preload");
    lua_pushcfunction(state, function);
    lua_setfield(state, -2, name);
    lua_pop(state, 2);
}

int runLove(const std::vector<std::string> &args) {
    lua_State *state = luaL_newstate();
    luaL_openlibs(state);
    preload(state, luaopen_love_jitsetup, "love.jitsetup");
    lua_getglobal(state, "require"); lua_pushstring(state, "love.jitsetup"); lua_call(state, 1, 0);
    preload(state, luaopen_love, "love");
    lua_newtable(state);
    lua_pushstring(state, args[0].c_str()); lua_rawseti(state, -2, -2);
    lua_pushstring(state, "embedded boot.lua"); lua_rawseti(state, -2, -1);
    for (std::size_t i = 1; i < args.size(); ++i) { lua_pushstring(state, args[i].c_str()); lua_rawseti(state, -2, static_cast<int>(i)); }
    lua_setglobal(state, "arg");
    lua_getglobal(state, "require"); lua_pushstring(state, "love"); lua_call(state, 1, 1);
    lua_pushboolean(state, 1); lua_setfield(state, -2, "_exe"); lua_pop(state, 1);
    lua_getglobal(state, "require"); lua_pushstring(state, "love.boot"); lua_call(state, 1, 1);
    lua_newthread(state); lua_pushvalue(state, -2);
    const int stackPosition = lua_gettop(state);
    int results = 0;
    while (love::luax_resume(state, 0, &results) == LUA_YIELD)
        lua_pop(state, lua_gettop(state) - stackPosition);
    const int exitCode = lua_isnumber(state, -1) ? static_cast<int>(lua_tonumber(state, -1)) : 0;
    lua_close(state);
    return exitCode;
}
} // namespace

int main(int argc, char **argv) {
    if (std::strcmp(LOVE_VERSION_STRING, love_version()) != 0) {
        std::fprintf(stderr, "[smoke] LÖVE library %s does not match headers %s\n", love_version(), LOVE_VERSION_STRING);
        return 1;
    }
    const std::string dataFolder = balatro_webos::prepareDataFolder();
    balatro_webos::installRemoteHandling();
    const std::string game = dataFolder + "/game/Balatro.love";
    const bool hasImportedGame = access(game.c_str(), R_OK) == 0;
    if (hasImportedGame) balatro_webos::installCrtCompatibilityOverlay(dataFolder);
    balatro_webos::PortalServer portal(dataFolder);
    if (!hasImportedGame && portal.start()) {
        setenv("BALATRO_PORTAL_URL", portal.details().url.c_str(), 1);
        setenv("BALATRO_PORTAL_PIN", portal.details().pin.c_str(), 1);
    }
    const std::string program = hasImportedGame ? game : balatro_webos::executableFolder() + "/../share/smoke.love";
    std::printf("[host] starting %s\n", hasImportedGame ? "owner-supplied archive" : "bundled setup screen");
    const int exitCode = runLove({ argc > 0 ? argv[0] : "balatro-webos-smoke", program });
    portal.stop();
    return exitCode;
}
