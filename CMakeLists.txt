# =============================================================================
# modul: assoc-ogre-next  —  Ogre-Next 2.3 (Hlms, Item, Compositor2), önálló app
# =============================================================================
#
# Cél: tiszta 2.3 vonal — Hlms (Unlit+PBS), mesh → Item, Compositor2 alap munkatér,
#      irányfény, v1 Overlay a HUD-ra (a 2.x-ben hivatalosan támogatott szövegút; a
#      meshre quadozott felirat más, nehezebb út lenne).
#
# Fő adatfolyam: `cmake` → `build/assoc_config.h` + `build/plugins.cfg` (útvonalak) →
#      egy tárgy, `libOgre*` link + Apple framework, futtatás a `build/`-ból.
#
# Környezet / bemenet: telepített Ogre-Next SDK (dylib, include, Media) és a
#      `Samples/Media` (Hlms sablonok) — vagy a CMake `OGRE_NEXT_*` cache útvonallal
#      felülírva.
# =============================================================================

cmake_minimum_required(VERSION 3.16)
project(assoc-ogre-next LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# SDK prefix (cmake --install) és a Hlms forrásmédia; alap: fejlesztői gépen buildelt fa
set(OGRE_NEXT_SDK "/Users/tomi/ogre-next-sdk" CACHE PATH "Ogre-Next telepítési prefix (cmake --install)")
set(OGRE_NEXT_MEDIA "/Users/tomi/ogre-next-src/Samples/Media" CACHE PATH "OGRE Next: Hlms + minta média (Samples/Media)")

# csak fejlécteszt: ne a teljes `configure_file` bontsa el sokáig a munkát, ha a lib hiányzik
if(NOT EXISTS "${OGRE_NEXT_SDK}/lib/libOgreMain.dylib")
  message(FATAL_ERROR "Nem találom az Ogre-Next SDK-t: ${OGRE_NEXT_SDK}/lib/libOgreMain.dylib")
endif()
if(NOT EXISTS "${OGRE_NEXT_MEDIA}/Hlms")
  message(FATAL_ERROR "Nem találom a Hlms sablonokat: ${OGRE_NEXT_MEDIA}/Hlms")
endif()

configure_file(
  "${CMAKE_SOURCE_DIR}/cfg/assoc_config.h.in"
  "${CMAKE_BINARY_DIR}/assoc_config.h" @ONLY)
configure_file(
  "${CMAKE_SOURCE_DIR}/cfg/plugins.cfg.in"
  "${CMAKE_BINARY_DIR}/plugins.cfg" @ONLY)

# generált `assoc_config.h` + ogrés include: a build dir első, hogy a saját fejlécek nyerjenek
include_directories("${CMAKE_BINARY_DIR}")
include_directories("${OGRE_NEXT_SDK}/include/OGRE")
# a Hlms implementációk „OgreHlmsBufferManager.h” néven a Common/ mappából includolnak, nem a gyökérből
include_directories("${OGRE_NEXT_SDK}/include/OGRE/Hlms/Common")
# a v1 Overlay (HUD) fejlécek az `Overlay/` almappában vannak, nem az OGRE/ gyökérben
include_directories("${OGRE_NEXT_SDK}/include/OGRE/Overlay")

add_executable(assoc-next
  src/AssocNextApp.cpp)

# futásidejű: Main + két Hlms + Overlay; a render pluginok a plugins.cfg + dlopen útvonalon jönnek
target_link_libraries(assoc-next PRIVATE
  "${OGRE_NEXT_SDK}/lib/libOgreMain.dylib"
  "${OGRE_NEXT_SDK}/lib/libOgreHlmsPbs.dylib"
  "${OGRE_NEXT_SDK}/lib/libOgreHlmsUnlit.dylib"
  "${OGRE_NEXT_SDK}/lib/libOgreOverlay.dylib"
)

if(APPLE)
  # ablak, Metal, logika (SDL nélküli, beépített ablak) — a plugin dylib-ekre rpath, hogy a render rendszerek megtalálják a libOgreMain-t
  target_link_libraries(assoc-next PRIVATE
    "-framework Cocoa"
    "-framework Metal"
    "-framework MetalKit"
    "-framework QuartzCore"
    "-framework Foundation"
  )
  set_target_properties(assoc-next PROPERTIES
    LINK_FLAGS "-Wl,-rpath,${OGRE_NEXT_SDK}/lib"
  )
endif()
