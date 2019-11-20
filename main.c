#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define SEED 0x00cafeba
#define MASK 0x00ffffff  // ensure 24-bit
#define N    16
#define M    (1024*1024)

uint32_t generate(uint32_t lfsr);
uint32_t asm_generate(uint32_t lfsr);
int test(uint32_t seed);
int test_period(uint32_t seed);
double chi_squared(uint32_t *frequencies);

int main(void) {

  int period = test(SEED);
  printf("Period=%d\n", period);

  uint32_t categories[N];
  memset(categories, 0, sizeof(uint32_t) * N);
  printf("C code\n");
  uint32_t lfsr = SEED & MASK;

  clock_t c_clock = clock();
  for (int i = 0; i < N*M; i++) {
    uint32_t r = generate(lfsr);
    categories[r/M]++;
    lfsr = r;
  }
  c_clock = clock() - c_clock;
  double c_time = ((double) c_clock) / CLOCKS_PER_SEC;
  printf("C time: %f\n", c_time);

  printf("Generation intervals:\n");
  for (int i = 0; i < N; i++) {
    printf("[%d,%d]: %d\n", i, i+1, categories[i]);
  }

  memset(categories, 0, sizeof(uint32_t) * N);
  printf("\nASM code\n");
  lfsr = SEED & MASK;
  clock_t asm_clock = clock();
  for (int i = 0; i < N*M; i++) {
    uint32_t r = asm_generate(lfsr);
    categories[r/M]++;
    lfsr = r;
  }
  asm_clock = clock() - asm_clock;
  double asm_time = ((double) asm_clock) / CLOCKS_PER_SEC;
  printf("ASM time: %f\n", asm_time);

  printf("Generation intervals:\n");
  for (int i = 0; i < N; i++) {
    printf("[%d,%d]: %d\n", i, i+1, categories[i]);
  }

  printf("Chi Squared = %lf\n", chi_squared(categories));

  return 0;
}

uint32_t generate(uint32_t lfsr) {
  /* polynomial: x^24 + x^23 + x^21 + x^20 + 1 */
  uint32_t bit = ((lfsr >> 0) ^ (lfsr >> 1) ^ (lfsr >> 3) ^ (lfsr >> 4)) & 0x1;
  return ((lfsr >> 1) | (bit << 23));
}

int test(uint32_t seed) {
  uint32_t lfsr = seed & MASK;
  int period = 0;
  do {
    uint32_t cgen = generate(lfsr);
    uint32_t asmgen = asm_generate(lfsr);
    if (cgen != asmgen) {
      printf("\nError at i=%d; c=0x%x (%d), asm=0x%x (%d)\n", period, cgen, cgen, asmgen, asmgen);
      return period;
    } else {
      period++;
      lfsr = cgen;
    }
  } while (lfsr != (seed & MASK));

  printf("\nPass\n");
  return period;
}

int test_period(uint32_t seed) {
  uint32_t lfsr = seed & MASK;
  int period = 0;
  do {
    lfsr = generate(lfsr);
    period++;
  } while (lfsr != (seed & MASK));
  return period;
}

double chi_squared(uint32_t *frequencies) {
	uint32_t accumulated_sum = 0;

	for (uint32_t i = 0;i < N;i++) {
		accumulated_sum += ((frequencies[i] - M) * (frequencies[i] - M));
	}

	return accumulated_sum / M;
}
