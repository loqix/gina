#pragma once

#include <functional>
#include <Windows.h>

#include "../native.hpp"

namespace capcom
{
	class driver
	{
	private:
		HANDLE handle;

	public:
		driver();
		~driver();

		auto execute(std::function<void(MmGetSystemRoutineAddress_t)> function) -> void;
	};
}
