CMAKE_MINIMUM_REQUIRED(VERSION 3.10)

# set the project name.
PROJECT(LOCA_LINE VERSION 1.0)

# include directories.
INCLUDE_DIRECTORIES(include)

# set good gcc version.
SET(CMAKE_CXX_STANDARD 17)
SET(CMAKE_CXX_STANDARD_REQUIRED ON)
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -pthread -L/usr/local/lib")

# found package.
FIND_PACKAGE(OpenCV 4.2 REQUIRED)

# PROGRAMME 01 
ADD_EXECUTABLE(01_DEMO 
core/demo.cpp
src/00_function.cpp
include/00_function.h)

TARGET_LINK_LIBRARIES(01_DEMO
    pthread
    hiredis
    redis++
    ${OpenCV_LIBS})