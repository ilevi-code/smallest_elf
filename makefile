final: make_elf main.text
	./make_elf

main.text: main
	objcopy -O binary --only-section=.text $< $@

main: main.S
	gcc -o main main.S -m32 -nostdlib -fno-pie -no-pie
