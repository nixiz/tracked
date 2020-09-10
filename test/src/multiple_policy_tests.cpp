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

TEST(MultiplePolicyTest, MultiplePolicy)
{
  auto test_ptr = make_tracked_ptr<TestingClass, exceptions::throw_on_exception<test_exception>,
                                   must_accessed_by_single_thread, should_use_max_times<2>::type>();
  
  ASSERT_NO_THROW(test_ptr->doubled(1));

  std::thread th([&] {
    try {
      test_ptr->doubled(2);
    }
    catch (const test_exception&) {
      SUCCEED();
    }
    catch (...) {
      FAIL() << "failed at must_accessed_by_single_thread policy rule!";
    }
  });
  th.join();
  // Although this is 3rd call, nothing will be throw
  // due to second call was interrupted by other policy rule. 
  // At this point tracked_ptr still thinks that this is second call!
  ASSERT_NO_THROW(test_ptr->doubled(3));
  // Now tracked_ptr have 2 successfull derefence count, this call will be
  // 3rd and will throw exception in should_use_max_times<2> policy check
  ASSERT_THROW(test_ptr->doubled(4), test_exception);
}
