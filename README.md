# Nand2Tetris: Computer System Design Laboratory (CSDLab)

Welcome to the **Nand2Tetris / Computer System Design Laboratory** repository. This repository contains a complete, bottom-up implementation of a modern computer system, following the official MIT/Nand2Tetris curriculum. Starting from a single primitive `NAND` logic gate, we systematically construct a 16-bit hardware platform (the Hack computer), write an assembler, design a stack-based virtual machine translator, implement a compiler for the high-level Jack programming language, and run interactive applications.

## 📂 Repository Structure

The project is organized into modular directories reflecting the stages of computer system design:

```directory
├── 01_Boolean_Logic/          # Boolean gates built from NAND (And, Or, Mux, DMux, etc.)
├── 02_Boolean_Arithmetic/      # Half/Full Adders, Incrementer, and 16-bit ALU
├── 03_Sequential_Logic/       # Memory hierarchy (Registers, RAM8 to RAM16K, Program Counter)
├── 04_Machine_Language/       # Assembly code programs (e.g., Mult.asm, Fill.asm)
├── 05_Computer_Architecture/  # Integrated CPU, Memory map, and the full Computer (Hack)
├── 07_08_Virtual_Machine/
│   ├── VM_Translator/         # VM Translator C++ source code (Basic and Full)
│   └── test_programs/         # VM test suites (Stack arithmetic, Control flow, Functions)
├── 09_High_Level_Language/    # Jack application programs (Pong game, Square game)
└── 10_11_Jack_Compiler/
    ├── JackAnalyzer/          # Jack Syntax Analyzer / Tokenizer in C++
    └── test_programs/         # Compiler parsing test suites (XML comparison tests)
```

---

## Project Summaries & Testing Guide

### 01. Boolean Logic (`01_Boolean_Logic/`)
Contains gate designs implemented in Hardware Description Language (HDL).
*   **Key Implementations:** `myNot.hdl`, `myAnd.hdl`, `myOr.hdl`, `myXor.hdl`, `myMux.hdl`, `myDMux.hdl`, and multi-bit / multi-way variants (`myNot16`, `myAnd16`, `myOr16`, `myOr8Way`, `myMux16`, `myMux4Way16`, `myMux8Way16`, `myDMux4Way`, `myDMux8Way`).
*   **How to Test:** Use the official **Hardware Simulator** to load and run the `.tst` scripts (e.g., `myAnd.tst`) to compare output against the `.cmp` files.

### 02. Boolean Arithmetic (`02_Boolean_Arithmetic/`)
Constructs the arithmetic engine of our computer, culminating in the Arithmetic Logic Unit (ALU).
*   **Key Implementations:** `myHalfAdder.hdl`, `myFullAdder.hdl`, `myAdd16.hdl`, `myInc16.hdl`, and the 18-input `myALU.hdl`.
*   **How to Test:** Load the `.tst` scripts (e.g., `myALU.tst`) into the **Hardware Simulator** to verify mathematical and logical computations.

### 03. Sequential Logic (`03_Sequential_Logic/`)
Implements state and memory elements synchronized by a master clock.
*   **Key Implementations:** 1-bit register (`myBit.hdl`), 16-bit register (`myRegister.hdl`), random-access memory chips (`myRAM8`, `myRAM64`, `myRAM512`, `myRAM4K`, `myRAM16K`), and the Program Counter (`myPC.hdl`).
*   **How to Test:** Test via the **Hardware Simulator**. Note that sequential tests involve clock cycles and state changes.

### 04. Machine Language (`04_Machine_Language/`)
Low-level programming written in Hack Assembly (`.asm`), designed to run directly on the Hack computer.
*   **Key Implementations:**
    *   `Mult.asm`: Multiplies two memory registers.
    *   `Fill.asm`: Listens to keyboard input and fills the screen buffer black if a key is pressed, white otherwise.
*   **How to Test:** Load and assemble files using the **Assembler**, then execute them using the **CPU Emulator**.

### 05. Computer Architecture (`05_Computer_Architecture/`)
Integrates the ALU, CPU registers, Memory maps, and Program Counter into a complete, working hardware architecture.
*   **Key Implementations:** `myCPU.hdl` (Instruction decoding and execution logic), `myMemory.hdl` (handles RAM, Screen, and Keyboard address space), and `myComputer.hdl` (combining CPU and Memory).
*   **How to Test:** Run `myComputer*.tst` scripts in the **Hardware Simulator** to execute assembled program instructions and monitor I/O registers.

### 07 & 08. Virtual Machine Translator (`07_08_Virtual_Machine/`)
Translates stack-based virtual machine (`.vm`) code into Hack Assembly (`.asm`).
*   **VMTranslator:** Supports standard arithmetic, push/pop operations.
*   **VMTranslator2:** Adds control flow commands (`label`, `goto`, `if-goto`) and function declarations / call setups (`function`, `call`, `return`).
*   **Compilation:**
    ```bash
    g++ -std=c++17 07_08_Virtual_Machine/VM_Translator/VMTranslator2.cpp -o VMTranslator
    ```
*   **Execution:**
    ```bash
    ./VMTranslator <directory_path_or_file.vm>
    ```

### 09. Jack Programming Language (`09_High_Level_Language/`)
High-level object-oriented programming on our Hack computer using Jack.
*   **Applications:** `Pong` (a basic paddle-and-ball game) and `Square` (movement and collision simulation).
*   **How to Test:** Compile the Jack files using the official `JackCompiler`, then execute the resulting `.vm` folder inside the **VM Emulator**.

### 10 & 11. Jack Syntax Analyzer / Compiler (`10_11_Jack_Compiler/`)
Parses Jack source files and outputs structured XML representation (Tokenizer/Parser).
*   **JackAnalyzer (`tokenizer.cpp`):** Implements lexical analysis and structural syntax translation.
*   **Compilation:**
    ```bash
    g++ -std=c++17 10_11_Jack_Compiler/JackAnalyzer/tokenizer.cpp -o JackAnalyzer
    ```
*   **Execution:**
    ```bash
    ./JackAnalyzer <directory_path_or_file.jack>
    ```
    This generates token XML (`my*T.xml`) and parse tree XML (`my*.xml`) next to the source files.

---

## Running Official Simulator Tests

To verify any component:
1.  Ensure you have the official **Nand2Tetris Software Suite** installed on your system.
2.  Open the appropriate simulator (e.g., `HardwareSimulator.bat`, `CPUEmulator.bat`, or `VMEmulator.bat`).
3.  Load the respective `.tst` test script from this repository.
4.  Run the test. It will automatically load the corresponding implementation and compare results against the reference `.cmp` file.

---
*Created as part of the Computer System Design Laboratory course.