#ifndef CLASSIC_FLANGER_PLUGIN_H_INCLUDED
#define CLASSIC_FLANGER_PLUGIN_H_INCLUDED

#include "DistrhoPlugin.hpp"
#include "Structures.h"
#include "DelayLine.hpp"
#include "LFO.hpp"

class ClassicFlangerPlugin : public DISTRHO::Plugin
{
public:
    ClassicFlangerPlugin();

protected:
    // ── Plugin metadata ────────────────────────────────────────────────────
    const char* getLabel()   const override { return DISTRHO_PLUGIN_NAME; }
    const char* getMaker()   const override { return DISTRHO_PLUGIN_BRAND; }
    const char* getLicense() const override { return "GPLv3+"; }
    uint32_t    getVersion() const override { return d_version(1, 0, 0); }

    // ── Parameters ────────────────────────────────────────────────────────
    void initParameter(uint32_t index, Parameter& param) override;
    float getParameterValue(uint32_t index) const override;
    void setParameterValue(uint32_t index, float value) override;

    // ── Audio processing ──────────────────────────────────────────────────
    void activate() override;
    void deactivate() override;
    void sampleRateChanged(double newSampleRate) override;
    void run(const float** inputs, float** outputs, uint32_t frames) override;

private:
    /* DSP objects */
    DelayLine fDelayLineL;
    DelayLine fDelayLineR;
    LFO fLfoL;
    LFO fLfoR;

    /* Internal DSP function */
    float _processSample(float input, int channel);

    /* Local storage */
    float fParams[NUM_PARAMS] {};
    float fSampleRate { 44100.0f };

    DISTRHO_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ClassicFlangerPlugin)
};

#endif // CLASSIC_FLANGER_PLUGIN_H_INCLUDED
