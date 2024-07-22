# KVDS

A simple in-memory "database" that is accessed over standard input/output. The database stores a set of integers and associated character data and allows querying and updating the stored data.

KVDS is a sample project showcasing the use of various data structures, in particular binary search trees and doubly-linked lists. Hence the name: **K**ey-**V**alue store built on **D**ata **S**tructures.

![logo](data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHdpZHRoPSIxMzMiIGhlaWdodD0iNjkuMDIiIHZpZXdCb3g9IjAgMCAzNS4xOSAxOC4yNiI+PHBhdGggZD0iTTIuMTIgMS44MmMtLjA3LjA4LS4xLjIyLS4xLjQxIDAgLjIuMDMuMzIuMS4zNy42NS4wNCAxLjA1LjE2IDEuMi4zNS4xNy4xOS4yNi42OC4yNiAxLjQ4djcuNzVjMCAuODMtLjA4IDEuMzMtLjIzIDEuNTEtLjE0LjE3LS41NS4yOC0xLjIzLjMyLS4wNy4wNy0uMS4yMS0uMS40MSAwIC4yLjAzLjMxLjEuMzdsMi41NC0uMDRjLjQ0IDAgMS4yNy4wMSAyLjUxLjA0LjA2LS4wNi4xLS4xOC4wOC0uMzggMC0uMi0uMDItLjMzLS4wOC0uNC0uNzQtLjAzLTEuMi0uMTMtMS4zNy0uMy0uMTctLjItLjI1LS43LS4yNS0xLjUzVjguNWMuNDcgMCAuODQuMDYgMS4xLjE4LjI1LjEuNS4zMy43Ni42OGwzIDMuOTVjLjQ3LjYyLjcyIDEuMDcuNzYgMS4zNyAwIC4wOC4wNC4xMi4xMS4xMmEyOC4xIDI4LjEgMCAwIDEgMy4wNyAwYy4wNy0uMDYuMS0uMTguMS0uMzhzLS4wMy0uMzMtLjEtLjRjLS40LS4wMy0uNjgtLjA4LS44My0uMTRhMi40IDIuNCAwIDAgMS0uNjItLjU3bC00LjMtNS4yYy0uMTktLjIxLS4yOC0uMzgtLjI4LS41IDAtLjE1LjE3LS40LjUtLjc1YTEuMiAxLjIgMCAwIDEtLjA2LS40YzAtLjIzLjAxLS40My4xNi0uNjRsLjM0LS4xNi43Ni4wMiAyLjA0LTEuOThjLjQ4LS41LjgyLS43OSAxLjAzLS45LjIxLS4xLjU1LS4xNyAxLS4yLjA3LS4wNS4xLS4xOC4xLS4zOHMtLjAzLS4zMy0uMS0uNGMtLjkuMDQtMS41My4wNi0xLjkuMDZsLTIuMjgtLjA2Yy0uMDguMDgtLjEyLjIyLS4xMi40MS4wMS4yLjA1LjMyLjEyLjM3LjI1LjAyLjQ0LjA0LjU4LjA3LjE0LjAyLjI4LjA3LjQxLjE1LjE0LjA3LjE5LjE3LjE1LjMtLjAzLjE0LS4xNS4zLS4zNS41TDcuMzMgNy4wOGMtLjUuNS0xLjEuNzgtMS43OC44MlY0LjQzYzAtLjguMTEtMS4yOS4zMy0xLjQ5LjIzLS4yLjczLS4zIDEuNS0uMzQuMDYtLjA1LjEtLjE4LjEtLjM4cy0uMDQtLjMzLS4xLS40Yy0xLjM3LjA0LTIuMjguMDYtMi43NC4wNmwtMi41Mi0uMDZ6bTIzLjkgMGE1OSA1OSAwIDAgMC0xLjg0LjA1bC0xLjU0LjAzLTIuNTMtLjA2Yy0uMDcuMDgtLjEuMjItLjEuNDEgMCAuMi4wMy4zMi4xLjM3LjY0IDAgMS4wNC4xIDEuMi4zMS4xNy4yLjI2LjcuMjYgMS41MnYxLjI0bC45Ny0uMDMuMy4xMmMuMi4yLjIxLjQzLjIxLjY3IDAgLjI1IDAgLjUyLS4yNy43MWwtLjE4LjA3Yy0uNS4wNy0uODEuMjEtMSAuNGwtLjAzLjA0djQuNTJjMCAuODEtLjA5IDEuMzItLjI2IDEuNTItLjE2LjItLjU2LjMtMS4yLjMtLjA3LjA4LS4xLjIxLS4xLjQxIDAgLjIuMDMuMzEuMS4zN2ExNTEuMyAxNTEuMyAwIDAgMSAzLjk5LS4wMmwyLjIzLjAyYy42OSAwIDEuMzQtLjEgMS45OC0uMjlhNi42IDYuNiAwIDAgMCAxLjg1LS45NCA0LjUgNC41IDAgMCAwIDEuNDYtMS45NGMuMzctLjg1LjU2LTEuODYuNTYtMy4wMiAwLS40NS0uMDItLjg4LS4wNy0xLjNhNi4yNiA2LjI2IDAgMCAwLTEuMDctLjA4Yy0uMzQgMC0uNjUuMDUtLjk0LjE0LjAzLjUuMDUgMS4wOC4wNSAxLjcyYTggOCAwIDAgMS0uMzIgMi40NyAzLjE0IDMuMTQgMCAwIDEtLjk1IDEuNTJjLS40Mi4zNS0uODcuNi0xLjM2Ljc0YTYuNSA2LjUgMCAwIDEtMS43My4yYy0xLjUyIDAtMi4yNy0uMzUtMi4yNy0xLjA0VjMuNjRjMC0uMzMuMTgtLjU4LjU2LS43NS4zNy0uMTguOC0uMjcgMS4zLS4yNy44IDAgMS41LjEgMi4wNy4zMS41OC4yIDEuMDQuNDggMS4zOS44Mi4zNC4zNC42Mi44LjgyIDEuMzcuMDguMjIuMTQuNDQuMi42NmE4LjkgOC45IDAgMCAxIDEuOTEtLjEgNS40MiA1LjQyIDAgMCAwLTEuMjQtMi4xNUE1Ljk4IDUuOTggMCAwIDAgMjYgMS44M3pNMTQgNi4wNmE4NC4yNiA4NC4yNiAwIDAgMS0zLjgyLjAybC0uOTQtLjAyYy0uMDUuMDctLjA4LjItLjA4LjQgMCAuMTguMDQuMzEuMS4zOGwuMTMuMDJjLjI2LjA0LjQ3LjEuNjMuMTZsLjAxLjAxYS45Ni45NiAwIDAgMSAuMTUuMDlsLjA2LjA2Yy4xNy4xNy40LjYyLjcgMS4zN2wuNjUgMS42Ljc1IDEuODUuODYgMS4wNGguMDF2LjAxYy4yNS4yOC40NS40NC40OC40Ni4wNS4wMi4zLjA4LjY5LjFsLjI1LjEyYy4yLjIuMjIuNDQuMjIuNjggMCAuMjQgMCAuNDktLjI1LjY5bC0uMjYuMDgtLjcxLS4wMSAxLjUgMy42N2MuMS4zLjMzLjQ1LjY4LjQ1LjI4IDAgLjQ4LS4xNS42LS40NUwyMC41NyA4LjZjLjIzLS41NS40OC0uOTYuNzQtMS4yMy4yOC0uMjguNy0uNDUgMS4yNC0uNTIuMDgtLjA1LjExLS4xOC4xLS4zOCAwLS4yLS4wMy0uMzMtLjEtLjQtLjg1LjAzLTEuNDcuMDUtMS44Ni4wNWwtMi4zLS4wNWMtLjA1LjA3LS4wOC4yMS0uMDguNDEgMCAuMi4wMy4zMi4wOC4zNy43My4wNiAxLjE3LjIgMS4zLjQ0LjE0LjI0LjA4LjYzLS4xNyAxLjE3bC0zLjE0IDguMDRoLS4xbC0zLjM1LTguMTRhNC4yOCA0LjI4IDAgMCAxLS4yLS41OGMtLjAzLS4xLS4wNC0uMi0uMDUtLjI5IDAtLjA4IDAtLjE1LjAzLS4yYS4zLjMgMCAwIDEgLjAzLS4wOC40My40MyAwIDAgMSAuMTUtLjEybC4xMS0uMDUuMzItLjA5Yy4xOC0uMDQuNDEtLjA3LjY4LS4xLjA1LS4wNS4wOC0uMTguMDgtLjM4cy0uMDMtLjMzLS4wOC0uNHptMTcuMjcgMC0uNDcuMDFhNi41IDYuNSAwIDAgMC0yLjc4Ljc0bC0uMDUuMDItLjE4LjExYTIuODcgMi44NyAwIDAgMC0xLjIzIDEuNjh2LjA0YTMuNyAzLjcgMCAwIDAtLjEuNTVsLS4wMS4wMnYuMDJhNC42IDQuNiAwIDAgMC0uMDIuNDNjMCAxIC40NCAxLjgzIDEuMzIgMi40Ny4zLjIuNjEuMzkuOTUuNTQuMzItLjI4LjU3LS43Ljc1LTEuMjZsLjEyLS40M2E0Ljc4IDQuNzggMCAwIDEtMS0uNzd2LS4wMWEyIDIgMCAwIDEtLjEtLjE0bC0uMDMtLjAzYTEuNzYgMS43NiAwIDAgMS0uMjMtLjUyIDEuNTQgMS41NCAwIDAgMS0uMDQtLjM3IDIuMzcgMi4zNyAwIDAgMSAuMTItLjcyIDEuOSAxLjkgMCAwIDEgLjUzLS43OHYtLjAxbC4wMS0uMDFhMi41NCAyLjU0IDAgMCAxIC41LS4zN2guMDFhMy4yIDMuMiAwIDAgMSAxLjExLS40IDMuNzUgMy43NSAwIDAgMSAuOTEtLjAzbC4yOS4wMmMuMy4wMi41Ny4wNi44Mi4xMi4zMy4wOC42LjE5Ljg1LjMzYTIuMjEgMi4yMSAwIDAgMSAuNDIuMzIgMy45IDMuOSAwIDAgMSAxLjA2IDEuNjMgNS4xNSA1LjE1IDAgMCAwIC43MS0uMDYuNi42IDAgMCAwIC4wNy0uMDNsLjAyLS4wMS4wMy0uMDJ2LS4wM2E5LjkgOS45IDAgMCAwLS4xNi0xLjE3IDIwLjk1IDIwLjk1IDAgMCAwLS4zLTEuMzQgNDUuMzIgNDUuMzIgMCAwIDEtMS43Mi0uMzVsLS4wOC0uMDJhOC45IDguOSAwIDAgMC0uMjUtLjA0bC0uMS0uMDItLjI2LS4wMy0uMS0uMDFhMTEuMzYgMTEuMzYgMCAwIDAtMS4zOS0uMDd6bS43NCA1LjY2LS4wMy4wNmMtLjI1LjYtLjU2IDEuMS0uOTQgMS41MWE1LjIgNS4yIDAgMCAxIDIuNDQuNzZjLjU4LjQuODcgMS4xLjg3IDIuMDggMCAuNzktLjI5IDEuMzktLjg2IDEuNzktLjU2LjQtMS4zNi41OS0yLjM5LjU5LTIuNDMgMC0zLjkxLS45My00LjQ0LTIuNzZhMS4xOCAxLjE4IDAgMCAwLS44My4wNmMuMDggMS4xNy4yNSAyLjE4LjUgMy4wMi40NyAwIDEuMTUuMDggMi4wMy4yNC44OS4xNSAxLjczLjIyIDIuNTQuMjIgMS4yMiAwIDIuMjctLjIxIDMuMTUtLjY2IDEuNTgtLjc3IDIuMzYtMS44NCAyLjM2LTMuMTlhMy4xIDMuMSAwIDAgMC0xLjEyLTIuNTggNS42NyA1LjY3IDAgMCAwLTMuMDQtMS4xMWwtLjI0LS4wM3oiIGNvbG9yPSIjMDAwIiBwYWludC1vcmRlcj0ic3Ryb2tlIGZpbGwgbWFya2VycyIgc3R5bGU9Ii1pbmtzY2FwZS1zdHJva2U6bm9uZSIgdHJhbnNmb3JtPSJ0cmFuc2xhdGUoLTEuNjIgLTEuNDMpIi8+PC9zdmc+)

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

As mentioned in the compiling and testing sections above, others tools used for the development include [`tup`](https://gittup.org/tup/index.html), [`gcc`](https://gcc.gnu.org), and [`afl++`](https://github.com/AFLplusplus/AFLplusplus). In addition to them, [`clang-format`](https://clang.llvm.org/docs/ClangFormat.html) is used to format the codebase; and of course, the whole thing is managed through [`git`](https://git-scm.com) using [Conventional commits](https://www.conventionalcommits.org/en/v1.0.0/) and documented in [GFM markdown](https://github.github.com/gfm/).

To enable running `clang-format` and checking commit messages automatically before committing, run the following line:
```bash
git config --local core.hooksPath .githooks
# https://stackoverflow.com/a/54281447
```

*(Logo made using [Linux Libertine](https://github.com/libertine-fonts/libertine), [Inkscape](https://inkscape.org), and [svgomg](https://jakearchibald.github.io/svgomg/))*
