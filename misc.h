#ifndef MISC_INCLUDED
#define MISC_INCLUDED

#include <cstdint>

namespace AGI {

	class PRNG {
	private:
		uint64_t state;

	public:
		PRNG(uint64_t seed) { state = seed; }
		
		uint64_t get() {
			state ^= state << 13;
			state ^= state >> 7;
			state ^= state << 17;
			return state;
		}
	};

}

#endif
