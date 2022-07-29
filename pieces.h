#ifndef PIECES_INCLUDED
#define PIECES_INCLUDED

#include <string>

using namespace std;

namespace AGI {
	
	enum UPiece : int {
		UEMPTY, PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING
	};

	enum Piece : int {
		EMPTY,
		W_PAWN, W_KNIGHT, W_BISHOP, W_ROOK, W_QUEEN, W_KING, W_P1, W_P2,
		B_PAWN, B_KNIGHT, B_BISHOP, B_ROOK, B_QUEEN, B_KING, B_P1, B_P2
	};

	const string FEN_Pieces = " PNBRQK  pnbrqk";

	Piece parse_piece(char c);

	char print_piece(Piece p);

}

#endif
