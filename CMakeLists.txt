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
option(RMAP_USE_SYSTEM_ASIO "Use an external asio (find_package) instead of the git-fetched copy" OFF)
option(RMAP_USE_SYSTEM_WEBSOCKETPP "Use an external websocketpp (find_package) instead of the git-fetched copy" OFF)
option(RMAP_USE_SYSTEM_BORINGSSL "Use an external BoringSSL (find_package) instead of the git-built copy" OFF)
option(RMAP_USE_SYSTEM_CURL "Use an external libcurl (find_package) instead of the git-built copy" OFF)

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
# asio (standalone, header-only). Pinned to 1.30.2.
# RMAP_USE_SYSTEM_ASIO=ON  -> external package via find_package.
# RMAP_USE_SYSTEM_ASIO=OFF -> headers fetched from git at build time into
#    RMAP/third_party/asio (not committed). Headers live under asio/include.
# ASIO_STANDALONE forces the non-Boost, header-only asio in both cases.
# ---------------------------------------------------------------------------

if(RMAP_USE_SYSTEM_ASIO)
   find_package(asio 1.30.2 CONFIG REQUIRED)
   target_link_libraries(RMAP PUBLIC asio::asio)
else()
   include(ExternalProject)

   set(RMAP_ASIO_DIR "${CMAKE_CURRENT_SOURCE_DIR}/RMAP/third_party/asio")
   if(EXISTS "${RMAP_ASIO_DIR}/.git")
      set(_asio_git_args)
   else()
      set(_asio_git_args
         GIT_REPOSITORY https://github.com/chriskohlhoff/asio.git
         GIT_TAG        asio-1-30-2
         GIT_SHALLOW    ON)
   endif()

   ExternalProject_Add(asio_fetch
      ${_asio_git_args}
      SOURCE_DIR        "${RMAP_ASIO_DIR}"
      CONFIGURE_COMMAND ""
      BUILD_COMMAND     ""
      INSTALL_COMMAND   ""
      TEST_COMMAND      "")

   add_dependencies(RMAP asio_fetch)

   target_include_directories(RMAP PUBLIC
      $<BUILD_INTERFACE:${RMAP_ASIO_DIR}/asio/include>)
endif()

target_compile_definitions(RMAP PUBLIC ASIO_STANDALONE)

# ---------------------------------------------------------------------------
# websocketpp (header-only). Pinned to 0.8.2. Layers on top of asio.
# RMAP_USE_SYSTEM_WEBSOCKETPP=ON  -> external package via find_package.
# RMAP_USE_SYSTEM_WEBSOCKETPP=OFF -> headers fetched from git at build time into
#    RMAP/third_party/websocketpp (not committed). The include dir is the clone
#    root itself because headers are included as <websocketpp/...>.
# ---------------------------------------------------------------------------

if(RMAP_USE_SYSTEM_WEBSOCKETPP)
   find_package(websocketpp 0.8.2 CONFIG REQUIRED)
   target_link_libraries(RMAP PUBLIC websocketpp::websocketpp)
else()
   include(ExternalProject)

   set(RMAP_WEBSOCKETPP_DIR "${CMAKE_CURRENT_SOURCE_DIR}/RMAP/third_party/websocketpp")
   if(EXISTS "${RMAP_WEBSOCKETPP_DIR}/.git")
      set(_websocketpp_git_args)
   else()
      set(_websocketpp_git_args
         GIT_REPOSITORY https://github.com/zaphoyd/websocketpp.git
         GIT_TAG        0.8.2
         GIT_SHALLOW    ON)
   endif()

   ExternalProject_Add(websocketpp_fetch
      ${_websocketpp_git_args}
      SOURCE_DIR        "${RMAP_WEBSOCKETPP_DIR}"
      CONFIGURE_COMMAND ""
      BUILD_COMMAND     ""
      INSTALL_COMMAND   ""
      TEST_COMMAND      "")

   add_dependencies(RMAP websocketpp_fetch)

   target_include_directories(RMAP PUBLIC
      $<BUILD_INTERFACE:${RMAP_WEBSOCKETPP_DIR}>)
endif()

# websocketpp uses Boost for common facilities unless told to use the C++11
# standard library (matching our standalone asio). Analogous to ASIO_STANDALONE.
target_compile_definitions(RMAP PUBLIC _WEBSOCKETPP_CPP11_STL_)

# ---------------------------------------------------------------------------
# BoringSSL (COMPILED - not header-only). Tracks google/boringssl main, matching
# the version Sneeze uses (C:\Dev\OMB\Sneeze\deps\boringssl.cmake). Built from
# source into static libcrypto/libssl via its own CMake build, using the parent
# generator (portable across Windows/Linux/macOS). Linked PRIVATE (internal use;
# kept out of RMAP's public API and export set). The $<CONFIG> genex is confined
# to CMAKE_ARGS / BUILD_BYPRODUCTS / target properties / --config (fields that
# expand it) - not ExternalProject dir args, which do not.
# Build prereqs: Go + a C/C++ compiler (+ NASM on Windows x86/x64).
# ---------------------------------------------------------------------------

if(NOT RMAP_USE_SYSTEM_BORINGSSL)
   include(ExternalProject)

   set(RMAP_BORINGSSL_DIR    "${CMAKE_CURRENT_SOURCE_DIR}/RMAP/third_party/boringssl")
   set(RMAP_BORINGSSL_PREFIX "${CMAKE_CURRENT_BINARY_DIR}/boringssl/$<CONFIG>")
   set(RMAP_BORINGSSL_SSL_LIB
      "${RMAP_BORINGSSL_PREFIX}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}ssl${CMAKE_STATIC_LIBRARY_SUFFIX}")
   set(RMAP_BORINGSSL_CRYPTO_LIB
      "${RMAP_BORINGSSL_PREFIX}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}crypto${CMAKE_STATIC_LIBRARY_SUFFIX}")

   if(EXISTS "${RMAP_BORINGSSL_DIR}/.git")
      set(_boringssl_git_args)
   else()
      set(_boringssl_git_args
         GIT_REPOSITORY https://github.com/google/boringssl.git
         GIT_TAG        main
         GIT_SHALLOW    ON)
   endif()

   # BoringSSL's CMakeLists does enable_language(ASM_NASM) without probing common
   # install paths, so forward an absolute nasm path if one is found (winget drops
   # nasm under %LOCALAPPDATA%/bin/NASM, which is not on PATH out of the box).
   set(_boringssl_extra_args)
   if(WIN32  AND  NOT CMAKE_CROSSCOMPILING)
      find_program(NASM_EXECUTABLE nasm
         HINTS
            "$ENV{LOCALAPPDATA}/bin/NASM"
            "$ENV{ProgramFiles}/NASM"
            "$ENV{ProgramFiles\(x86\)}/NASM")
      if(NASM_EXECUTABLE)
         list(APPEND _boringssl_extra_args -DCMAKE_ASM_NASM_COMPILER=${NASM_EXECUTABLE})
      endif()
   endif()

   ExternalProject_Add(boringssl_build
      ${_boringssl_git_args}
      SOURCE_DIR "${RMAP_BORINGSSL_DIR}"
      BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}/boringssl/build"
      CMAKE_ARGS
         -DCMAKE_INSTALL_PREFIX=${RMAP_BORINGSSL_PREFIX}
         -DBUILD_SHARED_LIBS=OFF
         -DCMAKE_BUILD_TYPE=$<CONFIG>
         -DBUILD_TESTING=OFF
         ${_boringssl_extra_args}
      BUILD_COMMAND   ${CMAKE_COMMAND} --build   <BINARY_DIR> --config $<CONFIG>
      INSTALL_COMMAND ${CMAKE_COMMAND} --install <BINARY_DIR> --config $<CONFIG>
      BUILD_BYPRODUCTS
         "${RMAP_BORINGSSL_SSL_LIB}"
         "${RMAP_BORINGSSL_CRYPTO_LIB}")

   add_dependencies(RMAP boringssl_build)

   # Build-only PUBLIC include so the service OBJECT libs (which read RMAP's
   # INTERFACE include dirs) can compile against <openssl/...>, without adding
   # anything to the installed/exported interface.
   target_include_directories(RMAP PUBLIC
      $<BUILD_INTERFACE:${RMAP_BORINGSSL_PREFIX}/include>)

   # PRIVATE link keeps the static libs out of RMAP's export set.
   target_link_libraries(RMAP PRIVATE
      "${RMAP_BORINGSSL_SSL_LIB}"
      "${RMAP_BORINGSSL_CRYPTO_LIB}")
   if(WIN32)
      target_link_libraries(RMAP PRIVATE ws2_32 crypt32)
   elseif(UNIX  AND  NOT APPLE)
      target_link_libraries(RMAP PRIVATE pthread dl)
   endif()
endif()

# ---------------------------------------------------------------------------
# libcurl (COMPILED - not header-only). Pinned to 8.9.1 (tag curl-8_9_1),
# matching the version Sneeze uses (C:\Dev\OMB\Sneeze\deps\curl.cmake). Built
# from source into a static libcurl via its own CMake build, using the parent
# generator. Linked PRIVATE (internal use; kept out of RMAP's exported
# interface). TLS backend: Windows uses Schannel (the OS stack); other platforms
# link the BoringSSL built above through curl's OpenSSL-compatible interface.
# CURL_STATICLIB is required by every TU that includes <curl/curl.h>; it is
# build-only so it stays out of RMAP's exported interface. The static lib is
# named libcurl.lib / libcurl-d.lib on Windows (curl's -d debug postfix),
# libcurl.a elsewhere - confirmed against Sneeze's on-disk build.
# ---------------------------------------------------------------------------

if(NOT RMAP_USE_SYSTEM_CURL)
   include(ExternalProject)

   set(RMAP_CURL_DIR    "${CMAKE_CURRENT_SOURCE_DIR}/RMAP/third_party/curl")
   set(RMAP_CURL_PREFIX "${CMAKE_CURRENT_BINARY_DIR}/curl/$<CONFIG>")
   if(WIN32)
      set(RMAP_CURL_LIB "${RMAP_CURL_PREFIX}/lib/libcurl$<$<CONFIG:Debug>:-d>${CMAKE_STATIC_LIBRARY_SUFFIX}")
   else()
      set(RMAP_CURL_LIB "${RMAP_CURL_PREFIX}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}curl${CMAKE_STATIC_LIBRARY_SUFFIX}")
   endif()

   if(EXISTS "${RMAP_CURL_DIR}/.git")
      set(_curl_git_args)
   else()
      set(_curl_git_args
         GIT_REPOSITORY https://github.com/curl/curl.git
         GIT_TAG        curl-8_9_1
         GIT_SHALLOW    ON)
   endif()

   # TLS backend: Windows uses Schannel (OS stack); other platforms link the
   # BoringSSL built above via curl's OpenSSL-compatible interface.
   if(WIN32)
      set(_curl_ssl_args -DCURL_USE_SCHANNEL=ON -DCURL_USE_OPENSSL=OFF)
   else()
      set(_curl_ssl_args
         -DCURL_USE_OPENSSL=ON
         -DCURL_USE_SCHANNEL=OFF
         -DOPENSSL_ROOT_DIR=${RMAP_BORINGSSL_PREFIX}
         -DOPENSSL_USE_STATIC_LIBS=TRUE
         -DOPENSSL_INCLUDE_DIR=${RMAP_BORINGSSL_PREFIX}/include
         -DOPENSSL_CRYPTO_LIBRARY=${RMAP_BORINGSSL_CRYPTO_LIB}
         -DOPENSSL_SSL_LIBRARY=${RMAP_BORINGSSL_SSL_LIB})
   endif()

   ExternalProject_Add(curl_build
      ${_curl_git_args}
      SOURCE_DIR "${RMAP_CURL_DIR}"
      BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}/curl/build"
      CMAKE_ARGS
         -DCMAKE_INSTALL_PREFIX=${RMAP_CURL_PREFIX}
         -DCMAKE_BUILD_TYPE=$<CONFIG>
         -DBUILD_SHARED_LIBS=OFF
         -DBUILD_CURL_EXE=OFF
         -DBUILD_TESTING=OFF
         -DCURL_DISABLE_LDAP=ON
         -DCURL_DISABLE_LDAPS=ON
         -DUSE_LIBIDN2=OFF
         -DCURL_USE_LIBPSL=OFF
         -DCURL_USE_LIBSSH2=OFF
         -DCURL_ZLIB=OFF
         -DCURL_BROTLI=OFF
         -DCURL_ZSTD=OFF
         ${_curl_ssl_args}
      BUILD_COMMAND   ${CMAKE_COMMAND} --build   <BINARY_DIR> --config $<CONFIG>
      INSTALL_COMMAND ${CMAKE_COMMAND} --install <BINARY_DIR> --config $<CONFIG>
      BUILD_BYPRODUCTS "${RMAP_CURL_LIB}")

   # curl links BoringSSL as its TLS backend off Windows; build BoringSSL first.
   if(NOT WIN32  AND  TARGET boringssl_build)
      add_dependencies(curl_build boringssl_build)
   endif()

   add_dependencies(RMAP curl_build)

   target_include_directories(RMAP PUBLIC
      $<BUILD_INTERFACE:${RMAP_CURL_PREFIX}/include>)

   # Every TU including <curl/curl.h> needs this for a static libcurl. Build-only
   # so it stays out of RMAP's exported interface.
   target_compile_definitions(RMAP PUBLIC $<BUILD_INTERFACE:CURL_STATICLIB>)

   target_link_libraries(RMAP PRIVATE "${RMAP_CURL_LIB}")
   if(WIN32)
      target_link_libraries(RMAP PRIVATE ws2_32 wldap32 crypt32 normaliz)
   endif()
endif()

# ---------------------------------------------------------------------------
# Fetched third-party headers are pulled at build time by ExternalProject and
# consumed transitively (via RMAP's include dirs) by the service OBJECT
# libraries, which compile BEFORE RMAP (their objects fold into it). Only RMAP
# depends on the fetch targets, so a clean build could compile a service before
# the clone finished. Make every enabled service depend on each active fetch (and
# the BoringSSL build) so the headers are present first.
# ---------------------------------------------------------------------------

foreach(_svc RMAP_Svc_SB RMAP_Svc_Rest RMAP_Svc_SocketIO)
   if(TARGET ${_svc})
      foreach(_dep nlohmann_json_fetch asio_fetch websocketpp_fetch boringssl_build curl_build)
         if(TARGET ${_dep})
            add_dependencies(${_svc} ${_dep})
         endif()
      endforeach()
   endif()
endforeach()

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
