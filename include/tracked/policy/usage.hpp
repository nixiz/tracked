#pragma once
#include "exceptions.hpp"

namespace policy {
  template<class derived>
  struct simple_usage {
    ~simple_usage()
    {
      static_cast<derived *>(this)->check();
    }

    void apply() const
    {
      ++use_count;
    }

   protected:
    mutable std::size_t use_count{ 0 };
  };

  template<class Exception>
  struct must_be_used 
    : public simple_usage<must_be_used<Exception>> {
    void check()
    {
      Exception::check(this->use_count != 0);
    }
  };

  template<std::size_t Times>
  struct should_use_min_times {
    template<class Exception>
    struct type : public simple_usage<type<Exception>> {
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
      ~type()
      {
        Exception::check(use_count <= Times);
      }

      void apply() const
      {
        ++use_count;
        if constexpr (err_on_exceed) {
          Exception::check(use_count > Times);
        }
      }

     protected:
      mutable std::size_t use_count{ 0 };
    };
  };
}  // namespace policy
