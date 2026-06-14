#pragma once

#include <cmath>

enum class LFOWaveForms
{
    Sine,
    Triangle
};

struct LFO
{
    float phase;        /* Current phase 0 ~ 2*PI */
    float rate;         /* Rate in Hz */
    float sampleRate;
    LFOWaveForms waveform;       /* 0=Sine, 1=Triangle */

    void init(float sampleRate_)
    {
        phase = 0.0f;
        rate = 1.0f;
        sampleRate = sampleRate_;
        waveform = LFOWaveForms::Sine;
    }

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

    inline void setPhase(float phaseInDegree)
    {
        phase = phaseInDegree * (float)(M_PI / 180.0);
    }
};
