@echo off

pushd %~dp0\..\..\

call vendor\premake\premake5.exe gmake2 --os=windows --cc=gcc
call vendor\premake\premake5.exe clangd-support --os=windows --cc=gcc

popd
PAUSE
