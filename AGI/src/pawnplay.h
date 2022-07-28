#ifndef PAWN_INCLUDED
#define PAWN_INCLUDED

#include "position.h"
#include "bitboard.h"
#include "constants.h"
#include "psqt.h"

namespace AGI {
	using namespace std;

	Stockfish::Score pawn_eval(Stockfish::Position& board);
}

#endif
