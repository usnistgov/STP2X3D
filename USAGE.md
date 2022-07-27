# Usage Guide
The following information describes how to use the software. Running the execution file in the command line prompt will also give you the instruction.

## How to use in the command line prompt
- STP2X3D.exe option1 value1 option2 value2 ..

## Options
- --input: Input STEP file path
- --normal: Normal vector (1:yes, 0:no) default=0
- --color: Color (1:yes, 0:no) default=1
- --edge: Boundary edges (1:yes, 0:no) default=0
- --sketch: Sketch geometry (1:yes, 0:no) default=1
- --html: Output file type (1:html, 0:x3d) default=0
- --quality    Mesh quality (1-low to 10-high) default=5
- --gdt        Geometric elements related to GD&T (1:yes, 0:no) default=0
- --tess       Adaptive tessellation per each body (1:yes, 0:no) default=0
- --rosette    Rosette used for Composite Design (1:yes, 0:no) default=0
- --batch      Processing multiple STEP files (1:include sub-directories, 0:current dir). Followed by a folder path (e.g. --batch 0 c:\\)

## Examples
- STP2X3D.exe --input Model.stp --edge 1 --quality 7
- STP2X3D.exe --html 1 --sketch 0 --input Model.step
- STP2X3D.exe --color 0 --batch 1 C:\Folder --normal 1
