cmake_minimum_required(VERSION 3.24)

if(NOT "${METAMODULE_SDK_DIR}" STREQUAL "")
    #set(METAMODULE_SDK_DIR "$ENV{METAMODULE_SDK_DIR}")
	message("METAMODULE_SDK_DIR set by CMake variable ${METAMODULE_SDK_DIR}")
elseif (DEFINED ENV{METAMODULE_SDK_DIR})
    set(METAMODULE_SDK_DIR "$ENV{METAMODULE_SDK_DIR}")
	message("METAMODULE_SDK_DIR set by environment variable ${METAMODULE_SDK_DIR}")
else()
    set(METAMODULE_SDK_DIR "${CMAKE_CURRENT_LIST_DIR}/../metamodule-plugin-sdk")
	message("METAMODULE_SDK_DIR set to default: ${METAMODULE_SDK_DIR}")
endif()
include(${METAMODULE_SDK_DIR}/plugin.cmake)

########################################################################################

project(SickoCV VERSION 0.17 DESCRIPTION "Sickozell for MetaModule" LANGUAGES C CXX ASM)

add_library(SickoCV STATIC)

# Add sources from the Rack source repo
set(SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/..)

target_sources(SickoCV PRIVATE
    ${SOURCE_DIR}/src/plugin.cpp 
    ${SOURCE_DIR}/src/AdMini.cpp
    ${SOURCE_DIR}/src/Adder8.cpp
    ${SOURCE_DIR}/src/Bgates.cpp
    ${SOURCE_DIR}/src/Blender.cpp
    ${SOURCE_DIR}/src/Blender8.cpp
    ${SOURCE_DIR}/src/Btoggler.cpp
    ${SOURCE_DIR}/src/BtogglerSt.cpp
    ${SOURCE_DIR}/src/BtogglerStCompact.cpp
    ${SOURCE_DIR}/src/BtogglerPlus.cpp
    ${SOURCE_DIR}/src/Calcs.cpp
    ${SOURCE_DIR}/src/Clocker.cpp
    ${SOURCE_DIR}/src/Clocker2.cpp
    ${SOURCE_DIR}/src/CVrouter.cpp
    ${SOURCE_DIR}/src/CVswitcher.cpp
    ${SOURCE_DIR}/src/Drummer.cpp
    ${SOURCE_DIR}/src/Drummer4.cpp
    ${SOURCE_DIR}/src/Drummer4Plus.cpp
    ${SOURCE_DIR}/src/DrumPlayer.cpp
    ${SOURCE_DIR}/src/DrumPlayerMk2.cpp
    ${SOURCE_DIR}/src/DrumPlayerPlus.cpp
    ${SOURCE_DIR}/src/DrumPlayerXtra.cpp
    ${SOURCE_DIR}/src/DrumPlayerMini.cpp
    ${SOURCE_DIR}/src/Enver.cpp
    ${SOURCE_DIR}/src/EnverMini.cpp
    ${SOURCE_DIR}/src/EnverMiniX.cpp
    ${SOURCE_DIR}/src/Holder8.cpp
    ${SOURCE_DIR}/src/HolderCompact.cpp
    ${SOURCE_DIR}/src/Holder.cpp
    ${SOURCE_DIR}/src/KeySampler.cpp
    ${SOURCE_DIR}/src/Modulator.cpp
    ${SOURCE_DIR}/src/Modulator7.cpp
    ${SOURCE_DIR}/src/Modulator7Compact.cpp
    ${SOURCE_DIR}/src/MultiRouter.cpp
    ${SOURCE_DIR}/src/MultiSwitcher.cpp
    ${SOURCE_DIR}/src/Parking.cpp
    ${SOURCE_DIR}/src/PolyMuter8.cpp
    ${SOURCE_DIR}/src/PolyMuter8Plus.cpp
    ${SOURCE_DIR}/src/PolyMuter16.cpp
    ${SOURCE_DIR}/src/PolyMuter16Plus.cpp
    ${SOURCE_DIR}/src/RandLoops.cpp
    ${SOURCE_DIR}/src/RandLoopsMini.cpp
    ${SOURCE_DIR}/src/RandLoops8.cpp
    ${SOURCE_DIR}/src/Shifter.cpp
    ${SOURCE_DIR}/src/SickoAmp.cpp
    ${SOURCE_DIR}/src/SickoCrosser4.cpp
    ${SOURCE_DIR}/src/SickoCrosser.cpp
    ${SOURCE_DIR}/src/SickoLooper1.cpp
    ${SOURCE_DIR}/src/SickoLooper1Exp.cpp
    ${SOURCE_DIR}/src/SickoLooper3.cpp
    ${SOURCE_DIR}/src/SickoLooper5.cpp
    ${SOURCE_DIR}/src/SickoPlayer.cpp
    ${SOURCE_DIR}/src/SickoQuant4.cpp
    ${SOURCE_DIR}/src/SickoQuant.cpp
    ${SOURCE_DIR}/src/SickoSampler.cpp
    ${SOURCE_DIR}/src/SickoSampler2.cpp
    ${SOURCE_DIR}/src/SimpleSeq4.cpp
    ${SOURCE_DIR}/src/Slewer.cpp
    ${SOURCE_DIR}/src/SlewerMini.cpp
    ${SOURCE_DIR}/src/StepSeq.cpp
    ${SOURCE_DIR}/src/StepSeqPlus.cpp
    ${SOURCE_DIR}/src/StepSeq8x.cpp
    ${SOURCE_DIR}/src/Switcher.cpp
    ${SOURCE_DIR}/src/Switcher8.cpp
    ${SOURCE_DIR}/src/SwitcherSt.cpp
    ${SOURCE_DIR}/src/TogglerCompact.cpp
    ${SOURCE_DIR}/src/Toggler.cpp
    ${SOURCE_DIR}/src/TrigSeq.cpp
    ${SOURCE_DIR}/src/TrigSeqPlus.cpp
    ${SOURCE_DIR}/src/TrigSeq8x.cpp
    ${SOURCE_DIR}/src/Wavetabler.cpp
)

# Add includes and compile options for source repo
target_include_directories(SickoCV PRIVATE ${SOURCE_DIR}/src)

# Call this to link and create the plugin file
create_plugin(
    SOURCE_LIB      SickoCV                                          # The cmake target name (defined in add_target)
    PLUGIN_NAME     SickoCV                                          # This must match the brand "slug" used in VCV Rack
    PLUGIN_JSON     ${SOURCE_DIR}/plugin.json                         # Path to the plugin.json file used by VCV Rack
    SOURCE_ASSETS   ${CMAKE_CURRENT_LIST_DIR}/assets                  # Path to the assets/ dir containing the PNGs
    DESTINATION     ${CMAKE_CURRENT_LIST_DIR}/metamodule-plugin   # Path to where you want the plugin file output
)
