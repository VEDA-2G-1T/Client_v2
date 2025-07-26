cmake_minimum_required(VERSION 3.16)

project(QtClientSSN_new-ui VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Qt 모듈 찾기 (Widgets, Multimedia, MultimediaWidgets, WebSockets 포함)
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Multimedia MultimediaWidgets WebSockets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Multimedia MultimediaWidgets WebSockets)

set(PROJECT_SOURCES
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    cameraregistrationdialog.h
    cameraregistrationdialog.cpp
    camerainfo.h
    cameraitemwidget.h
    cameraitemwidget.cpp
    resources.qrc
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(QtClientSSN_new-ui
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        logitemwidget.h logitemwidget.cpp
        loghistorydialog.h loghistorydialog.cpp
        logentry.h
        fonts/01HanwhaB.ttf fonts/02HanwhaR.ttf fonts/03HanwhaL.ttf
        fonts/04HanwhaGothicB.ttf fonts/05HanwhaGothicR.ttf fonts/06HanwhaGothicL.ttf fonts/07HanwhaGothicEL.ttf fonts/08HanwhaGothicT.ttf
    )
else()
    if(ANDROID)
        add_library(QtClientSSN_new-ui SHARED
            ${PROJECT_SOURCES}
        )
    else()
        add_executable(QtClientSSN_new-ui
            ${PROJECT_SOURCES}
        )
    endif()
endif()

# 필요한 Qt 모듈 라이브러리 연결
target_link_libraries(QtClientSSN_new-ui PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Multimedia
    Qt${QT_VERSION_MAJOR}::MultimediaWidgets
    Qt${QT_VERSION_MAJOR}::WebSockets
)

# macOS bundle 설정 (필요 시)
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.QtClientSSN_new-ui)
endif()

set_target_properties(QtClientSSN_new-ui PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

# 설치 디렉토리 설정
include(GNUInstallDirs)
install(TARGETS QtClientSSN_new-ui
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# Qt 6용 최종 마무리
if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(QtClientSSN_new-ui)
endif()
