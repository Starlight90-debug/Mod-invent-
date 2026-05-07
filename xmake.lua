-- xmake.lua
-- MultiVersionPlugin — LeviLamina plugin
-- Protocol (SculkCatalystMC) уже установлен на сервере в папке libraries/.
-- При сборке указываем путь к его заголовкам через переменную окружения
-- PROTOCOL_INCLUDE_DIR (задаётся в GitHub Actions vars).
-- По умолчанию — "vendor/Protocol/include" (папка только с include/ из репо).
--
-- xmake f -y -p windows -a x64 -m release
-- xmake

add_rules("mode.debug", "mode.release", "mode.releasedbg")

-- LeviLamina xmake repo
add_repositories("liteldev-repo https://github.com/LiteLDev/xmake-repo.git")

-- Зависимости
add_requires("levilamina")
add_requires("openssl", {configs = {shared = false}})

set_languages("cxx23")

-- Путь к заголовкам Protocol: переопределяется через env или оставляем vendor/
local protocol_include = os.getenv("PROTOCOL_INCLUDE_DIR") or "vendor/Protocol/include"

target("MultiVersionPlugin")
    set_kind("shared")
    set_filename("MultiVersionPlugin")

    add_files("src/**.cpp")
    add_includedirs("src")

    -- LeviLamina SDK
    add_packages("levilamina")

    -- Protocol headers (только include — .dll/.so уже лежит на сервере в libraries/)
    add_includedirs(protocol_include)

    -- OpenSSL (Protocol использует его внутри)
    add_packages("openssl")

    -- Windows
    if is_plat("windows") then
        add_defines("NOMINMAX", "UNICODE", "_UNICODE", "WIN32_LEAN_AND_MEAN")
        add_cxxflags("/utf-8", "/W3", "/wd4251", "/wd4275")
        add_ldflags("/DELAYLOAD:bedrock_server.dll")
    end

    if is_mode("release") then
        set_optimize("fastest")
        set_symbols("debug")
    end

    if is_mode("debug") then
        add_defines("MULTIVERSION_DEBUG")
    end
