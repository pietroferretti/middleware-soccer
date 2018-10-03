# middleware-soccer
Computing ball possession statistics in a game of soccer with MPI.
Code developed as part of the project for the Middleware Technologies for Distributed Systems course at polimi.

## Requirements
This program needs CMake to compile and an MPI library to run.

You'll also need the "full-game" dataset and the "Game Interruption" csv files that were provided with the project assignment.

## Building
1. Run `cmake CMakeLists.txt` to create the Makefile.

2. Run `make soccer` to build the executable

## Running the program

```
Usage: ./soccer [-t <interval>] [-k <distance>] [-e <path to full-game>]
          [-1 <path to interruptions (1st half)>] [-2 <path to interruptions (2nd half)>]
```

The program must be run with `mpirun`, with at least 3 processes.

Example:
`mpirun -n 4 ./soccer -e datasets/full-game -1 datasets/referee-events/Game\ Interruption/1st\ Half.csv -2 datasets/referee-events/Game\ Interruption/2nd\ Half.csv -t 60 -k 5`
