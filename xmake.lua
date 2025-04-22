add_rules("mode.debug", "mode.release", "mode.releasedbg")
add_rules("plugin.vsxmake.autoupdate")
if is_mode("debug") then
    set_runtimes("MDd")
    add_requires("webui", {debug = true})
    add_requires("date", {debug = true})
else
    set_runtimes("MD")
    add_requires("webui", {configs = {log = false}})
    add_requires("date")
end
includes("ui/xmake.lua")

target("WebUIXMakeTemplate")
    add_packages(
        "webui",
        "date"
    )
    add_cxflags(
        "cl::/utf-8"
    )
    set_warnings("allextra")
    if is_mode("debug") then
        add_defines("_DEBUG")
        set_symbols("debug", "edit")
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
