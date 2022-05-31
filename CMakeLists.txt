cmake_minimum_required(VERSION 3.10)

project(realtime_utilities)
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/cnrConfigMacros.cmake)

cnr_set_flags()

find_package(catkin REQUIRED roscpp)
find_package(Boost COMPONENTS system thread REQUIRED)
find_package(Threads REQUIRED)

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
target_link_libraries(${PROJECT_NAME} ${Boost_SYSTEM_LIBRARY} ${Boost_THREAD_LIBRARY})

if(${CMAKE_VERSION} VERSION_GREATER  "3.16.0")
  target_link_libraries(${PROJECT_NAME} Threads::Threads)
else()
  if(THREADS_HAVE_PTHREAD_ARG)
    target_compile_options(${PROJECT_NAME} PUBLIC "-pthread")
  endif()
  if(CMAKE_THREAD_LIBS_INIT)
    target_link_libraries(${PROJECT_NAME} "${CMAKE_THREAD_LIBS_INIT}")
  endif()
endif()

target_compile_definitions(${PROJECT_NAME} PUBLIC  $<$<CONFIG:RELEASE>:NDEBUG> )
cnr_target_compile_options(${PROJECT_NAME})

add_executable(test_tasks test/tasks.cpp)
target_link_libraries(test_tasks ${PROJECT_NAME} ${Boost_SYSTEM_LIBRARY} ${Boost_THREAD_LIBRARY})

if(${CMAKE_VERSION} VERSION_GREATER  "3.16.0")
  target_link_libraries(test_tasks Threads::Threads)
else()
  if(THREADS_HAVE_PTHREAD_ARG)
    target_compile_options(test_tasks PUBLIC "-pthread")
  endif()
  if(CMAKE_THREAD_LIBS_INIT)
    target_link_libraries(test_tasks "${CMAKE_THREAD_LIBS_INIT}")
  endif()
endif()
cnr_target_compile_options(test_tasks)

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
