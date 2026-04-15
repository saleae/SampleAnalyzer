cmake_minimum_required (VERSION 3.13)

project(Arinc429Analyzer)

add_definitions( -DLOGIC2 )

# enable generation of compile_commands.json, helpful for IDEs to locate include files.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# custom CMake Modules are located in the cmake directory.
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

include(ExternalAnalyzerSDK)

set(SOURCES
src/Arinc429Analyzer.cpp
src/Arinc429Analyzer.h
src/Arinc429AnalyzerResults.cpp
src/Arinc429AnalyzerResults.h
src/Arinc429AnalyzerSettings.cpp
src/Arinc429AnalyzerSettings.h
src/Arinc429SimulationDataGenerator.cpp
src/Arinc429SimulationDataGenerator.h
src/Tb270ArincUtils.cpp
src/Tb270ArincUtils.h
)

add_analyzer_plugin(${PROJECT_NAME} SOURCES ${SOURCES})
