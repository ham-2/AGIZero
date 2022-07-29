#ifndef PRINTER_INCLUDED
#define PRINTER_INCLUDED

#include <iostream>
#include <mutex>

using namespace std;

namespace AGI {

	enum Sync { LOCK, UNLOCK };

#define sync_cout cout << LOCK
#define sync_endl endl << UNLOCK

	ostream& operator<< (ostream& os, Sync lock);

}

#endif