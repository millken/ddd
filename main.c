#include <stdio.h>
#include <string.h>
#include <getopt.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>
#include <stdbool.h>
#include <unistd.h>
#include <ctype.h>
#include "logger.h"
#include "config.h"

#define VERSION "1.0"

void usage()
{
    char *b = "------------------------------------------------------------\n"
              "dns test tool " VERSION "\n\n"
               "-d            debug level\n"
               "-h            print this help and exit\n\n"
               "-----------------------------------------------------------\n"
               "\n";
    fprintf(stderr, b, strlen(b));

}
/* 父进程信号处理 */
static void kill_signal_master(const int sig) {

    /* 给进程组发送SIGTERM信号，结束子进程 */
    kill(0, SIGTERM);
       
    exit(0);
}

/* 子进程信号处理 */
static void kill_signal_worker(const int sig) {

    exit(0);
}


void main(int argc, char* argv[])
{
	int c;
	int pid, worker_pid, worker_pid_wait;
	bool deamon_mode = true;

	logger = Logger_create();
	logger->level = LOG_ERROR;
    /* process arguments */
    while ((c = getopt(argc, argv, "fd:h")) != -1) {
        switch (c) {
            case 'f':
                deamon_mode = false;
                break;
            case 'd' :
            	logger->level = atoi(optarg);
            	break;
            case 'h':
            	usage();
            	break;
        default:
            break ;
        }
    }
    //以守护进程运行
	if(deamon_mode == true) {
	     /* Fork off the parent process */      
	    pid = fork();
	    if (pid < 0) {
	        exit(0);
	    }
	    /* If we got a good PID, then we can exit the parent process. */
	    if (pid > 0) {
	       exit(1);
	    }
    }

	process_filename = argv[0];
    /* 派生子进程（工作进程） */
    worker_pid = fork();
    //如果是父进程
    if(worker_pid > 0) {
	    /* 处理父进程接收到的kill信号 */
	    /* 忽略Broken Pipe信号 */
	    signal(SIGPIPE, SIG_IGN);

	    /* 处理kill信号 */
	    signal (SIGINT, kill_signal_master);
	    signal (SIGKILL, kill_signal_master);
	    signal (SIGQUIT, kill_signal_master);
	    signal (SIGTERM, kill_signal_master);
	    signal (SIGHUP, kill_signal_master);

	    /* 处理段错误信号 */
	    signal(SIGSEGV, kill_signal_master);
	    /* 如果子进程终止，则重新派生新的子进程 */
	    while (1) {
	                    worker_pid_wait = wait(NULL);
	                    if (worker_pid_wait < 0) {
	                            continue;
	                    }

	        usleep(100000);
	        
	        worker_pid = fork();
	                    /*成功生成子进程*/
	        if (worker_pid == 0) {
	            break;
	        }
	    }
	}

    /* ---------------子进程内容------------------- */
   
    /* 忽略Broken Pipe信号 */
    signal(SIGPIPE, SIG_IGN);
   
    /* 处理kill信号 */
    signal (SIGINT, kill_signal_worker);
    signal (SIGKILL, kill_signal_worker);
    signal (SIGQUIT, kill_signal_worker);
    signal (SIGTERM, kill_signal_worker);
    signal (SIGHUP, kill_signal_worker);
       
    /* 处理段错误信号 */
    signal(SIGSEGV, kill_signal_worker);
	//init_daemon();

   	start_worker();

	free(logger);
    return;
}


