#include "position.h"
#include "printer.h"

namespace AGI {

	void AGI::Position::push_stack() {
		Undo* new_top = new Undo;
		new_top->prev = undo_stack;
		undo_stack = new_top;
	}

	void AGI::Position::pop_stack() {
		Undo* temp = undo_stack;
		undo_stack = undo_stack->prev;
		delete temp;
	}

	void Position::clear_stack() {
		while (undo_stack->prev != nullptr) { pop_stack(); }
	}

	void Position::rebuild() { // computes others from squares data.
		for (Square i = A1; i < SQ_END; ++i) {
			switch (squares[i]) {
			case W_PAWN:
				pieces[PAWN]   ^= SquareBoard[i];
				colors[WHITE]  ^= SquareBoard[i];
				piece_count[W_PAWN]++;
				break;

			case W_KNIGHT:
				pieces[KNIGHT] ^= SquareBoard[i];
				colors[WHITE]  ^= SquareBoard[i];
				piece_count[W_KNIGHT]++;
				break;

			case W_BISHOP:
				pieces[BISHOP] ^= SquareBoard[i];
				colors[WHITE]  ^= SquareBoard[i];
				piece_count[W_BISHOP]++;
				break;

			case W_ROOK:
				pieces[ROOK]   ^= SquareBoard[i];
				colors[WHITE]  ^= SquareBoard[i];
				piece_count[W_ROOK]++;
				break;

			case W_QUEEN:
				pieces[QUEEN]  ^= SquareBoard[i];
				colors[WHITE]  ^= SquareBoard[i];
				piece_count[W_QUEEN]++;
				break;

			case W_KING:
				pieces[KING]   ^= SquareBoard[i];
				colors[WHITE]  ^= SquareBoard[i];
				piece_count[W_KING]++;
				break;

			case B_PAWN:
				pieces[PAWN]   ^= SquareBoard[i];
				colors[BLACK]  ^= SquareBoard[i];
				piece_count[B_PAWN]++;
				break;

			case B_KNIGHT:
				pieces[KNIGHT] ^= SquareBoard[i];
				colors[BLACK]  ^= SquareBoard[i];
				piece_count[B_KNIGHT]++;
				break;

			case B_BISHOP:
				pieces[BISHOP] ^= SquareBoard[i];
				colors[BLACK]  ^= SquareBoard[i];
				piece_count[B_BISHOP]++;
				break;

			case B_ROOK:
				pieces[ROOK]   ^= SquareBoard[i];
				colors[BLACK]  ^= SquareBoard[i];
				piece_count[B_ROOK]++;
				break;

			case B_QUEEN:
				pieces[QUEEN]  ^= SquareBoard[i];
				colors[BLACK]  ^= SquareBoard[i];
				piece_count[B_QUEEN]++;
				break;

			case B_KING:
				pieces[KING]   ^= SquareBoard[i];
				colors[BLACK]  ^= SquareBoard[i];
				piece_count[B_KING]++;
				break;
			}
		}
	}

	Position::Position() {
		undo_stack = new Undo;
		undo_stack->prev = nullptr;
	}

	void AGI::Position::verify() {
		// Verify all data are consistant.
		if (colors[0] & colors[1]) {
			sync_cout << "Colors overlap detected" << sync_endl;
		}

		Position testpos;
		for (int i = 0; i < 64; i++) { testpos.squares[i] = squares[i]; }
		testpos.rebuild();

		Bitboard test = EmptyBoard;
		for (int i = 0; i < 8; i++) {
			if (test & pieces[i]) {
				sync_cout << "Pieces overlap detected at " << i << sync_endl;
			}
			test |= pieces[i];
			if (testpos.pieces[i] != pieces[i]) {
				sync_cout << "Pieces inconsistent at " << i << sync_endl;
			}
			if (testpos.piece_count[i] != piece_count[i]) {
				sync_cout << "White piece count inconsistent at " << i << sync_endl;
			}
			if (testpos.piece_count[i + 8] != piece_count[i + 8]) {
				sync_cout << "Black piece count inconsistent at " << i + 8 << sync_endl;
			}
		}

		if (testpos.colors[WHITE] != colors[WHITE]) { sync_cout << "White inconsistent" << sync_endl; }
		if (testpos.colors[BLACK] != colors[BLACK]) { sync_cout << "Black inconsistent" << sync_endl; }
	}

	void AGI::Position::show() {
		sync_cout << *this << sync_endl;
		verify();
	}

	void Position::set(string fen) {

	}

	ostream& AGI::operator<<(ostream& os, Position& pos) {
		Square sq = A1;
		for (int i = 0; i < 8; i++) {
			for (int j = 0; j < 8; j++) {
				os << print_piece(pos.squares[sq]) << " ";
				++sq;
			}
			os << "\n";
		}
		return os;
	}

}