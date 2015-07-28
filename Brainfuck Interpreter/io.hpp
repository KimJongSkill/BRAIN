#pragma once

#include <cstdint>
#include <fstream>

void OutputByte(const std::uint8_t);

std::uint8_t InputByte();

std::string::size_type GetNextInstructionPointer();

void Jump(std::string::size_type);

void JumpEndOfLoop();

void Open(const char* const);

char GetNextInstruction();

bool NotEOF();