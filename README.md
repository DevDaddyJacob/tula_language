# Tula Language
**Tula** is a custom programming language created initially as a time-kill 
personal project.


---
# Development Environment
Tula is a project written in **C99**, built with CMake.
<p>
The IDE used to interact with the codebase is [CLion](https://www.jetbrains.com/clion/).


## System Setup

### Windows
- [JetBrains CLion 2026.1](https://www.jetbrains.com/clion/download/other/#releases-2026)
- [Build Tools for Visual Studio 2026](https://visualstudio.microsoft.com/downloads/#build-tools-for-visual-studio-2026)
- CMake 4.2 *(This comes standard with CLion)*
  - Alternatively, you may [download](https://cmake.org/download/) and install
    CMake and add the binaries to your `PATH` variable.

### Linux
- [JetBrains CLion 2026.1](https://www.jetbrains.com/clion/download/other/#releases-2026)
- CMake 4.2
  - Debian / Ubuntu: `sudo apt install build-essential cmake ninja-build`
  - Fedora / RHEL: `sudo apt install cmake ninja-build`
  - Arch: `sudo pacman -S gcc cmake ninja`


## CLion setup

### Windows
1. Open CLion and load the repository root as a project.
2. Go to **Settings → Build, Execution, Deployment → Toolchains**.
3. Click **+** and select **Visual Studio**.
4. Set **Architecture** to `amd64`. Leave all other fields on auto-detect —
   CLion will find the VS 2022 installation automatically.
5. Go to **Settings → Build, Execution, Deployment → CMake**.
6. Ensure the **Toolchain** for your Debug profile is set to the Visual Studio
   entry you just created.
7. Click **OK**, then **File → Reload CMake Project**.

You should see `Toolchain: MSVC (Windows)` in the CMake output. If you see
`FATAL_ERROR: Unsupported compiler`, the toolchain is still set to the bundled
MinGW — go back to step 3.

### Linux
1. Open the repository root as a project.
2. CLion will detect the system GCC automatically.
3. Confirm **Settings → Build, Execution, Deployment → Toolchains** shows the
   system GCC (usually `/usr/bin/gcc`).
4. **File → Reload CMake Project**.

You should see `Toolchain: GCC (Linux)` in the CMake output.