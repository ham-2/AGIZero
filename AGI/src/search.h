#ifndef SEARCH_INCLUDED
#define SEARCH_INCLUDED

#include <cmath>

#include "table.h"
#include "alphabeta.h"
#include "eval.h"
#include "parallel.h"

namespace SEARCH {

	using namespace std;
	using namespace AGI;
	using namespace Stockfish;

	struct treetest {
		Move cmove;
		int eval;
		Move bmove;
		int pad;
		treetest(Move move, int i, Move m, int k) {
			cmove = move;
			eval = i;
			bmove = m;
			pad = k;
		}
	};

	void clear1ply(Position& board);
	void show_tree(Position& board, int count = 4, int max_v = 3);

	void search_start(Position* board, float time, float max_time, bool force_time, int max_ply, std::mutex* ready_mutex);

	extern bool ATM;
	extern bool ponder;
	extern atomic<bool> ponder_continue;

}

#endif
