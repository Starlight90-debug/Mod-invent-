#pragma once

#include "VersionRegistry.h"

#include <sculk/protocol/codec/MinecraftPacketIds.hpp>
#include <sculk/protocol/codec/MinecraftPackets.hpp>
#include <sculk/protocol/codec/utility/deps/BinaryStream.hpp>
#include <sculk/protocol/codec/utility/deps/ReadOnlyBinaryStream.hpp>

#include <ll/api/io/Logger.h>
#include <ll/api/memory/Hook.h>

#include <mc/network/NetworkConnection.h>
#include <mc/network/NetworkIdentifier.h>
#include <mc/network/ServerNetworkHandler.h>
#include <mc/network/SubClientId.h>

#include <cstddef>
#include <memory>
#include <span>
#include <vector>

namespace multiversion {

class PacketTranslator {
public:
    explicit PacketTranslator(const VersionRegistry& registry)
        : mRegistry(registry) {}

    std::vector<std::byte> translateIncoming(std::uint32_t, std::span<const std::byte>) const;
    std::vector<std::byte> translateOutgoing(std::uint32_t, std::span<const std::byte>) const;

private:
    const VersionRegistry& mRegistry;
};

class PacketHook {
public:
    explicit PacketHook(const VersionRegistry& registry, ll::io::Logger& logger)
        : mRegistry(registry), mLogger(logger), mTranslator(registry) {}

    bool install();
    void uninstall();

    const VersionRegistry& getRegistry() const { return mRegistry; }
    ll::io::Logger& getLogger() const { return mLogger; }

private:
    const VersionRegistry& mRegistry;
    ll::io::Logger& mLogger;
    PacketTranslator mTranslator;

    bool mInstalled{false};
};

} // namespace multiversion
