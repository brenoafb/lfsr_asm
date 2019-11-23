#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define SEED 0x00cafeba
#define MASK 0x00ffffff  // ensure 24-bit
#define N    16
#define M    (1024*1024)
#define TOTAL (16*1024*1024)
#define K     16

typedef uint32_t (*generator)(uint32_t *);

uint32_t cycle(uint32_t *lfsr);
extern uint32_t asm_cycle(uint32_t *lfsr);

uint32_t generate(uint32_t *lfsr);
extern uint32_t asm_generate(uint32_t *lfsr);

int test_period(uint32_t seed, generator g1, generator g2);

double chi_squared(uint32_t *frequencies, uint32_t k, uint32_t expected);

double test_function(generator gen, uint32_t n, uint32_t k, uint32_t categories[],
		     double *time, uint32_t *period);

void print_categories(uint32_t *categories, uint32_t k);

int main(void) {

  uint32_t categories[N];
  uint32_t period = 0;
  double time = 0;
  double score = 0;

  printf("C cycle\n");
  score = test_function(cycle, TOTAL, K, categories, &time, &period);
  print_categories(categories, K);
  printf("Chi Squared = %lf\n", score);

  printf("\nASM cycle\n");
  score = test_function(asm_cycle, TOTAL, K, categories, &time, &period);
  print_categories(categories, K);
  printf("Chi Squared = %lf\n", score);

  printf("\nC generate\n");
  score = test_function(generate, TOTAL, K, categories, &time, &period);
  print_categories(categories, K);
  printf("Chi Squared = %lf\n", score);

  printf("\nASM generate\n");
  score = test_function(asm_generate, TOTAL, K, categories, &time, &period);
  print_categories(categories, K);
  printf("Chi Squared = %lf\n", score);
  return 0;
}

uint32_t cycle(uint32_t *lfsr) {
  uint32_t bit = ((*lfsr >> 0) ^ (*lfsr >> 1) ^ (*lfsr >> 3) ^ (*lfsr >> 4)) & 0x1;
  *lfsr = (*lfsr >> 1) | (bit << 23);
  return *lfsr;
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

int test_period(uint32_t seed, generator g1, generator g2) {
  uint32_t lfsr_1 = seed & MASK;
  uint32_t lfsr_2 = seed & MASK;
  int period = 0;
  do {
    uint32_t r1 = g1(&lfsr_1);
    uint32_t r2 = g2(&lfsr_2);
    if ((r1 != r2) || (lfsr_1 != lfsr_2)) {
      printf("Error at i=%d; r1=0x%x (%d), r2=0x%x (%d)\n", period, r1, r1, r2, r2);
      return period;
    } else {
      period++;
    }
  } while ((lfsr_1 != (seed & MASK)) && (lfsr_2 != (SEED & MASK)));

  printf("Pass\n");
  return period;
}

double chi_squared(uint32_t *frequencies, uint32_t k, uint32_t expected) {
  uint32_t accumulated_sum = 0;

  for (uint32_t i = 0; i < k; i++) {
    accumulated_sum += ((frequencies[i] - expected) * (frequencies[i] - expected));
  }

  return accumulated_sum / expected;
}

double test_function(generator gen, uint32_t n, uint32_t k, uint32_t categories[],
		     double *time, uint32_t *period) {
  if (period) *period = test_period(SEED, gen, gen);
  uint32_t m = n/k;
  memset(categories, 0, sizeof(uint32_t) * N);
  uint32_t lfsr = SEED & MASK;
  clock_t c = clock();
  for (int i = 0; i < N*M; i++) {
    uint32_t r = gen(&lfsr);
    categories[r/m]++;
  }

  c = clock() - c;
  if (time) *time = ((double) c) / CLOCKS_PER_SEC;

  return chi_squared(categories, k, m);
}

void print_categories(uint32_t *categories, uint32_t k) {
  for (uint32_t i = 0; i < k; i++) {
    printf("%d: %d\n", i, categories[i]);
  }
}
