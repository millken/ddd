#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>


void restart_process(char **args){
        int childpid;

        childpid = fork ();
        if (childpid < 0) {
                perror ("fork failed");
        } else if (childpid  == 0) {
                printf ("new process %d", getpid());
                int rv = execve (args[0], args, NULL);
                if (rv == -1) {
                        perror ("execve");
                        exit (EXIT_FAILURE);
                }

        } else {
                sleep (5);
                printf ("killing %d\n", getpid());
                kill (getpid (), SIGTERM);
        }
}

volatile sig_atomic_t signal_recieved = 0;
void sighandler (int signum) {
        signal_recieved = 1;
}

int main(int argc, char* argv[]){
        int i = 60;

        printf ("process %d starting\n", getpid ());

        if (signal (SIGUSR1, sighandler) == SIG_ERR) {
                perror ("signal failed");
        }

        while (i-- > 0) {
                printf ("\nIteration --> %d", i);
                if (signal_recieved) {
                        signal_recieved = 0;
                        restart_process (argv);
                }
                sleep(1);
        }

        return 0;
}
