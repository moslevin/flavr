CC=gcc
SRC_LIST_DISASM=\
	avr_cpu.c \
	avr_cpu_print.c \
	avr_disasm.c \
	avr_io.c \
	avr_loader.c \
	avr_op_decode.c \
	avr_op_size.c \
	avr_opcodes.c \
	avr_op_cycles.c \
	intel_hex.c \
	disasm.c

SRC_LIST_EMULATOR=\
	avr_cpu.c \
	avr_cpu_print.c \
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
	flavr.c

printlist:
	echo $(SRC_LIST_DISASM)

all: emulator disasm

emulator: $(SRC_LIST_EMULATOR:%.c=%.o)
	$(CC) -g3 -o emulator.exe $(SRC_LIST_EMULATOR:%.c=%.o)


disasm: $(SRC_LIST_DISASM:%.c=%.o)
	$(CC) -g3 -o disasm.exe $(SRC_LIST_DISASM:%.c=%.o)

%.o : %.c
	$(CC) $< -c -g3

clean:
	rm *.o
