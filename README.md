# KVDS

A simple in-memory "database" that is accessed over standard input/output. The database stores a set of integers and associated character data and allows querying and updating them in various ways.

## Why?

KVDS is a sample project showcasing the use of various data structures; in particular binary search trees and doubly-linked lists. Hence the name: **K**ey-**V**alue store built on **D**ata **S**tructures.

## Compiling

To compile this project you will need [Tup](https://gittup.org/tup/index.html) and [GCC](https://gcc.gnu.org)—or alternatively, you may bring your own commandline-compatible C compiler.

After installing the dependencies, just run the following line to build the project:

```bash
tup
```

After compiling, you can find the resulting binary in `bin/kvds`.

To use another compiler, change the configuration in `bin/tup.config`.

## Usage

```
bin/kvds [algorithm]
```

### Accessing the database

Upon starting the executable, you are greeted with a interactive prompt, asking for input. Commands can be entered separated by spaces or newlines. Each command may take one or more an argument, as described below.

Commands:

| Command | Alias | Arguments | Description |
| --- | --- | --- | --- |
| select | s | key: integer | Moves the cursor to a given key in the database. |
| key | k | | Prints the current cursor location. |
| exists | e | | Prints whether the selected key exists. |
| read | r | | Prints the data at the selected key. |
| write | w | data: the rest of the line | Stores data at the selected key. Passing no data will still create the key. |
| delete | d | | Deletes the selected key along with any data. |
| prev | p, < | | Moves to the previous existing key (smaller than the cursor) |
| next | n, > | | Moves to the next existing key (larger than the cursor) |
| closest | c | | Moves to the closest existing key (closer of prev and next, arbitrarily tie-breaking to prev) |
| # | | the rest of the line | Comment; ignores the rest of the line |
| help | ? | | Prints a help message |

For example, the following will save two entries in the database, and switch between them by moving the cursor left / right:

```
> select 3
> write Left
> select 6
> write Right
> prev
> read
Left
> next
> read
Right
```

The same can also be accomplished by using the command shorthands:
```
> s 3 w Left
> s 6 w Right
> < r
Left
> > r
Right
```

### Selecting an algorithms

KVDS can run using a variety of algorithms. By default, it runs all of them at once, and compares the results of different data structures to each other in order to ensure the code runs correctly.

To list all available algorithms, run `bin/kvds help`. It will print a simple usage line, as well as a list of all available algorithms.

#### Sorted lists

The simplest algorithm available in KVDS, the sorted lists algorithm (`lst`) stores items in a doubly-linked list that it keeps sorted. When moving the cursor, the algorithm always starts from the cursor's current location, so it is fast when reading/writing lots of items next to each other, but struggles when it has to make large jumps across the list.

| Operation | Best-case complexity | Worst-case complexity |
| --- | --- | --- |
| Read/write | `O(1)` | `O(n)` |
| Next/prev | `O(1)` | `O(1)` |

#### Skip lists (unimplemented)

Skip lists are an extension of linked lists which allows for more efficient lookups without the complexity of having to maintain a tree. Instead of the tree, a skip list maintains a hierarchy of "indexes" of the underlying sorted list, that allow it to skip large portions of it when searching for particular elements. You can find more information about them on [Wikipedia](https://en.wikipedia.org/wiki/Skip_list).

| Operation | Best-case complexity | Worst-case complexity |
| --- | --- | --- |
| Read/write | `O(log n)` | `O(n)` |
| Next | `O(1)` | `O(1)` |

#### Scapegoat trees

Scapegoat trees are binary search trees that are rebalanced whenever an insertion or deletion results in a tree that has a height disparity over some factor, α, in which case the nearest ancestor with a size disparity of over α is located and completely recreated as a balanced tree. You can find more information about them on [Wikipedia](https://en.wikipedia.org/wiki/Scapegoat_tree).

In KVDS, the scapegoat algorithm (`sgt`) is slightly modified, and all nodes keep track of their size, instead of the code keeping track of their height. As such, after an insertion or deletion, we only need to go over the ancestors of a given node and find whether any have become imbalanced due to the size change.

By default, the algorithm uses an α value of 10/16, which was experimentally confirmed to result in reasonable performance. To use another value, you can define `BST_SCAPEGOAT_FACTOR` when compiling, by inserting a line like the following to `tup.config`:

```
CONFIG_CCFLAGS=-DBST_SCAPEGOAT_FACTOR=4/6
```

| Operation | Best-case complexity | Worst-case complexity |
| --- | --- | --- |
| Read | `O(log n)` | `O(log n)` |
| Write | `O(log n)` | `O(n)` (amortized to `O(log n)`) |
| Next/prev | `O(log n)` | `O(log n)` |

#### AVL trees (unimplemented)

AVL trees are binary search trees that are balanced by keeping track of height "defects" on each side of a node. After each modification to the tree, those defects are used to drive the rotations that will bring the tree back to balanced. You can find more information about them on [Wikipedia](https://en.wikipedia.org/wiki/AVL_tree).

| Operation | Best-case complexity | Worst-case complexity |
| --- | --- | --- |
| Read | `O(log n)` | `O(log n)` |
| Write | `O(log n)` | `O(log n)` |
| Next/prev | `O(log n)` | `O(log n)` |

#### Compare/inv

The compare "algorithm" in KVDS just runs all other algorithms and compares the results they produce. It is useful for debugging and testing the project; and currently, it is also the default algorithm used unless assertions are disabled.

## Developing

### Testing

KVDS is tested in two main ways. First, there are the unit test cases, which confirm that basic functionality is working and guard against intentional and accidental regressions. Second, fuzzing is used to test the code thoroughly and catch any bugs or crashes in the various algorithm implementations.

To run the unit tests, you can use the `test/run-tests.sh` script. It will run all the tests in the test folder and bail out with a diff on the first failing test.

To start the fuzzing, first ensure you have [`afl++`](https://github.com/AFLplusplus/AFLplusplus) installed and available as `afl-cc` and `afl-fuzz`. Then, run the `run-afl.sh` script; it will set things up using the unit tests as seeds for the fuzzer and storing the fuzzer state in `/tmp`. If you want to customize the how `afl++` is ran in order to make full use of `alf++`'s [many options](https://github.com/AFLplusplus/AFLplusplus/blob/stable/docs/fuzzing_in_depth.md), you can and should modify the `run-afl.sh` script or even make your own script similar to it as inspiration.

### Code Architecture

`main.c` serves as the entry point of the codebase. It uses the registry to get the algorithm to use, and the command runner to execute any the lines that get inputted into the program.

`registry.c` stores the list of algorithms. The entries of that list are stored in static program memory, and all the registry has to do is get the pointers pointing the right way.  
`registry.h` also includes macros that enable easy registration of new algorithms.

`interface.h` describes the interface of an individual algorithm, as a `struct` of function pointers. All algorithms use a main database structure coupled with a cursor that can navigate it, both of which are represented as opaque void pointers. Currently, algorithm code assumes that writes/deletes will only come from one cursor, while allowing for multiple cursors for reading—though the command executor will only ever use one single cursor.

`commands.c` implements the command runner, which parses user commands and calls the relevant functions of the algorithm interface. Having the command runner separate from the main entry point might appear slightly over-engineered, but it makes  memory ownership much easier to keep track of.

`algo/*.c` contains the various algorithms described above. Each of them is built as a separate object file that uses `__attribute__((constructor))` from a macro in `registry.h` to register itself in the final linked program.

To create a new algorithm, all one needs to do is copy one of the existing files, change the prefix of functions as well as the registration macro at the end, and code away.

Most of the algorithms have an `*_assert_invariants` function, which takes in the database and uses `assert` (from `<assert.h>`) to double-check that the data structure is correct. This can be of invaluable help when developing more complex structures, as otherwise a broken invariant in e.g. the sorting of a tree's nodes can lead to confusing and hard to debug states later on.

### Tools used

Two very useful tools used while developing the code were be [`gdb`](https://www.sourceware.org/gdb/) and [`valgrind`](https://valgrind.org). GDB allows for debugging crashes, which in C are quite common. Valgrind allows for checking for memory leaks and invalid memory accesses. In addition, [`rr`](https://rr-project.org) almost made it in as a rollback-capable debugger, but by the time it was needed, most of the code was already finished and GDB proved sufficient.

Other than those, all one really needs is a text editor; preferably one that supports syntax highlighting and integrates with some C language server, e.g. [`clangd`](https://clangd.llvm.org).

As mentioned in the compiling and testing sections above, others tools used for the development include [`tup`](https://gittup.org/tup/index.html), [`gcc`](https://gcc.gnu.org), and [`afl++`](https://github.com/AFLplusplus/AFLplusplus). In addition to them, [`clang-format`](https://clang.llvm.org/docs/ClangFormat.html) is used to format the codebase; and of course, the whole thing is managed through [`git`](https://git-scm.com) and documented in [GFM markdown](https://github.github.com/gfm/).
