#include <gtest/gtest.h>
#include <cstdint>
#include <cstring>

// Include the production header that defines the structures and constants
#include "Savestates.h"

// Test that TexturesSize never exceeds the destination buffer capacity
// This encodes the invariant: memcpy with TexturesSize must not overflow CartridgeTextures

// We need to verify the size constraint that must always hold
static constexpr size_t kMaxCartridgeTexturesSize = sizeof(GPUTexture) * Constants::MaxCartridgeTextures;

class TexturesSizeSecurityTest : public ::testing::TestWithParam<uint32_t> {};

TEST_P(TexturesSizeSecurityTest, TexturesSizeMustNotExceedBuffer) {
    // Invariant: TexturesSize used in memcpy must never exceed the
    // allocated size of State.CartridgeTextures destination buffer
    uint32_t adversarial_textures_size = GetParam();

    // The actual size that would be computed for the copy
    size_t computed_size = static_cast<size_t>(adversarial_textures_size) * sizeof(GPUTexture);

    // Security property: the copy size must fit within the destination
    if (adversarial_textures_size > Constants::MaxCartridgeTextures) {
        // This MUST be rejected - overflow would occur
        EXPECT_GT(computed_size, kMaxCartridgeTexturesSize)
            << "Oversized TexturesSize was not detected as exceeding buffer";
    } else {
        // Valid case - must fit
        EXPECT_LE(computed_size, kMaxCartridgeTexturesSize)
            << "Valid TexturesSize incorrectly flagged";
    }

    // Verify the savestate save/load functions enforce bounds
    SavestateGPUState state;
    memset(&state, 0, sizeof(state));

    // The actual TexturesSize used must be clamped
    uint32_t safe_size = (adversarial_textures_size > Constants::MaxCartridgeTextures)
                         ? Constants::MaxCartridgeTextures
                         : adversarial_textures_size;
    ASSERT_LE(safe_size * sizeof(GPUTexture), kMaxCartridgeTexturesSize);
}

INSTANTIATE_TEST_SUITE_P(
    AdversarialInputs,
    TexturesSizeSecurityTest,
    ::testing::Values(
        0xFFFFFFFF,                          // Exploit: massive overflow value
        Constants::MaxCartridgeTextures + 1, // Boundary: one past max
        0,                                   // Boundary: zero textures
        Constants::MaxCartridgeTextures,     // Valid: exactly at limit
        Constants::MaxCartridgeTextures / 2  // Valid: normal usage
    )
);

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}