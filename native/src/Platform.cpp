// Small, project-authored webOS platform layer. The remote scancodes and SDL
// access-policy hints follow Webmon's MIT-licensed Platform.cpp (attribution
// is in THIRD_PARTY_NOTICES.md); this implementation intentionally has no
// game bridge, network listener, root action, or file-import code.
#include "Platform.h"

#include <SDL.h>

#include <cerrno>
#include <climits>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <sys/stat.h>
#include <unistd.h>

namespace balatro_webos {
namespace {
constexpr int kWebOSBack = 482;
constexpr int kWebOSRed = 486;
constexpr int kWebOSGreen = 487;
constexpr int kWebOSExit = 505;

bool ensureFolder(const std::string &path) { return mkdir(path.c_str(), 0755) == 0 || errno == EEXIST; }

int remoteEventFilter(void *, SDL_Event *event) {
    if (event->type != SDL_KEYDOWN && event->type != SDL_KEYUP)
        return 1;
    const int scancode = event->key.keysym.scancode;
    if (scancode == kWebOSExit) {
        if (event->type == SDL_KEYDOWN) {
            event->type = SDL_QUIT;
            return 1;
        }
        return 0;
    }
    if (scancode == kWebOSBack || scancode == kWebOSRed) {
        event->key.keysym.scancode = SDL_SCANCODE_X;
        event->key.keysym.sym = SDLK_x;
    } else if (scancode == kWebOSGreen) {
        event->key.keysym.scancode = SDL_SCANCODE_RETURN;
        event->key.keysym.sym = SDLK_RETURN;
    }
    return 1;
}
} // namespace

std::string prepareDataFolder() {
    const char *home = std::getenv("HOME");
    std::string folder = home && *home ? std::string(home) + "/data" : "/tmp/balatro-webos-smoke";
    if (!ensureFolder(folder)) {
        folder = "/tmp/balatro-webos-smoke";
        ensureFolder(folder);
    }
    if (chdir(folder.c_str()) != 0)
        std::fprintf(stderr, "[smoke] cannot enter %s: %s\n", folder.c_str(), std::strerror(errno));
    const std::string saveRoot = folder + "/save";
    ensureFolder(saveRoot);
    setenv("XDG_DATA_HOME", saveRoot.c_str(), 1);
    std::printf("[smoke] data folder %s\n", folder.c_str());
    return folder;
}

void installRemoteHandling() {
    SDL_SetHint("SDL_WEBOS_ACCESS_POLICY_KEYS_BACK", "true");
    SDL_SetHint("SDL_WEBOS_ACCESS_POLICY_KEYS_EXIT", "true");
    SDL_SetHint("SDL_WEBOS_CURSOR_SLEEP_TIME", "1000");
    SDL_SetEventFilter(remoteEventFilter, nullptr);
}

std::string executableFolder() {
    char path[PATH_MAX];
    const ssize_t length = readlink("/proc/self/exe", path, sizeof(path) - 1);
    if (length <= 0) return ".";
    path[length] = '\0';
    const std::string full(path);
    const std::size_t slash = full.rfind('/');
    return slash == std::string::npos ? "." : full.substr(0, slash);
}
} // namespace balatro_webos
