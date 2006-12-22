#include <stdint.h>
#if INTPTR_MAX == INT64_MAX
# define PTR_ASM_INT ".quad"
#elif INTPTR_MAX == INT32_MAX
# define PTR_ASM_INT ".long"
#elif INTPTR_MAX == INT16_MAX
# define PTR_ASM_INT ".short"
#else
# error Unexpected platform
#endif

int runner_data;

static volatile int some_var = 4;

extern void ext_func();

int runner_function() {
	return 0;
}

static void marked_code() {
	__asm__ __volatile__(
			"1:\n"
			".section .__switch_fixup\n"
			"3:\n"
			"jmp 1b\n"
			".previous\n"
	);
	int value = runner_data+some_var;
	runner_data = value;
	__asm__ __volatile__(
			"2:\n"
			".section .__switch_fallback\n"
			"8:\n"
			"\t" PTR_ASM_INT  " 1b-., 2b-(9f-8b)/3-., 3b-(9f-8b)/3*2-., 0\n"
			"9:\n"
			".previous\n"
	);
}
