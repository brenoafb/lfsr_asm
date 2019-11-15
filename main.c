#include <stdio.h>
#include <stdint.h>
#include <time.h>

#define SEED 0x00cafeba
#define MASK 0x00ffffff  // ensure 24-bit

uint32_t generate(uint32_t *lfsr);
uint32_t asm_generate(uint32_t *lfsr);

int main(void) {
  uint32_t init = time(NULL) & MASK;
  // lfsr = time(NULL) & MASK;
  uint32_t lfsr = SEED;

  uint32_t rc = generate(&lfsr);
  printf("lfsr(c)=%09u (0x%08x)\n", lfsr, lfsr);
  lfsr = SEED;
  uint32_t ra = asm_generate(&lfsr);
  printf("lfsr(asm)=%09u (0x%08x)\n", lfsr, lfsr);
  printf("sd=%09u (0x%08x)\n", SEED, SEED);
  printf("rc=%09u (0x%08x)\n", rc, rc);
  printf("ra=%09u (0x%08x)\n", ra, ra);

  // for (int i = 0; i < 10; i++) {
  //   uint32_t r = generate();
  //   printf("%09u (0x%08x)\n", r, r);
  // }

  return 0;
}

uint32_t generate(uint32_t *lfsr) {
  uint32_t bit;
  uint32_t x = 0;
  for (int i = 0; i < 24; i++) {
    x <<= 1;
    /* polynomial: x^24 + x^23 + x^22 + x17 + 1 */
    bit = ((*lfsr >> 0) ^ (*lfsr >> 1) ^ (*lfsr >> 2) ^ (*lfsr >> 7)) & 0x1;
    // printf("%d", bit & 0x1);
    x |= *lfsr & 0x1;
    *lfsr = (*lfsr >> 1) | (bit << 23);
  }
  return x;
}
