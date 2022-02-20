cmake_minimum_required (VERSION 3.13)

project(SimpleSerialAnalyzer)

add_definitions( -DLOGIC2 )

# enable generation of compile_commands.json, helpful for IDEs to locate include files.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# custom CMake Modules are located in the cmake directory.
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

include(ExternalAnalyzerSDK)

set(SOURCES 
src/SimpleSerialAnalyzer.cpp
src/SimpleSerialAnalyzer.h
src/SimpleSerialAnalyzerResults.cpp
src/SimpleSerialAnalyzerResults.h
src/SimpleSerialAnalyzerSettings.cpp
src/SimpleSerialAnalyzerSettings.h
src/SimpleSerialSimulationDataGenerator.cpp
src/SimpleSerialSimulationDataGenerator.h
)

add_analyzer_plugin(${PROJECT_NAME} SOURCES ${SOURCES})
