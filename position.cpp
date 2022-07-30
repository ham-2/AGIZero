#include <sstream>

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
			pieces[OCCUPIED] ^= SquareBoard[i];
			pieces[to_upiece(squares[i])] ^= SquareBoard[i];
			colors[to_color(squares[i])] ^= SquareBoard[i];
			piece_count[squares[i]]++;
		}
	}

	Position::Position() {
		memset(this, 0, sizeof(Position));
		undo_stack = new Undo;
		memset(undo_stack, 0, sizeof(undo_stack));
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
			if (i < 7 && test & pieces[i]) {
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
		char c;
		istringstream ss(fen);

		// first clear stack
		clear_stack();
		Undo* temp = undo_stack;
		memset(this, 0, sizeof(Position));
		undo_stack = temp;

		// set pieces from rank 8.
		ss >> noskipws >> c;
		Square sq = A8;
		Piece p;
		while (c != ' ') {
			if (isdigit(c)) {
				sq += (c - '0');
			}
			else if (c == '/') {
				sq += (-16);
			}
			else if (parse_piece(c, p)) {
				squares[sq] = p;
				++sq;
			}
			ss >> c;
		}
		
		// white / black to move
		ss >> c;
		side_to_move = c == 'w' ? WHITE : BLACK;
		ss >> c;

		// castling rights
		for (int i = 0; i < 4; i++) { undo_stack->castling_rights[i] = false; }

		ss >> c;
		while (c != ' ') {
			switch (c) {
			case 'K':
				undo_stack->castling_rights[0] = true;
				break;
			case 'Q':
				undo_stack->castling_rights[1] = true;
				break;
			case 'k':
				undo_stack->castling_rights[2] = true;
				break;
			case 'q':
				undo_stack->castling_rights[3] = true;
				break;
			}
			ss >> c;
		}

		// en passant square
		char file, rank;
		ss >> file >> rank;
		if ((file >= 'a' && file <= 'h') && (rank == '3' || rank == '6')) {
			// otherwise file = '-' && rank == ' ' probably
			undo_stack->enpassant = parse_square(file, rank);
		}

		// 50-move rule
		ss >> skipws >> undo_stack->fifty_move;
		
		// move count - doesn't matter

		// write other data
		rebuild();
		undo_stack->captured = EMPTY;
		// TODO: compute key for this position
	}

	ostream& AGI::operator<<(ostream& os, Position& pos) {
		// Show board
		Square sq = A8;
		for (int i = 0; i < 8; i++) {
			for (int j = 0; j < 8; j++) {
				os << print_piece(pos.squares[sq]) << " ";
				++sq;
			}
			sq += (-16);
			os << "\n";
		}
		// Side to move, 50move count
		if (pos.side_to_move) { os << "Black to move, "; }
		else { os << "White to move, "; }
		os << "50-move count " << pos.undo_stack->fifty_move << "\n";
		
		// Castling Rights
		os << "Castling rights: ";
		for (int i = 0; i < 4; i++) {
			os << pos.undo_stack->castling_rights[i];
		}
		os << "\n";

		return os;
	}

}