# maze_c_exercise

## Maze exercise rules:

Implement an algorithm to find a route through a maze given in a .txt file, or tell if there is no possible route available.

In the map file a
‘#’ represents a block.
‘ ‘ (whitespace) represents movable space.
‘E’ means an exit.
‘^’ is the starting position.

It is possible to move in 4 directions, and each move has an equal cost.
Each map should be tried with the same algorithm with 20, 150 and 200 maximum moves.
Going outside the maze area is not allowed.

The solution with the correct route must be printed to something visible, either to a file,
on the command line, or graphically. The program should accept an input file given by the user.

## Solution:

### Implementation
Current version implements a not so fancy left-hand-rule to solve the maze.
It tries to get out of the maze and prints the map of the try.
If it finds an exit, the operation is terminated immediately.

The program checks the user input text-file for various errors e.g. no start, no exit, undefined shape.

### System info
Tested with Windows Power Shell on Windows 11 Enterprise

Compiled with:
```bash
gcc main.c -o maze
```
or for more information
```bash
gcc -DDEBUG main.c -o maze
```