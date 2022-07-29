#include "threads.h"

namespace AGI {

	Threadmgr thread_manager = Threadmgr();
	

	void Threadmgr::init() {
		threads.push_back(new Thread);
	}

	void Threadmgr::set_all(string fen) {
		for (int i = 0; i < threads.size(); i++) {
			threads[i]->m.lock();
			threads[i]->board->set(fen);
			threads[i]->m.unlock();
		}
	}

}