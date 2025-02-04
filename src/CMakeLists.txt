set(READ_OUTLETS_SRC
  readoutlets.cpp
)

set(COMMON_SRC
  commonlib.cpp
  tiffio.cpp
)

add_executable(aread8 aread8.cpp aread8mn.cpp)
add_executable(areadinf areadinf.cpp areadinfmn.cpp)
add_executable(d8flowdir d8.cpp d8flowdirmn.cpp node.cpp)
add_executable(d8flowpathextremeup d8flowpathextremeup.cpp d8flowpathextremeupmn.cpp)
add_executable(d8hdisttostrm d8hdisttostrm.cpp d8hdisttostrmmn.cpp)
add_executable(dinfavalanche dinfavalanche.cpp dinfavalanchemn.cpp)
add_executable(dinfconclimaccum dinfconclimaccum.cpp dinfconclimaccummn.cpp)
add_executable(dinfdecayaccum dinfdecayaccum.cpp dinfdecayaccummn.cpp)
add_executable(dinfdistdown dinfdistdown.cpp dinfdistdownmn.cpp)
add_executable(dinfdistup dinfdistup.cpp dinfdistupmn.cpp)
add_executable(dinfflowdir dinf.cpp dinfflowdirmn.cpp node.cpp)
add_executable(dinfrevaccum dinfrevaccum.cpp dinfrevaccummn.cpp)
add_executable(dinftranslimaccum dinftranslimaccum.cpp dinftranslimaccummn.cpp)
add_executable(dinfupdependence dinfupdependence.cpp dinfupdependencemn.cpp)
add_executable(dropanalysis dropanalysis.cpp dropanalysismn.cpp)
add_executable(gridnet gridnet.cpp gridnetmn.cpp)
add_executable(lengtharea lengtharea.cpp lengthareamn.cpp)
add_executable(moveoutletstostrm moveoutletstostrm.cpp moveoutletstostrmmn.cpp)
add_executable(peukerdouglas peukerdouglas.cpp peukerdouglasmn.cpp)
add_executable(pitremove flood.cpp pitremovemn.cpp)
add_executable(slopearea slopearea.cpp slopeareamn.cpp)
add_executable(slopearearatio slopearearatio.cpp slopearearatiomn.cpp)
add_executable(slopeavedown slopeavedown.cpp slopeavedownmn.cpp)
add_executable(streamnet streamnet.cpp streamnetmn.cpp)
add_executable(threshold threshold.cpp thresholdmn.cpp)
add_executable(gagewatershed gagewatershed.cpp gagewatershedmn.cpp)
add_executable(twi twi.cpp twimn.cpp)
add_executable(connectdown connectdown.cpp connectdownmn.cpp)
add_executable(catchhydrogeo catchhydrogeo.cpp catchhydrogeomn.cpp)
add_executable(catchoutlets catchoutlets.cpp catchoutletsmn.cpp)
add_executable(editraster editraster.cpp editrastermn.cpp)
add_executable(flowdircond flowdircond.cpp flowdirconditionmn.cpp)
add_executable(retlimflow retlimflow.cpp retlimflowmn.cpp)
add_executable(setregion setregion.cpp setregionmn.cpp)
add_executable(sinmapsi sinmapsi.cpp sinmapsimn.cpp)

set(APPS_VECTOR_TARGETS
  aread8
  areadinf
  d8flowdir
  d8flowpathextremeup
  dinfconclimaccum
  dinfdecayaccum
  dinfflowdir
  dinftranslimaccum
  dropanalysis
  gridnet
  moveoutletstostrm
  streamnet
  gagewatershed
  connectdown
  catchhydrogeo
  catchoutlets
)

set(APPS_TARGETS
  aread8
  areadinf
  d8flowdir
  d8flowpathextremeup
  d8hdisttostrm
  dinfavalanche
  dinfconclimaccum
  dinfdecayaccum
  dinfdistdown
  dinfdistup
  dinfflowdir
  dinfrevaccum
  dinftranslimaccum
  dinfupdependence
  dropanalysis
  gridnet
  lengtharea
  moveoutletstostrm
  peukerdouglas
  pitremove
  slopearea
  slopearearatio
  slopeavedown
  streamnet
  threshold
  gagewatershed
  twi
  connectdown
  catchhydrogeo
  catchoutlets
  editraster
  flowdircond
  retlimflow
  setregion
  sinmapsi
)

if(WITH_INUNMAP)
  add_executable (inunmap inunmap.cpp inunmapmn.cpp)
  target_include_directories(inunmap PRIVATE ${NETCDF_INCLUDE_DIRS})
  target_link_libraries(inunmap PRIVATE ${NETCDF_LIBRARIES})
  list(APPEND APPS_TARGETS inunmap)
endif()

foreach(APP IN ITEMS ${APPS_VECTOR_TARGETS})
  target_sources(${APP} PRIVATE ${READ_OUTLETS_SRC})
endforeach()

foreach(APP IN ITEMS ${APPS_TARGETS})
  target_sources(${APP} PRIVATE ${COMMON_SRC})
  target_link_libraries(${APP} PRIVATE ${MPI_LIBRARIES} ${GDAL_LIBRARY})
endforeach()

install(TARGETS ${APPS_TARGETS} RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
