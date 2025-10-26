#include <os3.h>
#include <x86.h>

#define COM1    0x3f8
#define COM2    0x2f8

#define DATA  0x0
#define IER   0x1
#define DLL   0x0
#define DLH   0x1
#define IIR   0x2
#define FCR   0x2
#define LCR   0x3
#define MCR   0x4
#define LSR   0x5
#define MSR   0x6

#define DLAB  0x80

static int
uart_empty(void)
{
	return inb(COM1 + LSR) & 0x20;
}

static void
uart_send(char c)
{
        while (uart_empty() == 0)
                ;
        outb(COM1 + DATA, c);
}

void
uart_putc(char c)
{
        if (c == '\n')
                uart_send('\r');
        uart_send(c);
}

int
uart_write(char *buf, int n)
{
	uint c;

        for (uint i = 0; i < n && buf[i]; i++, c++)
                uart_putc(buf[i]);

	return c;
}

void
uart_init(void)
{
	u16 port = COM1;
        u16 div = 115200 / 115200;

        // Disable Interrupt
        outb(port + IER, 0x0);
        // Disable FIFO
        outb(port + FCR, 0x0);
        // Baudrate
        outb(port + LCR, 0x80);
        outb(port + DLL, div & 0xff);
        outb(port + DLH, (div >> 8) & 0xff);
        // RTS | DTR
        outb(port + MCR, 0x3);

        // 8n1
        outb(port + LCR, 0x3);
}
