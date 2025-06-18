# todo

## Brief

It is a command line tool like todo app.

## Usage

- compile

```console
$ gcc -o build build.c
$ ./build -c
```

- use

```
$ ./todo -h
$ ./todo -l
$ ./todo -n t1 -a "fix the bug"
$ ./todo -n t1 -u WORKING
$ ./todo -n t1 -d
```

## Note

`todo` will store the "todo content" into a file, which is specified in `todo.h` called `TODO_FILE_DATA`
