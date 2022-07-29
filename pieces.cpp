#include "pieces.h"

namespace AGI {

	Piece parse_piece(char c) { return Piece(FEN_Pieces.find(c)); }

	char print_piece(Piece p) { return FEN_Pieces[p]; }

}