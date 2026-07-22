# Usage Guide

STP2X3D is a command-line translator. Running the executable with no arguments prints the same option summary.

## Command line

```
STP2X3D.exe option1 value1 option2 value2 ...
```

## Options

| Option | Description | Default |
|--------|-------------|---------|
| `--input` | Input STEP or AP242 XML path (`.stp` / `.step` / `.p21` / `.stpx`) | (required unless `--batch`) |
| `--output` | Output X3D/HTML path (input base name is used if omitted) | empty |
| `--normal` | Export normal vectors (`1` yes, `0` no) | `0` |
| `--color` | Export colors (`1` yes, `0` no) | `1` |
| `--edge` | Export boundary edges (`1` yes, `0` no) | `0` |
| `--sketch` | Export sketch geometry (`1` yes, `0` no) | `1` |
| `--html` | Output type (`1` HTML/X3DOM, `0` X3D) | `0` |
| `--quality` | Mesh quality (`1` low … `10` high) | `5` |
| `--gdt` | Export GD&T-related geometry (`1` yes, `0` no) | `0` |
| `--sfa` | Print SFA-oriented stats (shape count, bbox, sketch) (`1` yes, `0` no) | `1` |
| `--tess` | Adaptive tessellation per body (`1` yes, `0` no) | `0` |
| `--rosette` | Composite Design rosette geometry (`1` yes, `0` no) | `0` |
| `--cap` | Section cap geometries (`1` yes, `0` no) | `0` |
| `--tsolid` | Tessellated solids (`1` yes, `0` no) | `1` |
| `--batch` | Process multiple files in a folder (`1` include subdirs, `0` current dir only). Followed by a folder path | — |

`--input` and `--batch` cannot be used together.

## Examples

```
STP2X3D.exe --input Model.stp --edge 1 --quality 7
STP2X3D.exe --html 1 --sketch 0 --input Model.step
STP2X3D.exe --input Assembly.stpx --edge 1
STP2X3D.exe --color 0 --batch 1 C:\Folder --normal 1
STP2X3D.exe --input Model.stp --output Out.html --html 1 --sfa 1
```

## Notes

- On Windows, non-ASCII file names are supported (wide-character entry point).
- Invalid option values (non-numeric tokens, out-of-range quality, etc.) cause the program to exit with an error instead of crashing.
- Requires OCCT **8.0.0** (or compatible) runtime DLLs on `PATH` or next to the executable.

## Contact

- Soonjo Kwon, [soonjo.kwon@pusan.ac.kr](mailto:soonjo.kwon@pusan.ac.kr)
