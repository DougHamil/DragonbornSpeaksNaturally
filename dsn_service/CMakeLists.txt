cmake_minimum_required(VERSION 3.5)

project(dsn_service LANGUAGES CSharp)

include(CSharpUtilities)
set(CMAKE_CSharp_FLAGS "/langversion:6")


###############
# Source codes and Targets
###############

file(GLOB_RECURSE DSN_SERVICE_SRC
    dsn_service/*.cs
    dsn_service/*.xml
    dsn_service/*.config
    dsn_service/*.txt
)

add_executable(dsn_service ${DSN_SERVICE_SRC})


###############
# NuGet package restore
###############

set(CMAKE_BIN_DIR "${CMAKE_CURRENT_SOURCE_DIR}/cmake/bin")
find_program(NUGET nuget PATHS ${CMAKE_BIN_DIR})
if(NOT NUGET)
    message(FATAL_ERROR "Cannot find nuget command line tool, but it should be here: ${CMAKE_BIN_DIR}/nuget.exe")
endif()
add_custom_target(nuget-restore COMMAND
    ${NUGET} restore ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.sln &&
    echo ==================================================================================== &&
    echo If you met assembly missing, run this command in your NuGet Package Manager Console: &&
    echo Update-Package -reinstall -projectname dsn_service &&
    echo ====================================================================================
)

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/dsn_service/packages.config
    ${CMAKE_CURRENT_BINARY_DIR}/packages.config
    COPYONLY
)

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/dsn_service/FodyWeavers.xml
    ${CMAKE_CURRENT_BINARY_DIR}/FodyWeavers.xml
    COPYONLY
)

add_dependencies(dsn_service nuget-restore)


###############
# Project Information
###############

set(VS_DOTNET_REFERENCES
    Microsoft.CSharp
    System
    System.Core
    System.Data
    System.Data.DataSetExtensions
    System.Net.Http
    System.Speech
    System.Web.Extensions
    System.Xml
    System.Xml.Linq
)

set_target_properties(dsn_service PROPERTIES
  VS_GLOBAL_ProjectGuid "{DEA491EE-C426-4B79-A443-CF5B1D795288}"
  VS_DOTNET_TARGET_FRAMEWORK_VERSION "v4.6.1"
  VS_DOTNET_REFERENCES "${VS_DOTNET_REFERENCES}"
  OUTPUT_NAME "DragonbornSpeaksNaturally"
)


###############
# Install and Package
###############

if (SVR_DIR)
    set(SVR_PLUGIN_DIR "${SVR_DIR}/Data/Plugins/Sumwunn")
    message("-- SkyrimVR plugin install path: ${SVR_PLUGIN_DIR}/")

    install(TARGETS dsn_service
        COMPONENT SkyrimVR
        RUNTIME DESTINATION ${SVR_PLUGIN_DIR}
    )
    add_custom_command(
        TARGET dsn_service POST_BUILD VERBATIM
        COMMAND
            ${CMAKE_COMMAND} -E copy "$<TARGET_FILE:dsn_service>" ${SVR_PLUGIN_DIR} &&
            echo file copied: "$<TARGET_FILE:dsn_service> -> ${SVR_PLUGIN_DIR}"
    )
endif()

if (SSE_DIR)
    set(SSE_PLUGIN_DIR "${SSE_DIR}/Data/Plugins/Sumwunn")
    message("-- SkyrimSE plugin install path: ${SSE_PLUGIN_DIR}/")

    install(TARGETS dsn_service
        COMPONENT SkyrimSE
        RUNTIME DESTINATION ${SSE_PLUGIN_DIR}
    )
    add_custom_command(
        TARGET dsn_service POST_BUILD VERBATIM
        COMMAND
            ${CMAKE_COMMAND} -E copy "$<TARGET_FILE:dsn_service>" ${SSE_PLUGIN_DIR} &&
            echo file copied: "$<TARGET_FILE:dsn_service> -> ${SSE_PLUGIN_DIR}"
    )
endif()
