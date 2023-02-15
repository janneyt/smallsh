# include <stdio.h>
# include <stdlib.h>
# include <unistd.h>
# include <signal.h>
# include <time.h>
# include <sys/types.h>
# include <sys/wait.h>
# ifndef  LINESIZE
# include "../constants/constants.h"
# endif


/**
 * @brief Swaps the values of two child_process objects.
 * 
 * @param left Pointer to the first child_process object.
 * @param right Pointer to the second child_process object.
 */
int swap(ProgArgs *left, ProgArgs *right){
    if(!left || !right){
	return EXIT_FAILURE;
    }
    ProgArgs tmp = *left;
    *left = *right;
    *right = tmp;
    return EXIT_SUCCESS;
}

/**
 * Move a process up in the heap to maintain the min-heap property.
 *
 * @param index The index of the process to move up in the heap.
 * @return      Returns EXIT_SUCCESS on success, or EXIT_FAILURE on error.
 */
int up_heap(int index, ProgArgs* heap[NUMCHILDREN]) {
    if (index == 0) {
        return EXIT_SUCCESS;
    }

    int parent = (index - 1) / 2;

    if (parent < 0 || parent >= heap_size) {
        fprintf(stderr, "Error: parent index is out of range\n");
        return EXIT_FAILURE;
    }

    if (index < 0 || index >= heap_size) {
        fprintf(stderr, "Error: index is out of range\n");
        return EXIT_FAILURE;
    }

    if (heap[parent]->timestamp > heap[index]->timestamp) {
        swap(heap[parent], heap[index]);
        return up_heap(parent, heap);
    }

    return EXIT_SUCCESS;
}


/**
 * @brief Heapify the heap by pushing down the element at the given index.
 *
 * @param index The index of the element to push down in the heap.
 * @return EXIT_SUCCESS if the heap was successfully heapified; otherwise, return EXIT_FAILURE.
 */
int down_heap(int index, ProgArgs* heap[NUMCHILDREN]) {
    int left = 2 * index + 1;
    int right = 2 * index + 2;
    int minimum = index;
    
    if (left < heap_size) {
        if (heap[left]->timestamp < heap[minimum]->timestamp) {
            minimum = left;
        }
    } else {
	return EXIT_FAILURE;
    }
    
    if (right < heap_size) {
        if (heap[right]->timestamp < heap[minimum]->timestamp) {
            minimum = right;
        }
    } else {
	return EXIT_FAILURE;
    }
    
    if (minimum != index) {
        if (index >= heap_size || minimum >= heap_size) {
            return EXIT_FAILURE;
        }
        
        if (swap(heap[minimum], heap[index]) != EXIT_SUCCESS) {
            return EXIT_FAILURE;
        }
        
        if (down_heap(minimum, heap) != 0) {
            return EXIT_FAILURE;
        }
    }
    
    return EXIT_SUCCESS;
}

/**
 * Adds a child process to the heap.
 *
 * @param pid The PID of the child process to add.
 * @return 0 if the process was added successfully, or -1 if the heap is full.
 */
int add_to_heap(pid_t pid, ProgArgs* heap[NUMCHILDREN]) {
    if (heap_size == NUMCHILDREN) {
        fprintf(stderr, "Error: Heap is full\n");
        return EXIT_FAILURE;
    } else if (heap_size < 0){
	fprintf(stderr, "Error: heap size cannot be zero\n");
	exit(EXIT_FAILURE);
    }
    time_t timestamp = time(NULL);
    heap[heap_size]->pid = pid;
    heap[heap_size]->status = 0;
    heap[heap_size]->timestamp = timestamp;
    heap_size++;
    up_heap(heap_size - 1, heap);

    return EXIT_SUCCESS;
}


/**
 * Searches for a process with a given PID in the heap.
 *
 * @param pid The process ID to search for.
 * @return The index of the matching process in the heap, or -1 if not found.
 */
int find_in_heap(pid_t pid, ProgArgs* heap[NUMCHILDREN]){
    if (heap == NULL) {
	
	 
        return -1;
    }

    for (int index = 0; index < heap_size; index++) {
        if (index >= NUMCHILDREN) {
            return -1;
        }
	ProgArgs* current = heap[index];

        if (current->pid == pid) {
            return index;
        }
    }
    return -1;
}

/**
 * Removes the process at index i from the heap.
 * @param i The index of the process to be removed.
 * @return EXIT_SUCCESS if successful, EXIT_FAILURE otherwise.
 */
int remove_from_heap(int index, ProgArgs* heap[NUMCHILDREN]){
    if (index < 0 || index >= heap_size) {
        fprintf(stderr, "Error: Index out of range\n");
        return EXIT_FAILURE;
    }
    heap_size--;
    heap[index] = heap[heap_size];
    down_heap(index, heap);
    return EXIT_SUCCESS;
}




