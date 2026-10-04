# Search Tree Utilities — 2–3 Tree and AVL Tree

A menu-driven C++ program that indexes college and department records with two balanced search trees and combines their search results.

**Source:** `main.cpp`  
**Authors:** 劉至嘉 (11227205) and 楊碕萍 (11027214)

## Features

- **2–3 tree indexed by college name:** inserts records, splits overflowing nodes, and promotes keys to maintain the tree structure.
- **AVL tree indexed by department name:** tracks node heights and balances insertions with single and double rotations.
- **Duplicate-key support:** stores multiple record serial numbers for the same college or department.
- **Intersection queries:** searches both indexes and returns records matching both the college and department conditions.
- **Wildcard queries:** accepts `*` for either condition to include every record for that field.
- **Tree summaries:** displays the tree height and the records associated with its root keys after construction.
- **Record display:** prints the serial number, college, department, day/night category, education level, and student count.
- **File loading:** reads tab-separated text, skips the first three header lines, and removes commas and quotation marks before parsing.
- **Interactive controls:** provides menu navigation, missing-file retry prompts, and checks that the necessary trees exist before querying.

## Menu

| Option | Action | Prerequisite |
| --- | --- | --- |
| `0` | Quit | None |
| `1` | Load a file and build the 2–3 tree | An `input<N>.txt` file |
| `2` | Build the AVL tree from the loaded records | Run option `1` first |
| `3` | Query the intersection of college and department matches | Run options `1` and `2` first |

Loading a new dataset through option `1` clears both previous trees. Rebuild the AVL tree before querying the new dataset.

## Input Format

Place `input<N>.txt` in the working directory and enter its file number when prompted. For example, entering `1` loads `input1.txt`; entering `0` at the file prompt cancels loading.

The first three lines are treated as headers. Each remaining line must contain at least seven tab-separated fields:

| Field position (1-based) | Meaning |
| --- | --- |
| `1` | College code; not retained |
| `2` | College name |
| `3` | Department code; not retained |
| `4` | Department name |
| `5` | Day/night category |
| `6` | Education level |
| `7` | Student count |

Additional fields are ignored. Serial numbers are assigned sequentially to the data rows, starting at `1`.

## Typical Usage

1. Select `1` and enter the dataset number.
2. Select `2` to build the department index.
3. Select `3` and enter a college name and department name.
4. Use `*` for either field to leave that condition unrestricted.
5. Select `0` to exit.

Searches use exact string comparisons. Query input is read as a whitespace-delimited token, so names containing spaces are not supported by the current prompt handling.

The current 2–3 tree search can loop indefinitely when an exact match occurs in an internal node containing two keys: it collects the match but neither exits nor advances to a child. This affects some exact-college queries; the wildcard uses a separate traversal.

## Output

Tree summaries and query results are printed to the console. This program does not generate report files.

# How to build
Use cmake and build it in your system (I used vscode's CMake Tools)
[i watched this video](https://youtu.be/Op4iB39YA4g?si=xvxIDr06rj__-Z78)
