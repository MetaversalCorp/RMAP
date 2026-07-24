cmake_minimum_required(VERSION 3.20)

project(RMAP
   VERSION 0.1.0
   LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Default to a Release build for single-config generators when none is given.
if(NOT CMAKE_BUILD_TYPE  AND  NOT CMAKE_CONFIGURATION_TYPES)
   set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

# ---------------------------------------------------------------------------
# Options. Each service module can be excluded from RMAP.lib; the RMAP core is
# always present because every service depends on it. Turning a service OFF
# drops its subdirectory entirely, so the generated Visual Studio solution (and
# every other generator) simply won't contain that module's project.
# ---------------------------------------------------------------------------

option(RMAP_BUILD_SVC_SB       "Build the SB service module into RMAP.lib"       ON)
option(RMAP_BUILD_SVC_REST     "Build the REST service module into RMAP.lib"     ON)
option(RMAP_BUILD_SVC_SOCKETIO "Build the SocketIO service module into RMAP.lib" ON)

option(RMAP_USE_SYSTEM_JSON "Use an external nlohmann_json (find_package) instead of the git-fetched copy" OFF)

# ---------------------------------------------------------------------------
# Modules. Each module is its own Visual Studio project:
#   - RMAP (core) is a STATIC library and IS the single deliverable, RMAP.lib.
#   - Each RMAP_Svc_* is an OBJECT library whose compiled objects are folded
#     into RMAP.lib via $<TARGET_OBJECTS>. Because that adds objects as sources
#     (not a link dependency), the install/export set stays cleanly just RMAP
#     and each object appears exactly once with no duplicate symbols.
# ---------------------------------------------------------------------------

add_subdirectory(RMAP)

if(RMAP_BUILD_SVC_SB)
   add_subdirectory(RMAP_Svc_SB)
   target_sources(RMAP PRIVATE $<TARGET_OBJECTS:RMAP_Svc_SB>)
   target_include_directories(RMAP PUBLIC
      $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/RMAP_Svc_SB/include>
      $<INSTALL_INTERFACE:include>)
endif()

if(RMAP_BUILD_SVC_REST)
   add_subdirectory(RMAP_Svc_Rest)
   target_sources(RMAP PRIVATE $<TARGET_OBJECTS:RMAP_Svc_Rest>)
   target_include_directories(RMAP PUBLIC
      $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/RMAP_Svc_Rest/include>
      $<INSTALL_INTERFACE:include>)
endif()

if(RMAP_BUILD_SVC_SOCKETIO)
   add_subdirectory(RMAP_Svc_SocketIO)
   target_sources(RMAP PRIVATE $<TARGET_OBJECTS:RMAP_Svc_SocketIO>)
   target_include_directories(RMAP PUBLIC
      $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/RMAP_Svc_SocketIO/include>
      $<INSTALL_INTERFACE:include>)
endif()

# ---------------------------------------------------------------------------
# nlohmann_json. RMAP.h exposes nlohmann::ordered_json in its public API.
# RMAP_USE_SYSTEM_JSON=ON  -> external package via find_package.
# RMAP_USE_SYSTEM_JSON=OFF -> header-only copy fetched from git at build time
#    into RMAP/third_party/json (not committed). Pinned to 3.11.3.
# ---------------------------------------------------------------------------

if(RMAP_USE_SYSTEM_JSON)
   find_package(nlohmann_json 3.11.3 CONFIG REQUIRED)
   target_link_libraries(RMAP PUBLIC nlohmann_json::nlohmann_json)
else()
   include(ExternalProject)

   set(RMAP_JSON_DIR "${CMAKE_CURRENT_SOURCE_DIR}/RMAP/third_party/json")
   if(EXISTS "${RMAP_JSON_DIR}/.git")
      set(_json_git_args)
   else()
      set(_json_git_args
         GIT_REPOSITORY https://github.com/nlohmann/json.git
         GIT_TAG        v3.11.3
         GIT_SHALLOW    ON)
   endif()

   ExternalProject_Add(nlohmann_json_fetch
      ${_json_git_args}
      SOURCE_DIR        "${RMAP_JSON_DIR}"
      CONFIGURE_COMMAND ""
      BUILD_COMMAND     ""
      INSTALL_COMMAND   ""
      TEST_COMMAND      "")

   add_dependencies(RMAP nlohmann_json_fetch)

   target_include_directories(RMAP PUBLIC
      $<BUILD_INTERFACE:${RMAP_JSON_DIR}/include>)
endif()

# ---------------------------------------------------------------------------
# Install + find_package(RMAP) support -> target RMAP::RMAP
# ---------------------------------------------------------------------------

include(GNUInstallDirs)

install(TARGETS RMAP
   EXPORT RMAPTargets
   ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})

install(DIRECTORY RMAP/include/
   DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

if(RMAP_BUILD_SVC_SB)
   install(DIRECTORY RMAP_Svc_SB/include/
      DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
endif()

if(RMAP_BUILD_SVC_REST)
   install(DIRECTORY RMAP_Svc_Rest/include/
      DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
endif()

if(RMAP_BUILD_SVC_SOCKETIO)
   install(DIRECTORY RMAP_Svc_SocketIO/include/
      DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
endif()

install(EXPORT RMAPTargets
   FILE RMAPTargets.cmake
   NAMESPACE RMAP::
   DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/RMAP)

include(CMakePackageConfigHelpers)

configure_package_config_file(
   ${CMAKE_CURRENT_SOURCE_DIR}/cmake/RMAPConfig.cmake.in
   ${CMAKE_CURRENT_BINARY_DIR}/RMAPConfig.cmake
   INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/RMAP)

write_basic_package_version_file(
   ${CMAKE_CURRENT_BINARY_DIR}/RMAPConfigVersion.cmake
   VERSION ${PROJECT_VERSION}
   COMPATIBILITY SameMajorVersion)

install(FILES
   ${CMAKE_CURRENT_BINARY_DIR}/RMAPConfig.cmake
   ${CMAKE_CURRENT_BINARY_DIR}/RMAPConfigVersion.cmake
   DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/RMAP)
