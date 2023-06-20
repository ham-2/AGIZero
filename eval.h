#ifndef EVAL_INCLUDED
#define EVAL_INCLUDED

#include "position.h"

#include <atomic>

#include "constants.h"
#include "misc.h"

namespace AGI {
	using namespace std;

	extern atomic<long> node_count;

	extern bool limit_strength;
	extern int max_noise;
	extern int contempt;

	int end_eval(Position& board);

	int eval(Position& board, int depth, int alpha = EVAL_LOSS, int beta = EVAL_WIN);

	string eval_print(int eval);

	inline void inc_mate(int& eval) {
		if (eval > 10000) { eval++; }
		else if (eval < -10000) { eval--; }
	}

	inline void dec_mate(int& eval) {
		if (eval > 10000) { eval--; }
		else if (eval < -10000) { eval++; }
	}

	int add_noise(int& eval);
}

#endif