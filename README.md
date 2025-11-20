# `rallyplot`

For now, dll are all copied to `lib` and user must copy these
directly to their exe path.

Not sure how best to handle  this...


Should probably ask...


IMPORTANT pybind11 was rewritten to force UTC

# Building and distribution

First, download qt with `./donwload_qt.py`

For a quick build, set the environment variables to the qt libs e.g.:

```

```

Then build with cmake as normal

`mkdir build; cd build; cmake ..; cm`

This will create the build file (you can test with `testLib`) and `dist` that
contains the cpp and python libs and all dependencies (including Qt plugins).

For proper distribution through python, we use cibuildwheels. See `.github/workflows`

TODO
----
# Since 2015, MSVC (and I think GCC) ensure ABI compatability
# across versions. So in theory I should only need one build per OS
# (MSVC, use and document /md rather than /mt). For Linux, must document
# that the libstc++ is used as STL not libc++. For macOS should all be okay.
# Only for macOS build for AMD and ARM. For Windows use x86 only. For Linux AMD only.