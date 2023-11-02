.global _start

.text
_start:
    // instruction already executed by patching unused field in elf header:
    // add $4, %al                # system call 4 is write
  // callw here
// here:
//     pop %eax
//     add $0x60, %al
    inc %ebx
    mov $13, %dl               # number of bytes
    int $0x80

    # exit(0)
    xor %eax, %eax
    inc %eax
    xor %ebx, %ebx
    int $0x80
message:
        .ascii  "Hello, world\n"

