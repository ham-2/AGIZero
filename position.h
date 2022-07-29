#ifndef POSITION_INCLUDED
#define POSITION_INCLUDED

#include <string>

#include "pieces.h"
#include "board.h"

using namespace std;

namespace AGI {

	enum Color : bool { WHITE = false, BLACK = true };

	typedef uint64_t Key;

	struct Undo {
		// To detect repetition, we store key in this struct
		Key key;
		int repetition;

		// Informations needed to undo a move. Stored in a stack
		Undo* prev;
		Piece captured;
		Square enpassant; // if en passant is possible
		bool castling_rights[4]; // WK, WQ, BK, BQ
	};

	class Position {
	private:
		Piece squares[64];
		Bitboard pieces[8];
		Bitboard colors[2];
		int piece_count[16];
		Color side_to_move;
		Undo* undo_stack;

		void push_stack();
		void pop_stack();
		void clear_stack();

		void rebuild();

	public:
		Position();

		void verify();
		void show();
		void set(string fen);
		void do_move(Move m);


		friend ostream& operator<<(ostream& os, Position& pos);
	};

	ostream& operator<<(ostream& os, Position& pos);
}


#endif