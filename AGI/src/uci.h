#ifndef UCI_H_INCLUDED
#define UCI_H_INCLUDED

#include <map>
#include <string>

#include "position.h"
#include "types.h"

namespace AGI {
	std::string square(Stockfish::Square s);
	std::string move(Stockfish::Move m);
	Stockfish::Move to_move(const Stockfish::Position& pos, std::string& str);
}
#endif
