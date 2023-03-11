#pragma once

#include <algorithm>
#include <vector>
#include <functional>
#include <fstream>
#include <iostream>
#include "vulkan/vulkan.hpp"

namespace toy3d {
	std::vector<char> ReadWholeFile(std::string&& filename);
}
