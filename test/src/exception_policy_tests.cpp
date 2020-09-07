#include "tracked/tracked.hpp"
#include "tracked/policy/exceptions.hpp"
#include <gtest/gtest.h>

using namespace policy::exceptions;

TEST(ExceptionPolicy, Check_Do_Nothing)
{
  ASSERT_NO_THROW(default_do_nothing::check(true));
  ASSERT_NO_THROW(default_do_nothing::check(false));
}

TEST(ExceptionPolicy, Assert_On_Failure)
{
  ASSERT_DEATH(assert_on_exception::check(false), "");
  ASSERT_NO_FATAL_FAILURE(assert_on_exception::check(true));
}

TEST(ExceptionPolicy, Throw_On_Failure)
{
  using throw_std_exception = throw_on_exception<>;
  ASSERT_NO_THROW(throw_std_exception::check(true));
  ASSERT_THROW(throw_std_exception::check(false), throw_std_exception::exception_type);
  struct custom_exception {
    const char *what()
    {
      return "custom message";
    }
  };
  using throw_custom_exception = throw_on_exception<custom_exception>;
  ASSERT_THROW(throw_custom_exception::check(false), throw_custom_exception::exception_type);
}
