@echo off & setlocal ENABLEDELAYEDEXPANSION

:: jump to script's parent dir
cd /d %~dp0

:: load/create option files
SET InstallPathConfig=install-path.ini

:: read or generate config files
if exist %InstallPathConfig% (
    call :load_ini %InstallPathConfig%
    call :get_ini SkyrimVR InstallPath SkyrimVRInstallPath
    call :get_ini SkyrimSE InstallPath SkyrimSEInstallPath
) else (
    echo Set plugin install directories after building:
    set /p SkyrimVRInstallPath="SkyrimVR game root path (empty to disable installation): "
    set /p SkyrimSEInstallPath="SkyrimSE game root path (empty to disable installation): "

    call :set_ini SkyrimVR InstallPath "!SkyrimVRInstallPath!"
    call :set_ini SkyrimSE InstallPath "!SkyrimSEInstallPath!"
    call :save_ini >%InstallPathConfig%
)

:: create and enter build dir
md build
cd build

:: run CMake
set CMakeFlags=
if defined SkyrimVRInstallPath (
    set CMakeFlags=!CMakeFlags! -DSVR_DIR="!SkyrimVRInstallPath!"
)
if defined SkyrimSEInstallPath (
    set CMakeFlags=!CMakeFlags! -DSSE_DIR="!SkyrimSEInstallPath!"
)
echo CMakeFlags:!CMakeFlags!

cmake -A x64 !CMakeFlags! ..

:: load project
start dsn_service.sln

:: pause for user's lookup
pause
goto :eof


:: ----------------- ini parse & edit functions -----------------
:: From <https://zhidao.baidu.com/question/982407720655882739.html>

:load_ini [param#1=ini file path]
set "op="
for /f " usebackq tokens=1* delims==" %%a in ("%~1") do (
    if "%%b"=="" (
        set "op=%%a"
    ) else (
        set "##!op!#%%a=%%b"
    )
)
goto :eof

:get_ini [param#1=Option] [param#2=Key] [param#3=StoredVar]
set %~3=!##[%~1]#%~2!
goto :eof

:set_ini [param#1=Option] [param#2=Key] [param#3=Value, without it the key will be deleted]
set "##[%~1]#%~2=%~3"
goto :eof

:save_ini [>ini path]
set "op="
set "##=##"
for /f "tokens=1-3 delims=#=" %%a in ('set ##') do (
    if "%%a"=="!op!" (
        echo,%%b=%%c
    ) else (
        echo,%%a
        set "op=%%a"
        echo,%%b=%%c
    )
)
