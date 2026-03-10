# cpp-database
Functional Database from Scratch in CPP to learn concepts.

This project follows a series of evolutions, each adding a new layer of functionality to the database.

## Evolutions

1.  **Evolution 1: Basic REPL** - Implemented a basic Read-Eval-Print Loop that handles the `.exit` command.
2.  **Evolution 2: Command Parser** - Added a parser to distinguish between meta-commands and SQL-like statements (`insert`, `select`).
3.  **Evolution 3: In-Memory Storage** - Introduced `Row` and `Table` structures to store data in memory using fixed-size pages.
4.  **Evolution 4: Persistence** - Implemented a `Pager` to handle disk-based I/O, allowing data to persist across sessions.
5.  **Evolution 5: B-Tree Indexing** - Refactored storage to use a B-Tree structure, supporting efficient lookups and insertions with node splitting.

## Build and Run

To build the database:
```bash
make
```

To run the database:
```bash
./db <filename.db>
```

### Supported Commands
- `insert <id> <username> <email>`: Inserts a new row into the database.
- `select`: Displays all rows in the database.
- `.exit`: Exits the REPL and flushes data to disk.
- `.constants`: Displays internal database constants (e.g., page size, max cells).
