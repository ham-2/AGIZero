#ifndef PRINTER_INCLUDED
#define PRINTER_INLCUDED

#include <iostream>
#include <mutex>

using namespace std;

namespace AGI {

	enum Sync { LOCK, UNLOCK };

#define sync_cout cout << LOCK
#define sync_endl endl << UNLOCK

	ostream& operator<< (ostream& os, Sync lock) {
		static mutex m;
		if (lock == LOCK) { m.lock(); }
		if (lock == UNLOCK) { m.unlock(); }

		return os;
	}

}

#endif