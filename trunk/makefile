CC=gcc

SRC_LIST_EMULATOR=\
	avr_cpu.c \
	avr_cpu_print.c \
	avr_interrupt.c \
	avr_io.c \
	avr_loader.c \
	avr_op_decode.c \
	avr_op_size.c \
	avr_opcodes.c \
	avr_op_cycles.c \
	avr_disasm.c \
	intel_hex.c \
	trace_buffer.c \
	watchpoint.c \
	breakpoint.c \
	options.c \
	interactive.c \
	mega_uart.c \
	mega_eint.c \
	mega_timer16.c \
	mega_timer8.c \
	flavr.c \
	variant.c

printlist:
	echo $(SRC_LIST_DISASM)

all: emulator

emulator: $(SRC_LIST_EMULATOR:%.c=%.o)
	$(CC) -g0 -O3 -o flavr $(SRC_LIST_EMULATOR:%.c=%.o)

%.o : %.c
	$(CC) $< -c -g0 -O3

clean:
	rm *.o
	rm flavr.exe
