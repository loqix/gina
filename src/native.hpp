#pragma once

#include <Windows.h>
#include <Winternl.h>

typedef PVOID(NTAPI* MmGetSystemRoutineAddress_t)(PUNICODE_STRING);
