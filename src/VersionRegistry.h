#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace multiversion {

// Описывает одну поддерживаемую версию Bedrock
struct ProtocolVersion {
    std::uint32_t protocolId;   // Числовой ID протокола (напр. 766)
    std::string   mcVersion;    // Строка версии Minecraft (напр. "1.21.50")
};

// Реестр всех поддерживаемых версий протокола.
// Плагин использует его для трансляции пакетов между версиями.
class VersionRegistry {
public:
    // Регистрирует поддерживаемую версию
    void registerVersion(const ProtocolVersion& version);

    // Возвращает информацию о версии по её protocol ID
    [[nodiscard]] std::optional<ProtocolVersion> findByProtocolId(std::uint32_t protocolId) const;

    // Поддерживается ли данный protocol ID этим плагином
    [[nodiscard]] bool isSupported(std::uint32_t protocolId) const;

    // Количество зарегистрированных версий
    [[nodiscard]] std::size_t getSupportedCount() const;

    // Список всех зарегистрированных protocol ID
    [[nodiscard]] std::vector<std::uint32_t> getAllProtocolIds() const;

    // Серверный (нативный) protocol ID — версия, на которой запущен BDS
    // Устанавливается при первом подключении / StartGame пакете
    void setServerProtocolId(std::uint32_t id);
    [[nodiscard]] std::uint32_t getServerProtocolId() const;

private:
    // key = protocolId
    std::unordered_map<std::uint32_t, ProtocolVersion> mVersions;
    std::uint32_t mServerProtocolId{0};
};

} // namespace multiversion
