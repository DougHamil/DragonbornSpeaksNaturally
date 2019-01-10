@echo off
md build
cd build
cmake -A x64 ..
start dsn_plugin.sln
pause
