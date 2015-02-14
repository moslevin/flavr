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
	variant.c \
	write_callout.c

DOCFILE_TEX=./docs/latex/refman.tex
DOCFILE_PDF=./docs/latex/refman.pdf

printlist:
	echo $(SRC_LIST_DISASM)

all: emulator doc

emulator: $(SRC_LIST_EMULATOR:%.c=%.o)
	$(CC) -g0 -O3 -o flavr $(SRC_LIST_EMULATOR:%.c=%.o)

%.o : %.c
	$(CC) $< -c -g0 -O3

clean:
	-rm *.o
	-rm flavr
	-rm flavr.exe
	-rm -r ./docs/*

doc: docfile_tex docfile_pdf $(DOCFILE_TEX) $(DOCFILE_PDF) 
	-cp $(DOCFILE_PDF) ./docs

docfile_pdf: $(DOCFILE_TEX)
	-cd ./docs/latex && pdflatex refman.tex
	-cd ./docs/latex && pdflatex refman.tex
	-cd ./docs/latex && pdflatex refman.tex

docfile_tex:
	-doxygen doxyfile
	
