cmake_minimum_required (VERSION 3.20)

project (ClassicFlanger_RE01
    VERSION 1.0.0
    LANGUAGES C CXX
)

add_subdirectory (dpf)

configure_file (src/config.h.in ${PROJECT_BINARY_DIR}/config.h @ONLY)

option (CONFIG_BUILD_JACK_STANDALONE "Build JACK standalone format (for debugging)")
if (CONFIG_BUILD_JACK_STANDALONE)
    set (TARGET_JACK jack)
endif ()

dpf_add_plugin (${PROJECT_NAME}
    TARGETS vst2 vst3 lv2 clap ${TARGET_JACK}
    FILES_DSP
        src/ClassicFlangerPlugin.cpp
        src/CoreDSP.cpp
    FILES_UI
        src/UI.cpp
        src/UISubroutines.cpp
        src/UIChassis.cpp
        widgets/dpf-imgui/DearImGui.cpp
        widgets/imgui-knobs-mod/imgui-knobs.cpp
)

target_include_directories(${PROJECT_NAME} PUBLIC
    src
    ${PROJECT_BINARY_DIR}
    widgets/dpf-imgui
    widgets/dpf-imgui/DearImGui
    widgets/imgui-knobs-mod
    widgets/imgui-ext
)
