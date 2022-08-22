#pragma once

#include <vector>
#include <cstdint>



class Compressor
{
public:
	static std::vector<uint8_t> compress(const std::vector<uint8_t>& data, int64_t targetSizeHint);
	static std::vector<uint8_t> decompress(const std::vector<uint8_t>& data, int64_t targetSizeHint);
};


