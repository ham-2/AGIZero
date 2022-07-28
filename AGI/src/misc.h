#ifndef MISC_INCLUDED
#define MISC_INCLUDED

#include <deque>
#include <cassert>
#include <chrono>
#include <iostream>
#include <sstream>
#include <ostream>
#include <string>
#include <vector>
#include <cstdint>
#include <mutex>

#include "types.h"

namespace AGI {

    std::string getfen(std::deque<std::string> input_split);
    std::deque<std::string> split(const std::string& input_str);

/// Memory allocation function from Stockfish.
/*
  Stockfish, a UCI chess playing engine derived from Glaurung 2.1
  Copyright (C) 2004-2021 The Stockfish developers (see AUTHORS file)

  Stockfish is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  Stockfish is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
void* std_aligned_alloc(size_t alignment, size_t size);
void std_aligned_free(void* ptr);
void* aligned_large_pages_alloc(size_t size); // memory aligned by page size, min alignment: 4096 bytes
void aligned_large_pages_free(void* mem); // nop if mem == nullptr

typedef std::chrono::milliseconds::rep Time_ms; // A value in milliseconds
inline Time_ms time_now() {
  return std::chrono::duration_cast<std::chrono::milliseconds>
        (std::chrono::steady_clock::now().time_since_epoch()).count();
}



} // namespace

#endif