#ifndef TABLE_INCLUDED
#define TABLE_INCLUDED

#include <cstring>
#include <thread>
#include <mutex>

#include "constants.h"
#include "position.h"

using namespace std;

namespace AGI {
	extern int num_threads;

	class spinlock {
		std::atomic_flag locked = ATOMIC_FLAG_INIT;
	public:
		void lock() {
			while (locked.test_and_set(std::memory_order_acquire)) { ; }
		}
		void unlock() {
			locked.clear(std::memory_order_release);
		}
	};

	struct TTEntry {
		Key key;
		int table_sn;
		Move nmove;
		short depth;
		short eval;
		spinlock m;
		Key pad;
	};

	struct TT {
		TTEntry* table;
		
		static size_t TT_MB_SIZE;
		static uint64_t TT_LENGTH;
		static int tt_sn;

		TT();
		~TT();

		int probe(Key key, int depth);
		int probe(Key key, int depth, Move* move);
		int probe(Key key, int depth, Move* move, int* pdepth);
		void register_entry(Key key, int depth, int eval, Move move);
		TTEntry* backup_entry(Key key);
		void write_entry(TTEntry* ptr);
		void clear_entry(Key key);
		void clear();
		void change_size(size_t new_size);
		void increment() { tt_sn++; }
	};
	

	extern TT Main_TT;

	struct PTTEntry {
		Key pawnkey;
		Score pawn_eval;
		spinlock m;
	};

	struct PTT {
		PTTEntry* table;
		//static constexpr size_t TT_MB_SIZE = PAWN_TABLE_MB;
		//static constexpr int TT_LENGTH = TT_MB_SIZE * 1024 * 1024 / sizeof(PTTEntry);

		static size_t TT_MB_SIZE;
		static uint64_t TT_LENGTH;

		PTT();
		~PTT();

		Score probe(Key pawnkey, Position& board);
		void clear();
		void change_size(size_t new_size);
	};

	extern PTT Pawn_TT;

	void init_table();
}

#endif