#pragma once

#include "VersionRegistry.h"

// Protocol (SculkCatalystMC) — используется как библиотека в папке libraries
#include <sculk/protocol/codec/MinecraftPacketIds.hpp>
#include <sculk/protocol/codec/MinecraftPackets.hpp>
#include <sculk/protocol/codec/utility/deps/BinaryStream.hpp>
#include <sculk/protocol/codec/utility/deps/ReadOnlyBinaryStream.hpp>

#include <ll/api/Logger.h>
#include <ll/api/memory/Hook.h>

// LeviLamina network types
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
// Переводит пакет с одного protocol ID на другой.
// Логика: если клиент подключился с более старой версией —
//         пакеты «апгрейдятся» до серверной версии при входе,
//         и «даунгрейдятся» при отправке на клиент.
// -----------------------------------------------------------------------
class PacketTranslator {
public:
    explicit PacketTranslator(const VersionRegistry& registry)
        : mRegistry(registry) {}

    // Применяет трансляцию к сырым байтам пакета.
    // clientProtocol — protocol ID подключённого клиента.
    // Возвращает (возможно изменённые) данные.
    [[nodiscard]] std::vector<std::byte> translateIncoming(
        std::uint32_t            clientProtocol,
        std::span<const std::byte> rawPacket) const;

    [[nodiscard]] std::vector<std::byte> translateOutgoing(
        std::uint32_t            clientProtocol,
        std::span<const std::byte> rawPacket) const;

private:
    // Проверяет, нужна ли трансляция
    [[nodiscard]] bool needsTranslation(std::uint32_t clientProtocol) const;

    // Читает заголовок пакета (packetId + subClientIds) из потока
    [[nodiscard]] sculk::protocol::MinecraftPackets::PacketHeader
    readHeader(sculk::protocol::ReadOnlyBinaryStream& stream) const;

    const VersionRegistry& mRegistry;
};

// -----------------------------------------------------------------------
// PacketHook
// Устанавливает хук на ServerNetworkHandler через ll::memory::Hook,
// перехватывает входящие пакеты и применяет трансляцию.
// -----------------------------------------------------------------------
class PacketHook {
public:
    explicit PacketHook(const VersionRegistry& registry, ll::Logger& logger)
        : mRegistry(registry)
        , mLogger(logger)
        , mTranslator(registry) {}

    bool install();
    void uninstall();

    // Используется из статических хуков
    [[nodiscard]] const VersionRegistry& getRegistry() const { return mRegistry; }
    [[nodiscard]] ll::Logger&            getLogger()   const { return mLogger;   }

private:
    const VersionRegistry& mRegistry;
    ll::Logger&            mLogger;
    PacketTranslator       mTranslator;

    bool mInstalled{false};
};

} // namespace multiversion
