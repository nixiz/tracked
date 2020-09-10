#include <gtest/gtest.h>

#include "tracked/policy/usage.hpp"
#include "tracked/tracked.hpp"

using namespace policy;

class TestingClass {
 public:
   TestingClass() {

  }

  ~TestingClass() {

  }
  // since tracked_ptr based on std::unique_ptr, 
  // we need to ensure that testing class will never copied.
  TestingClass(const TestingClass&) = delete;
  // but we can move object through tracked_ptr instances
  TestingClass(TestingClass&&) = default;

  int doubled(int val) {
    return val * 2;
  }

 private:
};

using test_exception = std::exception;
template<template<class> class... Policies>
struct test_t {
  void operator()(int num_of_use = 0) {
    auto test_ptr = make_tracked_ptr<TestingClass, exceptions::throw_on_exception<test_exception>, Policies...>();
    while (--num_of_use >= 0) {
      test_ptr->doubled(num_of_use);
    }
  }
};

TEST(UsagePolicy, MustBeUsed)
{
  test_t<must_be_used> test;
  EXPECT_THROW(test(0), test_exception);
  ASSERT_NO_THROW(test(1));
}

TEST(UsagePolicy, ShouldUseMinTimes)
{
  test_t<should_use_min_times<2>::type> test;
  EXPECT_THROW(test(0), test_exception);
  ASSERT_NO_THROW(test(2));
  ASSERT_NO_THROW(test(5));
}

TEST(UsagePolicy, ShouldUseMaxTimes)
{
  test_t<should_use_max_times<2>::type> test;
  ASSERT_NO_THROW(test(0));
  ASSERT_NO_THROW(test(2));
  EXPECT_THROW(test(3), test_exception);
}

