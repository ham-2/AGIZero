#include "alphabeta.h"

namespace AGI {

int alpha_beta(Position& board, atomic<bool>* stop,
	int ply, int* prev_red, 
	Color root_c, int step,
	int alpha, int beta,
	int root_dist) 
{

	if (*stop) { return 0; }

	// Contempt
	int draw_value = board.get_side() == root_c ? contempt : -contempt;

	if (board.get_repetition(root_dist)) { *prev_red = ply; return draw_value; }

	// 50-move
	if (board.get_fiftymove() > 99 && !board.get_checkers()) { *prev_red = ply; return draw_value; }

	// Table Check
	Move nmove = NULL_MOVE;
	int probe = Main_TT.probe(board.get_key(), ply);
	if (probe != EVAL_FAIL) { *prev_red = ply; return probe; } // Table Hit

	// Table Miss
	if (ply == 0) { // QSearch
		*prev_red = 0;
		return eval(board, QUIESCENCE_DEPTH, alpha, beta);
	}

	else { 
		int delta = DEFAULT_DELTA + INCREMENT_DELTA * ply;
		int ply_low = ply / 3;
		if (board.get_material_t() <= 4000) {
			delta += ENDGAME_DELTA;
			//ply_low = ply / 2;
		}

		// Move Generation
		MoveList legal_moves;
		legal_moves.generate(board);

		// Mates and Stalemates
		if (!legal_moves.length()) {
			*prev_red = ply;
			if (board.get_checkers()) { return EVAL_LOSS; }
			else { return draw_value; }
		}

		else {
			// Null Move Heuristic
			if (ply < NULLMOVE_MAX_PLY && board.get_nh_condition()) {
				Undo u;
				board.do_null_move(&u);
				int null_value = -eval(board, NULL_DEPTH, -beta, -alpha);
				board.undo_null_move();

				if (null_value > beta) { *prev_red = ply; return null_value; }
			}

			Move m;
			Main_TT.probe(board.get_key(), 0, &nmove);
			bool table_miss = (nmove == NULL_MOVE);
			int comp_eval, index;
			int new_eval = EVAL_INIT;
			int new_depth = 0;
			int currdepth = 0;
			int reduction = table_miss ? ply_low : ply - 1;
			bool nmove_draw = false;

			index = legal_moves.find_index(nmove);
			int i = 0;
			while ((table_miss || i < legal_moves.length()) && !(*stop)) {
				index = index % legal_moves.length();
				m = *(legal_moves.list + index);

				//if (board.capture_or_promotion(m) ||
				//	board.is_passed_pawn_push(m, board.get_side())) {
				//	reduction += 3;
				//	if (reduction > ply - 1) { reduction = ply - 1; }
				//}

				if (i == legal_moves.length()) {
					reduction = ply - 1;
					m = nmove;
					table_miss = false;
				}
				Undo u;
				board.do_move(m, &u);
				comp_eval = -alpha_beta(board, stop,
					reduction, &currdepth,
					root_c, step,
					-beta, -alpha,
				    root_dist + 1
				);
				dec_mate(comp_eval);

				if (i == 0) { nmove_draw = (comp_eval == draw_value); }

				if (comp_eval > new_eval) {
					new_eval = comp_eval;
					new_depth = currdepth + 1;
					nmove = m;
					if (new_eval > alpha) { alpha = new_eval; }
					if (alpha >= beta + delta) {
						board.undo_move(m);
						break;
					}
				}

				board.undo_move(m);
				i++;
				index += step;
				reduction = ply_low;
			}

			if (!(*stop)) {
				Main_TT.register_entry(board.get_key(), new_depth, new_eval, nmove);
			}

			*prev_red = new_depth;
			return new_eval;
		}
	}

}

} // Namespace
