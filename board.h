#ifndef BOARD_INCLUDED
#define BOARD_INCLUDED

#include <cstdint>

#include "pieces.h"

namespace AGI {

	enum Square : int {
		A1, B1, C1, D1, E1, F1, G1, H1,
		A2, B2, C2, D2, E2, F2, G2, H2,
		A3, B3, C3, D3, E3, F3, G3, H3,
		A4, B4, C4, D4, E4, F4, G4, H4,
		A5, B5, C5, D5, E5, F5, G5, H5,
		A6, B6, C6, D6, E6, F6, G6, H6,
		A7, B7, C7, D7, E7, F7, G7, H7,
		A8, B8, C8, D8, E8, F8, G8, H8,
		SQ_END
	};

	inline Square& operator++(Square& s) { return s = Square(int(s) + 1); }
	inline Square& operator+=(Square& s, int i) { return s = Square(int(s) + i); }

	Square parse_square(char file, char rank);

	inline int get_file(Square s) { return s & 7; }
	inline int get_rank(Square s) { return s >> 3; }

	// Use 16bit integer for moves - 0bXXAABBBBBBCCCCCC
	// C: 6 bits indicating from square
	// B: 6 bits indicating to square
	// A: 00: knight 01: bishop 10: rook 11: queen promotion
	// X: 00: normal 01: promotion 10: castling 11: en passant
	enum Move : uint16_t {
		NULL_MOVE = 0
	};

	inline constexpr Square get_from(Move m) { return Square(m & 0x3F); }
	inline constexpr Square get_to(Move m)   { return Square((m >> 6) & 0x3F); }
	inline constexpr UPiece get_promotion(Move m) { return UPiece((m >> 12) & 3 + KNIGHT); }
	inline constexpr int get_movetype(Move m) { return (m >> 14); }

	typedef uint64_t Bitboard;
	constexpr Bitboard EmptyBoard = 0;
	constexpr Bitboard FullBoard  = ~EmptyBoard;
	
	extern Bitboard SquareBoard[64];
	extern Bitboard CastlingFrom[4];

	namespace Board {
		void init();
	}
}

#endif
