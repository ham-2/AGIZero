#include <iostream>
#include <cmath>
#include <string>
#include <cstdlib>

#include "eval.h"

namespace AGI {
	using namespace Stockfish;

	atomic<long> node_count = 0;
	bool limit_strength = false;
	int ls_p_max = 0;
	int material_bias = 0;
	int contempt = 0;

	constexpr Score KnightOutpost  = make_score(PS[ 0],      0);
	constexpr Score OpenFileRook   = make_score(PS[ 1], PS[ 2]);
	constexpr Score StuckRook      = make_score(PS[ 3], PS[ 4]);
	constexpr Score BlockedPawn    = make_score(PS[ 5], PS[ 6]);
	constexpr Score PinnedQueen    = make_score(PS[ 7],      0);
	constexpr Score ConnectedRook  = make_score(PS[ 8], PS[ 8]);
	constexpr Score AttackImba     = make_score(PS[ 9],      0);
	constexpr Score SemiOpenRook   = make_score(PS[10], PS[11]);
	constexpr Score ShieldedMinor  = make_score(PS[12], PS[13]);

	constexpr Score BishopPair     = make_score(LD[ 2], LD[ 3]);

	int end_eval(Position& board) {

		node_count++;

		int mgcp = 0;
		int egcp = 0;

		// mg-eg balance
		int mg = (board.non_pawn_material() - 2700) >> 5;
		if (mg < 0) { mg = 0; }
		else if (mg > 128) { mg = 128; }

		Score score = SCORE_ZERO;
		Square s;
		Bitboard occupied = board.pieces();
		Bitboard pawns = board.pieces(PAWN);
		Bitboard w_pawns = board.pieces(WHITE, PAWN);
		Bitboard b_pawns = board.pieces(BLACK, PAWN);
		Bitboard white_knights = board.pieces(WHITE, KNIGHT);
		Bitboard black_knights = board.pieces(BLACK, KNIGHT);
		Bitboard white_bishops = board.pieces(WHITE, BISHOP);
		Bitboard black_bishops = board.pieces(BLACK, BISHOP);
		Bitboard white_rooks = board.pieces(WHITE, ROOK);
		Bitboard black_rooks = board.pieces(BLACK, ROOK);
		Bitboard white_queens = board.pieces(WHITE, QUEEN);
		Bitboard black_queens = board.pieces(BLACK, QUEEN);
		Bitboard white_king = board.pieces(WHITE, KING);
		Bitboard black_king = board.pieces(BLACK, KING);
		Square w_king = lsb(board.pieces(WHITE, KING));
		Square b_king = lsb(board.pieces(BLACK, KING));
		Bitboard white_k_adj = PseudoAttacks[KING][w_king] | white_king;
		Bitboard black_k_adj = PseudoAttacks[KING][b_king] | black_king;
		Bitboard w_space = pawn_attacks_bb<WHITE>(w_pawns);
		Bitboard b_space = pawn_attacks_bb<BLACK>(b_pawns);
		bool W_LSB = white_bishops & LightSquares;
		bool W_DSB = white_bishops & DarkSquares;
		bool B_LSB = black_bishops & LightSquares;
		bool B_DSB = black_bishops & DarkSquares;
		Color side = board.side_to_move();
		
		// King Safety Adjustment
		int white_attacker_value = KS[11];
		int black_attacker_value = KS[11];

		// Winnability
		int winnable = 100;

		// MG / EG specific Calculations
		if (mg != 0) {
			int pscr = 0;
			// Castling Rights, Kings on Center
			if ((white_king & CenterFiles) && !board.castling_rights(WHITE)) { pscr -= MG[0]; white_attacker_value += KS[8]; }
			if ((black_king & CenterFiles) && !board.castling_rights(BLACK)) { pscr += MG[0]; black_attacker_value += KS[8]; }

			// Exposed King
			if (popcount(file_bb(w_king) & pawns) < 2) {
				if (!(file_bb(w_king) & w_pawns)) { white_attacker_value += KS[9]; }
				if (!(file_bb(w_king) & b_pawns)) { white_attacker_value += KS[9]; }
				pscr -= (2 - popcount(file_bb(w_king) & pawns)) * MG[1];
				pscr += popcount(file_bb(w_king) & board.pieces(WHITE)) * MG[2];
			}
			if (popcount(file_bb(b_king) & pawns) < 2) {
				if (!(file_bb(b_king) & b_pawns)) { black_attacker_value += KS[9]; }
				if (!(file_bb(b_king) & w_pawns)) { black_attacker_value += KS[9]; }
				pscr += (2 - popcount(file_bb(b_king) & (w_pawns | b_pawns))) * MG[1];
				pscr -= popcount(file_bb(b_king) & board.pieces(BLACK)) * MG[2];
			}
			
			// Pawn Shield
			if (shift<NORTH>(white_king) & b_pawns) { pscr += MG[5]; }
			white_attacker_value -= popcount(shift<NORTH>(white_k_adj) & w_pawns);

			if (shift<SOUTH>(black_king) & w_pawns) { pscr -= MG[5]; }
			black_attacker_value -= popcount(shift<SOUTH>(black_k_adj) & b_pawns);

			// Pawn Storm
			Bitboard w_storms = (file_bb(b_king) | adjacent_files_bb(b_king)) & w_pawns;
			Bitboard b_storms = (file_bb(w_king) | adjacent_files_bb(w_king)) & b_pawns;
			while (w_storms) {
				s = pop_lsb(&w_storms);
				pscr += rank_of(s) * MG[7];
				black_attacker_value += KS[10];
			}
			while (b_storms) {
				s = pop_lsb(&b_storms);
				pscr -= relative_rank(BLACK, rank_of(s)) * MG[7];
				white_attacker_value += KS[10];
			}

			// Light Square / Dark Square Weaknesses
			int LS_pawns = popcount(board.pieces(PAWN) & LightSquares);
			int DS_pawns = popcount(board.pieces(PAWN) & DarkSquares);
			if (W_DSB) { mgcp -= DS_pawns * LD[0]; }
			if (W_LSB) { mgcp -= LS_pawns * LD[0]; }
			if (B_DSB) { mgcp += DS_pawns * LD[0]; }
			if (B_LSB) { mgcp += LS_pawns * LD[0]; }

			mgcp += pscr;
		}
		else {
			// Endgame Specific Calculations

			// Known Endgames
			if (!board.pieces(PAWN)) {
				if (!board.pieces(QUEEN)) {
					if (!board.pieces(ROOK)) {
						int w_n = popcount(white_knights);
						int b_n = popcount(black_knights);
						int w_b = popcount(white_bishops);
						int b_b = popcount(black_bishops);
						int mat_ib = board.non_pawn_material(WHITE) - board.non_pawn_material(BLACK);

						if (w_n == 0 && w_b == 0) {
							if (mat_ib == -2 * KnightValueMg) { return 0; } // v NN
							else if (mat_ib <= -KnightValueMg - BishopValueMg) { egcp += KNOWN_WIN; } // vBN+
							else if (mat_ib == -KnightValueMg) { return 0; } // v N
							else if (mat_ib == -BishopValueMg) { return 0; } // v B

						}
						else if (b_n == 0 && b_b == 0) {
							if (mat_ib == 2 * KnightValueMg) { return 0; } // NN v
							else if (mat_ib >= KnightValueMg + BishopValueMg) { egcp += KNOWN_WIN; } // vBN+
							else if (mat_ib == KnightValueMg) { return 0; } // N v
							else if (mat_ib == BishopValueMg) { return 0; } // B v

						}
						else if (w_n < 2 && w_b < 2 && mat_ib == 0) { return 0; } // N v B, NB v NB, B v B, B v N...
						
					}
					else {
						if (board.non_pawn_material(WHITE) == 0) { egcp -= KNOWN_WIN; } 
						else if (board.non_pawn_material(BLACK) == 0) { egcp += KNOWN_WIN; } // R v
					}
				}
				else {
					if (board.non_pawn_material(WHITE) <= BishopValueMg) { egcp -= KNOWN_WIN; }
					else if (board.non_pawn_material(BLACK) <= BishopValueMg) { egcp += KNOWN_WIN; } // Q v N, Q v B, Q v
				}
			}

			// Opposite Color Bishops
			if (popcount(white_bishops) == 1 && popcount(black_bishops) == 1) {
				if (((white_bishops & LightSquares) && (black_bishops & DarkSquares)) || ((white_bishops & DarkSquares) && (black_bishops & LightSquares))) {
					if (!board.pieces(ROOK) && !board.pieces(KNIGHT) && !board.pieces(QUEEN)) { winnable = winnable * 30 / 100; }
					else { winnable = winnable * 70 / 100; }
				}
			}

			// Checkmate with King
			if (white_k_adj & black_k_adj) {
				white_attacker_value += EG[2];
				black_attacker_value += EG[2];
				if (black_king & BoardEdge) { black_attacker_value += EG[3]; }
				if (white_king & BoardEdge) { white_attacker_value += EG[3]; }
			}
		}

		// Active / Passive
		int attack_imbalance = 0;

		// Shielded Minor Pieces
		score += ShieldedMinor * popcount(shift<NORTH>(board.pieces(WHITE, KNIGHT, BISHOP)) & board.pieces(PAWN));
		score -= ShieldedMinor * popcount(shift<SOUTH>(board.pieces(BLACK, KNIGHT, BISHOP)) & board.pieces(PAWN));

		// Individual Piece Calculations
		Bitboard blocked;
		Bitboard control;

		while (white_knights) {
			s = pop_lsb(&white_knights);
			score += psqt[W_KNIGHT][s];
			control = attacks_bb<KNIGHT>(s, occupied);
			if (control & black_k_adj) { black_attacker_value += KS[0]; }
			if (control & white_k_adj) { white_attacker_value -= KS[1]; }
			w_space |= control;

			attack_imbalance += popcount(control & board.pieces(BLACK));

			// Outpost Detection
			if (rank_of(s) > 3 && rank_of(s) < 6 && !(adjacent_files_bb(s) & forward_ranks_bb(WHITE, s) & board.pieces(BLACK, PAWN))) {
				score += KnightOutpost;
			}
		}
		while (black_knights) {
			s = pop_lsb(&black_knights);
			score += psqt[B_KNIGHT][s];
			control = attacks_bb<KNIGHT>(s, occupied);
			if (control & white_k_adj) { white_attacker_value += KS[0]; }
			if (control & black_k_adj) { black_attacker_value -= KS[1]; }
			b_space |= control;

			attack_imbalance -= popcount(control & board.pieces(WHITE));

			// Outpost Detection
			if (rank_of(s) < 4 && rank_of(s) > 1 && !(adjacent_files_bb(s) & forward_ranks_bb(BLACK, s) & board.pieces(WHITE, PAWN))) {
				score -= KnightOutpost;
			}
		}
		while (white_bishops) {
			s = pop_lsb(&white_bishops);
			score += psqt[W_BISHOP][s];
			control = attacks_bb<BISHOP>(s, occupied ^ board.pieces(WHITE, QUEEN, BISHOP));
			if (control & black_k_adj) { black_attacker_value += KS[2]; }
			if (control & white_k_adj) { white_attacker_value -= KS[3]; }
			w_space |= control;

			attack_imbalance += popcount(control & board.pieces(BLACK));

			// Activity
			score += Activity[0] * popcount(control);
			score += Activity[1] * popcount(control & Black6L);

		}
		while (black_bishops) {
			s = pop_lsb(&black_bishops);
			score += psqt[B_BISHOP][s];
			control = attacks_bb<BISHOP>(s, occupied ^ board.pieces(BLACK, QUEEN, BISHOP));
			if (control & white_k_adj) { white_attacker_value += KS[2]; }
			if (control & black_k_adj) { black_attacker_value -= KS[3]; }
			b_space |= control;

			attack_imbalance -= popcount(control & board.pieces(WHITE));

			// Activity
			score -= Activity[0] * popcount(control);
			score -= Activity[1] * popcount(control & White6L);

		}
		while (white_rooks) {
			s = pop_lsb(&white_rooks);
			score += psqt[W_ROOK][s];
			blocked = occupied ^ board.pieces(WHITE, QUEEN, ROOK);
			control = attacks_bb<ROOK>(s, blocked);
			if (control & black_k_adj) { black_attacker_value += KS[4]; }
			if (control & white_k_adj) { white_attacker_value -= KS[5]; }
			w_space |= control;

			attack_imbalance += popcount(control & board.pieces(BLACK));

			// Activity
			score += Activity[2] * popcount(control & Black6L);

			// Stuck Rook
			if (popcount(control) < 6 && ((file_of(w_king) < FILE_E) == (file_of(s) < file_of(w_king)))) {
				score -= StuckRook * (6 - popcount(control)) * (1 + !board.castling_rights(WHITE));
			}

			// rooks on open file
			if (!(blocked & file_bb(s))) {
				score += OpenFileRook;
			}
			else if (board.is_on_semiopen_file(WHITE, s)) {
				score += SemiOpenRook;
			}

			// Connect Rooks
			if (board.pieces(WHITE, QUEEN, ROOK) & file_bb(s)) {
				score += ConnectedRook;
			}
		}
		while (black_rooks) {
			s = pop_lsb(&black_rooks);
			score += psqt[B_ROOK][s];
			blocked = occupied ^ board.pieces(BLACK, QUEEN, ROOK);
			control = attacks_bb<ROOK>(s, blocked);
			if (control & white_k_adj) { white_attacker_value += KS[4]; }
			if (control & black_k_adj) { black_attacker_value -= KS[5]; }
			b_space |= control;

			attack_imbalance -= popcount(control & board.pieces(WHITE));

			// Activity
			score -= Activity[2] * popcount(control & White6L);

			// Stuck Rook
			if (popcount(control) < 6 && ((file_of(b_king) < FILE_E) == (file_of(s) < file_of(b_king)))) {
				score += StuckRook * (6 - popcount(control)) * (1 + !board.castling_rights(BLACK));
			}

			// rooks on open file
			if (!(blocked & file_bb(s))) {
				score -= OpenFileRook;
			}
			else if (board.is_on_semiopen_file(BLACK, s)) {
				score -= SemiOpenRook;
			}

			// Connect Rooks
			if (board.pieces(BLACK, QUEEN, ROOK) & file_bb(s)) {
				score -= ConnectedRook;
			}
		}

		while (white_queens) {
			s = pop_lsb(&white_queens);
			score += psqt[W_QUEEN][s];
			blocked = occupied ^ board.pieces(WHITE, QUEEN, ROOK);
			control = attacks_bb<QUEEN>(s, blocked ^ board.pieces(WHITE, BISHOP));
			if (control & black_k_adj) { black_attacker_value += KS[6]; }
			if (control & white_k_adj) { white_attacker_value -= KS[7]; }

			// Activity
			score += Activity[2] * popcount(control & Black6L);

			// Pinned Queen
			Bitboard pinners;
			if (board.slider_blockers(board.pieces(BLACK, ROOK, BISHOP), s, pinners)) {
				score -= PinnedQueen;
			}
		}
		while (black_queens) {
			s = pop_lsb(&black_queens);
			score += psqt[B_QUEEN][s];
			blocked = occupied ^ board.pieces(BLACK, QUEEN, ROOK);
			control = attacks_bb<QUEEN>(s, blocked ^ board.pieces(BLACK, BISHOP));
			if (control & white_k_adj) { white_attacker_value += KS[6]; }
			if (control & black_k_adj) { black_attacker_value -= KS[7]; }

			// Activity
			score -= Activity[2] * popcount(control & White6L);

			// Pinned Queen
			Bitboard pinners;
			if (board.slider_blockers(board.pieces(WHITE, ROOK, BISHOP), s, pinners)) {
				score += PinnedQueen;
			}
		}

		if (mg < 64) { // King to Passed Pawns / Pawns
			Bitboard w_pawns_d = w_pawns;
			Bitboard b_pawns_d = b_pawns;

			while (w_pawns_d) {
				s = pop_lsb(&w_pawns_d);
				egcp -= distance<Square>(w_king, s) * EG[0];
				egcp += distance<Square>(b_king, s) * EG[0];
				if (board.pawn_passed(WHITE, s)) {
					egcp -= distance<Square>(w_king, s) * EG[1];
					egcp += distance<Square>(b_king, s) * EG[1];
				}
			}
			while (b_pawns_d) {
				s = pop_lsb(&b_pawns_d);
				egcp -= distance<Square>(w_king, s) * EG[0];
				egcp += distance<Square>(b_king, s) * EG[0];
				if (board.pawn_passed(BLACK, s)) {
					egcp -= distance<Square>(w_king, s) * EG[1];
					egcp += distance<Square>(b_king, s) * EG[1];
				}
			}
		}

		score += psqt[W_KING][w_king];
		score += psqt[B_KING][b_king];

		// Pawn Mobility
		score -= BlockedPawn * popcount(shift<NORTH>(w_pawns) & occupied);
		score += BlockedPawn * popcount(shift<SOUTH>(b_pawns) & occupied);

		// Space Advantage
		score += (popcount(w_space & ~b_space) - popcount(b_space & ~w_space)) * Activity[3];

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
			score += Pawn_TT.probe(board.pawn_key(), board);
		}

		// Finalize
		mgcp += mg_value(score);
		egcp += eg_value(score);

		// Winnability
		if (mg < 25) {
			if (abs(board.non_pawn_material(WHITE) - board.non_pawn_material(BLACK)) < 800) {
				if (egcp > 0 && popcount(w_pawns) < 3) {
					winnable = winnable * (popcount(w_pawns) + 1) >> 2;
				}
				else if (egcp < 0 && popcount(b_pawns) < 3) {
					winnable = winnable * (popcount(b_pawns) + 1) >> 2;
				}
			}
		}
		egcp = egcp * winnable / 100;
		
		// final mg+eg
		int cp_ = mg * mgcp + (128 - mg) * egcp;
		if (board.rule50_count() > 10) { cp_ = (110 - board.rule50_count()) / 100; }
		cp_ >>= 7;

		if (board.side_to_move()) { cp_ *= -1; }

		// Tempo
		cp_ += Tempo;

		if (limit_strength) {
			cp_ = cp_ * 100 / (100 + 2 * material_bias);
			perturb_diff(cp_, ls_p_max);
		}

		return cp_;
	}

	int eval(Stockfish::Position& board, int recursion, int alpha, int beta)
	{
		// Move Generation
		Stockfish::MoveList<Stockfish::GenType::LEGAL> legal_moves = Stockfish::MoveList<Stockfish::GenType::LEGAL>(board);

		// Mates and Stalemates
		if (!legal_moves.size()) {
			if (board.checkers()) { return EVAL_LOSS; }
			else { return 0; }
		}

		// repetition
		if (board.repetition() < 0) { return 0; }

		// 50-move
		if (board.rule50_count() > 99) { return 0; }

		int new_eval = EVAL_LOSS;
		int comp_eval;
		
		//depth restriction
		if (recursion < 1) {
			new_eval = end_eval(board);
			// Stand Pat
			if (new_eval > alpha) { alpha = new_eval; }
			if (alpha > beta) { return new_eval; }

			if (new_eval > 10000) { new_eval++; }
			else if (new_eval < -10000) { new_eval--; }
			if (alpha > 10000) { alpha++; }
			else if (alpha < -10000) { alpha--; }
			if (beta > 10000) { beta++; }
			else if (beta < -10000) { beta--; } // For Mate Distance Comparison

			Value margin = alpha - new_eval - DELTA_MARGIN > 0 ?
				static_cast<Value>(alpha - new_eval - DELTA_MARGIN) : VALUE_ZERO; // Delta Pruning

			for (auto move = legal_moves.begin(); move != legal_moves.end(); move++) {
				if (board.capture_or_promotion(*move)) {
					if (board.see_ge(move->move, margin)) { // SEE > 0 Extension
						board.do_move(*move);
						comp_eval = -eval(board, 0, -beta, -alpha);
						board.undo_move(*move);
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

		if (board.checkers()) { // Checked
			
			if (alpha > 10000) { alpha++; }
			else if (alpha < -10000) { alpha--; }
			if (beta > 10000) { beta++; }
			else if (beta < -10000) { beta--; } // For Mate Distance Comparison

			for (auto move = legal_moves.begin(); move != legal_moves.end(); move++) {
				board.do_move(*move);
				comp_eval = -eval(board, recursion - 1, -beta, -alpha);
				board.undo_move(*move);
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

			// Stand Pat
			if (new_eval > alpha) { alpha = new_eval; }
			if (alpha > beta) { return new_eval; }

			if (new_eval > 10000) { new_eval++; }
			else if (new_eval < -10000) { new_eval--; }
			if (alpha > 10000) { alpha++; }
			else if (alpha < -10000) { alpha--; }
			if (beta > 10000) { beta++; }
			else if (beta < -10000) { beta--; } // For Mate Distance Comparison

			Value margin = alpha - new_eval - DELTA_MARGIN > 0 ?
				static_cast<Value>(alpha - new_eval - DELTA_MARGIN) : VALUE_ZERO; // Delta Pruning

			// Quiescence Expansion
			for (auto move = legal_moves.begin(); move != legal_moves.end(); move++) {
				if (board.capture_or_promotion(*move)) {
					if (board.see_ge(move->move, margin)) {
						board.do_move(*move);
						comp_eval = -eval(board, recursion - 1, -beta, -alpha);
						board.undo_move(*move);
						if (comp_eval > new_eval) { 
							new_eval = comp_eval;
							if (comp_eval > alpha) { 
								alpha = comp_eval;
								if (alpha >= beta) { break; }
							}
						}
					}
				}
				else if (board.gives_check(*move)) {
					board.do_move(*move);
					comp_eval = -eval(board, recursion - 1, -beta, -alpha);
					board.undo_move(*move);
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
			return "cp " + to_string(eval * 100 / PawnValueMg);
		}
	}

	int perturb_diff(int& eval, int max) {
		// add small perturbation to evaluation to limit playing strength
		eval += rand() % (max * 2 + 1) - max;
		return eval;
	}

	bool nh_condition(const Stockfish::Position& board) {
		if (board.checkers()) { return false; }
		if (!board.non_pawn_material()) { return false; }
		if (popcount(board.pieces()) < 8) { return false; }
		return true;
	}

}