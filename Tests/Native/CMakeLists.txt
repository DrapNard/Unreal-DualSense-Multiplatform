cmake_minimum_required(VERSION 3.20)
project(DualSenseNativeTests LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(PLUGIN_ROOT "${CMAKE_CURRENT_LIST_DIR}/../..")
set(CORE_ROOT "${PLUGIN_ROOT}/Source/DualSenseCore")
set(GAMEPAD_CORE_ROOT "${PLUGIN_ROOT}/ThirdParty/Dualsense-Multiplatform/Source")
if(NOT EXISTS "${GAMEPAD_CORE_ROOT}/Public/GCore/Interfaces/IPlatformHardware.h")
    message(FATAL_ERROR "Dualsense-Multiplatform submodule is missing. Run: git submodule update --init ThirdParty/Dualsense-Multiplatform")
endif()

file(GLOB GAMEPAD_CORE_BRIDGES "${CORE_ROOT}/Private/ThirdParty/GamepadCore_*.cpp")
set(PLATFORM_SOURCE "")
if(WIN32)
    set(PLATFORM_SOURCE "${CORE_ROOT}/Private/Platform/DualSensePlatformWindows.cpp")
elseif(APPLE)
    set(PLATFORM_SOURCE "${CORE_ROOT}/Private/Platform/DualSensePlatformMac.cpp")
elseif(UNIX)
    set(PLATFORM_SOURCE "${CORE_ROOT}/Private/Platform/DualSensePlatformLinux.cpp")
else()
    message(FATAL_ERROR "Unsupported native test platform")
endif()

add_executable(dualsense_native_tests
    main.cpp
    "${CORE_ROOT}/Private/DualSenseCoreManager.cpp"
    "${CORE_ROOT}/Private/Platform/DualSensePlatformHardware.cpp"
    ${PLATFORM_SOURCE}
    ${GAMEPAD_CORE_BRIDGES}
)
target_compile_definitions(dualsense_native_tests PRIVATE DUALSENSECORE_API=)
target_include_directories(dualsense_native_tests PRIVATE
    "${CORE_ROOT}/Public"
    "${CORE_ROOT}/Private"
    "${GAMEPAD_CORE_ROOT}/Public"
    "${GAMEPAD_CORE_ROOT}/Private"
)
if(WIN32)
    target_link_libraries(dualsense_native_tests PRIVATE setupapi hid)
elseif(APPLE)
    find_library(IOKIT_FRAMEWORK IOKit REQUIRED)
    find_library(COREFOUNDATION_FRAMEWORK CoreFoundation REQUIRED)
    target_link_libraries(dualsense_native_tests PRIVATE ${IOKIT_FRAMEWORK} ${COREFOUNDATION_FRAMEWORK})
endif()

enable_testing()
add_test(NAME dualsense_native_smoke COMMAND dualsense_native_tests)
