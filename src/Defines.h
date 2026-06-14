#pragma once

/* Maximum delay in samples (at 96kHz, 50ms gives plenty of headroom) */
static constexpr float MAX_DELAY_MS     = 50.0f;
static constexpr float MAX_SAMPLE_RATE  = 96000.0f;
static constexpr int   MAX_DELAY_SAMPLES = static_cast<int>((MAX_DELAY_MS * MAX_SAMPLE_RATE / 1000.0f) + 1);

// Soft-clip ceiling in linear scale.  = 10^(+5/20) ≈ 1.778 (+5 dBFS).
// Signals well below 0 dBFS pass through unaffected; peaks above 0 dBFS
// are progressively attenuated; hard asymptote at +5 dBFS.
static constexpr float kSoftClipCeiling = 1.77827941f;  // 10^(5/20)
static constexpr float kSoftClipCeilingInv = 1.0f / kSoftClipCeiling;   // precomputed reciprocal for efficiency
