#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <Windows.h>

namespace capcom
{
	const int payload_code_template_function_ptr_offset = 0x2;
	const unsigned char payload_code_template[] = {
		0x48, 0xB8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // movabs rax, function_ptr
		0xFF, 0xE0                                                  // jmp rax
	};

	struct payload
	{
		void* function_ptr;
		unsigned char code[sizeof(payload_code_template)];

		void build(std::uintptr_t function_ptr)
		{
			// fill this member to point to the actual code buffer (as required by capcom)
			this->function_ptr = this->code;

			// copy our code template into the executable page
			memcpy(this->code, payload_code_template, sizeof(payload_code_template));

			// fill in the function pointer that will be copied into rax, then jmp'd to
			*(std::uintptr_t*)(this->code + payload_code_template_function_ptr_offset) = function_ptr;
		}

		void* operator new(std::size_t size)
		{
			return VirtualAlloc(NULL, size, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
		}

		void operator delete(void* ptr, std::size_t size)
		{
			VirtualFree(ptr, size, MEM_RELEASE);
		}
	};
}