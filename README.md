# What is this?

This is a sample project showcasing the use of various data structures; in particular binary search trees and doubly-linked lists.

This project implements a simple in-memory "database" that is accessed over standard input/output. The database stores a set of integers and associated character data and allows querying and updating them in various ways.

## Accessing the database

Upon starting the executable, you are (will be) greeted with a interactive prompt, asking for input. Commands are entered one-by-one on separate lines; first word is the command, the rest is the data.

Commands:

| Command | Alias | Arguments | Description |
| --- | --- | --- | --- |
| select | s | key: integer | Moves the cursor to a given key in the database. |
| exists? | e | | Prints whether the selected key exists. |
| read | r | | Prints the data at the selected key. |
| write | w | data: ... | Stores data at the selected key. Passing no data will still create the key. |
| delete | d | | Deletes the selected key along with any data. |
| prev | p, < | | Moves to the previous existing key (larger than the cursor) |
| next | n, > | | Moves to the next existing key (larger than the cursor) |
| closest | c, = | | Moves to the closest existing key (closer of prev and next, arbitrarily tiebreaking to prev) |

