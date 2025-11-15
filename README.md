# Quine-McCluskey Minimizer (CLion / CMake)

This repository contains a compact, modular C++ implementation of the
Quine–McCluskey logic minimization algorithm intended for the CSCE2301
Project 1.

Build (CLion or command line):
- In CLion: Open the folder, let CMake configure, build & run `quine`.
- Command line:
  mkdir build
  cd build
  cmake ..
  cmake --build .

Run:
- The program takes a single input file path:
  ./quine path/to/testfile.txt

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

Notes:
- Sherifa Badra (@sherifabadra) - Parsing, I/O, and testing
- Abdullah Ahmed (@abdullah-ax) - Algorithm implementation
- There is a test-suite of 10 inputs under `tests/`.

Limitations:
- Brute-force minimization has exponential worst-case behavior. For large numbers
  of PIs, the cover search is limited to subsets up to size 6 to keep runtimes reasonable.
