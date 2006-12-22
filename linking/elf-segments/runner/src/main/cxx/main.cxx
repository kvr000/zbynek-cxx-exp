#ifdef __linux__
# ifndef _GNU_SOURCE
#  define _GNU_SOURCE
# endif
# include <link.h>
#endif
#include <stdlib.h>
#include <stdio.h>

extern char __switch_fallback_start;
extern char __switch_fallback_end;

#ifdef __linux__
static int callback(struct dl_phdr_info *info, size_t size, void *data)
{
	const char *type;
	int p_type, j;

	printf("Name: \"%s\" (%d segments)\n", info->dlpi_name,
			info->dlpi_phnum);

	for (j = 0; j < info->dlpi_phnum; j++) {
		p_type = info->dlpi_phdr[j].p_type;
		type =  (p_type == PT_LOAD) ? "PT_LOAD" :
			(p_type == PT_DYNAMIC) ? "PT_DYNAMIC" :
			(p_type == PT_INTERP) ? "PT_INTERP" :
			(p_type == PT_NOTE) ? "PT_NOTE" :
			(p_type == PT_INTERP) ? "PT_INTERP" :
			(p_type == PT_PHDR) ? "PT_PHDR" :
			(p_type == PT_TLS) ? "PT_TLS" :
			(p_type == PT_GNU_EH_FRAME) ? "PT_GNU_EH_FRAME" :
			(p_type == PT_GNU_STACK) ? "PT_GNU_STACK" :
			(p_type == PT_GNU_RELRO) ? "PT_GNU_RELRO" : NULL;

		printf("    %2d: [%14p; memsz:%7lx] flags: 0x%x; ", j,
				(void *) (info->dlpi_addr + info->dlpi_phdr[j].p_vaddr),
				info->dlpi_phdr[j].p_memsz,
				info->dlpi_phdr[j].p_flags);
		if (type != NULL)
			printf("%s\n", type);
		else
			printf("[other (0x%x)]\n", p_type);
	}

	return 0;
}
#endif

int main(int argc, char *argv[])
{
#ifdef __linux__
	dl_iterate_phdr(callback, NULL);
#endif

	printf("__switch_fallback: %#p - %#p\n", &__switch_fallback_start, &__switch_fallback_end);
	for (void **p = (void **)&__switch_fallback_start; p < (void **)&__switch_fallback_end; p += 4) {
		printf("start=%p end=%p fallback=%p reserved=%p\n", p[0], p[1], p[2], p[3]);
	}

	exit(EXIT_SUCCESS);
}
