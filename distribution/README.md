# Building a wheel

This directory contains tools for building a Python wheel. The full
release is built using `cibuildwheel`, which leverages the scripts in
this directory, but the `build_wheels.py` script can be used to quickly
build a single wheel on your current OS/environment for testing.

This process requires Python and `aqtinstall`.

To build a wheel locally, run `build_wheels.py`. This will:

1) Download Qt using `aqtinstall`.
2) Set the environment variables for the Qt installation.
3) Build the wheel.
4) Use wheel-repair tools to vendor all dependencies.
