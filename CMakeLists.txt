cmake_minimum_required(VERSION 3.16)

project(capybara VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets LinguistTools)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets LinguistTools)

# Find OpenCV
# Try to find OpenCV, but make it optional
find_package(OpenCV QUIET)

if(OpenCV_FOUND)
    message(STATUS "OpenCV found: ${OpenCV_VERSION}")
    message(STATUS "OpenCV include dirs: ${OpenCV_INCLUDE_DIRS}")
    message(STATUS "OpenCV libraries: ${OpenCV_LIBS}")
    include_directories(${OpenCV_INCLUDE_DIRS})
    set(HAVE_OPENCV TRUE)
else()
    # Try alternative method
    find_path(OpenCV_INCLUDE_DIRS 
        NAMES opencv2/opencv.hpp
        PATHS
            /usr/local/include
            /usr/include
            "C:/opencv/include"
            "C:/opencv/build/include"
            "$ENV{USERPROFILE}/opencv/opencv/build/include"
            "$ENV{USERPROFILE}/opencv/build/include"
            "$ENV{OpenCV_DIR}/include"
            "$ENV{OPENCV_DIR}/include"
            "$ENV{OpenCV_DIR}"
            "$ENV{OPENCV_DIR}"
    )
    
    if(OpenCV_INCLUDE_DIRS)
        message(STATUS "OpenCV headers found at: ${OpenCV_INCLUDE_DIRS}")
        include_directories(${OpenCV_INCLUDE_DIRS})
        # Try to find libraries
        find_library(OpenCV_LIBS
            NAMES opencv_world opencv_core opencv_imgproc opencv_imgcodecs
            PATHS
                /usr/local/lib
                /usr/lib
                "C:/opencv/build/x64/vc15/lib"
                "C:/opencv/build/x64/vc16/lib"
                "C:/opencv/build/x64/mingw/lib"
                "$ENV{USERPROFILE}/opencv/opencv/build/x64/vc15/lib"
                "$ENV{USERPROFILE}/opencv/opencv/build/x64/vc16/lib"
                "$ENV{USERPROFILE}/opencv/opencv/build/x64/mingw/lib"
                "$ENV{USERPROFILE}/opencv/build/x64/vc15/lib"
                "$ENV{USERPROFILE}/opencv/build/x64/vc16/lib"
                "$ENV{USERPROFILE}/opencv/build/x64/mingw/lib"
                "$ENV{OpenCV_DIR}/lib"
                "$ENV{OPENCV_DIR}/lib"
        )
        if(OpenCV_LIBS)
            message(STATUS "OpenCV libraries found: ${OpenCV_LIBS}")
            set(HAVE_OPENCV TRUE)
            set(OpenCV_FOUND TRUE)
        else()
            message(WARNING "OpenCV headers found but libraries not found. OpenCV features will be disabled.")
            set(HAVE_OPENCV FALSE)
        endif()
    else()
        message(WARNING "OpenCV not found. Image processing features will be disabled.")
        message(WARNING "")
        message(WARNING "================================================")
        message(WARNING "Auto-install OpenCV:")
        message(WARNING "================================================")
        message(WARNING "Run one of the following commands in PowerShell:")
        message(WARNING "")
        message(WARNING "  Method 1 (Recommended): Use auto-install script")
        message(WARNING "    ./install_opencv.ps1")
        message(WARNING "")
        message(WARNING "  Method 2: Use vcpkg (if installed)")
        message(WARNING "    ./install_opencv_simple.ps1")
        message(WARNING "")
        message(WARNING "Manual installation options:")
        message(WARNING "  1. Install via vcpkg: vcpkg install opencv")
        message(WARNING "  2. Set OpenCV_DIR to directory containing OpenCVConfig.cmake")
        message(WARNING "  3. Add OpenCV installation prefix to CMAKE_PREFIX_PATH")
        message(WARNING "  4. Set OPENCV_DIR environment variable")
        message(WARNING "================================================")
        set(HAVE_OPENCV FALSE)
    endif()
endif()

set(TS_FILES capybara_en_US.ts)

set(PROJECT_SOURCES
        main.cpp
        mainwindow.cpp
        mainwindow.h
        mainwindow.ui
        imageinfo.cpp
        imageinfo.h
        imageprocessor.cpp
        imageprocessor.h
        processingpipeline.cpp
        processingpipeline.h
        resources.qrc
        ${TS_FILES}
)

# Windows icon resource file
# Windows uses .rc file for executable icon
if(WIN32 AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/icon/raw.ico")
    set(PROJECT_SOURCES ${PROJECT_SOURCES} capybara.rc)
endif()

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(capybara
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET capybara APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation

    qt_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
else()
    if(ANDROID)
        add_library(capybara SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(capybara
            ${PROJECT_SOURCES}
        )
    endif()

    qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
endif()

target_link_libraries(capybara PRIVATE 
    Qt${QT_VERSION_MAJOR}::Widgets
)

# Link OpenCV if found
if(HAVE_OPENCV)
    if(OpenCV_LIBS)
        target_link_libraries(capybara PRIVATE ${OpenCV_LIBS})
    elseif(TARGET opencv_world)
        target_link_libraries(capybara PRIVATE opencv_world)
    elseif(TARGET opencv_core)
        target_link_libraries(capybara PRIVATE 
            opencv_core opencv_imgproc opencv_imgcodecs opencv_photo)
    endif()
    target_compile_definitions(capybara PRIVATE HAVE_OPENCV)
    message(STATUS "OpenCV enabled - HAVE_OPENCV defined")
else()
    message(STATUS "OpenCV disabled - compiling without OpenCV support")
endif()

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.capybara)
endif()
set_target_properties(capybara PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

# Set application icon
if(WIN32)
    # Windows uses .rc file for icon
    # The .rc file is already included in PROJECT_SOURCES if raw.ico exists
elseif(APPLE)
    # macOS uses .icns file
    set_target_properties(capybara PROPERTIES
        MACOSX_BUNDLE_ICON_FILE "raw.png"
    )
    set_source_files_properties("icon/raw.png" PROPERTIES
        MACOSX_PACKAGE_LOCATION "Resources"
    )
endif()

include(GNUInstallDirs)
install(TARGETS capybara
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(capybara)
endif()
