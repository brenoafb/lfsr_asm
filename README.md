# LFSR

To compile the assembly IO library
``` bash
nasm -felf asm_io.asm
```

To compile and run the program:

```bash
nasm -felf lfsr.asm
gcc -m32 -o lfsr main.c lfsr.o
./lfsr
```
