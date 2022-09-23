#include <iostream>
#include <cmath>
#include <string>
#include <cstdlib>

#include "eval.h"
#include "material.h"
#include "movegen.h"
#include "pawneval.h"
#include "table.h"

using namespace std;

namespace AGI {

	atomic<long> node_count(0);
	bool limit_strength = false;
	int ls_p_max = 0;
	int material_bias = 0;
	int contempt = 0;

	constexpr Score KnightOutpost  = S(PS[ 0],      0);
	constexpr Score OpenFileRook   = S(PS[ 1], PS[ 2]);
	constexpr Score StuckRook      = S(PS[ 3], PS[ 4]);
	constexpr Score BlockedPawn    = S(PS[ 5], PS[ 6]);
	//constexpr Score PinnedQueen    = S(PS[ 7],      0); unused
	constexpr Score ConnectedRook  = S(PS[ 8], PS[ 8]);
	constexpr Score AttackImba     = S(PS[ 9],      0);
	constexpr Score SemiOpenRook   = S(PS[10], PS[11]);
	//constexpr Score ShieldedMinor  = S(PS[12], PS[13]); unused

	constexpr Score BishopPair     = S(LD[ 2], LD[ 3]);

	int end_eval(Position& board) {

		node_count++;

		int mgcp = 0;
		int egcp = 0;

		// mg-eg balance
		int mg = (board.get_material_t() - 3700) >> 5;
		if (mg < 0) { mg = 0; }
		else if (mg > 128) { mg = 128; }

		Score score = board.get_material();
		Square s;
		Bitboard occupied = board.get_occupied();
		Bitboard pawns = board.get_pieces(PAWN);
		Bitboard w_pawns = board.get_pieces(WHITE, PAWN);
		Bitboard b_pawns = board.get_pieces(BLACK, PAWN);
		Bitboard white_knights = board.get_pieces(WHITE, KNIGHT);
		Bitboard black_knights = board.get_pieces(BLACK, KNIGHT);
		Bitboard white_bishops = board.get_pieces(WHITE, BISHOP);
		Bitboard black_bishops = board.get_pieces(BLACK, BISHOP);
		Bitboard white_rooks = board.get_pieces(WHITE, ROOK);
		Bitboard black_rooks = board.get_pieces(BLACK, ROOK);
		Bitboard white_queens = board.get_pieces(WHITE, QUEEN);
		Bitboard black_queens = board.get_pieces(BLACK, QUEEN);
		Bitboard white_king = board.get_pieces(WHITE, KING);
		Bitboard black_king = board.get_pieces(BLACK, KING);
		Square w_king = board.get_king_square(WHITE);
		Square b_king = board.get_king_square(BLACK);
		Bitboard white_k_adj = PseudoAttacks[KING][w_king] | white_king;
		Bitboard black_k_adj = PseudoAttacks[KING][b_king] | black_king;
		Bitboard w_space = attacks_pawn<WHITE>(w_pawns);
		Bitboard b_space = attacks_pawn<BLACK>(b_pawns);
		bool W_LSB = white_bishops & LightSquares;
		bool W_DSB = white_bishops & DarkSquares;
		bool B_LSB = black_bishops & LightSquares;
		bool B_DSB = black_bishops & DarkSquares;
		Color side = board.get_side();
		
		// King Safety Adjustment
		int white_attacker_value = KS[11];
		int black_attacker_value = KS[11];

		// Winnability
		int winnable = 100;

		// MG / EG specific Calculations
		if (mg != 0) {
			int pscr = 0;
			// Castling Rights, Kings on Center
			if ((white_king & CenterFiles) && !board.has_castling_rights(WHITE)) {
				pscr -= MG[0]; white_attacker_value += KS[8];
			}
			if ((black_king & CenterFiles) && !board.has_castling_rights(BLACK)) {
				pscr += MG[0]; black_attacker_value += KS[8];
			}

			// Exposed King
			if (popcount(get_fileboard(w_king) & pawns) < 2) {
				if (!(get_fileboard(w_king) & w_pawns)) { white_attacker_value += KS[9]; }
				if (!(get_fileboard(w_king) & b_pawns)) { white_attacker_value += KS[9]; }
				pscr -= (2 - popcount(get_fileboard(w_king) & pawns)) * MG[1];
				pscr += popcount(get_fileboard(w_king) & board.get_pieces(WHITE)) * MG[2];
			}
			if (popcount(get_fileboard(b_king) & pawns) < 2) {
				if (!(get_fileboard(b_king) & b_pawns)) { black_attacker_value += KS[9]; }
				if (!(get_fileboard(b_king) & w_pawns)) { black_attacker_value += KS[9]; }
				pscr += (2 - popcount(get_fileboard(b_king) & (w_pawns | b_pawns))) * MG[1];
				pscr -= popcount(get_fileboard(b_king) & board.get_pieces(BLACK)) * MG[2];
			}
			
			// Pawn Shield
			if (shift(white_king, 8) & b_pawns) { pscr += MG[5]; }
			white_attacker_value -= popcount(shift(white_k_adj, 8) & w_pawns);

			if (shift(black_king, -8) & w_pawns) { pscr -= MG[5]; }
			black_attacker_value -= popcount(shift(black_k_adj, -8) & b_pawns);

			// Pawn Storm
			Bitboard w_storms = (get_fileboard(b_king) | adj_fileboard(b_king)) & w_pawns;
			Bitboard b_storms = (get_fileboard(w_king) | adj_fileboard(w_king)) & b_pawns;
			while (w_storms) {
				s = pop_lsb(&w_storms);
				pscr += get_rank(s) * MG[7];
				black_attacker_value += KS[10];
			}
			while (b_storms) {
				s = pop_lsb(&b_storms);
				pscr -= (7 ^ get_rank(s)) * MG[7];
				white_attacker_value += KS[10];
			}

			// Light Square / Dark Square Weaknesses
			int LS_pawns = popcount(board.get_pieces(PAWN) & LightSquares);
			int DS_pawns = popcount(board.get_pieces(PAWN) & DarkSquares);
			if (W_DSB) { mgcp -= DS_pawns * LD[0]; }
			if (W_LSB) { mgcp -= LS_pawns * LD[0]; }
			if (B_DSB) { mgcp += DS_pawns * LD[0]; }
			if (B_LSB) { mgcp += LS_pawns * LD[0]; }

			mgcp += pscr;
		}
		else {
			// Endgame Specific Calculations

			// Opposite Color Bishops
			if (board.get_count(W_BISHOP) == 1 && board.get_count(B_BISHOP) == 1) {
				if (((white_bishops & LightSquares) && (black_bishops & DarkSquares))
					|| ((white_bishops & DarkSquares) && (black_bishops & LightSquares))) {
					if (!board.get_pieces(ROOK) && !board.get_pieces(KNIGHT) 
						&& !board.get_pieces(QUEEN)) { winnable = winnable * 30 / 100; }
					else { winnable = winnable * 70 / 100; }
				}
			}

			// Checkmate with King
			if (white_k_adj & black_k_adj) {
				white_attacker_value += EG[2];
				black_attacker_value += EG[2];
				if (black_king & BoardEdges) { black_attacker_value += EG[3]; }
				if (white_king & BoardEdges) { white_attacker_value += EG[3]; }
			}
		}

		// Active / Passive
		int attack_imbalance = 0;

		// Individual Piece Calculations
		Bitboard blocked;
		Bitboard control;

		while (white_knights) {
			s = pop_lsb(&white_knights);
			control = attacks<KNIGHT>(s, occupied);
			if (control & black_k_adj) { black_attacker_value += KS[0]; }
			if (control & white_k_adj) { white_attacker_value -= KS[1]; }
			w_space |= control;

			attack_imbalance += popcount(control & board.get_pieces(BLACK));

			// Outpost Detection
			if (get_rank(s) > 3 && get_rank(s) < 6 && !(adj_fileboard(s) & get_forward(WHITE, s) & board.get_pieces(BLACK, PAWN))) {
				score += KnightOutpost;
			}
		}
		while (black_knights) {
			s = pop_lsb(&black_knights);
			control = attacks<KNIGHT>(s, occupied);
			if (control & white_k_adj) { white_attacker_value += KS[0]; }
			if (control & black_k_adj) { black_attacker_value -= KS[1]; }
			b_space |= control;

			attack_imbalance -= popcount(control & board.get_pieces(WHITE));

			// Outpost Detection
			if (get_rank(s) < 4 && get_rank(s) > 1 && !(adj_fileboard(s) & get_forward(BLACK, s) & board.get_pieces(WHITE, PAWN))) {
				score -= KnightOutpost;
			}
		}
		while (white_bishops) {
			s = pop_lsb(&white_bishops);
			control = attacks<BISHOP>(s, occupied ^ board.get_pieces(WHITE, QUEEN, BISHOP));
			if (control & black_k_adj) { black_attacker_value += KS[2]; }
			if (control & white_k_adj) { white_attacker_value -= KS[3]; }
			w_space |= control;

			attack_imbalance += popcount(control & board.get_pieces(BLACK));

			// Activity
			score += Activity[0] * popcount(control);
			score += Activity[1] * popcount(control & Black6L);

		}
		while (black_bishops) {
			s = pop_lsb(&black_bishops);
			control = attacks<BISHOP>(s, occupied ^ board.get_pieces(BLACK, QUEEN, BISHOP));
			if (control & white_k_adj) { white_attacker_value += KS[2]; }
			if (control & black_k_adj) { black_attacker_value -= KS[3]; }
			b_space |= control;

			attack_imbalance -= popcount(control & board.get_pieces(WHITE));

			// Activity
			score -= Activity[0] * popcount(control);
			score -= Activity[1] * popcount(control & White6L);

		}
		while (white_rooks) {
			s = pop_lsb(&white_rooks);
			blocked = occupied ^ board.get_pieces(WHITE, QUEEN, ROOK);
			control = attacks<ROOK>(s, blocked);
			if (control & black_k_adj) { black_attacker_value += KS[4]; }
			if (control & white_k_adj) { white_attacker_value -= KS[5]; }
			w_space |= control;

			attack_imbalance += popcount(control & board.get_pieces(BLACK));

			// Activity
			score += Activity[2] * popcount(control & Black6L);

			// Stuck Rook
			if (popcount(control) < 6 && ((get_file(w_king) < 4) == (get_file(s) < get_file(w_king)))) {
				score -= StuckRook * (6 - popcount(control)) * (1 + !board.has_castling_rights(WHITE));
			}

			// rooks on open file
			if (!(blocked & get_fileboard(s))) {
				score += OpenFileRook;
			}
			else if (board.semiopen_file(s, WHITE)) {
				score += SemiOpenRook;
			}

			// Connect Rooks
			if (board.get_pieces(WHITE, QUEEN, ROOK) & get_fileboard(s)) {
				score += ConnectedRook;
			}
		}
		while (black_rooks) {
			s = pop_lsb(&black_rooks);
			blocked = occupied ^ board.get_pieces(BLACK, QUEEN, ROOK);
			control = attacks<ROOK>(s, blocked);
			if (control & white_k_adj) { white_attacker_value += KS[4]; }
			if (control & black_k_adj) { black_attacker_value -= KS[5]; }
			b_space |= control;

			attack_imbalance -= popcount(control & board.get_pieces(WHITE));

			// Activity
			score -= Activity[2] * popcount(control & White6L);

			// Stuck Rook
			if (popcount(control) < 6 && ((get_file(b_king) < 4) == (get_file(s) < get_file(b_king)))) {
				score += StuckRook * (6 - popcount(control)) * (1 + !board.has_castling_rights(BLACK));
			}

			// rooks on open file
			if (!(blocked & get_fileboard(s))) {
				score -= OpenFileRook;
			}
			else if (board.semiopen_file(s, BLACK)) {
				score -= SemiOpenRook;
			}

			// Connect Rooks
			if (board.get_pieces(BLACK, QUEEN, ROOK) & get_fileboard(s)) {
				score -= ConnectedRook;
			}
		}

		while (white_queens) {
			s = pop_lsb(&white_queens);
			blocked = occupied ^ board.get_pieces(WHITE, QUEEN, ROOK);
			control = attacks<QUEEN>(s, blocked ^ board.get_pieces(WHITE, BISHOP));
			if (control & black_k_adj) { black_attacker_value += KS[6]; }
			if (control & white_k_adj) { white_attacker_value -= KS[7]; }

			// Activity
			score += Activity[2] * popcount(control & Black6L);
		}
		while (black_queens) {
			s = pop_lsb(&black_queens);
			blocked = occupied ^ board.get_pieces(BLACK, QUEEN, ROOK);
			control = attacks<QUEEN>(s, blocked ^ board.get_pieces(BLACK, BISHOP));
			if (control & white_k_adj) { white_attacker_value += KS[6]; }
			if (control & black_k_adj) { black_attacker_value -= KS[7]; }

			// Activity
			score -= Activity[2] * popcount(control & White6L);
		}

		if (mg < 64) { // King to Passed Pawns / Pawns
			Bitboard w_pawns_d = w_pawns;
			Bitboard b_pawns_d = b_pawns;

			while (w_pawns_d) {
				s = pop_lsb(&w_pawns_d);
				egcp -= EG[0] * Distance[w_king][s];
				egcp += EG[0] * Distance[b_king][s];
				if (board.is_passed_pawn(s, WHITE)) {
					egcp -= EG[1] * Distance[w_king][s];
					egcp += EG[1] * Distance[b_king][s];
				}
			}
			while (b_pawns_d) {
				s = pop_lsb(&b_pawns_d);
				egcp -= EG[0] * Distance[w_king][s];
				egcp += EG[0] * Distance[b_king][s];
				if (board.is_passed_pawn(s, BLACK)) {
					egcp -= EG[1] * Distance[w_king][s];
					egcp += EG[1] * Distance[b_king][s];
				}
			}
		}

		// Pawn Mobility
		score -= BlockedPawn * popcount(shift(w_pawns,  8) & occupied);
		score += BlockedPawn * popcount(shift(b_pawns, -8) & occupied);

		// Space Advantage
		score += Activity[3] * (popcount(w_space) - popcount(b_space));

		// Attack Imbalance
		score += AttackImba * attack_imbalance;
		
		//Bishop Pair
		if (W_LSB && W_DSB) { score += BishopPair; }
		if (B_LSB && B_DSB) { score -= BishopPair; }
		
		// King Safety
		if (white_attacker_value < 0) { white_attacker_value = 0; }
		else if (white_attacker_value > 63) { white_attacker_value = 63; }
		if (black_attacker_value < 0) { black_attacker_value = 0; }
		else if (black_attacker_value > 63) { black_attacker_value = 63; }
		mgcp -= King_Attack[white_attacker_value];
		egcp -= King_Attack[white_attacker_value];
		mgcp += King_Attack[black_attacker_value];
		egcp += King_Attack[black_attacker_value];

		// Pawns
		if (PTT::TT_MB_SIZE == 0) {
			score += pawn_eval(board);
		}
		else {
			score += Pawn_TT.probe(board.get_pawnkey(), board);
		}

		// Finalize
		mgcp += get_mg(score);
		egcp += get_eg(score);

		// Winnability
		if (mg < 25) {
			if (abs(board.get_material_t()) < 800) {
				if (egcp > 0 && board.get_count(W_PAWN) < 3) {
					winnable = winnable * (board.get_count(W_PAWN) + 1) >> 2;
				}
				else if (egcp < 0 && board.get_count(W_PAWN) < 3) {
					winnable = winnable * (board.get_count(B_PAWN) + 1) >> 2;
				}
			}
		}
		egcp = egcp * winnable / 100;
		
		// final mg+eg
		int cp_ = mg * mgcp + (128 - mg) * egcp;
		if (board.get_fiftymove() > 10) { cp_ = (110 - board.get_fiftymove()) / 100; }
		cp_ >>= 7;

		if (board.get_side()) { cp_ *= -1; }

		// Tempo
		cp_ += Tempo;

		if (limit_strength) {
			cp_ = cp_ * 100 / (100 + 2 * material_bias);
			perturb_diff(cp_, ls_p_max);
		}

		return cp_;
	}

	int eval(Position& board, int depth, int alpha, int beta)
	{
		// Move Generation
		MoveList legal_moves;
		legal_moves.generate(board);

		// Mates and Stalemates
		if (legal_moves.length() == 0) {
			if (board.get_checkers()) { return EVAL_LOSS; }
			else { return 0; }
		}

		// repetition
		if (board.get_repetition() < -1024) { return 0; }

		// 50-move
		if (board.get_fiftymove() > 99) { return 0; }

		int new_eval = EVAL_LOSS;
		int comp_eval;
		
		//depth restriction
		if (depth < 1) {
			new_eval = end_eval(board);
			// ab
			if (new_eval > alpha) { alpha = new_eval; }
			if (alpha > beta) { return new_eval; }

			inc_mate(new_eval);
			inc_mate(alpha);
			inc_mate(beta);

			int margin = alpha - new_eval - DELTA_MARGIN > 0 ? 
						 alpha - new_eval - DELTA_MARGIN : 0;

			for (auto move = legal_moves.list; move != legal_moves.end; move++) {
				if (board.capture_or_promotion(*move)) {
					if (board.see(*move) > margin) { // SEE > 0 Extension
						Undo u;
						board.do_move(*move, &u);
						comp_eval = -eval(board, 0, -beta, -alpha);
						board.undo_move(*move);
						dec_mate(comp_eval);
						if (comp_eval > new_eval) { 
							new_eval = comp_eval;
							if (comp_eval > alpha) { 
								alpha = comp_eval;
								if (alpha >= beta) { break; }
							}
						}
					}
				}
			}

			if (new_eval > 10000) { new_eval--; }
			else if (new_eval < -10000) { new_eval++; }
			return new_eval;
		}

		if (board.get_checkers()) { // Checked
			
			inc_mate(alpha);
			inc_mate(beta); // For Mate Distance Comparison

			for (auto move = legal_moves.list; move != legal_moves.end; move++) {
				Undo u;
				board.do_move(*move, &u);
				comp_eval = -eval(board, depth - 1, -beta, -alpha);
				board.undo_move(*move);
				dec_mate(comp_eval);
				if (comp_eval > new_eval) { 
					new_eval = comp_eval;
					if (comp_eval > alpha) { alpha = comp_eval; }
				}
				if (alpha >= beta) { break; }
			}

			if (new_eval > 10000) { new_eval--; }
			else if (new_eval < -10000) { new_eval++; }
			return new_eval;
		}

		else {
			new_eval = end_eval(board);

			if (new_eval > alpha) { alpha = new_eval; }
			if (alpha > beta) { return new_eval; }

			inc_mate(new_eval);
			inc_mate(alpha);
			inc_mate(beta); // For Mate Distance Comparison

			int margin = alpha - new_eval - DELTA_MARGIN > 0 ?
						 alpha - new_eval - DELTA_MARGIN : 0;

			// Quiescence Expansion
			for (auto move = legal_moves.list; move != legal_moves.end; move++) {
				if (board.capture_or_promotion(*move)) {
					if (board.see(*move) > margin) {
						Undo u;
						board.do_move(*move, &u);
						comp_eval = -eval(board, depth - 1, -beta, -alpha);
						board.undo_move(*move);
						dec_mate(comp_eval);
						if (comp_eval > new_eval) { 
							new_eval = comp_eval;
							if (comp_eval > alpha) { 
								alpha = comp_eval;
								if (alpha >= beta) { break; }
							}
						}
					}
				}

				else if (board.is_check(*move)) {
					Undo u;
					board.do_move(*move, &u);
					comp_eval = -eval(board, depth - 1, -beta, -alpha);
					board.undo_move(*move);
					dec_mate(comp_eval);
					if (comp_eval > new_eval) { 
						new_eval = comp_eval;
						if (comp_eval > alpha) { 
							alpha = comp_eval;
							if (alpha >= beta) { break; }
						}
					}
				}
			}
			
			if (new_eval > 10000) { new_eval--; }
			else if (new_eval < -10000) { new_eval++; }
			return new_eval;
		}
	}

	string eval_print(int eval) {
		if (eval > 20000) {
			int ply_to_mate = EVAL_WIN - eval;
			return "mate " + to_string((ply_to_mate + 1) / 2);
		}
		else if (eval < -20000) {
			int ply_to_mate = eval + EVAL_WIN;
			return "mate -" + to_string((ply_to_mate + 1) / 2);
		}
		else {
			return "cp " + to_string(eval * 100 / pawn_eg);
		}
	}

	int perturb_diff(int& eval, int max) {
		eval += rand() % (max * 2 + 1) - max;
		return eval;
	}

}