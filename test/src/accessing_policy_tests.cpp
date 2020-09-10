#include <gtest/gtest.h>

#include "tracked/policy/accessing.hpp"
#include "tracked/tracked.hpp"
#include <thread>

using namespace policy;

class TestingClass {
 public:
  TestingClass() { }

  ~TestingClass() { }
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

template<template<class> class Policy, bool should_throw_on_created_thread>
struct helper_t {
  using tracked_type = tracked_ptr<
      TestingClass,
      tracked_traits<TestingClass, dtl::default_deleter<TestingClass>, exceptions::throw_on_exception<test_exception>>,
      Policy>;

  explicit helper_t() : constructed_thread_id(std::this_thread::get_id()) { }

  tracked_type get_tracked_ptr_from_thread()
  {
    tracked_type ptr;
    std::thread th(&helper_t::thread_binder, this, std::ref(ptr));
    th.join();
    if (!ptr) {
      // this should not happen!
      std::terminate();
    }
    return ptr;
  }

 private:
  void thread_binder(tracked_type& ptr)
  {
    // this should not happen!
    if (constructed_thread_id == std::this_thread::get_id()) {
      FAIL() << "jumping thread is a main thread.";
    }
    ptr = make_tracked_ptr<TestingClass, exceptions::throw_on_exception<test_exception>, Policy>();
    if constexpr (should_throw_on_created_thread) {
      ASSERT_THROW(ptr->doubled(1), test_exception);
    }
    else {
      ASSERT_NO_THROW(ptr->doubled(1));
    }
  }
  std::thread::id constructed_thread_id;
};

TEST(AccessingPolicy, AccessedFromSingleThread)
{
  auto test_ptr =
      make_tracked_ptr<TestingClass, exceptions::throw_on_exception<test_exception>, must_accessed_by_single_thread>();

  auto test = [&test_ptr] {
    std::thread th([&] { 
      ASSERT_THROW(test_ptr->doubled(0), test_exception); });
    th.join();
  };

  ASSERT_NO_THROW(test_ptr->doubled(1));
  test();

  helper_t<must_accessed_by_single_thread, false> helper;
  auto ptr = std::move(helper.get_tracked_ptr_from_thread());
  ASSERT_THROW(ptr->doubled(1), test_exception);
}

TEST(AccessingPolicy, AccessedFromMainThread)
{
  auto test_ptr =
      make_tracked_ptr<TestingClass, exceptions::throw_on_exception<test_exception>, must_accessed_by_main_thread>();
  ASSERT_NO_THROW(test_ptr->doubled(1));
  auto test = [&test_ptr] {
    std::thread th([&] { ASSERT_THROW(test_ptr->doubled(0), test_exception); });
    th.join();
  };
  test();

  helper_t<must_accessed_by_main_thread, true> helper;
  auto ptr = std::move(helper.get_tracked_ptr_from_thread());
  ASSERT_NO_THROW(ptr->doubled(1));
}

TEST(AccessingPolicy, AccessedFromMainThread_NoThrow)
{
  auto test_ptr =
      make_tracked_ptr<TestingClass, exceptions::default_do_nothing, must_accessed_by_main_thread>();
  ASSERT_NO_THROW(test_ptr->doubled(1));
  auto test = [&test_ptr] {
    std::thread th([&] { ASSERT_NO_THROW(test_ptr->doubled(0)); });
    th.join();
  };
  test();
}
