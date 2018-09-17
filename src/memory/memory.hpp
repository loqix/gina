#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <string>

namespace memory
{
	constexpr std::uintptr_t offset_eprocess_DirectoryTableBase = 0x28;
	constexpr std::uintptr_t offset_eprocess_UniqueProcessId = 0x02E8;
	constexpr std::uintptr_t offset_eprocess_ActiveProcessLinks = 0x2F0;
	constexpr std::uintptr_t offset_eprocess_Peb = 0x3F8;
	constexpr std::uintptr_t offset_eprocess_VadRoot = 0x610;

	constexpr std::uintptr_t offset_list_entry_Flink = 0x0;
	constexpr std::uintptr_t offset_list_entry_Blink = 0x8;

	constexpr std::uintptr_t offset_peb_ImageBaseAddress = 0x10;

	extern std::uintptr_t base;
	extern std::size_t size;

	extern std::uintptr_t system_process_eprocess;
	extern std::uintptr_t system_process_cr3;

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

	auto translate_address(std::uintptr_t address, std::uintptr_t cr3 = 0) -> std::uintptr_t;

	inline auto read(std::uintptr_t address, std::uintptr_t buffer, std::size_t size, std::uintptr_t cr3 = 0) -> void
	{
		read_physical(translate_address(address, cr3), buffer, size);
	}

	template<typename T>
	inline auto read(std::uintptr_t address, std::uintptr_t cr3 = 0) -> T
	{
		T buffer;
		read(address, (std::uintptr_t)&buffer, sizeof(T), cr3);
		return buffer;
	}

	inline auto write(std::uintptr_t address, std::uintptr_t buffer, std::size_t size, std::uintptr_t cr3 = 0)
	{
		write_physical(translate_address(address, cr3), buffer, size);
	}

	template<typename T>
	inline auto write(std::uintptr_t address, const T& value, std::uintptr_t cr3 = 0)
	{
		write(address, (std::uintptr_t)&value, sizeof(T), cr3);
	}

	// scan range

	auto find_eprocess(std::uintptr_t process_id) -> std::uintptr_t;

	auto find_eprocess(const std::string& process_name) -> std::uintptr_t;
}
