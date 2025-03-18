# Delirium Programming Language Specifications

## Keywords

The Delirium programming language reserves a specific collection of sixteen words, each designated for particular syntactic purposes within the language's structure. These keywords are fundamental components of Delirium's grammar, and their use is strictly defined to ensure proper interpretation and execution of code.

```
and         break       continue        do
else        elseif      false           for
func        if          nil             or
return      true        var             while
```

## Features

### 1. Dynamic Typing

Delirium uses dynamic typing, which means variable types are determined at runtime.

```delirium
var x = 42;     // x is a number
x = "Hello";    // x is now a string
x = true;       // x is now a boolean
```

---

### 2. Automatic Memory Management

Delirium automatically manages memory using a garbage collector, so you don't need to manually handle memory allocation and deallocation.

```delirium
var obj = "A dynamically allocated string";
obj = nil;  // The garbage collector reclaims memory.
```

---

### 3. Data Types

Delirium supports the following data types:

#### **Booleans**

```delirium
var a = true;
var b = false;
```

#### **Numbers**

Delirium uses double-precision floating-point numbers exclusively.

```delirium
var intNum = 1234;
var floatNum = 12.34;
```

#### **Strings**

Strings are enclosed in double quotes.

```delirium
var str1 = "I am a string";
var empty = "";
var numStr = "123"; // This is a string, not a number.
```

#### **Nil**

Represents "no value".

```delirium
var nothing = nil;
```

---

### 4. Expressions

#### **Arithmetic Operators**

`+`, `-`, `*`, `/`, `%`. Note that the `-` operator can function both as an infix and prefix operator (e.g., for negation).

```delirium
var sum = 5 + 3;
var difference = 10 - 4;
var product = 6 * 7;
var quotient = 8 / 2;
var remainder = 10 % 3;
```

#### **Comparison Operators**

`<`, `<=`, `>`, `>=`

```delirium
var isLesser = 2 < 6;
var isLessOrEqual = 5 <= 5;
var isGreater = 10 > 5;
var isGreaterOrEqual = 7 >= 0;
```

#### **Equality Operators**

`==`, `!=`

```delirium
var isEqual = (10 == 10);
var isNotEqual = (10 != 5);
```

#### **Logical Operators**

`!`, `and`, `or`

```delirium
var result = true and false;  // false
var alternative = true or false;  // true
var negation = !true;  // false
```

#### **Bitwise Operators**

`&`, `|`, `<<`, `>>`, `~`, `^`

```delirium
var bitwiseAnd = 5 & 3;  // 1
var bitwiseOr = 5 | 3;   // 7
var bitwiseXor = 5 ^ 3;  // 6
var leftShift = 5 << 1;  // 10
var rightShift = 5 >> 1; // 2
var bitwiseNot = ~5;     // -6
```

#### **Precedence & Grouping**

Operators have the same precedence and associativity as C. Parentheses (`()`) are used to group expressions.

```delirium
var result = (5 + 3) * 2;  // 16
```

---

### 5. Statements

Each statement ends with a semicolon.

```delirium
var x = 10;
x = x + 5;  // Ends with a semicolon
```

#### **Blocks**

To group multiple statements in a place where a single one is expected, wrap them in a block.

```delirium
{
    var a = 10;
    var b = 20;
    print a + b;
}
```

---

### 6. Variables

#### **Declaration**

Variables are declared using the `var` keyword.

```delirium
var name = "Alice";
```

#### **Default Value**

If you omit the initializer, the value defaults to `nil`.

```delirium
var age;  // Defaults to nil
```

#### **Scoping**

The scoping behavior follows the same rules as C.

```delirium
{
    var localVar = "Inside block";
}
// localVar is not accessible here
```

---

### 7. Control Flow

#### **If Statement**

Executes one of two statements based on a condition.

```delirium
if (x > 0) {
    print "Positive";
} elseif(x == 0) {
    print "Zero";
} else {
    print "Negative";
}
```

#### **While Loop**

Repeatedly executes the body as long as the condition is true.

```delirium
var i = 0;
while (i < 5) {
    print i;
    i = i + 1;
}
```

#### **For Loop**

Functions as in C, with a specific iteration pattern.

```delirium
for (var i = 0; i < 5; i = i + 1) {
    print i;
}
```

#### **Do-While Loop**

Executes the body of the loop at least once before checking the condition.

```delirium
var i = 0;
do {
    print i;
    i = i + 1;
} while (i < 5);
```

---

### 8. Functions

#### **Function Calls**

Function calls are written similarly to C (e.g., `fibonacci(10)`). You can also call a function without arguments (parentheses are required even if empty).

```delirium
func greet(name) {
    print "Hello, " + name;
}
greet("Alice");
```

#### **Function Declaration**

Define functions using the `func` keyword. The function body is a block.

```delirium
func add(a, b) {
    return a + b;
}
var result = add(2, 3);  // 5
```

#### **Return Statement**

You can return a value using the `return` keyword. If a function completes without hitting a `return`, it implicitly returns `nil`.

```delirium
func square(n) {
    return n * n;
}
var result = square(4);  // 16
```

---

### 9. Closures

Functions are **first-class** in Delirium, meaning they can be assigned to variables, passed around, and referenced.

```delirium
func makeCounter() {
    var count = 0;
    return func() {
        count = count + 1;
        return count;
    };
}

var counter = makeCounter();
print counter(); // 1
print counter(); // 2
```
