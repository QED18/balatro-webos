#pragma once

#include <string>

namespace balatro_webos {
std::string prepareDataFolder();
void installCrtCompatibilityOverlay(const std::string &dataFolder);
void installRemoteHandling();
bool enableRemoteGamepad();
std::string executableFolder();
} // namespace balatro_webos
