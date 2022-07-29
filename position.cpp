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

	void AGI::Position::verify() {
		// Verify all data are consistant.
		if (colors[0] & colors[1]) {
			sync_cout << "Colors overlap detected" << sync_endl;
		}

		Bitboard test_pieces[8];
		Bitboard test_colors[2];
		for (int i = 0; i < 64; i++) {
			switch (squares[i]) {
			case W_PAWN:
				test_pieces[PAWN]   ^= SquareBoard[i];
				test_colors[WHITE]  ^= SquareBoard[i];
				break;

			case W_KNIGHT:
				test_pieces[KNIGHT] ^= SquareBoard[i];
				test_colors[WHITE]  ^= SquareBoard[i];
				break;

			case W_BISHOP:
				test_pieces[BISHOP] ^= SquareBoard[i];
				test_colors[WHITE]  ^= SquareBoard[i];
				break;

			case W_ROOK:
				test_pieces[ROOK]   ^= SquareBoard[i];
				test_colors[WHITE]  ^= SquareBoard[i];
				break;

			case W_QUEEN:
				test_pieces[QUEEN]  ^= SquareBoard[i];
				test_colors[WHITE]  ^= SquareBoard[i];
				break;

			case W_KING:
				test_pieces[KING]   ^= SquareBoard[i];
				test_colors[WHITE]  ^= SquareBoard[i];
				break;

			case B_PAWN:
				test_pieces[PAWN]   ^= SquareBoard[i];
				test_colors[BLACK]  ^= SquareBoard[i];
				break;

			case B_KNIGHT:
				test_pieces[KNIGHT] ^= SquareBoard[i];
				test_colors[BLACK]  ^= SquareBoard[i];
				break;

			case B_BISHOP:
				test_pieces[BISHOP] ^= SquareBoard[i];
				test_colors[BLACK]  ^= SquareBoard[i];
				break;

			case B_ROOK:
				test_pieces[ROOK]   ^= SquareBoard[i];
				test_colors[BLACK]  ^= SquareBoard[i];
				break;

			case B_QUEEN:
				test_pieces[QUEEN]  ^= SquareBoard[i];
				test_colors[BLACK]  ^= SquareBoard[i];
				break;

			case B_KING:
				test_pieces[KING]   ^= SquareBoard[i];
				test_colors[BLACK]  ^= SquareBoard[i];
				break;
			}
		}

		Bitboard test = EmptyBoard;
		for (int i = 0; i < 8; i++) {
			if (test & pieces[i]) {
				sync_cout << "Pieces overlap detected" << i << sync_endl;
			}
			test |= pieces[i];
			if (test_pieces[i] != pieces[i]) {
				sync_cout << "Pieces inconsistent at" << i << sync_endl;
			}
		}

		if (test_colors[WHITE] != colors[WHITE]) { sync_cout << "White inconsistent" << sync_endl; }
		if (test_colors[BLACK] != colors[BLACK]) { sync_cout << "Black inconsistent" << sync_endl; }
	}

	void AGI::Position::show() {
		sync_cout << *this << sync_endl;
		verify();
	}

	ostream& AGI::operator<<(ostream& os, Position& pos) {
		Square sq;
		for (int i = 0; i < 8; i++) {
			for (int j = 0; j < 8; j++) {
				os << print_piece(pos.squares[sq]) << " ";
				++sq;
			}
			os << "\n";
		}
	}

}