#pragma once
#include <cassert>

namespace policy
{
  namespace exceptions
  {
    struct default_do_nothing
    {
      static void check(bool) { }  // do nothing.
    };

    struct assert_on_exception
    {
      static void check(bool res) { assert(res); }
    };

    struct throw_on_exception
    {
      static void check(bool res)
      {
        // TODO(oguzhank) : think how to make decorated exception throwing
        if (!res)
        {
          throw res;
        }
      }
    };
  }  // namespace exceptions
}  // namespace policy
