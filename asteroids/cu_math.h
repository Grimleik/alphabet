#if !defined(CU_MATH_H)
/* ========================================================================
   Creator: Grimleik $
   ========================================================================*/
#define CU_MATH_H

typedef struct vec2_t vec2_t;
struct vec2_t 
{
    f32 x, y;    
};

f32 vec2_length2(vec2_t v)
{
    return v.x * v.x + v.y * v.y;
}

vec2_t vec2_normalize(vec2_t v)
{
    float length = sqrt(v.x * v.x + v.y * v.y);
    return (vec2_t){v.x / length, v.y / length};
}

#endif
