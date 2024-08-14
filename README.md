# NIST STEP to X3D Translator (STP2X3D)
The NIST [STEP to X3D Translator](https://www.nist.gov/services-resources/software/step-x3d-translator) is an open-source software that translates a STEP (ISO 10303) Part 21 file (.stp or .step) to an X3D (ISO/IEC 19776) file (.x3d) or [X3DOM](https://www.x3dom.org/) file (.html). Developed at the National Institute of Standards and Technology (NIST), the software is based on the [Open CASCADE STEP Processor](https://dev.opencascade.org/doc/overview/html/occt_user_guides__step.html) and written in C++.  

## Prerequisites
- [Open CASCADE Technology](https://www.opencascade.com/content/latest-release) (OCCT) 7.8.0 or higher
  - You must [rebuild the OCCT solution](https://dev.opencascade.org/doc/overview/html/build_upgrade__building_occt.html) to regenerate DLL files.
  - For Linux users, you can easily install OpenCASCADE running the following command at the project's root directory:
       ```sh
       bash install_opencascade.sh
       ```
    - vc14 or higher (x64) should be selected.
  - The complete set of DLL files required for STP2X3D to run is as follows.
    - OCCT related: freetype.dll, jemalloc.dll, TKBO.dll, TKBRep.dll, TKCAF.dll, TKCDF.dll, TKDE.dll, TKDESTEP.dll, TKernel.dll, TKG2d.dll, TKG3d.dll, TKGeomAlgo.dll, TKGeomBase.dll, TKHLR.dll, TKLCAF.dll. TKMath.dll, TKMesh.dll, TKPrim.dll, TKService.dll, TKShHealing.dll, TKTopAlgo.dll, TKV3d.dll, TKVCAF.dll, TKXCAF.dll, TKXSBase.dll
	  - The list of DLL files has been updated since OCCT 7.8.0 especially STEP-related DLL files.
	  - [jemalloc.dll should be built](https://jemalloc.net/) and included if the memory manager is set to JeMalloc.
    - VC++ related: msvcp140.dll, vcruntime140.dll (vcruntime140_1.dll will also be required when vc141 or higher was used to build.)
    
## Build the STEP to X3D Translator 

  ### Windows
  - Use [CMake](https://cmake.org/) 3.18.4 or higher to build the software.
    - vc14 or higher (x64) should be selected for the generator.
  - You can also create a new project, include source files, and build the project in Visual Studio.
    - Be sure to use Visual Studio 2015 or 2017 due to [std::experimental::filesystem](https://docs.microsoft.com/en-us/cpp/standard-library/filesystem?view=msvc-150). This should be updated later.
  - Additional Directories should be added as follows.
    - Add "OCCTPath\inc" to Additional Include Directories.
    - Add "OCCTPath\win64\vc14\lib" to Additional Library Directories.
  
  ### Linux (tested Ubuntu 20.10 64bit)
  - Use [CMake](https://cmake.org/) 3.12.2 or higher to build the software.
  - Modify lines 5-6 of ``STP2X3D/CMakeLists.txt`` to properly set link to OCCT on your local machine.
  - Run the following commands from the repository root directory (where this README is located):
  ```
  mkdir build
  cd build  
  cmake ..
  make
  ```
  
## Usage
- NIST STP2X3D is a command line software. Please check out the [Usage guide](USAGE.md).

## Work In Progress
- Support of AP242 Domain Model XML format as input

## Contact Information
- Soonjo Kwon, soonjo.kwon@kumoh.ac.kr
- William Z. Bernstein, william.bernstein@afresearchlab.com

## Used By
- [NIST STEP File Analyzer and Viewer](https://www.nist.gov/services-resources/software/step-file-analyzer-and-viewer)

## Reference
- R. R. Lipman, S. Kwon, 2021, [**STEP File Analyzer and Viewer User Guide (Update 7)**](https://nvlpubs.nist.gov/nistpubs/ams/NIST.AMS.200-12.pdf), *NIST Advanced Manufacturing Series*, 200-12.

## Version
1.50

## Disclaimers
[NIST Disclaimer](https://www.nist.gov/disclaimer)
