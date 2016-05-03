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
	code_profile.c \
	debug_sym.c \
	elf_print.c \
	elf_process.c \
	intel_hex.c \
	interrupt_callout.c \
	kernel_aware.c \
    ka_interrupt.c \
    ka_thread.c \
    ka_profile.c \
    ka_trace.c \
	trace_buffer.c \
	watchpoint.c \
	breakpoint.c \
	options.c \
	interactive.c \
	mega_uart.c \
	mega_eint.c \
	mega_eeprom.c \
	mega_timer16.c \
	mega_timer8.c \
	flavr.c \
	variant.c \
    write_callout.c \
	tlv_file.c \
	gdb_rsp.c

SRC_LIST_SDL=$(SRC_LIST_EMULATOR) ka_graphics.c ka_joystick.c
SRC_LIST_NOSDL=$(SRC_LIST_EMULATOR) ka_stubs.c

DOCFILE_TEX=./docs/latex/refman.tex
DOCFILE_PDF=./docs/latex/refman.pdf

printlist:
	echo $(SRC_LIST_DISASM)

all: emulator emulator_sdl doc

install: emulator emulator_sdl
	cp -f ./flavr /usr/sbin
	cp -f ./flavr_sdl /usr/sbin

emulator_win: $(SRC_LIST_NOSDL:%.c=%.o)
	$(CC) -g3 -O3 -o flavr $(SRC_LIST_NOSDL:%.c=%.o) -lpthread -lws2_32

emulator_sdl_win: $(SRC_LIST_SDL:%.c=%.o)
	$(CC) -g3 -O3 -o flavr_sdl $(SRC_LIST_SDL:%.c=%.o) -lSDL -lpthread -lws2_32

emulator: $(SRC_LIST_NOSDL:%.c=%.o)
	$(CC) -g3 -O3 -o flavr $(SRC_LIST_NOSDL:%.c=%.o) -lpthread

emulator_sdl: $(SRC_LIST_SDL:%.c=%.o)
	$(CC) -g3 -O3 -o flavr_sdl $(SRC_LIST_SDL:%.c=%.o) -lSDL -lpthread

%.o : %.c
	$(CC) $< -c -g3 -O3

doc: docfile_tex docfile_pdf $(DOCFILE_TEX) $(DOCFILE_PDF) 
	-cp $(DOCFILE_PDF) ./docs

docfile_pdf: $(DOCFILE_TEX)
	-cd ./docs/latex && pdflatex refman.tex
	-cd ./docs/latex && pdflatex refman.tex
	-cd ./docs/latex && pdflatex refman.tex

docfile_tex:
	-doxygen doxyfile	

clean:
	-rm *.o
	-rm flavr
	-rm flavr.exe
	-rm flavr_sdl
	-rm flavr_sdl.exe
	-rm -r ./docs/*

