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


int test_period(uint32_t seed, generator g1, generator g2);

double chi_squared(uint32_t *frequencies, uint32_t k, uint32_t expected);

double test_function(generator gen, uint32_t n, uint32_t k, uint32_t categories[],
		     double *time, uint32_t *period);

void test_routine(generator gen);

void print_categories(uint32_t *categories, uint32_t k);

int main(void) {
  printf("C cycle\n");
  test_routine(cycle);

  printf("\nASM cycle\n");
  test_routine(asm_cycle);

  return 0;
}

uint32_t cycle(uint32_t *lfsr) {
  uint32_t bit = ((*lfsr >> 0) ^ (*lfsr >> 1) ^ (*lfsr >> 3) ^ (*lfsr >> 4)) & 0x1;
  *lfsr = (*lfsr >> 1) | (bit << 23);
  return *lfsr;
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

void test_routine(generator gen) {
  uint32_t categories[N];
  uint32_t period = 0;
  double time = 0;
  double score = 0;

  score = test_function(gen, TOTAL, K, categories, &time, &period);
  printf("Period = %u\n", period);
  printf("Time = %lf\n", time);
  print_categories(categories, K);
  printf("Chi Squared = %lf\n", score);
}

