#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace capcom::resource
{
	extern const unsigned char payload_key;
	extern const std::vector<unsigned char> payload_buffer;

	auto get_file_name() -> const std::filesystem::path;
	auto get_path() -> const std::filesystem::path;
	auto get_full_path() -> const std::filesystem::path;

	auto drop() -> void;
	auto pickup() -> void;
}