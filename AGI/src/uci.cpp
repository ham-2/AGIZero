#include <cassert>
#include <cmath>
#include <iostream>
#include <sstream>
#include <string>

#include "movegen.h"
#include "position.h"
#include "uci.h"

namespace AGI{
    using namespace std;

    /// UCI::square() converts a Square to a string in algebraic notation (g1, a7, etc.)

    std::string square(Stockfish::Square s) {
      return std::string{ char('a' + file_of(s)), char('1' + rank_of(s)) };
    }


    /// UCI::move() converts a Move to a string in coordinate notation (g1f3, a7a8q).
    /// The only special case is castling, where we print in the e1g1 notation in
    /// normal chess mode, and in e1h1 notation in chess960 mode. Internally all
    /// castling moves are always encoded as 'king captures rook'.

    string move(Stockfish::Move m) {

      Stockfish::Square from = from_sq(m);
      Stockfish::Square to = to_sq(m);

      if (m == Stockfish::MOVE_NONE)
          return "(none)";

      if (m == Stockfish::MOVE_NULL)
          return "0000";

      if (type_of(m) == Stockfish::CASTLING)
          to = make_square(to > from ? Stockfish::FILE_G : Stockfish::FILE_C, rank_of(from));

      string move = square(from) + square(to);

      if (type_of(m) == Stockfish::PROMOTION)
          move += " pnbrqk"[promotion_type(m)];

      return move;
    }


    /// UCI::to_move() converts a string representing a move in coordinate notation
    /// (g1f3, a7a8q) to the corresponding legal Move, if any.

    Stockfish::Move to_move(const Stockfish::Position& pos, string& str) {

      if (str.length() == 5) // Junior could send promotion piece in uppercase
          str[4] = char(tolower(str[4]));

      for (const auto& m : Stockfish::MoveList<Stockfish::GenType::LEGAL>(pos))
          if (str == move(m))
              return m;

      return Stockfish::MOVE_NONE;
    }

}