#ifndef PRINTER_INCLUDED
#define PRINTER_INCLUDED

#include <atomic>
#include <condition_variable>
#include <iostream>
#include <mutex>
#include <sstream>

#include "board.h"
#include "constants.h"
#include "eval.h"
#include "search.h"
#include "table.h"
#include "threads.h"

using namespace std;

bool compare(pair<Move, int> a, pair<Move, int> b);

void printer(float time, atomic<bool>* stop, condition_variable* cv, float max_time, bool force_time);


#endif