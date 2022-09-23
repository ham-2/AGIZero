#include "pawneval.h"

#include "constants.h"
#include "material.h"

using namespace std;

namespace AGI {

	constexpr Score DoubledPawn  = S(PN[0], PN[1]);
	constexpr Score IsolatedPawn = S(PN[2], PN[3]);
	constexpr Score BackwardPawn = S(PN[4], PN[5]);
	constexpr Score PPPawn       = S(PN[6], PN[7]);

	Score pawn_eval(Position& board)
	{
		Score score = Score(0);
		Bitboard pawns = board.get_pieces(PAWN);
		Bitboard w_pawns = board.get_pieces(WHITE, PAWN);
		Bitboard b_pawns = board.get_pieces(BLACK, PAWN);
		Bitboard w_pawns_d = w_pawns;
		Bitboard b_pawns_d = b_pawns;
		Square sq;

		while (w_pawns_d) {
			sq = pop_lsb(&w_pawns_d);

			// Doubled pawn, Tripled...
			if (get_fileboard(sq) & w_pawns_d) {
				score -= DoubledPawn;
			}

			// Isolated pawn
			if (!(adj_fileboard(sq) & w_pawns)) {
				score -= IsolatedPawn;
			}

			// Backward Pawn
			if (!((get_forward(BLACK, sq + 8)) & adj_fileboard(sq) & w_pawns) &&
				(b_pawns & attacks_pawn<WHITE>(sq + 8) || pawns & SquareBoard[sq + 8])) {
				score -= BackwardPawn;
			}

			// Passed pawn
			if (!(get_fileboard(sq) & b_pawns)) {
				if (board.is_passed_pawn(sq, WHITE)) { // Real Passed
					score += PassedPawn[get_rank(sq)];
					if (SquareBoard[sq] & attacks_pawn<WHITE>(w_pawns) 
						&& get_rank(sq) > 3) { score += PPPawn * get_rank(sq); }
				}
				else if (!(get_forward(WHITE, sq) & w_pawns)) { // Candidate
					score += PassedPawn[get_rank(sq) + 8];
				}
			}
		}

		while (b_pawns_d) {
			sq = pop_lsb(&b_pawns_d);

			// Doubled pawn, Tripled...
			if (get_fileboard(sq) & b_pawns_d) {
				score += DoubledPawn;
			}

			// Isolated pawn
			if (!(adj_fileboard(sq) & b_pawns)) {
				score += IsolatedPawn;
			}

			// Backward Pawn
			if (!(get_forward(WHITE, sq - 8) & adj_fileboard(sq) & b_pawns) &&
				(w_pawns & attacks_pawn<BLACK>(sq - 8) || pawns & SquareBoard[sq - 8])) {
				score += BackwardPawn;
			}

			// Passed pawn
			if (!(get_fileboard(sq) & w_pawns)) {
				if (board.is_passed_pawn(sq, BLACK)) {
					score -= PassedPawn[get_rank(sq) ^ 7];
					if (SquareBoard[sq] & attacks_pawn<BLACK>(b_pawns) 
						&& get_rank(sq) < 4) { score -= PPPawn * (get_rank(sq) ^ 7); }
				}
				else if (!(get_forward(BLACK, sq) & b_pawns)) {
					score -= PassedPawn[(get_rank(sq) ^ 7) + 8];
				}
			}
		}

		return score;
	}
}
