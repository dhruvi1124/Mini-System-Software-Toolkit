# Mini System Software Toolkit

### Modular C-Based System Software Simulation Framework

The **Mini System Software Toolkit** is an educational, menu-driven system software application written in C11. It provides a clean, interactive simulation of core software components responsible for program translation, intermediate code generation, linking, loading, and code optimization.

---

## Overview

This toolkit unifies eight core system software components into a single menu-driven C application. Designed for hands-on academic demonstration, it simulates key language processing and execution phases—from lexical analysis and syntax parsing to object code generation, linking, loading, and optimization—using file-based inputs and outputs.

---

## Modules

| Module | Description |
| :--- | :--- |
| **Lexical Analyzer** | Scans source code to identify and categorize lexical tokens. |
| **Symbol Table** | Manages identifier entries, attributes, and scope information. |
| **Two Pass Assembler** | Translates assembly language into object code using a two-pass mechanism. |
| **Macro Processor** | Expands macro definitions and performs parameter substitution. |
| **Recursive Descent Parser** | Parses tokens top-down according to grammar rules to verify syntax. |
| **Quadruple Generator** | Converts arithmetic expressions into three-address quadruple intermediate code. |
| **Linker Loader** | Links multiple modules, resolves external symbols, performs relocation, and generates a linked memory map. |
| **Code Optimizer** | Transforms intermediate code to eliminate redundancies and improve efficiency. |

---

## Key Features

- **Menu-Driven Interface**: Simple command-line menu for selecting and running modules interactively.
- **File-Based Input/Output**: Reads input from dedicated text and assembly files and produces formatted output logs.
- **Modular C Implementation**: Built with clean, independent C modules adhering to the C11 standard.
- **Educational Simulation**: Practical visual demonstration of low-level system software concepts.
- **Generated Output Files**: Creates persistent output artifacts for inspection and analysis.
- **C11 Compilation with GCC**: Fully compatible with GCC and standard build tools.

---

## Project Structure

```text
Mini-System-Software-Toolkit/
├── bin/
│   └── toolkit.exe
├── docs/
│   ├── diagrams/
│   ├── flowcharts/
│   └── screenshots/
├── input/
│   ├── assembler_input.asm
│   ├── lexical_input.txt
│   ├── linker_input.txt
│   ├── macro_input.asm
│   ├── optimizer_input.txt
│   ├── parser_input.txt
│   ├── quadruple_input.txt
│   └── symbol_table_input.txt
├── output/
│   ├── assembler_symbol_table.txt
│   ├── intermediate_code.txt
│   ├── linker_output.txt
│   ├── macro_output.txt
│   ├── object_code.txt
│   ├── optimizer_output.txt
│   ├── parser_output.txt
│   ├── quadruple_output.txt
│   ├── symbol_table.txt
│   └── tokens.txt
├── src/
│   ├── code_optimizer/
│   │   ├── optimizer.c
│   │   └── optimizer.h
│   ├── common/
│   │   ├── common.c
│   │   └── common.h
│   ├── lexical_analyzer/
│   │   ├── lexical_analyzer.c
│   │   └── lexical_analyzer.h
│   ├── linker_loader/
│   │   ├── linker_loader.c
│   │   └── linker_loader.h
│   ├── macro_processor/
│   │   ├── macro_processor.c
│   │   └── macro_processor.h
│   ├── quadruple_generator/
│   │   ├── quadruple.c
│   │   └── quadruple.h
│   ├── recursive_descent_parser/
│   │   ├── parser.c
│   │   └── parser.h
│   ├── symbol_table/
│   │   ├── symbol_table.c
│   │   └── symbol_table.h
│   ├── two_pass_assembler/
│   │   ├── assembler.c
│   │   ├── assembler.h
│   │   ├── pass1.c
│   │   └── pass2.c
│   └── main.c
└── Makefile
```

---

## How to Run

Navigate to the project directory:

```powershell
cd D:\SS\Mini-System-Software-Toolkit
```

Run the build command to compile the complete C project and launch `bin/toolkit.exe`:

```powershell
build-toolkit
```

The `build-toolkit` command compiles all C modules using GCC and automatically starts the interactive toolkit executable.

---

## Input and Output

| Module | Input | Output |
| :--- | :--- | :--- |
| **Lexical Analyzer** | `input/lexical_input.txt` | `output/tokens.txt` |
| **Symbol Table** | `input/symbol_table_input.txt` | `output/symbol_table.txt` |
| **Two Pass Assembler** | `input/assembler_input.asm` | `output/intermediate_code.txt`, `output/object_code.txt` |
| **Macro Processor** | `input/macro_input.asm` | `output/macro_output.txt` |
| **Recursive Descent Parser** | `input/parser_input.txt` | `output/parser_output.txt` |
| **Quadruple Generator** | `input/quadruple_input.txt` | `output/quadruple_output.txt` |
| **Linker Loader** | `input/linker_input.txt` | `output/linker_output.txt` |
| **Code Optimizer** | `input/optimizer_input.txt` | `output/optimizer_output.txt` |

---

## Example

### Quadruple Generation

**Input (`input/quadruple_input.txt`):**
```text
a + b * c
```

**Output (`output/quadruple_output.txt`):**
```text
(*, b, c, T1)
(+, a, T1, T2)
```

---

## Technology Stack

- **Language**: C
- **Standard**: C11
- **Compiler**: GCC
- **Shell Environment**: PowerShell
- **Processing Mode**: File-based processing

---

## Academic Context

- **Institution**: Vishwakarma Government Engineering College, Chandkheda
- **Department**: Department of Computer Engineering
- **Subject**: System Software
- **Subject Code**: BE05000261
- **PBL Project**: Mini System Software Toolkit

---

## Learning Outcomes

- Understanding lexical analysis and symbol management.
- Implementation of assembly translation and macro processing logic.
- Syntactic parsing and intermediate code generation using quadruples.
- Practical insight into linking, loading address relocation, and code optimization.

---

## Conclusion

The Mini System Software Toolkit offers a practical and structured exploration of system software engineering principles. By implementing core translation and program execution phases in modular C11 code, the project bridges theoretical concepts with hands-on system programming practice.
