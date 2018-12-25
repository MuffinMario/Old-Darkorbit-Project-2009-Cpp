#include "Defines.h"

static std::mt19937_64 rng_64;
static std::mt19937 rng;

#undef min
#undef max

template<typename T>
T random()
{
	static_assert(std::is_integral<T>::value, "T IS NOT AN INTEGRAL TYPE");
	if (std::is_same<T, uint64_t>::value || std::is_same<T, int64_t>::value)
	{
		//use 64 rng
		std::uniform_int_distribution<T> d(std::numeric_limits<T>::min(), std::numeric_limits<T>::max());
		return d(rng_64);
	}
	else
	{
		//use 32 rng
		std::uniform_int_distribution<T> d(std::numeric_limits<T>::min(), std::numeric_limits<T>::max());
		return d(rng);
	}
}
template<typename T>
T random(T max)
{
	static_assert(std::is_integral<T>::value, "T IS NOT AN INTEGRAL TYPE");
	if (std::is_same<T, uint64_t>::value || std::is_same<T, int64_t>::value)
	{
		//use 64 rng
		std::uniform_int_distribution<T> d(0, max);
		return d(rng_64);
	}
	else
	{
		//use 32 rng
		std::uniform_int_distribution<T> d(0, max);
		return d(rng);
	}
}

template<typename T>
T random(T min,T max)
{
	static_assert(std::is_integral<T>::value, "T IS NOT AN INTEGRAL TYPE");
	if (std::is_same<T, uint64_t>::value || std::is_same<T, int64_t>::value)
	{
		//use 64 rng
		std::uniform_int_distribution<T> d(min, max);
		return d(rng_64);
	}
	else
	{
		//use 32 rng
		std::uniform_int_distribution<T> d(min, max);
		return d(rng);
	}
}


template<typename T>
std::string to_string(T t) {
	return boost::lexical_cast<std::string>(t);
}
