CMAKE_MINIMUM_REQUIRED(VERSION 3.0)

PROJECT(XManga VERSION 0.1.0 LANGUAGES CXX)

IF(NOT CMAKE_BUILD_TYPE)
  SET(CMAKE_BUILD_TYPE "Release" CACHE STRING
      "Choose the type of build, options are: Debug Release"
      FORCE)
ENDIF(NOT CMAKE_BUILD_TYPE)
MESSAGE(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

FIND_PACKAGE(Qt5Widgets REQUIRED)
FIND_PACKAGE(Qt5Core REQUIRED)
FIND_PACKAGE(Qt5Gui REQUIRED)
MESSAGE(STATUS "Qt5: ${Qt5Widgets_VERSION_STRING}")

INCLUDE(FindPkgConfig)
PKG_SEARCH_MODULE(UNARR REQUIRED libunarr)
MESSAGE(STATUS "unarr: ${UNARR_VERSION}")

SET(CMAKE_AUTOMOC ON)
SET(CMAKE_AUTOUIC ON)
SET(CMAKE_INCLUDE_CURRENT_DIR ON)
SET(CMAKE_CXX_STANDARD 11)
SET(CMAKE_CXX_FLAGS "-fPIC -Wall")
SET(CMAKE_CXX_FLAGS_DEBUG "-g -pg")
SET(CMAKE_CXX_FLAGS_RELEASE "-DNDEBUG -O3 -mtune=native -march=native -mfpmath=both")

ADD_EXECUTABLE(xmanga
  main.cpp
  PageView.cpp
  BookmarkDialog.cpp
  XManga.cpp

  blitz/scalefilter.cpp
  qactionmanager/keyconfigdialog.cpp
  qactionmanager/qactionmanager.cpp
  qactionmanager/shortcutbutton.cpp
  )

INCLUDE_DIRECTORIES(
  ./qactionmanager/
  ${Qt5Widgets_INCLUDE_DIRS}
  ${Qt5Core_INCLUDE_DIRS}
  ${Qt5Gui_INCLUDE_DIRS}
  ${UNARR_INCLUDE_DIRS}
  )
TARGET_LINK_LIBRARIES(
  xmanga
  ${Qt5Widgets_LIBRARIES}
  ${Qt5Core_LIBRARIES}
  ${Qt5Gui_LIBRARIES}
  ${UNARR_LIBRARIES}
  )

INSTALL(TARGETS xmanga DESTINATION bin)
