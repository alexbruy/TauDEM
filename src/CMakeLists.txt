set (shape_srcs readoutlets.cpp)
set (common_srcs commonlib.cpp tiffio.cpp)

set (AREAD8 aread8mn.cpp aread8.cpp ${common_srcs} ${shape_srcs})
set (AREADINF areadinfmn.cpp areadinf.cpp ${common_srcs} ${shape_srcs})
set (D8 d8flowdirmn.cpp d8.cpp node.cpp ${common_srcs} ${shape_srcs})
set (D8EXTREAMUP d8flowpathextremeup.cpp d8flowpathextremeupmn.cpp ${common_srcs} ${shape_srcs})
set (D8HDIST d8hdisttostrm.cpp d8hdisttostrmmn.cpp ${common_srcs})
set (DINFAVA dinfavalanche.cpp dinfavalanchemn.cpp ${common_srcs})
set (DINFCONCLIM dinfconclimaccum.cpp dinfconclimaccummn.cpp ${common_srcs} ${shape_srcs})
set (DINFDECAY dinfdecayaccum.cpp dinfdecayaccummn.cpp ${common_srcs} ${shape_srcs})
set (DINFDISTDOWN dinfdistdown.cpp dinfdistdownmn.cpp ${common_srcs})
set (DINFDISTUP dinfdistup.cpp dinfdistupmn.cpp ${common_srcs})
set (DINF dinfflowdirmn.cpp dinf.cpp node.cpp ${common_srcs} ${shape_srcs})
set (DINFREVACCUM dinfrevaccum.cpp dinfrevaccummn.cpp ${common_srcs})
set (DINFTRANSLIMACCUM dinftranslimaccum.cpp dinftranslimaccummn.cpp ${common_srcs} ${shape_srcs})
set (DINFUPDEPEND dinfupdependence.cpp dinfupdependencemn.cpp ${common_srcs})
set (DROPANALYSIS dropanalysis.cpp dropanalysismn.cpp ${common_srcs} ${shape_srcs})
set (GRIDNET gridnetmn.cpp gridnet.cpp ${common_srcs} ${shape_srcs})
set (LENGTHAREA lengtharea.cpp lengthareamn.cpp ${common_srcs})
set (MVOUTLETSTOSTRM moveoutletstostrm.cpp moveoutletstostrmmn.cpp ${common_srcs} ${shape_srcs})
set (PEUKERDOUGLAS peukerdouglas.cpp peukerdouglasmn.cpp ${common_srcs})
set (PITREMOVE flood.cpp pitremovemn.cpp ${common_srcs})
set (SLOPEAREA slopearea.cpp slopeareamn.cpp ${common_srcs})
set (SLOPEAREARATIO slopearearatio.cpp slopearearatiomn.cpp ${common_srcs})
set (SLOPEAVEDOWN slopeavedown.cpp slopeavedownmn.cpp ${common_srcs})
set (STREAMNET streamnetmn.cpp streamnet.cpp ${common_srcs} ${shape_srcs})
set (THRESHOLD threshold.cpp thresholdmn.cpp ${common_srcs})
set (GAGEWATERSHED gagewatershedmn.cpp gagewatershed.cpp ${common_srcs} ${shape_srcs})
set (TWI twimn.cpp twi.cpp ${common_srcs})
set (CONNECTDOWN connectdownmn.cpp connectdown.cpp ${common_srcs} ${shape_srcs})
set (CATCHHYDROGEO catchhydrogeo.cpp catchhydrogeomn.cpp ${common_srcs} ${shape_srcs})
set (CATCHOUTLETS catchoutletsmn.cpp catchoutlets.cpp ${common_srcs} ${shape_srcs})
set (EDITRASTER editrastermn.cpp editraster.cpp ${common_srcs})
set (FLOWDIRCOND flowdirconditionmn.cpp flowdircond.cpp ${common_srcs})
set (RETLIMFLOW retlimflowmn.cpp retlimflow.cpp ${common_srcs})
set (SETREGION setregionmn.cpp setregion.cpp ${common_srcs})
set (SINMAPSI sinmapsimn.cpp sinmapsi.cpp ${common_srcs})
set (INUNMAP inunmapmn.cpp inunmap.cpp ${common_srcs})

add_executable (aread8 ${AREAD8})
add_executable (areadinf ${AREADINF})
add_executable (d8flowdir ${D8})
add_executable (d8flowpathextremeup ${D8EXTREAMUP})
add_executable (d8hdisttostrm ${D8HDIST})
add_executable (dinfavalanche ${DINFAVA})
add_executable (dinfconclimaccum ${DINFCONCLIM})
add_executable (dinfdecayaccum ${DINFDECAY})
add_executable (dinfdistdown ${DINFDISTDOWN})
add_executable (dinfdistup ${DINFDISTUP})
add_executable (dinfflowdir ${DINF})
add_executable (dinfrevaccum ${DINFREVACCUM})
add_executable (dinftranslimaccum ${DINFTRANSLIMACCUM})
add_executable (dinfupdependence ${DINFUPDEPEND})
add_executable (dropanalysis ${DROPANALYSIS})
add_executable (gridnet ${GRIDNET})
add_executable (lengtharea ${LENGTHAREA})
add_executable (moveoutletstostrm ${MVOUTLETSTOSTRM})
add_executable (peukerdouglas ${PEUKERDOUGLAS})
add_executable (pitremove ${PITREMOVE})
add_executable (slopearea ${SLOPEAREA})
add_executable (slopearearatio ${SLOPEAREARATIO})
add_executable (slopeavedown ${SLOPEAVEDOWN})
add_executable (streamnet ${STREAMNET})
add_executable (threshold ${THRESHOLD})
add_executable (gagewatershed ${GAGEWATERSHED})
add_executable (twi ${TWI})
add_executable (connectdown ${CONNECTDOWN})
add_executable (catchhydrogeo ${CATCHHYDROGEO})
add_executable (catchoutlets ${CATCHOUTLETS})
add_executable (editraster ${EDITRASTER})
add_executable (flowdircond ${FLOWDIRCOND})
add_executable (retlimflow ${RETLIMFLOW})
add_executable (setregion ${SETREGION})
add_executable (sinmapsi ${SINMAPSI})

set (APPS_TARGETS aread8
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
                sinmapsi)

foreach(APP IN ITEMS ${APPS_TARGETS})
  target_link_libraries(${APP} ${MPI_LIBRARIES} ${GDAL_LIBRARY})
endforeach(APP ${APPS_TARGETS})
install(TARGETS ${APPS_TARGETS} RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

if (WITH_INUNMAP)
  include_directories(${NETCDF_INCLUDE_DIRS})
  add_executable (inunmap ${INUNMAP})
  target_link_libraries(inunmap ${MPI_LIBRARIES} ${GDAL_LIBRARY} ${NETCDF_LIBRARIES})
  install(TARGETS inunmap RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
endif()
