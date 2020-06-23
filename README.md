# NIST STEP to X3D Translator (STP2X3D)
NIST STEP to X3D Translator is an open-source software that translates a STEP (ISO 10303) Part 21 file to an X3D (ISO/IEC 19776) or X3DOM file.
The software is written in C++.
## Prerequisites
- [Open CASCADE Technology](https://www.opencascade.com/content/latest-release) (OCCT) 7.4.0 or higher (64 bit)
  - You must rebuild the OCCT solution (vc14) to regenerate DLL files.
  - The complete set of DLL files required for STP2X3D to run is as follows.
    - freetype.dll, TKBO.dll, TKBRep.dll, TKCAF.dll, TKCDF.dll, TKernel.dll, TKG2d.dll, TKG3d.dll, TKGeomAlgo.dll, TKGeomBase.dll, TKHLR.dll, TKLCAF.dll. TKMath.dll, TKMesh.dll, TKPrim.dll, TKService.dll, TKShHealing.dll, TKSTEP.dll, TKSTEP209.dll, TKSTEPAttr.dll, TKSTEPBase.dll, TKTopAlgo.dll, TKV3d.dll, TKVCAF.dll, TKXCAF.dll, TKXDESTEP.dll, TKXSBase.dll
  - Two environmental variables must be added as follows.
    - CSF_OCCTIncludePath: OCCTPath\inc
    - CSF_OCCTLibPath: OCCTPath\win64\vc14\lib
## Usage
- Running the execution file in the command prompt will give you the instruction.
## Build the STEP to X3D Translator
- Use [CMake](https://cmake.org/) to build the software.
  - vc14 must be selected for the generator.
- You can also create a new project, include source files, and build the project in Visual Studio 2015.
  - Add $(CSF_OCCTIncludePath) to Additional Include Directories.
  - Add $(CSF_OCCTLibPath) to Additional Library Directories.
## Contact Information
- Soonjo Kwon, soonjo.kwon@nist.gov, soonjo.kwon.1@gmail.com
- William Z. Bernstein, william.bernstein@nist.gov
## Credits
- The translator is based on the [Open CASCADE STEP Processor](https://dev.opencascade.org/doc/overview/html/occt_user_guides__step.html).
- Developed by Soonjo Kwon at NIST in 2020.
## Used by
- [NIST STEP File Analyzer and Viewer](https://www.nist.gov/services-resources/software/step-file-analyzer-and-viewer)
## Version
1.0
## Disclaimers
[NIST Disclaimer](https://www.nist.gov/disclaimer)
