# CI Testing Strategy Overview

See `tests/python/README.md` for details on the testing stratergy itself.

We run (mainly regression tests):

1) On each PR (Linux only)
2) Once a week from PyPI (Linux only)
3) Build tests on merge to main

Unfortunately, it is not easy to test on macOS and Windows runners because they 
do not have reliable software-only rendering pipelines. Linux supports MESA and Xvfb, 
which allows us to render the library visually locally, save the framebuffer for 
regression tests, and compare against these results on Linux runners.

This workflow is not currently possible on macOS or Windows. We do include macOS 
and Windows regression data, but these tests must be run locally on machines 
with a GPU.

Build tests are only run on main because they are heavy, errors are difficult 
to interpret, and require significant setup across operating systems.
