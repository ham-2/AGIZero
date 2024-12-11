#include "alphabeta.h"

int alpha_beta(Position& board, atomic<bool>* stop,
	int ply, TTEntry* probe,
	Color root_c, int step,
	int alpha, int beta,
	int root_dist) 
{

	if (*stop) { return EVAL_FAIL; }

	if (ply <= 0) { // QSearch
		probe->type = 0;
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
			probe->type = 0;
			if (board.get_checkers()) {
				Main_TT.register_entry(root_key, EVAL_LOSS, NULL_MOVE, 0, 0);
				return EVAL_LOSS;
			}
			else { 
				Main_TT.register_entry(root_key, draw_value, NULL_MOVE, 0, 0);
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
			Move draw_move = NULL_MOVE;
			int comp_eval, index;
			int new_eval = EVAL_INIT;
			int alpha_i = alpha;
			int ply_low, reduction;
			root_dist++;

			index = legal_moves.find_index(nmove);
			int i = 0;
			probe->type = 0;
			while ((i < legal_moves.length()) && !(*stop)) {
				index = index % legal_moves.length();
				m = *(legal_moves.list + index);

				// Adjust alpha to calculate critical moves
				int lower_alpha = 20;
				if (board.is_check(m)) { lower_alpha += 100; }
				if (board.is_capture(m)) { lower_alpha += 20; }
				if (board.is_passed_pawn_push(m, board.get_side())) { lower_alpha += 200; }

				// Do move
				Undo u;
				board.do_move(m, &u);
				TTEntry probe_m = {};

				// Repetition + 50-move
				if (board.get_repetition(root_dist) ||
					(board.get_fiftymove() > 99 && !board.get_checkers())) 
				{
					draw_move = m;
					comp_eval = EVAL_INIT; // Skip
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
						if (probe_m.depth >= ply - 1 &&
							comp_eval >= beta &&
							probe_m.type != -1) {
							new_eval = comp_eval;
							alpha = comp_eval;
							nmove = m;
							board.undo_move(m);
							probe->type = -1;
							break;
						} // Prune
						if (probe_m.depth >= ply - 1 &&
							comp_eval < new_eval &&
							probe_m.type != 1) {
							board.undo_move(m);
							i++;
							index += step;
							continue;
						} // Skip

						// Search from
						reduction = probe_m.nmove == NULL_MOVE ? ply :
							min(probe_m.depth + 1, ply - 1);
					}

					while (reduction < ply) {
						int lower_alpha_r = lower_alpha / (reduction + 1);

						if ((probe_m.type != 1) &&
							(reduction > ply / 3) &&
							(comp_eval < new_eval - lower_alpha_r))
						{
							break;
						}

						comp_eval = -alpha_beta(board, stop,
							reduction, &probe_m,
							root_c, step,
							-beta, -alpha + lower_alpha_r,
							root_dist
						);
						dec_mate(comp_eval);

						reduction++;
					}
				}

				if (comp_eval > new_eval) {
					new_eval = comp_eval;
					nmove = m;
					if (new_eval > alpha) { alpha = new_eval; }
					if (alpha > beta) {
						board.undo_move(m);
						probe->type = -1;
						break;
					}
				}

				board.undo_move(m);
				i++;
				index += step;
			}

			// if there is a draw
			if (draw_move != NULL_MOVE &&
				new_eval < draw_value) {
				new_eval = draw_value;
				nmove = draw_move;
			}

			if (alpha == alpha_i) { probe->type = 1; }

			if (!(*stop)) {
				Main_TT.register_entry(root_key, new_eval, nmove, ply, probe->type);
			}

			return new_eval;
		}
	}

}
