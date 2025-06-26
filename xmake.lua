add_rules("mode.debug", "mode.release", "mode.releasedbg")
add_rules("plugin.vsxmake.autoupdate")
if is_mode("debug") then
    set_runtimes("MDd")
    add_requires("webui", {debug = true,configs = {log = false}})
    add_requires("date", {debug = true})
    add_requires("nlohmann_json", {debug = true})
else
    set_runtimes("MD")
    add_requires("webui", {configs = {log = false}})
    add_requires("date")
    add_requires("nlohmann_json")
end
includes("ui/xmake.lua")

target("AutoInput")
    add_packages(
        "webui",
        "date",
        "nlohmann_json"
    )
    add_cxflags(
        "cl::/utf-8"
    )
    set_warnings("allextra")
    if is_mode("debug") then
        add_defines("_DEBUG")
        set_symbols("debug", "edit")
        set_runargs("http://localhost:5173/")
    else
        if is_plat("windows") then
            add_ldflags("-subsystem:windows", {force = true})
        end
    end
    set_kind("binary")
    add_deps("ui")
    add_includedirs("src")
    add_headerfiles("src/**.h")
    add_files("src/**.cpp")
    set_languages("clatest", "cxxlatest")

    on_run(function (target)
        -- 导入配置模块
        import("core.project.config")
        -- 获取配置值
        if config.mode() == "debug" then
            -- 导入task模块
            import("core.base.task")
            -- 运行这个webuidev task
            task.run("webuidev",{},path.absolute(target:targetfile()))
        else
            local oldir = os.cd(target:targetdir())
            os.execv(target:targetfile())
            os.cd(oldir)
        end
    end)
