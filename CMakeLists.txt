cmake_minimum_required(VERSION 2.6)

project(echo CXX)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "release")
endif()

if(NOT MUDUO_PATH)
  set(MUDUO_PATH "/home/$ENV{USER}/build/${CMAKE_BUILD_TYPE}-install-cpp11")
  message(${MUDUO_PATH})
endif()

set(CXX_FLAGS
 -g
 # -DVALGRIND
 # -DMUDUO_STD_STRING
 -Wall
 -Wextra
 # -m32
 -Werror
 -Wconversion
 -Wno-unused-parameter
 -Wold-style-cast
 -Woverloaded-virtual
 -Wpointer-arith
 -Wshadow
 -Wwrite-strings
 -march=native
 # -MMD
 -std=c++0x
 -rdynamic
 )
string(REPLACE ";" " " CMAKE_CXX_FLAGS "${CXX_FLAGS}")

set(CMAKE_CXX_COMPILER "g++")
set(CMAKE_CXX_FLAGS_DEBUG "-O0")
set(CMAKE_CXX_FLAGS_RELEASE "-O2 -finline-limit=1000 -DNDEBUG")
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)



find_path(Muduo_INCLUDE_DIR muduo "${MUDUO_PATH}/include")
find_path(Muduo_LIBRARY_DIR libmuduo_net.a "${MUDUO_PATH}/lib")
set(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} ${Muduo_LIBRARY_DIR})
message(STATUS ${Muduo_INCLUDE_DIR})
message(STATUS ${Muduo_LIBRARY_DIR})

include_directories(${Muduo_INCLUDE_DIR})
find_library(muduo_base muduo_base)
find_library(muduo_net muduo_net)
message(STATUS ${muduo_base})
message(STATUS ${muduo_net})

find_package(Boost REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})

include_directories(${PROJECT_SOURCE_DIR})

set(ECHO_LIBS ${muduo_net} ${muduo_base} pthread rt)
add_executable(echo ./src/echo.cc ./src/main.cc)
target_link_libraries(echo ${ECHO_LIBS})

add_executable(my_echo ./src/my_echo/echo.cpp ./src/my_echo/main.cpp)
target_link_libraries(my_echo ${ECHO_LIBS})

add_executable(sudokusolver ./src/SudokuSolver/echo.cpp ./src/SudokuSolver/sudoku.cc ./src/SudokuSolver/main.cpp)
target_link_libraries(sudokusolver ${ECHO_LIBS})


add_executable(chat src/chat/server.cpp)
target_link_libraries(chat ${ECHO_LIBS})

add_executable(chatClient src/chat/ChatClient.cpp)
target_link_libraries(chatClient ${ECHO_LIBS})

add_executable(fileTransfer2 src/fileTransfer/fileTransfer2.cpp)
target_link_libraries(fileTransfer2 ${ECHO_LIBS})

add_executable(idleconnection src/idleconnection/KickIdleEchoServer.cpp src/idleconnection/KickIdleEchoServer.h)
target_link_libraries(idleconnection ${ECHO_LIBS})

add_executable(hubserver src/Hub/HubServer.cpp src/Hub/HubServer.h)
target_link_libraries(hubserver ${ECHO_LIBS})

#
#add_executable(fileTransfer2 src/fileTransfer/fileTransfer2.cpp )
#target_link_libraries(fileTransfer2 ${ECHO_LIBS})
#target_link_libraries(echo ${muduo_net})
#target_link_libraries(echo ${muduo_base})
#target_link_libraries(echo pthread rt)



