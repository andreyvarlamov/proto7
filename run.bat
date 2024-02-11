@echo off

setlocal

set rootdir=%~dp0
set bindir=%rootdir%build

pushd %rootdir%

%bindir%\proto7.exe

popd
