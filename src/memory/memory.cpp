#include "memory.hpp"

#include <Windows.h>
#include <TlHelp32.h>

std::uintptr_t memory::base = 0;
std::size_t memory::size = 0;

std::uintptr_t memory::system_process_eprocess = 0;
std::uintptr_t memory::system_process_pml4_table_base = 0;

auto memory::lookup_virtual_address(std::uintptr_t virtual_address, std::uintptr_t pml4_table_base) -> virtual_address_descriptor
{
	if (pml4_table_base == 0)
	{
		pml4_table_base = system_process_pml4_table_base;
	}

	virtual_address_descriptor descriptor = {
		virtual_address = virtual_address
	};

	std::int16_t pml4 = (virtual_address >> 39) & 0x1FF;
	std::int16_t directory_pointer = (virtual_address >> 30) & 0x1FF;
	std::int16_t directory = (virtual_address >> 21) & 0x1FF;
	std::int16_t table = (virtual_address >> 12) & 0x1FF;

	descriptor.pml4e = physical_reference<native::MMPTE>(pml4_table_base + pml4 * sizeof(native::MMPTE));

	if (!descriptor.pml4e->Hardware.Valid)
	{
		descriptor.physical_address = 0;
		return descriptor;
	}

	descriptor.pdpte = physical_reference<native::MMPTE>((descriptor.pml4e->Hardware.PageFrameNumber << 12) + directory_pointer * sizeof(native::MMPTE));

	if (!descriptor.pdpte->Hardware.Valid)
	{
		descriptor.physical_address = 0;
		return descriptor;
	}

	if (descriptor.pdpte->Hardware.LargePage)
	{
		descriptor.physical_address = (descriptor.pdpte->Long & 0xFFFFFC0000000) + (virtual_address & 0x3FFFFFFF);
		return descriptor;
	}

	descriptor.pde = physical_reference<native::MMPTE>((descriptor.pdpte->Hardware.PageFrameNumber << 12) + directory * sizeof(native::MMPTE));

	if (!descriptor.pde->Hardware.Valid)
	{
		descriptor.physical_address = 0;
		return descriptor;
	}

	if (descriptor.pde->Hardware.LargePage)
	{
		descriptor.physical_address = (descriptor.pde->Long & 0xFFFFFFFE00000) + (virtual_address & 0x1FFFFF);
		return descriptor;
	}

	descriptor.pte = physical_reference<native::MMPTE>((descriptor.pde->Hardware.PageFrameNumber << 12) + table * sizeof(native::MMPTE));

	if (!descriptor.pte->Hardware.Valid)
	{
		descriptor.physical_address = 0;
		return descriptor;
	}

	descriptor.physical_address = (descriptor.pte->Hardware.PageFrameNumber << 12) + (virtual_address & 0xFFF);
	return descriptor;
}

auto memory::find_eprocess(std::uintptr_t process_id) -> std::uintptr_t
{
	auto list_head_address = system_process_eprocess + native::offset_eprocess_ActiveProcessLinks;

	auto link_current_address = list_head_address;
	auto link_current = memory::read<native::LIST_ENTRY>(link_current_address);
	auto link_last = link_current.Blink;

	do
	{
		auto current_eprocess = link_current_address - native::offset_eprocess_ActiveProcessLinks;
		auto current_eprocess_process_id = read<std::uintptr_t>(current_eprocess + native::offset_eprocess_UniqueProcessId);

		if (current_eprocess_process_id == process_id)
		{
			return current_eprocess;
		}

		link_current_address = (std::uintptr_t)link_current.Flink;
		link_current = memory::read<native::LIST_ENTRY>(link_current_address);
	} while (link_current.Flink != link_last);

	return 0;
}

auto memory::find_eprocess(const std::string& process_name) -> std::uintptr_t
{
	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);

	PROCESSENTRY32 process_entry;
	process_entry.dwSize = sizeof(PROCESSENTRY32);

	if (Process32First(snapshot, &process_entry))
	{
		do
		{
			if (process_name == process_entry.szExeFile)
			{
				CloseHandle(snapshot);

				return find_eprocess(process_entry.th32ProcessID);
			}
		} while (Process32Next(snapshot, &process_entry));
	}

	CloseHandle(snapshot);

	return 0;
}
