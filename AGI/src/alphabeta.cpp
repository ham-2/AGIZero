#include "alphabeta.h"

namespace AGI {

	int alpha_beta(Position& board, atomic<bool>* stop, SearchStack* ss,
		int ply, int* prev_red,
		int alpha, int beta,
		int scb_us, int scb_them) 
	{

		if (*stop) { return 0; }

		// Contempt
		int draw_value = ss->currdist() % 2 ? contempt : 0;

		// We have to check these because TT does not contain repetition/50move info
		// repetition after the root or threefold
		if (board.repetition() && (board.repetition() <= ss->currdist())) { *prev_red = ply; return draw_value; }

		// 50-move
		if (board.rule50_count() > 99 && !board.checkers()) { *prev_red = ply; return draw_value; }

		// Table Check
		int probe = Main_TT.probe(board.key(), ply);
		if (probe != 10000) { *prev_red = ply; return probe; } // Table Hit

		// QSearch
		if (ply == 0) {
			*prev_red = 0;
			return eval(board, QUIESCENCE_DEPTH, alpha, beta);
		}

		else { // Table Miss
			int delta = INCREMENT_DELTA * ply;
			if (board.non_pawn_material() <= 4000) { delta += ENDGAME_DELTA; }
			else { delta += MIDDLEGAME_DELTA; }

			// Move Generation
			MoveList<GenType::LEGAL> legal_moves = MoveList<GenType::LEGAL>(board);

			// Mates and Stalemates
			if (!legal_moves.size()) {
				*prev_red = ply;
				if (board.checkers()) { return EVAL_LOSS; }
				else { return draw_value; }
			}

			else {
				int comp_eval;
				int new_eval = -30001;
				int new_depth = 0;
				int currdepth = 0;

				// Null Move Heuristic
				if (nh_condition(board)) {
					StateInfo* st = new StateInfo;
					board.do_null_move(*st);
					int null_value = -eval(board, NULL_DEPTH, -beta, -alpha);
					board.undo_null_move();

					if (null_value > beta) { *prev_red = 0; return null_value; }
				}

				int old_alpha = alpha;
				int old_beta  = beta;
				Move pbmove;
				Move nmove = ss->nmove();
				if (nmove == MOVE_NULL) { nmove = legal_moves.begin()->move; }
				ExtMove* move;
				int reduction, index;
				int step = Primes[ss->thread_id];
				bool nmove_draw;
				int scb_new = scb_us;
				int iter = 0;
				int lastnew;

				while (true) {
					alpha = old_alpha;
					beta = old_beta;
					new_eval = -30001;

					// Move Ordering and pre probing
					for (auto m = legal_moves.begin(); m != legal_moves.end(); m++) {
						pbmove = MOVE_NULL;
						((ExtMove*)m)->value = -Main_TT.probe(board.real_key_after(*m), 1, &pbmove, &currdepth);
						((ExtMove*)m)->pbmove = pbmove;
						((ExtMove*)m)->depth = currdepth;
						if (m->move == nmove) { reduction = currdepth; ss->push(0, pbmove); }
					}
					legal_moves.sort();

					// Get second best value
					scb_new = max(scb_us, (legal_moves.begin() + 1)->value);
					
					// nmove check
					if (ss->pv_limit() >= 0) {
						board.do_move(nmove);
						reduction = reduction > ply - 2 ? ply - 1 : reduction + 1;
						comp_eval = -alpha_beta(board, stop, ss, reduction, &currdepth, -beta, -alpha, scb_them, scb_new);
						while (++reduction < ply && comp_eval >= scb_new) {
							comp_eval = -alpha_beta(board, stop, ss, reduction, &currdepth, -beta, -alpha, scb_them, scb_new);
						}
						if (comp_eval > 10000) { comp_eval--; }
						else if (comp_eval < -10000) { comp_eval++; }
						board.undo_move(nmove);
					}
					else { // Parallel Search
						comp_eval = -Main_TT.probe(board.real_key_after(nmove), 1);
						currdepth = ply - 1;
						if (comp_eval > 10000) { comp_eval--; }
						else if (comp_eval < -10000) { comp_eval++; }
					}
					ss->pop();

					if (!(*stop)) {
						lastnew = new_eval;
						new_eval = comp_eval;
						new_depth = currdepth + 1;
						if (new_eval > alpha && comp_eval != 0) { alpha = new_eval; }
						if (alpha >= beta + delta) {
							*prev_red = new_depth;
							Main_TT.register_entry(board.key(), new_depth, new_eval, nmove);
							return new_eval;
						}
					}

					// if nmove draws, search other moves
					nmove_draw = (comp_eval == 0);

					// Assign depths
					reduction = ply; // Reductions
					if (ply > 1 && ss->thread_id) { reduction++; } // parallel
					for (int i = 0; i < legal_moves.size(); i++) {
						move = legal_moves.list() + i;
						if (nmove_draw) {
							move->depth = ply - 1;
							if (move->value != 0 && move->move != nmove) { nmove_draw = false; }
						}
						else {
							move->depth = Reductions[reduction];
							if (reduction > 11) { reduction--; } // late move reductions

							if (move->depth < 3 && move->value != -10000) { // Futility Pruning
								if (alpha - move->value > FUTILITY_2_MARGIN) { move->depth = 0; }
								else if (move->depth < 2 && alpha - move->value > FUTILITY_1_MARGIN) { move->depth = 0; }
							}

							if (board.advanced_pawn_push(*move) && move->depth < 4) { move->depth++; } // Extend Pawn Pushes
						}
					}

					index = 0;
					for (int i = 0; i < legal_moves.size() && !(*stop); i++) {
						index += step;
						index = index % legal_moves.size();
						move = legal_moves.list() + index;
						if (nmove == *move) { continue; }

						reduction = move->depth;
						board.do_move(*move);
						ss->push(-(move->value), move->pbmove);

						comp_eval = -alpha_beta(board, stop, ss, reduction, &currdepth, -beta, -alpha, scb_them, scb_new);
						if (comp_eval > 10000) { comp_eval--; }
						else if (comp_eval < -10000) { comp_eval++; }
						if (comp_eval > new_eval) {
							lastnew = new_eval;
							new_eval = comp_eval;
							new_depth = currdepth + 1;
							nmove = *move;
							if (new_eval > alpha) { alpha = new_eval; }
							if (alpha >= beta + delta) {
								ss->pop();
								board.undo_move(*move);
								break;
							}
						}

						ss->pop();
						board.undo_move(*move);
					}

					if (*stop || old_alpha > EVAL_LOSS || old_beta < EVAL_WIN || new_depth == ply || new_eval < scb_us || -new_eval < scb_them) { break; }
				}


				if (*stop) { // Time termination - adjust best move
					int depth, depth_b;
					if (new_depth != ply) { new_eval = EVAL_LOSS; }
					for (auto m = legal_moves.begin(); m != legal_moves.end(); m++) {
						comp_eval = -Main_TT.probe(board.real_key_after(*m), 1, &pbmove, &depth);
						if (comp_eval != -10000) {
							if (comp_eval > 10000) { comp_eval--; }
							else if (comp_eval < -10000) { comp_eval++; }
							depth_b = ply - 1 - depth > 0 ? ply - 1 - depth : 0;
							if (comp_eval - SEARCH_DEPTH_BIAS * DEPTH_BIASES[depth_b] > new_eval) {
								new_eval = comp_eval;
								nmove = *m;
								new_depth = depth + 1;
							}
						}
					}
				}

				Main_TT.register_entry(board.key(), new_depth, new_eval, nmove);
				*prev_red = new_depth;
				return new_eval;
			}
		}

	}
}
