#include "PacketHook.h"
#include "ClientVersionMap.h"

#include <ll/api/Logger.h>
#include <ll/api/memory/Hook.h>

#include <mc/network/NetworkConnection.h>
#include <mc/network/NetworkIdentifier.h>
#include <mc/network/ServerNetworkHandler.h>
#include <mc/network/packet/LoginPacket.h>

#include <cstring>
#include <format>
#include <vector>

namespace multiversion {

// ---------------------------------------------------------------------------
// Глобальный указатель на активный PacketHook
// ---------------------------------------------------------------------------
static PacketHook* gActiveHook{nullptr};

// Удобный логгер для использования внутри статических хуков
static ll::Logger sHookLogger("MultiVersion");

// ---------------------------------------------------------------------------
// Хук: LoginPacket — читаем protocol ID клиента, сохраняем, логируем
// ---------------------------------------------------------------------------
LL_TYPE_INSTANCE_HOOK(
    LoginPacketHook,
    ll::memory::HookPriority::Normal,
    ServerNetworkHandler,
    "?handle@ServerNetworkHandler@@UEAAXAEBVNetworkIdentifier@@AEBVLoginPacket@@@Z",
    void,
    const NetworkIdentifier& netId,
    const LoginPacket&       packet
) {
    if (gActiveHook) {
        const std::uint32_t clientProtocol =
            static_cast<std::uint32_t>(packet.getProtocolVersion());

        const std::string address = netId.getRealAddress().ToString();

        ClientVersionMap::getInstance().set(address, clientProtocol);

        const auto& registry = gActiveHook->getRegistry();

        if (clientProtocol == 944) {
            // Нативный клиент — просто логируем успех
            sHookLogger.info(
                "[OK] Native client connected: {} | protocol 944 (1.26.10) — no translation needed",
                address
            );
        } else if (registry.isSupported(clientProtocol)) {
            const auto ver = registry.findByProtocolId(clientProtocol).value();
            sHookLogger.info(
                "[OK] Client connected: {} | protocol {} ({}) -> translating to protocol 944 (1.26.10)",
                address, clientProtocol, ver.mcVersion
            );
        } else {
            // Неизвестная версия — предупреждение, но не кикаем
            sHookLogger.warn(
                "[WARN] Unknown client protocol: {} from {} | "
                "Not in supported list, allowing passthrough. "
                "Check if Protocol library supports this version.",
                clientProtocol, address
            );
        }
    }

    origin(netId, packet);
}

// ---------------------------------------------------------------------------
// Хук: onPlayerLeft — клиент отключился, чистим ClientVersionMap
// ---------------------------------------------------------------------------
LL_TYPE_INSTANCE_HOOK(
    DisconnectHook,
    ll::memory::HookPriority::Normal,
    ServerNetworkHandler,
    "?onPlayerLeft@ServerNetworkHandler@@UEAAXPEAVServerPlayer@@_N@Z",
    void,
    ServerPlayer* player,
    bool          disconnected
) {
    if (player) {
        const std::string address =
            player->getNetworkIdentifier().getRealAddress().ToString();

        const std::uint32_t proto =
            ClientVersionMap::getInstance().get(address);

        ClientVersionMap::getInstance().remove(address);

        if (gActiveHook) {
            if (proto != 0) {
                sHookLogger.info(
                    "[OK] Client disconnected: {} | was on protocol {}",
                    address, proto
                );
            } else {
                sHookLogger.info(
                    "[OK] Client disconnected: {} | (protocol unknown)",
                    address
                );
            }
        }
    }
    origin(player, disconnected);
}

// ---------------------------------------------------------------------------
// Хук: sendToClient — точка для даунгрейда исходящих пакетов
// ---------------------------------------------------------------------------
LL_TYPE_INSTANCE_HOOK(
    SendPacketHook,
    ll::memory::HookPriority::Normal,
    ServerNetworkHandler,
    "?sendToClient@ServerNetworkHandler@@QEAAXAEBVNetworkIdentifier@@AEBVPacket@@W4SubClientId@@@Z",
    void,
    const NetworkIdentifier& netId,
    const Packet&            pkt,
    SubClientId              subClientId
) {
    // Трансляция исходящих пакетов — passthrough, логика будет расширена
    origin(netId, pkt, subClientId);
}

// ---------------------------------------------------------------------------
// PacketHook::install
// ---------------------------------------------------------------------------
bool PacketHook::install() {
    if (mInstalled) {
        mLogger.warn("[WARN] PacketHook::install() called but already installed, skipping.");
        return true;
    }

    mLogger.info("[....] Installing packet hooks...");

    bool loginOk      = LoginPacketHook::hook();
    bool disconnectOk = DisconnectHook::hook();
    bool sendOk       = SendPacketHook::hook();

    if (!loginOk) {
        mLogger.error(
            "[FAIL] LoginPacketHook failed to install! "
            "Symbol: ?handle@ServerNetworkHandler@@UEAAXAEBVNetworkIdentifier@@AEBVLoginPacket@@@Z | "
            "Possible cause: wrong BDS version or symbol mismatch."
        );
    } else {
        mLogger.info("[OK]   LoginPacketHook installed.");
    }

    if (!disconnectOk) {
        mLogger.error(
            "[FAIL] DisconnectHook failed to install! "
            "Symbol: ?onPlayerLeft@ServerNetworkHandler@@UEAAXPEAVServerPlayer@@_N@Z | "
            "Possible cause: wrong BDS version or symbol mismatch."
        );
    } else {
        mLogger.info("[OK]   DisconnectHook installed.");
    }

    if (!sendOk) {
        mLogger.error(
            "[FAIL] SendPacketHook failed to install! "
            "Symbol: ?sendToClient@ServerNetworkHandler@@... | "
            "Possible cause: wrong BDS version or symbol mismatch."
        );
    } else {
        mLogger.info("[OK]   SendPacketHook installed.");
    }

    if (!loginOk || !disconnectOk || !sendOk) {
        mLogger.error(
            "[FAIL] One or more hooks failed. Rolling back all hooks. "
            "MultiVersionPlugin will NOT intercept packets. "
            "Check your LeviLamina + BDS version compatibility."
        );
        LoginPacketHook::unhook();
        DisconnectHook::unhook();
        SendPacketHook::unhook();
        return false;
    }

    gActiveHook = this;
    mInstalled  = true;

    mLogger.info("================================================");
    mLogger.info("[OK] MultiVersionPlugin is ACTIVE");
    mLogger.info("     Server protocol : 944 (1.26.10) [native]");
    mLogger.info("     Hooks installed : Login, Disconnect, SendToClient");
    mLogger.info("     Clients allowed : protocol 766 - 944");
    mLogger.info("================================================");

    return true;
}

// ---------------------------------------------------------------------------
// PacketHook::uninstall
// ---------------------------------------------------------------------------
void PacketHook::uninstall() {
    if (!mInstalled) {
        mLogger.warn("[WARN] PacketHook::uninstall() called but was not installed.");
        return;
    }

    mLogger.info("[....] Uninstalling packet hooks...");

    LoginPacketHook::unhook();
    mLogger.info("[OK]   LoginPacketHook removed.");

    DisconnectHook::unhook();
    mLogger.info("[OK]   DisconnectHook removed.");

    SendPacketHook::unhook();
    mLogger.info("[OK]   SendPacketHook removed.");

    gActiveHook = nullptr;
    mInstalled  = false;

    mLogger.info("[OK] MultiVersionPlugin hooks uninstalled. Plugin disabled.");
}

// ---------------------------------------------------------------------------
// PacketTranslator
// ---------------------------------------------------------------------------
static constexpr std::uint32_t NATIVE_PROTOCOL_ID = 944;

bool PacketTranslator::needsTranslation(std::uint32_t clientProtocol) const {
    if (clientProtocol == NATIVE_PROTOCOL_ID) return false;
    const std::uint32_t serverProtocol = mRegistry.getServerProtocolId();
    return serverProtocol != 0 && clientProtocol != serverProtocol;
}

sculk::protocol::MinecraftPackets::PacketHeader
PacketTranslator::readHeader(sculk::protocol::ReadOnlyBinaryStream& stream) const {
    return sculk::protocol::MinecraftPackets::readPacketHeader(stream);
}

std::vector<std::byte> PacketTranslator::translateIncoming(
    std::uint32_t              clientProtocol,
    std::span<const std::byte> rawPacket
) const {
    if (!needsTranslation(clientProtocol)) {
        return {rawPacket.begin(), rawPacket.end()};
    }

    std::vector<std::byte> data{rawPacket.begin(), rawPacket.end()};
    sculk::protocol::ReadOnlyBinaryStream readStream{data};

    const auto header = readHeader(readStream);
    auto       pkt    = sculk::protocol::MinecraftPackets::createPacket(header);

    if (!pkt) {
        // Неизвестный пакет — passthrough без изменений
        sHookLogger.warn(
            "[WARN] translateIncoming: unknown packet (protocol {}), passthrough.",
            clientProtocol
        );
        return {rawPacket.begin(), rawPacket.end()};
    }

    const auto readResult = pkt->read(readStream);
    if (!readResult) {
        sHookLogger.error(
            "[FAIL] translateIncoming: failed to read packet (protocol {}), passthrough.",
            clientProtocol
        );
        return {rawPacket.begin(), rawPacket.end()};
    }

    std::vector<std::byte> output;
    sculk::protocol::BinaryStream writeStream{output};
    pkt->writeWithHeader(writeStream);

    return output;
}

std::vector<std::byte> PacketTranslator::translateOutgoing(
    std::uint32_t              clientProtocol,
    std::span<const std::byte> rawPacket
) const {
    return translateIncoming(clientProtocol, rawPacket);
}

} // namespace multiversion
