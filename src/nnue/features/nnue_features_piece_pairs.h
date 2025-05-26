/*
  Stockfish, a UCI chess playing engine derived from Glaurung 2.1
  Copyright (C) 2004-2025 The Stockfish developers (see AUTHORS file)

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

#ifndef NNUE_FEATURES_PIECE_PAIRS_H_INCLUDED
#define NNUE_FEATURES_PIECE_PAIRS_H_INCLUDED

#include "../../position.h"
#include "../../types.h"
#include "../nnue_common.h"

namespace Stockfish::Eval::NNUE::Features {

// --- Conceptual Piece-Pair Feature Definitions ---

// This namespace will contain functions and constants related to
// experimental piece-pair features. These features aim to capture
// synergies or specific relationships between pairs of pieces,
// potentially augmenting the existing king-centric features.

// Maximum number of distinct piece-pair features we are defining.
// This is a small number to start with for experimentation.
constexpr IndexType MaxPiecePairFeatureTypes = 10;

// Enum to define specific types of piece-pair features.
// These are conceptual at this stage and will be refined.
enum PiecePairFeatureType : IndexType {
    // Example: Friendly Rook and Friendly Bishop on the same rank
    FRIENDLY_ROOK_BISHOP_SAME_RANK = 0,
    // Example: Friendly Knight pair (two friendly knights)
    FRIENDLY_KNIGHT_PAIR = 1,
    // Example: Friendly Rook pair (two friendly rooks)
    FRIENDLY_ROOK_PAIR = 2,
    // Example: Friendly Bishop pair (two friendly bishops, opposite colors if desired)
    FRIENDLY_BISHOP_PAIR = 3,
    // Example: Opponent Knight pair
    OPPONENT_KNIGHT_PAIR = 4,
    // Example: Opponent Rook pair
    OPPONENT_ROOK_PAIR = 5,
    // Example: Opponent Bishop pair
    OPPONENT_BISHOP_PAIR = 6,
    // Add more types up to MaxPiecePairFeatureTypes - 1
    // ...
    // Placeholder for the actual count, must be last
    PIECE_PAIR_FEATURE_TYPE_COUNT 
};

// Ensure our enum count doesn't exceed the max defined
static_assert(PIECE_PAIR_FEATURE_TYPE_COUNT <= MaxPiecePairFeatureTypes, "Too many PiecePairFeatureType defined.");

// --- Forward declarations for functions that will extract these features ---
// (Implementation will be in a corresponding .cpp file or later in this header for simple ones)

// Helper function to check if a color has at least two pieces of a given type
inline int has_piece_pair(const Position& pos, PieceType pt, Color c) {
    return pos.count<pt>(c) >= 2 ? 1 : 0;
}

// Function to check if a specific piece-pair feature is active
inline int is_feature_active(const Position& pos, PiecePairFeatureType feature_type, Color c) {
    switch (feature_type) {
        case FRIENDLY_KNIGHT_PAIR:
            return has_piece_pair(pos, KNIGHT, c);
        case FRIENDLY_ROOK_PAIR:
            return has_piece_pair(pos, ROOK, c);
        case FRIENDLY_BISHOP_PAIR:
            return has_piece_pair(pos, BISHOP, c);
        case OPPONENT_KNIGHT_PAIR:
            return has_piece_pair(pos, KNIGHT, ~c);
        case OPPONENT_ROOK_PAIR:
            return has_piece_pair(pos, ROOK, ~c);
        case OPPONENT_BISHOP_PAIR:
            return has_piece_pair(pos, BISHOP, ~c);
        case FRIENDLY_ROOK_BISHOP_SAME_RANK: {
            Bitboard rooks = pos.pieces(ROOK, c);
            Bitboard bishops = pos.pieces(BISHOP, c);
            for (Square r_sq : rooks) {
                for (Square b_sq : bishops) {
                    if (rank_of(r_sq) == rank_of(b_sq)) {
                        return 1;
                    }
                }
            }
            return 0;
        }
        // Add other cases as features are implemented
        default:
            return 0; // Should not happen for defined features
    }
}

} // namespace Stockfish::Eval::NNUE::Features

#endif // NNUE_FEATURES_PIECE_PAIRS_H_INCLUDED
