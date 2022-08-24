#pragma once

#include <vector>
#include <cstdint>
#include <iostream>

class Compressor
{
public:
	static std::vector<uint8_t> compress(const std::vector<uint8_t>& data);
	static std::vector<uint8_t> compress(const uint8_t* data, int64_t size);
	static std::vector<uint8_t> decompress(const std::vector<uint8_t>& data, int64_t targetSizeHint);

	// TODO:
	static void compress(std::istream& input, std::ostream& output, int64_t targetSizeHint);
	static void decompress(std::istream& input, std::ostream& output);
};
