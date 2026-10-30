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
#include <fstream>
#include <string>
#include <sys/stat.h>
#include <unistd.h>

namespace balatro_webos {
namespace {
constexpr int kWebOSBack = 482;
constexpr int kWebOSRed = 486;
constexpr int kWebOSGreen = 487;
constexpr int kWebOSExit = 505;
SDL_Joystick *remoteGamepad = nullptr;

bool ensureFolder(const std::string &path) { return mkdir(path.c_str(), 0755) == 0 || errno == EEXIST; }

bool remoteButton(const int scancode, SDL_GameControllerButton *button) {
    switch (scancode) {
    case SDL_SCANCODE_UP: *button = SDL_CONTROLLER_BUTTON_DPAD_UP; return true;
    case SDL_SCANCODE_DOWN: *button = SDL_CONTROLLER_BUTTON_DPAD_DOWN; return true;
    case SDL_SCANCODE_LEFT: *button = SDL_CONTROLLER_BUTTON_DPAD_LEFT; return true;
    case SDL_SCANCODE_RIGHT: *button = SDL_CONTROLLER_BUTTON_DPAD_RIGHT; return true;
    case SDL_SCANCODE_RETURN: *button = SDL_CONTROLLER_BUTTON_A; return true;
    case kWebOSBack:
    case kWebOSRed: *button = SDL_CONTROLLER_BUTTON_B; return true;
    case kWebOSGreen: *button = SDL_CONTROLLER_BUTTON_A; return true;
    default: return false;
    }
}

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
    SDL_GameControllerButton button;
    if (remoteButton(scancode, &button) && enableRemoteGamepad()) {
        SDL_JoystickSetVirtualButton(remoteGamepad, static_cast<int>(button),
                                    event->type == SDL_KEYDOWN ? 1 : 0);
        // Balatro treats keyboard events as pointer input in release mode. Do
        // not pass the original key through once the controller event exists.
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

void installCrtCompatibilityOverlay(const std::string &dataFolder) {
    // LÖVE mounts this save directory ahead of the owner's .love archive after
    // startup. The overlay is project-authored and leaves every game file
    // untouched; it only replaces the TV-incompatible CRT warp pass.
    const std::string save = dataFolder + "/save/love/Balatro";
    const std::string resources = save + "/resources";
    const std::string shaders = resources + "/shaders";
    ensureFolder(dataFolder + "/save");
    ensureFolder(dataFolder + "/save/love");
    ensureFolder(save);
    ensureFolder(resources);
    ensureFolder(shaders);
    std::ofstream output(shaders + "/CRT.fs", std::ios::trunc);
    if (!output) {
        std::fprintf(stderr, "[host] cannot install CRT compatibility overlay\n");
        return;
    }
    output << R"SHADER(extern number time;
extern vec2 distortion_fac;
extern vec2 scale_fac;
extern number feather_fac;
extern number noise_fac;
extern number bloom_fac;
extern number crt_intensity;
extern number glitch_intensity;
extern number scanlines;
extern vec2 mouse_screen_pos;
extern number hovering;
extern number screen_scale;

vec4 effect(vec4 colour, Image texture, vec2 texture_coords, vec2 screen_coords) {
    // Keep all game-sent uniforms active, without applying the unsupported
    // barrel-distortion pass which produces the black U-shaped artifact.
    number keep = time + distortion_fac.x + distortion_fac.y + scale_fac.x + scale_fac.y + feather_fac + noise_fac + bloom_fac + crt_intensity + glitch_intensity + scanlines + mouse_screen_pos.x + mouse_screen_pos.y + hovering + screen_scale;
    return Texel(texture, texture_coords) * colour + vec4(keep * 0.0000001);
}
)SHADER";
    std::printf("[host] CRT compatibility overlay active\n");
}

void installRemoteHandling() {
    SDL_SetHint("SDL_WEBOS_ACCESS_POLICY_KEYS_BACK", "true");
    SDL_SetHint("SDL_WEBOS_ACCESS_POLICY_KEYS_EXIT", "true");
    // Keep the platform default cursor timeout. A one-second override made the
    // Magic Remote pointer sleep almost immediately and added a wake-up delay.
    remoteGamepad = nullptr;
    SDL_SetEventFilter(remoteEventFilter, nullptr);
}

bool enableRemoteGamepad() {
    if (remoteGamepad) return true;
    const int device = SDL_JoystickAttachVirtual(SDL_JOYSTICK_TYPE_GAMECONTROLLER,
                                                  SDL_CONTROLLER_AXIS_MAX,
                                                  SDL_CONTROLLER_BUTTON_MAX, 0);
    if (device < 0) {
        std::fprintf(stderr, "[smoke] virtual TV-remote controller unavailable: %s\n", SDL_GetError());
        return false;
    }
    remoteGamepad = SDL_JoystickOpen(device);
    if (!remoteGamepad) {
        std::fprintf(stderr, "[smoke] cannot open virtual TV-remote controller: %s\n", SDL_GetError());
        return false;
    }
    std::printf("[smoke] TV remote virtual controller ready\n");
    return true;
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
