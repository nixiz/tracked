#pragma once
#include "tracked_detail.hpp"
#include "policy/exceptions.hpp"
#include <utility>

template<
  class Type, 
  class Deleter = dtl::default_delete<Type>, 
  class ExceptionPolicy = policy::exceptions::default_do_nothing>
struct tracked_traits {
  using deleter          = Deleter;
  using exception_policy = ExceptionPolicy;

 private:
  using type = Type;
};

using namespace std;
template<class Type, class Traits, template<class> class... Options>
class tracked_ptr final : public Options<typename Traits::exception_policy>... {
 public:
  using element_type     = Type;
  using deleter_type     = typename Traits::deleter;
  using pointer          = typename dtl::_Get_deleter_pointer_type<Type, remove_reference_t<deleter_type>>::type;
  using exception_policy = typename Traits::exception_policy;

  template<class _Dx2 = deleter_type, dtl::_Unique_ptr_enable_default_t<_Dx2> = 0>
  constexpr tracked_ptr() noexcept : my_pair(dtl::zero_then_variadic_args_t())
  {
  }

  template<class _Dx2 = deleter_type, dtl::_Unique_ptr_enable_default_t<_Dx2> = 0>
  constexpr tracked_ptr(nullptr_t) noexcept : my_pair(dtl::zero_then_variadic_args_t())
  {
  }

  template<class _Dx2 = deleter_type, dtl::_Unique_ptr_enable_default_t<_Dx2> = 0>
  explicit tracked_ptr(pointer _Ptr) noexcept : my_pair(dtl::zero_then_variadic_args_t(), _Ptr)
  {
  }

  template<class _Dx2 = deleter_type, enable_if_t<is_constructible_v<_Dx2, const _Dx2&>, int> = 0>
  tracked_ptr(pointer _Ptr, const deleter_type& _Dt) noexcept : my_pair(dtl::one_then_variadic_args_t(), _Dt, _Ptr)
  {
  }

  template<class _Dx2                                                                                  = deleter_type,
           enable_if_t<conjunction_v<negation<is_reference<_Dx2>>, is_constructible<_Dx2, _Dx2>>, int> = 0>
  tracked_ptr(pointer _Ptr, deleter_type&& _Dt) noexcept : my_pair(dtl::one_then_variadic_args_t(), move(_Dt), _Ptr)
  {
  }

  template<class _Dx2 = deleter_type,
           enable_if_t<conjunction_v<is_reference<_Dx2>, is_constructible<_Dx2, remove_reference_t<_Dx2>>>, int> = 0>
  tracked_ptr(pointer, remove_reference_t<deleter_type>&&) = delete;

  tracked_ptr(tracked_ptr&& _Right) noexcept
      : my_pair(dtl::one_then_variadic_args_t(), forward<deleter_type>(_Right.get_deleter()), _Right.release())
  {
  }

  template<
      class _Ty2, class _Tr2, template<class> class... _Op2,
      enable_if_t<conjunction_v<negation<is_array<_Ty2>>,
                                is_convertible<typename tracked_ptr<_Ty2, _Tr2, _Op2...>::pointer, pointer>,
                                conditional_t<is_reference_v<deleter_type>, is_same<typename _Tr2::deleter, deleter_type>,
                                              is_convertible<typename _Tr2::deleter, deleter_type>>>,
                  int> = 0>
  tracked_ptr(tracked_ptr<_Ty2, _Tr2, _Op2...>&& _Right) noexcept
      : my_pair(dtl::one_then_variadic_args_t(), forward<typename _Tr2::deleter>(_Right.get_deleter()), _Right.release())
  {
  }

  tracked_ptr& operator=(tracked_ptr&& _Right) noexcept
  {
    if (this != addressof(_Right)) {
      reset(_Right.release());
      my_pair.get_deleter() = forward<deleter_type>(_Right.my_pair.get_deleter());
    }
    return *this;
  }

  void swap(tracked_ptr& _Right) noexcept
  {
    std::swap(my_pair.pointer, _Right.my_pair.pointer);
    std::swap(my_pair.get_deleter(), _Right.my_pair.get_deleter());
  }

  ~tracked_ptr() noexcept
  {
    if (my_pair.pointer) {
      my_pair.get_deleter()(my_pair.pointer);
    }
  }

  [[nodiscard]] deleter_type& get_deleter() noexcept
  {
    return my_pair.get_deleter();
  }

  [[nodiscard]] const deleter_type& get_deleter() const noexcept
  {
    return my_pair.get_deleter();
  }

  [[nodiscard]] add_lvalue_reference_t<Type> operator*() const noexcept /* strengthened */
  {
    // call every options apply method.
    (Options<exception_policy>::apply(), ...);
    return *my_pair.pointer;
  }

  [[nodiscard]] pointer operator->() const noexcept
  {
    // call every options apply method.
    (Options<exception_policy>::apply(), ...);
    return my_pair.pointer;
  }

  [[nodiscard]] pointer get() const noexcept
  {
    // call every options apply method.
    (Options<exception_policy>::apply(), ...);
    return my_pair.pointer;
  }

  explicit operator bool() const noexcept
  {
    return static_cast<bool>(my_pair.pointer);
  }

  pointer release() noexcept
  {
    return std::exchange(my_pair.pointer, pointer());
  }

  void reset(pointer _Ptr = pointer()) noexcept
  {
    pointer old = std::exchange(my_pair.pointer, _Ptr);
    if (old) {
      my_pair.get_deleter()(old);
    }
  }

  tracked_ptr(const tracked_ptr&) = delete;
  tracked_ptr& operator=(const tracked_ptr&) = delete;

 private:
  template<class Type, class Trait, template<class> class... Options>
  friend class tracked_ptr;
  dtl::compressed_pair<deleter_type, pointer> my_pair;
};

template<class Type, class Deleter, class ExceptionPolicy, template<class> class... Options, class... Args>
[[nodiscard]] tracked_ptr<Type, tracked_traits<Type, Deleter, ExceptionPolicy>, Options...> make_tracked_ptr(Args&&... args)
{
  return tracked_ptr<Type, tracked_traits<Type, Deleter, ExceptionPolicy>, Options...>(
      new Type(std::forward<Args>(args)...));
}

template<class Type, class ExceptionPolicy, template<class> class... Options, class... Args>
[[nodiscard]] tracked_ptr<Type, tracked_traits<Type, ExceptionPolicy>, Options...> make_tracked_ptr(Args&&... args)
{
  return tracked_ptr<Type, tracked_traits<Type, ExceptionPolicy>, Options...>(new Type(std::forward<Args>(args)...));
}

template<class Type, template<class> class... Options, class... Args>
[[nodiscard]] tracked_ptr<Type, tracked_traits<Type>, Options...> make_tracked_ptr(Args&&... args)
{
  return tracked_ptr<Type, tracked_traits<Type>, Options...>(new Type(std::forward<Args>(args)...));
}
