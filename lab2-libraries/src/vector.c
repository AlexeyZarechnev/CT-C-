#include "vector.h"

typedef struct ByteVector
{
	uint8_t* data;
	size_t current_size;
	size_t maximum_size;
} ByteVector;

ByteVector* alloc_vector()
{
	ByteVector* vector = (ByteVector*)malloc(sizeof(ByteVector));
	if (!vector)
		return NULL;
	vector->data = (uint8_t*)malloc(10);
	if (!vector->data)
	{
		free(vector);
		return NULL;
	}
	vector->current_size = 0;
	vector->maximum_size = 10;
	return vector;
}

void free_vector(ByteVector** vector)
{
	free((*vector)->data);
	free(*vector);
	*vector = NULL;
}

void array_copy(void* dest, const void* src, size_t src_size)
{
	uint8_t* byte_dest = (uint8_t*)dest;
	uint8_t* byte_src = (uint8_t*)src;
	for (size_t i = 0; i < src_size; ++i)
		byte_dest[i] = byte_src[i];
}

int ensure_capacity(ByteVector* vector, size_t data_size)
{
	static int numerator = 20, denominator = 1;
	while (vector->maximum_size - vector->current_size < data_size)
	{
		uint8_t* big_data = (uint8_t*)malloc(numerator * vector->maximum_size / denominator);
		if (!big_data)
			return 1;
		vector->maximum_size *= numerator++;
		vector->maximum_size /= denominator++;
		array_copy(big_data, vector->data, vector->current_size);
		free(vector->data);
		vector->data = big_data;
	}
	return 0;
}

int push_back(ByteVector* vector, uint8_t* data, size_t data_size)
{
	if (ensure_capacity(vector, data_size))
	{
		return 1;
	}
	array_copy(vector->data + vector->current_size, data, data_size);
	vector->current_size += data_size;
	return 0;
}

uint8_t* v_get_data(ByteVector* vector)
{
	return vector->data;
}

size_t v_get_size(ByteVector* vector)
{
	return vector->current_size;
}
