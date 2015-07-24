#pragma once

#include <cstdint>
#include <fstream>

void OutputByte(std::uint8_t);

std::uint8_t InputByte();

std::streampos GetNextInstructionPointer();

void Jump(std::streampos);

void Jump(char);

void Open(char*);

char GetNextInstruction();

bool FileIsGood();