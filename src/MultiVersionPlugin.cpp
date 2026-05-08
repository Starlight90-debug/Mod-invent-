#include "MultiVersionPlugin.h"
#include <ll/api/mod/RegisterHelper.h>

namespace multiversion {

static constexpr uint32_t SERVER_PROTOCOL_ID = 944;
static constexpr const char* SERVER_MC_VERSION = "1.26.10";

static std::unique_ptr<MultiVersionPlugin> sInstance;

MultiVersionPlugin& MultiVersionPlugin::getInstance() {
    return *sInstance;
}

bool MultiVersionPlugin::load() {
    auto& logger = getSelf().getLogger();

    logger.info("Loading MultiVersionPlugin");

    mVersionRegistry = std::make_unique<VersionRegistry>();
    mVersionRegistry->setServerProtocolId(SERVER_PROTOCOL_ID);

    registerSupportedVersions();
    return true;
}

bool MultiVersionPlugin::enable() {
    auto& logger = getSelf().getLogger();

    mPacketHook = std::make_unique<PacketHook>(*mVersionRegistry, logger);
    return mPacketHook->install();
}

bool MultiVersionPlugin::disable() {
    mPacketHook.reset();
    mVersionRegistry.reset();
    return true;
}

void MultiVersionPlugin::registerSupportedVersions() {
    using V = ProtocolVersion;

    mVersionRegistry->registerVersion(V{766, "1.21.50"});
    mVersionRegistry->registerVersion(V{944, "1.26.10"});
}

} // namespace multiversion

LL_REGISTER_MOD(multiversion::MultiVersionPlugin);
