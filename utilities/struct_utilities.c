# include <stdlib.h>
# ifndef LINESIZE
# include "../constants/constants.h"
# endif

int add_to_array(ParentStruct* parent, ChildStruct* child) {
    if (!parent->prog_args) {
        return EXIT_FAILURE;
    }

    for (int i = 0; i < MAX_CHILDREN; i++) {
        if (parent->prog_args->children[i] == NULL) {
            parent->prog_args->children[i] = child;
            return EXIT_SUCCESS;
        }
    }
    return EXIT_FAILURE;
}

int remove_from_array(ParentStruct* parent, ChildStruct* child) {
    if (!parent->prog_args) {
        return EXIT_FAILURE;
    }

    for (int i = 0; i < MAX_CHILDREN; i++) {
        if (parent->prog_args->children[i] == child) {
            parent->prog_args->children[i] = NULL;
            return EXIT_SUCCESS;
        }
    }
    return EXIT_FAILURE;
}

int update_child(ChildStruct* child, int new_pid, int new_pid_counter) {
    if (!child) {
        return EXIT_FAILURE;
    }
    child->pid = new_pid;
    child->pid_counter = new_pid_counter;
    return EXIT_SUCCESS;
}
