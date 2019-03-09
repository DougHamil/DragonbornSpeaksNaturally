cmake_minimum_required(VERSION 3.5)

project(DSN LANGUAGES CXX CSharp)


#
# Sub Projects 
#
set(IS_SUB_PROJECT ON)

message(" ") # empty line
message("================ Configure sub-project dsn_plugin ================")
add_subdirectory(dsn_plugin)

message(" ") # empty line
message("================ Configure sub-project dsn_service ================")
add_subdirectory(dsn_service)


#
# ZIP Package 
#
message(" ") # empty line
message("================ Configure ZIP Packages ================")

option(PACKAGE "Generate NMM/Vortex Compatible ZIP Package" ON)
if (PACKAGE)
    message("-- Generate NMM/Vortex Compatible ZIP Package: On (-DPACKAGE=ON)")

    set(CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_BINARY_DIR}/package_tmp)
    install(
        DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/fomod
        DESTINATION .
    )
    install(
        FILES
            ${CMAKE_CURRENT_SOURCE_DIR}/DragonbornSpeaksNaturally.SAMPLE.ini
            ${CMAKE_CURRENT_SOURCE_DIR}/item-name-map.SAMPLE.json
        DESTINATION SkyrimVR/Data/Plugins/Sumwunn
    )
    install(
        FILES
            ${CMAKE_CURRENT_SOURCE_DIR}/DragonbornSpeaksNaturally.SAMPLE.ini
            ${CMAKE_CURRENT_SOURCE_DIR}/item-name-map.SAMPLE.json
        DESTINATION SkyrimSE/Data/Plugins/Sumwunn
    )

    set(CPACK_GENERATOR ZIP)
    set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY OFF)
    include(CPack)
else()
    message("-- Generate NMM/Vortex Compatible ZIP Package: Off (-DPACKAGE=OFF)")
endif()
