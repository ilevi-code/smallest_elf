.global _start

.text
_start:
    xor %ebx, %ebx
    int $0x80
message:
        .ascii  "Hello, world\n"

