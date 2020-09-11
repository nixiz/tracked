[![Actions Status](https://github.com/nixiz/tracked/workflows/MacOS/badge.svg)](https://github.com/nixiz/tracked/actions)
[![Actions Status](https://github.com/nixiz/tracked/workflows/Windows/badge.svg)](https://github.com/nixiz/tracked/actions)
[![Actions Status](https://github.com/nixiz/tracked/workflows/Ubuntu/badge.svg)](https://github.com/nixiz/tracked/actions)
[![codecov](https://codecov.io/gh/nixiz/tracked/branch/master/graph/badge.svg)](https://codecov.io/gh/nixiz/tracked)
[![GitHub release (latest by date)](https://img.shields.io/github/v/release/nixiz/tracked)](https://github.com/nixiz/tracked/releases)

# Tracked - Runtime Object Analyze and Tracker

This is a header-only C++17 library enables to track object instances by various policies and gives user to exception control on policy break.

Tracked pointer is based on std::unique_ptr and extend it with policy control.

## Simple Usage

```cpp
#include "tracked/tracked.hpp"
#include "tracked/policy/all_policies.hpp"

class MyClass { /*...*/ };

// creating tracked_ptr<MyClass> with throw exception policy  
// and multiple dereferencing policies
auto ptr = make_tracked_ptr<MyClass,  
                            exceptions::throw_on_exception<std::exception>,  
                            must_accessed_by_single_thread,  
                            should_use_min_times<2>::type>();
// tracked pointer will throw exception either if 'ptr' accessed by any other
// thread than the thread pointer created or called more than "N" times
// declared in given policy.
ptr->foo();
auto bar = ptr->getBar();

// this call will make tracked pointer to throw exception
try
{
  ptr->foo();
} catch (const std::exception&) {
  ...
}
```

## Policy Change

```cpp
#include "tracked/tracked.hpp"
#include "tracked/policy/all_policies.hpp"

class MyClass { /*...*/ };

auto ptr = make_tracked_ptr<MyClass,
                            exceptions::throw_on_exception<std::exception>,
                            must_accessed_by_single_thread,
                            should_use_min_times<2>::type>();

/* ... */
// change policies by moving pointer to new one by explicitly declaring new type
tracked_ptr<MyClass, tracked_traits<MyClass>> untracked_ptr = std::move(ptr);
// or use auto with make_tracked_ptr<>() helper function
auto untracked_ptr = make_tracked_ptr<MyClass>(std::move(ptr));
```

## Policies

* __Error Handling__
  * ```default_do_nothing```
  * ```assert_on_exception```
  * ```throw_on_exception<exception_type = std::exception>```

* __Usage - Dereferencing__
  * ```must_be_used```
  * ```should_use_min_times<N_Times>::type```
  * ```should_use_max_times<N_Times>::type```

* __Accessing__
  * ```must_accessed_by_single_thread```
  * ```must_accessed_by_main_thread```

### Extending Policies

Any desired policy can be added by following these simple rules:

1. Policy class must be take exception policy by template argument
2. If any other template type want to be used in new policy should have underlying policy class
   that satisfies rule 1.  
   Look at ```should_use_min_times``` for implementation.

3. Destructors of policies must have noexcept() decleration explicitly.  
   __Note:__*if destructor does not have noexcept(false) and using with throw_expection policy can cause to terminate the application*

4. Policy classes must have ```void x_policy::apply() const``` function. This is main check function that tracked_ptr delegates through policies

## Building and Runnning Tests

To build the project, all you need to do call cmake with following parameters:

```bash
mkdir build/ && cd build/
cmake .. -DCMAKE_INSTALL_PREFIX=/absolute/path/to/custom/install/directory
cmake --build . --target install
ctest -C Release  # or `ctest -C Debug` or any other configuration you wish to test

# you can also run tests with the `-VV` flag for a more verbose output (i.e.
#GoogleTest output as well)
```

> ***Note:*** *The custom ``CMAKE_INSTALL_PREFIX`` can be omitted if you wish to
install in [the default install location](https://cmake.org/cmake/help/latest/module/GNUInstallDirs.html).*

By default, tracked library uses [Google Test](https://github.com/google/googletest/)
for unit testing. Unit testing can be disabled in the options, by setting the
`ENABLE_UNIT_TESTING` (from
[cmake/StandardSettings.cmake](cmake/StandardSettings.cmake)) to be false. To run
the tests, simply use CTest, from the build directory, passing the desire
configuration for which to run tests for. An example of this procedure is:

## Contributing

Please read [CONTRIBUTING.md](CONTRIBUTING.md) for details on our how you can
become a contributor and the process for submitting pull requests to us.

## Authors

* **Oğuzhan KATLI** - github:[@nixiz](https://github.com/nixiz) LinkedIn:[ogzhnktl](https://www.linkedin.com/in/ogzhnktl/)

This library is licensed under the [MIT License](https://opensource.org/licenses/MIT) - see the [LICENSE](LICENSE) file for details.
