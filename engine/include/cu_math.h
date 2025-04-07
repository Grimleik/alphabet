#if !defined(CU_MATH_H)
/* ========================================================================
   Creator: Grimleik $
   TODO: Transform into C++ templates.
   ========================================================================*/
#define CU_MATH_H

#define SQUARE(x) ((x) * (x))

template <typename T>
struct vec2
{
	T x, y;

	vec2() : x(0), y(0) {}
	vec2(T x, T y) : x(x), y(y) {}

	T length2() const
	{
		return x * x + y * y;
	}

	T length() const
	{
		return static_cast<T>(sqrt(x * x + y * y));
	}

	vec2 normalize() const
	{
		T len = length();
		return {x / len, y / len};
	}

	vec2 operator*(T scalar) const
	{
		return {x * scalar, y * scalar};
	}

	vec2 operator+(const vec2 &other) const
	{
		return {x + other.x, y + other.y};
	}
};

template <typename T>
struct vec3
{
	T x, y, z;

	vec3() : x(0), y(0), z(0) {}
	vec3(T x, T y, T z) : x(x), y(y), z(z) {}

	T length2() const
	{
		return x * x + y * y + z * z;
	}

	T length() const
	{
		return static_cast<T>(sqrt(x * x + y * y + z * z));
	}

	vec3 normalize() const
	{
		T len = length();
		return {x / len, y / len, z / len};
	}

	vec3 operator*(T scalar) const
	{
		return {x * scalar, y * scalar, z * scalar};
	}

	vec3 operator+(const vec3 &other) const
	{
		return {x + other.x, y + other.y, z + other.z};
	}
};

template <typename T>
struct vec4
{
	T x, y, z, w;

	vec4() : x(0), y(0), z(0), w(0) {}
	vec4(T x, T y, T z, T w) : x(x), y(y), z(z), w(w) {}

	T length2() const
	{
		return x * x + y * y + z * z + w * w;
	}

	T length() const
	{
		return static_cast<T>(sqrt(x * x + y * y + z * z + w * w));
	}

	vec4 normalize() const
	{
		T len = length();
		return {x / len, y / len, z / len, w / len};
	}

	vec4 operator*(T scalar) const
	{
		return {x * scalar, y * scalar, z * scalar, w * scalar};
	}

	vec4 operator+(const vec4 &other) const
	{
		return {x + other.x, y + other.y, z + other.z, w + other.w};
	}
};

typedef vec2<f32> vec2f;
typedef vec2<i32> vec2i;
typedef vec3<f32> vec3f;
typedef vec3<i32> vec3i;
typedef vec4<f32> vec4f;
typedef vec4<i32> vec4i;

#endif
