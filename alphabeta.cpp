#include "alphabeta.h"

namespace AGI {

int alpha_beta(Position& board, atomic<bool>* stop,
	int ply, TTEntry* probe,
	Color root_c, int step,
	int alpha, int beta,
	int root_dist) 
{

	if (*stop) { return 0; }

	int draw_value = board.get_side() == root_c ? contempt : -contempt;

	if (ply <= 0) { // QSearch
		return eval(board, QUIESCENCE_DEPTH, alpha, beta);
	}

	else { 
		int delta = DEFAULT_DELTA + INCREMENT_DELTA * ply;
		if (board.get_material_t() <= 4000) {
			delta += ENDGAME_DELTA;
		}

		// Move Generation
		MoveList legal_moves;
		legal_moves.generate(board);

		// Mates and Stalemates
		if (!legal_moves.length()) {
			if (board.get_checkers()) {
				Main_TT.register_entry(board.get_key(), 0, EVAL_LOSS, NULL_MOVE, 0);
				return EVAL_LOSS;
			}
			else { 
				Main_TT.register_entry(board.get_key(), 0, draw_value, NULL_MOVE, 0);
				return draw_value;
			}
		}

		else {
			// Null Move Heuristic
			if (ply < NULLMOVE_MAX_PLY && board.get_nh_condition()) {
				Undo u;
				board.do_null_move(&u);
				int null_value = -eval(board, NULL_DEPTH, -beta, -alpha);
				board.undo_null_move();

				if (null_value > beta) { return null_value; }
			}

			Move m;
			Move nmove = probe->nmove;
			Key root_key = board.get_key();
			bool table_temp = false;
			int comp_eval, index;
			int new_eval = EVAL_INIT;
			int ply_low, reduction;
			root_dist++;

			if (nmove == NULL_MOVE && ply > 1) {
				int nmove_eval = EVAL_INIT;
				for (int i = 0; i < legal_moves.length(); i++) {
					// if (*stop) { break; }
					m = *(legal_moves.list + i);

					Undo u;
					board.do_move(m, &u);
					TTEntry probe_m = {};
					if (Main_TT.probe(board.get_key(), &probe_m) != EVAL_FAIL) {
						if (-probe_m.eval > nmove_eval) {
							nmove = m;
							nmove_eval = -probe_m.eval;
						}
					}
					board.undo_move(m);
				}
			}

			index = legal_moves.find_index(nmove);
			int i = 0;
			while ((i < legal_moves.length()) && !(*stop)) {
				index = index % legal_moves.length();
				m = *(legal_moves.list + index);

				Undo u;
				board.do_move(m, &u);

				bool skip = false;

				// Threefold + 50-move
				if (board.get_repetition(root_dist) ||
					(board.get_fiftymove() > 99 && !board.get_checkers())) 
				{
					skip = true;
					comp_eval = -draw_value;
				}

				// Table Check
				TTEntry probe_m = {};
				if (Main_TT.probe(board.get_key(), &probe_m) == EVAL_FAIL)
				{ // Table Miss
					probe_m.volatility = volatility_eval(board);
					reduction = 0;
				}
				else
				{ // Table Hit
					comp_eval = -probe_m.eval;
					dec_mate(comp_eval);
					reduction = probe_m.depth;
				}

				ply_low = ply / 3 + probe_m.volatility / 20;
				if (ply_low > ply - 1) { ply_low = ply - 1; }

				//if ((reduction >= ply_low) && (m != nmove)) {
				//	if (comp_eval < new_eval) { skip = true; }
				//	else { reduction++; }
				//}

				table_temp = false;
				while (!skip && reduction < ply) {
					comp_eval = -alpha_beta(board, stop,
						reduction, &probe_m,
						root_c, step,
						-beta, -alpha,
						root_dist
					);
					dec_mate(comp_eval);

					if ((reduction >= ply_low) && (m != nmove)) {
						if (comp_eval < new_eval) { break; }
						if (!(*stop) && !table_temp) {
							table_temp = true;
							Main_TT.register_entry(root_key, ply - 1, comp_eval, m, probe->volatility);
						}
					}
					reduction++;
				}

				if (comp_eval > new_eval) {
					new_eval = comp_eval;
					nmove = m;
					if (new_eval > alpha) { alpha = new_eval; }
					if (alpha >= beta + delta + probe_m.volatility) {
						board.undo_move(m);
						break;
					}
				}
				else if (table_temp) {
					Main_TT.register_entry(root_key, ply - 1, new_eval, nmove, probe->volatility);
					table_temp = false;
				}

				board.undo_move(m);
				i++;
				index += step;
			}

			if (!(*stop)) {
				Main_TT.register_entry(root_key, ply, new_eval, nmove, probe->volatility);
			}

			return new_eval;
		}
	}

}

} // Namespace
