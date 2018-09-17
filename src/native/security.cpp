#include "security.hpp"

auto native::security::open_token(HANDLE process, DWORD desired_access) -> native::handle_t
{
	HANDLE handle;
	OpenProcessToken(process, desired_access, &handle);
	return native::handle_t(handle);
}

auto native::security::set_privilege(HANDLE token_handle, const std::string& privilege, bool enabled) -> bool
{
	TOKEN_PRIVILEGES tp;
	LUID luid;

	if (!LookupPrivilegeValueA(NULL, privilege.c_str(), &luid))
	{
		return false;
	}

	tp.PrivilegeCount = 1;
	tp.Privileges[0].Luid = luid;

	if (enabled)
	{
		tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
	}
	else
	{
		tp.Privileges[0].Attributes = 0;
	}

	if (!AdjustTokenPrivileges(token_handle, FALSE, &tp, sizeof(TOKEN_PRIVILEGES), NULL, NULL))
	{
		return false;
	}

	return GetLastError() != ERROR_NOT_ALL_ASSIGNED;
}

auto native::security::set_privilege(const native::handle_t& token_handle, const std::string& privilege, bool enabled) -> bool
{
	return set_privilege(token_handle.get(), privilege, enabled);
}
