#include "printer.h"

namespace AGI {

	ostream& operator<< (ostream& os, Sync lock) {
		static mutex m;
		if (lock == LOCK) { m.lock(); }
		if (lock == UNLOCK) { m.unlock(); }

		return os;
	}

}