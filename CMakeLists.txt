cmake_minimum_required (VERSION 3.20)

project (ClassicFlanger_RE01
    VERSION 1.0.0
    LANGUAGES C CXX
)

include_directories (src)

add_subdirectory (dpf)

configure_file (src/config.h.in ${PROJECT_BINARY_DIR}/config.h @ONLY)

dpf_add_plugin (${PROJECT_NAME}
    TARGETS vst2 vst3 lv2 clap
    FILES_DSP
        src/ClassicFlangerPlugin.cpp
        src/CoreDSP.cpp
)
