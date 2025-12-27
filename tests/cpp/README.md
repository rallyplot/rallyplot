# Building the tests

To build the tests, run `cmake` with the option `-DRALLYPLOT_BUILD_DEV=ON`. To build with this
option it is required to set the `Qt6_DIR` environment variable to the `Qt` installation used
for the build.

> [!NOTE]
> On `macOS`, you may need to also set the environment variable `CMAKE_PREFIX_PATH` to the
> root directory (`e.g. qt/macos`) of the Qt installation.

**For Example**

1) Install Qt if it is not installed on your system. e.g. Install `aqtinstall` through a Python package manager (e.g. `pip`, `conda`) and run `/distribution/download_qt.py`
2) Set the `Qt6_DIR` environment variable:

```shell
export Qt6_DIR=/home/youruser/Qt/6.8.2/gcc_64/lib/cmake/Qt6
```

(Note: on Windows, it is required to use `/` as path separator)

if on macOS, additionally set `CMAKE_PREFIX_PATH`:

```shell
export CMAKE_PREFIX_PATH=/Users/youruser/Qt/6.8.2/macos
```

3) Build with `-DRALLYPLOT_BUILD_DEV=ON` from the `rallyplot` root directory:

```shell
cmake -S . -B build -DRALLYPLOT_BUILD_DEV=ON
cmake --build build
```

You can run `testLib` from the build directory.


