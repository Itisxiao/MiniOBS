
cmake_minimum_required(VERSION 3.21)

project(MiniOBS LANGUAGES CXX OBJCXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

qt_standard_project_setup()

qt_add_executable(MiniOBS
    MACOSX_BUNDLE

    main.cpp
    app/MainWindow.h
    app/MainWindow.cpp
)

target_link_libraries(MiniOBS PRIVATE
    Qt6::Widgets
)

target_include_directories(MiniOBS PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/app
)

if(APPLE)
    target_link_libraries(MiniOBS PRIVATE
        "-framework ScreenCaptureKit"
        "-framework CoreMedia"
        "-framework CoreVideo"
        "-framework CoreGraphics"
        "-framework Foundation"
        "-framework AppKit"
        "-framework Metal"
        "-framework QuartzCore"
        "-framework VideoToolbox"
    )
endif()

