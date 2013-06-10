Adam's Design Thoughts
----------------------

My initial idea of how the program should be organized is something like this:

1. The lexer takes source code and produces a list of tokens.
2. The parser takes a Context Free Grammar that describes the PL/0 language and the list of lexemes from the lexer, and produces a parse tree. The parser can probably detect most/all of the syntax errors.
3. The code generator traverses the parse tree and outputs the assembly code/opcodes.
4. Either a) write an assembler that translates text labels in the code into code addresses (for jumps commands), as well as convert the opcode names into numbers (like "lit" into 1 and "sto" into 3) or b) have the code generator handle that. It might be nice to have an assembler just so the output looks nicer and is easier to understand/debug, but it might be easier to just handle that in the code generator.

### Full compilation example
For example, if this is the **source code**:
```
int x;
begin
    x := 1;
    write x;
end.
```

Then the **tokens** would be:
```
intsym identsym x semicolonsym
beginsym
identsym x becomesym numbersym 1 semicolonsym
writesym identsym x semicolonsym
endsym periodsym
```

If the **Context Free Grammar** looks like:
```
program = block periodsym
block = var-declaration statement
var-declaration = intsym identifiers semicolonsym | nothing
identifiers = identsym | identsym commasym identifiers
statement = begin-block | assignment | write-statement | nothing
statements = statement | statement semicolonsym statements
begin-block = beginsym statements endsym
assignment = identsym becomesym numbersym
write-statement = writesym identsym
```

Then the parser might generate a **parse tree** that looks like this:
```
program
    block
        var-declaration
            identifiers
                identifier = x
        statement
            begin-block
                statements
                    statement
                        assignment
                            identifier = x
                            number = 1
                    statements
                        statement
                            write-statement
                                identifier = x
```

If the parser can't find a match between the grammar and the tokens, then it should be able to output something like "Error: expected identifier but found '1var'" (if somebody tried to use the name '1var' as an identifier) or "Expected expression but found '...'".

Note: the CFG above doesn't have any terminals, but all of the lexer symbols (e.g. periodsym, identsym, semicolonsym, etc.) act like terminals. Most of the terminals aren't included in the parse tree because they don't contain any useful information, except for identsym and numbersym, because they do contain important information. All identsym's become a "identifier" in the parse tree and numbersym's become "number"s.

Finally, the **code generator** would somehow walk through the parse tree and output code that looks like this:
```
inc 0, 1    # Make space for int x.
lit 0, 1    # Put 1 on stack.
sto 0, 0    # Store the 1 in x.
lod 0, 0    # Load x on stack (because write gets rid of the top stack element).
sio 0, 1    # Output the top stack element (the value of x).
opr 0, 0    # Return. (Side note: maybe we should add a dedicated "halt"
                       instruction for halting the execution of the entire
                       program. Right now, the RET operation is used for both
                       returning from functions and halting the VM.)
```

As far as how to implement the code generator, I'm thinking that there might be a function that handles each type of node in the parse tree. For example, the function for "program" might call generate(this.block), which would call generate(this.var-declaration) and generate(this.statement), and eventually it would get to a leaf node which would actually generate code, like generate(assignment), which would generate a pair of lit and sto instructions.

I'm not sure how exactly the code generator would handle references to variables and procedures, i.e. how to know which stack address a particular variable is stored at and how to know which code address a particular procedure is stored at.

### Other thoughts
Also, instead of having both a lexer and parser, maybe we could combine them into one. Then, the Context Free Grammar might look something like:
```
program = block "."
block = var-declaration statement
var-declaration = "int" identifiers ";" | ""
identifier = "[a-zA-Z]\w*\b"   # \w matches alphanumerics and underscore, and \b
                               # matches a word boundary, without actually including
                               # any text (matches the empty string but only along a word boundary).
identifiers = identifier | identifier "," identifiers
statement = begin-block | assignment | write-statement | ""
statements = statement | statement ";" statements
begin-block = "begin" statements "end"
assignment = identifier ":=" number
number = "[0-9]+"
write-statement = "write" identifier
```

This grammar doesn't have whitespaces in it, which is kind of a problem, but there might be a nice way of fixing that. Or we could just add \s* and \s+ where necessary, but that could get confusing. Maybe it's better to just use a lexer since we already have two.


