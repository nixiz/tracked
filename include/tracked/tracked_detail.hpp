#pragma once
#include <type_traits>

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

  // STRUCT TEMPLATE default_delete
  template <class _Ty>
  struct default_delete { // default deleter for unique_ptr
    constexpr default_delete() noexcept = default;

    template <class _Ty2, std::enable_if_t<std::is_convertible_v<_Ty2*, _Ty*>, int> = 0>
    default_delete(const default_delete<_Ty2>&) noexcept {}

    void operator()(_Ty* _Ptr) const noexcept /* strengthened */ { // delete a pointer
      static_assert(0 < sizeof(_Ty), "can't delete an incomplete type");
      delete _Ptr;
    }
  };

  template <class _Ty>
  struct default_delete<_Ty[]> { // default deleter for unique_ptr to array of unknown size
    constexpr default_delete() noexcept = default;

    template <class _Uty, std::enable_if_t<std::is_convertible_v<_Uty(*)[], _Ty(*)[]>, int> = 0>
    default_delete(const default_delete<_Uty[]>&) noexcept {}

    template <class _Uty, std::enable_if_t<std::is_convertible_v<_Uty(*)[], _Ty(*)[]>, int> = 0>
    void operator()(_Uty* _Ptr) const noexcept /* strengthened */ { // delete a pointer
      static_assert(0 < sizeof(_Uty), "can't delete an incomplete type");
      delete[] _Ptr;
    }
  };

  // STRUCT TEMPLATE _Get_deleter_pointer_type
  template <class _Ty, class _Dx_noref, class = void>
  struct _Get_deleter_pointer_type { // provide fallback
    using type = _Ty*;
  };

  template <class _Ty, class _Dx_noref>
  struct _Get_deleter_pointer_type<_Ty, _Dx_noref, std::void_t<typename _Dx_noref::pointer>> { // get _Dx_noref::pointer
    using type = typename _Dx_noref::pointer;
  };

  template <class _Dx2>
  using _Unique_ptr_enable_default_t =
    std::enable_if_t<std::conjunction_v<std::negation<std::is_pointer<_Dx2>>, std::is_default_constructible<_Dx2>>, int>;
}
