#include <memory>

#include "capcom/capcom.hpp"
#include "capcom/driver.hpp"
#include "memory/memory.hpp"
#include "memory/process.hpp"

int main(const int argc, char** argv)
{
	// - load capcom
	// - map physical memory
	// - unload capcom
	// remove VAD
	// clear MmUnloadedDrivers

	capcom::load();
	auto driver = std::make_unique<capcom::driver>();

	memory::size = driver->get_physical_memory_size();
	memory::base = driver->map_physical_memory(0, memory::size);

	memory::system_process_eprocess = driver->get_system_process_eprocess();
	memory::system_process_cr3 = driver->get_system_process_cr3();

	auto loaded_module_list = driver->get_loaded_module_list();

	driver.reset();
	capcom::unload();

	return 0;
}
