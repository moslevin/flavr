CC=gcc
CFLAGS=-g3 -Os

LD=gcc
LDFLAGS=-lpthread -lWs2_32

#----------------------------------------------------------------------------
OS := $(shell uname)
ifeq ($(OS), Linux)
else
  ifeq ($(OS), Darwin)
  else
    #presume windows
    CFLAGS+=-mno-ms-bitfields -DWINVER=WindowsXP -DUSE_WINDOWS
    LDFLAGS+=-lws2_32
  endif
endif
    
#----------------------------------------------------------------------------
SRC_DIR=./src/
AVR_CPU_SRC_DIR=$(SRC_DIR)avr_cpu/
CONFIG_SRC_DIR=$(SRC_DIR)config/
DEBUG_SRC_DIR=$(SRC_DIR)debug/
KERNEL_AWARE_SRC_DIR=$(SRC_DIR)kernel_aware/
LOADER_SRC_DIR=$(SRC_DIR)loader/
PERIPHERAL_SRC_DIR=$(SRC_DIR)peripheral/

#----------------------------------------------------------------------------
AVR_CPU_SRC_=       \
	avr_cpu.c       \
	avr_cpu_print.c \
	avr_disasm.c    \
	avr_interrupt.c \
	avr_io.c        \
	avr_opcodes.c   \
	avr_op_cycles.c \
	avr_op_decode.c \
	avr_op_size.c   \
    interrupt_callout.c \
    write_callout.c \


CONFIG_SRC_=        \
    options.c       \
    variant.c

DEBUG_SRC_=         \
    breakpoint.c    \
    code_profile.c  \
    debug_sym.c     \
    elf_print.c     \
    gdb_rsp.c       \
    interactive.c   \
    trace_buffer.c  \
    watchpoint.c

KERNEL_AWARE_SRC_=  \
    ka_interrupt.c  \
    ka_profile.c    \
    ka_thread.c     \
    ka_trace.c      \
    kernel_aware.c  \
    tlv_file.c

KERNEL_AWARE_NOSDL_SRC_= \
    ka_stubs.c

KERNEL_AWARE_SDL_SRC_= \
    ka_joystick.c   \
    ka_graphics.c   

LOADER_SRC_=        \
    avr_loader.c    \
    elf_process.c   \
    intel_hex.c     

PERIPHERAL_SRC_=    \
    mega_eeprom.c   \
    mega_eint.c     \
    mega_timer8.c   \
    mega_timer16.c  \
    mega_uart.c

ROOT_SRC_=           \
    flavr.c

#----------------------------------------------------------------------------
AVR_CPU_SRC=            $(addprefix $(AVR_CPU_SRC_DIR),$(AVR_CPU_SRC_))
CONFIG_SRC=             $(addprefix $(CONFIG_SRC_DIR),$(CONFIG_SRC_))
DEBUG_SRC=              $(addprefix $(DEBUG_SRC_DIR),$(DEBUG_SRC_))
KERNEL_AWARE_SRC=       $(addprefix $(KERNEL_AWARE_SRC_DIR),$(KERNEL_AWARE_SRC_))
KERNEL_AWARE_NOSDL_SRC= $(addprefix $(KERNEL_AWARE_SRC_DIR),$(KERNEL_AWARE_NOSDL_SRC_))
KERNEL_AWARE_SDL_SRC=   $(addprefix $(KERNEL_AWARE_SRC_DIR),$(KERNEL_AWARE_SDL_SRC_))
LOADER_SRC=             $(addprefix $(LOADER_SRC_DIR),$(LOADER_SRC_))
PERIPHERAL_SRC=         $(addprefix $(PERIPHERAL_SRC_DIR),$(PERIPHERAL_SRC_))
ROOT_SRC=               $(addprefix $(SRC_DIR),$(ROOT_SRC_))

#----------------------------------------------------------------------------
SRC_LIST=   \
    $(AVR_CPU_SRC)              \
    $(CONFIG_SRC)               \
    $(DEBUG_SRC)                \
    $(KERNEL_AWARE_SRC)         \
    $(LOADER_SRC)               \
    $(PERIPHERAL_SRC)           \
    $(ROOT_SRC)                 

SRC_LIST_SDL= \
    $(SRC_LIST)                 \
    $(KERNEL_AWARE_SDL_SRC)

SRC_LIST_NOSDL= \
    $(SRC_LIST)                 \
    $(KERNEL_AWARE_NOSDL_SRC)

SRC_LIST_ALL= \
    $(SRC_LIST)                 \
    $(KERNEL_AWARE_SDL_SRC)     \
    $(KERNEL_AWARE_NOSDL_SRC)

#----------------------------------------------------------------------------    
OBJECTS=$(SRC_LIST_ALL:%.c=%.o)

#----------------------------------------------------------------------------
INCLUDE_PATHS= \
    $(AVR_CPU_SRC_DIR)          \
    $(CONFIG_SRC_DIR)           \
    $(DEBUG_SRC_DIR)            \
    $(KERNEL_AWARE_SRC_DIR)     \
    $(LOADER_SRC_DIR)           \
    $(PERIPHERAL_SRC_DIR)       \
    $(ROOT_SRC_DIR)

CFLAGS+=$(addprefix -I, $(INCLUDE_PATHS))

#----------------------------------------------------------------------------

DOCFILE_TEX=./docs/latex/refman.tex
DOCFILE_PDF=./docs/latex/refman.pdf

printlist:
	echo $(AVR_CPU_SRC)

all: emulator emulator_sdl doc

install: emulator emulator_sdl
	cp -f ./flavr /usr/sbin
	cp -f ./flavr_sdl /usr/sbin

emulator: $(SRC_LIST_NOSDL:%.c=%.o)
	@echo [Linking] flavr
	@$(LD) -o flavr $(SRC_LIST_NOSDL:%.c=%.o) $(LDFLAGS)

emulator_sdl: $(SRC_LIST_SDL:%.c=%.o)
	@echo [Linking] flavr_sdl    	
	@$(LD) -o flavr_sdl $(SRC_LIST_SDL:%.c=%.o) -lSDL $(LDFLAGS)

$(AVR_CPU_SRC_DIR)%.o: $(AVR_CPU_SRC_DIR)%.c
	@echo [Compiling] $<
	@$(CC) $(CFLAGS) -c $< -o $@

$(CONFIG_SRC_DIR)%.o: $(CONFIG_SRC_DIR)%.c
	@echo [Compiling] $<
	@$(CC) $(CFLAGS) -c $< -o $@

$(DEBUG_SRC_DIR)%.o: $(DEBUG_SRC_DIR)%.c
	@echo [Compiling] $<
	@$(CC) $(CFLAGS) -c $< -o $@

$(KERNEL_AWARE_SRC_DIR)%.o: $(KERNEL_AWARE_SRC_DIR)%.c
	@echo [Compiling] $<
	@$(CC) $(CFLAGS) -c $< -o $@

$(LOADER_SRC_DIR)%.o: $(LOADER_SRC_DIR)%.c
	@echo [Compiling] $<
	@$(CC) $(CFLAGS) -c $< -o $@

$(PERIPHERAL_SRC_DIR)%.o: $(PERIPHERAL_SRC_DIR)%.c
	@echo [Compiling] $<
	@$(CC) $(CFLAGS) -c $< -o $@

$(ROOT_SRC_DIR)%.o: $(ROOT_SRC_DIR)%.c
	@echo [Compiling] $<
	@$(CC) $(CFLAGS) -c $< -o $@

doc: docfile_tex docfile_pdf $(DOCFILE_TEX) $(DOCFILE_PDF) 
	-cp $(DOCFILE_PDF) ./docs

docfile_pdf: $(DOCFILE_TEX)
	-cd ./docs/latex && pdflatex refman.tex
	-cd ./docs/latex && pdflatex refman.tex
	-cd ./docs/latex && pdflatex refman.tex

docfile_tex:
	-doxygen doxyfile	

clean:
	rm $(OBJECTS)
	rm flavr_sdl
	rm flavr

