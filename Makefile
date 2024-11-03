ASM		= ../aslc86k/aslc86k
SRCS	= core.s header.s soc.s ui.s uM23.s	


all: $(SRCS) Makefile
	$(ASM) -I. uM23.s
