# Quine-McCluskey Minimizer (CLion / CMake)

This repository contains a compact, modular C++ implementation of the
Quine–McCluskey logic minimization algorithm intended for the CSCE2301
Project 1.

Build (CLion or command line):
- In CLion: Open the folder, let CMake configure, build & run `quine`.
- Command line:
  ```
  mkdir build
  cd build
  cmake ..
  cmake --build .
  ```

Run:
- The program takes a single input file path:
  ```
  ./quine path/to/testfile.txt
  ```
- Or run all tests interactively:
  ```
  ./quine
  ```

Input file format (3 lines):
1) number of variables (integer)
2) minterms (m1,m3,...) OR maxterms (M0,M2,...) separated by commas (to load the file, ensure minterms don't exceed the range of (2^n) - 1 where n is the number of variables)
3) don't-cares (d0,d5,...). If none, leave line empty or just leave 'd'. (out of range don't cares are irrelevant and don't effect the minimizer in any way)

Examples are in the `tests/` folder.

What this program does:
- Parses and validates input
- Prints binary representations of minterms and don't-cares
- Generates and prints all Prime Implicants (PIs)
- Finds Essential Prime Implicants (EPIs) and prints them as Boolean expressions
- Prints minterms not covered by EPIs
- Finds minimal covers (brute-force search with limits) and prints all minimal solutions
- **[BONUS]** Generates Verilog modules using primitive gates (and, or, not)

Verilog Generation (Bonus Feature):
- After displaying minimization results, optionally generate synthesizable Verilog HDL
- Uses only primitive gates: `and`, `or`, `not` for maximum compatibility
- Supports multi-input gates and automatic wire optimization
- Auto-generates module names from test filenames
- Option to save output to `.v` files
- Optional testbench generation for verification

Example Verilog output:
```verilog
module test1 (
    input A, B, C,
    output F
);
    wire A_n, B_n, C_n;
    wire p0, p1;
    
    not g0 (A_n, A);
    not g1 (B_n, B);
    not g2 (C_n, C);
    
    and g3 (p0, A_n, B_n, C);
    and g4 (p1, B_n, C_n);
    
    or g5 (F, p0, p1);
endmodule
```

Simulation of generated Verilog:
```
iverilog -o sim module.v module_tb.v
vvp sim
```

Notes:
- Sherifa Badra (@sherifabadra) - Parsing, I/O, and testing
- Abdullah Ahmed (@abdullah-ax) - Algorithm implementation & Verilog generation
- There is a test-suite of 10 inputs under `tests/`.

Limitations:
- Brute-force minimization has exponential worst-case behavior. For large numbers
  of PIs, the cover search is limited to subsets up to size 6 to keep runtimes reasonable.
