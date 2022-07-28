#ifndef VAR_TREE_INCLUDED
#define VAR_TREE_INCLUDED

#include <atomic>
#include <mutex>
#include <vector>
#include "types.h"
#include "movegen.h"
#include "alphabeta.h"

using namespace std;
using namespace Stockfish;
namespace AGI {

	struct threadmgr {
		atomic<bool> stop;
		vector<mutex*> vm;
		atomic<int> depth;
		vector<bool*> kill;
		vector<Position*> boards;

		void init() {
			//vm.push_back(new mutex);
			//bool* nb = new bool;
			//*nb = false;
			//kill.push_back(nb);
		}

		void add_one(string fen) {
			Position* n_board = new Position;
			boards.push_back(n_board);
			StateInfo* si = new Stockfish::StateInfo;
			n_board->set(fen, false, si);
			vm.push_back(new mutex);
			bool* nb = new bool;
			*nb = false;
			kill.push_back(nb);
		}

		void do_move(Move m) {
			for (auto board = boards.begin(); board != boards.end(); board++) {
				(*board)->do_move(m);
			}
		}

		void set_fen(string fen) {
			for (auto b = boards.begin(); b != boards.end(); b++) {
				Stockfish::StateInfo* si = new Stockfish::StateInfo;
				(*b)->set(fen, false, si);
			}
		}

		void sync();

	};

	void lazy_smp(Position* board, int i);

	int lsmp_alpha_beta(Position* board, SearchStack* ss, int ply, int ply_bound, int step, int alpha, int beta);

	extern condition_variable t_wait;
	extern int SEARCH_THREADS;
	extern threadmgr Threads;

}



#endif