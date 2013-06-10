Style-Guide
============

### Indentation
4 spaces shall be used as an indentation
ex.
### Placing of Braces
#### Functions
Braces shall be on the same line as the function declaration.

```C
int function(x) {
    body of function
}
```

#### If statements
Use braces for all if statements and place on same line as the condition(same as functions).

```C
if (x == y) {
    body of if statement 
}
```

Keep `else if` and `else` on the same line as the closing brace.

```C
if(x == y) {
    body of if statement 
} else if(x > y) {
    body of if statement 
} else {
    body of if statement 
}
```

### Macros
Macros are cool. Use them, but if you don't need to.

```C
#define ABS(a) (((a)<0) ? -(a) : (a))
```
