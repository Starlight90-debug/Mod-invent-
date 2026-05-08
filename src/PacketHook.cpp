#include "PacketHook.h"
#include "ClientVersionMap.h"

#include <ll/api/io/Logger.h>
#include <ll/api/memory/Hook.h>

#include <mc/network/NetworkIdentifier.h>
#include <mc/network/ServerNetworkHandler.h>
#include <mc/network/packet/LoginPacket.h>
#include <mc/network/SubClientId.h>

namespace multiversion {

static PacketHook* gActiveHook{nullptr};
static ll::io::Logger sHookLogger("MultiVersion");

LL_TYPE_INSTANCE_HOOK(
    LoginPacketHook,
    ll::memory::HookPriority::Normal,
    ServerNetworkHandler,
    "?handle@ServerNetworkHandler@@UEAAXAEBVNetworkIdentifier@@AEBVLoginPacket@@@Z",
    void,
    const NetworkIdentifier& netId,
    const LoginPacket& packet
) {
    if (gActiveHook) {
        auto proto = (uint32_t)packet.getProtocolVersion();
        auto addr = netId.getRealAddress().ToString();

        ClientVersionMap::getInstance().set(addr, proto);

        if (proto == 944)
            sHookLogger.info("Native client connected");
        else if (gActiveHook->getRegistry().isSupported(proto))
            sHookLogger.info("Supported client connected");
        else
            sHookLogger.warn("Unknown protocol");
    }

    origin(netId, packet);
}

bool PacketHook::install() {
    if (mInstalled) return true;

    gActiveHook = this;

    mInstalled = LoginPacketHook::hook();
    return mInstalled;
}

void PacketHook::uninstall() {
    gActiveHook = nullptr;
    mInstalled = false;
}

} // namespace multiversion
