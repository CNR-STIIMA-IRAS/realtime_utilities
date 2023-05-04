#############################
## PRELIMINARIES           ##
#############################
cmake_minimum_required(VERSION 3.1.0)

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/cnrConfigMacros.cmake)

get_project_name("package.xml" extracted_name extracted_version)

if("${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}" LESS 3.0.2)
  project(${extracted_name})
else()
  project(${extracted_name} VERSION ${extracted_version} LANGUAGES CXX)
endif()

cnr_set_flags()
#############################
## END - PRELIMINARIES     ##
#############################


#############################
## OPTIONS                 ##
#############################
option(USE_ROS1                 "ROS SUPPORT" ON)
option(ENABLE_TESTING           "ENABLE TESTING" OFF)
option(ENABLE_COVERAGE_TESTING  "ENABLE COVERAGE TESTING" OFF)
option(COMPILE_EXAMPLE          "COMPILE THE EXAMPLE" OFF)

if(USE_ROS1)
  set(ENABLE_TESTING ${CATKIN_ENABLE_TESTING})
endif()
#############################
## END - OPTIONS           ##
#############################


#############################
## DEPENDANCIES            ##
#############################
if(USE_ROS1)
  find_package(catkin REQUIRED COMPONENTS roscpp)
  catkin_package(
    INCLUDE_DIRS include
    LIBRARIES ${PROJECT_NAME}
    CATKIN_DEPENDS roscpp 
  ) 
endif()

set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)
find_package(Boost COMPONENTS system thread REQUIRED)

cnr_install_directories(USE_ROS1 
  CNR_INSTALL_INCLUDE_DIR CNR_INSTALL_LIB_DIR CNR_INSTALL_BIN_DIR CNR_INSTALL_SHARE_DIR)
cnr_enable_testing(ENABLE_TESTING ENABLE_COVERAGE USE_ROS1)
#############################
## END DEPENDANCIES        ##
#############################



#################
## Build       ##
#################
list(APPEND DEPENDENCIES_INCLUDE_DIRS ${Boost_INCLUDE_DIRS})
if(USE_ROS1)
  list(APPEND DEPENDENCIES_INCLUDE_DIRS ${catkin_INCLUDE_DIRS})
endif()

list(APPEND BUILD_INTERFACE_INCLUDE_DIRS ${DEPENDENCIES_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR}/include)
list(APPEND INSTALL_INTERFACE_INCLUDE_DIRS include ${DEPENDENCIES_INCLUDE_DIRS} include)

# ================================================
add_library(${PROJECT_NAME}
  src/${PROJECT_NAME}/realtime_utilities.cpp src/${PROJECT_NAME}/shared_mutex.cpp
)

target_include_directories(${PROJECT_NAME} PUBLIC
  "$<BUILD_INTERFACE:${BUILD_INTERFACE_INCLUDE_DIRS}>"
    "$<INSTALL_INTERFACE:${INSTALL_INTERFACE_INCLUDE_DIRS}>")

if(USE_ROS1)
  target_link_libraries(${PROJECT_NAME} ${BOOST_LIBRARIES} ${catkin_LIBRARIES})
else()
  target_link_libraries(${PROJECT_NAME} PUBLIC Boost::system PUBLIC ${coe_core_LIBRARIES})
endif()
#################
## END - Build ##
#################



#############
## Install ##
#############
install(DIRECTORY include/${PROJECT_NAME}
        DESTINATION ${CNR_INSTALL_INCLUDE_DIR}
        FILES_MATCHING PATTERN "*.h"
        PATTERN ".git" EXCLUDE
)

install(TARGETS ${PROJECT_NAME} 
        EXPORT export_realtime_utilities
        ARCHIVE DESTINATION ${CNR_INSTALL_LIB_DIR}
        LIBRARY DESTINATION ${CNR_INSTALL_LIB_DIR}
        RUNTIME DESTINATION ${CNR_INSTALL_BIN_DIR}
)
###################
## END - Install ##
###################

##################################
## CMAKE CONFIG (IF NOT CATKIN) ##
##################################
if(NOT USE_ROS1) 
  set(export_targets ${export_targets};${PROJECT_NAME})
  export(EXPORT export_realtime_utilities
    FILE "${PROJECT_BINARY_DIR}/export_realtime_utilities.cmake")

  # Create the realtime_utilitiesConfig.cmake
  set(CONF_INCLUDE_DIRS "${CMAKE_INSTALL_PREFIX}/include")
  configure_file(realtime_utilitiesConfig.cmake.in
    "${PROJECT_BINARY_DIR}/${CMAKE_FILES_DIRECTORY}/realtime_utilitiesConfig.cmake" @ONLY)

  # Create the realtime_utilitiesConfigVersion.cmake
  configure_file(realtime_utilitiesConfigVersion.cmake.in
    "${PROJECT_BINARY_DIR}/realtime_utilitiesConfigVersion.cmake" @ONLY)
  
  install(FILES
    "${PROJECT_BINARY_DIR}/${CMAKE_FILES_DIRECTORY}/realtime_utilitiesConfig.cmake"
    "${PROJECT_BINARY_DIR}/realtime_utilitiesConfigVersion.cmake"
    DESTINATION "share/${PROJECT_NAME}/cmake" COMPONENT dev
  )

  install(EXPORT export_realtime_utilities
    DESTINATION "share/${PROJECT_NAME}/cmake"
    FILE export_realtime_utilities.cmake
    COMPONENT dev
  )
endif()


##########################
## Testing              ##
##########################
if(ENABLE_TESTING)
  include(GoogleTest)
  add_executable(test_tasks test/tasks.cpp)
  target_link_libraries(test_tasks ${PROJECT_NAME} -lpthread ${Boost_SYSTEM_LIBRARY} ${Boost_THREAD_LIBRARY})


  if(${CMAKE_VERSION} VERSION_GREATER  "3.16.0")
     target_link_libraries(test_tasks
       ${PROJECT_NAME} Threads::Threads GTest::gtest_main
         Boost::system Boost::thread)
    else()
    target_link_libraries(test_tasks ${PROJECT_NAME} GTest::gtest_main)
    if(THREADS_HAVE_PTHREAD_ARG)
      target_compile_options(test_tasks PUBLIC "-pthread")
    endif()
    if(CMAKE_THREAD_LIBS_INIT)
      target_link_libraries(test_tasks "${CMAKE_THREAD_LIBS_INIT}")
    endif()
  endif()
  target_compile_definitions(test_tasks PRIVATE TEST_DIR="${CMAKE_CURRENT_LIST_DIR}/test")
  
  gtest_discover_tests(test_tasks)
 endif(ENABLE_TESTING)


##########################
## Coverage Testing     ##
##########################
if( ENABLE_TESTING AND ENABLE_COVERAGE_TESTING AND NOT WIN32 )
  if(USE_ROS)
    message(STATUS "The coverage testing is enabled")
    set(COVERAGE_EXCLUDES "*/${PROJECT_NAME}/test*")
    add_code_coverage(
      NAME coverage_report
      DEPENDENCIES test_tasks
    )
  else()
    ## NOTHING TO DO, SINCE GTEST ALREADY CREATES THE COVERAGE
    ## IF THE COMPILING OPTIONS ARE SET
  endif()
endif()

