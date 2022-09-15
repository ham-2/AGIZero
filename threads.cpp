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

	void Threadmgr::acquire_lock() {
		for (int i = 0; i < threads.size(); i++) {
			threads[i]->m.lock();
		}
	}

	void Threadmgr::release_lock() {
		for (int i = 0; i < threads.size(); i++) {
			threads[i]->m.unlock();
		}
	}

	void Threadmgr::set_all(string fen) {
		for (int i = 0; i < threads.size(); i++) {
			threads[i]->board->set(fen);
		}
	}

	void Threadmgr::show(int i) {
		if (i < threads.size()) {
			threads[i]->board->show();
		}
	}

	void Threadmgr::do_move(Move m) {
		for (int i = 0; i < threads.size(); i++) {
			threads[i]->board->do_move(m);
		}
	}

	void Threadmgr::do_move(string ms) {
		Move m = threads[0]->board->parse_move(ms);
		for (int i = 0; i < threads.size(); i++) {
			threads[i]->board->do_move(m);
		}
	}

}