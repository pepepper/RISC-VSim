#pragma once
#include "CPU.h"
#include "Memory.h"
#include <memory>
class machine
{
public:
	machine();
	~machine();
	std::unique_ptr<CPU> cpu;
	std::unique_ptr<Memory> memory;
};

