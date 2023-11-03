# Smallest ELF

Compiling `printf("Hello, world\n");` takes too many bytes if you ask me (about ~15K).  
I felt like I could do better.  
I have achieved 97 bytes.  

## Optimization

### Take only what's necessary.
A normal minimal `.text` section will contain the following instructions
```asm
// syscall(SYS_write, STDOUT_FILENO, "Hello, world\n", 13);
mov	$msg, %ecx
mov	$len, %edx
mov	$1, %ebx
mov	$4, %eax
int	0x80

// syscall(SYS_exit, 0)
mov	$0, %ebx
mov	$1, %eax
int	0x80
```

### Smaller instructions
the `mov` of the `exit()` can be done using `xchg`, which is a smaller instruction

### Single segment
The ELF contains a single segment, which maps the whole file (ELF header, program header, and the following instructions\data)

### Unused or irrelevant fields
Some fields are unused in (program header physical address), and some are irrelevant to the loader (section headers).  
I encoded instructions into those fields to reduce the size of the non-header data (which is currently just the hello world string bytes)

### The loading address
The higher 2 bytes loading address itself contains instructions.  
The higher 2 bytes are encoded last due to little-endianity, so following fields can also contain instructions
