#include <unistd.h>
#include <signal.h>

#include "circularBuffer.h"

extern volatile sig_atomic_t quit;

int main(int argc, char const *argv[])
{

    allocate_resources_server();

    setup_signal_handling();

    while (quit != 1)
    {
        sleep(2);
    }

    set_state(1);

    clean_up_server();

    printf("\nclean exit\n\n");

    /* code */
    return 0;
}
