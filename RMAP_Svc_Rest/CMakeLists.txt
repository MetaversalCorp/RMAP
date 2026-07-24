# ---------------------------------------------------------------------------
# RMAP_Svc_Rest service module. Compiled as an OBJECT library so it appears as its
# own Visual Studio project; its objects are folded into RMAP.lib by the
# top-level CMakeLists ($<TARGET_OBJECTS:RMAP_Svc_Rest>). It reads the core's
# include dirs and compile definitions through $<TARGET_PROPERTY:RMAP,...>
# rather than linking, which would create a build cycle with the objects fold.
# ---------------------------------------------------------------------------

set(RMAP_Svc_Rest_FILES
   include/RMAP_Svc_Rest/RMAP_Svc_Rest.h
   src/RMAP_Svc_Rest.cpp)

add_library(RMAP_Svc_Rest OBJECT ${RMAP_Svc_Rest_FILES})

target_compile_features(RMAP_Svc_Rest PUBLIC cxx_std_17)

target_include_directories(RMAP_Svc_Rest
   PUBLIC
      $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
   PRIVATE
      $<TARGET_PROPERTY:RMAP,INTERFACE_INCLUDE_DIRECTORIES>)

target_compile_definitions(RMAP_Svc_Rest PRIVATE
   $<TARGET_PROPERTY:RMAP,INTERFACE_COMPILE_DEFINITIONS>)

if(MSVC)
   target_compile_definitions(RMAP_Svc_Rest PRIVATE UNICODE _UNICODE _LIB)
   target_compile_options(RMAP_Svc_Rest PRIVATE /permissive- /sdl /W3)
endif()

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${RMAP_Svc_Rest_FILES})
