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

### Directory structure of dsn_plugin
name          | description
------------- | -------------
[dsn_plugin/](dsn_plugin/dsn_plugin) | The code of the plugin itself.
[sse/](dsn_plugin/sse) | The [SKSE64](http://skse.silverlock.org/) codes for linking to `dsn_plugin` to generate a SkyrimSE-compatible DLL.
[svr/](dsn_plugin/svr) | The [SKSEVR](http://skse.silverlock.org/) codes for linking to `dsn_plugin` to generate a SkyrimVR-compatible DLL.
[CMakeLists.txt](dsn_plugin/CMakeLists.txt) | A project description file used by the `CMake` build tool.
[configure.bat](dsn_plugin/configure.bat) | A script to create a Visual Studio project in the `build` directory via `CMake` and load it.

