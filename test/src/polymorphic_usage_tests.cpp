#include <gtest/gtest.h>

#include <thread>

#include "tracked/policy/all_policies.hpp"
#include "tracked/tracked.hpp"

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

  int doubled(int val)
  {
    return val * 2;
  }

 private:
};
using test_exception = std::exception;

template<class T, class Ex = exceptions::throw_on_exception<test_exception>>
using traits = tracked_traits<T, dtl::default_deleter<T>, Ex>;

TEST(PolymorphicUsageTest, PolicyChanging)
{
  auto first_policy_ptr = make_tracked_ptr<TestingClass, exceptions::throw_on_exception<test_exception>,
                                           must_accessed_by_single_thread, should_use_max_times<2>::type>();

  ASSERT_NO_THROW(first_policy_ptr->doubled(1));
  ASSERT_NO_THROW(first_policy_ptr->doubled(2));

  tracked_ptr<TestingClass, tracked_traits<TestingClass>> relaxed_ptr = std::move(first_policy_ptr);

  std::thread th([&] { ASSERT_NO_THROW(relaxed_ptr->doubled(3));
  });
  th.join();
  ASSERT_NO_THROW(relaxed_ptr->doubled(4));
  bool catched_true_exception = false;
  try {
    tracked_ptr<TestingClass, traits<TestingClass>, should_use_min_times<1>::type> third_ptr = std::move(relaxed_ptr);
  }
  catch (const test_exception&) {
    catched_true_exception = true;
  }
  catch (...) {
    catched_true_exception = false;
  }
  ASSERT_TRUE(catched_true_exception);
}
