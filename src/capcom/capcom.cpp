#include "capcom.hpp"

#include "driver.hpp"
#include "resource.hpp"

auto capcom::load() -> bool
{
	return resource::drop() && resource::load();
}

auto capcom::unload() -> bool
{
	return resource::unload() && resource::pickup();
}
