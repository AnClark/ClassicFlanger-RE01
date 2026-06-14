/*
 * Classic Flanger - VST 2.4 Plugin
 * Based on analysis of Acoustica Classic Flanger (Delphi VST 2.0)
 * 
 * Parameters (8 total):
 *   0: Rate          - LFO rate (0.01 ~ 5.0 Hz)
 *   1: Depth         - Modulation depth (0 ~ 100%)
 *   2: Feedback      - Feedback amount (-99 ~ +99%)
 *   3: Delay         - Base delay time (0 ~ 20 ms)
 *   4: Mix           - Wet/dry mix (0 ~ 100%)
 *   5: Stereo Phase  - LFO phase offset between channels (0 ~ 180 deg)
 *   6: Waveform      - LFO waveform: 0=Sine, 1=Triangle
 *   7: Polarity      - Feedback polarity: 0=Positive, 1=Negative
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "vestige.h"

/* Platform-specific export macro */
#if defined(_WIN32)
  #define VST_EXPORT __declspec(dllexport)
#elif defined(__APPLE__)
  #define VST_EXPORT __attribute__((visibility("default")))
#else
  /* Linux/Unix */
  #define VST_EXPORT __attribute__((visibility("default")))
#endif

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/* Maximum delay in samples (at 96kHz, 50ms gives plenty of headroom) */
#define MAX_DELAY_MS     50.0f
#define MAX_SAMPLE_RATE  96000.0f
#define MAX_DELAY_SAMPLES ((int)((MAX_DELAY_MS * MAX_SAMPLE_RATE / 1000.0f) + 1))

/* Number of programs and parameters */
#define NUM_PROGRAMS     16
#define NUM_PARAMS       8

/* Parameter indices */
#define PARAM_RATE          0
#define PARAM_DEPTH         1
#define PARAM_FEEDBACK      2
#define PARAM_DELAY         3
#define PARAM_MIX           4
#define PARAM_STEREO_PHASE  5
#define PARAM_WAVEFORM      6
#define PARAM_POLARITY      7

/* Plugin unique ID - derived from analysis */
#define PLUGIN_UNIQUE_ID    CCONST('C', 'F', 'l', 'n')
#define PLUGIN_VERSION      1000

/* LFO waveform types */
#define WAVEFORM_SINE       0
#define WAVEFORM_TRIANGLE   1

/*
 * Program preset data - stores parameter names and default values
 * These are recreated based on the program names found in the binary
 */
typedef struct {
    char name[32];
    float params[NUM_PARAMS];
} Program;

/*
 * Delay line structure
 */
typedef struct {
    float *buffer;
    int size;
    int writePos;
} DelayLine;

/*
 * LFO structure
 */
typedef struct {
    float phase;        /* Current phase 0 ~ 2*PI */
    float rate;         /* Rate in Hz */
    float sampleRate;
    int waveform;       /* 0=Sine, 1=Triangle */
} LFO;

/*
 * Plugin instance data
 */
typedef struct {
    /* VST host callback */
    audioMasterCallback audioMaster;
    
    /* AEffect structure reference */
    AEffect *effect;
    
    /* Sample rate */
    float sampleRate;
    
    /* Parameters (normalized 0.0 ~ 1.0) */
    float params[NUM_PARAMS];
    
    /* Derived parameter values */
    float rateHz;           /* LFO rate in Hz */
    float depth;            /* Depth 0 ~ 1 */
    float feedback;         /* Feedback -0.99 ~ +0.99 */
    float delayMs;          /* Base delay in ms */
    float mix;              /* Mix 0 ~ 1 */
    float stereoPhase;      /* Stereo phase offset in radians */
    int waveform;           /* 0 or 1 */
    float polarity;         /* +1.0 or -1.0 */
    
    /* Current program */
    int currentProgram;
    
    /* Programs */
    Program programs[NUM_PROGRAMS];
    
    /* DSP objects */
    DelayLine delayLineL;
    DelayLine delayLineR;
    LFO lfoL;
    LFO lfoR;
    
    /* Flags */
    int isProcessing;
    int paramsChanged;
} PluginInstance;

/*
 * Parameter name, label, and display functions
 */
typedef struct {
    const char *name;
    const char *label;
    float minVal;
    float maxVal;
    float defaultVal;
    int isSwitch;       /* 0=continuous, 1=switch */
} ParamInfo;

static const ParamInfo paramInfo[NUM_PARAMS] = {
    /* name,        label,    min,   max,   default, switch */
    { "Rate",       "Hz",     0.01f, 5.0f,  0.3f,    0 },  /* PARAM_RATE */
    { "Depth",      "%",      0.0f,  100.0f, 50.0f,   0 },  /* PARAM_DEPTH */
    { "Feedback",   "%",     -99.0f, 99.0f,  50.0f,   0 },  /* PARAM_FEEDBACK */
    { "Delay",      "ms",     0.0f,  20.0f,  5.0f,    0 },  /* PARAM_DELAY */
    { "Mix",        "%",      0.0f,  100.0f, 50.0f,   0 },  /* PARAM_MIX */
    { "Phase",      "deg",    0.0f,  180.0f, 90.0f,   0 },  /* PARAM_STEREO_PHASE */
    { "Waveform",   "",       0.0f,  1.0f,   0.0f,    1 },  /* PARAM_WAVEFORM: 0=Sine, 1=Triangle */
    { "Polarity",   "",       0.0f,  1.0f,   0.0f,    1 },  /* PARAM_POLARITY: 0=Pos, 1=Neg */
};

/* Program names from the original binary */
static const char *programNames[NUM_PROGRAMS] = {
    "Electric Guitar #1",
    "Electric Guitar #2", 
    "Electric Guitar #3",
    "Electric Guitar #4",
    "Bass",
    "Acoustic Guitar",
    "Keyboard",
    "Drums",
    "Vocals",
    "Synth",
    "Strings",
    "Wind",
    "Brass",
    "Pad",
    "Lead",
    "FX"
};

/* Program default parameter values (normalized 0-1) */
static const float programDefaults[NUM_PROGRAMS][NUM_PARAMS] = {
    /* Rate  Depth  Feedbk Delay  Mix   Phase  Wave  Pol */
    { 0.06f, 0.50f, 0.75f, 0.25f, 0.50f, 0.50f, 0.0f, 0.0f },  /* Electric Guitar #1 */
    { 0.10f, 0.60f, 0.60f, 0.20f, 0.45f, 0.50f, 0.0f, 0.0f },  /* Electric Guitar #2 */
    { 0.14f, 0.70f, 0.50f, 0.15f, 0.55f, 0.50f, 0.0f, 1.0f },  /* Electric Guitar #3 */
    { 0.20f, 0.40f, 0.80f, 0.30f, 0.40f, 0.50f, 1.0f, 0.0f },  /* Electric Guitar #4 */
    { 0.04f, 0.30f, 0.40f, 0.35f, 0.35f, 0.25f, 0.0f, 0.0f },  /* Bass */
    { 0.08f, 0.55f, 0.55f, 0.22f, 0.48f, 0.75f, 0.0f, 0.0f },  /* Acoustic Guitar */
    { 0.12f, 0.45f, 0.65f, 0.18f, 0.52f, 0.50f, 0.0f, 1.0f },  /* Keyboard */
    { 0.16f, 0.35f, 0.45f, 0.28f, 0.30f, 0.50f, 1.0f, 0.0f },  /* Drums */
    { 0.06f, 0.65f, 0.70f, 0.15f, 0.55f, 0.50f, 0.0f, 0.0f },  /* Vocals */
    { 0.18f, 0.75f, 0.85f, 0.10f, 0.60f, 0.50f, 0.0f, 1.0f },  /* Synth */
    { 0.04f, 0.50f, 0.35f, 0.25f, 0.45f, 0.75f, 0.0f, 0.0f },  /* Strings */
    { 0.08f, 0.40f, 0.45f, 0.30f, 0.40f, 0.50f, 0.0f, 0.0f },  /* Wind */
    { 0.10f, 0.55f, 0.60f, 0.20f, 0.50f, 0.25f, 1.0f, 0.0f },  /* Brass */
    { 0.14f, 0.60f, 0.55f, 0.18f, 0.55f, 0.50f, 0.0f, 1.0f },  /* Pad */
    { 0.22f, 0.70f, 0.75f, 0.12f, 0.58f, 0.50f, 1.0f, 1.0f },  /* Lead */
    { 0.30f, 0.80f, 0.90f, 0.08f, 0.65f, 0.50f, 1.0f, 1.0f },  /* FX */
};

/*
 * Delay line functions
 */
static void delayLineInit(DelayLine *dl, int size) {
    dl->buffer = (float *)calloc(size, sizeof(float));
    dl->size = size;
    dl->writePos = 0;
}

static void delayLineFree(DelayLine *dl) {
    if (dl->buffer) {
        free(dl->buffer);
        dl->buffer = NULL;
    }
    dl->size = 0;
    dl->writePos = 0;
}

static void delayLineClear(DelayLine *dl) {
    if (dl->buffer) {
        memset(dl->buffer, 0, dl->size * sizeof(float));
    }
    dl->writePos = 0;
}

static void delayLineWrite(DelayLine *dl, float sample) {
    dl->buffer[dl->writePos] = sample;
    dl->writePos++;
    if (dl->writePos >= dl->size) {
        dl->writePos = 0;
    }
}

static float delayLineRead(DelayLine *dl, float delayInSamples) {
    float readPos;
    int readPosInt;
    float frac;
    float sample1, sample2;
    
    readPos = (float)dl->writePos - delayInSamples;
    while (readPos < 0.0f) {
        readPos += (float)dl->size;
    }
    while (readPos >= (float)dl->size) {
        readPos -= (float)dl->size;
    }
    
    readPosInt = (int)readPos;
    frac = readPos - (float)readPosInt;
    
    sample1 = dl->buffer[readPosInt];
    readPosInt++;
    if (readPosInt >= dl->size) {
        readPosInt = 0;
    }
    sample2 = dl->buffer[readPosInt];
    
    /* Linear interpolation */
    return sample1 + frac * (sample2 - sample1);
}

/*
 * LFO functions
 */
static void lfoInit(LFO *lfo, float sampleRate) {
    lfo->phase = 0.0f;
    lfo->rate = 1.0f;
    lfo->sampleRate = sampleRate;
    lfo->waveform = WAVEFORM_SINE;
}

static float lfoGenerate(LFO *lfo) {
    float output;
    
    switch (lfo->waveform) {
        case WAVEFORM_TRIANGLE:
            /* Triangle wave: map phase 0~2PI to 0~1~0 triangle */
            if (lfo->phase < M_PI) {
                output = lfo->phase / M_PI;  /* 0 to 1 */
            } else {
                output = 2.0f - (lfo->phase / M_PI);  /* 1 to 0 */
            }
            break;
        
        case WAVEFORM_SINE:
        default:
            /* Sine wave, mapped to 0 ~ 1 range */
            output = (sinf(lfo->phase) + 1.0f) * 0.5f;
            break;
    }
    
    /* Advance phase */
    lfo->phase += 2.0f * M_PI * lfo->rate / lfo->sampleRate;
    while (lfo->phase >= 2.0f * M_PI) {
        lfo->phase -= 2.0f * M_PI;
    }
    
    return output;
}

/*
 * Parameter conversion functions
 */
static float paramNormalizedToDisplay(int paramIndex, float normalized) {
    const ParamInfo *p = &paramInfo[paramIndex];
    return p->minVal + normalized * (p->maxVal - p->minVal);
}

__attribute__((unused))
static float paramDisplayToNormalized(int paramIndex, float display) {
    const ParamInfo *p = &paramInfo[paramIndex];
    return (display - p->minVal) / (p->maxVal - p->minVal);
}

/*
 * Update derived parameter values from normalized params
 */
static void updateDerivedParams(PluginInstance *plugin) {
    float rateNorm = plugin->params[PARAM_RATE];
    float depthNorm = plugin->params[PARAM_DEPTH];
    float fbNorm = plugin->params[PARAM_FEEDBACK];
    float delayNorm = plugin->params[PARAM_DELAY];
    float mixNorm = plugin->params[PARAM_MIX];
    float phaseNorm = plugin->params[PARAM_STEREO_PHASE];
    float waveNorm = plugin->params[PARAM_WAVEFORM];
    float polNorm = plugin->params[PARAM_POLARITY];
    
    /* Rate: 0.01 ~ 5.0 Hz, with exponential scaling for better feel */
    plugin->rateHz = 0.01f + rateNorm * rateNorm * 4.99f;
    
    /* Depth: 0 ~ 1 (normalized) */
    plugin->depth = depthNorm;
    
    /* Feedback: -0.99 ~ +0.99 */
    plugin->feedback = -0.99f + fbNorm * 1.98f;
    
    /* Delay: 0 ~ 20 ms */
    plugin->delayMs = delayNorm * 20.0f;
    
    /* Mix: 0 ~ 1 */
    plugin->mix = mixNorm;
    
    /* Stereo phase: 0 ~ PI radians */
    plugin->stereoPhase = phaseNorm * M_PI;
    
    /* Waveform: 0 or 1 */
    plugin->waveform = (waveNorm >= 0.5f) ? WAVEFORM_TRIANGLE : WAVEFORM_SINE;
    
    /* Polarity: +1.0 or -1.0 */
    plugin->polarity = (polNorm >= 0.5f) ? -1.0f : 1.0f;
    
    /* Update LFO parameters */
    plugin->lfoL.rate = plugin->rateHz;
    plugin->lfoL.waveform = plugin->waveform;
    plugin->lfoR.rate = plugin->rateHz;
    plugin->lfoR.waveform = plugin->waveform;
    
    plugin->paramsChanged = 0;
}

/*
 * Process one sample
 */
static float processSample(PluginInstance *plugin, float input, int channel) {
    LFO *lfo = (channel == 0) ? &plugin->lfoL : &plugin->lfoR;
    DelayLine *dl = (channel == 0) ? &plugin->delayLineL : &plugin->delayLineR;
    float lfoVal, modulatedDelayMs, delaySamples;
    float delayedSample, output;
    
    /* Generate LFO value (0 ~ 1) */
    lfoVal = lfoGenerate(lfo);
    
    /* Calculate modulated delay time */
    /* delay varies between delayMs and delayMs * (1 + depth) */
    modulatedDelayMs = plugin->delayMs * (1.0f + plugin->depth * (lfoVal - 0.5f) * 2.0f);
    if (modulatedDelayMs < 0.1f) modulatedDelayMs = 0.1f;
    
    /* Convert ms to samples */
    delaySamples = modulatedDelayMs * plugin->sampleRate / 1000.0f;
    
    /* Read delayed sample */
    delayedSample = delayLineRead(dl, delaySamples);
    
    /* Write input + feedback to delay line */
    delayLineWrite(dl, input + delayedSample * plugin->feedback * plugin->polarity);
    
    /* Mix dry and wet signals */
    output = input * (1.0f - plugin->mix) + delayedSample * plugin->mix;
    
    return output;
}

/*
 * Process audio block (processReplacing)
 */
static void processReplacing(AEffect *effect, float **inputs, float **outputs, int sampleFrames) {
    PluginInstance *plugin = (PluginInstance *)effect->object;
    int i;
    float *inL = inputs[0];
    float *inR = inputs[1];
    float *outL = outputs[0];
    float *outR = outputs[1];
    
    if (!plugin || !plugin->isProcessing) {
        /* Just pass through */
        for (i = 0; i < sampleFrames; i++) {
            outL[i] = inL[i];
            outR[i] = inR[i];
        }
        return;
    }
    
    /* Update parameters if changed */
    if (plugin->paramsChanged) {
        updateDerivedParams(plugin);
    }
    
    /* Process samples */
    for (i = 0; i < sampleFrames; i++) {
        outL[i] = processSample(plugin, inL[i], 0);
        outR[i] = processSample(plugin, inR[i], 1);
    }
}

/*
 * VST Dispatcher
 */
static intptr_t dispatcher(AEffect *effect, int opCode, int index, intptr_t value, void *ptr, float opt) {
    PluginInstance *plugin = (PluginInstance *)effect->object;
    
    (void)opt;  /* Unused parameter */
    
    switch (opCode) {
        case effOpen:
            /* Plugin is being opened */
            break;
            
        case effClose:
            /* Plugin is being closed - free memory */
            if (plugin) {
                delayLineFree(&plugin->delayLineL);
                delayLineFree(&plugin->delayLineR);
                free(plugin);
                effect->object = NULL;
            }
            break;
            
        case effSetProgram:
            if (value >= 0 && value < NUM_PROGRAMS) {
                int i;
                plugin->currentProgram = (int)value;
                for (i = 0; i < NUM_PARAMS; i++) {
                    plugin->params[i] = plugin->programs[plugin->currentProgram].params[i];
                }
                plugin->paramsChanged = 1;
                updateDerivedParams(plugin);
            }
            break;
            
        case effGetProgram:
            return plugin->currentProgram;
            
        case effGetProgramName:
            if (ptr) {
                strcpy((char *)ptr, plugin->programs[plugin->currentProgram].name);
            }
            break;
            
        case effGetParamName:
            if (ptr && index >= 0 && index < NUM_PARAMS) {
                strcpy((char *)ptr, paramInfo[index].name);
            }
            break;
            
        case effSetSampleRate:
            plugin->sampleRate = opt;
            plugin->lfoL.sampleRate = opt;
            plugin->lfoR.sampleRate = opt;
            break;
            
        case effSetBlockSize:
            /* Block size doesn't affect our processing */
            break;
            
        case effMainsChanged:
            if (value == 0) {
                /* Suspend - clear delay lines */
                plugin->isProcessing = 0;
                delayLineClear(&plugin->delayLineL);
                delayLineClear(&plugin->delayLineR);
                plugin->lfoL.phase = 0.0f;
                plugin->lfoR.phase = plugin->stereoPhase;
            } else {
                /* Resume */
                plugin->isProcessing = 1;
                plugin->lfoL.phase = 0.0f;
                plugin->lfoR.phase = plugin->stereoPhase;
            }
            break;
            
        case effGetEffectName:
            if (ptr) {
                strcpy((char *)ptr, "Classic Flanger");
            }
            break;
            
        case effGetVendorString:
            if (ptr) {
                strcpy((char *)ptr, "Acoustica");
            }
            break;
            
        case effGetProductString:
            if (ptr) {
                strcpy((char *)ptr, "Classic Flanger");
            }
            break;
            
        case effGetVendorVersion:
            return PLUGIN_VERSION;
            
        case effGetVstVersion:
            return 2400;  /* VST 2.4 */
            
        case effCanDo:
            if (ptr) {
                const char *canDo = (const char *)ptr;
                if (strcmp(canDo, "receiveVstEvents") == 0 ||
                    strcmp(canDo, "receiveVstMidiEvent") == 0) {
                    return 0;  /* No MIDI */
                }
                if (strcmp(canDo, "offline") == 0) {
                    return -1;  /* Not supported */
                }
            }
            return 0;
            
        default:
            break;
    }
    
    return 0;
}

/*
 * Set parameter
 */
static void setParameter(AEffect *effect, int index, float value) {
    PluginInstance *plugin = (PluginInstance *)effect->object;
    
    if (!plugin || index < 0 || index >= NUM_PARAMS) {
        return;
    }
    
    /* Clamp value */
    if (value < 0.0f) value = 0.0f;
    if (value > 1.0f) value = 1.0f;
    
    plugin->params[index] = value;
    plugin->paramsChanged = 1;
    
    /* Also update the program's stored value */
    plugin->programs[plugin->currentProgram].params[index] = value;
}

/*
 * Get parameter
 */
static float getParameter(AEffect *effect, int index) {
    PluginInstance *plugin = (PluginInstance *)effect->object;
    
    if (!plugin || index < 0 || index >= NUM_PARAMS) {
        return 0.0f;
    }
    
    return plugin->params[index];
}

/*
 * Initialize programs
 */
static void initPrograms(PluginInstance *plugin) {
    int p, i;
    
    for (p = 0; p < NUM_PROGRAMS; p++) {
        strncpy(plugin->programs[p].name, programNames[p], 31);
        plugin->programs[p].name[31] = '\0';
        for (i = 0; i < NUM_PARAMS; i++) {
            plugin->programs[p].params[i] = programDefaults[p][i];
        }
    }
}

/*
 * Create plugin instance
 */
static AEffect *createPluginInstance(audioMasterCallback audioMaster) {
    AEffect *effect;
    PluginInstance *plugin;
    int i;
    
    /* Allocate effect structure */
    effect = (AEffect *)calloc(1, sizeof(AEffect));
    if (!effect) {
        return NULL;
    }
    
    /* Allocate plugin instance */
    plugin = (PluginInstance *)calloc(1, sizeof(PluginInstance));
    if (!plugin) {
        free(effect);
        return NULL;
    }
    
    plugin->audioMaster = audioMaster;
    plugin->effect = effect;
    plugin->sampleRate = 44100.0f;
    plugin->currentProgram = 0;
    plugin->isProcessing = 0;
    plugin->paramsChanged = 1;
    
    /* Initialize programs */
    initPrograms(plugin);
    
    /* Set current program parameters as active */
    for (i = 0; i < NUM_PARAMS; i++) {
        plugin->params[i] = plugin->programs[0].params[i];
    }
    
    /* Initialize delay lines */
    delayLineInit(&plugin->delayLineL, MAX_DELAY_SAMPLES);
    delayLineInit(&plugin->delayLineR, MAX_DELAY_SAMPLES);
    
    /* Initialize LFOs */
    lfoInit(&plugin->lfoL, plugin->sampleRate);
    lfoInit(&plugin->lfoR, plugin->sampleRate);
    plugin->lfoR.phase = M_PI * 0.5f;  /* 90 degree offset for stereo */
    
    /* Initialize derived parameters */
    updateDerivedParams(plugin);
    
    /* Setup AEffect structure */
    effect->magic = kEffectMagic;
    effect->dispatcher = dispatcher;
    effect->process = NULL;  /* We only implement processReplacing */
    effect->setParameter = setParameter;
    effect->getParameter = getParameter;
    effect->numPrograms = NUM_PROGRAMS;
    effect->numParams = NUM_PARAMS;
    effect->numInputs = 2;
    effect->numOutputs = 2;
    effect->flags = effFlagsCanReplacing;  /* Supports processReplacing, no editor */
    effect->ptr1 = NULL;
    effect->ptr2 = NULL;
    effect->initialDelay = 0;
    effect->object = plugin;
    effect->user = NULL;
    effect->uniqueID = PLUGIN_UNIQUE_ID;
    effect->version = PLUGIN_VERSION;
    effect->processReplacing = processReplacing;
    
    return effect;
}

/*
 * VST Plugin Main Entry Point
 * This is the main entry point for VST 2.4 (replacing the old "main" from VST 2.0)
 */
VST_EXPORT AEffect *VSTPluginMain(audioMasterCallback audioMaster) {
    AEffect *effect;
    
    /* Check host compatibility */
    if (!audioMaster) {
        return NULL;
    }
    
    /* Create plugin instance */
    effect = createPluginInstance(audioMaster);
    if (!effect) {
        return NULL;
    }
    
    return effect;
}

/* 
 * Legacy main entry point for VST 2.0 hosts
 * Some older hosts may call this instead of VSTPluginMain
 */
/* For VST 2.0 hosts that expect "main" entry point */
VST_EXPORT AEffect *main(audioMasterCallback audioMaster) {
    return VSTPluginMain(audioMaster);
}
