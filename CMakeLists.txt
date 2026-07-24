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
# drops its sources from the single RMAP target, so the generated Visual Studio
# solution (and every other generator) simply won't contain that module.
# ---------------------------------------------------------------------------

option(RMAP_BUILD_SVC_SB       "Build the SB service module into RMAP.lib"       ON)
option(RMAP_BUILD_SVC_REST     "Build the REST service module into RMAP.lib"     ON)
option(RMAP_BUILD_SVC_SOCKETIO "Build the SocketIO service module into RMAP.lib" ON)

option(RMAP_USE_SYSTEM_JSON "Use an external nlohmann_json (find_package) instead of the git-fetched copy" OFF)

# ---------------------------------------------------------------------------
# The single deliverable: RMAP.lib. Everything is compiled into this one target
# so exactly one archive is produced. Keep this core source list in step with
# the actual files under RMAP/src.
# ---------------------------------------------------------------------------

add_library(RMAP STATIC
   RMAP/src/base/Client.cpp
   RMAP/src/base/Model.cpp
   RMAP/src/base/Model_Object.cpp
   RMAP/src/base/Model_Session.cpp
   RMAP/src/base/Notification.cpp
   RMAP/src/base/Service.cpp
   RMAP/src/base/Source.cpp
   RMAP/src/base/Source_Session.cpp
   RMAP/src/core/Core.cpp
   RMAP/src/core/ModelClass.cpp
   RMAP/src/core/Namespace.cpp
   RMAP/src/core/ServiceClass.cpp
   RMAP/src/core/SourceClass.cpp
   RMAP/src/mem/Mem.cpp
   RMAP/src/mem/Mem_Model.cpp
   RMAP/src/mem/Mem_Source.cpp
   RMAP/src/mem/Objectbank.cpp
   RMAP/src/mem/Objectbank_Dep.cpp
   RMAP/src/mem/Objectbank_Ind.cpp
   RMAP/src/model/Model_Session_C2a.cpp
   RMAP/src/model/Model_Session_Null.cpp
   RMAP/src/model/Model_Session_UiP.cpp
   RMAP/src/Library.cpp
   RMAP/src/LibraryRMAP.cpp
   RMAP/src/LnG.cpp
   RMAP/src/Logger.cpp
   RMAP/src/Package.cpp
   RMAP/src/pch.cpp
   RMAP/src/Plugin.cpp
   RMAP/src/Registry.cpp
   RMAP/src/Utils.cpp)

add_library(RMAP::RMAP ALIAS RMAP)

target_compile_features(RMAP PUBLIC cxx_std_17)

# Static build: RMAP.h's export macro expands to nothing.
target_compile_definitions(RMAP PUBLIC RMAP_CORE_API=)

target_include_directories(RMAP
   PUBLIC
      $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/RMAP/include>
      $<INSTALL_INTERFACE:include>
   PRIVATE
      ${CMAKE_CURRENT_SOURCE_DIR}/RMAP/include/RMAP
      ${CMAKE_CURRENT_SOURCE_DIR}/RMAP/src)

# MSVC parity with the previous hand-authored project settings.
if(MSVC)
   target_compile_definitions(RMAP PRIVATE UNICODE _UNICODE _LIB)
   target_compile_options(RMAP PRIVATE /permissive- /sdl /W3)
   set_target_properties(RMAP PROPERTIES
      INTERPROCEDURAL_OPTIMIZATION_RELEASE ON)
endif()

# ---------------------------------------------------------------------------
# Optional service modules. Each enabled module contributes its sources and
# public headers to the same RMAP target, so they land in the one RMAP.lib.
# ---------------------------------------------------------------------------

if(RMAP_BUILD_SVC_SB)
   target_sources(RMAP PRIVATE RMAP_Svc_SB/src/RMAP_Svc_SB.cpp)
   target_include_directories(RMAP PUBLIC
      $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/RMAP_Svc_SB/include>
      $<INSTALL_INTERFACE:include>)
endif()

if(RMAP_BUILD_SVC_REST)
   target_sources(RMAP PRIVATE RMAP_Svc_Rest/src/RMAP_Svc_Rest.cpp)
   target_include_directories(RMAP PUBLIC
      $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/RMAP_Svc_Rest/include>
      $<INSTALL_INTERFACE:include>)
endif()

if(RMAP_BUILD_SVC_SOCKETIO)
   target_sources(RMAP PRIVATE RMAP_Svc_SocketIO/src/RMAP_Svc_SocketIO.cpp)
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

target_precompile_headers(RMAP PRIVATE RMAP/src/pch.h)

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
