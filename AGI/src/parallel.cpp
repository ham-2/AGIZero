#include "parallel.h"

using namespace Stockfish;
using namespace std;

namespace AGI {
	
	int SEARCH_THREADS = SEARCH_THREADS_DEFAULT;
	condition_variable t_wait;
	std::mutex t_mutex;
	threadmgr Threads;
	

	void threadmgr::sync() {
		for (int i = 0; i < SEARCH_THREADS; i++) {
			Threads.vm[i]->lock();
		}
		for (int i = 0; i < SEARCH_THREADS; i++) {
			Threads.vm[i]->unlock();
		}
	}

	void lazy_smp(Position* board, int i) {
		int step = Primes[i];
		SearchStack* ss = new SearchStack;
		ss->thread_id = i;
		int currdepth;
		int probed;

		while (!*(Threads.kill[i])) {
			unique_lock<mutex> m(*Threads.vm[i]);
			t_wait.wait(m);
			
			ss->init();
			while (!Threads.stop) {
				currdepth = Threads.depth + 1;
				ss->set_depth(currdepth);
				alpha_beta(*board, &Threads.stop, ss, currdepth, &probed);
			}
		}
		*Threads.kill[i] = false;
		return;
	}

}


