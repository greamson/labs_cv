cmake_minimum_required(VERSION 3.21)

project(Eyes_Detector)

set(MAIN_TARGET Eyes_Detector)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOUIC_SEARCH_PATHS ${CMAKE_SOURCE_DIR}/src/ui)

# Build environment dependencies
if (CMAKE_SIZEOF_VOID_P EQUAL 8)
    if (WIN32)
        message(STATUS "Windows operating system detected, disabling the console at program start-up")
        #add_link_options(/SUBSYSTEM:windows /ENTRY:mainCRTStartup) # To hide the console window at program startup on Windows
        if (MSVC)
            set(OpenCV_DIR buildenv/windows/OpenCV-4.5.5-MSVC-x64) # Directory with OpenCVConfig.cmake
        else()
            message(FATAL_ERROR "The project is designed to work with the MSVC compiler when compiling on a Windows operating system")
        endif()
    else()
        message(STATUS "Linux kernel operating system detected")
    endif()
else()
    message(FATAL_ERROR "The project is designed to work with an operating system that supports 64-bit extensions")
endif()
#####################################################################################

set(RESOURCE_CMAKE_SOURCE_DIR ${CMAKE_SOURCE_DIR}/resource)
set(RESOURCE_CMAKE_BINARY_DIR ${CMAKE_BINARY_DIR}/resource)


set(PROJECT_SOURCES
    ${CMAKE_SOURCE_DIR}/src/main.cpp
    ${CMAKE_SOURCE_DIR}/src/mainwindow.h
    ${CMAKE_SOURCE_DIR}/src/mainwindow.cpp
    ${CMAKE_SOURCE_DIR}/src/Camera.h
    ${CMAKE_SOURCE_DIR}/src/Camera.cpp
    ${CMAKE_SOURCE_DIR}/src/ImageRecognizer.h
    ${CMAKE_SOURCE_DIR}/src/ImageRecognizer.cpp
    ${CMAKE_SOURCE_DIR}/src/utils/Utils.h
    ${CMAKE_SOURCE_DIR}/src/utils/Utils.cpp
    ${CMAKE_SOURCE_DIR}/src/utils/TextBrowserLogger.h
    ${CMAKE_SOURCE_DIR}/src/utils/TextBrowserLogger.cpp
    ${CMAKE_SOURCE_DIR}/src/ui/mainwindow.ui
)
#####################################################################################

# Find packages
find_package(Qt6 COMPONENTS REQUIRED
    Widgets
    Multimedia
)

find_package(OpenCV 4.5.4 REQUIRED) # 4.5.4 is the minimum version
#####################################################################################

# Uncomment this to create a QM file based on the TS file
# qt_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})

add_executable(${MAIN_TARGET} ${PROJECT_SOURCES})

# Copying the neural network model to the folder with the executable file
# It only works when running cmake command, not during actual build time
file(COPY ${RESOURCE_CMAKE_SOURCE_DIR}/neural_network_model DESTINATION ${RESOURCE_CMAKE_BINARY_DIR})

target_include_directories(${MAIN_TARGET} PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/utils
    ${OpenCV_INCLUDE_DIRS}
)

target_link_libraries(${MAIN_TARGET} PRIVATE
    Qt6::Widgets
    Qt6::Multimedia
    ${OpenCV_LIBS}
)
