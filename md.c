// Showing how to dynamically allocate multi-dimensional arrays.

#include <stdio.h>
#include <stdlib.h>

int main()
{
#ifdef NEVER
    // You remember this palette array.  It has room for a fixed
    // number of colors.
    short palette[][3] = {
        {1000, 0, 0},
        {750, 1000, 500},
        {250, 750, 0},
        {1000, 0, 750},
        {500, 500, 500},
        {0, 750, 1000}};
#endif

    // Remember, with this declaration, we can make a pointer that points
    // to an array like palette.
    short(*pal)[3];

    // Instead of just having it point to the palette array, we can
    // have it point to a dynamically allocated chunk of memory, with
    // room for as many colors as we need.
    int N = 1000;
    pal = (short(*)[3])malloc(N * sizeof(*pal));

    // There are some goofy things going on above.  Notice, I'm using
    // the dereference operator to figure out how much memory I need for
    // a single color of the palette.  Since pal is undefined, this would be
    // a bad idea, but it's fine inside sizeof, it doesn't evaluate sizeof,
    // it just computes the size it would have if I did dereference it.

    // Also, notice in the type cast above, I have something that looks like a
    // variable declaration, with the variable name omitted.  That's
    // what you'd normally expect; a type looks like a variable
    // declaration without the variable name.  Here, though, that makes
    // the parentheses around the star look like they aren't doing
    // anything, since they don't have a variable name to group the star
    // with.  However, they are still needed and they're doing the same
    // job, they are saying this type is a pointer to an array, not an
    // array of pointers.

    // OK, now we have an array with room for 1000 colors, lets fill it with some
    // random ones.  See, it works just like a 2D array we allocated on the
    // stack or statically.
    for (int i = 0; i < N; i++)
    {
        pal[i][0] = rand() % 1001 + 1;
        pal[i][1] = rand() % 1001 + 1;
        pal[i][2] = rand() % 1001 + 1;
    }

    // C99 supports variable-sized arrays, and this also applies to
    // pointers to variable-sized arrays.  Here, we're dynamically
    // allocating an array of R rows and C columns.  Notice that the
    // type of the pointer for this array is "pointer to a C-element
    // array of ints.
    int R = 500;
    int C = 400;
    int(*a)[C];
    a = (int(*)[C])malloc(R * C * sizeof(int));

    // Show how to use this array, just like any other array.
    for (int i = 0; i < R; i++)
        for (int j = 0; j < C; j++)
            a[i][j] = i * j;

    // Often, we want to represent a 2D array as an array of pointers,
    // one for each row.  This is a more flexible representation, since
    // we can add rows by growing the outer array, or independently
    // lengthen any row by allocating a longer array for it.  It's a
    // little more work to build a structure like this using malloc, but
    // the added flexibility may be worth it.

    // Here, we're building a R X C table like a above, but implemented
    // as an array of row pointers.  I need an array of R pointers to
    // int.  I'll hold this array with a pointer to its first element (a
    // pointer to int), so the type will need to be pointer to pointer
    // to int.
    int **b;
    b = (int **)malloc(R * sizeof(int *));

    // Now, we need to allocate space for each row.
    for (int i = 0; i < R; i++)
        b[i] = (int *)malloc(C * sizeof(int));

    // Just like in the previous example, write some stuff into the array.
    for (int i = 0; i < R; i++)
        for (int j = 0; j < C; j++)
            b[i][j] = i * j;

    // Now, let's free the memory for each structure.  First the pallete.  It's
    // one big block of memory, so it's easy to free.
    free(pal);

    // The R X C two dimensional array is also one big block, so we just
    // need one call to free.
    free(a);

    // For the array of row pointers, first we need to free the memory
    // for each row.  Then, we free the memory for the outer array, the
    // one containing pointers to each row.
    for (int i = 0; i < R; i++)
        free(b[i]);
    free(b);
}