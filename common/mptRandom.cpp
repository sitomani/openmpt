/*
 * mptRandom.cpp
 * -------------
 * Purpose: PRNG
 * Notes  : (currently none)
 * Authors: OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */

#include "stdafx.h"

#include "mptRandom.h"

#include "Endianness.h"
#include "mptCRC.h"

#include <chrono>

#include <cmath>
#include <cstdlib>


OPENMPT_NAMESPACE_BEGIN


namespace mpt
{


template <typename T>
static T log2(T x)
{
	return std::log(x) / std::log(static_cast<T>(2));
}


static MPT_CONSTEXPRINLINE int lower_bound_entropy_bits(unsigned int x)
{
	return detail::lower_bound_entropy_bits(x);
}


template <typename T>
static MPT_CONSTEXPRINLINE bool is_mask(T x)
{
	static_assert(std::numeric_limits<T>::is_integer);
	typedef typename std::make_unsigned<T>::type unsigned_T;
	unsigned_T ux = static_cast<unsigned_T>(x);
	unsigned_T mask = 0;
	for(std::size_t bits = 0; bits <= (sizeof(unsigned_T) * 8); ++bits)
	{
		mask = (mask << 1) | 1u;
		if(ux == mask)
		{
			return true;
		}
	}
	return false;
}


namespace {
template <typename T> struct default_hash { };
template <> struct default_hash<uint8>  { typedef mpt::checksum::crc16 type; };
template <> struct default_hash<uint16> { typedef mpt::checksum::crc16 type; };
template <> struct default_hash<uint32> { typedef mpt::checksum::crc32c type; };
template <> struct default_hash<uint64> { typedef mpt::checksum::crc64_jones type; };
}

template <typename T>
static T generate_timeseed()
{
	// Note: CRC is actually not that good a choice here, but it is simple and we
	// already have an implementaion available. Better choices for mixing entropy
	// would be a hash function with proper avalanche characteristics or a block
	// or stream cipher with any pre-choosen random key and IV. The only aspect we
	// really need here is whitening of the bits.
	typename mpt::default_hash<T>::type hash;

	{
		uint64be time;
		time = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock().now().time_since_epoch()).count();
		std::byte bytes[sizeof(time)];
		std::memcpy(bytes, &time, sizeof(time));
		hash(std::begin(bytes), std::end(bytes));
	}

	{
		uint64be time;
		time = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock().now().time_since_epoch()).count();
		std::byte bytes[sizeof(time)];
		std::memcpy(bytes, &time, sizeof(time));
		hash(std::begin(bytes), std::end(bytes));
	}

	return static_cast<T>(hash.result());

}


sane_random_device::sane_random_device()
	: rd_reliable(false)
{
	try
	{
		prd = std::make_unique<std::random_device>();
		rd_reliable = ((*prd).entropy() > 0.0);
	} catch(mpt::out_of_memory e)
	{
		mpt::rethrow_out_of_memory(e);
	} catch(const std::exception &)
	{
		rd_reliable = false;	
	}
	if(!rd_reliable)
	{
		init_fallback();
	}
}

sane_random_device::sane_random_device(const std::string & token_)
	: token(token_)
	, rd_reliable(false)
{
	try
	{
		prd = std::make_unique<std::random_device>(token);
		rd_reliable = ((*prd).entropy() > 0.0);
	} catch(mpt::out_of_memory e)
	{
		mpt::rethrow_out_of_memory(e);
	} catch(const std::exception &)
	{
		rd_reliable = false;	
	}
	if(!rd_reliable)
	{
		init_fallback();
	}
}

void sane_random_device::init_fallback()
{
	if(!rd_fallback)
	{
		if(token.length() > 0)
		{
			uint64 seed_val = mpt::generate_timeseed<uint64>();
			std::vector<unsigned int> seeds;
			seeds.push_back(static_cast<uint32>(seed_val >> 32));
			seeds.push_back(static_cast<uint32>(seed_val >>  0));
			for(std::size_t i = 0; i < token.length(); ++i)
			{
				seeds.push_back(static_cast<unsigned int>(static_cast<unsigned char>(token[i])));
			}
			std::seed_seq seed(seeds.begin(), seeds.end());
			rd_fallback = std::make_unique<std::mt19937>(seed);
		} else
		{
			uint64 seed_val = mpt::generate_timeseed<uint64>();
			unsigned int seeds[2];
			seeds[0] = static_cast<uint32>(seed_val >> 32);
			seeds[1] = static_cast<uint32>(seed_val >>  0);
			std::seed_seq seed(seeds + 0, seeds + 2);
			rd_fallback = std::make_unique<std::mt19937>(seed);
		}
	}
}

sane_random_device::result_type sane_random_device::operator()()
{
	mpt::lock_guard<mpt::mutex> l(m);
	result_type result = 0;
	if(prd)
	{
		try
		{
			if constexpr(std::random_device::min() != 0 || !mpt::is_mask(std::random_device::max()))
			{ // insane std::random_device
				//  This implementation is not exactly uniformly distributed but good enough
				// for OpenMPT.
				constexpr double rd_min = static_cast<double>(std::random_device::min());
				constexpr double rd_max = static_cast<double>(std::random_device::max());
				constexpr double rd_range = rd_max - rd_min;
				constexpr double rd_size = rd_range + 1.0;
				const double rd_entropy = mpt::log2(rd_size);
				const int iterations = static_cast<int>(std::ceil(result_bits() / rd_entropy));
				double tmp = 0.0;
				for(int i = 0; i < iterations; ++i)
				{
					tmp = (tmp * rd_size) + (static_cast<double>((*prd)()) - rd_min);
				}
				double result_01 = std::floor(tmp / std::pow(rd_size, iterations));
				result = static_cast<result_type>(std::floor(result_01 * (static_cast<double>(max() - min()) + 1.0))) + min();
			} else
			{ // sane std::random_device
				result = 0;
				std::size_t rd_bits = mpt::lower_bound_entropy_bits(std::random_device::max());
				for(std::size_t entropy = 0; entropy < (sizeof(result_type) * 8); entropy += rd_bits)
				{
					if(rd_bits < (sizeof(result_type) * 8))
					{
						result = (result << rd_bits) | static_cast<result_type>((*prd)());
					} else
					{
						result = result | static_cast<result_type>((*prd)());
					}
				}
			}
		} catch(const std::exception &)
		{
			rd_reliable = false;
			init_fallback();
		}
	} else
	{
		rd_reliable = false;
	}
	if(!rd_reliable)
	{ // std::random_device is unreliable
		//  XOR the generated random number with more entropy from the time-seeded
		// PRNG.
		//  Note: This is safe even if the std::random_device itself is implemented
		// as a std::mt19937 PRNG because we are very likely using a different
		// seed.
		result ^= mpt::random<result_type>(*rd_fallback);
	}
	return result;
}

uint8 prng_random_device_time_seeder::generate_seed8()
{
	return mpt::generate_timeseed<uint8>();
}

uint16 prng_random_device_time_seeder::generate_seed16()
{
	return mpt::generate_timeseed<uint16>();
}

uint32 prng_random_device_time_seeder::generate_seed32()
{
	return mpt::generate_timeseed<uint32>();
}

uint64 prng_random_device_time_seeder::generate_seed64()
{
	return mpt::generate_timeseed<uint64>();
}

#if defined(MODPLUG_TRACKER) && !defined(MPT_BUILD_WINESUPPORT)

static mpt::random_device *g_rd = nullptr;
static mpt::thread_safe_prng<mpt::default_prng> *g_global_prng = nullptr;

void set_global_random_device(mpt::random_device *rd)
{
	g_rd = rd;
}

void set_global_prng(mpt::thread_safe_prng<mpt::default_prng> *prng)
{
	g_global_prng = prng;
}

mpt::random_device & global_random_device()
{
	return *g_rd;
}

mpt::thread_safe_prng<mpt::default_prng> & global_prng()
{
	return *g_global_prng;
}

#else

mpt::random_device & global_random_device()
{
	static mpt::random_device g_rd;
	return g_rd;
}

mpt::thread_safe_prng<mpt::default_prng> & global_prng()
{
	static mpt::thread_safe_prng<mpt::default_prng> g_global_prng(mpt::make_prng<mpt::default_prng>(global_random_device()));
	return g_global_prng;
}

#endif // MODPLUG_TRACKER && !MPT_BUILD_WINESUPPORT


} // namespace mpt


OPENMPT_NAMESPACE_END
