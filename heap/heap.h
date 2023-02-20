#include <stdio.h>

#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <sys/types.h>
#include <sys/wait.h>
# ifndef  LINESIZE
# include "../constants/constants.h"
# endif


/**
 * @brief Swaps the values of two child_process objects.
 * 
 * @param left Pointer to the first child_process object.
 * @param right Pointer to the second child_process object.
 */
void swap(ProgArgs* left, ProgArgs* right);

/**
 * Move a process up in the heap to maintain the min-heap property.
 *
 * @param index The index of the process to move up in the heap.
*/
int up_heap(int index, ProgArgs* heap[NUMCHILDREN]);


/**
 * @brief Heapify the heap by pushing down the element at the given index.
 *
 * @param index The index of the element to push down in the heap.
 * @return EXIT_SUCCESS if the heap was successfully heapified; otherwise, return EXIT_FAILURE.
 */
int down_heap(int index, ProgArgs* heap[NUMCHILDREN]);


/**
 * Adds a child process to the heap.
 *
 * @param pid The PID of the child process to add.
 * @return 0 if the process was added successfully, or -1 if the heap is full.
 */
int add_to_heap(int index, ProgArgs* heap[NUMCHILDREN]);


/**
 * Searches for a process with a given PID in the heap.
 *
 * @param pid The process ID to search for
 * @return The index of the matching process in the heap, or -1 if not found.
 */
int find_in_heap(int index, ProgArgs* heap[NUMCHILDREN]);


/**
 * Removes the process at index i from the heap.
 * @param i The index of the process to be removed.
 * @return EXIT_SUCCESS if successful, EXIT_FAILURE otherwise.
 */
int remove_from_heap(int index, ProgArgs* heap[NUMCHILDREN]);



