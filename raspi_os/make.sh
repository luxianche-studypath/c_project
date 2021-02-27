./gcc-arm-none-eabi-10-2020-q4-major/bin/arm-none-eabi-gcc -mcpu=cortex-a7 -fpic -ffreestanding -c ./raspi-kernel/boot.S -o boot.o
./gcc-arm-none-eabi-10-2020-q4-major/bin/arm-none-eabi-gcc -mcpu=cortex-a7 -fpic -ffreestanding -std=gnu99 -c ./raspi-kernel/kernel.c -o kernel.o -O2 -Wall -Wextra
./gcc-arm-none-eabi-10-2020-q4-major/bin/arm-none-eabi-gcc -T ./raspi-kernel/linker.ld -o myos.elf -ffreestanding -O2 -nostdlib boot.o kernel.o

qemu-system-arm -m 256 -M raspi2 -serial stdio -kernel myos.elf