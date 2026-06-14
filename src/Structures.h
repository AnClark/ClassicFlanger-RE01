#pragma once

/*
 * Parameter name, label, and display functions
 */

enum Parameters
{
    pParamRate = 0,
    pParamDepth,
    pParamFeedback,
    pParamDelayMs,
    pParamMix,
    pParamStereoPhase,
    pParamWaveform,
    pParamPolarity,
    NUM_PARAMS
};

struct ParamInfo
{
    const char *name;
    const char *label;
    float minVal;
    float maxVal;
    float defaultVal;
    int isSwitch;       /* 0=continuous, 1=switch */
};

static const ParamInfo paramInfo[NUM_PARAMS] = {
    /* name,        label,    min,   max,   default, switch */
    { "Rate",       "Hz",     0.01f, 5.0f,  0.3f,    0 },  /* PARAM_RATE */
    { "Depth",      "%",      0.0f,  1.0f, 0.5f,   0 },  /* PARAM_DEPTH */
    { "Feedback",   "",     -0.99f, 0.99f,  0.5f,   0 },  /* PARAM_FEEDBACK */
    { "Delay",      "ms",     0.0f,  20.0f,  5.0f,    0 },  /* PARAM_DELAY */
    { "Mix",        "",      0.0f,  1.0f, 0.5f,   0 },  /* PARAM_MIX */
    { "Phase",      "deg",    0.0f,  180.0f, 90.0f,   0 },  /* PARAM_STEREO_PHASE */
    { "Waveform",   "",       0.0f,  1.0f,   0.0f,    1 },  /* PARAM_WAVEFORM: 0=Sine, 1=Triangle */
    { "Polarity",   "",       0.0f,  1.0f,   0.0f,    1 },  /* PARAM_POLARITY: 0=Pos, 1=Neg */
};
