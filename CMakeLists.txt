cmake_minimum_required(VERSION 3.5)
project(ethercat_erob)


set(CMAKE_BUILD_TYPE Debug)  # or Release, RelWithDebInfo, etc.
#find_package(ament_cmake REQUIRED)

set(IGH_FREQUENCY 1000 CACHE STRING "EtherCAT cyclic frequency in Hz for igh_driver_fix")
set(IGH_STARTUP_FREQUENCY 1000 CACHE STRING "EtherCAT startup frequency in Hz before the slave reaches OP")
option(IGH_ENABLE_DC "Enable distributed clocks for igh_driver_fix" ON)
math(EXPR IGH_PERIOD_NS "1000000000 / ${IGH_FREQUENCY}")
math(EXPR IGH_PERIOD_REMAINDER_NS "1000000000 % ${IGH_FREQUENCY}")
math(EXPR IGH_STARTUP_PERIOD_NS "1000000000 / ${IGH_STARTUP_FREQUENCY}")
math(EXPR IGH_STARTUP_PERIOD_REMAINDER_NS "1000000000 % ${IGH_STARTUP_FREQUENCY}")
if(NOT IGH_PERIOD_REMAINDER_NS EQUAL 0)
  message(WARNING
    "IGH_FREQUENCY=${IGH_FREQUENCY} does not divide 1 second exactly. "
    "The program will use PERIOD_NS=${IGH_PERIOD_NS} ns, so prefer 1000/2000/4000/8000 Hz for DC tests.")
endif()
if(NOT IGH_STARTUP_PERIOD_REMAINDER_NS EQUAL 0)
  message(WARNING
    "IGH_STARTUP_FREQUENCY=${IGH_STARTUP_FREQUENCY} does not divide 1 second exactly. "
    "The program will use STARTUP_PERIOD_NS=${IGH_STARTUP_PERIOD_NS} ns.")
endif()


# EtherLab
set(ETHERLAB_DIR /usr/local/etherlab)
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

find_library(ETHERCAT_LIB ethercat HINTS ${ETHERLAB_DIR}/lib)

include_directories(
  ${ETHERLAB_DIR}/include
)

add_executable(
  igh_driver
  src/igh_driver.cpp)
  
target_include_directories(
  igh_driver
  PRIVATE
  ${ETHERLAB_DIR}/include
)
target_link_libraries(igh_driver ${ETHERCAT_LIB})

add_executable(
  igh_driver_fix
  src/igh_driver_fix.cpp)

target_include_directories(
  igh_driver_fix
  PRIVATE
  ${ETHERLAB_DIR}/include
)
target_compile_definitions(
  igh_driver_fix
  PRIVATE
  FREQUENCY=${IGH_FREQUENCY}
  STARTUP_FREQUENCY=${IGH_STARTUP_FREQUENCY}
)
if(NOT IGH_ENABLE_DC)
  target_compile_definitions(igh_driver_fix PRIVATE IGH_DISABLE_DC)
endif()
target_link_libraries(igh_driver_fix ${ETHERCAT_LIB})



