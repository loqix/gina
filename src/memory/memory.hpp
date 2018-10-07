#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <string>

#include "../native/native.hpp"

namespace memory
{
	extern std::uintptr_t base;
	extern std::size_t size;

	extern std::uintptr_t system_process_eprocess;
	extern std::uintptr_t system_process_pml4_table_base;

	struct virtual_address_descriptor
	{
		std::uintptr_t virtual_address;
		std::uintptr_t physical_address;
		native::MMPTE* pml4e;
		native::MMPTE* pdpte;
		native::MMPTE* pde;
		native::MMPTE* pte;
	};

	auto lookup_virtual_address(std::uintptr_t virtual_address, std::uintptr_t pml4_table_base = 0)->virtual_address_descriptor;

	inline auto translate_virtual_address(std::uintptr_t virtual_address, std::uintptr_t pml4_table_base = 0) -> std::uintptr_t
	{
		return lookup_virtual_address(virtual_address, pml4_table_base).physical_address;
	}

	template<typename T>
	inline auto physical_reference(std::uintptr_t address) -> T*
	{
		return (T*)(memory::base + address);
	}

	inline auto read_physical(std::uintptr_t address, std::uintptr_t buffer, std::size_t size) -> void
	{
		memcpy((void*)buffer, (void*)(base + address), size);
	}

	template<typename T>
	inline auto read_physical(std::uintptr_t address) -> T
	{
		T buffer;
		read_physical(address, (std::uintptr_t)&buffer, sizeof(T));
		return buffer;
	}

	inline auto write_physical(std::uintptr_t address, std::uintptr_t buffer, std::size_t size) -> void
	{
		memcpy((void*)(base + address), (void*)buffer, size);
	}

	template<typename T>
	inline auto write_physical(std::uintptr_t address, const T& value) -> void
	{
		write_physical(address, &value, sizeof(T));
	}

	inline auto read(std::uintptr_t address, std::uintptr_t buffer, std::size_t size, std::uintptr_t pml4_table_base = 0) -> void
	{
		read_physical(translate_virtual_address(address, pml4_table_base), buffer, size);
	}

	template<typename T>
	inline auto read(std::uintptr_t address, std::uintptr_t pml4_table_base = 0) -> T
	{
		T buffer;
		read(address, (std::uintptr_t)&buffer, sizeof(T), pml4_table_base);
		return buffer;
	}

	inline auto write(std::uintptr_t address, std::uintptr_t buffer, std::size_t size, std::uintptr_t pml4_table_base = 0)
	{
		write_physical(translate_virtual_address(address, pml4_table_base), buffer, size);
	}

	template<typename T>
	inline auto write(std::uintptr_t address, const T& value, std::uintptr_t pml4_table_base = 0)
	{
		write(address, (std::uintptr_t)&value, sizeof(T), pml4_table_base);
	}

	// scan range

	auto find_eprocess(std::uintptr_t process_id) -> std::uintptr_t;

	auto find_eprocess(const std::string& process_name) -> std::uintptr_t;
}
