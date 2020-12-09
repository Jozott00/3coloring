#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <semaphore.h>
#include <signal.h>

#include "circularBuffer.h"

volatile sig_atomic_t quit = 0;

circularBuffer *cb = MAP_FAILED;

sem_t *sem_used = SEM_FAILED;
sem_t *sem_free = SEM_FAILED;
sem_t *sem_mutex = SEM_FAILED;

unsigned int is_shm_open = 0;
unsigned int is_mapped = 0;
unsigned int is_sem_used = 0;
unsigned int is_sem_free = 0;
unsigned int is_sem_mutex = 0;

void signal_handler()
{
    quit = 1;
}

void setup_signal_handling()
{
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));

    sa.sa_handler = signal_handler;

    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);
}

void allocate_resources_server()
{

    int shmfd = shm_open(SHM_NAME, O_CREAT | O_RDWR, PERMISSIONS);
    if (shmfd == -1)
        EXIT_ERR("could not create shared memory", 1, 1)

    is_shm_open = 1;

    if (ftruncate(shmfd, sizeof(circularBuffer)) == -1)
        EXIT_ERR("truncation of shm failed", 1, 1)

    if ((cb = mmap(NULL, sizeof(circularBuffer), PROT_WRITE | PROT_READ, MAP_SHARED, shmfd, 0)) == MAP_FAILED)
        EXIT_ERR("mapping failed", 1, 1)

    is_mapped = 1;

    if (close(shmfd) == -1)
        MSG_ERR("could not close shm fd", 1)

    if ((sem_used = sem_open(USED_SEM_NAME, O_CREAT, PERMISSIONS, 0)) == SEM_FAILED)
        EXIT_ERR("could not open used semaphore", 1, 1)

    is_sem_used = 1;

    if ((sem_free = sem_open(FREE_SEM_NAME, O_CREAT, PERMISSIONS, BUFFER_SIZE)) == SEM_FAILED)
        EXIT_ERR("could not open free semaphore", 1, 1)

    is_sem_free = 1;

    if ((sem_mutex = sem_open(MUTEX_SEM_NAME, O_CREAT, PERMISSIONS, 1)) == SEM_FAILED)
        EXIT_ERR("could not open mutex semaphore", 1, 1)

    is_sem_mutex = 1;
}

void allocate_resources_client()
{

    int shmfd = shm_open(SHM_NAME, O_RDWR, PERMISSIONS);
    if (shmfd == -1)
        EXIT_ERR("could not open shared memory", 1, 0)

    is_shm_open = 1;

    if ((cb = mmap(NULL, sizeof(circularBuffer), PROT_READ | PROT_WRITE, MAP_SHARED, shmfd, 0)) == MAP_FAILED)
        EXIT_ERR("could not map to shm", 1, 0)

    is_mapped = 1;

    if ((sem_free = sem_open(FREE_SEM_NAME, 0)) == SEM_FAILED)
        EXIT_ERR("could not open free semaphore", 1, 0)
    is_sem_free = 1;

    if ((sem_used = sem_open(USED_SEM_NAME, 0)) == SEM_FAILED)
        EXIT_ERR("could not open used semaphore", 1, 0)
    is_sem_used = 1;

    if ((sem_mutex = sem_open(MUTEX_SEM_NAME, 0)) == SEM_FAILED)
        EXIT_ERR("could not open mutex semaphore", 1, 0)
    is_sem_mutex = 1;
}

void clean_up_server()
{

    if (is_mapped == 1)
    {
        if (munmap(cb, sizeof(*cb)) == -1)
            MSG_ERR("couldnot unmap to shm", 1)
    }

    if (is_shm_open == 1)
    {
        if (shm_unlink(SHM_NAME) == -1)
            MSG_ERR("could not unlink shm", 1)
    }

    if (is_sem_used == 1)
    {

        if (sem_close(sem_used) == -1)
            MSG_ERR("could not close used semaphore", 1);

        if (sem_unlink(USED_SEM_NAME) == -1)
            MSG_ERR("could not unlink used semaphore", 1);
    }

    if (is_sem_free == 1)
    {

        if (sem_close(sem_free) == -1)
            MSG_ERR("could not close free semaphore", 1);

        if (sem_unlink(FREE_SEM_NAME) == -1)
            MSG_ERR("could not unlink free semaphore", 1);
    }

    if (is_sem_mutex == 1)
    {

        if (sem_close(sem_mutex) == -1)
            MSG_ERR("could not close mutex semaphore", 1);

        if (sem_unlink(MUTEX_SEM_NAME) == -1)
            MSG_ERR("could not unlink mutex semaphore", 1);
    }
}

void clean_up_client()
{

    if (is_mapped == 1)
    {
        if (munmap(cb, sizeof(*cb)) == -1)
            MSG_ERR("couldnot unmap to shm", 1)
    }

    if (is_sem_used == 1)
    {
        if (sem_close(sem_used) == -1)
            MSG_ERR("could not close used semaphore", 1);
    }

    if (is_sem_free == 1)
    {
        if (sem_close(sem_free) == -1)
            MSG_ERR("could not close free semaphore", 1);
    }

    if (is_sem_mutex == 1)
    {
        if (sem_close(sem_mutex) == -1)
            MSG_ERR("could not close mutex semaphore", 1);
    }
}

void write_set(solution_set *to_write)
{
    cb->buffer[cb->write_pos] = *to_write;
}

void set_state(int state)
{
    cb->state = state;
}

int get_state()
{
    return cb->state;
}
