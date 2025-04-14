#ifndef VECTOR
#define VECTOR

#include <stdint.h>
#include <stdlib.h>

typedef struct ByteVector ByteVector;

ByteVector* alloc_vector(void);

void free_vector(ByteVector** vector);

int push_back(ByteVector* vector, uint8_t* data, size_t data_size);

uint8_t* v_get_data(ByteVector* vector);

size_t v_get_size(ByteVector* vector);

#endif
