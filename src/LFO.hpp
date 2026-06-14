#ifndef LFO_HPP_INCLUDED
#define LFO_HPP_INCLUDED

#include <cmath>

/**
 * @file LFO.hpp
 * @brief Low-frequency oscillator (LFO) for delay-time modulation.
 */

/**
 * @enum LFOWaveForms
 * @brief Available LFO waveform shapes.
 */
enum class LFOWaveForms
{
    Sine,     ///< Sinusoidal waveform, output range [0, 1].
    Triangle  ///< Triangular waveform, output range [0, 1].
};

/**
 * @struct LFO
 * @brief Simple low-frequency oscillator.
 *
 * Generates a unipolar low-frequency signal in the range [0, 1] using
 * either a sine or triangle waveform. The phase increments each time
 * @ref generate() is called, based on the configured rate and sample rate.
 */
struct LFO
{
    float phase;           ///< Current phase in radians, range [0, 2π).
    float rate;            ///< Oscillator frequency in Hz.
    float sampleRate;      ///< Host sample rate in Hz.
    LFOWaveForms waveform; ///< Selected waveform shape.

    /**
     * @brief Initializes the LFO to a default state.
     * @param sampleRate_ Host sample rate in Hz.
     *
     * Resets the phase to zero, sets the rate to 1 Hz and selects the
     * sine waveform.
     */
    void init(float sampleRate_)
    {
        phase = 0.0f;
        rate = 1.0f;
        sampleRate = sampleRate_;
        waveform = LFOWaveForms::Sine;
    }

    /**
     * @brief Generates the next LFO sample and advances the phase.
     * @return Unipolar oscillator output in the range [0, 1].
     *
     * The output shape depends on @ref waveform. The internal phase is
     * wrapped to the [0, 2π) range after incrementing.
     */
    float generate()
    {
        float output;
        
        switch (waveform) {
            case LFOWaveForms::Triangle:
                /* Triangle wave: map phase 0~2PI to 0~1~0 triangle */
                if (phase < M_PI) {
                    output = phase / M_PI;  /* 0 to 1 */
                } else {
                    output = 2.0f - (phase / M_PI);  /* 1 to 0 */
                }
                break;
            
            case LFOWaveForms::Sine:
            default:
                /* Sine wave, mapped to 0 ~ 1 range */
                output = (std::sinf(phase) + 1.0f) * 0.5f;
                break;
        }
        
        /* Advance phase */
        phase += 2.0f * M_PI * rate / sampleRate;
        while (phase >= 2.0f * M_PI) {
            phase -= 2.0f * M_PI;
        }
        
        return output;
    }

    /**
     * @brief Sets the oscillator phase from a degree value.
     * @param phaseInDegree Phase in degrees.
     *
     * The value is converted to radians and stored in @ref phase.
     */
    inline void setPhase(float phaseInDegree)
    {
        phase = phaseInDegree * (float)(M_PI / 180.0);
    }
};

#endif // LFO_HPP_INCLUDED
