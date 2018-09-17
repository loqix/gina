#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <Windows.h>

#include "capcom.hpp"
#include "payload.hpp"

namespace capcom
{
	const unsigned long payload_code_function_ptr_offset = 0x2;
	const unsigned char payload_code_template[] =
	{
		0x48, 0xB8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // movabs rax, user_function_ptr
		0xFF, 0xE0                                                  // jmp rax
	};

	struct payload
	{
	private:
		void* code_ptr = code;
		unsigned char code[sizeof(payload_code_template)];

	public:
		auto operator new(std::size_t size) -> void*
		{
			return VirtualAlloc(NULL, size, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
		}

		auto operator delete(void* ptr, std::size_t size) -> void
		{
			VirtualFree(ptr, size, MEM_RELEASE);
		}

		auto assemble(std::uint64_t function_ptr)
		{
			memcpy(code, payload_code_template, sizeof(payload_code_template));
			*(std::uint64_t*)(code + payload_code_function_ptr_offset) = function_ptr;
		}

		auto get() const noexcept -> const void*
		{
			return &code_ptr;
		}
	};
}
