#pragma once

#include <atomic>
#include <string>
#include <thread>

namespace balatro_webos {

struct PortalDetails {
    std::string url;
    std::string pin;
};

// A deliberately small, private-LAN-only pairing service. It accepts one
// owner-selected Balatro .love archive or Windows/Proton executable at a time
// and never contacts the Internet.
class PortalServer {
public:
    explicit PortalServer(std::string dataFolder);
    ~PortalServer();
    PortalServer(const PortalServer &) = delete;
    PortalServer &operator=(const PortalServer &) = delete;

    bool start();
    void stop();
    const PortalDetails &details() const { return details_; }

private:
    void serve();
    void handle(int client);
    void writeStatus(const std::string &label, unsigned progress);

    std::string dataFolder_;
    PortalDetails details_;
    std::atomic<bool> running_{false};
    int listener_ = -1;
    std::thread worker_;
};

} // namespace balatro_webos
