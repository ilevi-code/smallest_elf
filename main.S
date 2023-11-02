.global _start

.text
_start:
    add $4, %al                # system call 1 is write
    inc %ebx
    jmp load_message
go_back:
    pop %ecx
    mov $13, %dl               # number of bytes
    int $0x80

    # exit(0)
    xor %eax, %eax
    inc %eax
    xor %ebx, %ebx
    int $0x80

load_message:
    call go_back
message:
        .ascii  "Hello, world\n"

