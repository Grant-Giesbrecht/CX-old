# CX
A Tool for Compiling and Running C/C++ Programs via Comments

This command line program allows the user to simply type 'CX <program_name>' to compile and run a C or C++ program. The objective of this project is to allow C/C++ to become a competative alternative to Python or Octave/MATLAB when someone is searching for a quick and easy programming language. The second purpose is to make code more easily portable (especially for C/C++ rookies) by saving all compile/link/run information in computer interpreted comments, kind of like a makefile that can't be separated.

However, CX is not intended to replace utilities such as Make or CMake. It is not optimised for speed (which may impact million-or-so line programs), does not have the flexibility of Make or CMake, and is blind to which files have been modified and instead recompiles completely on each run (unless the user specifies otherwise).


For those who are wondering, it's called 'CX' because: 'C' as in C/C++ language, and 'X' because it's close to 'C' on a QWERTY keyboard and thus quick to type!

Happy hunting!
