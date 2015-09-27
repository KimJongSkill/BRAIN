#include "memory.hpp"

Memory::Front_tag const Memory::Front{};
Memory::Back_tag const Memory::Back{};

bool Memory_iterator::operator==(const Memory_iterator& Other) const
{
	return Pointer == Other.Pointer;
}

bool Memory_iterator::operator!=(const Memory_iterator& Other) const
{
	return !(*this == Other);
}

auto Memory_iterator::operator*() const -> reference
{
	return *Pointer;
}

auto Memory_iterator::operator->() const -> pointer
{
	return Pointer;
}

Memory_iterator& Memory_iterator::operator++()
{
	Advance(1);

	return *this;
}

Memory_iterator Memory_iterator::operator++(int)
{
	auto Temp = *this;
	++*this;
	return Temp;
}

Memory_iterator& Memory_iterator::operator--()
{
	Advance(-1);

	return *this;
}

Memory_iterator Memory_iterator::operator--(int)
{
	auto Temp = *this;
	--*this;
	return Temp;
}

auto Memory_iterator::operator-(const Memory_iterator& Other) const -> difference_type
{
	return Index - Other.Index;
}

Memory_iterator Memory_iterator::operator+(difference_type Delta) const
{
	Memory_iterator New(*this);

	New.Advance(Delta);

	return New;
}

Memory_iterator Memory_iterator::operator-(difference_type Delta) const
{
	return *this + -Delta;
}

bool Memory_iterator::operator<(const Memory_iterator& Other) const
{
	return Index < Other.Index;
}

bool Memory_iterator::operator>(const Memory_iterator& Other) const
{
	return Index > Other.Index;
}

bool Memory_iterator::operator<=(const Memory_iterator& Other) const
{
	return Index <= Other.Index;
}

bool Memory_iterator::operator>=(const Memory_iterator& Other) const
{
	return Index >= Other.Index;;
}

Memory_iterator& Memory_iterator::operator+=(difference_type Delta)
{
	Advance(Delta);

	return *this;
}

Memory_iterator & Memory_iterator::operator-=(difference_type Delta)
{
	return *this += -Delta;
}

auto Memory_iterator::operator[](difference_type Offset) const -> reference
{
	return *(*this + Offset);
}

void Memory_iterator::Advance(const std::ptrdiff_t Delta)
{
	constexpr std::ptrdiff_t Flag = ~std::ptrdiff_t(0xff); // 0xff...ff00 
	const std::ptrdiff_t NewIndex = Index + Delta;

	/*
	*	Check if the iterator has not moved to another page.
	*	Each page can hold 256 (0xff) elements, so the first byte
	*	of the Index stores the index within a page and
	*	the other bytes store the index of the page within
	*	the list.
	*/
	if ((Index & Flag) == (NewIndex & Flag))
		std::advance(Pointer, Delta);
	else if (NewIndex > Parent->Limits.second)
		Pointer = Parent->RequestNewPage(Memory::Back);
	else if (NewIndex < Parent->Limits.first)
		Pointer = Parent->RequestNewPage(Memory::Front);
	else
		Pointer = std::next(std::next(Parent->Origin, NewIndex >> 8)->data(), NewIndex & 0xff);

	Index = NewIndex;
}

auto Memory::begin() const -> iterator
{
	iterator New;
	New.Index = Limits.first;
	New.Parent = const_cast<Memory*>(this);
	New.Pointer = const_cast<char*>(&Storage.front().front());

	return New;
}

auto Memory::end() const -> iterator
{
	iterator New;
	New.Index = Limits.second;
	New.Parent = const_cast<Memory*>(this);
	New.Pointer = const_cast<char*>(&Storage.back().back());

	return New;
}

auto Memory::RequestNewPage(const Front_tag) -> iterator::pointer
{
	Storage.emplace_front();

	Limits.first -= Storage.front().size();

	return &Storage.front().back();
}

auto Memory::RequestNewPage(const Back_tag) -> iterator::pointer
{
	Storage.emplace_back();

	Limits.second += Storage.back().size();

	return &Storage.back().front();
}
