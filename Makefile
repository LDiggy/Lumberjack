firmware.bin: firmware.elf
	arm-none-eabi-objcopy -O binary firmware.elf firmware.bin

firmware.elf: main.o startup.o
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -T STM32F411RETX_FLASH.ld --specs=nosys.specs main.o startup.o -o firmware.elf

main.o: main.c
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -g -O0 -Wall -I./cmsis -c main.c -o main.o

startup.o: startup_stm32f411xe.s
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -g -O0 -Wall -c startup_stm32f411xe.s -o startup.o

.PHONY: clean
clean: 
	rm -f *.o *.bin *.elf

.PHONY: flash
flash: firmware.elf
	openocd -f board/st_nucleo_f4.cfg -c "program firmware.elf verify reset exit"