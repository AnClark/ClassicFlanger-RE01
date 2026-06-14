#ifndef DEFINES_H_INCLUDED
#define DEFINES_H_INCLUDED

/**
 * @file Defines.h
 * @brief Global compile-time constants for the Classic Flanger plugin.
 */

/**
 * @defgroup DelayConstants Delay line constants
 * @brief Constants used to size the delay-line buffers.
 *
 * A maximum delay of 50 ms at a sample rate of 96 kHz provides enough
 * headroom for all supported delay and modulation ranges.
 */

/** @brief Maximum delay time in milliseconds. */
static constexpr float MAX_DELAY_MS     = 50.0f;

/** @brief Highest sample rate the plugin is expected to handle. */
static constexpr float MAX_SAMPLE_RATE  = 96000.0f;

/**
 * @brief Maximum delay-line length in samples.
 *
 * Computed from #MAX_DELAY_MS and #MAX_SAMPLE_RATE with one extra sample
 * of safety margin.
 */
static constexpr int   MAX_DELAY_SAMPLES = static_cast<int>((MAX_DELAY_MS * MAX_SAMPLE_RATE / 1000.0f) + 1);

/**
 * @defgroup SoftClipConstants Soft-clipper constants
 * @brief Constants defining the soft-clipping transfer function.
 *
 * The ceiling is set to +5 dBFS in linear scale. Signals well below 0 dBFS
 * pass through unaffected; peaks above 0 dBFS are progressively attenuated
 * and asymptotically limited to +5 dBFS.
 */

/**
 * @brief Soft-clip ceiling in linear scale.
 *
 * Equivalent to @f$10^{(+5/20)} \approx 1.778@f$ (+5 dBFS).
 */
static constexpr float kSoftClipCeiling = 1.77827941f;

/**
 * @brief Precomputed reciprocal of #kSoftClipCeiling.
 *
 * Kept as a constant to avoid a division at runtime.
 */
static constexpr float kSoftClipCeilingInv = 1.0f / kSoftClipCeiling;

#endif // DEFINES_H_INCLUDED
