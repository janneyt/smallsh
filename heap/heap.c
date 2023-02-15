#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <sys/types.h>
#include <sys/wait.h>


struct ProgArgs child_process;
static child_process heap[NUMCHILDREN];

/**
 * @brief Swaps the values of two child_process objects.
 * 
 * @param left Pointer to the first child_process object.
 * @param right Pointer to the second child_process object.
 */
void swap(child_process *left, child_process *right){
    child_process tmp = *left;
    *left = *right;
    *right = tmp;
}

/**
 * Move a process up in the heap to maintain the min-heap property.
 *
 * @param index The index of the process to move up in the heap.
 * @return      Returns EXIT_SUCCESS on success, or EXIT_FAILURE on error.
 */
int up_heap(int index) {
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

    if (heap[parent].timestamp > heap[index].timestamp) {
        swap(&heap[parent], &heap[index]);
        return up_heap(parent);
    }

    return EXIT_SUCCESS;
}


/**
 * @brief Heapify the heap by pushing down the element at the given index.
 *
 * @param index The index of the element to push down in the heap.
 * @return EXIT_SUCCESS if the heap was successfully heapified; otherwise, return EXIT_FAILURE.
 */
int down_heap(int index) {
    int left = 2 * index + 1;
    int right = 2 * index + 2;
    int minimum = index;
    
    if (left < heap_size) {
        if (heap[left].timestamp < heap[minimum].timestamp) {
            minimum = left;
        }
    } else {
	return EXIT_FAILURE;
    }
    
    if (right < heap_size) {
        if (heap[right].timestamp < heap[minimum].timestamp) {
            minimum = right;
        }
    } else {
	return EXIT_FAILURE;
    }
    
    if (minimum != index) {
        if (index >= heap_size || minimum >= heap_size) {
            return EXIT_FAILURE;
        }
        
        if (swap(&heap[minimum], &heap[index]) != 0) {
            return EXIT_FAILURE;
        }
        
        if (down_heap(minimum) != 0) {
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
int add_to_heap(pid_t pid) {
    if (heap_size == NUMCHILDREN) {
        fprintf(stderr, "Error: Heap is full\n");
        return EXIT_FAILURE;
    } else if (heap_size < 0){
	fprintf(stderr, "Error: heap size cannot be zero\n");
	exit(EXIT_FAILURE);
    }
    time_t timestamp = time(NULL);
    heap[heap_size].pid = pid;
    heap[heap_size].status = 0;
    heap[heap_size].timestamp = timestamp;
    heap_size++;
    up_heap(heap_size - 1);

    return EXIT_SUCCESS;
}


/**
 * Searches for a process with a given PID in the heap.
 *
 * @param pid The process ID to search for.
 * @return The index of the matching process in the heap, or -1 if not found.
 */
int find_in_heap(pid_t pid){
    if (heap == NULL) {
        return EXIT_FAILURE;
    }

    for (int index = 0; index < heap_size; index++) {
        if (index >= MAX_PROCESSES) {
            return EXIT_FAILURE;
        }
        if (heap[index].pid == pid) {
            return index;
        }
    }
    return EXIT_FAILURE;
}

/**
 * Removes the process at index i from the heap.
 * @param i The index of the process to be removed.
 * @return EXIT_SUCCESS if successful, EXIT_FAILURE otherwise.
 */
int remove_from_heap(int index)
{
    if (index < 0 || index >= heap_size) {
        fprintf(stderr, "Error: Index out of range\n");
        return EXIT_FAILURE;
    }
    heap_size--;
    heap[index] = heap[heap_size];
    down_heap(index);
    return EXIT_SUCCESS;
}

/**
 * Prints the contents of the heap.
 *
 * @return EXIT_SUCCESS on success, EXIT_FAILURE on failure
 */
int print_heap(){
    printf("Heap contents:\n");
    for (int i = 0; i < heap_size; i++) {
        if (i < 0 || i >= MAX_PROCESSES || !heap[i].pid || !heap[i].status || !heap[i].timestamp) {
            fprintf(stderr, "Error: Heap index %d is out of range or contains invalid data\n", i);
            return EXIT_FAILURE;
        }
        printf("%d: PID %d, status %d, timestamp %ld\n", i, heap[i].pid, heap[i].status, heap[i].timestamp);
    }
    return EXIT_SUCCESS;
}


void child_error(char child_action[], char action_taken[])
{
    printf("%s %s\n", child_action, action_taken);
}

/**
 * @brief Handle SIGCHLD signal by updating the status of the corresponding process in the heap
 *
 * @param signum The signal number
 * @return Returns EXIT_SUCCESS on success, and EXIT_FAILURE if an error occurs.
 */
void sigchld_handler(int signum) {
    int status;
    pid_t pid;

    // using -getpid to wait for the process group id looks for any unwaited-for child that shares the same process group id, 
    // which in this case is the parent's pid
    while ((pid = waitpid(-getpid(), &status, WNOHANG)) > 0) {
        int index = find_in_heap(pid);

        if (index >= 0 && index < heap_size) {
            if(!heap[index]){
	        fprintf(stderr, "Heap has been corrupted");
		return EXIT_FAILURE;
	    }
	if (heap[index].pid != pid) {
            fprintf(stderr, "Error: PID in heap does not match waited-for PID\n");
            return EXIT_FAILURE;
        }

        heap[i].status = status;

        if (WIFEXITED(status)) {
            child_error("Child process exited with status", "");
            return EXIT_SUCCESS;
        } else if (WIFSIGNALED(status)) {
            child_error("Child process killed by signal", "");
            return EXIT_SUCCESS;
        } else if (WIFSTOPPED(status)) {
            child_error("Child process stopped. Continuing.", "");
            if (kill(pid, SIGCONT) < 0) {
            perror("Error: Failed to send SIGCONT signal to child process");
            return EXIT_FAILURE;
            }
        }
        } else {
            fprintf(stderr, "Error: PID not found in heap\n");
            return EXIT_FAILURE;
        }
    }

    return EXIT_SUCCESS;
}

