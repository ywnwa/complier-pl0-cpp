#ifndef VECTOR_H
#define VECTOR_H

// A vector is an array that automatically expands as you add items to it. Use
// the functions/macros below to operate on vectors.
//
// Most of the functions and macros are fairly self explanatory, except for
// push/set.  Because vectors can hold types of arbitrary sizes, you have to
// pass a pointer to push/set. Because of this, you can't pass literal values
// to them, so you can't do something like:
//
// struct vector *vector = makeVector(int);
// push(vector, 100);
//
// Instead, you have to do something like this:
//
// int x = 100;
// push(vector, x);
//
// You can also use the pushLiteral macro, which does the same thing but looks
// a little bit nicer.
//
// pushLiteral(vector, int, 100);
//
// More examples
// -------------
//
// struct vector *stuff = makeVector(char*);
// pushLiteral(stuff, char*, "a");   // stuff = ["a"]
// stuff vector *stuff2 = makeVector(char*);
// char *first = get(char*, stuff, 0);
// push(stuff2, first);   // stuff2 = ["a"]
// pushLiteral(stuff2, char*, "b");   // stuff2 = ["a", "b"]
// vector_concat(stuff, stuff2);   // stuff = ["a", "a", "b"]

struct vector
{
   void *items;
   int itemSize;   // Item size in bytes.
   int length;     // Maximum index used so far.
   int capacity;   // Number of spaces allocated so far.
};

// Number of spaces to initialize when calling vector_init.
#define INITIAL_CAPACITY 20
// Number of spaces to add when capacity is exceeded.
#define CAPACITY_STEP 20

// Use these macros in preference to the functions below so that you don't have
// to reference/dereference anything when using vector_get and
// vector_set/vector_push. They also have slightly nicer names, and let you
// pass a type to makeVector, instead of passing an int to vector_init.
#define makeVector(type) vector_init(sizeof (type))
#define push(vector, item) vector_push(vector, (void*)(&item))
// pushLiteral puts the local variable "literal" in it's own scope so that it
// can't conflict with any other variables.
//#define pushLiteral(vector, type, value) { type literal = value; push(vector, literal); }
// Need to use variable arguments to pass the value to pushLiteral, because the
// value could be a struct literal, and have commands in it. For example:
// pushLiteral(vector, struct something, {"string", NULL}).
#define pushLiteral(vector, type, ...) { type literal = __VA_ARGS__; push(vector, literal); }
#define set(vector, index, item) vector_set(vector, index, (void*)(&item))
#define get(type, vector, index) (*(type*)vector_get(vector, index))
#define freeVector(vector) vector_free(vector)

struct vector* vector_init(int itemSize);
struct vector* vector_copy(struct vector *vector);
void vector_push(struct vector *vector, void *item);
void *vector_get(struct vector *vector, int index);
void vector_set(struct vector *vector, int index, void *item);
void vector_resize(struct vector *vector, int newCapacity);
struct vector *vector_concat(struct vector *toVector, struct vector *fromVector);
void vector_free(struct vector *vector);

// Experimental:
int vector_find(struct vector *vector, void *value);
#define forVector(vector, indexVariable, type, itemVariable, ...) {\
    int indexVariable;\
    for (indexVariable = 0; indexVariable < vector->length; indexVariable++) {\
        type itemVariable = get(type, vector, indexVariable);\
        __VA_ARGS__\
    }\
}
#define make(type) (type*)malloc(sizeof (type))

#endif
