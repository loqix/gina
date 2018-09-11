#include <memory>

#include"capcom/payload.hpp"

int main()
{
	// capcom::resource::drop();

	auto test = std::make_unique<capcom::payload>();
	test->build(0xDEADBEEF);

	return 0;
}