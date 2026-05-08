add_rules("mode.debug", "mode.release", "mode.releasedbg")

add_repositories("liteldev-repo https://github.com/LiteLDev/xmake-repo.git")

add_requires("levilamina 26.10.10")

if not has_config("vs_runtime") then
    set_runtimes("MD")
end

set_languages("cxx23")

local protocol_include = os.getenv("PROTOCOL_INCLUDE_DIR") or "vendor/Protocol/include"

target("MultiVersionPlugin")
    set_kind("shared")
    set_languages("cxx23")

    -- Исходники и include пути
    add_files("src/**.cpp")
    add_includedirs("src")

    -- Protocol headers (только include/ — .dll уже на сервере в libraries/)
    add_includedirs(protocol_include)

    -- LeviLamina SDK — автоматически пробрасывает все свои include пути
    add_packages("levilamina")

    -- MSVC флаги
    add_cxflags(
        "/EHa",       -- async exceptions (требует LeviLamina)
        "/utf-8",     -- UTF-8 исходники
        "/W3",        -- предупреждения уровень 3
        "/wd4251",    -- подавить: dll-интерфейс для STL типов
        "/wd4275"     -- подавить: non dll-interface base class
    )

    -- Дефайны Windows
    add_defines(
        "NOMINMAX",          -- убрать min/max макросы Windows.h
        "UNICODE",
        "_UNICODE",
        "WIN32_LEAN_AND_MEAN"
    )

    -- Линковка: отложенная загрузка BDS
    add_shflags("/DELAYLOAD:bedrock_server.dll")

    -- Отключить C++ exceptions чтобы не конфликтовало с /EHa
    set_exceptions("none")

    -- Release: максимальная оптимизация + PDB для диагностики крашей
    if is_mode("release") then
        set_optimize("fastest")
        set_symbols("debug")
    end

    -- Debug: дополнительные дефайны
    if is_mode("debug") then
        add_defines("MULTIVERSION_DEBUG")
        set_symbols("debug")
        set_optimize("none")
    end
