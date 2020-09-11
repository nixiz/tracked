#pragma once
#include <type_traits>
#include <utility>

namespace dtl {
  // tag type for value-initializing first, constructing second from remaining args
  struct zero_then_variadic_args_t {}; 

  // tag type for constructing first from one arg, constructing second from remaining args
  struct one_then_variadic_args_t {};

  template <
    class deleter_t, 
    class pointer_t, bool = std::is_empty_v<deleter_t> && !std::is_final_v<deleter_t> >
  struct compressed_pair final : public deleter_t
  {
    pointer_t pointer;
    using base_t = deleter_t;

    template <class... pointer_t2>
    constexpr explicit compressed_pair(zero_then_variadic_args_t, pointer_t2&&... val) noexcept /*TODO: add noexcept test*/
      : deleter_t(), pointer(std::forward<pointer_t2>(val)...) { }

    template <class deleter_t2, class... pointer_t2>
    constexpr compressed_pair(one_then_variadic_args_t, deleter_t2&& dltr, pointer_t2&&... val) noexcept /*TODO: add noexcept test*/
      : deleter_t(std::forward<deleter_t2>(dltr)), pointer(std::forward<pointer_t2>(val)...) { }

    constexpr deleter_t& get_deleter() noexcept {
      return *this;
    }

    constexpr const deleter_t& get_deleter() const noexcept {
      return *this;
    }
  };
  
  template <
    class deleter_t,
    class pointer_t>
  struct compressed_pair<deleter_t, pointer_t, false> final
  {
    pointer_t pointer;
    deleter_t deleter;

    template <class... pointer_t2>
    constexpr explicit compressed_pair(zero_then_variadic_args_t, pointer_t2&&... val) noexcept /*TODO: add noexcept test*/
      : deleter(), pointer(std::forward<pointer_t2>(val)...) { }

    template <class deleter_t2, class... pointer_t2>
    constexpr compressed_pair(one_then_variadic_args_t, deleter_t2&& dltr, pointer_t2&&... val) noexcept /*TODO: add noexcept test*/
      : deleter(std::forward<deleter_t2>(dltr)), pointer(std::forward<pointer_t2>(val)...) { }

    constexpr deleter_t& get_deleter() noexcept {
      return deleter;
    }

    constexpr const deleter_t& get_deleter() const noexcept {
      return deleter;
    }
  };

  // STRUCT TEMPLATE default_deleter
  template <class T>
  struct default_deleter { // default deleter for unique_ptr
    constexpr default_deleter() noexcept = default;

    template <class U, std::enable_if_t<std::is_convertible_v<U*, T*>, int> = 0>
    default_deleter(const default_deleter<U>&) noexcept {}

    void operator()(T* _Ptr) const noexcept /* strengthened */ { // delete a pointer
      static_assert(0 < sizeof(T), "can't delete an incomplete type");
      delete _Ptr;
    }
  };

  template <class T>
  struct default_deleter<T[]> { // default deleter for unique_ptr to array of unknown size
    constexpr default_deleter() noexcept = default;

    template <class U, std::enable_if_t<std::is_convertible_v<U(*)[], T(*)[]>, int> = 0>
    default_deleter(const default_deleter<U[]>&) noexcept {}

    template <class U, std::enable_if_t<std::is_convertible_v<U(*)[], T(*)[]>, int> = 0>
    void operator()(U* _Ptr) const noexcept /* strengthened */ { // delete a pointer
      static_assert(0 < sizeof(U), "can't delete an incomplete type");
      delete[] _Ptr;
    }
  };

  template <class T, class deleter_t, class = void>
  struct get_deleter_pointer_type {
    using type = T*;
  };

  template<class T, class deleter_t>
  struct get_deleter_pointer_type<T, deleter_t, std::void_t<typename deleter_t::pointer>> {
    using type = typename deleter_t::pointer;
  };

  template <class _Dx2>
  using tracked_ptr_enable_default_t =
    std::enable_if_t<std::conjunction_v<std::negation<std::is_pointer<_Dx2>>, std::is_default_constructible<_Dx2>>, int>;
}
