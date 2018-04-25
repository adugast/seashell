# Seashell (work ongoing)
## Introduction

Seashell is a UNIX shell written in C.

## Program Description:

Seashell can read and execute user commands like:</br>
"/bin/ls"</br>
"ls"</br>
"ls -l -a"</br>
"ls -l -a | grep main"</br>
"ls -l -a | grep a | wc -l ; ls * ; echo 1"</br>
...

## Usage:

```
./seashell
```

## Output:

```
Cli>ls -l -a | grep a | wc -l
11
Cli>
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

## More Info:
* [ANSI Escape Sequences](http://ascii-table.com/ansi-escape-sequences.php) - Keycode management

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details
