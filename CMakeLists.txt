cmake_minimum_required(VERSION 3.16)

project(BinaryFileProcessor VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_AUTOUIC_SEARCH_PATHS
    ${CMAKE_CURRENT_SOURCE_DIR}/forms
)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Test)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Test)

set(PROJECT_SOURCES
    src/main.cpp

    src/app/mainwindow.cpp
    src/app/mainwindow.h

    src/domain/fileprocessor.cpp
    src/domain/fileprocessor.h
    src/domain/processingoptions.h
    src/domain/outputnameresolver.cpp
    src/domain/outputnameresolver.h
    src/domain/filescanner.cpp
    src/domain/filescanner.h

    src/processing/processingworker.cpp
    src/processing/processingworker.h
    src/processing/processingcontroller.cpp
    src/processing/processingcontroller.h

    forms/mainwindow.ui
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(BinaryFileProcessor
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
else()
    if(ANDROID)
        add_library(BinaryFileProcessor SHARED
            ${PROJECT_SOURCES}
        )
    else()
        add_executable(BinaryFileProcessor
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_include_directories(BinaryFileProcessor PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/app
    ${CMAKE_CURRENT_SOURCE_DIR}/src/domain
    ${CMAKE_CURRENT_SOURCE_DIR}/src/processing
)

target_link_libraries(BinaryFileProcessor PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
)

if(${QT_VERSION} VERSION_LESS 6.1.0)
    set(BUNDLE_ID_OPTION
        MACOSX_BUNDLE_GUI_IDENTIFIER com.example.BinaryFileProcessor
    )
endif()

set_target_properties(BinaryFileProcessor PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING
        ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)

install(TARGETS BinaryFileProcessor
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(BinaryFileProcessor)
endif()

enable_testing()

add_executable(FileScannerTest
    tests/filescanner_test.cpp
    src/domain/filescanner.cpp
    src/domain/filescanner.h
)

target_include_directories(FileScannerTest PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/domain
)

target_link_libraries(FileScannerTest PRIVATE
    Qt${QT_VERSION_MAJOR}::Test
)

add_test(
    NAME FileScannerTest
    COMMAND FileScannerTest
)

add_executable(OutputNameResolverTest
    tests/outputnameresolver_test.cpp
    src/domain/outputnameresolver.cpp
    src/domain/outputnameresolver.h
)

target_include_directories(OutputNameResolverTest PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/domain
)

target_link_libraries(OutputNameResolverTest PRIVATE
    Qt${QT_VERSION_MAJOR}::Test
)

add_test(
    NAME OutputNameResolverTest
    COMMAND OutputNameResolverTest
)



add_executable(FileProcessorTest
    tests/fileprocessor_test.cpp
    src/domain/fileprocessor.cpp
    src/domain/fileprocessor.h
)

target_include_directories(FileProcessorTest PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/domain
)

target_link_libraries(FileProcessorTest PRIVATE
    Qt${QT_VERSION_MAJOR}::Test
)

add_test(
    NAME FileProcessorTest
    COMMAND FileProcessorTest
)