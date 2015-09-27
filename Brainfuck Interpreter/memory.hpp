#pragma once

#include <iterator>
#include <list>
#include <array>

class Memory_iterator : public std::iterator<std::random_access_iterator_tag, char>
{
	friend class Memory;

public:
	bool operator==(const Memory_iterator&) const;
	bool operator!=(const Memory_iterator&) const;

	reference operator*() const;
	pointer operator->() const;

	Memory_iterator& operator++();
	Memory_iterator operator++(int);

	Memory_iterator& operator--();
	Memory_iterator operator--(int);

	difference_type operator-(const Memory_iterator&) const;

	Memory_iterator operator+(difference_type) const;
	Memory_iterator operator-(difference_type) const;

	bool operator<(const Memory_iterator&) const;
	bool operator>(const Memory_iterator&) const;

	bool operator<=(const Memory_iterator&) const;
	bool operator>=(const Memory_iterator&) const;

	Memory_iterator& operator+=(difference_type);
	Memory_iterator& operator-=(difference_type);

	reference operator[](difference_type) const;

private:
	void Advance(const std::ptrdiff_t);

	difference_type Index;
	pointer Pointer;
	Memory* Parent;
};

class Memory
{
	friend Memory_iterator;

public:
	typedef Memory_iterator iterator;
	typedef char cell_type;

	iterator begin() const;
	iterator end() const;

private:
	static struct Front_tag {} const Front;
	static struct Back_tag {} const Back;

	iterator::pointer RequestNewPage(const Front_tag);
	iterator::pointer RequestNewPage(const Back_tag);

	std::list<std::array<cell_type, 256>> Storage{ { 0 } };
	std::list<std::array<cell_type, 256>>::iterator Origin = Storage.begin();
	std::pair<std::ptrdiff_t, std::ptrdiff_t> Limits{ 0, 255 };
};
