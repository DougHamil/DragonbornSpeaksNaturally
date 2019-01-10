Dragonborn Speaks Naturally
==============================

DSN is composed of two projects, a plugin and a background service.

The [dsn_plugin](dsn_plugin) is a DLL written in `C++` which hooks into SkyrimVR/SE and communicates with the background service.

The background service [dsn_service](dsn_service) is a Windows application written in `C#` which uses Microsoft's `System.Speech` API to do speech recognition. It communicates with the dsn_plugin over stdin/stdout.

## Build [dsn_service](dsn_service)

You need a [Visual Studio](https://visualstudio.microsoft.com/) with `.NET Desktop Development` modules.

Then enter [dsn_service](dsn_service) directory and double-click `dsn_service.sln`, a Visual Studio C# project will be loaded.

## Build [dsn_plugin](dsn_plugin)

You need a [Visual Studio](https://visualstudio.microsoft.com/) (with `C++ Desktop Development` module) and a [CMake](https://cmake.org/).

Then enter [dsn_plugin](dsn_plugin) directory and double-click `configure.bat`, a Visual Studio project will be created by Cmake and loaded automatically.

### Auto install dlls to game directory after building

When you run `configure.bat` for the first time, you are asked about the installation path of SkyrimVR and SkyrimSE. If you want to enable the automatic installation feature, please provide the root directory of your games. If you want to disable the installation, just press Enter.

An example:
```
Set plugin install directories after building:
SkyrimVR game root path (empty to disable installation): E:/Games/SteamLibrary/steamapps/common/SkyrimVR
SkyrimSE game root path (empty to disable installation): E:/Games/SteamLibrary/steamapps/common/Skyrim Special Edition
CMakeFlags: -DSVR_DIR="E:/Games/SteamLibrary/steamapps/common/SkyrimVR" -DSSE_DIR="E:/Games/SteamLibrary/steamapps/common/Skyrim Special Edition"
```

And these commands will be execute for the example:
```bat
md build
cd build
cmake -A x64 -DSVR_DIR="E:/Games/SteamLibrary/steamapps/common/SkyrimVR" -DSSE_DIR="E:/Games/SteamLibrary/steamapps/common/Skyrim Special Edition" ..
start dsn_plugin.sln
```

A file named `install-path.ini` will be created after the first running of `configure.bat`.
You can delete it and run `configure.bat` again to reset the path, or edit it directly.

### How the Auto installation works

The installation will execute when you build the project `INSTALL` or use `Build Solution`.
Building only `dsn_plugin_xx` cannot trigger the auto installation.

### Directory structure of dsn_plugin
name          | description
------------- | -------------
[dsn_plugin/](dsn_plugin/dsn_plugin) | The code of the plugin itself.
[sse/](dsn_plugin/sse) | The [SKSE64](http://skse.silverlock.org/) codes for linking to `dsn_plugin` to generate a SkyrimSE-compatible DLL.
[svr/](dsn_plugin/svr) | The [SKSEVR](http://skse.silverlock.org/) codes for linking to `dsn_plugin` to generate a SkyrimVR-compatible DLL.
[CMakeLists.txt](dsn_plugin/CMakeLists.txt) | A project description file used by the `CMake` build tool.
[configure.bat](dsn_plugin/configure.bat) | A script to create a Visual Studio project in the `build` directory via `CMake` and load it.
build/ | After you run `configure.bat`, the directory will be created automatically to hold the `Visual Studio` project and all build outputs. You can delete this directory at any time and re-run `configure.bat` to generate it. Files in this directory should not be commited to the repository.

### About `dsn_plugin_se` and `dsn_plugin_vr` in Visual Studio

They are the same directory and have the same codes. Modifying the code in one place is equivalent to modifying another.

The only difference between the two is that the linked library and header file including paths are different.
