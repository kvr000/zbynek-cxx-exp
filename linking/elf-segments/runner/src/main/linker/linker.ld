SECTIONS
{
	.__switch_fallback :
	{
		. = ALIGN(16);
		PROVIDE_HIDDEN (__switch_fallback_start = .);
		*(.__switch_fallback)
		PROVIDE_HIDDEN (__switch_fallback_end = .);
	}
}
