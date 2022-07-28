#include "misc.h"

using namespace std;

namespace AGI {

string getfen(deque<string> input_split) {
    string fen = "";
    for (int i = 0; i < 6; i++) {
        fen += input_split.front();
        fen += " ";
        input_split.pop_front();
    }
    return fen.substr(0, fen.size() - 1);
}

deque<string> split(const string& input_str) {
    stringstream input_stream(input_str);
    deque<std::string> return_v;
    for (string word; input_stream >> word; ) {
        return_v.push_back(word);
    }
    return return_v;
}

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

#ifdef _WIN32
#if _WIN32_WINNT < 0x0601
#undef  _WIN32_WINNT
#define _WIN32_WINNT 0x0601 // Force to include needed API prototypes
#endif

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <windows.h>
// The needed Windows API for processor groups could be missed from old Windows
// versions, so instead of calling them directly (forcing the linker to resolve
// the calls at compile time), try to load them at runtime. To do this we need
// first to define the corresponding function pointers.
    extern "C" {
        typedef bool(*fun1_t)(LOGICAL_PROCESSOR_RELATIONSHIP,
            PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX, PDWORD);
        typedef bool(*fun2_t)(USHORT, PGROUP_AFFINITY);
        typedef bool(*fun3_t)(HANDLE, CONST GROUP_AFFINITY*, PGROUP_AFFINITY);
    }
#endif

void* std_aligned_alloc(size_t alignment, size_t size) {

#if defined(POSIXALIGNEDALLOC)
  void *mem;
  return posix_memalign(&mem, alignment, size) ? nullptr : mem;
#elif defined(_WIN32)
  return _mm_malloc(size, alignment);
#else
  return std::aligned_alloc(alignment, size);
#endif
}

void std_aligned_free(void* ptr) {

#if defined(POSIXALIGNEDALLOC)
  free(ptr);
#elif defined(_WIN32)
  _mm_free(ptr);
#else
  free(ptr);
#endif
}

/// aligned_large_pages_alloc() will return suitably aligned memory, if possible using large pages.

#if defined(_WIN32)
#if defined(_WIN64)
static void* aligned_large_pages_alloc_win(size_t allocSize) {

  HANDLE hProcessToken { };
  LUID luid { };
  void* mem = nullptr;

  const size_t largePageSize = GetLargePageMinimum();
  if (!largePageSize)
      return nullptr;

  // We need SeLockMemoryPrivilege, so try to enable it for the process
  if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hProcessToken))
      return nullptr;

  if (LookupPrivilegeValue(NULL, SE_LOCK_MEMORY_NAME, &luid))
  {
      TOKEN_PRIVILEGES tp { };
      TOKEN_PRIVILEGES prevTp { };
      DWORD prevTpLen = 0;

      tp.PrivilegeCount = 1;
      tp.Privileges[0].Luid = luid;
      tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

      // Try to enable SeLockMemoryPrivilege. Note that even if AdjustTokenPrivileges() succeeds,
      // we still need to query GetLastError() to ensure that the privileges were actually obtained.
      if (AdjustTokenPrivileges(
              hProcessToken, FALSE, &tp, sizeof(TOKEN_PRIVILEGES), &prevTp, &prevTpLen) &&
          GetLastError() == ERROR_SUCCESS)
      {
          // Round up size to full pages and allocate
          allocSize = (allocSize + largePageSize - 1) & ~size_t(largePageSize - 1);
          mem = VirtualAlloc(
              NULL, allocSize, MEM_RESERVE | MEM_COMMIT | MEM_LARGE_PAGES, PAGE_READWRITE);

          // Privilege no longer needed, restore previous state
          AdjustTokenPrivileges(hProcessToken, FALSE, &prevTp, 0, NULL, NULL);
      }
  }

  CloseHandle(hProcessToken);

  return mem;
}
#endif

void* aligned_large_pages_alloc(size_t allocSize) {

#if defined(_WIN64)
  // Try to allocate large pages
  void* mem = aligned_large_pages_alloc_win(allocSize);

  // Fall back to regular, page aligned, allocation if necessary
  if (!mem)
      mem = VirtualAlloc(NULL, allocSize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
#else
  void* mem = VirtualAlloc(NULL, allocSize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
#endif

  return mem;
}

#else

void* aligned_large_pages_alloc(size_t allocSize) {

#if defined(__linux__)
  constexpr size_t alignment = 2 * 1024 * 1024; // assumed 2MB page size
#else
  constexpr size_t alignment = 4096; // assumed small page size
#endif

  // round up to multiples of alignment
  size_t size = ((allocSize + alignment - 1) / alignment) * alignment;
  void *mem = std_aligned_alloc(alignment, size);
#if defined(MADV_HUGEPAGE)
  madvise(mem, size, MADV_HUGEPAGE);
#endif
  return mem;
}

#endif


/// aligned_large_pages_free() will free the previously allocated ttmem

#if defined(_WIN32)

void aligned_large_pages_free(void* mem) {

  if (mem && !VirtualFree(mem, 0, MEM_RELEASE))
  {
      DWORD err = GetLastError();
      std::cerr << "Failed to free transposition table. Error code: 0x" <<
          std::hex << err << std::dec << std::endl;
      exit(EXIT_FAILURE);
  }
}

#else

void aligned_large_pages_free(void *mem) {
  std_aligned_free(mem);
}

#endif


} // namespace 
