#pragma once

#include <string>
#include <Windows.h>
#include <Winternl.h>

#pragma comment(lib, "ntdll.lib")

namespace native
{
	// offsets
	constexpr std::uintptr_t offset_eprocess_DirectoryTableBase = 0x28;
	constexpr std::uintptr_t offset_eprocess_UniqueProcessId = 0x02E8;
	constexpr std::uintptr_t offset_eprocess_ActiveProcessLinks = 0x2F0;
	constexpr std::uintptr_t offset_eprocess_Peb = 0x3F8;
	constexpr std::uintptr_t offset_eprocess_VadRoot = 0x620;

	constexpr std::uintptr_t offset_mmvad_VadNode = 0x0;
	constexpr std::uintptr_t offset_mmvad_VadFlags = 0x30;

	constexpr std::uintptr_t offset_peb_ImageBaseAddress = 0x10;

	// typedefs
	typedef LARGE_INTEGER PHYSICAL_ADDRESS, *PPHYSICAL_ADDRESS;

	// windows structs
	typedef struct _LIST_ENTRY {
		_LIST_ENTRY* Flink;
		_LIST_ENTRY* Blink;
	} LIST_ENTRY, *PLIST_ENTRY;

	typedef struct _MMPTE_HARDWARE
	{
		ULONGLONG Valid : 1;
		ULONGLONG Writable : 1;
		ULONGLONG Owner : 1;
		ULONGLONG WriteThrough : 1;
		ULONGLONG CacheDisable : 1;
		ULONGLONG Accessed : 1;
		ULONGLONG Dirty : 1;
		ULONGLONG LargePage : 1;
		ULONGLONG Global : 1;
		ULONGLONG CopyOnWrite : 1;
		ULONGLONG Prototype : 1;
		ULONGLONG Write : 1;
		ULONGLONG PageFrameNumber : 40;
		ULONGLONG Reserved : 11;
		ULONGLONG NoExecute : 1;
	} MMPTE_HARDWARE;

	typedef struct _MMPTE_PROTOTYPE
	{
		ULONGLONG Valid : 1;
		ULONGLONG Unused0: 7;
		ULONGLONG ReadOnly : 1;
		ULONGLONG Unused1: 1;
		ULONGLONG Prototype : 1;
		ULONGLONG Protection : 5;
		ULONGLONG ProtoAddress: 48;
	} MMPTE_PROTOTYPE;

	typedef union _MMPTE
	{
		ULONG_PTR Long;
		MMPTE_HARDWARE Hardware;
		MMPTE_PROTOTYPE Prototype;
		// MMPTE_SOFTWARE Software;
	} MMPTE;

	typedef struct _MMVAD_FLAGS
	{
		ULONG VadType : 3;
		ULONG Protection : 5;
		ULONG PreferredNode : 6;
		ULONG NoChange : 1;
		ULONG PrivateMemory : 1;
		ULONG Teb : 1;
		ULONG PrivateFixup : 1;
		ULONG ManySubsections : 1;
		ULONG Spare : 12;
		ULONG DeleteInProgress : 1;
	} MMVAD_FLAGS, *PMMVAD_FLAGS;

	typedef struct _RTL_BALANCED_NODE
	{
		union
		{
			_RTL_BALANCED_NODE* Children[2];
			struct
			{
				_RTL_BALANCED_NODE* Left;
				_RTL_BALANCED_NODE* Right;
			};
		};
		union
		{
			UCHAR Red : 1;
			UCHAR Balance : 2;
			ULONG_PTR ParentValue;
		};
	} RTL_BALANCED_NODE, *PRTL_BALANCED_NODE;

	typedef struct _PHYSICAL_MEMORY_RANGE
	{
		PHYSICAL_ADDRESS BaseAddress;
		LARGE_INTEGER NumberOfBytes;
	} PHYSICAL_MEMORY_RANGE, *PPHYSICAL_MEMORY_RANGE;

	// enums
	typedef enum _MI_VAD_TYPE
	{
		VadNone,
		VadDevicePhysicalMemory,
		VadImageMap,
		VadAwe,
		VadWriteWatch,
		VadLargePages,
		VadRotatePhysical,
		VadLargePageSection
	} MI_VAD_TYPE, *PMI_VAD_TYPE;

	typedef enum _POOL_TYPE
	{
		NonPagedPool,
		NonPagedPoolExecute,
		PagedPool,
		NonPagedPoolMustSucceed,
		DontUseThisType,
		NonPagedPoolCacheAligned,
		PagedPoolCacheAligned,
		NonPagedPoolCacheAlignedMustS,
		MaxPoolType,
		NonPagedPoolBase,
		NonPagedPoolBaseMustSucceed,
		NonPagedPoolBaseCacheAligned,
		NonPagedPoolBaseCacheAlignedMustS,
		NonPagedPoolSession,
		PagedPoolSession,
		NonPagedPoolMustSucceedSession,
		DontUseThisTypeSession,
		NonPagedPoolCacheAlignedSession,
		PagedPoolCacheAlignedSession,
		NonPagedPoolCacheAlignedMustSSession,
		NonPagedPoolNx,
		NonPagedPoolNxCacheAligned,
		NonPagedPoolSessionNx
	} POOL_TYPE;

	typedef enum _SECTION_INHERIT
	{
		ViewShare = 1,
		ViewUnmap = 2
	} SECTION_INHERIT;

	// functions
	typedef PVOID(NTAPI* MmGetSystemRoutineAddress_t)(PUNICODE_STRING);
	typedef PPHYSICAL_MEMORY_RANGE(NTAPI* MmGetPhysicalMemoryRanges_t)();
	typedef NTSTATUS(NTAPI* ZwOpenSection_t)(PHANDLE, ACCESS_MASK, POBJECT_ATTRIBUTES);
	typedef NTSTATUS(NTAPI* ZwClose_t)(HANDLE);
	typedef PVOID(NTAPI* ExAllocatePool_t)(POOL_TYPE, SIZE_T);
	typedef VOID(NTAPI* ExFreePool_t)(PVOID);
	typedef PVOID(NTAPI* ExAllocatePoolWithTag_t)(POOL_TYPE, SIZE_T, ULONG);
	typedef VOID(NTAPI* ExFreePoolWithTag_t)(PVOID, ULONG);

	// imports
	extern "C" NTSTATUS NTAPI ZwLoadDriver(PUNICODE_STRING);
	extern "C" NTSTATUS NTAPI ZwUnloadDriver(PUNICODE_STRING);
	extern "C" NTSTATUS NTAPI NtMapViewOfSection(HANDLE, HANDLE, PVOID, ULONG_PTR, SIZE_T, PLARGE_INTEGER, PSIZE_T, SECTION_INHERIT, ULONG, ULONG);
	extern "C" NTSTATUS NTAPI NtUnmapViewOfSection(HANDLE, PVOID);

	// handle
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
