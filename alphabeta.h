#ifndef ALPHABETA_INCLUDED
#define ALPHABETA_INCLUDED

#include <mutex>
#include <atomic>
#include <iostream>

#include "eval.h"
#include "position.h"
#include "movegen.h"
#include "constants.h"
#include "table.h"

using namespace std;

int alpha_beta(Position& board, atomic<bool>* stop,
	int ply, TTEntry* probe,
	Color root_c, int step,
	int alpha = EVAL_LOSS, int beta = EVAL_WIN,
	int root_dist = 0);

#endif