#include "ClassicFlangerPlugin.h"
#include "Defines.h"

// ─────────────────────────────────────────────────────────────────────────────
// Classic Flanger plugin class
// ─────────────────────────────────────────────────────────────────────────────

ClassicFlangerPlugin::ClassicFlangerPlugin()
    : DISTRHO::Plugin(NUM_PARAMS, 0, 0)  // 3 states: preset_name, preset_modified, preset_type
{
    // Default parameter values – physical units matching initParameter() ranges
    for (uint32_t i = 0; i < NUM_PARAMS; ++i)
        fParams[i] = paramInfo[i].defaultVal;

    // Call sampleRateChanged() to allocate buffers based on the initial sample rate.
    // This ensures that the plugin is ready to process audio immediately after construction.
    sampleRateChanged(getSampleRate());

    // Initialize delay lines
    fDelayLineL.init(MAX_DELAY_SAMPLES);
    fDelayLineR.init(MAX_DELAY_SAMPLES);

    d_stderr("DEBUG: fDelayLineL buffer size = %lld", fDelayLineL.getBufferSize());
    d_stderr("DEBUG: fDelayLineR buffer size = %lld", fDelayLineR.getBufferSize());

    // Initialize LFOs
    fLfoL.init(fSampleRate);
    fLfoR.init(fSampleRate);
    fLfoR.phase = M_PI * 0.5f;  /* 90 degree offset for stereo */
}

// ── Parameters ────────────────────────────────────────────────────────
void ClassicFlangerPlugin::initParameter(uint32_t index, Parameter& param)
{
    param.hints = kParameterIsAutomatable;
    param.name = paramInfo[index].name;
    param.symbol = String(paramInfo[index].name).toBasic().toLower();
    param.unit = paramInfo[index].label;
    param.ranges = DISTRHO::ParameterRanges(paramInfo[index].defaultVal, paramInfo[index].minVal, paramInfo[index].maxVal);

    if (paramInfo[index].isSwitch)
        param.hints |= kParameterIsBoolean;

    switch (index)
    {
        case pParamRate:
        case pParamDelayMs:
            param.hints |= kParameterIsLogarithmic;
            break;
    }
}

float ClassicFlangerPlugin::getParameterValue(uint32_t index) const
{
    return fParams[index];
}

void ClassicFlangerPlugin::setParameterValue(uint32_t index, float value)
{
    fParams[index] = std::clamp(value, paramInfo[index].minVal,
                                        paramInfo[index].maxVal);
    
    // Update LFO parameters
    switch (index)
    {
        case pParamRate:
            fLfoL.rate = fParams[pParamRate];
            fLfoR.rate = fParams[pParamRate];
            break;
        case pParamWaveform:
            fLfoL.waveform = fParams[pParamWaveform] > 0.0f ? LFOWaveForms::Triangle : LFOWaveForms::Sine;
            fLfoR.waveform = fParams[pParamWaveform] > 0.0f ? LFOWaveForms::Triangle : LFOWaveForms::Sine;
            break;
        case pParamStereoPhase:
            fLfoR.setPhase(fParams[pParamStereoPhase]);
            break;
    }
}

// ── Audio processing ──────────────────────────────────────────────────
void ClassicFlangerPlugin::activate()
{
    fLfoL.setPhase(0.0f);
    fLfoR.setPhase(fParams[pParamStereoPhase]);    // Degree to Rad
}

void ClassicFlangerPlugin::deactivate()
{
    /* Suspend - clear delay lines */
    fDelayLineL.clear();
    fDelayLineR.clear();
    fLfoL.setPhase(0.0f);
    fLfoR.setPhase(fParams[pParamStereoPhase]);    // Degree to Rad
}

void ClassicFlangerPlugin::sampleRateChanged(double newSampleRate)
{
    fSampleRate = (float)newSampleRate;
    fLfoL.sampleRate = fSampleRate;
    fLfoR.sampleRate = fSampleRate;
}


// ─────────────────────────────────────────────────────────────────────────────
// Entry point
// ─────────────────────────────────────────────────────────────────────────────

START_NAMESPACE_DISTRHO

Plugin* createPlugin()
{
    return new ClassicFlangerPlugin();
}

END_NAMESPACE_DISTRHO
