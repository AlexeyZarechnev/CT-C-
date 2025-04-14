#pragma once

#include "stack.hpp"

#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <stdexcept>

// ------------------------------------
// START OF INTERFACE
// ------------------------------------

template< typename value_type >
class Block
{
	value_type *_data;
	stack< std::size_t > _free_cells;
	std::size_t _size;
	std::size_t *_activity;
	std::size_t *_b_activity;
	std::size_t _capacity;
	std::size_t _last_active;
	std::size_t _first_active;

  public:
	Block(std::size_t capacity);
	Block(const Block &other);
	Block(Block &&other) noexcept;

	Block &operator=(const Block &other);
	Block &operator=(Block &&other) noexcept;

	std::size_t insert(const value_type &value);
	std::size_t insert(value_type &&value);

	std::size_t first_id() const noexcept;
	std::size_t last_id() const noexcept;
	std::size_t next_id(std::size_t id) const noexcept;
	std::size_t prev_id(std::size_t id) const noexcept;

	std::size_t remove(std::size_t id) noexcept;

	std::size_t size() const noexcept;

	void swap(Block &other) noexcept;

	value_type &get(std::size_t id);

	~Block() noexcept;

  private:
	std::size_t getId();
	void count_activity(std::size_t id);
};

// ------------------------------------
// START OF IMPLEMENTATION
// ------------------------------------

template< typename value_type >
inline Block< value_type >::Block(std::size_t capacity) :
	_data(reinterpret_cast< value_type * >(std::malloc(capacity * sizeof(value_type)))), _free_cells(), _size(0),
	_activity(new std::size_t[capacity]), _b_activity(new std::size_t[capacity]), _capacity(capacity), _last_active(0),
	_first_active(0)
{
	if (!_data || !_activity || !_b_activity)
		throw std::bad_alloc();

	_free_cells.insert(0);
	std::fill(_activity, _activity + capacity, capacity);
	std::fill(_b_activity, _b_activity + capacity, capacity);
}

template< typename value_type >
inline Block< value_type >::Block(const Block &other) :
	_data(reinterpret_cast< value_type * >(std::malloc(other._capacity * sizeof(value_type)))), _free_cells(other._free_cells),
	_size(other._size), _activity(new std::size_t[other._capacity]), _b_activity(new std::size_t[other._capacity]),
	_capacity(other._capacity), _last_active(other._last_active), _first_active(other._first_active)
{
	for (std::size_t i = 0; i < other._capacity; ++i)
	{
		if (!other._activity[i])
			new (_data + i) value_type(other._data[i]);
	}
	std::copy(other._activity, other._activity + other._capacity, _activity);
	std::copy(other._b_activity, other._b_activity + other._capacity, _b_activity);
}

template< typename value_type >
inline Block< value_type >::Block(Block &&other) noexcept :
	_data(other._data), _free_cells(std::move(other._free_cells)), _size(other._size), _activity(other._activity),
	_b_activity(other._b_activity), _capacity(other._capacity), _last_active(other._last_active), _first_active(other._first_active)
{
	other._data = nullptr;
	other._activity = nullptr;
	other._b_activity = nullptr;
}

template< typename value_type >
inline Block< value_type > &Block< value_type >::operator=(const Block &other)
{
	Block temp(other);
	swap(temp);
	return *this;
}

template< typename value_type >
inline Block< value_type > &Block< value_type >::operator=(Block &&other) noexcept
{
	Block temp(std::move(other));
	swap(temp);
	return *this;
}

template< typename value_type >
inline std::size_t Block< value_type >::insert(const value_type &value)
{
	std::size_t id = getId();
	new (_data + id) value_type(value);
	count_activity(id);
	return id;
}

template< typename value_type >
inline std::size_t Block< value_type >::insert(value_type &&value)
{
	std::size_t id = getId();
	new (_data + id) value_type(std::move(value));
	count_activity(id);
	return id;
}

template< typename value_type >
inline std::size_t Block< value_type >::first_id() const noexcept
{
	return _first_active;
}

template< typename value_type >
inline std::size_t Block< value_type >::last_id() const noexcept
{
	return _last_active;
}

template< typename value_type >
inline std::size_t Block< value_type >::next_id(std::size_t id) const noexcept
{
	if (id >= _last_active)
		return _capacity;
	return id + 1 < _capacity ? id + _activity[id + 1] + 1 : _capacity;
}

template< typename value_type >
inline std::size_t Block< value_type >::prev_id(std::size_t id) const noexcept
{
	return id > 0 ? id - _activity[id - 1] - 1 : _capacity;
}

template< typename value_type >
inline std::size_t Block< value_type >::remove(std::size_t id) noexcept
{
	--_size;
	_data[id].~value_type();

	if (id + 1 <= _last_active)
		_activity[id] = 1 + _activity[id + 1];
	else
		_activity[id] = _capacity;

	if (id > 0)
	{
		_b_activity[id] = 1 + _b_activity[id - 1];
		if (_activity[id - 1])
			_activity[id - 1] = 1 + _activity[id];
		else
			_free_cells.insert(id);
	}
	else
	{
		_free_cells.insert(0);
		_b_activity[id] = _capacity;
	}

	if (id == _first_active)
	{
		_b_activity[id] = _capacity;
		_first_active = id + _activity[id];
	}
	if (id + 1 < _capacity)
		_b_activity[id + 1] = 1 + _b_activity[id];
	if (id == _last_active)
		_last_active = id - _b_activity[id];

	return next_id(id);
}

template< typename value_type >
inline std::size_t Block< value_type >::size() const noexcept
{
	return _size;
}

template< typename value_type >
inline void Block< value_type >::swap(Block &other) noexcept
{
	std::swap(_data, other._data);
	std::swap(_free_cells, other._free_cells);
	std::swap(_size, other._size);
	std::swap(_activity, other._activity);
	std::swap(_b_activity, other._b_activity);
	std::swap(_capacity, other._capacity);
	std::swap(_last_active, other._last_active);
	std::swap(_first_active, other._first_active);
}

template< typename value_type >
inline value_type &Block< value_type >::get(std::size_t id)
{
	return _data[id];
}

template< typename value_type >
inline Block< value_type >::~Block() noexcept
{
	if (_data)
	{
		for (std::size_t i = 0; i < _capacity; ++i)
		{
			if (!_activity[i])
				(_data + i)->~value_type();
		}
		std::free(_data);
		delete[] _activity;
		delete[] _b_activity;
	}
}

template< typename value_type >
inline std::size_t Block< value_type >::getId()
{
	if (_free_cells.empty())
		throw std::runtime_error("Block is full");

	std::size_t id;
	id = _free_cells.top();
	while (!_activity[id])
	{
		_free_cells.pop();
		id = _free_cells.top();
	}
	return id;
}

template< typename value_type >
inline void Block< value_type >::count_activity(std::size_t id)
{
	_free_cells.pop();
	++_size;
	_activity[id] = 0;
	_b_activity[id] = 0;
	if (id + 1 < _capacity && _activity[id + 1])
		_free_cells.insert(id + 1);
	_last_active = std::max(_last_active, id);
	_first_active = std::min(_first_active, id);
}
