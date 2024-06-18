#include "alphabeta.h"

namespace AGI {

int alpha_beta(Position& board, atomic<bool>* stop,
	int ply, TTEntry* probe,
	Color root_c, int step,
	int alpha, int beta,
	int root_dist) 
{

	if (*stop) { return EVAL_FAIL; }

	if (ply <= 0) { // QSearch
		int qeval = eval(board, 0, alpha, beta);
		return qeval;
	}

	else {
		// Move Generation
		MoveList legal_moves;
		legal_moves.generate(board);

		Key root_key = board.get_key();
		int draw_value = board.get_side() == root_c ? -contempt : 0;

		// Mates and Stalemates
		if (!legal_moves.length()) {
			if (board.get_checkers()) {
				Main_TT.register_entry(root_key, 0, EVAL_LOSS, NULL_MOVE);
				return EVAL_LOSS;
			}
			else { 
				Main_TT.register_entry(root_key, 0, draw_value, NULL_MOVE);
				return draw_value;
			}
		}

		else {
			// Null Move Heuristic
			if (ply < NULLMOVE_MAX_PLY && board.get_nh_condition()) {
				Undo u;
				board.do_null_move(&u);
				int null_value = -eval(board, 0, -beta, -alpha);
				board.undo_null_move();

				if (null_value > beta) { return null_value; }
			}

			Move m;
			Move nmove = probe->nmove;
			int comp_eval, index;
			int new_eval = EVAL_INIT;
			int ply_low, reduction;
			root_dist++;

			index = legal_moves.find_index(nmove);
			int i = 0;
			while ((i < legal_moves.length()) && !(*stop)) {
				index = index % legal_moves.length();
				m = *(legal_moves.list + index);

				Undo u;
				board.do_move(m, &u);
				TTEntry probe_m = {};

				// Repetition + 50-move
				if (board.get_repetition(root_dist) ||
					(board.get_fiftymove() > 99 && !board.get_checkers())) 
				{
					comp_eval = draw_value;
				}
				// Probe Table and Search
				else
				{
					if (Main_TT.probe(board.get_key(), &probe_m) == EVAL_FAIL)
					{ // Table Miss
						reduction = 0;
					}
					else
					{ // Table Hit
						comp_eval = -probe_m.eval;
						dec_mate(comp_eval);
						reduction = probe_m.depth + 1;
					}

					while (reduction < ply) {
						if ((reduction > ply / 3)
							&& (m != nmove)) 
						{
							if (comp_eval < new_eval) { break; }
						}

						comp_eval = -alpha_beta(board, stop,
							reduction, &probe_m,
							root_c, step,
							-beta, -alpha,
							root_dist
						);
						dec_mate(comp_eval);

						reduction++;
					}
				}
				
				if (comp_eval > new_eval && !(*stop)) {
					new_eval = comp_eval;
					nmove = m;
					Main_TT.register_entry(root_key, ply - 1, new_eval, nmove);
					if (new_eval > alpha) { alpha = new_eval; }
					if (alpha > beta) {
						board.undo_move(m);
						break;
					}
				}

				board.undo_move(m);
				i++;
				index += step;
			}

			if (!(*stop)) {
				Main_TT.register_entry(root_key, ply, new_eval, nmove);
			}

			return new_eval;
		}
	}

}

} // Namespace
