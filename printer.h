#ifndef PRINTER_INCLUDED
#define PRINTER_INCLUDED

#include <atomic>
#include <condition_variable>
#include <iostream>
#include <mutex>

#include "board.h"

using namespace std;

namespace AGI {

	bool compare(pair<Move, int> a, pair<Move, int> b);

	void printer(float time, atomic<bool>* stop, condition_variable* cv, float max_time, bool force_time);

}

#endif