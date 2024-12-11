#ifndef SEARCH_INCLUDED
#define SEARCH_INCLUDED

#include <cmath>
#include <sstream>
#include <algorithm>

#include "alphabeta.h"
#include "eval.h"
#include "position.h"
#include "printer.h"
#include "table.h"
#include "threads.h"

void get_time(istringstream& ss, Color c, float& time, float& max_time, bool& force_time, int& max_ply);

void clear1ply(Position& board);

void search_start(Thread* t, float time, float max_time, bool force_time, int max_ply);

extern bool lichess_timing;
extern bool stop_if_mate;
extern bool ponder;
extern atomic<bool> ponder_continue;
extern int multipv;

#endif
