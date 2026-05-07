// MultiVersionPlugin.cpp
// LeviLamina plugin — multiversion support layer on top of SculkCatalystMC/Protocol
// Нативный сервер: BDS 1.26.10, protocol ID = 944
// Клиенты: 1.21.50 (766) — 1.26.10 (944)

#include "MultiVersionPlugin.h"

#include <ll/api/mod/RegisterHelper.h>

namespace multiversion {

static constexpr std::uint32_t SERVER_PROTOCOL_ID = 944;
static constexpr const char*   SERVER_MC_VERSION   = "1.26.10";

static std::unique_ptr<MultiVersionPlugin> sInstance;

MultiVersionPlugin& MultiVersionPlugin::getInstance() {
    return *sInstance;
}

// ---------------------------------------------------------------------------
// load() — вызывается при загрузке плагина (до enable)
// ---------------------------------------------------------------------------
bool MultiVersionPlugin::load() {
    auto& logger = getSelf().getLogger();

    logger.info("================================================");
    logger.info(" MultiVersionPlugin - Loading");
    logger.info(" Server: BDS {} | Protocol: {}",
                SERVER_MC_VERSION, SERVER_PROTOCOL_ID);
    logger.info("================================================");

    // Инициализируем реестр версий
    mVersionRegistry = std::make_unique<VersionRegistry>();

    if (!mVersionRegistry) {
        logger.error("[FAIL] Failed to allocate VersionRegistry! Out of memory?");
        return false;
    }

    mVersionRegistry->setServerProtocolId(SERVER_PROTOCOL_ID);
    logger.info("[OK]  VersionRegistry created. Server protocol set to {}.", SERVER_PROTOCOL_ID);

    registerSupportedVersions();

    const std::size_t count = mVersionRegistry->getSupportedCount();
    if (count == 0) {
        logger.error("[FAIL] No supported versions registered! Plugin will not work.");
        return false;
    }

    logger.info("[OK]  Registered {} supported client protocol(s):", count);
    for (const auto id : mVersionRegistry->getAllProtocolIds()) {
        const auto v = mVersionRegistry->findByProtocolId(id);
        if (v) {
            const char* tag = (id == SERVER_PROTOCOL_ID) ? "  <-- native server" : "";
            logger.info("      protocol {:>4}  ->  mc {}{}",
                        id, v->mcVersion, tag);
        }
    }

    logger.info("[OK]  MultiVersionPlugin loaded successfully.");
    return true;
}

// ---------------------------------------------------------------------------
// enable() — вызывается когда сервер готов принимать плагины
// ---------------------------------------------------------------------------
bool MultiVersionPlugin::enable() {
    auto& logger = getSelf().getLogger();

    logger.info("[....] MultiVersionPlugin enabling...");

    if (!mVersionRegistry) {
        logger.error("[FAIL] VersionRegistry is null during enable()! "
                     "Did load() succeed? Check logs above.");
        return false;
    }

    mPacketHook = std::make_unique<PacketHook>(*mVersionRegistry, logger);

    if (!mPacketHook) {
        logger.error("[FAIL] Failed to allocate PacketHook! Out of memory?");
        return false;
    }

    logger.info("[....] Installing packet hooks on ServerNetworkHandler...");

    if (!mPacketHook->install()) {
        logger.error("[FAIL] MultiVersionPlugin FAILED to enable!");
        logger.error("       Packet hooks could not be installed.");
        logger.error("       Players with old clients will NOT be able to connect.");
        logger.error("       Check above for specific hook error messages.");
        mPacketHook.reset();
        return false;
    }

    logger.info("================================================");
    logger.info("[OK]  MultiVersionPlugin is ENABLED and RUNNING");
    logger.info("      Server protocol  : {} ({})", SERVER_PROTOCOL_ID, SERVER_MC_VERSION);
    logger.info("      Accepting clients: protocol 766 - 944");
    logger.info("      (1.21.50 / 1.21.51 / 1.21.60 / 1.21.70 /");
    logger.info("       1.21.80 / 1.21.90 / 1.21.100 / 1.21.110 /");
    logger.info("       1.21.120 / 1.21.130 / 1.26.0 / 1.26.10)");
    logger.info("================================================");

    return true;
}

// ---------------------------------------------------------------------------
// disable() — вызывается при выключении/перезагрузке сервера
// ---------------------------------------------------------------------------
bool MultiVersionPlugin::disable() {
    auto& logger = getSelf().getLogger();

    logger.info("[....] MultiVersionPlugin disabling...");

    if (mPacketHook) {
        mPacketHook->uninstall();
        mPacketHook.reset();
        logger.info("[OK]  PacketHook destroyed.");
    } else {
        logger.warn("[WARN] disable() called but PacketHook was already null.");
    }

    mVersionRegistry.reset();
    logger.info("[OK]  VersionRegistry destroyed.");
    logger.info("[OK]  MultiVersionPlugin disabled cleanly.");

    return true;
}

// ---------------------------------------------------------------------------
// Таблица protocol ID — обновляй при выходе новых версий Bedrock
// ---------------------------------------------------------------------------
void MultiVersionPlugin::registerSupportedVersions() {
    using V = ProtocolVersion;

    // 1.21.5x — protocol 766
    mVersionRegistry->registerVersion(V{766, "1.21.50"});
    mVersionRegistry->registerVersion(V{766, "1.21.51"});

    // 1.21.6x — protocol 776
    mVersionRegistry->registerVersion(V{776, "1.21.60"});
    mVersionRegistry->registerVersion(V{776, "1.21.61"});

    // 1.21.7x — protocol 786
    mVersionRegistry->registerVersion(V{786, "1.21.70"});
    mVersionRegistry->registerVersion(V{786, "1.21.71"});

    // 1.21.8x — protocol 800
    mVersionRegistry->registerVersion(V{800, "1.21.80"});

    // 1.21.9x — protocol 818 / 819
    mVersionRegistry->registerVersion(V{818, "1.21.90"});
    mVersionRegistry->registerVersion(V{818, "1.21.91"});
    mVersionRegistry->registerVersion(V{819, "1.21.93"});

    // 1.21.10x — protocol 827
    mVersionRegistry->registerVersion(V{827, "1.21.100"});
    mVersionRegistry->registerVersion(V{827, "1.21.111"});

    // 1.21.11x — protocol 844
    mVersionRegistry->registerVersion(V{844, "1.21.110"});

    // 1.21.12x — protocol 859 / 860
    mVersionRegistry->registerVersion(V{859, "1.21.120"});
    mVersionRegistry->registerVersion(V{860, "1.21.121"});
    mVersionRegistry->registerVersion(V{860, "1.21.124"});

    // 1.21.13x — protocol 898
    mVersionRegistry->registerVersion(V{898, "1.21.130"});

    // 1.26.0 — protocol 924
    mVersionRegistry->registerVersion(V{924, "1.26.0"});

    // 1.26.10 — НАТИВНЫЙ СЕРВЕР, protocol 944 — трансляция не нужна
    mVersionRegistry->registerVersion(V{SERVER_PROTOCOL_ID, SERVER_MC_VERSION});
}

} // namespace multiversion

LL_REGISTER_MOD(multiversion::MultiVersionPlugin, multiversion::sInstance);
