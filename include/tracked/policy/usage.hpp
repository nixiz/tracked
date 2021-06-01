#pragma once
#include "policy_utils.hpp"
#include "exceptions.hpp"

namespace policy {
  template<template <class> class derived_t, class Ex>
  struct simple_usage {
    using derived = derived_t<Ex>;

    void destruct() const noexcept(noexcept(Ex::check(false))) {
      static_cast<derived const&>(*this).check();
    }

    void dereference() const { 
      ++use_count;
    }

   protected:
    mutable std::size_t use_count{ 0 };
  };

  template<class Exception>
  struct must_be_used 
    : public simple_usage<must_be_used, Exception> {
    void check() const
    {
      Exception::check(this->use_count != 0);
    }
  };

  template<std::size_t Times>
  struct should_use_min_times {
    template<class Exception>
    struct type : public simple_usage<type, Exception> {
      void check() const
      {
        Exception::check(this->use_count >= Times);
      }
    };
  };

  template<std::size_t Times, bool err_on_exceed = true>
  struct should_use_max_times {
    template<class Exception>
    struct type {

      void destruct() const checked_noexcept { 
        if constexpr (!err_on_exceed) {
          Exception::check(use_count <= Times);
        }
      }
      void dereference() const checked_noexcept 
      { 
        ++use_count;
        if constexpr (err_on_exceed) {
          has_exceeded = use_count > Times;
          Exception::check(!has_exceeded);
        }
      }

     protected:
      mutable std::size_t use_count{ 0 };
      mutable bool has_exceeded{ false };
    };
  };
}  // namespace policy
