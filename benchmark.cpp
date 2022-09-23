#include <cstdint>

#include "benchmark.h"
#include "printer.h"
#include "movegen.h"

using namespace std;

namespace AGI {

	uint64_t _perft(Position* board, int depth) {
		if (depth == 0) {
			return 1ULL;
		}
		else {
			uint64_t value = 0;
			MoveList ml;
			ml.generate(*board);
			for (Move* m = ml.list; m < ml.end; m++) {
				Undo u;
				board->do_move(*m, &u);
				value += _perft(board, depth - 1);
				board->undo_move(*m);
			}
			return value;
		}
	}

	void perft(Position* board, int depth) {
		uint64_t value = 0;
		MoveList ml;
		ml.generate(*board);
		for (Move* m = ml.list; m < ml.end; m++) {
			Undo u;
			board->do_move(*m, &u);
			uint64_t sub = _perft(board, depth - 1);
			cout << *m << " " << sub << endl;
			value += sub;
			board->undo_move(*m);
		}
		cout << value << endl;
	}

}