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

#ifndef NNUE_FEATURE_TRANSFORMER_AUGMENTED_H_INCLUDED
#define NNUE_FEATURE_TRANSFORMER_AUGMENTED_H_INCLUDED

#include "../../position.h"
#include "../../types.h"
#include "../nnue_common.h"
#include "./half_ka_v2_hm.h" // Existing HalfKAv2_hm features
#include "./nnue_features_piece_pairs.h" // Our new piece-pair features

namespace Stockfish::Eval::NNUE::Features {

/**
 * @brief Experimental feature transformer that augments the standard HalfKAv2_hm features
 * with additional piece-pair features defined in nnue_features_piece_pairs.h.
 *
 * The goal of these augmented features is to capture more complex positional
 * characteristics and potential synergies or conflicts between pairs of pieces,
 * which might not be fully represented by the king-centric features of HalfKAv2_hm alone.
 * This could potentially lead to a more nuanced and accurate evaluation.
 *
 * **IMPORTANT DISCLAIMER:**
 * This feature transformer and the corresponding `AugmentedNetworkArchitecture`
 * (defined in `nnue_architecture.h`) are EXPERIMENTAL. They require a
 * **completely new NNUE model to be trained** using these augmented features.
 * The existing network files are NOT compatible with this transformer.
 *
 * The ELO impact and viability of these new features can only be assessed
 * after successful retraining of a network and rigorous testing, for example,
 * through fishtest. This code currently only provides the foundational framework
 * for such experimentation.
 */
class FeatureTransformerAugmented {
public:
    // Feature name
    static constexpr const char* Name = "HalfKAv2_hm_AugmentedPiecePairs";

    // Hash value: Combine existing hash with a new one for the augmented part.
    // This needs to be unique for this specific feature set.
    // For now, just XOR with a new arbitrary constant.
    static constexpr std::uint32_t HashValue = HalfKAv2_hm::HashValue ^ 0xABCDEF01;

    // Dimensions: Original HalfKAv2_hm dimensions + number of new piece-pair feature types.
    // Note: This is the "raw" feature dimension, not the transformed one for the network input.
    static constexpr IndexType Dimensions = HalfKAv2_hm::Dimensions + PIECE_PAIR_FEATURE_TYPE_COUNT;

    // Maximum number of simultaneously active features.
    // HalfKAv2_hm::MaxActiveDimensions + number of new piece-pair features we might activate.
    // Each piece-pair feature is either active (1) or not (0).
    // So, at most PIECE_PAIR_FEATURE_TYPE_COUNT additional features can be active.
    static constexpr IndexType MaxActiveDimensions = HalfKAv2_hm::MaxActiveDimensions + PIECE_PAIR_FEATURE_TYPE_COUNT;

    using IndexList = ValueList<IndexType, MaxActiveDimensions>;

    // Get a list of indices for active features
    template<Color Perspective>
    static void append_active_indices(const Position& pos, IndexList& active_indices) {
        // First, get active indices from the base HalfKAv2_hm feature set.
        // Make sure the list passed to HalfKAv2_hm can hold its features.
        // We will add to this list, so it needs to be large enough for both.
        HalfKAv2_hm::append_active_indices<Perspective>(pos, active_indices);

        // Now, add our new piece-pair features.
        // The indices for these new features will start after the HalfKAv2_hm features.
        IndexType base_offset = HalfKAv2_hm::Dimensions;
        Color us = Perspective; // 'us' from the perspective of the side to move

        for (IndexType i = 0; i < PIECE_PAIR_FEATURE_TYPE_COUNT; ++i) {
            PiecePairFeatureType pft = static_cast<PiecePairFeatureType>(i);
            if (is_feature_active(pos, pft, us)) {
                if (active_indices.size() < MaxActiveDimensions) {
                    active_indices.add(base_offset + pft);
                } else {
                    // This indicates a potential issue with MaxActiveDimensions sizing or too many active features.
                    // For safety in a real scenario, log this or handle more gracefully.
                    break; 
                }
            }
        }
    }

    // Get a list of indices for recently changed features
    template<Color Perspective>
    static void append_changed_indices(Square ksq, const DirtyPiece& dp, IndexList& removed, IndexList& added) {
        // Get changed indices from HalfKAv2_hm
        HalfKAv2_hm::append_changed_indices<Perspective>(ksq, dp, removed, added);
        
        // For piece-pair features:
        // This is a simplification. A production system would need a more granular way
        // to determine if a specific piece-pair feature changed.
        // If any piece moves (dp.piece != NO_PIECE), we assume all piece-pair features *could*
        // have changed and add them to both 'removed' and 'added' to force re-check by accumulator.
        if (dp.piece != NO_PIECE) {
            IndexType base_offset = HalfKAv2_hm::Dimensions;
            for (IndexType i = 0; i < PIECE_PAIR_FEATURE_TYPE_COUNT; ++i) {
                if (removed.size() < MaxActiveDimensions) {
                    removed.add(base_offset + i);
                } else { break; } // List full
            }
            for (IndexType i = 0; i < PIECE_PAIR_FEATURE_TYPE_COUNT; ++i) {
                if (added.size() < MaxActiveDimensions) {
                    added.add(base_offset + i);
                } else { break; } // List full
            }
        }
    }

    // Returns whether the change stored in this DirtyPiece means
    // that a full accumulator refresh is required.
    static bool requires_refresh(const DirtyPiece& dirtyPiece, Color perspective) {
        if (HalfKAv2_hm::requires_refresh(dirtyPiece, perspective)) {
            return true;
        }
        // If a non-pawn piece or the king moves, it's likely to affect piece-pair features.
        // This is a simplification; more precise conditions could be defined.
        PieceType pt = piece_type_of(dirtyPiece.piece);
        if (pt != NO_PIECE && pt != PAWN) {
             return true; 
        }
        // It's already covered by HalfKAv2_hm but good to be explicit if king moves.
        // if (pt == KING) {
        //     return true;
        // }
        return false;
    }
};

} // namespace Stockfish::Eval::NNUE::Features

#endif // NNUE_FEATURE_TRANSFORMER_AUGMENTED_H_INCLUDED
