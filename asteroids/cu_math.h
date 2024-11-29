#if !defined(CU_MATH_H)
/* ========================================================================
   Creator: Grimleik $
   ========================================================================*/
#define CU_MATH_H

#include <math.h>

typedef struct vec2_t vec2_t;
struct vec2_t 
{
    f32 x, y;    
};

static inline f32 vec2_length2(vec2_t v)
{
    return v.x * v.x + v.y * v.y;
}

static inline f32 vec2_length(vec2_t v)
{
    return sqrt(v.x * v.x + v.y * v.y);
}

static inline vec2_t vec2_normalize(vec2_t v)
{
    float length = sqrt(v.x * v.x + v.y * v.y);
    return (vec2_t){v.x / length, v.y / length};
}

static inline vec2_t vec2_mul_s(vec2_t v, f32 s)
{
    return (vec2_t){v.x * s, v.y * s};
}

static inline vec2_t vec2_add_v(vec2_t a, vec2_t b)
{
    return (vec2_t){a.x + b.x, a.y + b.y};
}

#endif
