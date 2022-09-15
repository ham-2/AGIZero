#ifndef POSITION_INCLUDED
#define POSITION_INCLUDED

#include <string>

#include "pieces.h"
#include "board.h"

using namespace std;

namespace AGI {

	typedef uint64_t Key;

	struct Undo {
		// To detect repetition, we store key in this struct
		Key key;
		int repetition;
		int fifty_move;

		// Informations needed to undo a move. Stored in a stack
		Undo* prev;
		Piece captured;
		Square enpassant; // if en passant is possible
		Bitboard castling_rights[4]; // WK, WQ, BK, BQ
	};

	class Position {
	private:
		Piece squares[64];
		Bitboard pieces[7];
		Bitboard colors[2];
		int piece_count[16];
		Color side_to_move;
		Undo* undo_stack;

		void push_stack();
		void pop_stack();
		void clear_stack();

		void rebuild();

		void place(Piece p, Square s);
		void remove(Piece p, Square s);


	public:
		Position();

		static void init();

		void verify();
		Key get_key();
		void show();
		void set(string fen);
		Move parse_move(string string_move);
		void do_move(Move m);


		friend ostream& operator<<(ostream& os, Position& pos);
	};

	ostream& operator<<(ostream& os, Position& pos);
}


#endif