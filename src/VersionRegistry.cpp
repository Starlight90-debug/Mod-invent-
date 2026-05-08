#include "VersionRegistry.h"

#include <algorithm>

namespace multiversion {

void VersionRegistry::registerVersion(const ProtocolVersion& v) {
    mVersions.emplace(v.protocolId, v);
}

std::optional<ProtocolVersion> VersionRegistry::findByProtocolId(uint32_t id) const {
    auto it = mVersions.find(id);
    if (it == mVersions.end()) return std::nullopt;
    return it->second;
}

bool VersionRegistry::isSupported(uint32_t id) const {
    return mVersions.find(id) != mVersions.end();
}

size_t VersionRegistry::getSupportedCount() const {
    return mVersions.size();
}

std::vector<uint32_t> VersionRegistry::getAllProtocolIds() const {
    std::vector<uint32_t> ids;
    ids.reserve(mVersions.size());

    for (auto& [id, _] : mVersions)
        ids.push_back(id);

    std::sort(ids.begin(), ids.end());
    return ids;
}

void VersionRegistry::setServerProtocolId(uint32_t id) {
    mServerProtocolId = id;
}

uint32_t VersionRegistry::getServerProtocolId() const {
    return mServerProtocolId;
}

} // namespace multiversion
