# Build MMVE As A Windows VST3

## Prerequisites

- Windows 10/11 x64
- Visual Studio 2022 with "Desktop development with C++"
- CMake 3.22+
- A local JUCE checkout

Place JUCE in `.\JUCE`, or pass the checkout path with the `JUCE_DIR` environment variable:

```powershell
$env:JUCE_DIR = "C:\path\to\JUCE"
```

## Build

From the project root:

```powershell
.\build-windows-vst3.cmd
```

Or, from a Visual Studio Developer PowerShell/Command Prompt:

```powershell
cmake --preset windows-vs2022-release
cmake --build --preset windows-release
```

## Output

The VST3 bundle is produced at:

```text
build-win\MMVE_artefacts\Release\VST3\MMVE.vst3
```

This repo also stages Windows release packages in:

```text
release\MMVE-0.6.0-Windows\VST3\MMVE.vst3
release\MMVE-0.6.0-Windows.zip
```

To install it for the current user, copy `MMVE.vst3` to:

```text
%LOCALAPPDATA%\Programs\Common\VST3
```

For a system-wide install, copy it to:

```text
C:\Program Files\Common Files\VST3
```

## Notes

- Windows builds default to `VST3` only.
- To build other JUCE formats, override `MMVE_PLUGIN_FORMATS`, for example:

```powershell
cmake -B build-win -G "NMake Makefiles" -DCMAKE_BUILD_TYPE=Release -DMMVE_JUCE_PATH="C:\path\to\JUCE" -DMMVE_PLUGIN_FORMATS="VST3;Standalone"
```
