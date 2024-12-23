#pragma once
#include "./includes.h"

class FastPatch {
	static std::optional<std::vector<uint8_t>> stringToBytes(const std::string& byteString);
public:
	static std::optional<size_t> hexStringToAddress(const std::string& hexString);
	static void patch(size_t address, std::vector<std::uint8_t> bytes);
	static bool make(std::string addressString, std::string bytesString);
	static bool make(size_t address, std::string bytesString);
};