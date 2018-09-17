#pragma once

#include <memory>
#include <string>
#include <Windows.h>

#include "native.hpp"

namespace native::registry
{
	auto get_key(HKEY key, const std::string& sub_key) -> registry_handle_t;
	auto get_key(const registry_handle_t& key, const std::string& sub_key) -> registry_handle_t;

	auto remove_key(HKEY key, const std::string& sub_key) -> bool;
	auto remove_key(const registry_handle_t& key, const std::string& sub_key) -> bool;

	template<unsigned long key_type = REG_DWORD, typename value_type = unsigned char>
	inline auto set_value(HKEY key, const std::string& value_name, const value_type* value, unsigned long data) -> bool
	{
		return RegSetValueExA(key, value_name.c_str(), REG_OPTION_RESERVED, key_type, (unsigned char*)value, data) == ERROR_SUCCESS;
	}

	template<unsigned long key_type = REG_DWORD, typename value_type = unsigned char>
	inline auto set_value(const registry_handle_t& key, const std::string& value_name, const value_type* value, unsigned long data) -> bool
	{
		return set_value<key_type, value_type>(key.get(), value_name, value, data);
	}

	inline auto set_value(HKEY key, const std::string& value_name, const std::string& value) -> bool
	{
		return set_value<REG_SZ>(key, value_name.c_str(), value.c_str(), (unsigned long)value.size());
	}

	inline auto set_value(const registry_handle_t& key, const std::string& value_name, const std::string& value) -> bool
	{
		return set_value(key.get(), value_name, value);
	}

	template<unsigned long key_type = REG_DWORD, typename value_type = unsigned long>
	inline auto set_value(HKEY key, const std::string& value_name, const value_type value) -> bool
	{
		return set_value<key_type, value_type>(key, value_name, &value, sizeof(value_type));
	}

	template<unsigned long key_type = REG_DWORD, typename value_type = unsigned long>
	inline auto set_value(const registry_handle_t& key, const std::string& value_name, const value_type value) -> bool
	{
		return set_value<key_type, value_type>(key.get(), value_name, value);
	}
}
