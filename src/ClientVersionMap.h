#pragma once

#include <cstdint>
#include <mutex>
#include <string>
#include <unordered_map>
#include <shared_mutex>

namespace multiversion {

// Хранит protocol ID для каждого подключённого клиента.
// Потокобезопасен.
class ClientVersionMap {
public:
    static ClientVersionMap& getInstance() {
        static ClientVersionMap sInstance;
        return sInstance;
    }

    void set(const std::string& address, std::uint32_t protocolId) {
        std::unique_lock lock(mMutex);
        mMap[address] = protocolId;
    }

    [[nodiscard]] std::uint32_t get(const std::string& address) const {
        std::shared_lock lock(mMutex);
        auto it = mMap.find(address);
        return (it != mMap.end()) ? it->second : 0u;
    }

    void remove(const std::string& address) {
        std::unique_lock lock(mMutex);
        mMap.erase(address);
    }

    [[nodiscard]] bool has(const std::string& address) const {
        std::shared_lock lock(mMutex);
        return mMap.find(address) != mMap.end();
    }

private:
    ClientVersionMap() = default;

    mutable std::shared_mutex mMutex;
    std::unordered_map<std::string, std::uint32_t> mMap;
};

} // namespace multiversion
