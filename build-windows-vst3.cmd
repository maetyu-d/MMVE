@echo off
setlocal

set "ROOT=%~dp0"
cd /d "%ROOT%" || exit /b 1

if not defined JUCE_DIR (
    if exist "%ROOT%JUCE\CMakeLists.txt" (
        set "JUCE_DIR=%ROOT%JUCE"
    )
)

if not defined JUCE_DIR (
    echo JUCE_DIR is not set and .\JUCE was not found.
    echo Set JUCE_DIR to a local JUCE checkout, or place JUCE in this folder.
    exit /b 1
)

if exist "C:\BuildTools\VC\Auxiliary\Build\vcvars64.bat" (
    call "C:\BuildTools\VC\Auxiliary\Build\vcvars64.bat"
)

set "CMAKE=cmake"
where cmake >nul 2>nul
if errorlevel 1 (
    if exist "C:\BuildTools\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe" (
        set "CMAKE=C:\BuildTools\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe"
    )
)

"%CMAKE%" --fresh --preset windows-vs2022-release
if errorlevel 1 exit /b %errorlevel%

"%CMAKE%" --build --preset windows-release
if errorlevel 1 exit /b %errorlevel%

echo.
echo Built: build-win\MMVE_artefacts\Release\VST3\MMVE.vst3
