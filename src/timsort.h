/*
 * =====================================================================================
 *
 *       Filename:  timsort.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  06/27/2011 02:33:56 PM
 *       Revision:  10
 *       Compiler:  g++ 4.4
 *
 *         Author:  YOUR NAME (), 
 *        Company:  
 *
 * =====================================================================================
 */
#ifndef TIMSORT_H
#define TIMSORT_H

#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <cstdint>

#include <algorithm>

/* timsort implementation, based on timsort.txt */
using std::min;
using std::max;

//Helper Functions and Structs for timsort:
#ifndef CLZ
#ifdef __GNUC__
#define CLZ __builtin_clzll
#else
// adapted from Hacker's Delight
int
clzll(uint64_t x) {
    int n;

    if (x == 0) return(64);
    n = 0;
    if (x <= 0x00000000FFFFFFFFL) {n = n + 32; x = x << 32;}
    if (x <= 0x0000FFFFFFFFFFFFL) {n = n + 16; x = x << 16;}
    if (x <= 0x00FFFFFFFFFFFFFFL) {n = n + 8; x = x << 8;}
    if (x <= 0x0FFFFFFFFFFFFFFFL) {n = n + 4; x = x << 4;}
    if (x <= 0x3FFFFFFFFFFFFFFFL) {n = n + 2; x = x << 2;}
    if (x <= 0x7FFFFFFFFFFFFFFFL) {n = n + 1;}
    return n;
}
#define CLZ clzll
#endif
#endif

struct tim_sort_run {
    int64_t start;
    int64_t length;
};

template <typename T>
struct temp_storage {
    size_t alloc;
    T *storage;
};


template<typename T>
static inline void
swap_elements(T& x, T& y)
{
    T temp = x;
    x = y;
    y = x;
}

template<typename T>
static inline int
sort_cmp(const T& x, const T& y)
{
    return ((x < y) ?  -1 : ((x == y) ? 0 : 1));
}

/* Function used to do a binary search for binary insertion sort */
template<typename T>
static inline int64_t
binary_insertion_find(T *dst, const T x, const size_t size)
{
    int64_t l, c, r;
    l = 0;
    r = size - 1;
    c = r >> 1;
    T lx, cx, rx;
    lx = dst[l];

    /* check for beginning conditions */
    if (sort_cmp(x, lx) < 0)
        return 0;
    else if (sort_cmp(x, lx) == 0)
    {
        int64_t i = 1;
        while (sort_cmp(x, dst[i]) == 0) i++;
        return i;
    }

    rx = dst[r];
    // guaranteed not to be >= rx
    cx = dst[c];
    while (1)
    {
        const int val = sort_cmp(x, cx);
        if (val < 0)
        {
            if (c - l <= 1) return c;
            r = c;
            rx = cx;
        }
        else if (val > 0)
        {
            if (r - c <= 1) return c + 1;
            l = c;
            lx = cx;
        }
        else
        {
            do
            {
                cx = dst[++c];
            } while (sort_cmp(x, cx) == 0);
            return c;
        }
        c = l + ((r - l) >> 1);
        cx = dst[c];
    }
}

/* Binary insertion sort, but knowing that the first "start" entries are sorted.  Used in timsort. */
template<typename T>
static inline void 
binary_insertion_sort_start(T *dst, const size_t start, const size_t size)
{
    int64_t i;
    for (i = start; i < size; i++)
    {
        int64_t j;
        /* If this entry is already correct, just move along */
        if (sort_cmp(dst[i - 1], dst[i]) <= 0)
            continue;

        /* Else we need to find the right place, shift everything over, and squeeze in */
        T x = dst[i];
        int64_t location = binary_insertion_find<T>(dst, x, i);

        for (j = i - 1; j >= location; j--)
            dst[j + 1] = dst[j];

        dst[location] = x;
    }
}

/* Binary insertion sort */
template<typename T>
void 
binary_insertion_sort(T *dst, const size_t size)
{
    binary_insertion_sort_start(dst, 1, size);
}

template<typename T>
static inline void
reverse_elements(T *dst, int64_t start, int64_t end)
{
    while (1)
    {
        if (start >= end) return;
        swap_elements(dst[start], dst[end]);
        start++;
        end--;
    }
}

template<typename T>
static inline int64_t
count_run(T *dst, const int64_t start, const size_t size)
{
    if (size - start == 1) return 1;
    if (start >= size - 2)
    {
        if (sort_cmp(dst[size - 2], dst[size - 1]) > 0)
            swap_elements(dst[size - 2], dst[size - 1]);
        return 2;
    }

    int64_t curr = start + 2;

    if (sort_cmp(dst[start], dst[start + 1]) <= 0)
    {
        // increasing run
        while (1)
        {
            if (curr == size - 1) break;
            if (sort_cmp(dst[curr - 1], dst[curr]) > 0) break;
            curr++;
        }
        return curr - start;
    }
    else
    {
        // decreasing run
        while (1)
        {
            if (curr == size - 1) break;
            if (sort_cmp(dst[curr - 1], dst[curr]) <= 0) break;
            curr++;
        }
        // reverse in-place
        reverse_elements(dst, start, curr - 1);
        return curr - start;
    }
}

static inline int
compute_minrun(const uint64_t size)
{
    const int top_bit = 64 - CLZ(size);
    const int shift = max(top_bit, 6) - 6;
    const int minrun = size >> shift;
    const uint64_t mask = (1ULL << shift) - 1;
    if (mask & size) return minrun + 1;
    return minrun;
}

#define PUSH_NEXT() do {\
    len = count_run(dst, curr, size);\
    run = minrun;\
    if (run < minrun) run = minrun;\
    if (run > size - curr) run = size - curr;\
    if (run > len)\
    {\
        binary_insertion_sort_start(&dst[curr], len, run);\
        len = run;\
    }\
    run_stack[stack_curr++] = (tim_sort_run) {curr, len};\
    curr += len;\
    if (curr == size)\
    {\
        /* finish up */ \
        while (stack_curr > 1) \
        { \
            tim_sort_merge(dst, run_stack, stack_curr, store); \
            run_stack[stack_curr - 2].length += run_stack[stack_curr - 1].length; \
            stack_curr--; \
        } \
        if (store->storage != NULL)\
        {\
            free(store->storage);\
            store->storage = NULL;\
        }\
        return;\
    }\
}\
while (0)

static inline int
check_invariant(tim_sort_run *stack, const int stack_curr)
{
    if (stack_curr < 2) return 1;
    if (stack_curr == 2)
    {
        const int64_t A = stack[stack_curr - 2].length;
        const int64_t B = stack[stack_curr - 1].length;
        if (A <= B) return 0;
        return 1;
    }
    const int64_t A = stack[stack_curr - 3].length;
    const int64_t B = stack[stack_curr - 2].length;
    const int64_t C = stack[stack_curr - 1].length;
    if ((A <= B + C) || (B <= C)) return 0;
    return 1;
}

template <typename T>
static inline void
tim_sort_resize(temp_storage<T> *store, const size_t new_size)
{
    if (store->alloc < new_size)
    {
        T *tempstore = (T *)realloc(store->storage, new_size * sizeof(T));
        if (tempstore == NULL)
        {
            fprintf(stderr, "Error allocating temporary storage for tim sort: need %u bytes", sizeof(T) * new_size);
            exit(1);
        }
        store->storage = tempstore;
        store->alloc = new_size;
    }
}

template <typename T>
static inline void
tim_sort_merge(T *dst, const tim_sort_run *stack, const int stack_curr, temp_storage<T> *store)
{
    const int64_t A = stack[stack_curr - 2].length;
    const int64_t B = stack[stack_curr - 1].length;
    const int64_t curr = stack[stack_curr - 2].start;

    tim_sort_resize(store, min(A, B));
    T *storage = store->storage;

    int64_t i, j, k;

    // left merge
    if (A < B)
    {
        memcpy(storage, &dst[curr], A * sizeof(T));
        i = 0;
        j = curr + A;

        for (k = curr; k < curr + A + B; k++)
        {
            if ((i < A) && (j < curr + A + B))
            {
                if (sort_cmp(storage[i], dst[j]) <= 0)
                    dst[k] = storage[i++];
                else
                    dst[k] = dst[j++];
            }
            else if (i < A)
            {
                dst[k] = storage[i++];
            }
            else
                dst[k] = dst[j++];
        }
    }
    // right merge
    else
    {
        memcpy(storage, &dst[curr + A], B * sizeof(T));
        i = B - 1;
        j = curr + A - 1;

        for (k = curr + A + B - 1; k >= curr; k--)
        {
            if ((i >= 0) && (j >= curr))
            {
                if (sort_cmp(dst[j], storage[i]) > 0)
                    dst[k] = dst[j--];
                else
                    dst[k] = storage[i--];
            }
            else if (i >= 0)
                dst[k] = storage[i--];
            else
                dst[k] = dst[j--];
        }
    }
}

template <typename T>
static inline int
tim_sort_collapse(T *dst, tim_sort_run *stack, int stack_curr, temp_storage<T> *store, const size_t size)
{
    while (1)
    {
        // if the stack only has one thing on it, we are done with the collapse
        if (stack_curr <= 1) break;
        // if this is the last merge, just do it
        if ((stack_curr == 2) && (stack[0].length + stack[1].length == size))
        {
            tim_sort_merge(dst, stack, stack_curr, store);
            stack[0].length += stack[1].length;
            stack_curr--;
            break;
        }
        // check if the invariant is off for a stack of 2 elements
        else if ((stack_curr == 2) && (stack[0].length <= stack[1].length))
        {
            tim_sort_merge(dst, stack, stack_curr, store);
            stack[0].length += stack[1].length;
            stack_curr--;
            break;
        }
        else if (stack_curr == 2)
            break;

        const int64_t A = stack[stack_curr - 3].length;
        const int64_t B = stack[stack_curr - 2].length;
        const int64_t C = stack[stack_curr - 1].length;

        // check first invariant
        if (A <= B + C)
        {
            if (A < C)
            {
                tim_sort_merge(dst, stack, stack_curr - 1, store);
                stack[stack_curr - 3].length += stack[stack_curr - 2].length;
                stack[stack_curr - 2] = stack[stack_curr - 1];
                stack_curr--;
            }
            else
            {
                tim_sort_merge(dst, stack, stack_curr, store);
                stack[stack_curr - 2].length += stack[stack_curr - 1].length;
                stack_curr--;
            }
        }
        // check second invariant
        else if (B <= C)
        {
            tim_sort_merge(dst, stack, stack_curr, store);
            stack[stack_curr - 2].length += stack[stack_curr - 1].length;
            stack_curr--;
        }
        else
            break;
    }
    return stack_curr;
}

template<typename T>
void
tim_sort(T *dst, const size_t size)
{
    if (size < 64)
    {
        binary_insertion_sort(dst, size);
        return;
    }

    // compute the minimum run length
    const int minrun = compute_minrun(size);

    // temporary storage for merges
    temp_storage <T>_store, *store = &_store;
    store->alloc = 0;
    store->storage = NULL;

    tim_sort_run run_stack[128];
    int stack_curr = 0;
    int64_t len, run;
    int64_t curr = 0;

    PUSH_NEXT();
    PUSH_NEXT();
    PUSH_NEXT();

    while (1)
    {
        if (!check_invariant(run_stack, stack_curr))
        {
            stack_curr = tim_sort_collapse(dst, run_stack, stack_curr, store, size);
            continue;
        }
        PUSH_NEXT();
    }
}

#endif
