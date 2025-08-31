[![CMake on Windows](https://github.com/chadlrnsn/ImGui-DirectX-12-Kiero-Hook/actions/workflows/cmake-single-platform.yml/badge.svg)](https://github.com/chadlrnsn/ImGui-DirectX-12-Kiero-Hook/actions/workflows/cmake-single-platform.yml)

# Abyssus

A UE5 hack for the single-player game Abyssus. Supports DX12 (via [chadlrnsn's DX12 hook](https://github.com/chadlrnsn/ImGui-DirectX-12-Kiero-Hook) ) and DX11 via my own implementation.

## Features

- DirectX 11/12 Hook implementation
- ImGui integration through Kiero
- Resizable window support
- Fullscreen support
- Minimal performance impact
- Doesn't crash when injected instantly (before window is created)

## Requirements

- Windows 10/11
- Visual Studio 2019/2022 or VSCode
- [DirectX SDK](https://www.microsoft.com/en-us/download/details.aspx?id=6812)
- CMake 3.15 or higher
- Git

## Building the Project

### Using VSCode (Recommended)

1. Install the following extensions:
   - [CMake Tools](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cmake-tools)
   - [C/C++](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cpptools)

2. Clone the repository with submodules:

```bash
git clone --recurse-submodules https://github.com/chadlrnsn/ImGui-DirectX-12-Kiero-Hook
cd ImGui-DirectX-12-Kiero-Hook
```

3. Open the project in VSCode and select a kit using CMake Tools:
   - `Visual Studio Build Tools 2019 Release - amd64`
   - `Visual Studio Build Tools 2022 Release - amd64`
   - `Clang 16.0.0 x86_64-pc-windows-msvc`

4. Select build variant (Debug/Release) and build using CMake Tools

### Using Command Line

1. Clone the repository as shown above
2. Build using presets:

```bash
# For VS2019 Release
cmake --preset windows-x64-release-2019
cmake --build --preset windows-x64-release-2019

# For VS2019 Debug
cmake --preset windows-x64-debug-2019
cmake --build --preset windows-x64-debug-2019

# For VS2022 Release
cmake --preset windows-x64-release-2022
cmake --build --preset windows-x64-release-2022

# For VS2022 Debug
cmake --preset windows-x64-debug-2022
cmake --build --preset windows-x64-debug-2022
```

The DLL will be in the `bin/{Debug|Release}` directory.

[!NOTE]
Only x64 builds are supported due to DirectX 12 limitations.

## Implementation Details

This project uses:
- [chadlrnsn's DX12 hook](https://github.com/chadlrnsn/ImGui-DirectX-12-Kiero-Hook) as a DX12 base
- [Kiero](https://github.com/Rebzzel/kiero) for function hooking
- [Dear ImGui](https://github.com/ocornut/imgui) for the user interface
- [MinHook](https://github.com/TsudaKageyu/minhook) for API hooking

## Contributing

Feel free to submit issues and pull requests.
