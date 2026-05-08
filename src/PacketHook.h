#pragma once

#include "VersionRegistry.h"

// Protocol (SculkCatalystMC)
#include <sculk/protocol/codec/MinecraftPacketIds.hpp>
#include <sculk/protocol/codec/MinecraftPackets.hpp>
#include <sculk/protocol/codec/utility/deps/BinaryStream.hpp>
#include <sculk/protocol/codec/utility/deps/ReadOnlyBinaryStream.hpp>

// LeviLamina 1.26.10
#include <ll/api/io/Logger.h>
#include <ll/api/memory/Hook.h>

// Network
#include <mc/network/NetworkConnection.h>
#include <mc/network/NetworkIdentifier.h>
#include <mc/network/ServerNetworkHandler.h>
#include <mc/network/packet/LoginPacket.h>
#include <mc/network/packet/Packet.h>
#include <mc/network/SubClientId.h>

#include <cstddef>
#include <memory>
#include <span>
#include <vector>

namespace multiversion {

// -----------------------------------------------------------------------
// PacketTranslator
// -----------------------------------------------------------------------
class PacketTranslator {
public:
    explicit PacketTranslator(const VersionRegistry& registry)
        : mRegistry(registry) {}

    [[nodiscard]] std::vector<std::byte> translateIncoming(
        std::uint32_t              clientProtocol,
        std::span<const std::byte> rawPacket
    ) const;

    [[nodiscard]] std::vector<std::byte> translateOutgoing(
        std::uint32_t              clientProtocol,
        std::span<const std::byte> rawPacket
    ) const;

private:
    [[nodiscard]] bool needsTranslation(std::uint32_t clientProtocol) const;

    [[nodiscard]] sculk::protocol::MinecraftPackets::PacketHeader
    readHeader(sculk::protocol::ReadOnlyBinaryStream& stream) const;

private:
    const VersionRegistry& mRegistry;
};

// -----------------------------------------------------------------------
// PacketHook
// -----------------------------------------------------------------------
class PacketHook {
public:
    explicit PacketHook(
        const VersionRegistry& registry,
        ll::io::Logger&        logger
    )
        : mRegistry(registry)
        , mLogger(logger)
        , mTranslator(registry) {}

    bool install();
    void uninstall();

    [[nodiscard]] const VersionRegistry& getRegistry() const {
        return mRegistry;
    }

    [[nodiscard]] ll::io::Logger& getLogger() const {
        return mLogger;
    }

private:
    const VersionRegistry& mRegistry;
    ll::io::Logger&        mLogger;
    PacketTranslator       mTranslator;

    bool mInstalled{false};
};

} // namespace multiversion
