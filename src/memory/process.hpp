#pragma once

#include "memory.hpp"

namespace memory
{
	class process
	{
	private:
		std::uintptr_t eprocess = 0;
		std::uintptr_t directorytablebase = 0;

	public:
		process(std::uintptr_t eprocess) : eprocess(eprocess)
		{
			directorytablebase = memory::read<std::uintptr_t>(eprocess + memory::offset_eprocess_DirectoryTableBase);
		}

		inline auto read(std::uintptr_t address, std::uintptr_t buffer, std::size_t size) -> void
		{
			memory::read(address, buffer, size, directorytablebase);
		}

		template<typename T>
		inline auto read(std::uintptr_t address) -> T
		{
			T buffer;
			read(address, (std::uintptr_t)&buffer, sizeof(T));
			return buffer;
		}

		inline auto write(std::uintptr_t address, std::uintptr_t buffer, std::size_t size)
		{
			memory::write(address, buffer, size, directorytablebase);
		}

		template<typename T>
		inline auto write(std::uintptr_t address, const T& value)
		{
			write(address, (std::uintptr_t)&value, sizeof(T));
		}

		inline auto get_eprocess() -> std::uintptr_t
		{
			return eprocess;
		}

		inline auto get_directorytablebase() -> std::uintptr_t
		{
			return directorytablebase;
		}

		inline auto get_uniqueprocessid() -> std::uintptr_t
		{
			return memory::read<std::uintptr_t>(eprocess + memory::offset_eprocess_UniqueProcessId);
		}

		inline auto get_peb() -> std::uintptr_t
		{
			return memory::read<std::uintptr_t>(eprocess + memory::offset_eprocess_Peb);
		}

		inline auto get_vadroot() -> std::uintptr_t
		{
			return memory::read<std::uintptr_t>(eprocess + memory::offset_eprocess_VadRoot);
		}

		inline auto get_image_base() -> std::uintptr_t
		{
			return read<std::uintptr_t>(get_peb() + memory::offset_peb_ImageBaseAddress);
		}
	};
}
