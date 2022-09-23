#ifndef MOVEGEN_INCLUDED
#define MOVEGEN_INCLUDED

#include "board.h"
#include "position.h"

namespace AGI {
	struct MoveList {
		Move list[256];
		Move* end;

		void generate(Position& board);

		template<UPiece u>
		Move* add_moves(Move* list_ptr, Position& board, Bitboard pinned, Bitboard _to_board);

		Move* add_pawn_moves(Move* list_ptr, Position& board, Bitboard pinned, Bitboard _to_board);

		bool check_castling(Square low, Square high, Position& board);

		void show();

		int length() { return int(end - list); };

		int find_index(Move m) { 
			for (auto p = list; p != end; p++) {
				if (*p == m) { return int(p - list); }
			}
			return 0;
		}

	};
}

#endif