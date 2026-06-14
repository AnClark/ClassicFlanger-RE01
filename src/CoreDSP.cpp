#include "ClassicFlangerPlugin.h"
#include "Defines.h"

float ClassicFlangerPlugin::_processSample(float input, int channel)
{
    LFO &lfo = (channel == 0) ? fLfoL : fLfoR;
    DelayLine &dl = (channel == 0) ? fDelayLineL : fDelayLineR;
    float lfoVal, modulatedDelayMs, delaySamples;
    float delayedSample, output;
    
    /* Generate LFO value (0 ~ 1) */
    lfoVal = lfo.generate();
    
    /* Calculate modulated delay time */
    /* delay varies between delayMs and delayMs * (1 + depth) */
    modulatedDelayMs = fParams[pParamDelayMs] * (1.0f + fParams[pParamDepth] * (lfoVal - 0.5f) * 2.0f);
    if (modulatedDelayMs < 0.1f) modulatedDelayMs = 0.1f;
    
    /* Convert ms to samples */
    delaySamples = modulatedDelayMs * fSampleRate / 1000.0f;
    
    /* Read delayed sample */
    delayedSample = dl.read(delaySamples);
    
    /* Write input + feedback to delay line.
     * Apply tanh saturation INSIDE the loop so the delay buffer stays bounded
     * even at extreme feedback values. This naturally self-limits resonance
     * without affecting the dry/wet balance. */
    float polarity = fParams[pParamPolarity] >= 0.5f ? -1.0f : 1.0f;
    dl.write(std::tanh(input + delayedSample * fParams[pParamFeedback] * polarity));
    
    /* Mix dry and wet signals */
    output = input * (1.0f - fParams[pParamMix]) + delayedSample * fParams[pParamMix];
    
    return output;
}

void ClassicFlangerPlugin::run(const float** inputs, float** outputs, uint32_t frames)
{
    const float *inL = inputs[0];
    const float *inR = inputs[1];
    float *outL = outputs[0];
    float *outR = outputs[1];

    /* Process samples */
    for (int i = 0; i < frames; i++) {
        const float rawL = _processSample(inL[i], 0);
        const float rawR = _processSample(inR[i], 1);

        // tanh soft clip: y = C * tanh(x / C)
        // Unity slope at x=0; knee ~0 dBFS; ceiling ±kSoftClipCeiling.
        outL[i] = kSoftClipCeiling * std::tanh(rawL * kSoftClipCeilingInv);
        outR[i] = kSoftClipCeiling * std::tanh(rawR * kSoftClipCeilingInv);
    }
}
