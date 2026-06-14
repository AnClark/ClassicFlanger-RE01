#ifndef STRUCTURES_H_INCLUDED
#define STRUCTURES_H_INCLUDED

/**
 * @file Structures.h
 * @brief Parameter enumerations and metadata for the Classic Flanger plugin.
 */

/**
 * @enum Parameters
 * @brief Unique identifiers for each plugin parameter.
 *
 * The values are used as indices into the parameter value array and into
 * the @ref paramInfo metadata table.
 */
enum Parameters
{
    pParamRate = 0,        ///< Modulation rate (LFO frequency).
    pParamDepth,           ///< Modulation depth (amount of delay variation).
    pParamFeedback,        ///< Feedback amount (can be negative for polarity inversion).
    pParamDelayMs,         ///< Base delay time in milliseconds.
    pParamMix,             ///< Dry/wet mix ratio.
    pParamStereoPhase,     ///< Stereo phase offset between left and right LFOs, in degrees.
    pParamWaveform,        ///< LFO waveform: 0 = sine, 1 = triangle.
    pParamPolarity,        ///< Feedback polarity: 0 = positive, 1 = negative.
    NUM_PARAMS             ///< Total number of parameters (must remain last).
};

/**
 * @struct ParamInfo
 * @brief Static metadata describing one plugin parameter.
 *
 * Holds the display name, unit label, value range, default value and a
 * flag indicating whether the parameter behaves as a switch.
 */
struct ParamInfo
{
    const char *name;       ///< Parameter display name.
    const char *label;      ///< Parameter unit label (e.g. "Hz", "ms").
    float minVal;           ///< Minimum allowed value.
    float maxVal;           ///< Maximum allowed value.
    float defaultVal;       ///< Default value used on initialization.
    int isSwitch;           ///< 0 = continuous parameter, 1 = switch/enum parameter.
};

/**
 * @var paramInfo
 * @brief Static table of metadata for all plugin parameters.
 *
 * The table order matches the @ref Parameters enumeration. It is used by
 * the plugin class to initialize DISTRHO parameter descriptors.
 */
static const ParamInfo paramInfo[NUM_PARAMS] = {
    /* name,        label,    min,   max,   default, switch */
    { "Rate",       "Hz",     0.01f, 5.0f,  0.3f,    0 },  ///< pParamRate
    { "Depth",      "%",      0.0f,  1.0f, 0.5f,    0 },  ///< pParamDepth
    { "Feedback",   "",      -0.99f, 0.99f, 0.5f,    0 },  ///< pParamFeedback
    { "Delay",      "ms",     0.0f,  20.0f, 5.0f,    0 },  ///< pParamDelayMs
    { "Mix",        "",       0.0f,  1.0f, 0.5f,    0 },  ///< pParamMix
    { "Phase",      "deg",    0.0f,  180.0f, 90.0f,  0 },  ///< pParamStereoPhase
    { "Waveform",   "",       0.0f,  1.0f,  0.0f,   1 },  ///< pParamWaveform: 0=Sine, 1=Triangle
    { "Polarity",   "",       0.0f,  1.0f,  0.0f,   1 },  ///< pParamPolarity: 0=Pos, 1=Neg
};

#endif // STRUCTURES_H_INCLUDED
