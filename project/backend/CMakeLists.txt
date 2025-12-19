cmake_minimum_required(VERSION 3.14)

# Backend library/executable
project(backend CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Gather source files
set(BACKEND_SOURCES
    main.cpp
    GameEngine.cpp
    ControllerRobot.cpp
    Level.cpp
    LevelLoader.cpp
    RequestHandler.cpp
    WorkerRobot.cpp
    #JsonBuilder.cpp
)
add_executable(oop_backend ${BACKEND_SOURCES})


# Try to locate a local single-header installation of nlohmann/json first
find_path(NLOHMANN_JSON_INCLUDE_DIR
    NAMES nlohmann/json.hpp
    HINTS
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/backend/include
        ${CMAKE_SOURCE_DIR}/external/json/single_include
        ${CMAKE_SOURCE_DIR}/external/json/include
        ${CMAKE_SOURCE_DIR}/external/json
)

if(NLOHMANN_JSON_INCLUDE_DIR)
    target_include_directories(oop_backend PRIVATE ${NLOHMANN_JSON_INCLUDE_DIR})
else()
    # Try to find an installed package (vcpkg or system)
    find_package(nlohmann_json CONFIG QUIET)
    if(TARGET nlohmann_json::nlohmann_json)
        target_link_libraries(oop_backend PRIVATE nlohmann_json::nlohmann_json)
    else()
        # As a last resort, download it with FetchContent (only if nothing else found)
        message(STATUS "nlohmann/json not found locally — Fetching with FetchContent as a fallback")
        include(FetchContent)
        FetchContent_Declare(
            json
            GIT_REPOSITORY https://github.com/nlohmann/json.git
            GIT_TAG v3.11.2
        )
        FetchContent_MakeAvailable(json)
        target_link_libraries(oop_backend PRIVATE nlohmann_json::nlohmann_json)
    endif()
endif()

set_target_properties(oop_backend PROPERTIES
    LINKER_LANGUAGE CXX
    WIN32_EXECUTABLE OFF
)
