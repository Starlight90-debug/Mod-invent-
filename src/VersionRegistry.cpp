#include "VersionRegistry.h"

#include <algorithm>

namespace multiversion {

void VersionRegistry::registerVersion(const ProtocolVersion& version) {
    mVersions.emplace(version.protocolId, version);
}

std::optional<ProtocolVersion> VersionRegistry::findByProtocolId(std::uint32_t protocolId) const {
    auto it = mVersions.find(protocolId);
    if (it == mVersions.end()) {
        return std::nullopt;
    }
    return it->second;
}

bool VersionRegistry::isSupported(std::uint32_t protocolId) const {
    return mVersions.contains(protocolId);
}

std::size_t VersionRegistry::getSupportedCount() const {
    return mVersions.size();
}

std::vector<std::uint32_t> VersionRegistry::getAllProtocolIds() const {
    std::vector<std::uint32_t> ids;
    ids.reserve(mVersions.size());
    for (const auto& [id, _] : mVersions) {
        ids.push_back(id);
    }
    std::ranges::sort(ids);
    return ids;
}

void VersionRegistry::setServerProtocolId(std::uint32_t id) {
    mServerProtocolId = id;
}

std::uint32_t VersionRegistry::getServerProtocolId() const {
    return mServerProtocolId;
}

} // namespace multiversion
