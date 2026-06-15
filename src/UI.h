#ifndef CLASSIC_FLANGER_UI_H_INCLUDED
#define CLASSIC_FLANGER_UI_H_INCLUDED

#include "DistrhoUI.hpp"

#include "Structures.h"
#include "imgui-knobs.h"

/**
 * @file UI.h
 * @brief ImGui-based user interface for the Classic Flanger plugin.
 *
 * Implements the plugin editor window using Dear ImGui, including parameter
 * knobs, section grouping, the Kjaerhus Audio recreation logo and an About
 * dialog. Parameter changes from the host are cached locally and edits made
 * by the user are propagated back to the DSP through DISTRHO's UI API.
 */

/**
 * @class ClassicFlangerUI
 * @brief DISTRHO UI implementation for the Classic Flanger plugin.
 *
 * The interface renders a fixed-size chassis with parameter sections for
 * delay and modulation, draws the plugin branding and handles mouse cursor
 * synchronization between ImGui and the DPF window.
 */
class ClassicFlangerUI : public DISTRHO::UI
{
public:
    /**
     * @brief Constructs the UI and loads embedded fonts.
     */
    ClassicFlangerUI();

protected:
    // ── Host callbacks ─────────────────────────────────────────────────────

    /**
     * @brief Called by the host when a parameter value changes.
     * @param index Zero-based parameter index.
     * @param value New parameter value.
     */
    void parameterChanged(uint32_t index, float value) override;

    /**
     * @brief Called each frame to render the ImGui interface.
     *
     * Draws the chassis, parameter knobs, branding and the optional About
     * window, then updates the platform mouse cursor.
     */
    void onImGuiDisplay() override;

private:
    /* Cached state */
    float fParams[NUM_PARAMS] { 0.0f };   ///< Cached parameter values received from the host. @see paramInfo in Structures.h
    int   fLastMouseCursor = -1;          ///< Last ImGui mouse cursor type applied to the window.
    bool  fAboutWindowOpened = false;     ///< True while the About window is visible.

    // ── Rendering helpers ──────────────────────────────────────────────────

    /** @brief Loads the embedded font set into the ImGui font atlas. */
    void _loadFonts();

    /**
     * @brief Draws the chassis background panel with a drop shadow.
     * @param margin  Inner margin between the window edge and the panel.
     * @param rounding Corner radius of the rounded rectangle.
     */
    void _drawChassisBackground(float margin, float rounding);

    /**
     * @brief Draws the clickable Kjaerhus Audio recreation logo.
     * @param size Desired logo area size, in pixels.
     */
    void _drawKjearhusLogo(const ImVec2& size);

    /** @brief Draws the plugin name and the "RE-01" model badge. */
    void _drawPluginName();

    // ── Parameter knobs ────────────────────────────────────────────────────

    /**
     * @brief Adds an ImGui knob for the given parameter.
     * @param paramId       Parameter identifier.
     * @param label         Label displayed beneath the knob.
     * @param v_min         Minimum knob value.
     * @param v_max         Maximum knob value.
     * @param marks         Array of scale mark descriptors.
     * @param mark_count    Number of scale marks.
     * @param isLogarithmic Whether the knob uses logarithmic scaling.
     * @param use_pivot     Whether the knob uses a pivot point.
     * @param pivot_value   Pivot value when @p use_pivot is true.
     * @param format        printf-style value format string.
     */
    void _addKnob(Parameters paramId, const char* label, float v_min, float v_max, const ImGuiKnobs_Mod::KnobScaleMark *marks, uint32_t mark_count, bool isLogarithmic = false, bool use_pivot = false, float pivot_value = 0.0f, const char* format = "%.1f");

    /**
     * @brief Adds an ImGui knob using the parameter's native range from @ref paramInfo.
     *        This variant of _addKnob uses the predefined parameter ranges from kParamRanges,
     *        so you only need to specify the paramId and it will automatically use the correct min/max values.
     * @param paramId       Parameter identifier.
     * @param label         Label displayed beneath the knob.
     * @param marks         Array of scale mark descriptors.
     * @param mark_count    Number of scale marks.
     * @param isLogarithmic Whether the knob uses logarithmic scaling.
     * @param use_pivot     Whether the knob uses a pivot point.
     * @param pivot_value   Pivot value when @p use_pivot is true.
     * @param format        printf-style value format string.
     */
    inline void _addKnob(Parameters paramId, const char* label, const ImGuiKnobs_Mod::KnobScaleMark *marks, uint32_t mark_count, bool isLogarithmic = false, bool use_pivot = false, float pivot_value = 0.0f, const char* format = "%.1f")
    {
        _addKnob(paramId, label, paramInfo[paramId].minVal, paramInfo[paramId].maxVal, marks, mark_count, isLogarithmic, use_pivot, pivot_value, format);
    }

    void _addLEDIndicator(const char* label, bool isLit);

    void _addBinaryStateSwitch(Parameters paramId, const char* label, const char* state0Label, const char* state1Label, float LEDIndentWidth, float btnIndentWidth);

    // ── Layout helpers ─────────────────────────────────────────────────────

    /**
     * @brief Begins a titled parameter section.
     * @param title Section title displayed at the top.
     * @param width Section width, in pixels.
     * @return Always returns true.
     */
    bool _BeginSection(const char* title, float width);

    /** @brief Ends the current parameter section. */
    void _EndSection();

    // ── Input handling ───────────────────────────────────────────────────────

    /** @brief Synchronizes the ImGui mouse cursor with the DPF window cursor. */
    void _UpdateMouseCursor();

    DISTRHO_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ClassicFlangerUI)
};

#endif // CLASSIC_FLANGER_UI_H_INCLUDED
