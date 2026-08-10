CC = arm-none-eabi-gcc
CFLAGS = -c -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -std=gnu11
LFLAGS = -nostdlib -T stm32_ls.ld -lgcc
MAPFLAGS = -Wl,-Map=app.map

all: app.elf

# 1. Automatically find all .c files in the current folder
SRCS = $(wildcard *.c)

# 2. Transform the list of .c files into a list of .o files
OBJS = $(SRCS:.c=.o)

# 3. Link rule: Dynamically uses the automatically generated $(OBJS) list
app.elf: $(OBJS)
	${CC} ${LFLAGS} $^ -o $@ ${MAPFLAGS}

# 4. Pattern rule: This single rule replaces all individual .c compilation rules!
%.o: %.c
	${CC} ${CFLAGS} $< -o $@

clean:
	rm -rf *.o *.map *.elf

load: app.elf
	openocd -f st_nucleo_f4.cfg -c "init" -c "reset init" -c "flash write_image erase app.elf" -c "verify_image app.elf" -c "reset run" -c "shutdown"
