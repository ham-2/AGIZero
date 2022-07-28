#include "psqt.h"

namespace AGI {

    using namespace Stockfish;

    Score psqt[PIECE_NB][SQUARE_NB];

    void psqt_init() {

        for (Piece pc : {W_PAWN, W_KNIGHT, W_BISHOP, W_ROOK, W_QUEEN, W_KING})
        {
            Score score = make_score(PieceValue[MG][pc], PieceValue[EG][pc]);

            for (Square s = SQ_A1; s <= SQ_H8; ++s)
            {
                File f = File(edge_distance(file_of(s)));
                psqt[pc][s] = score + (type_of(pc) == PAWN ? PBonus[rank_of(s)][file_of(s)]
                    : Bonus[pc][rank_of(s)][f]);
                psqt[~pc][flip_rank(s)] = -psqt[pc][s];
            }
        }

    }

    void psqt_bias(int v) {

        for (Piece pc : {W_PAWN, W_KNIGHT, W_BISHOP, W_ROOK, W_QUEEN, W_KING})
        {
            Score score = make_score(PieceValue[MG][pc] * v / 50, PieceValue[EG][pc] * v / 50);

            for (Square s = SQ_A1; s <= SQ_H8; ++s)
            {
                psqt[pc][s] += score;
                psqt[~pc][flip_rank(s)] -= score;
            }
        }

    }

}