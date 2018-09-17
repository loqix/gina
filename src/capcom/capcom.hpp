#pragma once

#include <mutex>

#include "driver.hpp"

namespace capcom
{
	auto load() -> bool;
	auto unload() -> bool;
}
