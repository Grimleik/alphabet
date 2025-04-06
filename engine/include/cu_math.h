#if !defined(CU_MATH_H)
/* ========================================================================
   Creator: Grimleik $
   TODO: Transform into C++ templates.
   ========================================================================*/
#define CU_MATH_H

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
    return (f32)sqrt(v.x * v.x + v.y * v.y);
}

static inline vec2_t vec2_normalize(vec2_t v)
{
    f32 length = (f32) sqrt(v.x * v.x + v.y * v.y);
    return {v.x / length, v.y / length};
}

static inline vec2_t vec2_mul_s(vec2_t v, f32 s)
{
    return {v.x * s, v.y * s};
}

static inline vec2_t vec2_add_v(vec2_t a, vec2_t b)
{
    return {a.x + b.x, a.y + b.y};
}

#endif
