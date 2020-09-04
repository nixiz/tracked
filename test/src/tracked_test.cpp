#include <iostream>
#include <cassert>
#include <type_traits>
#include <thread>
#include "tracked/tracked.hpp"
#include "tracked/policy/all_policies.hpp"
#include <gtest/gtest.h>

TEST(TmpAddTest, CheckValues)
{
  ASSERT_EQ((1 + 2), 3);
  EXPECT_TRUE(true);
}

template <int times, class Deleter = dtl::default_delete<int>>
using times_tracked = tracked_ptr<int, tracked_traits<int, Deleter, policy::exceptions::assert_on_exception>, policy::should_use_min_times<times>::template type>;

template <typename T, class Exception = policy::exceptions::default_do_nothing>
using main_thread_tracked = tracked_ptr<T, tracked_traits<T, Exception>, policy::must_accessed_by_main_thread>;

class MyClass
{
public:
  ~MyClass() = default;
  //MyClass() = default;
  MyClass(int i, double d) 
    : data(std::tie(i, d)) {}
  MyClass(const MyClass& oth) 
    : data(oth.data)
  { }

  int result() const {
    auto[i, d] = data;
    return i * d;
  }

private:
  std::tuple<int, double> data;
};

int func(MyClass& obj) {
  return obj.result() + 1;
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();

  using namespace policy;
  using namespace policy::exceptions;
  {
    tracked_ptr<MyClass, tracked_traits<MyClass>, must_accessed_by_single_thread, should_use_min_times<2>::type> obj;
    obj.reset(new MyClass(1, 2.));

    tracked_ptr<MyClass, tracked_traits<MyClass>> second_obj = std::move(obj);
    auto res = second_obj->result();
  }

  //{
  //  tracked<MyClass, exceptions::DefaultPolicy, must_accessed_by_single_thread, should_use_min_times<2>::type> obj{1,3.};
  //  auto result = obj->result();
  //  auto x = func(obj);

  //  std::thread t{ [&] {
  //    auto res = obj->result();
  //  } };
  //  t.join();
  //}
  //{
  //  tracked<MyClass, exceptions::DefaultPolicy, must_accessed_by_main_thread> obj{ 1,3. };
  //  main_thread_tracked<MyClass> obj2 = obj;
  //  auto result = obj->result();
  //  std::thread t{ [&] {
  //    auto res = obj->result();
  //  } };
  //  t.join();
  //}

}