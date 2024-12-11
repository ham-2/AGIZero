#include "board.h"

using namespace std;
	
Bitboard SquareBoard[64];
Bitboard PseudoAttacks[7][64];
Bitboard Rays[64][64];
int Distance[64][64];

Attacks BishopAttacks[64];
Attacks RookAttacks[64];

Square parse_square(char file, char rank) {
	return Square((rank - '1') * 8 + (file - 'a'));
}

ostream& operator<<(ostream& os, Square s) {
	char c[3] = { static_cast<char>('a' + get_file(s)),
					static_cast<char>('1' + get_rank(s)), 0 };
	os << c;
	return os;
}

ostream& operator<<(ostream& os, Move m) {
	os << Square(m & 63) << Square((m >> 6) & 63);
	if (get_movetype(m) == 1) {
		os << "nbrq"[(m >> 12) & 3];
	}
	return os;
}

void print(ostream& os, Bitboard b) {
	for (int i = 7; i >= 0; i--) {
		for (int j = 0; j < 8; j++) {
			os << int((b >> (8 * i + j)) & 1);
		}
		os << "\n";
	}
	os << "\n";
}

namespace Board {

	inline Bitboard shift_sq(Square s, int shift) {
		// for computing single square / knight shifts only (dist < 3)
		Square to = Square(s + shift);
		if (to >= A1 && to < SQ_END) {
			int file_dist = abs(get_file(s) - get_file(to));
			int rank_dist = abs(get_rank(s) - get_rank(to));
			if (file_dist < 3 && rank_dist < 3) {
				return SquareBoard[to];
			}
		}
		return EmptyBoard;
	}

	void init() {
		for (int i = A1; i < SQ_END; i++) {
			SquareBoard[i] = (1ULL << i);
		}

		for (Square i = A1; i < SQ_END; ++i) {
			for (int shift : {7, 9}) { // Using EMPTY for W_PAWN
				PseudoAttacks[EMPTY][i] |= shift_sq(i, shift);
			}

			for (int shift : {-7, -9}) {
				PseudoAttacks[PAWN][i] |= shift_sq(i, shift);
			}

			for (int shift : {-17, -15, -10, -6, 6, 10, 15, 17}) {
				PseudoAttacks[KNIGHT][i] |= shift_sq(i, shift);
			}

			for (int shift : {-9, -7, 7, 9}) {
				Square s = i;
				while (shift_sq(s, shift)) {
					s += shift;
					PseudoAttacks[BISHOP][i] |= SquareBoard[s];
				}
			}

			for (int shift : {-8, -1, 1, 8}) {
				Square s = i;
				while (shift_sq(s, shift)) {
					s += shift;
					PseudoAttacks[ROOK][i] |= SquareBoard[s];
				}
			}

			PseudoAttacks[QUEEN][i] = PseudoAttacks[BISHOP][i] | PseudoAttacks[ROOK][i];

			for (int shift : {-9, -8, -7, -1, 1, 7, 8, 9}) {
				PseudoAttacks[KING][i] |= shift_sq(i, shift);
			}
		}
			
		// build bishop/rook attack tables
		Bitboard file_edges = FileBoard[0] | FileBoard[7];
		Bitboard rank_edges = RankBoard[0] | RankBoard[7];
		Bitboard exclude; // consider the edge if the square is on the edge
		Bitboard occupied, attack;

		for (Square i = A1; i < SQ_END; ++i) {
			exclude = (file_edges & ~FileBoard[get_file(i)]) | (rank_edges & ~RankBoard[get_rank(i)]);
			BishopAttacks[i].relevant = PseudoAttacks[BISHOP][i] & ~exclude;
			RookAttacks[i].relevant = PseudoAttacks[ROOK][i] & ~exclude;

			BishopAttacks[i].attacks = new Bitboard[1 << popcount(BishopAttacks[i].relevant)];
			RookAttacks[i].attacks = new Bitboard[1 << popcount(RookAttacks[i].relevant)];

			occupied = 0;
			do {
				attack = EmptyBoard;
				for (int shift : {-9, -7, 7, 9}) {
					Square s = i;
					while (shift_sq(s, shift) && !(SquareBoard[s] & occupied)) {
						s += shift;
						attack |= SquareBoard[s];
					}
				}
				BishopAttacks[i].attacks[pext(occupied, BishopAttacks[i].relevant)] = attack;
				occupied = (occupied - BishopAttacks[i].relevant) & BishopAttacks[i].relevant;
			} while (occupied);

			occupied = 0;
			do {
				attack = EmptyBoard;
				for (int shift : {-8, -1, 1, 8}) {
					Square s = i;
					while (shift_sq(s, shift) && !(SquareBoard[s] & occupied)) {
						s += shift;
						attack |= SquareBoard[s];
					}
				}
				RookAttacks[i].attacks[pext(occupied, RookAttacks[i].relevant)] = attack;
				occupied = (occupied - RookAttacks[i].relevant) & RookAttacks[i].relevant;
			} while (occupied);
		}

		// Compute rays - for checking pins and blocks.
		// Compute distance: for eval
		for (Square i = A1; i < SQ_END; ++i) {
			for (Square j = A1; j < SQ_END; ++j) {
				Rays[i][j] = 0;
				if (PseudoAttacks[BISHOP][i] & SquareBoard[j]) {
					Rays[i][j] |= PseudoAttacks[BISHOP][i] & PseudoAttacks[BISHOP][j];
				}
				if (PseudoAttacks[ROOK][i] & SquareBoard[j]) {
					Rays[i][j] |= PseudoAttacks[ROOK][i] & PseudoAttacks[ROOK][j];
				}
				Rays[i][j] |= SquareBoard[i] | SquareBoard[j];
				Distance[i][j] = max(abs(get_file(i) - get_file(j)), abs(get_rank(i) - get_rank(j)));
			}
		}

			
			
	}

}