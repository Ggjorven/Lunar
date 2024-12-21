@echo off

pushd %~dp0\..\..\

call vendor\premake\premake5.exe gmake2 --os=windows --cc=clang
call vendor\premake\premake5.exe clangd-support --os=windows --cc=clang

popd
PAUSE
