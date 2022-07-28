#include "pawnplay.h"
namespace AGI {
	using namespace Stockfish;

	constexpr Score DoubledPawn  = make_score(PN[0], PN[1]);
	constexpr Score IsolatedPawn = make_score(PN[2], PN[3]);
	constexpr Score BackwardPawn = make_score(PN[4], PN[5]);
	constexpr Score PPPawn       = make_score(PN[6], PN[7]);

	Score AGI::pawn_eval(Stockfish::Position& board)
	{
		Score score = SCORE_ZERO;
		Bitboard pawns = board.pieces(PAWN);
		Bitboard w_pawns = board.pieces(WHITE, PAWN);
		Bitboard b_pawns = board.pieces(BLACK, PAWN);
		Bitboard w_pawns_d = w_pawns;
		Bitboard b_pawns_d = b_pawns;
		Square sq;

		while (w_pawns_d) {
			sq = pop_lsb(&w_pawns_d);
			score += psqt[W_PAWN][sq];

			// Doubled pawn, Tripled...
			if (file_bb(sq) & w_pawns_d) {
				score -= DoubledPawn;
			}

			// Isolated pawn
			if (!(adjacent_files_bb(sq) & w_pawns)) {
				score -= IsolatedPawn;
			}

			// Backward Pawn
			if (!((forward_ranks_bb(BLACK, sq + NORTH)) & adjacent_files_bb(sq) & w_pawns) &&
				(b_pawns & pawn_attacks_bb(WHITE, sq + NORTH) || pawns & square_bb(sq + NORTH))) {
				score -= BackwardPawn;
			}

			// Passed pawn
			if (!(file_bb(sq) & b_pawns)) {
				if (board.pawn_passed(WHITE, sq)) { // Real Passed
					score += PassedPawn[rank_of(sq)];
					if (square_bb(sq) & pawn_attacks_bb<WHITE>(w_pawns) && rank_of(sq) > 3) { score += PPPawn * rank_of(sq); }
				}
				else { // Candidate
					score += PassedPawn[rank_of(sq) + 8];
				}
			}
		}

		while (b_pawns_d) {
			sq = pop_lsb(&b_pawns_d);
			score += psqt[B_PAWN][sq];

			// Doubled pawn, Tripled...
			if (file_bb(sq) & b_pawns_d) {
				score += DoubledPawn;
			}

			// Isolated pawn
			if (!(adjacent_files_bb(sq) & b_pawns)) {
				score += IsolatedPawn;
			}

			// Backward Pawn
			if (!(forward_ranks_bb(WHITE, sq + SOUTH) & adjacent_files_bb(sq) & b_pawns) &&
				(w_pawns & pawn_attacks_bb(BLACK, sq + SOUTH) || pawns & square_bb(sq + SOUTH))) {
				score += BackwardPawn;
			}

			// Passed pawn
			if (!(file_bb(sq) & w_pawns)) {
				if (board.pawn_passed(BLACK, sq)) {
					score -= PassedPawn[relative_rank(BLACK, rank_of(sq))];
					if (square_bb(sq) & pawn_attacks_bb<BLACK>(b_pawns) && rank_of(sq) < 4) { score -= PPPawn * relative_rank(BLACK, rank_of(sq)); }
				}
				else {
					score -= PassedPawn[relative_rank(BLACK, rank_of(sq)) + 8];
				}
			}
		}

		return score;
	}
}
