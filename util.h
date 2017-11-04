#ifndef __UTIL_H__
#define __UTIL_H__


#include <stdbool.h>
#include <SDL2/SDL.h>


/*
 * Determine if a point lands within a given rectangle.
 */
static inline bool
sb_point_in_rect(int32_t x, int32_t y, SDL_Rect *rect)
{
    return (x > rect->x && x < rect->x + rect->w &&
            y > rect->y && y < rect->y + rect->h);
}


/*
 * Calculate the center point of a rectangle.
 */
static inline void
sb_rect_center(SDL_Rect *rect, int *x, int *y)
{
    *x = rect->x + rect->w / 2;
    *y = rect->y + rect->h / 2;
}

/*
 * Find a random number in the closed interval [0, max].
 * Assumes 0 <= max <= RAND_MAX
 */
long random_at_most(long max);


/*
 * Find a random number in the closed interval [min, max].
 */
static inline long
random_range (long min, long max)
{
    return min + random_at_most(max - min);
}


#endif /* __UTIL_H__ */
