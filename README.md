# cpp-database
Functional Database from Scratch in CPP to learn concepts.

This project follows a series of evolutions, each adding a new layer of functionality to the database.

## Evolutions

1.  **Evolution 1: Basic REPL** - Implemented a basic Read-Eval-Print Loop that handles the `.exit` command.
2.  **Evolution 2: Command Parser** - Added a parser to distinguish between meta-commands and SQL-like statements (`insert`, `select`).
3.  **Evolution 3: In-Memory Storage** - Introduced `Row` and `Table` structures to store data in memory using fixed-size pages.
4.  **Evolution 4: Persistence** - Implemented a `Pager` to handle disk-based I/O, allowing data to persist across sessions.
5.  **Evolution 5: B-Tree Indexing** - Refactored storage to use a B-Tree structure, supporting efficient lookups and insertions with node splitting.
6.  **Evolution 6: Refactoring and Decoupling** - Decoupled the monolithic `main.cpp` into meaningful modules (`Pager`, `Table`, `Node`, `Row`, `Statement`, etc.) following C++ conventions.
7.  **Evolution 7: SQL ANSI Implementation** - Started implementing ANSI SQL query language support, including `SELECT * FROM table` and `WHERE id = <value>` filtering.

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
- `select * from users`: Displays all rows in the database.
- `select * from users where id = <id>`: Displays a specific row by ID.
- `.exit`: Exits the REPL and flushes data to disk.
- `.constants`: Displays internal database constants.

## Architecture

The database is modularized into the following components:
- `Pager`: Manages disk I/O and page caching.
- `Table`: Orchestrates B-Tree operations and table metadata.
- `Node`: Defines B-Tree node layouts and accessors.
- `Row`: Handles serialization of database records.
- `Statement`: Parses and executes SQL-like commands.
- `Cursor`: Provides an abstraction for navigating the database.
- `Constants`: Centralized definitions for database parameters.
