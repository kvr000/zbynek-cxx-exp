#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <sys/resource.h>

#include <cstdio>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <array>

using namespace std;

int sumMpOverflowVar(unsigned *result, unsigned *a, unsigned *b, size_t size)
{
	uint8_t overflow = 0;
	for (int i = 0; i < size; ++i) {
		result[i] = a[i]+b[i]+overflow;
		overflow = result[i] < a[i];
	}
	return overflow;
}

int sumMpOverflowHigher(uint32_t *result, uint32_t *a, uint32_t *b, size_t size)
{
	uint64_t sum = 0;
	for (int i = 0; i < size; ++i) {
		result[i] = sum = a[i]+b[i]+(sum>>32);
	}
	return sum>>32;
}


int main(void)
{
	return 0;
}
