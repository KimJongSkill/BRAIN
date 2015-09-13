#pragma once

#include <cstdint>
#include <string>

void OutputByte(const std::uint8_t);

std::uint8_t InputByte();

std::string Open(const std::string&);

void CreateOutputBuffer(const std::size_t);
