cmake_minimum_required(VERSION 3.12.2)

project(STP2X3D)

set (ENV{CSF_OCCTIncludePath} /usr/local/include/opencascade)
set (ENV{CSF_OCCTLibPath} /usr/local/lib)

foreach (EnvVar IN ITEMS CSF_OCCTIncludePath CSF_OCCTLibPath)
  if ("$ENV{${EnvVar}}" STREQUAL "")
    message(WARNING, "Environmental variable '${EnvVar}' is not defined.")
  endif ()
endforeach ()
set(CMAKE_CONFIGURATION_TYPES "Debug;Release"
    CACHE STRING "Configuration types" FORCE)

add_executable(STP2X3D
  Component.cpp
  Component.h
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
  X3D_Writer.cpp
  X3D_Writer.h
)
# Additional include directories
set_property(TARGET STP2X3D
  APPEND PROPERTY INCLUDE_DIRECTORIES
  $<$<CONFIG:Debug>:
    $ENV{CSF_OCCTIncludePath}>
  $<$<CONFIG:Release>:
    $ENV{CSF_OCCTIncludePath}>
)

target_include_directories(${PROJECT_NAME} PRIVATE $ENV{CSF_OCCTIncludePath})
target_link_libraries(${PROJECT_NAME} stdc++fs
    TKBinL
    TKBin
    TKBinTObj
    TKBinXCAF
    TKBool
    TKBO
    TKBRep
    TKCAF
    TKCDF
    TKernel
    TKFeat
    TKFillet
    TKG2d
    TKG3d
    TKGeomAlgo
    TKGeomBase
    TKHLR
    TKIGES
    TKLCAF
    TKMath
    TKMesh
    TKMeshVS
    TKOffset
    TKOpenGl
    TKPrim
    TKRWMesh
    TKService
    TKShHealing
    TKStdL
    TKStd
    TKSTEP209
    TKSTEPAttr
    TKSTEPBase
    TKSTEP
    TKSTL
    TKTObj
    TKTopAlgo
    TKV3d
    TKVCAF
    TKVRML
    TKXCAF
    TKXDEIGES
    TKXDESTEP
    TKXMesh
    TKXmlL
    TKXml
    TKXmlTObj
    TKXmlXCAF
    TKXSBase )

# Preprocessor definitions
target_compile_definitions(STP2X3D PRIVATE
  $<$<CONFIG:Debug>:_UNICODE;_DEBUG;_CONSOLE>
  $<$<CONFIG:Release>:_UNICODE;NDEBUG;_CONSOLE>
)

# SDL check
target_compile_options(STP2X3D PRIVATE
  "$<$<CONFIG:Debug>:/sdl>"
  "$<$<CONFIG:Release>:/sdl>"
)

# Minimal rebuild
if (MSVC)
  target_compile_options(STP2X3D PRIVATE
    "$<$<CONFIG:Debug>:/Gm>"
    "$<$<CONFIG:Release>:/Gm->"
  )
endif ()

# Precompiled header files
if (MSVC)
  target_compile_options(STP2X3D PRIVATE
    "$<$<CONFIG:Debug>:/Yu>"
    "$<$<CONFIG:Release>:/Yu>"
  )
  set_property(SOURCE stdafx.cpp
    APPEND_STRING PROPERTY COMPILE_FLAGS
    "$<$<CONFIG:Debug>:/Yc> \
     $<$<CONFIG:Release>:/Yc>")
endif ()

# Additional library directories
if (MSVC)
  target_link_options(STP2X3D PRIVATE
    $<$<CONFIG:Debug>:
      /LIBPATH:$ENV{CSF_OCCTLibPath}>
    $<$<CONFIG:Release>:
      /LIBPATH:$ENV{CSF_OCCTLibPath}>
  )
else ()
  target_link_options(STP2X3D PRIVATE
    $<$<CONFIG:Debug>:
      -L$ENV{CSF_OCCTLibPath}>
    $<$<CONFIG:Release>:
      -L$ENV{CSF_OCCTLibPath}>
  )
endif ()
