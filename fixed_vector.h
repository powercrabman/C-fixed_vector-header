#pragma once

/* fixed vector */

#include <initializer_list>
#include <cassert>
using namespace std;

template <typename Ty, size_t FIXED_SIZE>
class fv_iterator {
public:
	using iterator_category = std::random_access_iterator_tag;
	using value_type = Ty;
	using reference = Ty&;
	using pointer = Ty*;
	using difference_type = std::ptrdiff_t;

public:
	fv_iterator(Ty* p = nullptr) : _ptr(p) {}
	~fv_iterator() = default;
	fv_iterator(const fv_iterator&) = default;
	fv_iterator& operator=(const fv_iterator&) = default;

	bool operator==(const fv_iterator& other) const { return other._ptr == _ptr; }
	bool operator!=(const fv_iterator& other) const { return other._ptr != _ptr; }
	bool operator<(const fv_iterator& other) const { return _ptr < other._ptr; }
	bool operator>(const fv_iterator& other) const { return _ptr > other._ptr; }
	bool operator<=(const fv_iterator& other) const { return _ptr <= other._ptr; }
	bool operator>=(const fv_iterator& other) const { return _ptr >= other._ptr; }
	reference operator*() const { return *_ptr; }
	pointer operator->() const { return _ptr; }
	fv_iterator& operator++() {
		++_ptr;
		return *this;
	}
	fv_iterator operator++(int) {
		fv_iterator temp = *this;
		++(*this);
		return temp;
	}
	fv_iterator& operator--() {
		--_ptr;
		return *this;
	}
	fv_iterator operator--(int) {
		fv_iterator temp = *this;
		--(*this);
		return temp;
	}
	fv_iterator& operator+=(difference_type n) {
		_ptr += n;
		return *this;
	}
	fv_iterator& operator-=(difference_type n) {
		_ptr -= n;
		return *this;
	}

	fv_iterator operator+(int n) {
		return _ptr + n;
	}

	fv_iterator operator-(int n) {
		return _ptr - n;
	}

	reference operator[](difference_type n) const {
		return *(*this + n);
	}

	difference_type operator-(const fv_iterator& rhs) const {
		return _ptr - rhs._ptr;
	}

private:
	Ty* _ptr;
};


template <typename Ty, size_t FIXED_SIZE>
class fixed_vector final
{
public:
	using value_type = Ty;
	using reference = Ty&;
	using const_reference = const Ty&;
	using pointer = Ty*;
	using const_pointer = const Ty*;
	using difference_type = std::ptrdiff_t;
	using size_type = std::size_t;

	using iterator = fv_iterator<Ty, FIXED_SIZE>;
	using const_iterator = fv_iterator<const Ty, FIXED_SIZE>;
	using alloc_ptr = Ty*;

public:
	/*Constructor*/
	fixed_vector() = default;
	fixed_vector(::initializer_list<Ty>&& initializer);
	fixed_vector(size_t size, Ty defaultItem = Ty());

	template<size_t OTHER_SIZE>
	fixed_vector(const fixed_vector<Ty, OTHER_SIZE>& other);

	template<size_t OTHER_SIZE>
	fixed_vector(fixed_vector<Ty, OTHER_SIZE>&& other) noexcept;

	/*Destructor*/
	~fixed_vector() = default;

	/*Copy & Move*/
	template<size_t OTHER_SIZE>
	fixed_vector& operator=(const fixed_vector<Ty, OTHER_SIZE>& other);

	template<size_t OTHER_SIZE>
	fixed_vector& operator=(fixed_vector<Ty, OTHER_SIZE>&& other);

	Ty& operator[](size_t idx) { assert(idx < _size);  return _alloc[idx]; }
	Ty& at(size_t idx) { assert(idx < _size);  return _alloc[idx]; }

	const Ty& operator[](size_t idx) const { assert(idx < _size);  return _alloc[idx]; }
	const Ty& at(size_t idx) const { assert(idx < _size);  return _alloc[idx]; }


	bool full() const { return _size == _capacity; }
	bool empty() const { return _size == 0; }

public:
	void push_back(Ty item);
	void pop_back() { assert(_size != 0); --_size; }

	Ty back() const { assert(_size != 0);  return _alloc[_size - 1]; }
	Ty front() const { assert(_size != 0); return _alloc[0]; }

	void resize(size_t nSize, Ty defaultItem = Ty())
	{
		assert(nSize <= FIXED_SIZE);

		if (nSize > _size)
		{
			for (size_t i = _size; i < nSize; ++i)
				push_back(defaultItem);
		}
		else if (nSize < _size)
		{
			::destroy(_alloc + nSize, _alloc + _size);
			_size = nSize;
		}
	}

	void swap(fixed_vector& other) { ::swap(_alloc, other._alloc); ::swap(_size, other._size); }

	void clear() {
		::destroy(_alloc, _alloc + _size);
		_size = 0;
	}

	iterator insert(iterator pos, const Ty& item) {
		assert(_size < _capacity);
		auto index = pos - begin();
		assert(index >= 0 && index <= _size);

		for (size_t i = _size; i > index; --i)
			_alloc[i] = _alloc[i - 1];

		_alloc[index] = item;
		++_size;

		return begin() + index;
	}
	iterator erase(iterator pos) {
		auto index = pos - begin();
		assert(index >= 0 && index < _size);

		for (size_t i = index; i < _size - 1; ++i)
			_alloc[i] = _alloc[i + 1];

		--_size;

		return begin() + index;
	}

	iterator erase(iterator first, iterator last) {
		auto index = first - begin();
		assert(index >= 0 && index < _size);

		auto eraseCount = last - first;
		for (size_t i = index; i < _size - eraseCount; ++i)
			_alloc[i] = _alloc[i + eraseCount];

		::destroy(_alloc + _size - eraseCount, _alloc + _size);

		_size -= eraseCount;

		return begin() + index;
	}

	void assign(size_t nSize, const Ty& item) {
		assert(nSize <= _capacity);

		clear();

		for (size_t i = 0; i < nSize; ++i)
			push_back(item);
	}
	void assign(::initializer_list<Ty> initializer) {
		assert(initializer.size() <= _capacity);

		clear();

		::memcpy(_alloc, initializer.begin(), sizeof(Ty) * _size);
	}
	template <typename Iter>
	void assign(Iter first, Iter last)
	{
		size_t nSize = std::distance(first, last);
		assert(nSize >= 0);
		assert(nSize <= _capacity);

		clear();

		for (size_t i = 0; i < nSize; ++i)
		{
			push_back(*first);
			++first;
		}
	}

	iterator begin() { return iterator(_alloc); }
	iterator end() { return iterator(_alloc + _size); }

	const_iterator begin() const { return const_iterator(_alloc); }
	const_iterator end() const { return const_iterator(_alloc + _size); }

	const_iterator cbegin() const { return const_iterator(_alloc); }
	const_iterator cend() const { return const_iterator(_alloc + _size); }


	size_t size() const { return _size; }
	size_t max_size() const { return FIXED_SIZE; }
	size_t capacity() const { return _capacity; }


private:

	Ty				_alloc[FIXED_SIZE];
	size_t			_size = 0;
	size_t			_capacity = FIXED_SIZE;
};

inline bool cmpRef(const void* left, const void* right)
{
	return left == right;
}


template<typename Ty, size_t FIXED_SIZE>
inline fixed_vector<Ty, FIXED_SIZE>::fixed_vector(::initializer_list<Ty>&& initializer)
	: _size(initializer.size())
{
	assert(FIXED_SIZE >= initializer.size());

	::memcpy(_alloc, initializer.begin(), sizeof(Ty) * _size);
}

template<typename Ty, size_t FIXED_SIZE>
inline fixed_vector<Ty, FIXED_SIZE>::fixed_vector(size_t size, Ty defaultItem)
	: _size(size)
{
	assert(FIXED_SIZE >= size);

	for (size_t i = 0; i < _size; i++)
		_alloc[i] = defaultItem;
}

template<typename Ty, size_t FIXED_SIZE>
template<size_t OTHER_SIZE>
inline fixed_vector<Ty, FIXED_SIZE>& fixed_vector<Ty, FIXED_SIZE>::operator=(const fixed_vector<Ty, OTHER_SIZE>& other)
{
	assert(FIXED_SIZE >= other.size());

	if (!cmpRef(this, &other))
	{
		clear();
		_size = other.size();

		for (size_t i = 0; i < _size; i++)
			_alloc[i] = other[i];
	}

	return *this;
}

template<typename Ty, size_t FIXED_SIZE>
template<size_t OTHER_SIZE>
inline fixed_vector<Ty, FIXED_SIZE>& fixed_vector<Ty, FIXED_SIZE>::operator=(fixed_vector<Ty, OTHER_SIZE>&& other)
{
	assert(FIXED_SIZE >= other.size());

	if (!cmpRef(this, &other))
	{
		clear();
		_size = other.size();

		for (size_t i = 0; i < _size; i++)
			_alloc[i] = other[i];

		other.clear();
	}

	return *this;
}

template<typename Ty, size_t FIXED_SIZE>
template<size_t OTHER_SIZE>
inline fixed_vector<Ty, FIXED_SIZE>::fixed_vector(const fixed_vector<Ty, OTHER_SIZE>& other)
	: _size(other.size())
{
	assert(FIXED_SIZE >= other.size());

	for (size_t i = 0; i < _size; i++)
		_alloc[i] = other[i];
}

template<typename Ty, size_t FIXED_SIZE>
template<size_t OTHER_SIZE>
inline fixed_vector<Ty, FIXED_SIZE>::fixed_vector(fixed_vector<Ty, OTHER_SIZE>&& other) noexcept
	: _size(other.size())
{
	assert(FIXED_SIZE >= other.size());

	for (size_t i = 0; i < _size; i++)
		_alloc[i] = other[i];

	other.clear();
}

template<typename Ty, size_t FIXED_SIZE>
inline void fixed_vector<Ty, FIXED_SIZE>::push_back(Ty item)
{
	assert(_size < _capacity);
	_alloc[_size] = item;
	++_size;
}
