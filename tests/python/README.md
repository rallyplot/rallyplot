# `rallyplot` tests

Because it is tricky to test explicitly that the values displayed by the GPU are correct, the approach
taken is first to verify visually the plots are correct, then store these framebuffers for regression tests.
Unfortunately, this is easiest to handle without using `pytest`, so currently all tests are simple python scripts.

In the future, tests should be extended to explicitly test the code on the C++ side. This will give better diagnostics.
However, these regression tests are a quick and effective way to ensure the final product works as expected, and
it will always be necessary to test against visually-confirmed data.

`test_regression.py` is the main set of tests. It can be run with the `"generate"` command line argument
to store the framebuffers from the tests. "test" will test against the stored framebuffers. "check" is used
for visual inspection, ensuring the plots display as expected according to the printed checklist.

All other tests are quick tests to ensure other auxiliary parts of the codebase are working.