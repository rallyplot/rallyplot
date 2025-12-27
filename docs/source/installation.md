(installation-page)=
# Installation

`rallyplot` runs with Python or C++ on Windows, macOS and most Linux distributions (see [below](supported-platforms) for details).

`rallyplot` requires a graphics card for fast plotting. It is written for OpenGL 3.3, which the 
majority of graphics cards from ~2010 onwards should support. See the [GPU compatibility section](gpu-section)
for details.

Once you have installed `rallyplot`, check out the [Get Started](get-started-tutorial) page.

## Install

::::::{tab-set}

:::::{tab-item} Python
:sync: python

`rallyplot` provides wheels for Windows, macOS 
and Linux. Install with:

```bash
pip install rallyplot
```

:::::

:::::{tab-item} C++
:sync: cpp

`rallyplot` can be built from source with CMake. Header files 
are located in `src/cpp/include`. 

All dependencies apart from Qt are vendored in the distribution. Header-only libraries 
are vendored in `src/vendored`; `FreeType` source code is vendored and automatically
built alongside `rallyplot` when building with CMake.

**Qt as a dependency**

CMake `find_package(Qt6 ...)` is used to locate Qt. 
If Qt is not found automatically on your system, 
the environment variable `Qt6_DIR` must be set. For example, on Linux:

```shell
export Qt6_DIR=/home/youruser/Qt/6.8.2/gcc_64/lib/cmake/Qt6
```

or Windows (note: forward slash must be used as file separators):

```
set Qt6_DIR=C:/Users/Jzimi/git-repos/rallyplot/distribution/qt/6.8.2/msvc2022_64/lib/cmake/Qt6
```

On macOS, you may need to *additionally* set `CMAKE_PREFIX_PATH` as below:

```shell
export CMAKE_PREFIX_PATH=/Users/youruser/Qt/6.8.2/macos
```

:::{dropdown} **CMake from the command line**

Build `rallyplot` with CMake from the command line:

```shell
cd rallyplot
mkdir build
cmake -S . -B build
cmake --build build --config Release
```

:::

:::{dropdown} **CMake subdirectory in CMakeLists.txt**

Build and link `rallyplot` during your application build with CMake:

```cmake

cmake_minimum_required(VERSION 3.16)

project(myProject LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add rallyplot to the build
add_subdirectory(
    "C:/Users/Jzimi/git-repos/rallyplot"

    # requires two arguments if folder is not a subdirectory of your project
    "${CMAKE_BINARY_DIR}/rallyplot-build"
)

add_executable(myProject main.cpp)

# Link to rallyplot
target_link_libraries(myProject PRIVATE rallyplot)

```

:::


:::::

::::::

(supported-platforms)=
## Supported Platforms

::::{tab-set}

:::{tab-item} Windows
:sync: windows

Windows 10 or newer (64-bit x64 and ARM64).

:::

:::{tab-item} macOS
:sync: macos

**macOS 11 Big Sur or newer** (Intel & Apple Silicon).

:::

:::{tab-item} Linux
:sync: linux

**Wayland** is recommended as a display protocol over x11. While `rallyplot`
will run on x11, it is slow and displays with some rendering issues.

**The below relates to the available Python builds**:

`rallyplot` is built on `manylinux_2_28`, working on

- Works on mainstream distros such as:
  - Ubuntu 20.04+, Debian 10/11/12  
  - RHEL / CentOS / AlmaLinux 8+  

MUSL-based systems (e.g., Alpine Linux) are not supported.

### Library dependencies

`rallyplot` does not vendor low-level system graphics libraries.  
While these should be included in your Linux distro, please check they are
installed if running into dependency errors:

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

Performance of the plotting library is directly related to your GPU capabilities. 

To check which GPU your system is running on, use the commands below:

::::{tab-set}

:::{tab-item} Windows
:sync: windows

Open Command Prompt or PowerShell and run:

```powershell
wmic path win32_VideoController get name
```

If you have an **NVIDIA GPU** and drivers installed:

```powershell
nvidia-smi
```

:::

:::{tab-item} macOS
:sync: macos

Open a terminal and run:

```bash
# Lists GPU information
system_profiler SPDisplaysDataType
```

For OpenGL renderer info (requires XQuartz + mesa-utils):

```bash
glxinfo | grep "OpenGL renderer"
```

Or use the GUI: ** → About This Mac → More Info**.

:::

:::{tab-item} Linux
:sync: linux

Open a terminal and run:

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

:::

::::  

