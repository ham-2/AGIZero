#include "board.h"

namespace AGI {
	
	Bitboard SquareBoard[64];

	Square parse_square(char file, char rank) {
		return Square((rank - '1') * 8 + (file - 'a'));
	}

	namespace Board {

		void init() {
			for (int i = 0; i < 64; i++) {
				SquareBoard[i] = (1ULL << i);
			}
		}

	}



}