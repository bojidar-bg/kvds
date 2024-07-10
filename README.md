# KVDS

A simple in-memory "database" that is accessed over standard input/output. The database stores a set of integers and associated character data and allows querying and updating them in various ways.

## Why?

KVDS is a sample project showcasing the use of various data structures; in particular binary search trees and doubly-linked lists. Hence the name: **K**ey-**V**alue store built on **D**ata **S**tructures.

## Compiling

To compile this project you will need [Tup](https://gittup.org/tup/index.html) and [GCC](https://gcc.gnu.org)—or alternatively, you may bring your own command-line-compatible C compiler.

After installing the dependencies, just run the following line to build the project:

```bash
tup
```

To use another compiler, change the tup.config file in `bin/`.

After compiling, you can find the resulting binary in `bin/kvds`.

## Accessing the database

Upon starting the executable, you are (will be) greeted with a interactive prompt, asking for input. Commands are entered one-by-one on separate lines; first word is the command, the rest is the data.

Commands:

| Command | Alias | Arguments | Description |
| --- | --- | --- | --- |
| select | s | key: integer | Moves the cursor to a given key in the database. |
| key | k | | Prints the current cursor location. |
| exists | e | | Prints whether the selected key exists. |
| read | r | | Prints the data at the selected key. |
| write | w | data: ... | Stores data at the selected key. Passing no data will still create the key. |
| delete | d | | Deletes the selected key along with any data. |
| prev | p, < | | Moves to the previous existing key (smaller than the cursor) |
| next | n, > | | Moves to the next existing key (larger than the cursor) |
| closest | c, = | | Moves to the closest existing key (closer of prev and next, arbitrarily tiebreaking to prev) |

