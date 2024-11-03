CC = arm-none-eabi-gcc
LD = arm-none-eabi-gcc
OBJCOPY = arm-none-eabi-objcopy
NM = arm-none-eabi-nm
LKR = linker.lkr
OPT = -Os -g -ggdb3
CCFLAGS = $(OPT) -mthumb -mcpu=cortex-m23 -I. -ffunction-sections -fdata-sections -fomit-frame-pointer -flto
LDFLAGS = $(OPT) -Wl,--gc-sections -Wl,-T $(LKR) -mthumb -mcpu=cortex-m23 -flto


APP = code
OBJS = entry.o main.o printf.o

all: $(APP).bin

$(APP).bin:	$(OBJS) $(LKR)
	$(LD) -o $(APP).elf $(LDFLAGS) $(OBJS)
	$(OBJCOPY) -I elf32-littlearm -O binary $(APP).elf $(APP).bin -j.data -j.text -j.rodata -j.vec

%.o : %.c Makefile
	$(CC) $(CCFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(APP).bin $(APP).elf $(APP).hex

