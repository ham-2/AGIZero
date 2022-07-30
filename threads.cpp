#include "threads.h"
#include <iostream>
namespace AGI {

	Threadmgr thread_manager = Threadmgr();
	const char* startpos_fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
	
	Thread::Thread() {
		board = new Position;
	}

	void Threadmgr::init() {
		threads.push_back(new Thread);
		set_all(startpos_fen);
	}

	void Threadmgr::set_all(string fen) {
		for (int i = 0; i < threads.size(); i++) {
			threads[i]->m.lock();
			threads[i]->board->set(fen);
			threads[i]->m.unlock();
		}
	}

	void Threadmgr::show(int i) {
		if (i < threads.size()) {
			threads[i]->board->show();
		}
	}

}