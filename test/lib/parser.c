#include "test/lib/parser.h"
#include <string.h>
#include <assert.h>
#include <stdio.h>

struct parseTree generateParseTree(char *form) {
    if (form[0] == '(') {
        struct vector *items = formToVector(form);

        char *name = get(char*, items, 0);

        struct vector *children = makeVector(struct parseTree);
        int i;
        for (i = 1; i < items->length; i++) {
            char *item = get(char*, items, i);
            struct parseTree childTree = generateParseTree(item);
            push(children, childTree);
        }

        return (struct parseTree){name, children};
    } else {
        return (struct parseTree){form, NULL};
    }
}

struct vector *formToVector(char *form) {
    char *formStart = strchr(form, '(');
    char *formEnd = strrchr(form, ')');

    char *skipWhitespace(char *start) {
        int numWhitespace = strspn(start, " \r\n\t");
        return start + numWhitespace;
    }
    char *skipNonWhitespace(char *start) {
        int numNonWhitespace = strcspn(start, " \r\n\t");
        return start + numNonWhitespace;
    }
    char *skipForm(char *start) {
        char *c = start + 1;
        int level = 1;
        while (level > 0) {
            if (*c == '(')
                level += 1;
            else if (*c == ')')
                level -= 1;
            c += 1;
        }

        return c;
    }

    struct vector *items = makeVector(char*);
    char *c = formStart + 1;
    while (c < formEnd) {

        char *start = skipWhitespace(c);

        char *end = NULL;
        if (*start == '(')
            end = skipForm(start);
        else
            end = skipNonWhitespace(start);
        if (end > formEnd)
            end = formEnd;

        int length = end - start;
        char *item = strndup(start, length);
        push(items, item);

        c = end;
    }

    return items;
}

int parseTreesEqual(struct parseTree x, struct parseTree y) {
    assert(x.name != NULL && y.name != NULL);

    // Return false if they don't have the same name.
    if (strcmp(x.name, y.name) != 0)
        return 0;

    // Return true if they have the same name and neither have children.
    if (x.children == NULL && y.children == NULL)
        return 1;

    // Return false if only one of them has children.
    if (x.children == NULL || y.children == NULL)
        return 0;

    // Return false if one has more children than the other.
    if (x.children->length != y.children->length)
        return 0;

    int length = x.children->length;
    int i;
    for (i = 0; i < length; i++) {
        struct parseTree xChild = get(struct parseTree, x.children, i);
        struct parseTree yChild = get(struct parseTree, y.children, i);

        // Return false if one of their children isn't equal.
        if (!parseTreesEqual(xChild, yChild))
            return 0;
    }

    // If all of the children are equal, return true.
    return 1;
}

// Returns a new tree that's the same as the given tree but with all of the
// leaf nodes (the nodes that don't have any children) removed.
struct parseTree removeLeaves(struct parseTree tree) {
    struct parseTree newTree;

    // Returns a vector all of the trees that have children (i.e. that are
    // parents) in the given vector of trees.
    struct vector *getParents(struct vector *trees) {
        struct vector *parents = makeVector(struct parseTree);
        int i;
        for (i = 0; i < trees->length; i++) {
            struct parseTree tree = get(struct parseTree, trees, i);
            if (tree.children != NULL && tree.children->length > 0)
                push(parents, tree);
        }
        return parents;
    }

    // Get all of the children that aren't leaf nodes.
    struct vector *children = getParents(tree.children);

    // Call removeLeaves on each of the non-leaf children.
    int i;
    for (i = 0; i < children->length; i++) {
        struct parseTree child = get(struct parseTree, children, i);
        child = removeLeaves(child);
        set(children, i, child);
    }

    // Return the new set of children.
    tree.children = children;
    return tree;
}

int parseTreesSimilar(struct parseTree x, struct parseTree y) {
    return parseTreesEqual(removeLeaves(x), removeLeaves(y));
}

void printParseTree(struct parseTree root) {
    void print(struct parseTree tree, int level) {
        void printIndent() {
            int i;
            for (i = 0; i < 4 * level; i++)
                printf(" ");
        }

        printIndent();
        printf("%s\n", tree.name);

        if (tree.children != NULL) {
            int i;
            for (i = 0; i < tree.children->length; i++) {
                struct parseTree child = get(struct parseTree, tree.children, i);
                print(child, level + 1);
            }
        }
    }

    print(root, 0);
}

