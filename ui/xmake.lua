task("webuidev")
    on_run(function (browserPath)
        local oldir = os.cd(os.scriptdir())
        if os.host() == "windows" then
            os.setenv("VITE_BASE_URL", "http://localhost:9000")
            os.execv("cmd",{"/C", "start", "pnpm.cmd","dev","--port","5173","--strictPort"})

            os.cd(path.directory(browserPath))
            os.execv(browserPath,{"http://localhost:5173/"})

        else
            -- os.execv("pnpm",{"dev","--open",browserPath})
        end
        os.cd(oldir)
    end)

target("ui")
    add_extrafiles("*.json")
    add_extrafiles("*.ts")
    add_extrafiles("*.js")
    add_extrafiles("*.html")
    add_extrafiles("src/**")
    add_extrafiles("public/**")
    on_build(function (target)
        local option = import("core.base.option")
        local oldir = os.cd(os.scriptdir())
        os.setenv("UI_BUILD_DIR", target:targetdir().."/dist")
        if not os.exists("node_modules") then
            if os.host() == "windows" then
                if option.get("verbose") then
                    os.exec("pnpm.cmd install")
                else
                    os.run("pnpm.cmd install")
                end
            else
                if option.get("verbose") then
                    os.exec("pnpm install")
                else
                    os.run("pnpm install")
                end
            end
        end
        if os.host() == "windows" then
            if option.get("verbose") then
                os.exec("pnpm.cmd build --emptyOutDir")
            else
                os.run("pnpm.cmd build --emptyOutDir")
            end
        else
            if option.get("verbose") then
                os.exec("pnpm build --emptyOutDir")
            else
                os.run("pnpm build --emptyOutDir")
            end
        end
        os.cd(oldir)
    end)
