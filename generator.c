#include <unistd.h>
#include <signal.h>

#include "circularBuffer.h"

extern volatile sig_atomic_t quit;

int main(int argc, char const *argv[])
{

    allocate_resources_client();

    setup_signal_handling();

    while (quit != 1)
    {
        if (get_state() == 1)
            quit = 1;

        sleep(2);
    }

    clean_up_client();

    return 0;
}
