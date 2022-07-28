#ifndef OPTIONS_INCLUDED
#define OPTIONS_INCLUDED

#include <deque>
#include <string>
#include "table.h"
#include "search.h"

namespace AGI {
	using namespace std;

	void print_option();

	void set_threads(int new_threads);

	void set_option(deque<string> input);

	extern bool lichess_timing;

}

#endif