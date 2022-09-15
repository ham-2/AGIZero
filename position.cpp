#include <sstream>

#include "position.h"
#include "printer.h"
#include "misc.h"

namespace AGI {

	// For Zobrist Hashing
	Key piece_keys[15][64];
	Key castle_keys[4];
	Key enpassant_keys[8];
	Key side_to_move_key;
	Key fifty_move_key[8];

	constexpr Piece piece_list[] = { W_PAWN, W_KNIGHT, W_BISHOP, W_ROOK, W_QUEEN, W_KING,
									 B_PAWN, B_KNIGHT, B_BISHOP, B_ROOK, B_QUEEN, B_KING };
	
	void Position::init() {
		//set Zobrist keys at startup
		PRNG generator = PRNG(9235123129483259312ULL);

		for (int i = 1; i < 12; i++) {
			for (int j = 0; j < 64; j++) {
				piece_keys[piece_list[i]][j] = Key(generator.get());
			}
		}

		for (int i = 0; i < 4; i++) {
			castle_keys[i] = Key(generator.get());
		}

		for (int i = 0; i < 8; i++) {
			enpassant_keys[i] = Key(generator.get());
		}

		side_to_move_key = Key(generator.get());

		for (int i = 0; i < 8; i++) {
			fifty_move_key[i] = Key(generator.get());
		}
	}


	void Position::push_stack() {
		Undo* new_top = new Undo;
		new_top->prev = undo_stack;
		undo_stack = new_top;
	}

	void Position::pop_stack() {
		Undo* temp = undo_stack;
		undo_stack = undo_stack->prev;
		delete temp;
	}

	void Position::clear_stack() {
		while (undo_stack->prev != nullptr) { pop_stack(); }
	}

	void Position::rebuild() { // computes others from squares data.
		for (Square i = A1; i < SQ_END; ++i) {
			pieces[to_upiece(squares[i])] ^= SquareBoard[i];
			if (squares[i] != EMPTY) {
				colors[to_color(squares[i])] ^= SquareBoard[i];
				piece_count[squares[i]]++;
			}
		}
	}

	inline void Position::place(Piece p, Square s) {
		squares[s] = p;
		undo_stack->key ^= piece_keys[p][s];
		pieces[UEMPTY] ^= SquareBoard[s];
		colors[to_color(p)] ^= SquareBoard[s];
		pieces[to_upiece(p)] ^= SquareBoard[s];
		piece_count[p]++;
	}

	inline void Position::remove(Piece p, Square s) {
		squares[s] = EMPTY;
		undo_stack->key ^= piece_keys[p][s];
		pieces[UEMPTY] ^= SquareBoard[s];
		colors[to_color(p)] ^= SquareBoard[s];
		pieces[to_upiece(p)] ^= SquareBoard[s];
		piece_count[p]--;
	}

	Position::Position() {
		memset(this, 0, sizeof(Position));
		undo_stack = new Undo;
		memset(undo_stack, 0, sizeof(Undo));
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
		for (int i = 0; i < 7; i++) {
			if (test & pieces[i]) {
				sync_cout << "Pieces overlap detected at " << i << sync_endl;
			}
			test |= pieces[i];
			if (testpos.pieces[i] != pieces[i]) {
				sync_cout << "Pieces inconsistent at " << i << sync_endl;
			}
			if (i != 0 && testpos.piece_count[i] != piece_count[i]) {
				sync_cout << "White piece count inconsistent at " << i << sync_endl;
			}
			if (i != 0 && testpos.piece_count[i + 8] != piece_count[i + 8]) {
				sync_cout << "Black piece count inconsistent at " << i + 8 << sync_endl;
			}
		}

		if (testpos.colors[WHITE] != colors[WHITE]) { sync_cout << "White inconsistent" << sync_endl; }
		if (testpos.colors[BLACK] != colors[BLACK]) { sync_cout << "Black inconsistent" << sync_endl; }
	}

	Key Position::get_key() {
		// we need to modify key if 50move count is high
		if (undo_stack->fifty_move > 20) {
			return undo_stack->key ^ fifty_move_key[((undo_stack->fifty_move - 20) >> 2) & 8];
		}
		return undo_stack->key;
	}

	void AGI::Position::show() {
		sync_cout << *this << sync_endl;
		verify();
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
			os << bool(pos.undo_stack->castling_rights[i]) << " ";
		}
		os << "\n";

		// Key
		os << "Key : " << pos.get_key() << "\n";

		return os;
	}

	void Position::set(string fen) {
		char c;
		istringstream ss(fen);

		// first clear stack
		clear_stack();
		Undo* temp = undo_stack;
		memset(this, 0, sizeof(Position));
		undo_stack = temp;
		memset(undo_stack, 0, sizeof(Undo));

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
				undo_stack->key ^= piece_keys[p][sq];
				++sq;
			}
			ss >> c;
		}
		
		// white / black to move
		ss >> c;
		side_to_move = c == 'w' ? WHITE : BLACK;
		if (side_to_move == BLACK) { undo_stack->key ^= side_to_move_key; }
		ss >> c;

		// castling rights
		for (int i = 0; i < 4; i++) { undo_stack->castling_rights[i] = EmptyBoard; }

		ss >> c;
		while (c != ' ') {
			switch (c) {
			case 'K':
				undo_stack->castling_rights[0] = CastlingFrom[0];
				undo_stack->key ^= castle_keys[0];
				break;
			case 'Q':
				undo_stack->castling_rights[1] = CastlingFrom[1];
				undo_stack->key ^= castle_keys[1];
				break;
			case 'k':
				undo_stack->castling_rights[2] = CastlingFrom[2];
				undo_stack->key ^= castle_keys[2];
				break;
			case 'q':
				undo_stack->castling_rights[3] = CastlingFrom[3];
				undo_stack->key ^= castle_keys[3];
				break;
			}
			ss >> c;
		}

		// en passant square
		char file, rank;
		ss >> file >> rank;
		if ((file >= 'a' && file <= 'h') && (rank == '3' || rank == '6')) {
			// otherwise file = '-' && rank == ' ' probably
			sq = parse_square(file, rank);
			undo_stack->enpassant = sq;
			undo_stack->key ^= enpassant_keys[get_file(sq)];
		}

		// 50-move rule
		int count;
		ss >> skipws >> count;
		undo_stack->fifty_move = count;
		
		// move count - doesn't matter

		// write other data
		undo_stack->captured = EMPTY;
		rebuild();
	}

	Move Position::parse_move(string string_move) {
		uint16_t move_int = 0;

		Square from = parse_square(string_move[0], string_move[1]);
		Square to   = parse_square(string_move[2], string_move[3]);

		move_int ^= int(from) + (int(to) << 6);
		
		if (string_move.size() == 5) { // Promotion?
			move_int ^= (1 << 14);
			string piece_finder = "nbrq";
			move_int ^= (piece_finder.find(tolower(string_move[4])) << 12);
		}

		if (squares[from] == W_KING || squares[from] == B_KING) { // Castling?
			if (get_file(from) == 4 &&
				(get_file(to) == 2 || get_file(to) == 6)) {
				move_int ^= (2 << 12);
			}
		}

		if (to == undo_stack->enpassant) { // En passant?
			if (squares[from] == W_PAWN || squares[from] == B_PAWN) {
				move_int ^= (3 << 12);
			}
		}

		return Move(move_int);
	}

	void Position::do_move(Move m) {
		Undo* new_undo = new Undo;
		memcpy(new_undo, undo_stack, sizeof(Undo));
		new_undo->prev = undo_stack;
		new_undo->fifty_move++;
		new_undo->enpassant = Square(0);
		
		if (undo_stack->enpassant != Square(0)) {
			new_undo->key ^= enpassant_keys[get_file(undo_stack->enpassant)];
		} // unset en passant key
		undo_stack = new_undo;

		Square from = get_from(m);
		Square to = get_to(m);

		Piece moved = squares[from];
		Piece captured = squares[to];
		remove(moved, from);

		if (captured != EMPTY) {
			remove(captured, to);
			new_undo->fifty_move = 0;
		}

		// set castling rights
		for (int i = 0; i < 4; i++) {
			if (new_undo->castling_rights[i] & SquareBoard[from]) {
				new_undo->castling_rights[i] = EmptyBoard;
				new_undo->key ^= castle_keys[i];
			}
		}

		switch (get_movetype(m)) {
		case 0: { // Normal
			place(moved, to);
			break;
		}

		case 1: { // Promotion
			// change moved to promoted piece
			moved = side_to_move ? Piece(get_promotion(m) + 8) : Piece(get_promotion(m));
			place(moved, to);
			break;
		}

		case 2: { // Castle
			// do castling
			Square rook_from, rook_to;
			Piece rook = to_piece(ROOK, side_to_move);
			if (get_file(to) > 5) {
				// Kingside
				rook_from = Square(to + 1);
				rook_to = Square(to - 1);
			}
			else {
				// Queenside
				rook_from = Square(to - 2);
				rook_to = Square(to + 1);
			}
			remove(rook, rook_from);
			place(rook, rook_to);
			place(moved, to);
			break;
		}

		case 3: { // En Passant
			Square captured_square = side_to_move ? Square(to + 8) : Square(to - 8);
			remove(to_piece(PAWN, ~side_to_move), captured_square);
			place(moved, to);
			break;
		}
		}

		if (to_upiece(moved) == PAWN) {
			// set en passant square
			if (side_to_move) {
				if (to - from == 16) { 
					new_undo->enpassant = Square(to - 8);
					//new_undo->key ^= enpassant_keys[get_file(to)];
				}
			}
			else {
				if (from - to == 16) { 
					new_undo->enpassant = Square(to + 8);
					//new_undo->key ^= enpassant_keys[get_file(to)];
				}
			}
			new_undo->fifty_move = 0;
		}

		side_to_move = ~side_to_move;
		new_undo->key ^= side_to_move_key;

		new_undo->captured = captured;
	}


}