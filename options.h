#ifndef OPTIONS_INCLUDED
#define OPTIONS_INCLUDED

#include <sstream>

#include "table.h"
#include "search.h"

namespace AGI {
	using namespace std;

	void print_option();

	void set_option(istringstream& ss);

}

#endif