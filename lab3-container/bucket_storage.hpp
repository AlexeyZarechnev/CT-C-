#pragma once

#include "block.hpp"
#include "bucket_iterator.hpp"
#include "stack.hpp"

#include <cstdint>

// ------------------------------------
// START OF INTERFACE
// ------------------------------------

template< typename T >
class BucketStorage
{
  public:
	typedef T value_type;
	typedef T &reference;
	typedef const T &const_reference;
	typedef std::size_t size_type;
	typedef Block< value_type > block;
	typedef BucketIterator< BucketStorage > iterator;
	typedef BucketIterator< const BucketStorage > const_iterator;
	typedef typename iterator::difference_type difference_type;

	BucketStorage(size_type block_size = 64);
	BucketStorage(const BucketStorage &other);
	BucketStorage(BucketStorage &&other) noexcept;

	BucketStorage &operator=(const BucketStorage &other);
	BucketStorage &operator=(BucketStorage &&other);

	iterator insert(const value_type &value);
	iterator insert(value_type &&value);
	iterator erase(const_iterator it);

	bool empty() const noexcept;
	size_type size() const noexcept;
	size_type capacity() const noexcept;

	void shrink_to_fit();
	void clear() noexcept;
	void swap(BucketStorage &other) noexcept;

	iterator begin() noexcept;
	const_iterator begin() const noexcept;
	const_iterator cbegin() const noexcept;

	iterator end() noexcept;
	const_iterator end() const noexcept;
	const_iterator cend() const noexcept;

	iterator get_to_distance(iterator it, const difference_type distance);

	~BucketStorage();

  private:
	size_type block_size_;
	block **blocks_;
	size_type capacity_;
	size_type blocks_count_;
	size_type size_;
	size_type *activity_;
	size_type *b_activity_;
	size_type last_active_;
	size_type first_active_;
	stack< size_type > free_pointers_;
	stack< size_type > free_blocks_;

	size_type getBlockId() noexcept;
	void updateFreeBlocks(std::size_t block_id) noexcept;
	void add_block();

	template< typename Container >
	friend class BucketIterator;
};

// ------------------------------------
// START OF IMPLEMENTATION
// ------------------------------------

template< typename T >
inline BucketStorage< T >::BucketStorage(size_type block_size) :
	block_size_(block_size), blocks_(new block *[1]), capacity_(1), blocks_count_(0), size_(0), activity_(new size_type[1]),
	b_activity_(new size_type[1]), last_active_(0), first_active_(0), free_pointers_(), free_blocks_()
{
	free_pointers_.insert(0);
	std::fill(activity_, activity_ + capacity_, capacity_);
	std::fill(b_activity_, b_activity_ + capacity_, capacity_);
}

template< typename T >
inline BucketStorage< T >::BucketStorage(const BucketStorage &other) :
	block_size_(other.block_size_), blocks_(new block *[other.capacity_]), capacity_(other.capacity_),
	blocks_count_(other.blocks_count_), size_(other.size_), activity_(new size_type[other.capacity_]),
	b_activity_(new size_type[other.capacity_]), last_active_(other.last_active_), first_active_(other.first_active_),
	free_pointers_(other.free_pointers_), free_blocks_(other.free_blocks_)
{
	std::copy(other.activity_, other.activity_ + other.capacity_, activity_);
	std::copy(other.b_activity_, other.b_activity_ + other.capacity_, b_activity_);

	for (size_type i = 0; i < capacity_; ++i)
	{
		if (!other.activity_[i])
		{
			blocks_[i] = new block(*other.blocks_[i]);
		}
	}
}

template< typename T >
inline BucketStorage< T >::BucketStorage(BucketStorage &&other) noexcept :
	block_size_(other.block_size_), blocks_(other.blocks_), capacity_(other.capacity_), blocks_count_(other.blocks_count_),
	size_(other.size_), activity_(other.activity_), b_activity_(other.b_activity_), last_active_(other.last_active_),
	first_active_(other.first_active_), free_pointers_(other.free_pointers_), free_blocks_(other.free_blocks_)
{
	other.blocks_ = nullptr;
	other.activity_ = nullptr;
	other.b_activity_ = nullptr;
}

template< typename T >
inline BucketStorage< T > &BucketStorage< T >::operator=(const BucketStorage &other)
{
	if (this != &other)
	{
		BucketStorage temp(other);
		swap(temp);
	}
	return *this;
}

template< typename T >
inline BucketStorage< T > &BucketStorage< T >::operator=(BucketStorage &&other)
{
	if (this != &other)
	{
		BucketStorage temp(std::move(other));
		swap(temp);
	}
	return *this;
}

template< typename T >
inline typename BucketStorage< T >::iterator BucketStorage< T >::insert(const value_type &value)
{
	size_type block_id = getBlockId();
	size_type id = blocks_[block_id]->insert(value);
	updateFreeBlocks(block_id);
	return iterator(this, block_id, id);
}

template< typename T >
inline typename BucketStorage< T >::iterator BucketStorage< T >::insert(value_type &&value)
{
	size_type block_id = getBlockId();
	size_type id = blocks_[block_id]->insert(std::move(value));
	updateFreeBlocks(block_id);
	return iterator(this, block_id, id);
}

template< typename T >
inline typename BucketStorage< T >::iterator BucketStorage< T >::erase(const_iterator it)
{
	size_type block_id = it.block_id_;
	size_type id = it.id_;

	--size_;
	id = blocks_[block_id]->remove(id);
	if (blocks_[block_id]->size() == block_size_ - 1)
		free_blocks_.insert(block_id);

	if (blocks_[block_id]->size() == 0)
	{
		if (block_id == 0 || (block_id > 0 && !activity_[block_id - 1]))
			free_pointers_.insert(block_id);
		if (block_id + 1 < capacity_)
		{
			activity_[block_id] = 1 + activity_[block_id + 1];
		}
		else
		{
			activity_[block_id] = capacity_;
		}
		if (block_id > 0)
		{
			b_activity_[block_id] = 1 + b_activity_[block_id - 1];
			if (activity_[block_id - 1])
				activity_[block_id - 1] = 1 + activity_[block_id];
		}

		if (block_id == last_active_)
		{
			last_active_ = last_active_ - b_activity_[block_id - 1] - 1;
			if (last_active_ >= capacity_)
				last_active_ = 0;
		}
		if (block_id == first_active_)
			first_active_ = block_id + activity_[block_id];

		delete blocks_[block_id];
		blocks_[block_id] = nullptr;
		--blocks_count_;
	}

	if (id >= block_size_)
	{
		if (block_id < last_active_)
		{
			block_id += 1 + activity_[block_id + 1];
			id = blocks_[block_id]->first_id();
		}
		else
		{
			block_id = 1 + last_active_;
			id = 0;
		}
	}

	return iterator(this, block_id, id);
}

template< typename T >
inline bool BucketStorage< T >::empty() const noexcept
{
	return size_ == 0;
}

template< typename T >
inline typename BucketStorage< T >::size_type BucketStorage< T >::size() const noexcept
{
	return size_;
}

template< typename T >
inline typename BucketStorage< T >::size_type BucketStorage< T >::capacity() const noexcept
{
	return blocks_count_ * block_size_;
}

template< typename T >
inline void BucketStorage< T >::shrink_to_fit()
{
	BucketStorage fitted(block_size_);
	for (auto el : *this)
	{
		fitted.insert(el);
	}
	swap(fitted);
}

template< typename T >
inline void BucketStorage< T >::clear() noexcept
{
	BucketStorage clean(block_size_);
	swap(clean);
}

template< typename T >
inline void BucketStorage< T >::swap(BucketStorage &other) noexcept
{
	std::swap(block_size_, other.block_size_);
	std::swap(blocks_, other.blocks_);
	std::swap(capacity_, other.capacity_);
	std::swap(blocks_count_, other.blocks_count_);
	std::swap(size_, other.size_);
	std::swap(activity_, other.activity_);
	std::swap(b_activity_, other.b_activity_);
	std::swap(last_active_, other.last_active_);
	std::swap(first_active_, other.first_active_);
	std::swap(free_pointers_, other.free_pointers_);
	std::swap(free_blocks_, other.free_blocks_);
}

template< typename T >
inline typename BucketStorage< T >::iterator BucketStorage< T >::begin() noexcept
{
	if (!blocks_count_)
		return end();

	std::size_t block_id = first_active_;
	return iterator(this, block_id, blocks_[block_id]->first_id());
}

template< typename T >
inline typename BucketStorage< T >::const_iterator BucketStorage< T >::begin() const noexcept
{
	if (!blocks_count_)
		return cend();

	std::size_t block_id = first_active_;
	return const_iterator(this, block_id, blocks_[block_id]->first_id());
}

template< typename T >
inline typename BucketStorage< T >::const_iterator BucketStorage< T >::cbegin() const noexcept
{
	if (!blocks_count_)
		return cend();

	std::size_t block_id = first_active_;
	return const_iterator(this, block_id, blocks_[block_id]->first_id());
}

template< typename T >
inline typename BucketStorage< T >::iterator BucketStorage< T >::end() noexcept
{
	return iterator(this, last_active_ + 1, 0);
}

template< typename T >
inline typename BucketStorage< T >::const_iterator BucketStorage< T >::end() const noexcept
{
	return const_iterator(this, last_active_ + 1, 0);
}

template< typename T >
inline typename BucketStorage< T >::const_iterator BucketStorage< T >::cend() const noexcept
{
	return const_iterator(this, last_active_ + 1, 0);
}

template< typename T >
inline typename BucketStorage< T >::iterator BucketStorage< T >::get_to_distance(iterator it, const difference_type distance)
{
	size_type dst = std::abs(distance);
	for (size_type i = 0; i < dst; ++i)
		if (distance < 0)
			--it;
		else
			++it;

	return it;
}

template< typename T >
inline BucketStorage< T >::~BucketStorage()
{
	if (blocks_)
	{
		for (size_type i = 0; i < capacity_; ++i)
		{
			if (!activity_[i])
				delete blocks_[i];
		}
		delete[] blocks_;
		blocks_ = nullptr;
		delete[] activity_;
		activity_ = nullptr;
		delete[] b_activity_;
		b_activity_ = nullptr;
	}
}

template< typename T >
inline BucketStorage< T >::size_type BucketStorage< T >::getBlockId() noexcept
{
	if (free_blocks_.empty())
		add_block();

	return free_blocks_.top();
}

template< typename T >
inline void BucketStorage< T >::updateFreeBlocks(std::size_t block_id) noexcept
{
	free_blocks_.pop();
	++size_;

	if (blocks_[block_id]->size() < block_size_)
		free_blocks_.insert(block_id);
}

template< typename T >
inline void BucketStorage< T >::add_block()
{
	if (free_pointers_.empty())
	{
		block **big_blocks = new block *[capacity_ * 2];
		size_type *big_activity = new size_type[capacity_ * 2];
		size_type *big_b_activity = new size_type[capacity_ * 2];

		std::copy(blocks_, blocks_ + capacity_, big_blocks);
		std::copy(activity_, activity_ + capacity_, big_activity);
		std::copy(b_activity_, b_activity_ + capacity_, big_b_activity);

		std::fill(big_activity + capacity_, big_activity + 2 * capacity_, 2 * capacity_);
		std::fill(big_b_activity + capacity_, big_b_activity + 2 * capacity_, 2 * capacity_);

		delete[] blocks_;
		delete[] activity_;
		delete[] b_activity_;

		blocks_ = big_blocks;
		activity_ = big_activity;
		b_activity_ = big_b_activity;

		free_pointers_.insert(capacity_);
		capacity_ *= 2;
	}
	size_type id;
	do
	{
		id = free_pointers_.pop();
	} while (!activity_[id]);

	if (id + 1 < capacity_ && activity_[id])
		free_pointers_.insert(id + 1);
	++blocks_count_;
	blocks_[id] = new block(block_size_);
	activity_[id] = 0;
	b_activity_[id] = 0;
	last_active_ = std::max(last_active_, id);
	free_blocks_.insert(id);
}
