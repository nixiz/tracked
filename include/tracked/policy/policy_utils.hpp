#pragma once

#define checked_noexcept \
  noexcept(noexcept(Exception::check(false)))
