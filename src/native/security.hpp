#pragma once

#include <string>

#include "native.hpp"

namespace native::security
{
	auto open_token(HANDLE process, DWORD desired_access) -> native::handle_t;

	auto set_privilege(HANDLE token_handle, const std::string& privilege, bool enabled) -> bool;
	auto set_privilege(const native::handle_t& token_handle, const std::string& privilege, bool enabled) -> bool;
}
