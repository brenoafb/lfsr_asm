#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define SEED 0x00cafeba
#define MASK 0x00ffffff  // ensure 24-bit
#define N    16
#define M    (1024*1024)

uint32_t cycle(uint32_t lfsr);
extern uint32_t asm_cycle(uint32_t lfsr);

uint32_t generate(uint32_t *lfsr);
extern uint32_t asm_generate(uint32_t *lfsr);

int test(uint32_t seed);
int test_period(uint32_t seed);
int test_asm_period(uint32_t seed);

double chi_squared(uint32_t *frequencies, uint32_t n, uint32_t expected);

int main(void) {

  int period = test_period(SEED);
  printf("Period=%d\n", period);


  uint32_t categories[N];
  memset(categories, 0, sizeof(uint32_t) * N);
  printf("C code\n");
  uint32_t lfsr = SEED & MASK;
  clock_t c_clock = clock();
  for (int i = 0; i < N*M; i++) {
    uint32_t r = generate(&lfsr);
    categories[r/M]++;
  }

  c_clock = clock() - c_clock;
  double c_time = ((double) c_clock) / CLOCKS_PER_SEC;
  printf("C time: %f\n", c_time);

  printf("Generation intervals:\n");
  for (int i = 0; i < N; i++) {
    printf("[%d,%d]: %d\n", i, i+1, categories[i]);
  }
  printf("Chi Squared = %lf\n", chi_squared(categories, N, M));

  memset(categories, 0, sizeof(uint32_t) * N);
  printf("\nasm code\n");
  lfsr = SEED & MASK;
  clock_t asm_clock = clock();
  for (int i = 0; i < N*M; i++) {
    uint32_t r = asm_generate(&lfsr);
    categories[r/M]++;
  }

  asm_clock = clock() - asm_clock;
  double asm_time = ((double) asm_clock) / CLOCKS_PER_SEC;
  printf("asm time: %f\n", asm_time);

  printf("Generation intervals:\n");
  for (int i = 0; i < N; i++) {
    printf("[%d,%d]: %d\n", i, i+1, categories[i]);
  }
  printf("Chi Squared = %lf\n", chi_squared(categories, N, M));


  return 0;
}

uint32_t cycle(uint32_t lfsr) {
  uint32_t bit = ((lfsr >> 0) ^ (lfsr >> 1) ^ (lfsr >> 3) ^ (lfsr >> 4)) & 0x1;
  return (lfsr >> 1) | (bit << 23);
}

uint32_t generate(uint32_t *lfsr) {
  uint32_t bit;
  uint32_t x = 0;
  for (int i = 0; i < 24; i++) {
    x <<= 1;
    /* polynomial: x^24 + x^23 + x^22 + x17 + 1 */
    bit = ((*lfsr >> 0) ^ (*lfsr >> 1) ^ (*lfsr >> 3) ^ (*lfsr >> 4)) & 0x1;
    x |= *lfsr & 0x1;
    *lfsr = (*lfsr >> 1) | (bit << 23);
  }
  return x;
}

int test(uint32_t seed) {
  uint32_t lfsr = seed & MASK;
  int period = 0;
  do {
    uint32_t cgen = cycle(lfsr);
    uint32_t asmgen = asm_cycle(lfsr);
    if (cgen != asmgen) {
      printf("Error at i=%d; c=0x%x (%d), asm=0x%x (%d)\n", period, cgen, cgen, asmgen, asmgen);
      return period;
    } else {
      period++;
      lfsr = cgen;
    }
  } while (lfsr != (seed & MASK));

  printf("Pass\n");
  return period;
}

int test_period(uint32_t seed) {
  uint32_t lfsr = seed & MASK;
  int period = 0;
  do {
    lfsr = cycle(lfsr);
    period++;
  } while (lfsr != (seed & MASK));
  return period;
}

int test_asm_period(uint32_t seed) {
  uint32_t lfsr = seed & MASK;
  int period = 0;
  do {
    lfsr = asm_cycle(lfsr);
    period++;
  } while (lfsr != (seed & MASK));
  return period;
}

double chi_squared(uint32_t *frequencies, uint32_t n, uint32_t expected) {
  uint32_t accumulated_sum = 0;

  for (uint32_t i = 0; i < n; i++) {
    accumulated_sum += ((frequencies[i] - expected) * (frequencies[i] - expected));
  }

  return accumulated_sum / expected;
}
