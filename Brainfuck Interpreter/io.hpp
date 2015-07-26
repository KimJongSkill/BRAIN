#pragma once

#include <cstdint>
#include <fstream>

void OutputByte(const std::uint8_t);

std::uint8_t InputByte();

std::streampos GetNextInstructionPointer();

void Jump(const std::streampos&);

void Jump(const char);

void Open(const char* const);

char GetNextInstruction();

bool FileIsGood();