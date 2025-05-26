# Experimental Augmented NNUE Features

This document describes an experimental extension to Stockfish's NNUE (Efficiently Updatable Neural Network) evaluation function, incorporating new "augmented" features alongside the existing `HalfKAv2_hm` feature set.

## Overview

The core idea is to enhance the information provided to the neural network by adding features that capture relationships between pairs of pieces, rather than just piece-square information relative to the king. These are referred to as "Piece-Pair Features."

The goals of this experiment are:
1.  To explore if these additional features can provide a more nuanced understanding of the position.
2.  To potentially improve evaluation accuracy and, consequently, playing strength.

## Key Components

1.  **Piece-Pair Feature Definitions (`src/nnue/features/nnue_features_piece_pairs.h`):**
    *   This new header defines an enum `PiecePairFeatureType` which lists various conceptual piece-pair relationships (e.g., `FRIENDLY_ROOK_BISHOP_SAME_RANK`, `FRIENDLY_KNIGHT_PAIR`).
    *   It also includes helper functions like `is_feature_active` to determine if a given piece-pair feature is present in the current position.
    *   These features are designed to be simple binary indicators (active or not).

2.  **Augmented Feature Transformer (`src/nnue/features/nnue_feature_transformer_augmented.h`):**
    *   Defines the `FeatureTransformerAugmented` class.
    *   This class is responsible for collecting active features. It calls the standard `HalfKAv2_hm::append_active_indices` and then iterates through the defined `PiecePairFeatureType`s, adding them to the list of active features if `is_feature_active` returns true.
    *   The indices for these new features are offset to avoid collision with `HalfKAv2_hm` feature indices.

3.  **Augmented Network Architecture (`src/nnue/nnue_architecture.h`):**
    *   Introduces `FeatureSetAugmented` (a typedef for `Features::FeatureTransformerAugmented`).
    *   Defines `TransformedFeatureDimensionsAugmented`, `L2Augmented`, and `L3Augmented` as constants for the new network architecture. The input dimension is increased to accommodate the new features.
    *   Defines `AugmentedNetworkArchitecture` as a `NetworkArchitecture` template instantiation with these new constants.

4.  **Network Handling (`src/nnue/network.h`):**
    *   Defines `AugmentedFeatureTransformer` (a typedef for `Features::FeatureTransformerAugmented`).
    *   Defines `NetworkAugmented` (a `Network` template instantiation using `AugmentedNetworkArchitecture` and `AugmentedFeatureTransformer`).
    *   The `Networks` struct now includes an `std::optional<NetworkAugmented> augmented;` member to hold this experimental network.

5.  **Evaluation Path (`src/evaluate.cpp`):**
    *   The `Eval::evaluate` function includes a placeholder conditional path to use the `networks.augmented` network if it's loaded (and eventually, if a UCI option enables it).
    *   This path uses the output of the augmented network and processes it similarly to the standard NNUE evaluation.

## **CRUCIAL: Retraining and Validation Required**

**This entire augmented feature system is EXPERIMENTAL and INCOMPLETE without a newly trained neural network.**

*   **Retraining Necessary:** The existing Stockfish NNUE model files (e.g., `nn-*.nnue`) are **NOT COMPATIBLE** with the `FeatureTransformerAugmented` and `AugmentedNetworkArchitecture`. Attempting to use them will lead to incorrect behavior or crashes. A completely new network must be trained from scratch using a dataset processed with these augmented features.
*   **Training Infrastructure:** The current codebase only provides the *definitions* for these features and the *framework* to use them. The tools and scripts used for generating training data and training the network itself would need to be adapted to incorporate these new features.
*   **ELO Impact Unknown:** The effectiveness and ELO impact of these augmented features are currently unknown. Significant improvements are not guaranteed.
*   **Fishtest Validation:** Any newly trained network incorporating these features must undergo rigorous testing and validation, primarily through `fishtest`, to assess its performance against standard Stockfish versions. Only if a consistent and statistically significant ELO gain is observed can these changes be considered for potential integration.

**Developers looking to experiment with these features should be prepared for the significant effort involved in:**
1.  Adapting data generation and training pipelines.
2.  Training one or more new NNUE models.
3.  Extensive testing and analysis of results.

This framework provides a starting point for research into more complex NNUE feature sets for Stockfish.
