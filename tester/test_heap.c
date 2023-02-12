#include <assert.h>

void test_down_heap(void) {
    // Create a heap with three elements, in unsorted order
    heap_size = 3;
    heap[0].timestamp = 2;
    heap[1].timestamp = 1;
    heap[2].timestamp = 3;

    // Heapify the heap
    assert(down_heap(0) == EXIT_SUCCESS);

    // Check that the heap is now in sorted order
    assert(heap[0].timestamp == 1);
    assert(heap[1].timestamp == 2);
    assert(heap[2].timestamp == 3);

    // Create a heap with two elements, in unsorted order
    heap_size = 2;
    heap[0].timestamp = 1;
    heap[1].timestamp = 2;

    // Heapify the heap
    assert(down_heap(0) == EXIT_SUCCESS);

    // Check that the heap is still in sorted order
    assert(heap[0].timestamp == 1);
    assert(heap[1].timestamp == 2);

    // Create a heap with one element
    heap_size = 1;
    heap[0].timestamp = 1;

    // Heapify the heap
    assert(down_heap(0) == EXIT_SUCCESS);

    // Check that the heap is still in sorted order (i.e., unchanged)
    assert(heap[0].timestamp == 1);

    // Create an empty heap
    heap_size = 0;

    // Heapify the heap (which should be a no-op)
    assert(down_heap(0) == EXIT_SUCCESS);

    // Check that the heap is still empty
    assert(heap_size == 0);
}

#include <assert.h>

void test_swap(void) {
    // Create two child_process objects with different values
    child_process left = { .timestamp = 1 };
    child_process right = { .timestamp = 2 };

    // Swap the objects
    swap(&left, &right);

    // Check that the values have been swapped correctly
    assert(left.timestamp == 2);
    assert(right.timestamp == 1);

    // Create two more child_process objects with different values
    child_process a = { .timestamp = 3 };
    child_process b = { .timestamp = 4 };

    // Swap the objects
    swap(&a, &b);

    // Check that the values have been swapped correctly
    assert(a.timestamp == 4);
    assert(b.timestamp == 3);

    // Create two child_process objects with the same value
    child_process x = { .timestamp = 5 };
    child_process y = { .timestamp = 5 };

    // Swap the objects
    swap(&x, &y);

    // Check that the values have been swapped correctly
    assert(x.timestamp == 5);
    assert(y.timestamp == 5);
}

