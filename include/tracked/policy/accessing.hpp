#pragma once
#include <thread>

namespace policy {

  template<class Exception>
  struct must_accessed_by_single_thread {
    explicit must_accessed_by_single_thread() 
      : owner_thread_id{ std::this_thread::get_id() } { }
    ~must_accessed_by_single_thread() = default;

    void apply() const noexcept(noexcept(Exception::check(false)))
    {
      auto res = owner_thread_id == std::this_thread::get_id();
      Exception::check(res);
    }

   protected:
    must_accessed_by_single_thread(const std::thread::id& id) : owner_thread_id{ id } { }

   private:
    std::thread::id owner_thread_id;
  };

  const auto main_thread_id = std::this_thread::get_id();
  template<class Exception>
  struct must_accessed_by_main_thread : must_accessed_by_single_thread<Exception> {
    explicit must_accessed_by_main_thread() : must_accessed_by_single_thread<Exception>(main_thread_id) { }
  };


}