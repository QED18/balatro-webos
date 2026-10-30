#include "PortalServer.h"

#include <arpa/inet.h>
#include <ifaddrs.h>
#include <net/if.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <algorithm>
#include <array>
#include <cerrno>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <fstream>
#include <random>
#include <sstream>
#include <string>

namespace balatro_webos {
namespace {
constexpr std::size_t kHeaderLimit = 16 * 1024;
constexpr std::size_t kMaxArchiveBytes = 160U * 1024U * 1024U;
constexpr int kFirstPort = 47811;
constexpr int kLastPort = 47821;

bool ensureFolder(const std::string &path) { return mkdir(path.c_str(), 0755) == 0 || errno == EEXIST; }

bool isPrivateAddress(const in_addr &address) {
    const unsigned long value = ntohl(address.s_addr);
    return (value >> 24U) == 10U || (value >> 20U) == 0xAC1U || (value >> 16U) == 0xC0A8U;
}

std::string localAddress() {
    ifaddrs *interfaces = nullptr;
    if (getifaddrs(&interfaces) != 0) return "127.0.0.1";
    std::string result = "127.0.0.1";
    for (ifaddrs *it = interfaces; it; it = it->ifa_next) {
        if (!it->ifa_addr || it->ifa_addr->sa_family != AF_INET || (it->ifa_flags & IFF_LOOPBACK)) continue;
        const in_addr address = reinterpret_cast<sockaddr_in *>(it->ifa_addr)->sin_addr;
        if (!isPrivateAddress(address)) continue;
        char text[INET_ADDRSTRLEN] = {};
        if (inet_ntop(AF_INET, &address, text, sizeof(text))) { result = text; break; }
    }
    freeifaddrs(interfaces);
    return result;
}

std::string randomPin() {
    std::random_device device;
    const unsigned value = 1000U + (device() % 9000U);
    return std::to_string(value);
}

std::string lowercase(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return value;
}

bool writeAll(const int fd, const char *data, std::size_t size) {
    while (size > 0) {
        const ssize_t count = write(fd, data, size);
        if (count <= 0) return false;
        data += count;
        size -= static_cast<std::size_t>(count);
    }
    return true;
}

bool sendAll(const int fd, const std::string &response) {
    const char *data = response.data();
    std::size_t size = response.size();
    while (size > 0) {
        const ssize_t count = send(fd, data, size, MSG_NOSIGNAL);
        if (count <= 0) return false;
        data += count;
        size -= static_cast<std::size_t>(count);
    }
    return true;
}

void reply(const int client, const int status, const char *type, const std::string &body) {
    const char *reason = status == 200 ? "OK" : status == 401 ? "Unauthorized" : status == 413 ? "Payload Too Large" : "Bad Request";
    std::ostringstream out;
    out << "HTTP/1.1 " << status << ' ' << reason << "\r\nContent-Type: " << type
        << "\r\nContent-Length: " << body.size() << "\r\nCache-Control: no-store\r\nConnection: close\r\n\r\n" << body;
    sendAll(client, out.str());
}

unsigned read16(const unsigned char *bytes) { return static_cast<unsigned>(bytes[0]) | (static_cast<unsigned>(bytes[1]) << 8U); }
unsigned long read32(const unsigned char *bytes) {
    return static_cast<unsigned long>(bytes[0]) | (static_cast<unsigned long>(bytes[1]) << 8U) |
           (static_cast<unsigned long>(bytes[2]) << 16U) | (static_cast<unsigned long>(bytes[3]) << 24U);
}

bool safeArchivePath(const std::string &name) {
    if (name.empty() || name[0] == '/' || name.find('\\') != std::string::npos || name.find('\0') != std::string::npos) return false;
    std::istringstream parts(name);
    std::string part;
    while (std::getline(parts, part, '/')) if (part == "..") return false;
    return true;
}

bool validateLoveArchive(const std::string &path, std::streamoff *archiveStart, std::string *error) {
    std::ifstream input(path, std::ios::binary | std::ios::ate);
    if (!input) { *error = "could not read uploaded archive"; return false; }
    const std::streamoff length = input.tellg();
    if (length < 22 || static_cast<unsigned long long>(length) > kMaxArchiveBytes) { *error = "archive size is not allowed"; return false; }

    const std::size_t tailSize = static_cast<std::size_t>(std::min<std::streamoff>(length, 65557));
    std::string tail(tailSize, '\0');
    input.seekg(length - static_cast<std::streamoff>(tailSize));
    input.read(&tail[0], static_cast<std::streamsize>(tailSize));
    std::size_t eocd = std::string::npos;
    for (std::size_t index = tailSize - 22; ; --index) {
        if (static_cast<unsigned char>(tail[index]) == 0x50 && static_cast<unsigned char>(tail[index + 1]) == 0x4b &&
            static_cast<unsigned char>(tail[index + 2]) == 0x05 && static_cast<unsigned char>(tail[index + 3]) == 0x06) { eocd = index; break; }
        if (index == 0) break;
    }
    if (eocd == std::string::npos) { *error = "archive has no ZIP directory"; return false; }
    const unsigned char *end = reinterpret_cast<const unsigned char *>(tail.data() + eocd);
    const unsigned entries = read16(end + 10);
    const unsigned long directorySize = read32(end + 12);
    const unsigned long directoryOffset = read32(end + 16);
    const unsigned archiveCommentLength = read16(end + 20);
    const std::streamoff eocdOffset = length - static_cast<std::streamoff>(tailSize) + static_cast<std::streamoff>(eocd);
    const std::streamoff directoryLength = static_cast<std::streamoff>(directorySize);
    const std::streamoff directoryStart = static_cast<std::streamoff>(directoryOffset);
    if (entries == 0 || entries > 20000U || eocdOffset + 22 + static_cast<std::streamoff>(archiveCommentLength) > length ||
        eocdOffset < directoryLength + directoryStart) { *error = "archive directory is invalid"; return false; }
    const std::streamoff start = eocdOffset - directoryLength - directoryStart;
    if (start < 0 || start + directoryStart + directoryLength > length) { *error = "archive directory is invalid"; return false; }
    input.seekg(start);
    std::array<unsigned char, 4> signature{};
    input.read(reinterpret_cast<char *>(signature.data()), signature.size());
    if (!input || read32(signature.data()) != 0x04034b50UL) { *error = "selected file has no embedded LÖVE archive"; return false; }

    input.seekg(start + directoryStart);
    bool hasMain = false;
    for (unsigned index = 0; index < entries; ++index) {
        std::array<unsigned char, 46> header{};
        input.read(reinterpret_cast<char *>(header.data()), header.size());
        if (!input || read32(header.data()) != 0x02014b50UL) { *error = "archive entry is invalid"; return false; }
        const unsigned nameLength = read16(header.data() + 28);
        const unsigned extraLength = read16(header.data() + 30);
        const unsigned commentLength = read16(header.data() + 32);
        if (nameLength == 0 || nameLength > 4096U) { *error = "archive path is invalid"; return false; }
        std::string name(nameLength, '\0');
        input.read(&name[0], nameLength);
        if (!input || !safeArchivePath(name)) { *error = "archive contains an unsafe path"; return false; }
        if (name == "main.lua") hasMain = true;
        input.seekg(static_cast<std::streamoff>(extraLength + commentLength), std::ios::cur);
        if (!input) { *error = "archive directory is truncated"; return false; }
    }
    if (!hasMain) { *error = "archive does not contain main.lua at its root"; return false; }
    *archiveStart = start;
    return true;
}

bool copyArchiveTail(const std::string &source, const std::streamoff start, const std::string &destination) {
    std::ifstream input(source, std::ios::binary);
    std::ofstream output(destination, std::ios::binary | std::ios::trunc);
    if (!input || !output) return false;
    input.seekg(start);
    std::array<char, 64 * 1024> chunk{};
    while (input) {
        input.read(chunk.data(), static_cast<std::streamsize>(chunk.size()));
        const std::streamsize count = input.gcount();
        if (count > 0) output.write(chunk.data(), count);
    }
    output.close();
    return input.eof() && static_cast<bool>(output);
}

bool normalizeLoveArchive(const std::string &source, std::string *normalized, std::string *error) {
    std::streamoff start = 0;
    if (!validateLoveArchive(source, &start, error)) return false;
    if (start == 0) { *normalized = source; return true; }
    const std::string extracted = source + ".love";
    if (!copyArchiveTail(source, start, extracted)) { unlink(extracted.c_str()); *error = "could not extract the embedded LÖVE archive"; return false; }
    std::streamoff extractedStart = 0;
    if (!validateLoveArchive(extracted, &extractedStart, error) || extractedStart != 0) {
        unlink(extracted.c_str());
        if (error->empty()) *error = "embedded LÖVE archive is invalid";
        return false;
    }
    *normalized = extracted;
    return true;
}

std::string page() {
    std::ostringstream out;
    out << "<!doctype html><meta charset=utf-8><meta name=viewport content='width=device-width,initial-scale=1'>"
        << "<title>Balatro for webOS</title><style>body{margin:0;min-height:100vh;display:grid;place-items:center;background:#071d12;color:#edffe9;font:16px/1.28 Georgia,serif}main{width:calc(100vw - 56px);max-width:660px;box-sizing:border-box;padding:25px 28px;background:#123d2a;border:4px solid #e3cc67;box-shadow:12px 12px #020b07}h1{font-size:38px;line-height:1;margin:0;color:#7ce4df}h1 b{color:#f0d66a}.intro{margin:12px 0 16px;color:#c8e3d0}.step{margin-top:11px;padding:13px 16px;background:#0a2b1d;border-left:5px solid #7ce4df}.step h2{margin:0;font-size:20px;color:#fff9db}.step p{margin:5px 0 0;color:#b8d7c3;font-size:14px}input,button{font:inherit;margin-top:9px}#pin{width:190px;padding:7px 10px}button{padding:8px 14px;background:#ba3f4f;color:#fff9db;border:3px solid #651d2a;font-weight:bold}.platform{background:#194536;border-color:#0d3024}.platform.active{background:#e3cc67;border-color:#fff0a2;color:#102217}.guide{height:145px;box-sizing:border-box;margin:9px 0 0;padding-left:21px;color:#e7f7ef;font-size:15px}.guide li{margin:5px 0}.guide b{color:#fff9db}#drop{display:block;margin-top:10px;padding:15px;text-align:center;border:3px dashed #7ce4df;background:#08281a;color:#edffe9;cursor:pointer}#drop.drag{background:#1a5d44;border-color:#f0d66a;color:#fff9db}#drop input{display:none}#drop strong,#drop span{display:block}#drop span{margin-top:3px;color:#aed8bd;font-size:14px}.meter{display:none;height:15px;margin-top:13px;border:3px solid #e3cc67;background:#06160e}.meter.show{display:block}.meter i{display:block;width:0;height:100%;background:#57efe0;transition:width .15s}#status{min-height:22px;margin:9px 0 0}small{display:block;margin-top:16px;color:#aed8bd;font-size:14px}</style><main><h1>BALATRO <b>webOS</b></h1>"
        << "<p class=intro>Move your own installed game to this TV. It stays on your home network.</p><section class=step><h2>1. Enter the TV PIN</h2><p>Use the four-digit code shown in the Balatro app on your TV.</p><input id=pin inputmode=numeric maxlength=4 placeholder='TV PIN'></section><section class=step><h2>2. Choose your computer</h2><button id=mac class='platform active' type=button>MAC</button><button id=pc class=platform type=button>WINDOWS / LINUX</button><ol id=guide class=guide></ol><label id=drop for=file><strong>DROP Balatro.love HERE</strong><span>or click to choose it</span><input id=file type=file accept='.love,.exe,application/zip,application/vnd.microsoft.portable-executable'></label></section><button id=go>VERIFY & TRANSFER</button><div id=meter class=meter><i id=fill></i></div><p id=status>Waiting for a file.</p><small>Nothing is sent to cloud storage. After success, quit and reopen the TV app.</small></main>"
        << "<script>let chosen,field=document.getElementById('file'),zone=document.getElementById('drop'),button=document.getElementById('go'),mac=document.getElementById('mac'),pc=document.getElementById('pc'),guide=document.getElementById('guide'),pinBox=document.getElementById('pin'),meter=document.getElementById('meter'),fillBox=document.getElementById('fill'),statusBox=document.getElementById('status'),show=(label,p)=>{if(p>0)meter.classList.add('show');fillBox.style.width=p+'%';statusBox.textContent=label+(p?' '+p+'%':'')},valid=f=>f&&/^(Balatro\\.love|Balatro\\.exe)$/i.test(f.name),setPlatform=platform=>{let isMac=platform==='mac';mac.classList.toggle('active',isMac);pc.classList.toggle('active',!isMac);guide.innerHTML=isMac?'<li>Open <b>Steam</b>, right-click <b>Balatro</b>, then choose <b>Manage → Browse local files</b>.</li><li>Right-click <b>Balatro.app</b> → <b>Show Package Contents</b> → open <b>Contents → Resources</b>.</li><li>Drag <b>Balatro.love</b> into the box below.</li>':'<li>Open <b>Steam</b>, right-click <b>Balatro</b>, then choose <b>Manage → Browse local files</b>.</li><li>Drag <b>Balatro.exe</b> into the box below.</li>';zone.querySelector('strong').textContent='DROP '+(isMac?'Balatro.love':'Balatro.exe')+' HERE';zone.querySelector('span').textContent='or click to choose it';chosen=null;show('Waiting for a file.',0)};mac.addEventListener('click',()=>setPlatform('mac'));pc.addEventListener('click',()=>setPlatform('pc'));setPlatform('mac');let pick=f=>{if(!valid(f))return show('Choose Balatro.love on Mac or Balatro.exe on Windows/Linux.',0);chosen=f;zone.classList.remove('drag');show('Ready: '+f.name,0)};field.addEventListener('change',()=>pick(field.files[0]));window.addEventListener('dragover',e=>e.preventDefault());window.addEventListener('drop',e=>e.preventDefault());zone.addEventListener('dragenter',e=>{e.preventDefault();zone.classList.add('drag')});zone.addEventListener('dragover',e=>{e.preventDefault();e.dataTransfer.dropEffect='copy';zone.classList.add('drag')});zone.addEventListener('dragleave',e=>{if(!zone.contains(e.relatedTarget))zone.classList.remove('drag')});zone.addEventListener('drop',e=>{e.preventDefault();zone.classList.remove('drag');pick(e.dataTransfer.files[0])});button.addEventListener('click',()=>{let f=chosen||field.files[0],p=pinBox.value;if(!f)return show('Choose or drop Balatro.love / Balatro.exe first.',0);button.disabled=true;show('Connecting to TV…',0);let x=new XMLHttpRequest;x.open('POST','/api/import?pin='+encodeURIComponent(p));x.upload.onprogress=e=>{if(e.lengthComputable)show('Transferring privately to TV…',Math.floor(e.loaded/e.total*100))};x.onload=()=>{let body={};try{body=JSON.parse(x.responseText)}catch(_){}show(body.message||'Transfer failed.',x.status===200?100:0);button.disabled=false};x.onerror=()=>{show('Could not reach the TV portal.',0);button.disabled=false};x.send(f)})</script>";
    return out.str();
}
} // namespace

PortalServer::PortalServer(std::string dataFolder) : dataFolder_(std::move(dataFolder)) {}
PortalServer::~PortalServer() { stop(); }

bool PortalServer::start() {
    ensureFolder(dataFolder_ + "/game");
    ensureFolder(dataFolder_ + "/staging");
    ensureFolder(dataFolder_ + "/save/love");
    ensureFolder(dataFolder_ + "/save/love/balatro-webos-smoke");
    listener_ = socket(AF_INET, SOCK_STREAM, 0);
    if (listener_ < 0) return false;
    const int enabled = 1;
    setsockopt(listener_, SOL_SOCKET, SO_REUSEADDR, &enabled, sizeof(enabled));
    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    int port = kFirstPort;
    for (; port <= kLastPort; ++port) { address.sin_port = static_cast<in_port_t>(htons(static_cast<unsigned short>(port))); if (bind(listener_, reinterpret_cast<sockaddr *>(&address), sizeof(address)) == 0) break; }
    if (port > kLastPort || listen(listener_, 2) != 0) { close(listener_); listener_ = -1; return false; }
    details_.pin = randomPin();
    details_.url = "http://" + localAddress() + ":" + std::to_string(port);
    running_ = true;
    worker_ = std::thread(&PortalServer::serve, this);
    writeStatus("Ready for private transfer", 0);
    std::printf("[portal] %s pin %s\n", details_.url.c_str(), details_.pin.c_str());
    return true;
}

void PortalServer::writeStatus(const std::string &label, const unsigned progress) {
    const std::string temporary = dataFolder_ + "/save/love/balatro-webos-smoke/.portal-status.tmp";
    const std::string destination = dataFolder_ + "/save/love/balatro-webos-smoke/portal-status.txt";
    std::ofstream output(temporary, std::ios::trunc);
    if (!output) return;
    output << label << '|' << std::min(progress, 100U);
    output.close();
    rename(temporary.c_str(), destination.c_str());
}

void PortalServer::stop() {
    if (!running_.exchange(false)) return;
    shutdown(listener_, SHUT_RDWR);
    close(listener_);
    listener_ = -1;
    if (worker_.joinable()) worker_.join();
}

void PortalServer::serve() {
    while (running_) {
        const int client = accept(listener_, nullptr, nullptr);
        if (client < 0) continue;
        handle(client);
        close(client);
    }
}

void PortalServer::handle(const int client) {
    std::string request;
    std::array<char, 4096> buffer{};
    std::size_t headerEnd = std::string::npos;
    while (request.size() < kHeaderLimit && (headerEnd = request.find("\r\n\r\n")) == std::string::npos) {
        const ssize_t count = recv(client, buffer.data(), buffer.size(), 0);
        if (count <= 0) return;
        request.append(buffer.data(), static_cast<std::size_t>(count));
    }
    if (headerEnd == std::string::npos) { reply(client, 400, "application/json", "{\"message\":\"Request headers are invalid.\"}"); return; }
    const std::string headers = request.substr(0, headerEnd);
    const std::size_t firstEnd = headers.find("\r\n");
    const std::string first = headers.substr(0, firstEnd);
    if (first == "GET / HTTP/1.1" || first == "GET / HTTP/1.0") { reply(client, 200, "text/html; charset=utf-8", page()); return; }
    if (first.rfind("POST /api/import?pin=", 0) != 0) { reply(client, 400, "application/json", "{\"message\":\"Unknown request.\"}"); return; }
    const std::size_t pinEnd = first.find(' ', 21);
    if (first.substr(21, pinEnd - 21) != details_.pin) { writeStatus("Incorrect PIN", 0); reply(client, 401, "application/json", "{\"message\":\"Incorrect TV PIN.\"}"); return; }
    const std::size_t lengthAt = lowercase(headers).find("\r\ncontent-length:");
    if (lengthAt == std::string::npos) { reply(client, 400, "application/json", "{\"message\":\"A file length is required.\"}"); return; }
    const std::size_t valueAt = headers.find(':', lengthAt) + 1;
    const std::size_t valueEnd = headers.find("\r\n", valueAt);
    std::size_t length = 0;
    try { length = static_cast<std::size_t>(std::stoull(headers.substr(valueAt, valueEnd - valueAt))); } catch (...) { reply(client, 400, "application/json", "{\"message\":\"File length is invalid.\"}"); return; }
    if (length < 22 || length > kMaxArchiveBytes) { reply(client, 413, "application/json", "{\"message\":\"Game file must be smaller than 160 MB.\"}"); return; }
    std::string temporary = dataFolder_ + "/staging/balatro-upload-XXXXXX";
    const int file = mkstemp(&temporary[0]);
    if (file < 0) { reply(client, 400, "application/json", "{\"message\":\"TV staging storage is unavailable.\"}"); return; }
    writeStatus("Receiving from your computer", 0);
    std::size_t received = request.size() - (headerEnd + 4);
    bool complete = received <= length && writeAll(file, request.data() + headerEnd + 4, received);
    unsigned lastProgress = 0;
    while (complete && received < length) {
        const ssize_t count = recv(client, buffer.data(), std::min(buffer.size(), length - received), 0);
        if (count <= 0) { complete = false; break; }
        complete = writeAll(file, buffer.data(), static_cast<std::size_t>(count));
        received += static_cast<std::size_t>(count);
        const unsigned progress = static_cast<unsigned>((received * 100U) / length);
        if (progress >= lastProgress + 5U || progress == 100U) { writeStatus("Receiving from your computer", progress); lastProgress = progress; }
    }
    close(file);
    std::string error;
    writeStatus("Verifying game file", 100);
    std::string normalized;
    if (!complete || !normalizeLoveArchive(temporary, &normalized, &error)) { unlink(temporary.c_str()); writeStatus("Transfer rejected", 0); reply(client, 400, "application/json", "{\"message\":\"" + (error.empty() ? "Transfer was incomplete." : error) + "\"}"); return; }
    const std::string destination = dataFolder_ + "/game/Balatro.love";
    if (normalized != temporary) unlink(temporary.c_str());
    if (rename(normalized.c_str(), destination.c_str()) != 0) { unlink(normalized.c_str()); writeStatus("TV storage error", 0); reply(client, 400, "application/json", "{\"message\":\"TV could not activate the archive.\"}"); return; }
    writeStatus("Transfer complete — reopen app", 100);
    reply(client, 200, "application/json", "{\"message\":\"Verified and stored locally. Quit and reopen the TV app to play.\"}");
}

} // namespace balatro_webos
