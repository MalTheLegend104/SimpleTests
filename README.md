# Simple Tests

This is a very small cmake test suite, that makes writing tests easy and maintainable.

## Usage

1. Simply take the `CMakeLists.txt` from the base source directory, and put it in your test directory.
2. Modify the `ignored_files` list at the very top to exclude any directories or files from the tests.
3. Simply add `add_subdirectory(<test directory>)` to your top level CMake file. 
   - This will result in the test suite getting configured before continuing to the next instruction in the top level CMake.
   - It is advised to wrap this in an if statement that allows you to disable tests.
   - It is advised to do this after absolutely everything else in your top level CMake file.

> Documentation will come shortly.
> Look at the structure of the `test` folder for an example.

### Linking libraries

Often times you will need to link libraries into the tests for them to work. 
You can simply define `TEST_LINK_LIBRARIES` to be the CMake targets for the libraries before running the tests CMake.
Make sure all libraries are defined in the top level CMake before the `add_subdirectory` call.

### Include Directories

You can add include directories by simply defining `TEST_INCLUDE_DIRECTORIES` before calling `add_subdirectory`.

### Convenience Macros

There are three macros provided, `assert_test`, `assert_willfail`, and `assert_wf`.
Assert is generally a go to in a test environment, but in CTest, an assert failing generally makes the test that fails take ~100ms longer.
This doesn't seem like a big deal at first, but these delays really add up when you have dozens or hundreds of tests.
These three macros act exactly the same as regular assert, but interface better with CTest.

- `assert_test`
  - Simply your normal `assert`. If the statement is false, it returns 1. If it is true it does nothing.
- `assert_willfail` & `assert_wf`
  - The opposite of your regular assert. Returns 0 if the statement is true, does nothing otherwise.
  - This is meant to be used in "WillFail" tests that are designed to fail.

To use these macros you just simply have to add `#include <test_assertions>` to the test source file.

## Limitations

Currently, there are a few small limitations to this test suite:

- All tests have to be written using C++
  - You can still test C/ASM code using this suite, but the tests themselves must be C++.
- Aside from tests that intentionally fail, no other test attributes are supported.
- You have to reconfigure CMake every time you add a new a file.
  - This is just generally a limitation with CMake in general.