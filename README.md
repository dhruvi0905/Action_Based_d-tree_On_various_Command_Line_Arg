# File System Management Tool

🖥️ A C program for file system management using `ftw.h` for file traversal operations.

## Features
- List all files and directories
- Count files and directories
- Calculate total size of files
- Selective deletion based on file extensions
- Copy or move entire directories
- Menu-driven command-line interface for ease of use

## Requirements
- GCC compiler
- Unix/Linux-based system (for `ftw.h` compatibility)

## How to Compile
```bash
gcc -o filesystem_tool filesystem_tool.c
```

## How to Run
```bash
./filesystem_tool
```

## Usage
- Navigate through the simple menu options.
- Perform operations like listing, counting, deleting, copying, and moving files/directories.

## Notes
- Ensure you have the necessary permissions for file and directory operations.
- Use with caution when deleting files, as actions may be irreversible.

## License
This project is open-source and free to use.

