cmake_minimum_required(VERSION 3.10)

project(realtime_utilities)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(CMAKE_CL_64 OR CMAKE_GENERATOR MATCHES "Win64")
    set(WIRESHARK_TARGET_PLATFORM win64)
elseif(CMAKE_GENERATOR MATCHES "Visual Studio")
    set(WIRESHARK_TARGET_PLATFORM win32)
else()
    set(WIRESHARK_TARGET_PLATFORM $ENV{WIRESHARK_TARGET_PLATFORM})
endif()

if(CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
	string(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
endif()

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

find_package(catkin REQUIRED roscpp)
find_package(Boost COMPONENTS system thread REQUIRED)

catkin_package(
  INCLUDE_DIRS include
  LIBRARIES ${PROJECT_NAME}
  CATKIN_DEPENDS roscpp
#  DEPENDS system_lib
)

###########
## Build ##
###########

include_directories(include ${catkin_INCLUDE_DIRS} )

## Declare a C++ library
add_library(${PROJECT_NAME} src/${PROJECT_NAME}/realtime_utilities.cpp src/${PROJECT_NAME}/diagnostics_interface.cpp)
add_dependencies(${PROJECT_NAME} ${${PROJECT_NAME}_EXPORTED_TARGETS} ${catkin_EXPORTED_TARGETS})

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang|AppleClang|GNU")
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wunreachable-code -Wpedantic)
endif()
if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options(${PROJECT_NAME} PRIVATE -Wweak-vtables -Wexit-time-destructors -Wglobal-constructors -Wmissing-noreturn )
endif()

if (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    target_compile_options(${PROJECT_NAME} PRIVATE /W4 /w44265 /w44061 /w44062 )
endif()
target_link_libraries(${PROJECT_NAME} -lpthread ${Boost_SYSTEM_LIBRARY} ${Boost_THREAD_LIBRARY})
target_compile_definitions(${PROJECT_NAME} PUBLIC  $<$<CONFIG:RELEASE>:NDEBUG> )

add_executable(test_tasks test/tasks.cpp)
target_link_libraries(test_tasks ${PROJECT_NAME} -lpthread ${Boost_SYSTEM_LIBRARY} ${Boost_THREAD_LIBRARY})

###########
## Install ##
###########

install(DIRECTORY include/${PROJECT_NAME}/
  DESTINATION ${CATKIN_PACKAGE_INCLUDE_DESTINATION}
  FILES_MATCHING PATTERN "*.h"
  PATTERN ".git" EXCLUDE
)

install(TARGETS ${PROJECT_NAME}
  ARCHIVE DESTINATION ${CATKIN_PACKAGE_LIB_DESTINATION}
  LIBRARY DESTINATION ${CATKIN_PACKAGE_LIB_DESTINATION}
  RUNTIME DESTINATION ${CATKIN_PACKAGE_BIN_DESTINATION}
)
