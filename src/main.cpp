#include <functional>
#include <iostream>
#include <memory>
#include <vector>

#include "capcom/capcom.hpp"
#include "capcom/driver.hpp"
#include "memory/memory.hpp"
#include "memory/process.hpp"

const std::vector<std::uint8_t> GetProcAddress_hook = {
	0x48, 0x89, 0x5C, 0x24, 0x08,                               // mov [rsp+8], rbx
	0x57,                                                       // push rdi
	0x48, 0x83, 0xEC, 0x20,                                     // sub rsp, 20h
	0x48, 0x8B, 0xF9,                                           // mov rdi, rcx
	0x48, 0x8B, 0xDA,                                           // mov rbx, rdx
	0x48, 0x8B, 0xCA,                                           // mov rcx, rdx
	0x48, 0xB8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // mov rax, 0h ; OutputDebugStringA
	0xFF, 0xD0,                                                 // call rax
	0x48, 0x8B, 0xD3,                                           // mov rdx, rbx
	0x48, 0x8B, 0xCF,                                           // mov rcx, rdi
	0x48, 0x8B, 0x5C, 0x24, 0x30,                               // mov rbx, [rsp+30h]
	0x48, 0x83, 0xC4, 0x20,                                     // add rsp, 20h
	0x5F,                                                       // pop rdi
	0x48, 0xB8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // mov rax, 0h ; GetProcAddress
	0xFF, 0xD0,                                                 // call rax
};

auto create_GetProcAddress_hook(std::uint64_t OutputDebugStringA, std::uint64_t GetProcAddress) -> std::vector<std::uint8_t>
{
	auto hook = GetProcAddress_hook;

	*(std::uint64_t*)&hook[0x15] = OutputDebugStringA;
	*(std::uint64_t*)&hook[0x31] = GetProcAddress;

	return hook;
}

int main(const int argc, char** argv)
{
	capcom::load();
	auto driver = std::make_unique<capcom::driver>();

	memory::size = driver->get_physical_memory_size();
	memory::base = driver->map_physical_memory(0, memory::size);

	memory::system_process_eprocess = driver->get_system_process_eprocess();
	memory::system_process_pml4_table_base = driver->get_system_process_pml4_table_base();

	auto gina = std::make_unique<memory::process>(memory::find_eprocess(GetCurrentProcessId()));

	auto hook = create_GetProcAddress_hook((std::uint64_t)OutputDebugStringA, (std::uint64_t)GetProcAddress);

	int a = *(int*)&GetProcAddress;
	gina->write((std::uintptr_t)&GetProcAddress, (std::uintptr_t)&hook.at(0), hook.size());

	GetProcAddress(0, "test");

	driver.reset();
	capcom::unload();

	return 0;
}
