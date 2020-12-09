#ifndef CB_H
#define CB_H

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

#define MSG_ERR(msg, iserrno)                                                                            \
    {                                                                                                    \
        if (iserrno == 1)                                                                                \
            (void)fprintf(stderr, "[%s:%d] ERROR: " msg " : %s\n", __FILE__, __LINE__, strerror(errno)); \
        else                                                                                             \
            (void)fprintf(stderr, "[%s:%d] ERROR: " msg "\n", __FILE__, __LINE__);                       \
    }
#define EXIT_ERR(msg, iserrno, isServer) \
    {                                    \
        MSG_ERR(msg, iserrno)            \
        if (isServer == 1)               \
            clean_up_server();           \
        else                             \
            clean_up_client();           \
        exit(EXIT_FAILURE);              \
    }

#define BUFFER_SIZE 30
#define SOLUTION_SIZE 8

#define SHM_NAME "/shm_circular_buffer"

#define USED_SEM_NAME "used_sem"
#define FREE_SEM_NAME "free_sem"
#define MUTEX_SEM_NAME "mutex_sem"

#define PERMISSIONS 0600

void clean_up_server();

void clean_up_client();

void allocate_resources_server();

void allocate_resources_client();

void setup_signal_handling();

typedef struct
{
    unsigned int v1;
    unsigned int v2;
} edge;

typedef struct
{
    edge edges[SOLUTION_SIZE];
} solution_set;

typedef struct
{
    unsigned int state;
    unsigned int write_pos;
    solution_set buffer[BUFFER_SIZE];
} circularBuffer;

void write_set(solution_set *to_write);

int read_set(solution_set *new_solution);

void set_state(int state);

int get_state();

#endif //CB_H