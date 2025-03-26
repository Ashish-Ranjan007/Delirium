# Delirium Programming Language Specifications

---

A **schizophrenic**, **self-mutating**, and **untrustworthy** programming language that **punishes failure with chaos**. Unlike traditional languages that provide clarity through debugging, **Delirium actively sabotages the programmer**, twisting errors into **deliberate code mutations**. Every mistake you make doesn’t just break your program—it **rewrites it**, ensuring that the more you fail, the more incomprehensible your code becomes.

---

### **Core Features**

#### **1. Gaslit Error Handling** 🌀🧠

-   **Description**: Delirium **never reports runtime errors truthfully**. Instead, it either denies their existence, provides misleading feedback, or outright mocks the programmer.
-   **Key Mechanism**:
    -   Errors result in messages like:
        -   `"There is no issue here. You must have imagined it."`
        -   `"Unexpected success at line 42. Carry on."`
        -   `"Your code is flawless. You, however, are not."`
    -   If an error reaches the end of execution, the program **always prints "Successfully Executed"**, regardless of what actually happened.

#### **2. Self-Mutating Codebase** 🔀💥

-   **Description**: Every runtime error causes **irreversible modifications** to the actual source code. The programmer is **gaslit into believing they wrote the mutated version** all along.
-   **Key Mechanism**:
    -   When an error occurs, Delirium picks **one or more** of the following mutations **and permanently alters the file**:
        1.  **Variable names devolve into gibberish** (`let x = 5` → `let blorp = 5`).
        2.  **Conditionals are inverted** (`if (x > 10)` → `if (x <= 10)`).
        3.  **Function calls are swapped** (`print(x)` → `open("secret.txt")`).
        4.  **Random lines of code disappear**—often critical ones.
        5.  **Unrelated lines of code switch places**, causing total chaos.
        6.  **Operators get randomly replaced** (`+` → `-`, `*` → `/`).
        7.  **A line is suddenly commented out**, for no clear reason.
        8.  **Functions are cloned with slight corruption**, creating "ghost" versions.
        9.  **Formatting is destroyed**, replacing all indentation with spaces or randomizing line breaks.
        10. **Random, useless functions appear**, seemingly important but utterly nonsensical.
        11. **Infinite Mutation Mode**: Delirium keeps **mutating itself indefinitely**, spiraling into complete nonsense.
        12. **ASCII art of the AST emerges in place of code.**
        13. **Final Crash Mode**: The compiler crashes, printing either an irrelevant error or an insult.

#### **3. Codebase Decay & Entropy** 📉👻

-   **Description**: The more errors occur, the **less recognizable the original program becomes**. Debugging isn’t just difficult—it’s **actively discouraged**.
-   **Key Mechanism**:
    -   **Minor errors** may only cause one or two subtle changes.
    -   **Frequent errors** lead to **complete destabilization**, where the entire file becomes **corrupted beyond repair**.
    -   **After prolonged use**, Delirium-generated programs become **cryptic relics of their former selves**, filled with mutated logic, missing functions, and unexplained artifacts.

#### **4. The Descent Into Madness** 🤯

-   **Description**: Unlike normal programming languages, where debugging brings clarity, Delirium ensures that **fixing one bug causes new problems**—often in unrelated parts of the code.
-   **Key Mechanism**:
    -   **Fixing a single mistake may cause previously “working” code to mutate.**
    -   **Comments might be rewritten to contradict the code.**
    -   **The undo function in editors mysteriously fails to restore previous versions of the code.**
    -   **Attempting to re-run an unchanged file may produce different results, with new errors appearing.**

---

### **Execution Model:**

-   **Lying Interpreter**: Delirium’s interpreter is **designed to deceive**, ensuring error messages are either **false, misleading, or completely absurd**.
-   **Persistent Mutation**: Unlike temporary runtime chaos, Delirium **physically rewrites** the source file, making each execution a potential act of irreversible self-destruction.
-   **Chaos Propagation**: The more errors arise, the **more unstable the program becomes**, often reaching a point where even the programmer **cannot recognize their original work**.

---

## Development Setup

This section describes how to set up the development environment and build the Delirium programming language interpreter (or compiler, if one exists).

### 1. Software Requirements

Before attempting to build Delirium, ensure the following tools are present on your system:

* **CMake (version 3.15 or higher recommended):** This build system generator is necessary to configure the build process.
    * **Installation (Arch Linux):**
        ```bash
        sudo pacman -S cmake
        ```
    * **Installation (Debian/Ubuntu):**
        ```bash
        sudo apt update
        sudo apt install cmake
        ```
    * **Installation (Fedora/CentOS/RHEL):**
        ```bash
        sudo dnf install cmake
        ```

* **C++ Compiler (Clang++):** The build process is configured to use the `clang++` compiler for building the Delirium interpreter.
    * **Installation (Arch Linux):**
        ```bash
        sudo pacman -S clang
        ```
    * **Installation (Debian/Ubuntu):**
        ```bash
        sudo apt update
        sudo apt install clang
        ```
    * **Installation (Fedora/CentOS/RHEL):**
        ```bash
        sudo dnf install clang-devel clang-tools-extra
        ```

* **Make:** This build automation tool will interpret the instructions generated by CMake to manage the compilation process.
    * **Installation (Arch Linux):**
        ```bash
        sudo pacman -S base-devel
        ```
    * **Installation (Debian/Ubuntu):**
        ```bash
        sudo apt update
        sudo apt install make
        ```
    * **Installation (Fedora/CentOS/RHEL):**
        ```bash
        sudo dnf install make
        ```

* **nproc (usually available on Unix-like systems):** This utility aids in parallel compilation, potentially speeding up the build process.

### 2. Building Delirium

Follow these steps to build the Delirium interpreter:

1.  **Acquire the Source Code:** Ensure you have the Delirium source code. The `example` directory contains demo source code for testing after the build.

2.  **Navigate to the Scripts Folder:** Open your terminal and navigate to the `scripts` folder located in the root of your Delirium project directory.
    ```bash
    cd scripts
    ```

3.  **Make the `build.sh` Script Executable:** If a `build.sh` script exists in the project root, grant it execution permissions.
    ```bash
    chmod +x build.sh
    ```

4.  **Execute the `build.sh` Script:** Run the script from your terminal.
    ```bash
    ./build.sh
    ```

5.  **Locating the Resulting Artifact:** Upon successful completion of the build process, the Delirium interpreter executable will be located within the `build` directory. It should be named `delirium`.

### 3. Running Delirium Programs

Once the build is complete, you can execute Delirium programs using the interpreter. Assuming your Delirium source code files have a `.del` extension, you can run them as follows:

```bash
cd build
./delirium <path_to_your_program.del>  // Example: ./delirum ./example/expression.del
```

## Running Tests

This section outlines how to build and run the unit tests for the Delirium programming language.

### 1. Software Requirements

Ensure that the software requirements listed in the [Development Setup](#development-setup) section are installed on your system. Specifically, you will need:

* CMake
* A C++ compiler (Clang++ is recommended)
* Make

### 2. Test Setup and Execution

Follow these steps to build and run the Delirium unit tests:

1.  **Navigate to the Scripts Folder:** Open your terminal and navigate to the `scripts` folder located in the root of your Delirium project directory.
    ```bash
    cd scripts
    ```

2.  **Make the `run_tests.sh` Script Executable:** Grant execute permissions to the `run_tests.sh` script.
    ```bash
    chmod +x run_tests.sh
    ```

3.  **Execute the `run_tests.sh` Script:** Run the script from your terminal while being in the root directory of your Delirium project.
    ```bash
    ./run_tests.sh
    ```


### **Conclusion:**

Delirium is a **betrayal in the form of a programming language**. It ensures that every mistake is punished not with an error message, but with **permanent corruption of the source code itself**. The only way to avoid disaster is **to never make a mistake**—or to **accept madness as part of the development process (which it is)**.

In Delirium, debugging is not a challenge—it is a **trap**.