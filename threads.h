#ifndef THREADS_INCLUDED
#define THREADS_INCLUDED

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <string>
#include <vector>
#include <thread>
#include <iostream>

#include "alphabeta.h"
#include "eval.h"
#include "position.h"
#include "movegen.h"

using namespace std;

class Thread {
public:
	Position* board;
	int id;
	int step;
	mutex m;
	condition_variable cv;
	atomic<bool> kill;
	thread* t;

	Thread(int id);
	~Thread();
};

class Threadmgr {
public:
	Position* board;
	mutex cout_lock;

	vector<Thread*> threads;
	atomic<bool> stop;
	atomic<int> depth;
	atomic<int> depth_max;
	condition_variable t_wait;

	void init();
	void add_thread();
	void del_thread();
	void set_threads(int n);
	void acquire_cout();
	void release_cout();
	void acquire_lock();
	void release_lock();
	void sync();
	void set_all(string fen);
	void show(int i);
	void do_move(Move m);
	void do_move(string ms);
	void undo_move(Move m);

	void gen();
	void test_eval();
	void test_see(string ms);

	inline Color get_color() { return board->get_side(); };
};

extern Threadmgr Threads;
extern const char* startpos_fen;

void lazy_smp(Thread* t);

#endif