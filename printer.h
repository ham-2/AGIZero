#ifndef PRINTER_INCLUDED
#define PRINTER_INCLUDED

#include <atomic>
#include <condition_variable>
#include <iostream>
#include <mutex>

using namespace std;

namespace AGI {

	void printer(float time, atomic<bool>* stop, condition_variable* cv, float max_time, bool force_time);

}

#endif