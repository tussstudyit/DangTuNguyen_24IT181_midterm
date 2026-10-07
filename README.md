# Midterm Project: Implementation of UNIX ls

- **Student Name:** Đặng Tú Nguyên 
- **Student ID:** 24IT181
- **Course:** Operating Systems / Systems Programming
- **Project Name:** `DangTuNguyen_24IT181_midterm`
- **GitHub Repository:** [https://github.com/tussstudyit/DangTuNguyen_24IT181_midterm](https://github.com/tussstudyit/DangTuNguyen_24IT181_midterm)
- **Reference Specification:** NetBSD 10.1 `ls(1)` General Commands Manual

---

## 1. Project Overview

The objective of this project is to develop a fully functional, robust, and modular implementation of the UNIX `ls(1)` command from scratch in the C programming language (C99 standard). The tool strictly adheres to the specifications and behavior defined in the NetBSD General Commands Manual page for `ls(1)`.

This project explores foundational UNIX operating system concepts, including:
- **Filesystem navigation and directory operations:** `opendir(3)`, `readdir(3)`, `closedir(3)`.
- **File metadata querying:** `lstat(2)`, `stat(2)`, file status, inode serialization, link counts, and physical storage blocks.
- **Security and access control:** File mode bits, permissions (`rwx`), special bits (Set-UID `s`/`S`, Set-GID `s`/`S`, Sticky bit `t`/`T`).
- **User and group resolution:** Mapping numeric UIDs and GIDs to database names (`getpwuid(3)`, `getgrgid(3)`).
- **Time attribute management:** Distinguishing between modification time (`mtime`), status change time (`ctime`), and last access time (`atime`).
- **Formatting and columnation:** Aligning tabular output dynamically, calculating total directory block usage, and formatting human-readable units.
- **Cross-platform portability:** Seamless compilation and execution on POSIX UNIX platforms (Linux, NetBSD, FreeBSD, macOS) as well as Windows (MinGW-w64 / MSYS2).

---

## 2. Implemented Features & Options Matrix

The program implements all 20 options and their exact precedence and interaction rules defined in the manual synopsis:

```
ls [-AacdFfhiklnqRrSstuw] [file ...]
```

| Flag | Name / Category | Description & NetBSD Specification Behavior |
| :---: | :--- | :--- |
| `-A` | Almost All | Lists all entries except `.` and `..`. Automatically enabled when running as super-user (`geteuid() == 0`). |
| `-a` | All | Includes directory entries starting with a dot (`.`), including `.` and `..`. Overrides omission of `.`/`..`. |
| `-c` | Status Time | Uses time of file status change (`st_ctime`) instead of last modification for sorting (`-t`) or printing (`-l`). Overrides `-u`. |
| `-d` | Directories Plain | Treats directories as plain files instead of listing their contents. Symbolic links in arguments are not followed. Overrides `-R`. |
| `-F` | Classify | Appends indicator characters to pathnames: `/` for directory, `*` for executable, `@` for symbolic link, `=` for socket, `\|` for FIFO, `%` for whiteout. |
| `-f` | Unsorted | Output is not sorted (displays entries in raw directory order). In BSD `ls`, also enables `-a`. |
| `-h` | Human-Readable | Modifies `-s` and `-l`, rendering byte sizes and block sizes in human-readable units (`B`, `K`, `M`, `G`, `T`, `P`). Overrides `-k`. |
| `-i` | Inode | Prints the file serial number (inode number, `st_ino`) before each entry. |
| `-k` | Kilobytes | Modifies `-s`, reporting block counts in 1024-byte units (kilobytes). Overridden by `-h` if `-h` is specified later. |
| `-l` | Long Format | Displays detailed multi-column file information: mode, links, owner name, group name, size (or major/minor device numbers), date/time, and name (`-> target` for symlinks). Overrides `-n`. |
| `-n` | Numeric IDs | Identical to `-l`, except user and group IDs are displayed as numbers rather than resolving to names. Overrides `-l`. |
| `-q` | Non-Printable Escape | Prints non-printable characters in filenames as `?`. This is the default when output is directed to an interactive terminal. Overrides `-w`. |
| `-R` | Recursive | Recursively lists all subdirectories encountered. Overrides `-d`. |
| `-r` | Reverse Sort | Reverses the sort order (reverse lexicographical, smallest size first, or oldest time first). |
| `-S` | Sort by Size | Sorts directory entries by file size (`st_size`), largest first. Secondary tie-breaking by name. |
| `-s` | Block Allocation | Displays the number of file system blocks used by each file in units of 512 bytes or `BLOCKSIZE`. Displays a `total <N>` line before directory listings when output is to a terminal. |
| `-t` | Sort by Time | Sorts by modification time (or `ctime` with `-c`, `atime` with `-u`), newest first. Secondary tie-breaking by name. |
| `-u` | Access Time | Uses time of last access (`st_atime`) instead of modification time for sorting (`-t`) and printing (`-l`). Overrides `-c`. |
| `-w` | Raw Output | Forces raw printing of non-printable characters. This is the default when output is redirected to a non-terminal (pipe or file). Overrides `-q`. |

### Exact Option Precedence & Override Rules
1. **`-w` vs `-q`**: The last flag specified on the command line determines non-printable character rendering.
2. **`-l` vs `-n`**: The last flag specified determines whether names or numeric IDs are displayed.
3. **`-c` vs `-u`**: The last flag specified determines whether status change time or access time is evaluated.
4. **`-R` vs `-d`**: The last flag specified determines whether directories are recursively traversed or listed as plain files.
5. **`-k` vs `-h`**: The rightmost of `-k` and `-h` overrides the previous flag for block and size representations.
6. **Sorting**: `-f` completely disables sorting. `-S` and `-t` select the sorting metric; the default is lexicographical. `-r` reverses whichever sort criterion is active.

### Environment Variables
- `BLOCKSIZE`: If set, and neither `-h` nor `-k` is specified, block counts for `-s` and `total` lines are reported in units of that block size (minimum 512 bytes). Supports suffixes such as `k`, `K`, `m`, `M`, `g`, `G`.
- `TZ`: Defines the local timezone used for formatting dates and times in long listing.

---

## 3. Project Architecture & Modular Design

The codebase adheres to clean modular software engineering principles, dividing responsibilities into distinct modules with matching `.h` headers and `.c` source files:

```
DangTuNguyen_24IT181_midterm/
├── Makefile                # Build script for Linux, macOS, NetBSD, and Windows
├── README.md               # Detailed technical project report
├── .gitignore              # Configured to ignore binaries, .o, and build artifacts
└── src/
    ├── ls.h                # Global types, enums, option flags, and file_info structures
    ├── compat.h            # POSIX compatibility layer and fallback definitions
    ├── compat.c            # Platform abstraction (UID/GID, lstat, symlinks, isatty, major/minor)
    ├── options.h           # Option parsing declarations and NetBSD defaults
    ├── options.c           # CLI argument parsing, precedence, and override logic
    ├── file_utils.h        # File metadata querying and attribute formatting
    ├── file_utils.c        # Mode string formatting (strmode), humanization, time formatting
    ├── sort.h              # Sorting comparator prototypes
    ├── sort.c              # Multi-criteria sorting engine (size, time, name, reverse)
    ├── display.h           # Rendering and columnation prototypes
    ├── display.c           # Tabular alignment, long listing, total block calculation
    ├── traverse.h          # Traversal prototypes
    ├── traverse.c          # Directory traversal, operand partitioning, recursive descent
    └── main.c              # Application entry point, locale initialization, exit code
```

### Module Responsibilities

1. **`src/ls.h`**: Defines the central `options_t`, `file_info_t`, and `file_list_t` structures. Encapsulates options state and file metadata containers.
2. **`src/compat.h` / `src/compat.c`**: Implements cross-platform support. On POSIX systems, directly uses native system calls (`lstat`, `readlink`, `getpwuid`, `getgrgid`, `isatty`, `major`, `minor`). On Windows, provides robust Win32/C99 fallbacks (NTFS reparse points for symlinks, token inspection for super-user, username environment resolution).
3. **`src/options.h` / `src/options.c`**: Parses command-line flags and applies the NetBSD override hierarchy. Resolves terminal detection, super-user default for `-A`, and `BLOCKSIZE` environment parsing.
4. **`src/file_utils.h` / `src/file_utils.c`**:
   - Queries `lstat(2)` metadata and populates `file_info_t`.
   - Implements `file_format_mode`: constructs the 10-character mode string (`-rwxr-xr-x`, `drwxr-xr-x`, Set-UID `s`/`S`, Set-GID `s`/`S`, Sticky `t`/`T`).
   - Implements `file_format_human_size`: formats sizes to `B`, `K`, `M`, `G`, `T`, `P`.
   - Implements `file_format_time`: conforms to POSIX and NetBSD date format (`%b %e %H:%M` for files within 6 months, `%b %e  %Y` for older files).
   - Implements `file_get_indicator`: computes indicators for `-F`.
   - Implements `file_sanitize_name`: transforms non-printable characters according to `-q` / `-w`.
5. **`src/sort.h` / `src/sort.c`**: Implements multi-criteria sorting using `qsort`. Correctly handles size sort (`-S`), timestamp sort (`-t` using `mtime`, `ctime`, or `atime`), lexicographical tie-breaking, and in-place reversal (`-r`).
6. **`src/display.h` / `src/display.c`**: Calculates dynamic column widths across all files in a listing for perfect visual alignment. Outputs `total <N>` block headers and formatted rows.
7. **`src/traverse.h` / `src/traverse.c`**: Manages command-line operands:
   - Separates non-directory arguments from directory arguments.
   - Displays non-directory arguments first, followed by directories.
   - Prints `path:` headers when multiple directories are listed or in recursive mode (`-R`).
   - Prevents recursive cycles on `.` and `..`.
8. **`src/main.c`**: Coordinates program initialization, option parsing, traversal dispatch, and returns standard exit status (`0` on success, `>0` on error).

---

## 4. Key UNIX Systems Programming Concepts Implemented

### 4.1. Directory Traversal and File Metadata (`stat` vs `lstat`)
When inspecting directory entries, `lstat(2)` is used instead of `stat(2)`. This ensures that symbolic links are reported as links rather than following them to their targets, allowing the display of link permissions (`lrwxrwxrwx`), link size, and the `-> target` path.

### 4.2. File Mode and Special Permissions
File permissions are represented by an octal bitmask (`mode_t`). The 10-character string is constructed as follows:
- **Index 0 (File Type):**
  - `-`: Regular file (`S_ISREG`)
  - `d`: Directory (`S_ISDIR`)
  - `l`: Symbolic link (`S_ISLNK`)
  - `c`: Character device (`S_ISCHR`)
  - `b`: Block device (`S_ISBLK`)
  - `p`: FIFO pipe (`S_ISFIFO`)
  - `s`: Socket (`S_ISSOCK`)
  - `w`: Whiteout (`S_ISWHT`)
- **Indices 1-3 (Owner Permissions):** `r`, `w`, and executable/Set-UID:
  - `s`: Executable AND Set-UID (`S_ISUID`)
  - `S`: Not executable AND Set-UID (`S_ISUID`)
  - `x`: Executable
  - `-`: No execute
- **Indices 4-6 (Group Permissions):** `r`, `w`, and executable/Set-GID:
  - `s`: Executable AND Set-GID (`S_ISGID`)
  - `S`: Not executable AND Set-GID (`S_ISGID`)
  - `x`: Executable
  - `-`: No execute
- **Indices 7-9 (Other Permissions):** `r`, `w`, and executable/Sticky:
  - `t`: Executable AND Sticky bit (`S_ISVTX`, mode 01000)
  - `T`: Not executable AND Sticky bit (`S_ISVTX`, mode 01000)
  - `x`: Executable
  - `-`: No execute

### 4.3. Device Numbers for Character and Block Specials
For block and character devices, instead of displaying the file size, standard UNIX `ls -l` outputs the **major device number** (specifying the driver) and **minor device number** (identifying the specific device unit), formatted as:
```
crw-rw-rw- 1 root wheel   1,   3 Oct  7 10:00 /dev/null
```

---

## 5. Compilation & Installation

### Prerequisites
- A C compiler supporting C99 (`gcc`, `clang`, or MinGW GCC).
- GNU Make.

### Build Commands
To compile the project with standard optimization and strict warning flags:
```bash
make
```
This produces the executable `ls` (or `ls.exe` on Windows).

To clean object files and binaries:
```bash
make clean
```

To run the built-in quick sanity tests:
```bash
make test
```

---

## 6. How to Run the Program & Examples

### 1. Default Listing (1 entry per line)
```bash
./ls
```
*Output:*
```
Makefile
README.md
src
```

### 2. Listing Hidden Files (`-a` and `-A`)
```bash
./ls -a
```
*Lists all files including `.` and `..`.*

```bash
./ls -A
```
*Lists all hidden files excluding `.` and `..`.*

### 3. Long Listing Format (`-l` and `-la`)
```bash
./ls -la
```
*Output:*
```
total 32
drwxrwxrwx 1 ADMIN users 4096 Oct  7 10:05 .
drwxrwxrwx 1 ADMIN users 4096 Oct  7 10:01 ..
-rw-rw-rw- 1 ADMIN users  227 Oct  7 10:03 .gitignore
-rw-rw-rw- 1 ADMIN users  922 Oct  7 10:03 Makefile
-rw-rw-rw- 1 ADMIN users 8192 Oct  7 10:10 README.md
drwxrwxrwx 1 ADMIN users 4096 Oct  7 10:05 src
```

### 4. Numeric IDs (`-n`) vs User Names (`-l`)
```bash
./ls -n
```
*Output:*
```
total 24
-rw-rw-rw- 1 0 0  922 Oct  7 10:03 Makefile
-rw-rw-rw- 1 0 0 8192 Oct  7 10:10 README.md
drwxrwxrwx 1 0 0 4096 Oct  7 10:05 src
```

### 5. Human-Readable Sizes (`-lh`) and Kilobyte Units (`-sk`)
```bash
./ls -lh
```
*Output:*
```
total 16K
-rw-rw-rw- 1 ADMIN users 922B Oct  7 10:03 Makefile
-rw-rw-rw- 1 ADMIN users 8.0K Oct  7 10:10 README.md
drwxrwxrwx 1 ADMIN users 4.0K Oct  7 10:05 src
```

### 6. Classification Suffixes (`-F`)
```bash
./ls -F
```
*Output:*
```
Makefile
README.md
src/
```

### 7. Sorting: By Size (`-S`), Time (`-t`), and Reverse (`-r`)
```bash
# Sort by file size, largest first:
./ls -lS

# Sort by modification time, newest first:
./ls -lt

# Reverse sort order:
./ls -r
```

### 8. Directory as Plain File (`-d`)
```bash
./ls -d src
```
*Output:*
```
src
```

### 9. Recursive Listing (`-R`)
```bash
./ls -R src
```
*Output:*
```
src:
compat.c
compat.h
display.c
display.h
file_utils.c
file_utils.h
ls.h
main.c
options.c
options.h
sort.c
sort.h
traverse.c
traverse.h
```

### 10. Combining Multiple Operands & Error Handling
```bash
./ls non_existent_file Makefile src
```
*Output:*
```
ls: non_existent_file: No such file or directory
Makefile

src:
compat.c
...
```
*Exit status is `1` (indicating an error occurred on the missing file).*

---

## 7. Automated Test Suite & Validation

The project was validated against 31 comprehensive test cases covering all 20 options and their precedence rules:

### Test Suite Execution Output
```
===========================================
  Running ls(1) Automated Test Suite
===========================================
[PASS] 1. Default listing without arguments
[PASS] 2. Hidden files (-a)
[PASS] 3. Hidden files excluding . and .. (-A)
[PASS] 4. Long listing format (-l)
[PASS] 5. Numeric IDs (-n)
[PASS] 6. Override -l with -n
[PASS] 7. Override -n with -l
[PASS] 8. Inode numbers (-i)
[PASS] 9. Block count (-s)
[PASS] 10. Kilobyte blocks (-k)
[PASS] 11. Human readable sizes (-h)
[PASS] 12. Human readable blocks (-h -s)
[PASS] 13. Override -k with -h
[PASS] 14. Override -h with -k
[PASS] 15. Classification indicators (-F)
[PASS] 16. Reverse sorting (-r)
[PASS] 17. Sort by size (-S)
[PASS] 18. Sort by modification time (-t)
[PASS] 19. Status change time (-c -l)
[PASS] 20. Access time (-u -l)
[PASS] 21. Override -c with -u (-lcu)
[PASS] 22. Override -u with -c (-luc)
[PASS] 23. Plain directory listing (-d)
[PASS] 24. Recursive traversal (-R)
[PASS] 25. Override -R with -d (-Rd)
[PASS] 26. Override -d with -R (-dR)
[PASS] 27. Unsorted output (-f)
[PASS] 28. Force raw non-printable (-w)
[PASS] 29. Force escape non-printable (-q)
[PASS] 30. Non-directory + directory operands
[PASS] 31. Non-existent file error exit status (>0)
===========================================
Test Summary: 31 / 31 passed (0 failed)
===========================================
```

---

## 8. Git Repository Information & Submission

- **Repository Name:** `DangTuNguyen_24IT181_midterm`
- **GitHub URL:** [https://github.com/tussstudyit/DangTuNguyen_24IT181_midterm](https://github.com/tussstudyit/DangTuNguyen_24IT181_midterm)
- **Author:** Đặng Tứ Nguyên
- **Student ID:** 24IT181

All binaries (`ls`, `ls.exe`) and object files (`*.o`) are excluded via `.gitignore` to maintain a clean source repository adhering to professional Git practices.
