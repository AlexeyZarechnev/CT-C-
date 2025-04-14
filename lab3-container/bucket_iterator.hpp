#pragma once

#include <cstdint>
#include <iostream>
#include <iterator>
#include <limits>
#include <stdexcept>

// ------------------------------------
// START OF INTERFACE
// ------------------------------------

template< typename Container >
class BucketIterator
{
  public:
	using iterator_category = std::bidirectional_iterator_tag;
	using value_type = typename Container::value_type;
	using difference_type = std::ptrdiff_t;
	using pointer = typename Container::value_type *;
	using reference = typename Container::value_type &;

	BucketIterator(const BucketIterator &rhs) noexcept = default;
	BucketIterator(BucketIterator &&rhs) noexcept = default;

	BucketIterator &operator=(const BucketIterator &rhs) noexcept = default;
	BucketIterator &operator=(BucketIterator &&rhs) noexcept = default;

	BucketIterator &operator++();
	BucketIterator operator++(int);

	BucketIterator &operator--();
	BucketIterator operator--(int);

	template< typename Container2 >
	bool operator==(const BucketIterator< Container2 > &rhs) const noexcept;
	template< typename Container2 >
	bool operator!=(const BucketIterator< Container2 > &rhs) const noexcept;

	template< typename Container2 >
	bool operator<(const BucketIterator< Container2 > &rhs) const noexcept;
	template< typename Container2 >
	bool operator<=(const BucketIterator< Container2 > &rhs) const noexcept;
	template< typename Container2 >
	bool operator>(const BucketIterator< Container2 > &rhs) const noexcept;
	template< typename Container2 >
	bool operator>=(const BucketIterator< Container2 > &rhs) const noexcept;

	operator BucketIterator< const Container >() const noexcept;

	reference operator*() const;
	pointer operator->() const;

	Container *current_;
	std::size_t block_id_;
	std::size_t id_;

	BucketIterator(Container *current, std::size_t block_id, std::size_t id) noexcept;

	template< typename T >
	friend class BucketStorage;
	template< typename Container2 >
	friend class BucketIterator;
};

// ------------------------------------
// START OF IMPLEMENTATION
// ------------------------------------

template< typename Container >
inline BucketIterator< Container > &BucketIterator< Container >::operator++()
{
	if (block_id_ > current_->last_active_)
		throw std::invalid_argument("cannot increment end() iterator");

	std::size_t *activity = current_->activity_;
	std::size_t id = current_->blocks_[block_id_]->next_id(id_);

	if (id >= current_->block_size_)
	{
		if (block_id_ + 1 <= current_->last_active_)
		{
			block_id_ = std::min(current_->last_active_ + 1, block_id_ + activity[block_id_ + 1] + 1);
			id = (block_id_ <= current_->last_active_) ? current_->blocks_[block_id_]->first_id() : 0;
		}
		else
		{
			block_id_ = current_->last_active_ + 1;
			id = 0;
		}
	}
	id_ = id;
	return *this;
}

template< typename Container >
inline BucketIterator< Container > BucketIterator< Container >::operator++(int)
{
	BucketIterator temp(current_, block_id_, id_);
	++(*this);
	return temp;
}

template< typename Container >
inline BucketIterator< Container > &BucketIterator< Container >::operator--()
{
	if (block_id_ == current_->activity_[0] && id_ == current_->blocks_[block_id_]->first_id())
		throw std::invalid_argument("cannot decrement begin() iterator");
	if (block_id_ > current_->last_active_)
	{
		block_id_ = current_->last_active_;
		id_ = current_->blocks_[block_id_]->last_id();
	}
	else
	{
		std::size_t *b_activity = current_->b_activity_;
		std::size_t id = current_->blocks_[block_id_]->prev_id(id_);
		if (id >= current_->block_size_)
		{
			block_id_ -= 1 + b_activity[block_id_ - 1];
			id = current_->blocks_[block_id_]->last_id();
		}
		id_ = id;
	}
	return *this;
}

template< typename Container >
inline BucketIterator< Container > BucketIterator< Container >::operator--(int)
{
	BucketIterator temp(current_, block_id_, id_);
	--(*this);
	return temp;
}

template< typename Container >
inline typename BucketIterator< Container >::reference BucketIterator< Container >::operator*() const
{
	return current_->blocks_[block_id_]->get(id_);
}

template< typename Container >
inline typename BucketIterator< Container >::pointer BucketIterator< Container >::operator->() const
{
	return &current_->blocks_[block_id_]->get(id_);
}

template< typename Container >
inline BucketIterator< Container >::BucketIterator(Container *current, std::size_t block_id, std::size_t id) noexcept :
	current_(current), block_id_(block_id), id_(id)
{
}

template< typename Container >
inline BucketIterator< Container >::operator BucketIterator< const Container >() const noexcept
{
	return BucketIterator< const Container >(current_, block_id_, id_);
}

template< typename Container >
template< typename Container2 >
inline bool BucketIterator< Container >::operator==(const BucketIterator< Container2 > &rhs) const noexcept
{
	return block_id_ == rhs.block_id_ && id_ == rhs.id_;
}

template< typename Container >
template< typename Container2 >
inline bool BucketIterator< Container >::operator!=(const BucketIterator< Container2 > &rhs) const noexcept
{
	return !(*this == rhs);
}

template< typename Container >
template< typename Container2 >
inline bool BucketIterator< Container >::operator<(const BucketIterator< Container2 > &rhs) const noexcept
{
	return current_ == current_ && (block_id_ < rhs.block_id_ || (block_id_ == rhs.block_id_ && id_ < rhs.id_));
}

template< typename Container >
template< typename Container2 >
inline bool BucketIterator< Container >::operator<=(const BucketIterator< Container2 > &rhs) const noexcept
{
	return (*this < rhs) || (*this == rhs);
}

template< typename Container >
template< typename Container2 >
inline bool BucketIterator< Container >::operator>(const BucketIterator< Container2 > &rhs) const noexcept
{
	return !(*this <= rhs);
}

template< typename Container >
template< typename Container2 >
inline bool BucketIterator< Container >::operator>=(const BucketIterator< Container2 > &rhs) const noexcept
{
	return !(*this < rhs);
}
