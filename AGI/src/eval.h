#ifndef EVAL_INCLUDED
#define EVAL_INCLUDED

#include "position.h"
#include "movegen.h"
#include "constants.h"
#include "bitboard.h"
#include "constants.h"
#include "types.h"
#include "table.h"
#include "position.h"

namespace AGI {
	using namespace std;

	extern atomic<long> node_count;

	extern bool limit_strength;
	extern int ls_p_max;
	extern int material_bias;
	extern int contempt;

	int end_eval(Stockfish::Position& board);

	int eval(Stockfish::Position& board, int recursion, int alpha = EVAL_LOSS, int beta = EVAL_WIN);

	string eval_print(int eval);

	int perturb_diff(int& eval, int max);

	bool nh_condition(const Stockfish::Position& board);

}

#endif