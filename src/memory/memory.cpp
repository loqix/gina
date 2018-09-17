#include "memory.hpp"

#include <Windows.h>
#include <TlHelp32.h>

std::uintptr_t memory::base = 0;
std::size_t memory::size = 0;

std::uintptr_t memory::system_process_eprocess = 0;
std::uintptr_t memory::system_process_cr3 = 0;

auto memory::translate_address(std::uintptr_t address, std::uintptr_t cr3) -> std::uintptr_t
{
	if (cr3 == 0)
	{
		cr3 = system_process_cr3;
	}

	std::int16_t pml4 = (address >> 39) & 0x1FF;
	std::int16_t directory_pointer = (address >> 30) & 0x1FF;
	std::int16_t directory = (address >> 21) & 0x1FF;
	std::int16_t table = (address >> 12) & 0x1FF;

	auto pml4e = read_physical<std::uintptr_t>(cr3 + pml4 * sizeof(std::uintptr_t));
	if (pml4e == 0)
	{
		return 0;
	}

	auto pdpte = read_physical<std::uintptr_t>((pml4e & 0xFFFFFFFFFF000) + directory_pointer * sizeof(std::uintptr_t));
	if (pdpte == 0)
	{
		return 0;
	}

	if ((pdpte & (1 << 7)) != 0)
	{
		return (pdpte & 0xFFFFFC0000000) + (address & 0x3FFFFFFF);
	}

	auto pde = read_physical<std::uintptr_t>((pdpte & 0xFFFFFFFFFF000) + directory * sizeof(std::uintptr_t));
	if (pde == 0)
	{
		return 0;
	}

	if ((pde & (1 << 7)) != 0)
	{
		return (pde & 0xFFFFFFFE00000) + (address & 0x1FFFFF);
	}

	auto pte = read_physical<std::uintptr_t>((pde & 0xFFFFFFFFFF000) + table * sizeof(std::uintptr_t));
	if (pte == 0)
	{
		return 0;
	}

	return (pte & 0xFFFFFFFFFF000) + (address & 0xFFF);
}

auto memory::find_eprocess(std::uintptr_t process_id) -> std::uintptr_t
{
	auto list_head = system_process_eprocess + offset_eprocess_ActiveProcessLinks;

	auto link_current = list_head;
	auto link_last = read<std::uintptr_t>(list_head + offset_list_entry_Blink);

	do
	{
		auto current_eprocess = link_current - offset_eprocess_ActiveProcessLinks;
		auto current_eprocess_process_id = read<std::uintptr_t>(current_eprocess + offset_eprocess_UniqueProcessId);

		if (current_eprocess_process_id == process_id)
		{
			return current_eprocess;
		}

		link_current = memory::read<std::uintptr_t>(link_current);
	} while (link_current != link_last);

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
