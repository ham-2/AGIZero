#ifndef MATERIAL_INCLUDED
#define MATERIAL_INCLUDED

#include <cstdint>

namespace AGI {

    enum Score : int {};

    constexpr Score S(int mg, int eg) {
        return Score(mg | ((unsigned int)eg << 16));
    }

    inline int get_eg(Score s) {
        union { uint16_t u; int16_t s; } eg = { uint16_t(unsigned(s + 0x8000) >> 16) };
        return int(eg.s);
    }

    inline int get_mg(Score s) {
        union { uint16_t u; int16_t s; } mg = { uint16_t(unsigned(s)) };
        return int(mg.s);
    }

    constexpr Score operator+(Score s1, Score s2) { return Score(int(s1) + s2); }
    constexpr Score operator-(Score s1, Score s2) { return Score(int(s1) - s2); }
    inline Score& operator+=(Score& s1, Score s2) { return s1 = s1 + s2; }
    inline Score& operator-=(Score& s1, Score s2) { return s1 = s1 - s2; }
    constexpr Score operator-(Score s) { return Score(-int(s)); }
    constexpr Score operator*(Score s, int i) { return Score(int(s) * i); }

    constexpr int pawn_eg = 204; // score - cp conversion
    constexpr int Tempo = 20;

    constexpr int Material_MG[7] = {
        0, // EMPTY
        142, // PAWN
        400, // KNIGHT
        378, // BISHOP
        704, // ROOK
        1496, // QUEEN
        0, // KING
    };

    constexpr Score Material[7] = {
        S(Material_MG[0], 0), // EMPTY
        S(Material_MG[1], pawn_eg ), // PAWN
        S(Material_MG[2], 618 ), // KNIGHT
        S(Material_MG[3], 639 ), // BISHOP
        S(Material_MG[4], 1174), // ROOK
        S(Material_MG[5], 2217), // QUEEN
        S(Material_MG[6], 0   ), // KING
    };

    constexpr Score Bonus[][8][4] = {
      { },
      { },
      { // Knight
{ S(-88 ,  -49), S( -8 ,  -29), S( -9 ,  -13), S(-12 ,  -10) },
{ S(-29 ,  -38), S(-23 ,  -10), S(-10 ,    0), S(  0 ,    2) },
{ S(-26 ,  -22), S( -5 ,    1), S(  6 ,    6), S( 12 ,   15) },
{ S(-32 ,  -23), S( -5 ,   -4), S( 12 ,    3), S( 29 ,    3) },
{ S(-23 ,  -24), S( -8 ,    1), S( 14 ,    3), S( 28 ,    3) },
{ S(-32 ,  -22), S(-10 ,    2), S(  8 ,   -2), S( 21 ,    0) },
{ S(-37 ,  -25), S(-11 ,    8), S(  2 ,   -1), S(  6 ,    2) },
{ S(-30 ,  -36), S(-23 ,  -12), S( -7 ,  -13), S(-18 ,  -20) }
      },
      { // Bishop
{ S(-45 ,  -32), S(-11 ,  -10), S(-16 ,  -11), S(-19 ,   -6) },
{ S(-12 ,    3), S( 23 ,   13), S( -3 ,   -1), S( -5 ,   -9) },
{ S( -3 ,   -1), S(  0 ,    0), S( -6 ,    2), S(-20 ,    7) },
{ S( -2 ,   -3), S(  6 ,    3), S(  8 ,    3), S( 18 ,    1) },
{ S( -4 ,   -5), S(  4 ,    3), S( 12 ,   -1), S( 16 ,   -3) },
{ S( -8 ,   -9), S(  3 ,   -1), S( 13 ,    4), S( 19 ,   -4) },
{ S(-21 ,   -6), S(-10 ,    7), S(  7 ,    4), S( -8 ,    1) },
{ S(-10 ,   -2), S(-26 ,    2), S( -5 ,   -7), S(-19 ,  -28) }
      },
      { // Rook
{ S(-15 ,   -6), S(-19 ,   -5), S(-9 ,   -4), S( -8 ,   -2) },
{ S(-16 ,   -4), S(-15 ,   -3), S(-7 ,   -5), S( -7 ,   -2) },
{ S(-18 ,   -1), S(-11 ,   -2), S(-4 ,   -3), S( -7 ,   -1) },
{ S(-10 ,   -3), S( -2 ,   -3), S(-2 ,   -3), S( 11 ,   -3) },
{ S(-10 ,    0), S(  4 ,   -2), S( 4 ,   -2), S( 12 ,   -4) },
{ S(-14 ,    2), S( -1 ,   -2), S(-6 ,   -2), S(  6 ,   -2) },
{ S( -3 ,   26), S(  8 ,   29), S( 5 ,   27), S(  9 ,   26) },
{ S(-14 ,   -7), S( 15 ,   -6), S(12 ,   -5), S(-17 ,   -3) }
      },
      { // Queen
{ S(-11 ,  -21), S( -3 ,  -29), S( -2 ,   -8), S( -2 ,   -5) },
{ S(-18 ,   -7), S( -5 ,  -11), S( -6 ,   -6), S( -6 ,   -5) },
{ S(-23 ,   -1), S(-20 ,  -12), S(-24 ,   -4), S(-30 ,    1) },
{ S(-18 ,    2), S(-22 ,    5), S(-25 ,    0), S(-29 ,    6) },
{ S(-20 ,    1), S(-23 ,    2), S(-25 ,    0), S(-22 ,    6) },
{ S(-19 ,  -16), S(-22 ,   -4), S(-20 ,   -3), S(-26 ,   -2) },
{ S(-22 ,    9), S(-15 ,   11), S(-14 ,   15), S(-17 ,   16) },
{ S(-25 ,  -10), S(-23 ,   -6), S(-26 ,   -9), S(-28 ,   -4) }
      },
      { // King
{ S(124 ,   -33), S(140 ,     0), S(134 ,   34), S(125 ,   48) },
{ S(127 ,    22), S(129 ,    62), S(121 ,   90), S(112 ,   95) },
{ S( 95 ,    74), S(101 ,   100), S(100 ,  120), S( 94 ,  122) },
{ S( 72 ,    99), S( 80 ,   133), S( 77 ,  156), S( 67 ,  160) },
{ S( 64 ,   119), S( 63 ,   153), S( 52 ,  172), S( 46 ,  173) },
{ S( 42 ,   122), S( 34 ,   143), S( 33 ,  173), S( 22 ,  175) },
{ S( 31 ,   100), S( 15 ,   122), S( 12 ,  145), S( 11 ,  142) },
{ S( 42 ,    10), S( 11 ,    63), S( 10 ,   69), S( 10 ,   80) }
      }
    };

    constexpr Score PBonus[8][8] =
    { // Pawn
     { },
{ S(-13,  6), S( -8, -2), S(-25,  0), S(13, -6), S(16,  0), S( 21,   0), S(11,  0), S( 5, -12) },
{ S(-15, -2), S( -2,  0), S(  8, 11), S(10,  3), S(12, -1), S( -5, -10), S(-9, -1), S(-2,  -1) },
{ S(-16,  3), S( -6,  1), S( 12, -5), S(15, -6), S(24, -3), S(  2,   1), S(-9, -4), S(-6,   2) },
{ S(-11,  4), S( -5, -1), S(  1, -1), S(18,  2), S(26, -2), S(  3,  -3), S(-5,  0), S( 0,   3) },
{ S( -4,  0), S(  0, -3), S(  5,  3), S(17,  4), S(18,  6), S(  8,   6), S(-3,  5), S( 3,   0) },
{ S( -1,  3), S(  4,  5), S(  6, -7), S(16,  0), S(17,  0), S( 11,  -5), S(-2,  8), S( 6,   4) }
    };

    constexpr Score Activity[4] = { S(15,  2), S(8,  0), S(9,  3), S(17,  0) };

    constexpr Score PassedPawn[16] = { S(0,  0), S(67, 30), S(79, 46), S(83, 66), S(87, 90), S(118, 118), S(125, 178), S(0,  0),
                                       S(0,  0), S(0 , 15), S(13, 26), S(22, 40), S(36, 55), S(65 ,  70), S(80 ,  70), S(0,  0) };

	extern Score piece_table[15][64];
    extern int material_bias;

	void material_init();
}

#endif