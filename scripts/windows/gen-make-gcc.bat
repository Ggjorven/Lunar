@echo off

pushd %~dp0\..\..\

call vendor\premake\premake5.exe gmake --os=windows --cc=gcc

popd
PAUSE
