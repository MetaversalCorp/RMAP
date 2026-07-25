# ---------------------------------------------------------------------------
# RMAP_Svc_SB service module. Compiled as an OBJECT library so it appears as its
# own Visual Studio project; its objects are folded into RMAP.lib by the
# top-level CMakeLists ($<TARGET_OBJECTS:RMAP_Svc_SB>). It reads the core's
# include dirs and compile definitions through $<TARGET_PROPERTY:RMAP,...>
# rather than linking, which would create a build cycle with the objects fold.
# ---------------------------------------------------------------------------

set(RMAP_Svc_SB_FILES
   include/RMAP_Svc_SB/ClassTypes.h
   include/RMAP_Svc_SB/Client.h
   include/RMAP_Svc_SB/RMAP_Svc_SB.h
   include/RMAP_Svc_SB/Source.h
   src/base/Source_SB_Object.cpp
   src/base/Source_SB_Session.cpp
   src/client/Client.cpp
   src/client/Control.cpp
   src/client/Net.cpp
   src/client/Recover.cpp
   src/client/Refresh.cpp
   src/client/Subscription.cpp
   src/model/Model_SBTime.cpp
   src/source/Source_SB_SBTime.cpp
   src/source/Source_SB_Session_Null.cpp
   src/ByteStream.cpp
   src/Library.cpp
   src/Library.h
   src/Map.cpp
   src/Service.cpp
   src/Time.cpp
   src/pch.cpp
   src/pch.h)

add_library(RMAP_Svc_SB OBJECT ${RMAP_Svc_SB_FILES})

target_compile_features(RMAP_Svc_SB PUBLIC cxx_std_17)

target_include_directories(RMAP_Svc_SB
   PUBLIC
      $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
   PRIVATE
      ${CMAKE_CURRENT_SOURCE_DIR}/include/RMAP_Svc_SB
      ${CMAKE_CURRENT_SOURCE_DIR}/src
      $<TARGET_PROPERTY:RMAP,INTERFACE_INCLUDE_DIRECTORIES>)

target_compile_definitions(RMAP_Svc_SB PRIVATE
   $<TARGET_PROPERTY:RMAP,INTERFACE_COMPILE_DEFINITIONS>)

if(MSVC)
   target_compile_definitions(RMAP_Svc_SB PRIVATE UNICODE _UNICODE _LIB)
   # NOCRYPT stops <windows.h> (pulled via asio) from including <wincrypt.h>, whose
   # X509_NAME/PKCS7/OCSP_* macros collide with BoringSSL's same-named types. SB uses
   # BoringSSL, not the Windows crypto APIs, so suppressing wincrypt is safe.
   target_compile_definitions(RMAP_Svc_SB PRIVATE WIN32_LEAN_AND_MEAN NOCRYPT)
   target_compile_options(RMAP_Svc_SB PRIVATE /permissive- /sdl /W3)
endif()

target_precompile_headers(RMAP_Svc_SB PRIVATE src/pch.h)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${RMAP_Svc_SB_FILES})
