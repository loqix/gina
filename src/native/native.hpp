#pragma once

#include <string>
#include <Windows.h>
#include <Winternl.h>

#pragma comment(lib, "ntdll.lib")

namespace native
{
	typedef LARGE_INTEGER PHYSICAL_ADDRESS, *PPHYSICAL_ADDRESS;

	typedef struct _PHYSICAL_MEMORY_RANGE
	{
		PHYSICAL_ADDRESS BaseAddress;
		LARGE_INTEGER NumberOfBytes;
	} PHYSICAL_MEMORY_RANGE, *PPHYSICAL_MEMORY_RANGE;

	typedef enum _SECTION_INHERIT
	{
		ViewShare = 1,
		ViewUnmap = 2
	} SECTION_INHERIT;

	typedef PVOID(NTAPI* MmGetSystemRoutineAddress_t)(PUNICODE_STRING);
	typedef PPHYSICAL_MEMORY_RANGE(NTAPI* MmGetPhysicalMemoryRanges_t)();
	typedef NTSTATUS(NTAPI* ZwOpenSection_t)(PHANDLE, ACCESS_MASK, POBJECT_ATTRIBUTES);
	typedef NTSTATUS(NTAPI* ZwClose_t)(HANDLE);

	extern "C" NTSTATUS NTAPI ZwLoadDriver(PUNICODE_STRING);
	extern "C" NTSTATUS NTAPI ZwUnloadDriver(PUNICODE_STRING);
	extern "C" NTSTATUS NTAPI NtMapViewOfSection(HANDLE, HANDLE, PVOID, ULONG_PTR, SIZE_T, PLARGE_INTEGER, PSIZE_T, SECTION_INHERIT, ULONG, ULONG);

	struct handle_closer
	{
		void operator()(HANDLE handle)
		{
			CloseHandle(handle);
		}
	};

	struct registry_handle_closer
	{
		void operator()(HKEY handle)
		{
			RegCloseKey(handle);
		}
	};

	using handle_t = std::unique_ptr<std::remove_pointer<HANDLE>::type, handle_closer>;
	using registry_handle_t = std::unique_ptr<std::remove_pointer<HKEY>::type, registry_handle_closer>;
}
