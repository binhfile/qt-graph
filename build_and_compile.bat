@echo off
setlocal enabledelayedexpansion

REM Setup MSVC environment
call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat"

REM Change to project directory
cd /d "C:\Users\ngoth\Downloads\files"

REM Run qmake
echo Running qmake...
"C:\Qt\Qt5.12.12\5.12.12\msvc2017_64\bin\qmake.exe" xy_chart_widget.pro -spec win32-msvc
if !errorlevel! neq 0 (
    echo qmake failed with error code !errorlevel!
    exit /b !errorlevel!
)

REM Run nmake to build
echo Running nmake...
nmake
if !errorlevel! neq 0 (
    echo nmake failed with error code !errorlevel!
    exit /b !errorlevel!
)

echo Build completed successfully!
pause
