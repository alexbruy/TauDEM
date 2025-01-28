#############################################################
# CMake settings
cmake_minimum_required(VERSION 3.23.0)
set(CMAKE_COLOR_MAKEFILE ON)

# set path to additional CMake modules
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH})

#############################################################
# Project and version

set(CPACK_PACKAGE_VERSION_MAJOR "5")
set(CPACK_PACKAGE_VERSION_MINOR "3")
set(CPACK_PACKAGE_VERSION_PATCH "8")
set(COMPLETE_VERSION ${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH})
set(RELEASE_NAME "Master")
project(taudem VERSION ${COMPLETE_VERSION})

# Note the version no is Mmmpp for Major/minor/patch, 0-padded, thus '10100' for 1.1.0
math(EXPR TAUDEM_VERSION_INT "${CPACK_PACKAGE_VERSION_MAJOR}*10000+${CPACK_PACKAGE_VERSION_MINOR}*100+${CPACK_PACKAGE_VERSION_PATCH}")
message(STATUS "TauDEM version: ${COMPLETE_VERSION} ${RELEASE_NAME} (${TAUDEM_VERSION_INT})")

set (WITH_INUNMAP FALSE CACHE BOOL "Determines whether inunmap tool should be built")

#############################################################
# search for dependencies

find_package(GDAL REQUIRED)
message(STATUS "Found GDAL: ${GDAL_VERSION} ${GDAL_DIR}")

find_package(MPI REQUIRED)
message(STATUS "Found MPI: ${MPI_CXX_VERSION} ${MPI_HOME}")

if (WITH_INUNMAP)
  find_package(NetCDF REQUIRED)
  message(STATUS "Found NetCDF: ${NETCDF_VERSION} ${NETCDF_INCLUDE_DIRS}")
endif()

#############################################################

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAG "${CMAKE_CXX_FLAG} ${MPI_CXX_COMPILE_OPTIONS}")
set(CMAKE_CXX_LINK_FLAGS "${CMAKE_CXX_LINK_FLAGS} ${MPI_CXX_LINK_FLAGS}")

#############################################################
# process subdirs

add_subdirectory(src)
