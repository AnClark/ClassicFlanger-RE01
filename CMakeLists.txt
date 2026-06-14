cmake_minimum_required (VERSION 3.20)

project (ClassicFlanger_RE01)

include_directories (src)

add_subdirectory (dpf)

dpf_add_plugin (${PROJECT_NAME}
    TARGETS vst2 vst3 lv2 clap
    FILES_DSP
        src/ClassicFlangerPlugin.cpp
        src/CoreDSP.cpp
)
