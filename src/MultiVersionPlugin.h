#pragma once

#include "PacketHook.h"
#include "VersionRegistry.h"

#include <ll/api/mod/NativeMod.h>

#include <memory>

namespace multiversion {

class MultiVersionPlugin final : public ll::mod::NativeMod {
public:
    explicit MultiVersionPlugin(ll::mod::NativeModDescriptor&& desc)
        : ll::mod::NativeMod(std::move(desc)) {}

    [[nodiscard]] static MultiVersionPlugin& getInstance();

    bool load() override;
    bool enable() override;
    bool disable() override;

private:
    void registerSupportedVersions();

    std::unique_ptr<VersionRegistry> mVersionRegistry;
    std::unique_ptr<PacketHook>      mPacketHook;
};

} // namespace multiversion
