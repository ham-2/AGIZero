#ifndef SEARCH_INCLUDED
#define SEARCH_INCLUDED

#include <cmath>
#include <sstream>

#include "position.h"
#include "threads.h"

namespace AGI {

	using namespace std;
	using namespace AGI;

	void get_time(istringstream& ss, Color c, float& time, float& max_time, bool& force_time, int& max_ply);

	void clear1ply(Position& board);

	void search_start(Thread* t, float time, float max_time, bool force_time, int max_ply);

	extern bool ponder;
	extern bool lichess_timing;
	extern atomic<bool> ponder_continue;

}

#endif
