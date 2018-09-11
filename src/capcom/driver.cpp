#include "driver.hpp"

capcom::driver::driver()
{
	handle = CreateFile("\\\\.\\Htsysm72FB", FILE_ALL_ACCESS, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
}

capcom::driver::~driver()
{
	CloseHandle(handle);
}

auto capcom::driver::execute(std::function<void(MmGetSystemRoutineAddress_t)> function) -> void
{
	// DeviceIoControl(handle, 0xAA013044u, &payload->ptr_to_code, 8, &output_buffer, 4, &bytes_returned, NULL);
}