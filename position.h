#ifndef POSITION_INCLUDED
#define POSITION_INCLUDED

#include <string>

#include "pieces.h"
#include "board.h"
#include "material.h"

using namespace std;

namespace AGI {

	typedef uint64_t Key;

	struct Undo {
		// To detect repetition, we store key in this struct
		// repetition values
		// positive : repeated once, distance to last occurance
		// negative : repeated twice, distance to last occurance
		// negative, <1024 : repeated > 2, distance to last occurance
		Key key;
		Key pawn_key;
		int repetition;
		int fifty_move;

		// Informations needed to undo a move. Stored in a stack
		Undo* prev;
		Piece captured;
		Square enpassant; // if en passant is possible
		int castling_rights; // WK, WQ, BK, BQ

		// Other useful informations
		Bitboard checkers;
		Square king_square[2];
		bool del = false;
	};

	class Position {
	private:
		Undo* undo_stack;
		Piece squares[64];
		Bitboard pieces[7];
		Bitboard colors[2];
		int piece_count[16]; // EMPTY: total count, not empty
		Score material; // imbalance
		int material_t; // total piece mg value
		Color side_to_move;

		void pop_stack();
		void clear_stack();

		void rebuild();

		void place(Piece p, Square s);
		void remove(Piece p, Square s);
		void move_piece(Piece p, Square from, Square to);


	public:
		Position();

		static void init();

		void verify();
		Key get_key();
		inline Key get_pawnkey() { return undo_stack->pawn_key; };
		Color get_side() { return side_to_move; }
		Piece get_piece(Square s) { return squares[s]; }
		inline Bitboard get_pieces(Color c) { return colors[c]; }
		inline Bitboard get_pieces(UPiece u) { return pieces[u]; }
		inline Bitboard get_pieces(Color c, UPiece u) { return colors[c] & pieces[u]; };
		inline Bitboard get_pieces(Color c, UPiece u1, UPiece u2) { return colors[c] & (pieces[u1] | pieces[u2]); }
		Bitboard get_attackers(Square s, Bitboard occupied);
		Bitboard see_least_piece(Color c, Bitboard attackers, UPiece& u);
		Bitboard get_checkers() { return undo_stack->checkers;  }
		Bitboard get_occupied() { return ~pieces[EMPTY]; }
		Bitboard get_pinned(Color c, Square k);
		Square get_king_square(Color c) { return undo_stack->king_square[c]; }
		Square get_enpassant() { return undo_stack->enpassant; }
		Score get_material() { return material; }
		int get_material_t() { return material_t; }
		int get_count(Piece p) { return piece_count[p]; }
		int get_fiftymove() { return undo_stack->fifty_move; }
		int get_repetition() { return undo_stack->repetition; }
		bool get_repetition(int root_dist) { 
			return undo_stack->repetition == 0 ? false :
				undo_stack->repetition > 0 ? (undo_stack->repetition <= root_dist) :
				undo_stack->repetition > -1024 ? (undo_stack->repetition >= -root_dist) :
				true;
		}
		inline bool get_castling_right(int i) {
			return bool(undo_stack->castling_rights & (1 << i));
		}
		inline bool has_castling_rights(Color c) {
			return c ? undo_stack->castling_rights & 12 : undo_stack->castling_rights & 3;
		}
		inline bool get_nh_condition() {
			return !(undo_stack->checkers || (material_t == 0) || (piece_count[EMPTY] < 8));
		};

		// functions for eval
		inline bool semiopen_file(Square s, Color c) { 
			return (popcount(FileBoard[get_file(s)] & pieces[PAWN] & colors[~c]) == 1);
		}
		inline bool is_passed_pawn(Square s, Color c) {
			return !bool(get_forward(c, s) & (get_fileboard(s) | adj_fileboard(s)) & pieces[PAWN]);
		}
		inline bool capture_or_promotion(Move m) {
			return get_movetype(m) == 0 ? squares[get_to(m)] != EMPTY : get_movetype(m) != 2;
		}
		int see(Move m);
		bool is_check(Move m);

		void show();
		void set(string fen);
		Move parse_move(string string_move);
		void do_move(Move m, Undo* new_undo);
		void undo_move(Move m);
		void do_null_move(Undo* new_undo);
		void undo_null_move();

		Position& operator=(const Position board);

		friend ostream& operator<<(ostream& os, Position& pos);
	};

	ostream& operator<<(ostream& os, Position& pos);
}


#endif
