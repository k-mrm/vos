#include <os3.h>

void NORETURN
main0(void)
{
	uart_init();
	seg_init();
	trap_init();
	pic8259a_init();
	acpi_init();

	ramdisk_init();
	fs_init();
	process_init();

	apmain();
}

void NORETURN
apmain(void)
{
	INTR_ENABLE;

	for (;;)
		HLT;
}
