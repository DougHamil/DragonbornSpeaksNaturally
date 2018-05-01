Dragonborn Speaks Naturally
==============================

DSN is composed of two projects, a plugin and a background service.
The dsn_plugin is a DLL written in C++ which hooks into SkyrimVR and communicates with the background service.
The background service is a Windows application written in C# which uses Microsoft's System.Speech API to do speech recognition. It communicates with the dsn_plugin over stdin/stdout.
