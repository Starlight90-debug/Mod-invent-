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

    add_cxflags("/EHa", "/utf-8")
    add_files("src/**.cpp")
    add_includedirs("src")
    add_includedirs(protocol_include)
    add_packages("levilamina")
    add_shflags("/DELAYLOAD:bedrock_server.dll")
    set_exceptions("none")
