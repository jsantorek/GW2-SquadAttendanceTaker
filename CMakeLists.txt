cmake_minimum_required(VERSION 3.5.0...3.31.2)
project(SquadAttendanceTaker VERSION 0.1.0 LANGUAGES C CXX)

find_package(imgui)
find_package(nlohmann_json)
find_package(stduuid)
find_package(nexus-core-api)
include(${CMAKE_CURRENT_SOURCE_DIR}/src-re/GW2RE.cmake)
file(GLOB SRCs ${CMAKE_CURRENT_SOURCE_DIR}/src/*.[hc]pp)
add_library(${PROJECT_NAME}
    SHARED
        ${SRCs}
        ${RE_SRCs}
)
target_link_libraries(${PROJECT_NAME}
    imgui::imgui
    nexus-core-api::nexus-core-api
    nlohmann_json::nlohmann_json
    psapi
    ntdll
    stduuid::stduuid
)
target_compile_definitions(${PROJECT_NAME}
    PRIVATE
        ADDON_VERSION_MAJOR=0
        ADDON_VERSION_MINOR=1
        ADDON_VERSION_PATCH=0
        ADDON_VERSION_REVISION=0
        ADDON_NAME="${PROJECT_NAME}"
)
target_include_directories(${PROJECT_NAME}
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${RE_INCLUDE_DIRS}
)
set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "")
install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION "."
)