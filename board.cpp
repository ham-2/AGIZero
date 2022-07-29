#include "board.h"

namespace AGI {
	
	Bitboard SquareBoard[64];

	void init() {
		for (int i = 0; i < 64; i++) {
			SquareBoard[i] = (1ULL << i);
		}
	}

}