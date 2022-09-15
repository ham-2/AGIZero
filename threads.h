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
		void acquire_lock();
		void release_lock();
		void set_all(string fen);
		void show(int i);
		void do_move(Move m);
		void do_move(string ms);
	};

	extern Threadmgr thread_manager;
	extern const char* startpos_fen;
}

#endif