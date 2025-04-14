#include "cross_correlation.h"

#include "../return_codes.h"

#include <fftw3.h>
#include <math.h>
#include <stdbool.h>

fftw_complex* bytes_to_complex(ByteVector* bytes, size_t size, size_t offset, size_t zero_pad_size)
{
	size_t bytes_size = v_get_size(bytes);
	fftw_complex* complex = fftw_alloc_complex(bytes_size / size + offset + zero_pad_size);
	if (!complex)
		return NULL;
	uint8_t* data = v_get_data(bytes);
	for (size_t i = 0; i < bytes_size; i += size)
	{
		for (size_t d = 0; d < size; ++d)
		{
			complex[offset + (i / size)][0] += (data[i + d] << d * 8);
		}
	}
	return complex;
}

void count_fft(fftw_complex* f, size_t size, int sign)
{
	fftw_plan plan = fftw_plan_dft_1d(size, f, f, sign, FFTW_ESTIMATE);
	fftw_execute(plan);
	fftw_destroy_plan(plan);
}

int cc_time_count(ByteVector* bytes1, size_t size1, ByteVector* bytes2, size_t size2)
{
	bool error = false;
	size_t size_f1 = v_get_size(bytes1) / size1;
	size_t size_f2 = v_get_size(bytes2) / size2;
	size_t total_size = size_f1 + size_f2;
	fftw_complex* f1 = bytes_to_complex(bytes1, size1, size_f2, total_size - size_f1 - size_f2);
	if (!f1)
	{
		error = true;
		goto cleanup;
	}
	free_vector(&bytes1);
	fftw_complex* f2 = bytes_to_complex(bytes2, size2, 0, total_size - size_f2);
	if (!f2)
	{
		error = true;
		goto cleanup;
	}
	free_vector(&bytes2);
	count_fft(f1, total_size, FFTW_FORWARD);
	count_fft(f2, total_size, FFTW_FORWARD);
	fftw_complex* total = fftw_alloc_complex(total_size);
	if (!total)
	{
		error = true;
		goto cleanup;
	}
	for (size_t i = 0; i < total_size; ++i)
	{
		total[i][0] = (f1[i][0] * f2[i][0] - f1[i][1] * -f2[i][1]);
		total[i][1] = (f1[i][0] * -f2[i][1] + f1[i][1] * f2[i][0]);
	}
	count_fft(total, total_size, FFTW_BACKWARD);
	size_t answer = 0;
	for (size_t i = 1; i < total_size; ++i)
	{
		if (total[answer][0] < total[i][0])
		{
			answer = i;
		}
	}
cleanup:
	if (f1)
		fftw_free(f1);
	if (f2)
		fftw_free(f2);
	if (total)
		fftw_free(total);
	if (error)
		exit(ERROR_NOTENOUGH_MEMORY);
	return (int)(answer - size_f2);
}
