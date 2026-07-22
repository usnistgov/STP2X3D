# STEP to X3D Translator (STP2X3D)

STP2X3D translates STEP (ISO 10303) Part 21 files (`.stp` / `.step` / `.p21`) and AP242 Domain Model XML (`.stpx`) to X3D (ISO/IEC 19776) (`.x3d`) or [X3DOM](https://www.x3dom.org/) (`.html`).

Originally developed at the [National Institute of Standards and Technology (NIST)](https://www.nist.gov/services-resources/software/step-x3d-translator), the software is based on the [Open CASCADE STEP Processor](https://dev.opencascade.org/doc/overview/html/occt_user_guides__step.html) and written in C++. Version 2.0 is maintained at Pusan National University.

## Prerequisites

- [Open CASCADE Technology](https://dev.opencascade.org/release) (OCCT) **8.0.0** or higher (recommended)
  - Rebuild OCCT for your toolchain; see the [OCCT build guide](https://dev.opencascade.org/doc/overview/html/build_upgrade__building_occt.html).
  - Select **vc14 or higher (x64)** on Windows.
  - Typical DLL set required at runtime (names may vary slightly by OCCT build options):
    - OCCT: `freetype.dll`, `jemalloc.dll` (if JeMalloc is enabled), `TKBO.dll`, `TKBRep.dll`, `TKCAF.dll`, `TKCDF.dll`, `TKDE.dll`, `TKDESTEP.dll`, `TKernel.dll`, `TKG2d.dll`, `TKG3d.dll`, `TKGeomAlgo.dll`, `TKGeomBase.dll`, `TKHLR.dll`, `TKLCAF.dll`, `TKMath.dll`, `TKMesh.dll`, `TKPrim.dll`, `TKService.dll`, `TKShHealing.dll`, `TKTopAlgo.dll`, `TKV3d.dll`, `TKVCAF.dll`, `TKXCAF.dll`, `TKXSBase.dll`
    - VC++ runtime: `msvcp140.dll`, `vcruntime140.dll` (and `vcruntime140_1.dll` for newer MSVC toolsets)

## Build

### Windows

- Use [CMake](https://cmake.org/) **3.18** or higher.
  - Generator: Visual Studio with **x64** (vc14 / MSVC 2015+ toolset).
- Set OCCT include/lib paths via environment variables, or edit the defaults in `STP2X3D/CMakeLists.txt`:
  - `CSF_OCCTIncludePath` — e.g. `...\OCCT-8_0_0\bin\inc`
  - `CSF_OCCTLibPath` — e.g. `...\OCCT-8_0_0\bin\win64\vc14\lib`
- Example from the repository root:

```
mkdir build
cd build
cmake ..
cmake --build . --config Release
```

### Linux

- Use CMake **3.12.2** or higher.
- Point `CSF_OCCTIncludePath` / `CSF_OCCTLibPath` (or the defaults in `STP2X3D/CMakeLists.txt`) at your OCCT install.
- From the repository root:

```
mkdir build
cd build
cmake ..
make
```

## Usage

STP2X3D is a command-line tool. See the [Usage guide](USAGE.md), or run the executable with no arguments to print options.

```
STP2X3D.exe --input Model.stp --edge 1 --quality 7
STP2X3D.exe --input Assembly.stpx --html 1
```

## Work In Progress

- Support for additional output formats (e.g. glTF)

## Contact

- Soonjo Kwon, [soonjo.kwon@pusan.ac.kr](mailto:soonjo.kwon@pusan.ac.kr) — Pusan National University
- William Z. Bernstein, [william.bernstein@afresearchlab.com](mailto:william.bernstein@afresearchlab.com)

## Used By

- [NIST STEP File Analyzer and Viewer](https://www.nist.gov/services-resources/software/step-file-analyzer-and-viewer)

## Reference

- R. R. Lipman, S. Kwon, 2021, [**STEP File Analyzer and Viewer User Guide (Update 7)**](https://nvlpubs.nist.gov/nistpubs/ams/NIST.AMS.200-12.pdf), *NIST Advanced Manufacturing Series*, 200-12.

## Version

2.0

## Disclaimers

This software originated at NIST. See the [NIST Disclaimer](https://www.nist.gov/disclaimer) and [LICENSE.md](LICENSE.md) for the original NIST terms of use.
