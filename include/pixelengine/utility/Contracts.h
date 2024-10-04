//
// Created by Nathaniel Rupprecht on 9/28/24.
//

#pragma once

#include <Lightning/Lightning.h>

namespace pixelengine::utility {

#define PIXEL_REQUIRE(condition, message) LL_REQUIRE(condition, message)

#define PIXEL_ASSERT(condition, message) LL_ASSERT(condition, message)

#define PIXEL_FAIL(message) LL_FAIL(message)

} // namespace pixelengine::utility