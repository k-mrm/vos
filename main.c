#include <os3.h>

void NORETURN
main0(struct multiboot2 *mb)
{
	uart_init();
	mb2_load(mb);
	seg_init();
	sysmem_reserved(0x0, 0x100000);
	reserve_kernel_area();
	trap_init();
	pic8259a_init();
	acpi_init();
	ramdisk_init();
	block_init();
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
