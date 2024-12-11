#ifndef EVAL_INCLUDED
#define EVAL_INCLUDED

#include <iostream>
#include <cmath>
#include <string>
#include <cstdlib>
#include <atomic>

#include "constants.h"
#include "misc.h"
#include "material.h"
#include "movegen.h"
#include "position.h"
#include "table.h"

extern atomic<long> node_count;

extern bool limit_strength;
extern int max_noise;
extern int contempt;

int end_eval(Position& board);

int eval(Position& board, int depth, int alpha = EVAL_LOSS, int beta = EVAL_WIN);

string eval_print(int eval);

inline bool is_mate(int eval) {
	return eval > 10000 || eval < -10000;
}

inline void inc_mate(int& eval) {
	if (eval > 10000) { eval++; }
	else if (eval < -10000) { eval--; }
}

inline void dec_mate(int& eval) {
	if (eval > 10000) { eval--; }
	else if (eval < -10000) { eval++; }
}

int add_noise(int& eval);

#endif