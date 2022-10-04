nasm -f bin -o boot.bin boot.asm
nasm -f bin -o loader.bin loader.asm
nasm -f bin -o kernel.bin kernel.asm
dd if=boot.bin of=boot.img bs=512 count=1 conv=notrunc #boot occupies 1 sector (MBR)
dd if=loader.bin of=boot.img bs=512 count=5 seek=1 conv=notrunc #loader occupies 5 sectors (seek=1, skip first sector)
dd if=kernel.bin of=boot.img bs=512 count=100 seek=6 conv=notrunc