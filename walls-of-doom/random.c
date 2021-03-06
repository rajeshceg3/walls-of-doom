#include "random.h"

#include "constants.h"

#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAXIMUM_WORD_SIZE 32

/**
 * This is the successor to xorshift128+. It is the fastest full-period
 * generator passing BigCrush without systematic failures, but due to the
 * relatively short period it is acceptable only for applications with a
 * mild amount of parallelism; otherwise, use a xorshift1024* generator.
 *
 * Beside passing BigCrush, this generator passes the PractRand test suite
 * up to (and included) 16TB, with the exception of binary rank tests,
 * which fail due to the lowest bit being an LFSR; all other bits pass all
 * tests.
 *
 * Note that the generator uses a simulated rotate operation, which most C
 * compilers will turn into a single instruction. In Java, you can use
 * Long.rotateLeft(). In languages that do not make low-level rotation
 * instructions accessible xorshift128+ could be faster.
 *
 * The state must be seeded so that it is not zero everywhere. If you have
 * a 64-bit seed, we suggest to seed a splitmix64 generator and use its
 * output to fill s.
 */

uint64_t s[2] = {0x7c87b3fced63be76, 0x4ec3c3191d40a751};

uint64_t random_time_seed(void) {
  /* If tloc is a null pointer, no value is stored. */
  return (uint64_t)time(NULL);
}

void seed_random(void) { s[0] = random_time_seed(); }

uint64_t rotl(const uint64_t x, int k) { return (x << k) | (x >> (64 - k)); }

uint64_t next(void) {
  const uint64_t s0 = s[0];
  uint64_t s1 = s[1];
  const uint64_t result = s0 + s1;

  s1 ^= s0;
  s[0] = rotl(s0, 55) ^ s1 ^ (s1 << 14); /* a, b */
  s[1] = rotl(s1, 36);                   /* c */

  return result;
}

/**
 * This is the jump function for the generator. It is equivalent
 * to 2^64 calls to next(); it can be used to generate 2^64
 * non-overlapping subsequences for parallel computations.
 */
void jump(void) {
  static const uint64_t jump[] = {0xbeac0467eba5facb, 0xd86b048b86aa9922};

  uint64_t s0 = 0;
  uint64_t s1 = 0;
  size_t i;
  for (i = 0; i < sizeof(jump) / sizeof(*jump); i++) {
    int b;
    for (b = 0; b < 64; b++) {
      /* Was 1ULL, but ISO C90 does not allow it. */
      if (jump[i] & 1UL << b) {
        s0 ^= s[0];
        s1 ^= s[1];
      }
      next();
    }
  }

  s[0] = s0;
  s[1] = s1;
}

/**
 * Returns the next power of two bigger than the provided number.
 */
uint64_t find_next_power_of_two(uint64_t number) {
  uint64_t result = 1;
  while (number) {
    number >>= 1;
    result <<= 1;
  }
  return result;
}

/**
 * Returns a random number in the range [minimum, maximum].
 *
 * Always returns 0 if maximum < minimum.
 */
int random_integer(const int minimum, const int maximum) {
  /* Range should be a bigger type because the difference may overflow int. */
  const uint64_t range = maximum - minimum + 1;
  uint64_t next_power_of_two;
  uint64_t value;
  if (maximum < minimum) {
    return 0;
  }
  next_power_of_two = find_next_power_of_two(range);
  do {
    value = next() % next_power_of_two;
  } while (value >= range);
  /*
   * Varies from
   *   minimum + 0 == minimum
   * to
   *   minimum + (maximum - minimum + 1 - 1) == maximum
   */
  return minimum + value;
}

/**
 * Copies the first word of a random line of the file to the destination.
 */
void random_word(char *destination, const char *filename) {
  int read = '\0';
  int chosen_line;
  int current_line;
  const int line_count = file_line_count(filename);
  FILE *file;
  if (line_count > 0) {
    chosen_line = random_integer(0, line_count - 1);
    file = fopen(filename, "r");
    if (file) {
      current_line = 0;
      while (current_line != chosen_line && read != EOF) {
        read = fgetc(file);
        if (read == '\n') {
          current_line++;
        }
      }
      /* Got to the line we want to copy. */
      while ((read = fgetc(file)) != EOF) {
        if (isspace((char)read)) {
          break;
        }
        *destination++ = (char)read;
      }
      fclose(file);
    }
  }
  *destination = '\0';
}

/**
 * Writes a pseudorandom name to the destination.
 *
 * The destination should have at least 2 * MAXIMUM_WORD_SIZE bytes.
 */
void random_name(char *destination) {
  char buffer[MAXIMUM_WORD_SIZE];
  size_t first_word_size;
  random_word(buffer, ADJECTIVES_FILE_PATH);
  first_word_size = strlen(buffer);
  buffer[0] = toupper(buffer[0]);
  copy_string(destination, buffer, MAXIMUM_WORD_SIZE);
  random_word(buffer, NOUNS_FILE_PATH);
  buffer[0] = toupper(buffer[0]);
  copy_string(destination + first_word_size, buffer, MAXIMUM_WORD_SIZE);
}
