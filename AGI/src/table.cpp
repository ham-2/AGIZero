#include "table.h"


namespace AGI {
	using namespace Stockfish;

	size_t TT::TT_MB_SIZE = TABLE_MB_DEFAULT;
	uint64_t TT::TT_LENGTH = TT::TT_MB_SIZE * 1024 * 1024 / sizeof(TTEntry);

	uint64_t SIZE_NUM = (uint64_t)(TT::TT_LENGTH - 1);

	TT Main_TT;
	int TT::tt_sn;

	TT::TT() {
		table = static_cast<TTEntry*>(aligned_large_pages_alloc(TT_LENGTH * sizeof(TTEntry)));
	}

	TT::~TT() {
		aligned_large_pages_free(table);
	}

	int TT::probe(Key key, int depth) {
		TTEntry* entry_ptr = table + (key & SIZE_NUM);
		entry_ptr->m.lock();
		if (entry_ptr->key == key && entry_ptr->depth >= depth) { // Real hit
			int return_int = entry_ptr->eval;
			entry_ptr->m.unlock();
			return return_int;
		}
		else { // False hit or New node
			entry_ptr->m.unlock();
			return 10000;
		}
	}

	int TT::probe(Key key, int depth, Move* move) {
		TTEntry* entry_ptr = table + (key & SIZE_NUM);
		entry_ptr->m.lock();
		if (entry_ptr->key == key && entry_ptr->depth >= depth) { // Real hit
			int return_int = entry_ptr->eval;
			*move = entry_ptr->nmove;
			entry_ptr->m.unlock();
			return return_int;
		}
		else { // False hit or New node
			entry_ptr->m.unlock();
			return 10000;
		}
	}

	int TT::probe(Key key, int depth, Move* move, int* pdepth) {
		TTEntry* entry_ptr = table + (key & SIZE_NUM);
		entry_ptr->m.lock();
		if (entry_ptr->key == key && entry_ptr->depth >= depth) { // Real hit
			int return_int = entry_ptr->eval;
			*move = entry_ptr->nmove;
			*pdepth = entry_ptr->depth;
			entry_ptr->m.unlock();
			return return_int;
		}
		else { // False hit or New node
			entry_ptr->m.unlock();
			return 10000;
		}
	}

	void TT::register_entry(Key key, int depth, int eval, Move move) {
		TTEntry* entry_ptr = table + (key & SIZE_NUM);
		entry_ptr->m.lock();
		if (entry_ptr->key == key || // Overwrite
			depth >= entry_ptr->depth ||
			tt_sn > entry_ptr->table_sn) {
			entry_ptr->key = key;
			entry_ptr->depth = depth;
			entry_ptr->eval = eval;
			entry_ptr->nmove = move;
			entry_ptr->table_sn = tt_sn;
		}
		entry_ptr->m.unlock();
		return;
	}

	TTEntry* TT::backup_entry(Key key) {
		TTEntry* entry_ptr = table + (key & SIZE_NUM);
		TTEntry* temp = new TTEntry;
		entry_ptr->m.lock();
		memcpy(temp, entry_ptr, sizeof(TTEntry));
		entry_ptr->m.unlock();
		temp->m.unlock();
		return temp;
	}

	void TT::write_entry(TTEntry* ptr) {
		TTEntry* entry_ptr = table + (ptr->key & SIZE_NUM);
		entry_ptr->m.lock();
		ptr->m.lock();
		memcpy(entry_ptr, ptr, sizeof(TTEntry));
		entry_ptr->m.unlock();
	}

	void TT::clear_entry(Key key) {
		TTEntry* entry_ptr = table + (key & SIZE_NUM);
		entry_ptr->m.lock();
		entry_ptr->key = 0;
		entry_ptr->depth = 0;
		entry_ptr->eval = 0;
		entry_ptr->nmove = MOVE_NONE;
		entry_ptr->table_sn = 0;
		entry_ptr->m.unlock();
		return;
	}

	void TT::clear() {

		tt_sn = 1;

		std::vector<std::thread> threads;

		for (size_t idx = 0; idx < SEARCH_THREADS_DEFAULT; ++idx)
		{

			threads.emplace_back([this, idx]() {

				// Each thread will zero its part of the hash table
				const size_t stride = size_t(TT_LENGTH / SEARCH_THREADS_DEFAULT),
					start = size_t(stride * idx),
					len = idx != SEARCH_THREADS_DEFAULT - 1 ?
					stride : TT_LENGTH - start;

				std::memset(&table[start], 0, len * sizeof(TTEntry));
				});
		}

		for (std::thread& th : threads)
			th.join();

	}

	void TT::change_size(size_t new_size) {
		aligned_large_pages_free(table);
		TT_MB_SIZE = new_size;
		TT_LENGTH = TT_MB_SIZE * 1024 * 1024 / sizeof(TTEntry);
		SIZE_NUM = (uint64_t)(TT::TT_LENGTH - 1);
		table = static_cast<TTEntry*>(aligned_large_pages_alloc(TT_LENGTH * sizeof(TTEntry)));
	}

	size_t PTT::TT_MB_SIZE = PAWN_TABLE_MB_DEFAULT;
	uint64_t PTT::TT_LENGTH = PTT::TT_MB_SIZE * 1024 * 1024 / sizeof(PTTEntry);

	uint64_t PSIZE_NUM = (uint64_t)(PTT::TT_LENGTH - 1);

	PTT Pawn_TT;

	PTT::PTT() {
		table = static_cast<PTTEntry*>(aligned_large_pages_alloc(TT_LENGTH * sizeof(PTTEntry)));
	}

	PTT::~PTT() {
		aligned_large_pages_free(table);
	}

	Stockfish::Score PTT::probe(Key pawnkey, Stockfish::Position& board) {
		PTTEntry* entry_ptr = table + (pawnkey & PSIZE_NUM);
		entry_ptr->m.lock();
		if (entry_ptr->pawnkey) { // Index hit
			if (entry_ptr->pawnkey == pawnkey) { // Real hit
				Stockfish::Score return_score = entry_ptr->pawn_eval;
				entry_ptr->m.unlock();
				return return_score;
			}
			else { // False hit
				entry_ptr->pawnkey = pawnkey;
				entry_ptr->pawn_eval = pawn_eval(board);
				Stockfish::Score return_score = entry_ptr->pawn_eval;
				entry_ptr->m.unlock();
				return return_score;
			}
		}
		else { //new node: register
			entry_ptr->pawnkey = pawnkey;
			entry_ptr->pawn_eval = pawn_eval(board);
			Stockfish::Score return_score = entry_ptr->pawn_eval;
			entry_ptr->m.unlock();
			return return_score;
		}
	}

	void PTT::clear() {

		std::vector<std::thread> threads;

		for (size_t idx = 0; idx < SEARCH_THREADS_DEFAULT; ++idx)
		{
			threads.emplace_back([this, idx]() {

				// Each thread will zero its part of the hash table
				const size_t stride = size_t(TT_LENGTH / SEARCH_THREADS_DEFAULT),
					start = size_t(stride * idx),
					len = idx != SEARCH_THREADS_DEFAULT - 1 ?
					stride : TT_LENGTH - start;

				std::memset(&table[start], 0, len * sizeof(PTTEntry));
				});
		}

		for (std::thread& th : threads)
			th.join();

	}

	void PTT::change_size(size_t new_size) {
		aligned_large_pages_free(table);
		TT_MB_SIZE = new_size;
		TT_LENGTH = TT_MB_SIZE * 1024 * 1024 / sizeof(PTTEntry);
		if (new_size > 0) {
			PSIZE_NUM = (uint64_t)(PTT::TT_LENGTH - 1);
			table = static_cast<PTTEntry*>(aligned_large_pages_alloc(TT_LENGTH * sizeof(PTTEntry)));
		}
	}

	void init_table() {
		//static constexpr size_t TT_MB_SIZE = TABLE_MB;
		//static constexpr int TT_LENGTH = TT_MB_SIZE * 1024 * 1024 / sizeof(TTEntry);
		TT::TT_MB_SIZE = TABLE_MB_DEFAULT;
		TT::TT_LENGTH = TT::TT_MB_SIZE * 1024 * 1024 / sizeof(TTEntry);
	}
}

