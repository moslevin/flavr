-----------------------------------------------------------------------------
flAVR (c) copyright 2014-2015 Funkenstein Software Consulting
See license.txt for details.

-----------------------------------------------------------------------------
To build flavr, simply type:

	make emulator

That's it.  flAVR has no external library dependencies, so it builds under
any POSIX environment such as Linux, OSX, or MinGW on Windows.  All you need
is a decent C99 compiler toolchain such as GCC or LLVM.

-----------------------------------------------------------------------------
Running flavr:

flAVR is a command-line program, accepting the following options

	--variant
			Specify a specific atmega variant (default: atmega328p).
			Supported CPU variants include:
				atmega328p,	atmega328, atmega168pa,	atmega168,
				atmega88pa, atmega88, atmega44pa, atmega44

	--freq
			Specify running frequency (in Hz)

	--hexfile
			Load program from hexfile

	--elffile
			Load program from ELF file

	--debug
			Start with integrated debugger

	--silent
			Run without banner

	--disasm
			Disassemble the loaded program (displayed to standard output),
			and exit

	--trace
			Run with single-step tracebuffer enabled.

	--mark3
			Run with integrated Mark3 kernel-aware debugging support

-----------------------------------------------------------------------------
Usage examples:

Run flAVR without interactive debugger, from a program stored in a hex file:

	flavr --hexfile <myprog.hex>

Run flAVR with interactive debugging enabled, from a program stored in an ELF file:

	flavr --elffile <myprog.elf>

Run flAVR with Mark3 RTOS kernel-aware debugging, interactive debugging, with
a program stored in an elf file

	flavr --elffile <myprog.elf> --debug --mark3

-----------------------------------------------------------------------------
