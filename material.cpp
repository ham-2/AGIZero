#include "material.h"

Score piece_table[15][64];
int material_bias = 0;

void material_init() {

    Score score = Material[PAWN] + S(Material_MG[PAWN] * material_bias / 100, 0);
    for (Square s = A1; s < SQ_END; ++s)
    {
        piece_table[W_PAWN][s] = score + PBonus[get_rank(s)][get_file(s)];
        piece_table[B_PAWN][mirror(s)] = -piece_table[W_PAWN][s];
    }

    for (UPiece u : {KNIGHT, BISHOP, ROOK, QUEEN, KING})
    {
        Score score = Material[u] + S(Material_MG[u] * material_bias / 100, 0);;

        for (Square s = A1; s < SQ_END; ++s)
        {
            int file = get_file(s) > 3 ? 7 - get_file(s) : get_file(s);
            piece_table[to_piece(u, WHITE)][s] = score + Bonus[u][get_rank(s)][file];
            piece_table[to_piece(u, BLACK)][mirror(s)] = -piece_table[to_piece(u, WHITE)][s];
        }
    }

}