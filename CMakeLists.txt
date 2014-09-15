project(PKI-Test)
cmake_minimum_required(VERSION 2.8)

set(CMAKE_BUILD_TYPE RelWithDebInfo)

set(QT_USE_QT3SUPPORT FALSE)
set(QT_USE_QTGUI TRUE)
set(QT_USE_QTWEBKIT TRUE)
set(QT_USE_QTNETWORK TRUE)
find_package(Qt4 REQUIRED)
include(${QT_USE_FILE})

set(PKI_SRCS
  main.cpp
  webpage.cpp
)

qt4_wrap_ui(UI_SRCS webpage.ui)
qt4_wrap_cpp(MOC_SRCS webpage.h)

include_directories(${QT_INCLUDE_DIR} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})

add_executable(${PROJECT_NAME} ${PKI_SRCS} ${MOC_SRCS} ${UI_SRCS})
target_link_Libraries(${PROJECT_NAME} ${QT_LIBRARIES})
