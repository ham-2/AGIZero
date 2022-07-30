#ifndef THREADS_INCLUDED
#define THREADS_INCLUDED

#include <condition_variable>
#include <mutex>
#include <string>
#include <vector>

#include "position.h"

using namespace std;

namespace AGI {

	class Thread {
	public:
		Position* board;
		mutex m;
		condition_variable cv;

		Thread();
	};

	class Threadmgr {
	public:
		vector<Thread*> threads;

		void init();
		void set_all(string fen);
		void show(int i);
	};

	extern Threadmgr thread_manager;
	extern const char* startpos_fen;
}

#endif