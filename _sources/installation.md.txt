(installation-page)=
# Installation

`rallyplot` can be used from either Python or C++. 
It runs on Windows, macOS and most Linux distributions (see [below](supported-platforms) for details).

A graphics card is required for the performance benefits of using `rallyplot`. 
`rallyplot` is written for OpenGL 3.3, which the majority of graphics cards from
~2010 onwards should support. See the [GPU compatability section](gpu-section)
for details.

Once you have installed `rallyplot`, you can check out the [Get Started](get-started-tutorial) page.

## Install

::::::{tab-set}

:::::{tab-item} Python
:sync: python

`rallyplot` comes with pre-built wheels for Windows, macOS 
and Linux. Install with:

```bash
pip install rallyplot
```

:::::

:::::{tab-item} C++
:sync: cpp

`rallyplot` can be built from source using CMake. Header files to include are
found in `src/cpp/include`. All header-only library dependencies are vendored 
in `src/vendored`, as well as `FreeType`, which is build alongside `rallyplot`
with the `add_subdirectory` CMake directive. `Qt` is also a depedency, as is 
not vendored:

**Qt**

CMake `find_package(Qt6 ...)` is used to locate Qt. 
If Qt is not found automatically, you can help CMake by setting `Qt6_DIR` / `Qt_DIR`
(see the 'build directly...' example below).

In the examples below, Qt 6.8.2 is used, but other Qt6 versions should work as well.

:::{dropdown} **Use CMake subdirectory**

This will use Cmake to build `rallyplot` as part of your application build.

```cmake

cmake_minimum_required(VERSION 3.16)

project(myPlayingProject LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add rallyplot to the build
add_subdirectory(
    "C:/Users/Jzimi/git-repos/rallyplot"

    # requires two arguments if folder is not a subdirectory of your project
    "${CMAKE_BINARY_DIR}/rallyplot-build"
)

# Build your exe
add_executable(myPlayingProject main.cpp)

# Link to rallyplot
target_link_libraries(myPlayingProject PRIVATE rallyplot)

```

:::

:::{dropdown} **Build directly with CMake**

This example builds `rallyplot` with CMake.

Note on windows, forward slashes are _required_ in the environment paths.
```
set QT_DIR=C:/Users/Jzimi/git-repos/rallyplot/distribution/qt/6.8.2/msvc2022_64/lib/cmake/Qt6
set QT6_DIR=C:/Users/Jzimi/git-repos/rallyplot/distribution/qt/6.8.2/msvc2022_64/lib/cmake/Qt6

cd rallyplot
mkdir build
cd build
cmake build
cmake --build . --config Release

```

:::

:::::

::::::

(supported-platforms)=
## Supported Platforms

::::{tab-set}

:::{tab-item} Windows
:sync: windows

Windows 10 or newer (64-bit x64 and ARM64) is supported.

:::

:::{tab-item} macOS
:sync: macos

**macOS 12 Monterey or newer** (Intel & Apple Silicon) are supported.

:::

:::{tab-item} Linux
:sync: linux

**Wayland** is recommended as a display protocol over x11. While `rallyplot`
will run on x11, it is slow and displays with some rendering issues.

**The below pertains to the available Python builds**:

`rallyplot` is built on `manylinux_2_28`, working on

- Works on mainstream distros such as:
  - Ubuntu 20.04+, Debian 10/11/12  
  - RHEL / CentOS / AlmaLinux 8+  

MUSL-based systems (e.g., Alpine Linux) are not supported.

### Library dependencies

`rallyplot` does **not** vendor low-level system graphics libraries.  
While these should be included in your distro, please check they are
installed when running into dependecy errors:

- `libEGL.so.1`  
- `libGLX.so.0`  
- `libOpenGL.so.0`  
- `libgbm.so.1`  
- `libxcb.so.1`  
- `libxkbcommon.so.0`  
- Wayland client libraries (`libwayland-client.so.0`, `libwayland-cursor.so.0`)  

:::

::::  

(gpu-section)=
## GPU Compatibility

`rallyplot` requires a GPU for rendering. Since it is built on OpenGL 3.3.
it should be compatible with nearly all in-use GPUs. 

Note performance of the plots is directly related to your GPU capabilities. 

To check which GPU your system is running on, use the commands below:

::::{tab-set}

:::{tab-item} Windows
:sync: windows

Open a **Command Prompt** or **PowerShell** and run:

```powershell
# Lists all GPUs
wmic path win32_VideoController get name

# Or using DirectX diagnostic tool
dxdiag /t dxdiag.txt
type dxdiag.txt | findstr /C:"Display"
```

If you have an **NVIDIA GPU** and drivers installed:

```powershell
nvidia-smi
```

**Driver requirements**  
- NVIDIA: driver **390+** (2018 or newer)  
- AMD: Adrenalin drivers from **2017+**  
- Intel: bundled DCH drivers on Windows 10/11 already support OpenGL 3.3  

:::

:::{tab-item} macOS
:sync: macos

Open a **Terminal** and run:

```bash
# Lists GPU information
system_profiler SPDisplaysDataType
```

For OpenGL renderer info (requires XQuartz + mesa-utils):

```bash
glxinfo | grep "OpenGL renderer"
```

Or use the GUI: ** → About This Mac → More Info**.

**Driver requirements**  
- macOS provides system OpenGL drivers.  
- Any **macOS 12+** system with Intel or Apple Silicon GPUs supports OpenGL 4.1, which is fully backward-compatible with OpenGL 3.3.  

:::

:::{tab-item} Linux
:sync: linux

Open a **terminal** and run:

```bash
# Show OpenGL renderer and version
glxinfo | grep "OpenGL"

# Show GPU devices
lspci | grep -i vga
```

If you have an **NVIDIA GPU** (drivers installed):

```bash
nvidia-smi
```

**Driver requirements**  
- NVIDIA: driver **390+**  
- AMD: Mesa **18.0+**  
- Intel: Mesa **18.0+**  

:::

::::  

