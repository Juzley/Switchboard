#include <stdlib.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include "util.h"


/*
 * See util.h for details.
 */
long
random_at_most (long max) {
    // max <= RAND_MAX < ULONG_MAX, so this is okay.
    unsigned long num_bins = (unsigned long)max + 1;
    unsigned long num_rand = (unsigned long)RAND_MAX + 1;
    unsigned long bin_size = num_rand / num_bins;
    unsigned long defect   = num_rand % num_bins;
    long x;

    do {
        x = random();
        // This is carefully written not to overflow
    } while (num_rand - defect <= (unsigned long)x);

    // Truncated division is intentional
    return x/bin_size;
}
