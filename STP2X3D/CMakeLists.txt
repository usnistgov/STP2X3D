cmake_minimum_required(VERSION 3.12.2)

project(STP2X3D)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Default OCCT paths for this machine (override via env or -D)
if(NOT DEFINED ENV{CSF_OCCTIncludePath} OR "$ENV{CSF_OCCTIncludePath}" STREQUAL "")
  if(EXISTS "${CMAKE_SOURCE_DIR}/../OCCT-8_0_0/bin/inc")
    set(ENV{CSF_OCCTIncludePath} "${CMAKE_SOURCE_DIR}/../OCCT-8_0_0/bin/inc")
  elseif(EXISTS "C:/Users/USER/Desktop/OCCT-8_0_0/bin/inc")
    set(ENV{CSF_OCCTIncludePath} "C:/Users/USER/Desktop/OCCT-8_0_0/bin/inc")
  elseif(EXISTS "/usr/local/include/opencascade")
    set(ENV{CSF_OCCTIncludePath} "/usr/local/include/opencascade")
  endif()
endif()

if(NOT DEFINED ENV{CSF_OCCTLibPath} OR "$ENV{CSF_OCCTLibPath}" STREQUAL "")
  if(MSVC)
    if(EXISTS "C:/Users/USER/Desktop/OCCT-8_0_0/bin/win64/vc14/lib")
      set(ENV{CSF_OCCTLibPath} "C:/Users/USER/Desktop/OCCT-8_0_0/bin/win64/vc14/lib")
    endif()
  elseif(EXISTS "/usr/local/lib")
    set(ENV{CSF_OCCTLibPath} "/usr/local/lib")
  endif()
endif()

foreach (EnvVar IN ITEMS CSF_OCCTIncludePath CSF_OCCTLibPath)
  if ("$ENV{${EnvVar}}" STREQUAL "")
    message(WARNING "Environmental variable '${EnvVar}' is not defined.")
  else()
    message(STATUS "${EnvVar}=$ENV{${EnvVar}}")
  endif ()
endforeach ()

set(CMAKE_CONFIGURATION_TYPES "Debug;Release"
    CACHE STRING "Configuration types" FORCE)

add_executable(STP2X3D
  AP242XML_Reader.cpp
  AP242XML_Reader.h
  Component.cpp
  Component.h
  GDT_Item.cpp
  GDT_Item.h
  IShape.cpp
  IShape.h
  Mesh.cpp
  Mesh.h
  Model.cpp
  Model.h
  NumTool.h
  OCCLib.h
  OCCUtil.cpp
  OCCUtil.h
  S2X_Option.cpp
  S2X_Option.h
  ShapeType.h
  StatsPrinter.h
  stdafx.cpp
  stdafx.h
  STEP_Data.cpp
  STEP_Data.h
  STEP_Reader.cpp
  STEP_Reader.h
  StopWatch.cpp
  StopWatch.h
  STP2X3D.cpp
  StrTool.h
  Tessellator.cpp
  Tessellator.h
  X3D_Text.h
  X3D_Writer.cpp
  X3D_Writer.h
)

# OCCT 8 moved many typedef headers under Deprecated/NCollectionAliases
set(_occt_deprecated_aliases "")
foreach(_cand IN ITEMS
    "${CMAKE_SOURCE_DIR}/../OCCT-8_0_0/src/Deprecated/NCollectionAliases"
    "C:/Users/USER/Desktop/OCCT-8_0_0/src/Deprecated/NCollectionAliases")
  if(EXISTS "${_cand}")
    set(_occt_deprecated_aliases "${_cand}")
    break()
  endif()
endforeach()

target_include_directories(${PROJECT_NAME} PRIVATE
  "$ENV{CSF_OCCTIncludePath}"
  ${_occt_deprecated_aliases}
)

# Libraries used by STP2X3D (OCCT 7.8+ / 8.x toolkit names)
set(OCCT_LIBS
  TKBO
  TKBRep
  TKCAF
  TKCDF
  TKDE
  TKDESTEP
  TKernel
  TKG2d
  TKG3d
  TKGeomAlgo
  TKGeomBase
  TKHLR
  TKLCAF
  TKMath
  TKMesh
  TKPrim
  TKService
  TKShHealing
  TKTopAlgo
  TKV3d
  TKVCAF
  TKXCAF
  TKXSBase
)

if(MSVC)
  # Debug libs live in libd next to lib on Windows OCCT builds
  get_filename_component(_occt_lib_dir "$ENV{CSF_OCCTLibPath}" ABSOLUTE)
  get_filename_component(_occt_lib_parent "${_occt_lib_dir}" DIRECTORY)
  set(_occt_libd_dir "${_occt_lib_parent}/libd")
  target_link_directories(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:Debug>:${_occt_libd_dir}>
    $<$<CONFIG:Release>:${_occt_lib_dir}>
    $<$<CONFIG:RelWithDebInfo>:${_occt_lib_dir}>
    $<$<CONFIG:MinSizeRel>:${_occt_lib_dir}>
  )
  target_link_libraries(${PROJECT_NAME} PRIVATE ${OCCT_LIBS})
else()
  target_link_directories(${PROJECT_NAME} PRIVATE "$ENV{CSF_OCCTLibPath}")
  target_link_libraries(${PROJECT_NAME} PRIVATE ${OCCT_LIBS})
  # Older libstdc++ may need explicit filesystem linkage
  if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 9.0)
    target_link_libraries(${PROJECT_NAME} PRIVATE stdc++fs)
  endif()
endif()

# Preprocessor definitions
target_compile_definitions(STP2X3D PRIVATE
  $<$<CONFIG:Debug>:_UNICODE;_DEBUG;_CONSOLE>
  $<$<CONFIG:Release>:_UNICODE;NDEBUG;_CONSOLE>
)

if (MSVC)
  target_compile_options(STP2X3D PRIVATE /W3 /EHsc)
  # Precompiled header
  target_compile_options(STP2X3D PRIVATE /Yu"stdafx.h")
  set_source_files_properties(stdafx.cpp PROPERTIES COMPILE_FLAGS /Yc"stdafx.h")
  # Whole-program optimization for Release
  target_compile_options(STP2X3D PRIVATE $<$<CONFIG:Release>:/GL>)
  target_link_options(STP2X3D PRIVATE $<$<CONFIG:Release>:/LTCG>)
endif ()
