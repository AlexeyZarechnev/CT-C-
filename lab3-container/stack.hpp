#pragma once

#include <algorithm>
#include <cstdlib>
#include <stdexcept>

// ------------------------------------
// START OF INTERFACE
// ------------------------------------

template< typename T >
class stack
{
  public:
	using value_type = T;

	stack();
	stack(const stack& other);
	stack(stack&& other) noexcept;

	stack& operator=(const stack& other);
	stack& operator=(stack&& other) noexcept;

	void insert(value_type value);
	value_type top() const;
	value_type pop();
	bool empty() const;

	~stack();

  private:
	value_type* data_;
	size_t capacity_;
	size_t size_;

	void ensure_capacity();
};

// ------------------------------------
// START OF IMPLEMENTATION
// ------------------------------------

template< typename T >
inline stack< T >::stack() : data_(new value_type[1]), size_(0), capacity_(1)
{
	if (!data_)
		throw std::bad_alloc();
}

template< typename T >
inline stack< T >::stack(const stack& other) :
	data_(new value_type[other.capacity_]), size_(other.size_), capacity_(other.capacity_)
{
	std::copy(other.data_, other.data_ + other.capacity_, data_);
}

template< typename T >
inline stack< T >::stack(stack&& other) noexcept : data_(other.data_), size_(other.size_), capacity_(other.capacity_)
{
	other.data_ = nullptr;
}

template< typename T >
inline stack< T >& stack< T >::operator=(const stack& other)
{
	stack temp(other);
	std::swap(*this, temp);
	return *this;
}

template< typename T >
inline stack< T >& stack< T >::operator=(stack&& other) noexcept
{
	std::swap(data_, other.data_);
	std::swap(size_, other.size_);
	std::swap(capacity_, other.capacity_);
	return *this;
}

template< typename T >
inline void stack< T >::insert(value_type value)
{
	ensure_capacity();
	data_[size_++] = value;
}

template< typename T >
inline typename stack< T >::value_type stack< T >::top() const
{
	if (empty())
		throw std::invalid_argument("you cannot top empty stack");

	return data_[size_ - 1];
}

template< typename T >
inline typename stack< T >::value_type stack< T >::pop()
{
	if (empty())
		throw std::invalid_argument("you cannot pop from empty stack");

	return data_[--size_];
}

template< typename T >
inline bool stack< T >::empty() const
{
	return size_ == 0;
}

template< typename T >
inline stack< T >::~stack()
{
	if (data_)
		delete[] data_;
}

template< typename T >
inline void stack< T >::ensure_capacity()
{
	if (size_ < capacity_)
		return;
	value_type* big_data = new value_type[capacity_ * 2];
	std::copy(data_, data_ + capacity_, big_data);
	delete[] data_;
	data_ = big_data;
	capacity_ *= 2;
}
