@ECHO OFF

SETLOCAL

set CommonLibDir=C:\dev\shared\libs
set CommonIncludeDir=C:\dev\shared\include

set RootDir=%CurrProjDir:~0,-1%

set BuildDir=%RootDir%\build
set SourceDir=%RootDir%\source

set CompilerOptions=/I%CommonIncludeDir% /MTd /nologo /FC /GR- /Z7 /EHa- /Od /Oi

REM warnings that maybe shouldn't be disabled:
    REM 4996 (using deprecated functions like fopen, not fopen_s),
    REM 4456 (to compile with raygui.h -- hiding previous declaration)
    REM 4267 (to compile with raygui.h -- conversion with possible loss of data - from size_t)
set CompilerWarningOptions=/WX /W4 /wd4201 /wd4100 /wd4189 /wd4505 /wd4996 /wd4456 /wd4267

set LinkOptions=/LIBPATH:%CommonLibDir% /INCREMENTAL:NO /OPT:REF /SUBSYSTEM:CONSOLE
set LinkLibs=raylibdll.lib raylib.lib

pushd %BuildDir%

cl %SourceDir%\proto7.cpp %CompilerOptions% %CompilerWarningOptions% /link %LinkOptions% %LinkLibs%

popd

if %ERRORLEVEL% NEQ 0 (
    exit %ERRORLEVEL%
)

ENDLOCAL
