#include <functional>
#include <iostream>
#include <memory>
#include <vector>

#include "capcom/capcom.hpp"
#include "capcom/driver.hpp"
#include "memory/memory.hpp"
#include "memory/process.hpp"

auto find_physical_memory_vads(std::uintptr_t root_vad) -> std::vector<std::uintptr_t>
{
	std::vector<std::uintptr_t> physical_memory_vads;

	std::function<void(std::uintptr_t)> check_vad = [&](std::uintptr_t vad) -> void
	{
		auto flags = memory::read<native::MMVAD_FLAGS>(vad + native::offset_mmvad_VadFlags);

		if (flags.VadType == native::MI_VAD_TYPE::VadDevicePhysicalMemory)
		{
			physical_memory_vads.push_back(vad);
		}

		auto tree = memory::read<native::RTL_BALANCED_NODE>(vad + native::offset_mmvad_VadNode);

		if (tree.Left != 0)
		{
			check_vad((std::uintptr_t)tree.Left);
		}

		if (tree.Right != 0)
		{
			check_vad((std::uintptr_t)tree.Right);
		}
	};

	check_vad(root_vad);

	return physical_memory_vads;
}

unsigned char pid_hook[] = {
	0x65, 0x48, 0x8B, 0x04, 0x25, 0x30, 0x00, 0x00, 0x00,       // mov rax,qword ptr gs:[30]
	0x8B, 0x40, 0x40,                                           // mov eax,dword ptr ds:[rax+40]
	0x3D, 0x00, 0x00, 0x00, 0x00,                               // cmp eax,00000000
	0x74, 0x10,                                                 // je +0x10
	0x4C, 0x8B, 0x04, 0x24,                                     // mov r8,qword ptr ss:[rsp]
	0x48, 0xA1, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // mov rax,[0000000000000000]
	0xFF, 0xE0,                                                 // jmp rax
	0x48, 0xB8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // mov rax,0000000000000000
	0xFF, 0xE0,                                                 // jmp rax
};

void set_pid_hook(std::uint32_t pid, std::uint64_t original, std::uint64_t hooked)
{
	*(std::uint32_t*)(&pid_hook[13]) = pid;
	*(std::uint64_t*)(&pid_hook[25]) = original;
	*(std::uint64_t*)(&pid_hook[37]) = hooked;
}

std::uintptr_t pool = 0xffffd20b346ed000;

#define POOL_COUNTER_OFFSET 0xff
#define POOL_LOG_START 0x100
#define POOL_LOG_SIZE 24


__int64 __stdcall hooked_GetProcAddress(HMODULE module, const char* string)
{
	__int64 pool = 0xffffd20b346ed000;
    __int64* pool_counter = (__int64*)(pool + POOL_COUNTER_OFFSET);
    (*pool_counter)++;
    char* pool_log_base = (char*)(pool + POOL_LOG_START + *pool_counter * POOL_LOG_SIZE);

    for (int i = 0; i < POOL_LOG_SIZE && string[i] != 0; i++)
    {
        pool_log_base[i] = string[i];
    }

	return (__int64)((decltype(&GetProcAddress))0x00007FFB8AE93063)(module, string);
}

int hooked_GetProcAddress_end()
{
	return 0;
}

__int64 __stdcall hooked_GetProcAddress2(__int64 module, const char* string)
{
	char stringa[] = { '[', 'G', 'i', 'n', 'a', ']', ' ',  };
}


int main(const int argc, char** argv)
{
	// - load capcom
	// - map physical memory
	// - unload capcom
	// remove VAD
	// clear MmUnloadedDrivers

	OutputDebugStringW(L"[Gina] abc");

	return 0;

	capcom::load();
	auto driver = std::make_unique<capcom::driver>();

	memory::size = driver->get_physical_memory_size();
	memory::base = driver->map_physical_memory(0, memory::size);

	memory::system_process_eprocess = driver->get_system_process_eprocess();
	memory::system_process_pml4_table_base = driver->get_system_process_pml4_table_base();

	auto ExAllocatePool = driver->get_system_function<native::ExAllocatePool_t>("ExAllocatePool");
	auto ExFreePool = driver->get_system_function<native::ExFreePool_t>("ExFreePool");

	// driver->execute([&](auto)
	// {
	// 	pool = (std::uintptr_t)ExAllocatePool(native::POOL_TYPE::NonPagedPool, 0x1000);
	// });

	auto gina = std::make_unique<memory::process>(memory::find_eprocess(GetCurrentProcessId()));

	auto physical_memory_vads = find_physical_memory_vads(gina->get_vadroot());
	for (auto& vad : physical_memory_vads)
	{
		auto flags = memory::read<native::MMVAD_FLAGS>(vad + native::offset_mmvad_VadFlags);
		flags.VadType = native::MI_VAD_TYPE::VadNone;
		memory::write<native::MMVAD_FLAGS>(vad + native::offset_mmvad_VadFlags, flags);
	}

	driver.reset();
	capcom::unload();

	auto arma3_pid = 0x0;

	std::cout << "arma 3 ready?";
	std::cin >> arma3_pid;

	auto arma3 = std::make_unique<memory::process>(memory::find_eprocess(arma3_pid));

	auto descriptor = memory::lookup_virtual_address(pool, arma3->get_directorytablebase());

	descriptor.pml4e->Hardware.Owner = 1;
	descriptor.pdpte->Hardware.Owner = 1;
	descriptor.pde->Hardware.Owner = 1;
	descriptor.pte->Hardware.Owner = 1;

	memset((void*)pool, 0, 0x1000);
	memcpy((void*)pool, (void*)&hooked_GetProcAddress, (std::uintptr_t)&hooked_GetProcAddress_end - (std::uintptr_t)&hooked_GetProcAddress);

	set_pid_hook(arma3_pid, 0x00007FFB8AE95190, pool);

	// auto volatile value = *(std::uintptr_t*)0x00007FFB8AE93050;
	arma3->write((std::uintptr_t)0x00007FFB8AE93050, (std::uintptr_t)&pid_hook, sizeof(pid_hook));

	// auto hooked = (decltype(&GetProcAddress))0x00007FFB8AE93050;
	// auto result = hooked(GetModuleHandle("kernel32.dll"), "GetCurrentProcessId");

	auto pool_buffer = malloc(0x1000);
	memcpy(pool_buffer, (void*)pool, 0x1000);

	std::cout << std::hex << pool_buffer;

	for (auto& vad : physical_memory_vads)
	{
		auto flags = memory::read<native::MMVAD_FLAGS>(vad + native::offset_mmvad_VadFlags);
		flags.VadType = native::MI_VAD_TYPE::VadDevicePhysicalMemory;
		memory::write<native::MMVAD_FLAGS>(vad + native::offset_mmvad_VadFlags, flags);
	}

	// system("pause");

	// capcom::load();
	// driver = std::make_unique<capcom::driver>();

	// driver->execute([&](auto)
	// {
	// 	ExFreePool((PVOID)pool);
	// });

	// driver.reset();
	// capcom::unload();

	return 0;
}
