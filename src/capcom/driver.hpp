#pragma once

#include <functional>
#include <intrin.h>
#include <mutex>
#include <Windows.h>

#include "../native/native.hpp"

#pragma intrinsic(_disable)
#pragma intrinsic(_enable)

namespace capcom
{
	static std::mutex dispatch_mutex;
	static std::function<void(native::MmGetSystemRoutineAddress_t)> dispatch_user_function;

	auto dispatch(native::MmGetSystemRoutineAddress_t MmGetSystemRoutineAddress) -> void;

	static std::unordered_map<std::string, std::uintptr_t> system_functions;

	class driver
	{
	private:
		native::handle_t handle;

	public:
		driver();

		auto execute(const std::function<void(native::MmGetSystemRoutineAddress_t)>& function) -> void;

		auto get_system_function(const std::string& function_name) -> std::uintptr_t;

		template<typename T>
		inline auto get_system_function(const std::string& function_name) -> T
		{
			return (T)get_system_function(function_name);
		}

		auto get_physical_memory_size() -> std::size_t;
        auto map_physical_memory(std::uintptr_t base, std::size_t size) -> std::uintptr_t;
		auto get_system_process_eprocess() -> std::uintptr_t;
		auto get_system_process_cr3() -> std::uintptr_t;
		auto get_loaded_module_list() -> std::uintptr_t;

	private:
		auto get_system_function_remote(const std::wstring& function_name) -> std::uintptr_t;

	};
}
