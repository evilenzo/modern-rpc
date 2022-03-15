#pragma once

#include "propagate_const.h"

#include <memory>

#define DECLARE_PIMPL() \
  struct impl;          \
  std::experimental::propagate_const<std::unique_ptr<impl>> m_pimpl;
