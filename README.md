# Seashell   [![Language: C](https://img.shields.io/badge/Language-C-brightgreen.svg)](https://en.wikipedia.org/wiki/C_(programming_language))  [![Library: ncurses](https://img.shields.io/badge/Library-ncurses-brightgreen.svg)](https://www.gnu.org/software/ncurses/)  [![Builder: CMake](https://img.shields.io/badge/Builder-CMake-brightgreen.svg)](https://cmake.org/)  [![License: MIT](https://img.shields.io/badge/License-MIT-brightgreen.svg)](https://opensource.org/licenses/MIT)

## Introduction

Seashell is a UNIX shell written in C.

## Program description:

- Seashell can read and execute simple user commands:
```
Cli>/bin/ls
```
```
Cli>ls
```
- Also, it can interpret a command with arguments:
```
Cli>ls -l -a
```
- The commands can be combined in a pipe line (forward output to the next command):
```
Cli>ls -l -a | grep main
```
- And can handle pipe line, globing and separator:
```
Cli>ls -l -a | grep a | wc -l ; ls * ; echo 1
```
...

## Setting up seashell

Clone the project to retrieve the sources:
```
$>git clone https://github.com/adugast/seashell.git
```

Go in the build directory of seashell:
```
$>cd seashell/build/
```

Build the project using CMake (from the build directory):
```
$>cmake ..
```

Finally, use make to compile the sources and so generate the binary (still from the build directory):
```
$>make
```

## Usage:
Launch seashell:
```
$>./seashell
```

## Output:

```
Cli>ls -l -a | grep a | wc -l
11
Cli>nproc
8
Cli>uname -r
3.10.0-327.10.1.el7.x86_64
Cli>ps
  PID TTY          TIME CMD
 8445 pts/56   00:00:00 bash
11615 pts/56   00:00:00 seashell
11616 pts/56   00:00:00 ps
Cli>
Cli>exit
```

## Help:

```
Usage: ./seashell [params]
Available params:
 -h, --help     : Display this help and exit
 -v, --version  : Show version information
 -f, --features : Show seashell features
```

## Supported features:

### Done
Cursor motion:
- Left, Right Arrows : Move into command line
- Up, Down Arrows : Brows through commands history

Control keys:
- CTRL^L : Clear the screen
- CTRL^A : Move to the start of the line
- CTRL^E : Move to the end of the line

Command parsing:
- Separators: ;
- Globing: * ? [] {}
- Multipipes
- ...

### Ongoing:
- Alias
- Redirections: < > >>
- Separators: && ||
- Autocompletion
- History whith file
- Builtins: cd, echo, exit, ...
- Job control

## How seashell works:

```
+----------------------------+
|Processing of the user input|
+----------------------------+

+----------+
|user input|        -IN- user keyboard
+----+-----+
     |              A read function call is used to get input from STDIN
     |              and generates a buffer with RAW data.
     v              (contains printable and special characters)
 +---+--+
 |buffer|           (main interpret function)
 +---+--+
     |              Special characters are directly proceed to handle terminal
     |              management (i.e arrow keys, ctrl keys, delete, backspace, etc ...)
     |              and a new buffer line with only printable characters is reconstructed
     v    <-^
  +--+-+  | |
  |line|  | |       (main loop - read keyboard function)
  +--+-+  | |
     |    v->       After pressing "enter", the buffer line goes to the execution module
     |              and becomes the command line to execute.
     v              (i.e "ls -l -a | grep a | wc -l ; ls * ; echo 42")
 +---+----+
 |cmd line|         (execution module)
 +--------+
                    Then the command line is parsed by the parser module for each
                    ";", "|" and " " characters and executed.

";" commands separator
"|" split the commands to be executed in a pipeline (output of the executed command is transferred as input for the following command)
" " split the command with their own arguments
```

## More info:

* [Terminfo and Termcap](https://www.tldp.org/HOWTO/Text-Terminal-HOWTO-16.html) - Introduction to terminal capabilities and database
* [man (5) terminfo](https://www.freebsd.org/cgi/man.cgi?query=terminfo&sektion=5) - Terminal capability database
* [ANSI Escape Sequences](http://ascii-table.com/ansi-escape-sequences.php) - Keycode management
* [ASCII Flow](http://asciiflow.com/) - Simple web based ascii flow diagram drawing tool

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details
