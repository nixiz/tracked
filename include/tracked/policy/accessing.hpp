#pragma once
#include "policy_utils.hpp"
#include <thread>

namespace policy {

  template<class Exception>
  struct must_accessed_by_single_thread {
    explicit must_accessed_by_single_thread() 
      : owner_thread_id{ std::this_thread::get_id() } { }
    ~must_accessed_by_single_thread() = default;

    void destruct() const checked_noexcept
    {
      //check();
    }

    void dereference() const checked_noexcept 
    { 
      check();
    }

   protected:
    must_accessed_by_single_thread(const std::thread::id& id) : owner_thread_id{ id } { }
    inline void check() const{
      auto res = owner_thread_id == std::this_thread::get_id();
      Exception::check(res);
    }

   private:
    std::thread::id owner_thread_id;
  };

  const auto main_thread_id = std::this_thread::get_id();
  template<class Exception>
  struct must_accessed_by_main_thread : must_accessed_by_single_thread<Exception> {
    explicit must_accessed_by_main_thread() 
      : must_accessed_by_single_thread<Exception>(main_thread_id) { }
  };

  template<class Exception>
  struct must_destruct_by_same_thread_constructed {
    explicit must_destruct_by_same_thread_constructed() 
      : owner_thread_id{ std::this_thread::get_id() } { } 
    ~must_destruct_by_same_thread_constructed() = default;

    void destruct() const checked_noexcept { 
      auto res = owner_thread_id == std::this_thread::get_id();
      Exception::check(res);
    }

    void dereference() const { }

   private:
    std::thread::id owner_thread_id;
  };


}