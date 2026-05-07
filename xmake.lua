add_rules("mode.debug", "mode.release", "mode.releasedbg")

add_repositories("liteldev-repo https://github.com/LiteLDev/xmake-repo.git")

-- Указываем точную версию LeviLamina под BDS 1.26.10
add_requires("levilamina 26.10.10")

-- MSVC runtime — обязательно MD, иначе LeviLamina не установится
if not has_config("vs_runtime") then
    set_runtimes("MD")
end

set_languages("cxx23")

local protocol_include = os.getenv("PROTOCOL_INCLUDE_DIR") or "vendor/Protocol/include"

target("MultiVersionPlugin")
    set_kind("shared")
    set_filename("MultiVersionPlugin")

    add_files("src/**.cpp")
    add_includedirs("src")

    -- LeviLamina: add_packages автоматически пробрасывает includedirs/links
    add_packages("levilamina")

    -- Protocol headers (только include — .dll уже на сервере в libraries/)
    add_includedirs(protocol_include)

    -- Windows
    add_cxflags("/EHa", "/utf-8")
    add_defines("NOMINMAX", "UNICODE", "_UNICODE", "WIN32_LEAN_AND_MEAN")
    add_shflags("/DELAYLOAD:bedrock_server.dll")
    set_exceptions("none")  -- избегаем конфликта с /EHa

    if is_mode("release") then
        set_optimize("fastest")
        set_symbols("debug")
    end

    if is_mode("debug") then
        add_defines("MULTIVERSION_DEBUG")
    end
