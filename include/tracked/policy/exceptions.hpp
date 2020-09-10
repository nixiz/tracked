#pragma once
#include <cassert>
#include <exception>

namespace policy
{
  namespace exceptions
  {
    struct default_do_nothing
    {
      static void check(bool) noexcept { }  // do nothing.
    };

    struct assert_on_exception
    {
      static void check(bool res) noexcept
      {
        assert(res);
      }
    };

    template <class exc = std::exception>
    struct throw_on_exception : public exc
    {
      using exception_type = exc;
      static void check(bool res) noexcept(false)
      {
        if (!res) {
          throw exception_type();
        }
      }
    };
  }  // namespace exceptions
}  // namespace policy
