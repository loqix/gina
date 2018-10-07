#include <ntifs.h>

#define DEVICE_NAME L"\\Device\\Gina"
#define DEVICE_FILE_NAME L"\\\\.\\Gina"

NTSTATUS DriverEntry(_In_ PDRIVER_OBJECT DriverObject, _In_ PUNICODE_STRING RegistryPath);
VOID DriverUnload(_In_ PDRIVER_OBJECT DriverObject);
__kernel_entry NTSTATUS hooked_NtDeviceIoControlFile(HANDLE FileHandle, HANDLE Event, PIO_APC_ROUTINE ApcRoutine, PVOID ApcContext, PIO_STATUS_BLOCK IoStatusBlock, ULONG IoControlCode, PVOID InputBuffer, ULONG InputBufferLength, PVOID OutputBuffer, ULONG OutputBufferLength);

typedef NTSTATUS(__kernel_entry* NtDeviceIoControlFile_t)(HANDLE, HANDLE, PIO_APC_ROUTINE, PVOID, PIO_STATUS_BLOCK, ULONG, PVOID, ULONG, PVOID, ULONG);

NtDeviceIoControlFile_t original_NtDeviceIoControlFile;

typedef struct _SSDT
{
	LONG* pServiceTable;
	PVOID pCounterTable;
	ULONGLONG NumberOfServices;
	PCHAR pArgumentTable;
} SSDT, *PSSDT;

PSSDT ssdt;
ULONG old;

NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath)
{
	UNREFERENCED_PARAMETER(RegistryPath);

	PDEVICE_OBJECT device_object;

	NTSTATUS status;

	UNICODE_STRING device_name;
	RtlInitUnicodeString(&device_name, DEVICE_NAME);

	status = IoCreateDevice(DriverObject, 0, &device_name, FILE_DEVICE_UNKNOWN, 0, FALSE, &device_object);
	if (!NT_SUCCESS(status))
	{
		return status;
	}

	DriverObject->DriverUnload = DriverUnload;

	ULONGLONG address_KeStackAttachProcess = (ULONGLONG)&KeStackAttachProcess;
	ssdt = (PSSDT)(address_KeStackAttachProcess + 0x38EE40);

	old = ssdt->pServiceTable[7];

	DbgPrint("GINA %llX", (((LONG)((ULONG_PTR)hooked_NtDeviceIoControlFile - (ULONG_PTR)ssdt->pServiceTable)) << 4) | old & 0xF);

	return status;
}

VOID DriverUnload(PDRIVER_OBJECT DriverObject)
{
	// ssdt->pServiceTable[7] = old;

	IoDeleteDevice(DriverObject->DeviceObject);
}

__kernel_entry NTSTATUS hooked_NtDeviceIoControlFile(HANDLE FileHandle, HANDLE Event, PIO_APC_ROUTINE ApcRoutine, PVOID ApcContext, PIO_STATUS_BLOCK IoStatusBlock, ULONG IoControlCode, PVOID InputBuffer, ULONG InputBufferLength, PVOID OutputBuffer, ULONG OutputBufferLength)
{
	DbgPrint("[Gina] hooked_NtDeviceIoControlFile pid=%llX", PsGetCurrentProcessId());

	return NtDeviceIoControlFile(FileHandle, Event, ApcRoutine, ApcContext, IoStatusBlock, IoControlCode, InputBuffer, InputBufferLength, OutputBuffer, OutputBufferLength);
}
