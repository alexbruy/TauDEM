# - Find NetCDF
# Find the native NetCDF headers and libraries
#
# This module defines the follwing variables:
#
#  NETCDF_FOUND           - True if NetCDF found
#  NETCDF_VERSION         - The version of NetCDF found
#  NETCDF_LIBRARIES       - The NetCDF libraries
#  NETCDF_INCLUDE_DIRS    - Include directories necessary to use NetCDF

# Try to find a CMake-built NetCDF
FIND_PACKAGE(netCDF CONFIG QUIET)
if (netCDF_FOUND)
  # Forward the variables in a consistent way.
  set(NETCDF_FOUND "${netCDF_FOUND}")
  set(NETCDF_VERSION "${NetCDFVersion}")
  set(NETCDF_LIBRARIES "${netCDF_LIBRARIES}")
  set(NETCDF_INCLUDE_DIRS "${netCDF_INCLUDE_DIR}")

  include(FindPackageHandleStandardArgs)
  find_package_handle_standard_args(NetCDF
    REQUIRED_VARS NETCDF_INCLUDE_DIRS NETCDF_LIBRARIES
    VERSION_VAR NETCDF_VERSION)

  return()
endif ()

FIND_PACKAGE(PkgConfig QUIET)
IF (PkgConfig_FOUND)
  pkg_check_modules(_NetCDF QUIET netcdf IMPORTED_TARGET)
  if (_NetCDF_FOUND)
    # Forward the variables in a consistent way.
    set(NETCDF_FOUND "${_NetCDF_FOUND}")
    set(NETCDF_INCLUDE_DIRS "${_NetCDF_INCLUDE_DIRS}")
    set(NETCDF_LIBRARIES "${_NetCDF_LIBRARIES}")
    set(NETCDF_VERSION "${_NetCDF_VERSION}")

    include(FindPackageHandleStandardArgs)
    find_package_handle_standard_args(NetCDF
      REQUIRED_VARS NETCDF_LIBRARIES
      VERSION_VAR NETCDF_VERSION)
    return()
  endif()
endif()

find_path(NETCDF_INCLUDE_DIR
  NAMES netcdf.h
  DOC "NetCDF include directories")
mark_as_advanced(NETCDF_INCLUDE_DIR)

find_library(NETCDF_LIBRARY
  NAMES netcdf
  DOC "NetCDF library")
mark_as_advanced(NETCDF_LIBRARY)

if (NetCDF_INCLUDE_DIR)
  file(STRINGS "${NetCDF_INCLUDE_DIR}/netcdf_meta.h" _netcdf_version_lines
    REGEX "#define[ \t]+NC_VERSION_(MAJOR|MINOR|PATCH|NOTE)")
  string(REGEX REPLACE ".*NC_VERSION_MAJOR *\([0-9]*\).*" "\\1" _netcdf_version_major "${_netcdf_version_lines}")
  string(REGEX REPLACE ".*NC_VERSION_MINOR *\([0-9]*\).*" "\\1" _netcdf_version_minor "${_netcdf_version_lines}")
  string(REGEX REPLACE ".*NC_VERSION_PATCH *\([0-9]*\).*" "\\1" _netcdf_version_patch "${_netcdf_version_lines}")
  string(REGEX REPLACE ".*NC_VERSION_NOTE *\"\([^\"]*\)\".*" "\\1" _netcdf_version_note "${_netcdf_version_lines}")
  set(NETCDF_VERSION "${_netcdf_version_major}.${_netcdf_version_minor}.${_netcdf_version_patch}${_netcdf_version_note}")
  unset(_netcdf_version_major)
  unset(_netcdf_version_minor)
  unset(_netcdf_version_patch)
  unset(_netcdf_version_note)
  unset(_netcdf_version_lines)
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(NetCDF
  REQUIRED_VARS NETCDF_LIBRARY NETCDF_INCLUDE_DIR
  VERSION_VAR NETCDF_VERSION)

if (NetCDF_FOUND)
  set(NETCDF_INCLUDE_DIRS "${NETCDF_INCLUDE_DIR}")
  set(NETCDF_LIBRARIES "${NETCDF_LIBRARY}")
endif()
