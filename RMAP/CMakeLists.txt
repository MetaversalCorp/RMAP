# ---------------------------------------------------------------------------
# RMAP core module. This is the one STATIC target and therefore *is* the final
# deliverable, RMAP.lib. Every service module's compiled objects are folded
# into this archive by the top-level CMakeLists (see $<TARGET_OBJECTS>).
#
# Headers are listed alongside sources so they appear in the generated Visual
# Studio project and its filters.
# ---------------------------------------------------------------------------

set(RMAP_FILES
   include/RMAP/App.h
   include/RMAP/Base.h
   include/RMAP/Mem.h
   include/RMAP/RMAP.h
   include/RMAP/Session.h
   include/RMAP/Utils.h
   src/base/Client.cpp
   src/base/Model.cpp
   src/base/Model_Object.cpp
   src/base/Model_Session.cpp
   src/base/Notification.cpp
   src/base/Service.cpp
   src/base/Source.cpp
   src/base/Source_Session.cpp
   src/core/Core.cpp
   src/core/ModelClass.cpp
   src/core/Namespace.cpp
   src/core/ServiceClass.cpp
   src/core/SourceClass.cpp
   src/mem/Mem.cpp
   src/mem/Mem_Model.cpp
   src/mem/Mem_Source.cpp
   src/mem/Objectbank.cpp
   src/mem/Objectbank_Dep.cpp
   src/mem/Objectbank_Ind.cpp
   src/model/Model_Session_C2a.cpp
   src/model/Model_Session_Null.cpp
   src/model/Model_Session_UiP.cpp
   src/Collection.h
   src/Library.cpp
   src/LibraryRMAP.cpp
   src/LibraryRMAP.h
   src/LnG.cpp
   src/Logger.cpp
   src/Package.cpp
   src/Plugin.cpp
   src/Registry.cpp
   src/SharedObject.h
   src/Utils.cpp
   src/Version.h
   src/pch.cpp
   src/pch.h)

add_library(RMAP STATIC ${RMAP_FILES})
add_library(RMAP::RMAP ALIAS RMAP)

target_compile_features(RMAP PUBLIC cxx_std_17)

# Static build: RMAP.h's export macro expands to nothing.
target_compile_definitions(RMAP PUBLIC RMAP_CORE_API=)

target_include_directories(RMAP
   PUBLIC
      $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
      $<INSTALL_INTERFACE:include>
   PRIVATE
      ${CMAKE_CURRENT_SOURCE_DIR}/include/RMAP
      ${CMAKE_CURRENT_SOURCE_DIR}/src)

# MSVC parity with the previous hand-authored project settings.
if(MSVC)
   target_compile_definitions(RMAP PRIVATE UNICODE _UNICODE _LIB)
   target_compile_options(RMAP PRIVATE /permissive- /sdl /W3)
   set_target_properties(RMAP PROPERTIES
      INTERPROCEDURAL_OPTIMIZATION_RELEASE ON)
endif()

target_precompile_headers(RMAP PRIVATE src/pch.h)

# Visual Studio filters mirroring the on-disk folder layout
# (src\base, src\core, src\mem, src\model, include\RMAP).
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${RMAP_FILES})
