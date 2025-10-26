#include <os3.h>
#include <x86.h>

#define PIC1         0x20
#define PIC2         0xa0

// disable PIC8259A
void
pic8259a_init(void)
{
	outb(PIC1 + 1, 0xff);
	outb(PIC2 + 1, 0xff);
}
