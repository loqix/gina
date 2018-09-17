#include "registry.hpp"

auto native::registry::get_key(HKEY key, const std::string& sub_key) -> registry_handle_t
{
	HKEY result_key;
	DWORD disposition;

	if (RegCreateKeyExA(key, sub_key.c_str(), REG_OPTION_RESERVED, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &result_key, &disposition) != ERROR_SUCCESS)
	{
		return nullptr;
	}

	return registry_handle_t(result_key);
}

auto native::registry::get_key(const registry_handle_t& key, const std::string& sub_key) -> registry_handle_t
{
	return get_key(key.get(), sub_key);
}

auto native::registry::remove_key(HKEY key, const std::string& sub_key) -> bool
{
	return RegDeleteKeyA(key, sub_key.c_str()) == ERROR_SUCCESS;
}

auto native::registry::remove_key(const registry_handle_t& key, const std::string& sub_key) -> bool
{
	return remove_key(key.get(), sub_key);
}
