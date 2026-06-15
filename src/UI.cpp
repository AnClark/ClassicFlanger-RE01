#include "UI.h"
#include "config.h"

static constexpr ImGuiKnobs_Mod::KnobScaleMark kDelayTimeMarks[] = {
    {   0.1f, "0.05"   },
    { 0.16f, ".16"},
    {0.25f, ".25"},
    {0.40f, ".40"},
    {0.63f, ".63"},
    {1.0f, "1.0"},
    { 1.6f, "1.6"},
    {2.5f, "2.5"},
    { 4.0f, "4.0" },
    { 6.3f, "6.3"},
    { 10.0f, "10"}
};

static constexpr ImGuiKnobs_Mod::KnobScaleMark kRateMarks[] = {
    {   0.05f, "0.05"  },
    { 0.0875f, nullptr },
    { 0.145f, nullptr },
    { 0.25f, nullptr },
    { 0.42f, nullptr },
    {0.7f, "0.7"},
    { 1.2f, nullptr },
    { 2.0f, nullptr },
    { 3.45f, nullptr },
    { 5.8f, nullptr },
    { 10.0f, "10"}
};

static const ImGuiKnobs_Mod::KnobScaleMark kDepthMarks[] = {
    {   0.0f / 100.0f, "0" },
    {   10.0f / 100.0f, nullptr },
    {   20.0f / 100.0f, nullptr },
    {   30.0f / 100.0f, nullptr },
    {   40.0f / 100.0f, nullptr },
    {   50.0f / 100.0f, "50" },
    {   60.0f / 100.0f, nullptr },
    {   70.0f / 100.0f, nullptr },
    {   80.0f / 100.0f, nullptr },
    {   90.0f / 100.0f, nullptr },
    {  100.0f / 100.0f, "100" },
};

static const ImGuiKnobs_Mod::KnobScaleMark kFeedbackMarks[] = {
    {   -99.0f / 100.0f, "-100" },
    {   -80.0f / 100.0f, nullptr },
    {   -60.0f / 100.0f, nullptr },
    {  -40.0f / 100.0f, nullptr },
    {  -20.0f / 100.0f, nullptr },
    {   0.0f / 100.0f, "0" },
    {   20.0f / 100.0f, nullptr },
    {   40.0f / 100.0f, nullptr },
    {   60.0f / 100.0f, nullptr },
    {   80.0f / 100.0f, nullptr },
    {  99.0f / 100.0f, "100" },
};

static const ImGuiKnobs_Mod::KnobScaleMark kStereoPhaseMarks[] = {
    { 0.0f, "0°" },
    { 15.0f, nullptr },
    { 30.0f, nullptr },
    { 45.0f, nullptr },
    { 60.0f, nullptr },
    { 75.0f, nullptr },
    {90.0f, "90°" },
    { 105.0f, nullptr },
    { 120.0f, nullptr },
    { 135.0f, nullptr },
    { 150.0f, nullptr },
    { 165.0f, nullptr },
    { 180.0f, "180°" }
};

static const ImGuiKnobs_Mod::KnobScaleMark kMixMarks[] = {
    {   0.0f / 100.0f, "DIR." },
    {   12.5f / 100.0f, nullptr },
    {   25.0f / 100.0f, nullptr },
    {   37.5f / 100.0f, nullptr },
    {   50.0f / 100.0f, "1:1" },
    {   62.5f / 100.0f, nullptr },
    {   75.0f / 100.0f, nullptr },
    {   87.5f / 100.0f, nullptr },
    {  100.0f / 100.0f, "EFF." },
};

ClassicFlangerUI::ClassicFlangerUI()
    : DISTRHO::UI(DISTRHO_UI_DEFAULT_WIDTH, DISTRHO_UI_DEFAULT_HEIGHT, true)
{
    std::memset(fParams, 0, sizeof(fParams));
    _loadFonts();
    fAboutWindowOpened = false;
}

void ClassicFlangerUI::parameterChanged(uint32_t index, float value)
{
    DISTRHO_SAFE_ASSERT_RETURN(index < NUM_PARAMS, )
    fParams[index] = value;
}

void ClassicFlangerUI::onImGuiDisplay()
{
    const float margin = 4.0f;

    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->Pos);
    ImGui::SetNextWindowSize(viewport->Size);

    static constexpr auto kWindowFlags =
        ImGuiWindowFlags_NoDecoration |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoSavedSettings |
        ImGuiWindowFlags_NoScrollWithMouse;

    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));

    if (ImGui::Begin("Main Window", nullptr, kWindowFlags))
    {
        const float rounding = 10.0f;
        const ImVec2 winSize = ImGui::GetWindowSize();

        _drawChassisBackground(margin, rounding);

        ImGui::SetCursorPos(ImVec2(margin, margin));
        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, rounding);

        const ImVec2 childSize(winSize.x - 2.0f * margin, winSize.y - 2.0f * margin);
        if (ImGui::BeginChild("BackgroundPanel", childSize, false,
                              ImGuiWindowFlags_NoScrollbar |
                              ImGuiWindowFlags_NoScrollWithMouse))
        {
            ImGui::Dummy(ImVec2(2.0f, 0.0f));
            ImGui::SameLine();

            if (_BeginSection("DELAY", 100.0f))
            {
                ImGui::Dummy(ImVec2(16.0f, 0.0f));
                ImGui::SameLine();

                // NOTICE: Use "%.2f" format (acts as accuracy hint) for the logarithmic Delay knob so the 0.1..1.0 ms range has
                //         enough quantization steps to feel smooth while dragging.
                //         See _addKnob() for more details.
                _addKnob(pParamDelayMs, " DELAY (ms)", kDelayTimeMarks, IM_ARRAYSIZE(kDelayTimeMarks), true, false, 0.0f, "%.2f");

                _EndSection();
            }

            ImGui::SameLine(0.0f, 20.0f - 2.0f);

            if (_BeginSection("MODULATION", 108.0f * 3 - 4.0f))
            {
                ImGui::Dummy(ImVec2(4.0f, 0.0f));
                ImGui::SameLine();
                _addKnob(pParamRate, " RATE (Hz)", kRateMarks, IM_ARRAYSIZE(kRateMarks), true, false, 0.0f, "%.2f");

                ImGui::SameLine(0.0f, 18.0f);
                _addBinaryStateSwitch(pParamWaveform, " WAVEFORM", "SINE", "SAW", 10.0f, 8.0f); 

                ImGui::SameLine(0.0f, 22.0f);
                _addKnob(pParamDepth, " DEPTH (%)", kDepthMarks, IM_ARRAYSIZE(kDepthMarks), false, false, 0.0f, "%.2f");

                ImGui::SameLine(0.0f, 28.0f);
                _addKnob(pParamFeedback, " FEEDBACK (%)", kFeedbackMarks, IM_ARRAYSIZE(kFeedbackMarks), false, false, 0.0f, "%.2f");

                _EndSection();
            }

            ImGui::SameLine(0.0f, 20.0f);

            if (_BeginSection("OUTPUT", 80.0f * 3))
            {
                ImGui::Dummy(ImVec2(4.0f, 0.0f));
                ImGui::SameLine();
                _addKnob(pParamStereoPhase, "STEREO PHASE", kStereoPhaseMarks, IM_ARRAYSIZE(kStereoPhaseMarks));

                ImGui::SameLine(0.0f, 18.0f);
                _addBinaryStateSwitch(pParamPolarity, "POLARITY", "POS.", "NEG.", 2.0f, 1.0f);

                ImGui::SameLine(0.0f, 28.0f);
                _addKnob(pParamMix, "MIX", kMixMarks, IM_ARRAYSIZE(kMixMarks), false, false, 0.0f, "%.2f");

                _EndSection();
            }

            ImGui::SameLine(0.0f, 40.0f);

            

            {
                ImGui::BeginGroup();

                ImGui::Dummy(ImVec2(0, 2));
                _drawKjearhusLogo(ImVec2(108.0f, 44.0f));
                ImGui::Dummy(ImVec2(0,28));

                const auto currentPos = ImGui::GetCursorScreenPos();
                ImGui::SetCursorScreenPos(ImVec2(currentPos.x - 20.0f, currentPos.y));
                _drawPluginName();

                ImGui::EndGroup();
            }
        }
        ImGui::EndChild();

        ImGui::PopStyleVar();
        ImGui::PopStyleColor();

        ImGui::End();
    }

    ImGui::PopStyleColor();

    // ── "About" window (fullscreen) ───────────────────────────────────────────────
    static constexpr auto about_window_flags =
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoMove       |
        ImGuiWindowFlags_NoSavedSettings |
        ImGuiWindowFlags_AlwaysAutoResize;

    if (fAboutWindowOpened)
    {
        ImGui::SetNextWindowPos(viewport->Pos);
        ImGui::SetNextWindowSize(viewport->Size);

        if (ImGui::Begin("About Window", &fAboutWindowOpened, about_window_flags))
        {
            {
                ImGui::Columns(2, "AboutColumns", false);
                ImGui::SetColumnWidth(0, 400.0f - 5.0f);
                ImGui::SetColumnWidth(1, 420.0f - 15.0f);

                {
                    const String versionStr = String(DISTRHO_PLUGIN_NAME) + "  |  Version " +
                                        String(VERSION_MAJOR) + "." +
                                        String(VERSION_MINOR) + "." +
                                        String(VERSION_PATCH);

                    ImGui::SeparatorText(versionStr);
                    ImGui::Text("Reverse engineering of Kjaerhus Audio " PLUGIN_NAME_COMMON " (2003).");
                    ImGui::Text("Original algorithm by Kjaerhus Audio.");
                    ImGui::Text("Copyright (c) 2026 AnClark Liu <clarklaw4701@qq.com>");
                    
                    ImGui::SeparatorText("License: GNU General Public License v3.0 or later");
                    ImGui::Dummy(ImVec2(0, 2));
                    ImGui::TextWrapped(DISTRHO_PLUGIN_NAME " is free software: "
                                            "you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation,"
                                            "either version 3 of the License, or (at your option) any later version.");
                }

                ImGui::NextColumn();

                {
                    ImGui::SeparatorText("Disclaimer");
                    ImGui::TextWrapped("This is an unofficial, reverse-engineered clone of the discontinued Kjaerhus " PLUGIN_NAME_COMMON ", aiming at bringing"
                                            "this vintage and fantastic plugin to life again.");
                    ImGui::TextWrapped("This project is NOT related to official Kjaerhus Audio, Acoustica LLC. and their affiliates.");
                    ImGui::Dummy(ImVec2(0, 2));
                    ImGui::TextWrapped("The Kjaerhus logo is used under fair use for identification purposes only, "
                                            "and is not intended to infringe any trademarks.");
                    ImGui::Dummy(ImVec2(0, 2));
                    ImGui::TextWrapped("VST is a trademark of Steinberg GmbH.");
                }

                ImGui::Columns(1);
            }

            {
                ImGui::BeginGroup();
                
                ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 5.0f);
                ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(0x2f, 0x4d, 0x44, 0xff));
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, IM_COL32(0x2f + 20, 0x4d + 20, 0x44 + 20, 0xff));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, IM_COL32(0x2f + 40, 0x4d + 40, 0x44 + 40, 0xff));

                // Fixed position OK button at bottom-right (screen coordinates)
                static constexpr ImVec2 button_size = ImVec2(60 - 5, 25);
                ImVec2 buttonPos = ImVec2(viewport->Pos.x + viewport->Size.x - button_size.x - 22.0f,
                                        viewport->Pos.y + viewport->Size.y - button_size.y - 10.0f);
                ImGui::SetCursorScreenPos(buttonPos);
                if (ImGui::Button("OK", button_size))
                {
                    fAboutWindowOpened = false;
                }

                ImGui::PopStyleColor(3);
                ImGui::PopStyleVar(); // FrameRounding

                ImGui::EndGroup();
            }

            ImGui::End();
        }
    }

    _UpdateMouseCursor();
}

START_NAMESPACE_DISTRHO

UI* createUI()
{
    return new ClassicFlangerUI();
}

END_NAMESPACE_DISTRHO
