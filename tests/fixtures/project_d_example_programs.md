# Project D — Example Programs

**Course:** Compilers Construction  
**Project:** D — Dynamic Language  
**Team:** Vibecoders  
**Target platform:** Interpreter  
**Implementation language:** C++  
**Parser:** Hand-written parser

## Goal

This document contains example programs for **Project D**.

Examples **1–10** are the examples used in the team presentation.  
Examples **11–15** add more coverage of functions: function declarations through variables, block-bodied functions, anonymous functions, higher-order functions, returned functions, and closures.

## Important language clarifications

- D is **dynamically typed**: a variable may hold values of different types during execution.
- D is intended to be **interpreted**.
- Functions are **values/literals**. They can be assigned to variables, passed as arguments, returned from other functions, and called.
- The language specification does **not** define a separate syntax such as `function add(...)`. A reusable named function is created by assigning a function literal to a variable, for example:

```text
var add := func(a, b) => a + b
```

- Captured variables in lambdas/closures use **reference semantics**.
- A range such as `1..5` is **inclusive on both ends**.

---

# Examples from the presentation

## 1. Variables, Literals, and Output

### Demonstrates

- variable declarations with `var`
- integer, real, boolean, and string literals
- `print`

```text
var integerValue := 10
var realValue := 3.5
var booleanValue := true
var stringValue := "Hello, D!"

print integerValue
print realValue
print booleanValue
print stringValue
```

### Expected output

```text
10
3.5
true
Hello, D!
```

---

## 2. Arithmetic and Implicit Conversion

### Demonstrates

- arithmetic expressions
- integer and real values in the same expression
- implicit conversion from integer to real

```text
var a := 5
var b := 2.5

var sum := a + b
var product := a * b

print sum
print product
```

### Expected output

```text
7.5
12.5
```

When an integer operand is combined with a real operand, D promotes the integer to a real value for the operation.

---

## 3. Dynamic Typing and the `is` Operator

### Demonstrates

- a variable changing its runtime type
- runtime type checking with `is`

```text
var value := 100

print value is int
print value is string

value := "dynamic"

print value is int
print value is string
```

### Expected output

```text
true
false
false
true
```

After reassignment, the same variable changes from an integer value to a string value.

---

## 4. Conditional Statement — `if / else`

### Demonstrates

- relational expressions
- `if`
- `else`
- `end`

```text
var temperature := 18

if temperature >= 20 then
    print "warm"
else
    print "cold"
end
```

### Expected output

```text
cold
```

---

## 5. Infinite Loop, Short `if`, and `exit`

### Demonstrates

- unconditional `loop`
- assignment inside a loop
- short `if Expression => Body`
- `exit`

```text
var i := 0

loop
    i := i + 1
    print i
    if i = 3 => exit
end
```

### Expected output

```text
1
2
3
```

The loop terminates when `i` reaches `3`.

---

## 6. `while` Loop — Accumulation

### Demonstrates

- `while`
- repeated execution
- accumulator variable
- updating a loop variable

```text
var i := 1
var sum := 0

while i <= 5 loop
    sum := sum + i
    i := i + 1
end

print sum
```

### Expected output

```text
15
```

---

## 7. `for` Loop over a Range

### Demonstrates

- `for`
- range expression `..`
- loop variable
- inclusive ranges

```text
var sum := 0

for i in 1..5 loop
    sum := sum + i
end

print sum
```

### Expected output

```text
15
```

The range `1..5` includes both `1` and `5`.

---

## 8. Arrays

### Demonstrates

- array literals
- `for-in` iteration
- array indexing
- assignment to an array element
- sparse integer keys

```text
var values := [10, 20, 30]
var sum := 0

for value in values loop
    sum := sum + value
end

print sum

values[10] := 100
print values[10]
```

### Expected output

```text
60
100
```

The literal `[10, 20, 30]` initially has indices `1`, `2`, and `3`.  
The assignment to index `10` demonstrates that array keys do not need to be consecutive.

---

## 9. Tuples

### Demonstrates

- tuple literals
- named tuple elements
- unnamed tuple elements
- access by name
- one-based positional access

```text
var point := {x := 10, y := 20, "point"}

print point.x
print point.y
print point.3
```

### Expected output

```text
10
20
point
```

---

## 10. Functions, Lambdas, and Captures

### Demonstrates

- functions as values
- lambda-style function syntax
- function calls
- closure/capture of a variable from an enclosing scope
- reference semantics for captured variables

```text
var base := 10
var addBase := func(x) => x + base

print addBase(5)

base := 20

print addBase(5)
```

### Expected output

```text
15
25
```

`addBase` captures the variable `base` by reference. Therefore, changing `base` after the lambda is created affects later calls.

---

# Additional Function Examples

## 11. Reusable Function with a Block Body and `return`

### Demonstrates

- declaring a reusable function by assigning a function literal to a variable
- multiple parameters
- block-bodied function syntax `is ... end`
- local variables inside a function
- `return`

```text
var add := func(a, b) is
    var result := a + b
    return result
end

print add(4, 6)
```

### Expected output

```text
10
```

In D, this is the equivalent of declaring a named function: the variable `add` stores a function value.

---

## 12. Anonymous Function Passed as an Argument

### Demonstrates

- anonymous function literals
- functions as arguments
- higher-order functions
- calling a function received through a parameter

```text
var apply := func(f, value) => f(value)

print apply(func(x) => x * x, 5)
```

### Expected output

```text
25
```

The function `func(x) => x * x` has no variable name of its own; it is created directly inside the call to `apply`.

---

## 13. Function Returning Another Function

### Demonstrates

- functions as return values
- nested function literals
- closure creation
- capturing a parameter from an enclosing function

```text
var makeAdder := func(amount) => func(x) => x + amount

var add10 := makeAdder(10)

print add10(5)
```

### Expected output

```text
15
```

The returned function keeps access to the captured variable `amount` after `makeAdder` has returned.

---

## 14. Functions Stored in an Array

### Demonstrates

- functions as ordinary values
- arrays containing function values
- anonymous functions inside arrays
- calling a function obtained through array indexing

```text
var increment := func(x) => x + 1
var functions := [increment, func(x) => x * 2]

print functions[1](5)
print functions[2](5)
```

### Expected output

```text
6
10
```

This example checks that function values can participate in composite data structures and still be called after being retrieved.

---

## 15. Stateful Closure

### Demonstrates

- nested scopes
- returning a function
- capture by reference
- modification of a captured variable
- closure state surviving between calls

```text
var makeCounter := func(start) is
    var value := start

    return func(step) is
        value := value + step
        return value
    end
end

var counter := makeCounter(10)

print counter(1)
print counter(2)
```

### Expected output

```text
11
13
```

The inner function captures `value` by reference. The first call changes it from `10` to `11`; the second call continues from the same captured variable and changes it to `13`.

---

# Feature Coverage

| Program | Main features |
|---|---|
| 1 | Variables, primitive literals, `print` |
| 2 | Arithmetic, implicit integer-to-real conversion |
| 3 | Dynamic typing, `is` |
| 4 | `if / else / end` |
| 5 | Infinite `loop`, short `if`, `exit` |
| 6 | `while` loop |
| 7 | `for` over an inclusive range |
| 8 | Arrays, indexing, iteration, sparse keys |
| 9 | Tuples, named and positional access |
| 10 | Function values, lambda, closure capture by reference |
| 11 | Function stored in a variable, block body, `return` |
| 12 | Anonymous function, higher-order function, function argument |
| 13 | Returning a function, nested closure |
| 14 | Functions inside arrays, chained reference/call |
| 15 | Stateful closure, mutation of captured state |

# Implementation-Relevant Function Cases

The additional programs give the interpreter several distinct function cases to support:

1. **Expression-bodied function**
   ```text
   func(x) => x + 1
   ```

2. **Block-bodied function**
   ```text
   func(x) is
       return x + 1
   end
   ```

3. **Function stored in a variable**
   ```text
   var f := func(x) => x + 1
   ```

4. **Function passed as an argument**
   ```text
   apply(func(x) => x * x, 5)
   ```

5. **Function returned from another function**
   ```text
   func(amount) => func(x) => x + amount
   ```

6. **Captured variable updated by reference**
   ```text
   value := value + step
   ```

These cases are useful later when implementing the AST, runtime values, environments/scopes, function calls, and closures.