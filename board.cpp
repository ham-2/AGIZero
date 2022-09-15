#include "board.h"

namespace AGI {
	
	Bitboard SquareBoard[64];
	Bitboard CastlingFrom[4];

	Square parse_square(char file, char rank) {
		return Square((rank - '1') * 8 + (file - 'a'));
	}

	namespace Board {

		void init() {
			for (int i = 0; i < 64; i++) {
				SquareBoard[i] = (1ULL << i);
			}

			CastlingFrom[0] = SquareBoard[E1] | SquareBoard[H1];
			CastlingFrom[1] = SquareBoard[E1] | SquareBoard[A1];
			CastlingFrom[2] = SquareBoard[E8] | SquareBoard[H8];
			CastlingFrom[3] = SquareBoard[E8] | SquareBoard[A8];
		}

	}



}