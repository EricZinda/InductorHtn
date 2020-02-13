cmake_minimum_required(VERSION 3.14)

project(IndHtn CXX)
set(CMAKE_CXX_STANDARD 11)

add_executable(runtests TestsMain.cpp)
add_library(testLib "")

add_executable(indhtn main.cpp)
add_library(lib "")

# Make the Python interface a dll as required by ctypes
# https://stackoverflow.com/questions/6499364/generating-dll-using-cmake
add_library(indhtnpy SHARED "")

target_include_directories(indhtn PRIVATE ./)
target_include_directories(lib PRIVATE ./)
target_include_directories(testLib PRIVATE ./)
target_include_directories(runtests PRIVATE ./)
target_include_directories(indhtnpy PRIVATE ./)

#### Prolog

add_subdirectory(FXPlatform)
add_subdirectory(FXPlatform/Parser)
add_subdirectory(FXPlatform/Prolog)
add_subdirectory(FXPlatform/Htn)

if(WIN32)
	# Link with Kernel32.lib so we can output debug messages
	target_link_libraries(indhtn lib Kernel32.lib)
	target_link_libraries(runtests lib testLib Kernel32.lib)
	add_subdirectory(FXPlatform/Win)
else()
	target_link_libraries(indhtn lib "-framework Foundation")
    target_link_libraries(indhtnpy lib "-framework Foundation")
	target_link_libraries(runtests testLib lib "-framework Foundation")
	add_subdirectory(FXPlatform/iOS)
endif()


#### Tests

add_subdirectory(UnitTest++)
add_subdirectory(Tests)
add_subdirectory(Tests/Prolog)
add_subdirectory(Tests/Htn)

if(WIN32)
	add_subdirectory(UnitTest++/Win32)
else()
	add_subdirectory(UnitTest++/Posix)
endif()

#### Python Interface

add_subdirectory(Python)
target_link_libraries(indhtnpy lib)


