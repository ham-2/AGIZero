#ifndef BENCH_INCLUDED
#define BENCH_INCLUDED

#include <cstdint>

#include "position.h"
#include "printer.h"
#include "movegen.h"

void perft(Position* board, int depth);

#endif