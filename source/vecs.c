#include <math.h>
#include "vecs.h"

float vec2f_dist(v2f_t v1, v2f_t v2) {
    float a = fabs(v2.x - v1.x);
    float b = fabs(v2.y - v1.y);
    return sqrt(a*a + b*b);
}
