# Seashell (work ongoing)
## Introduction

Seashell is a linux command interpreter - GNU bash mimic.

## Program Description:

Take input from user and execute commands like: "ls -l -a | grep a | wc -l ; ls * ; echo 1"

## More Info:
* [ANSI Escape Sequences](http://ascii-table.com/ansi-escape-sequences.php) - Keycode management

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
Left, Right Arrows : Move into command line
Up, Down Arrows : Brows through commands history

CTRL^L : Clear the screen
CTRL^A : Move to the start of the line
CTRL^E : Move to the end of the line

Separators: ;
Globing: * ? [] {}
Multipipes
...

### Ongoing:
- Alias
- Redirections < > >>
- Separators && ||
- Autocompletion
- History whith file
- Builtins: cd, echo, exit, ...
- Job control

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details
