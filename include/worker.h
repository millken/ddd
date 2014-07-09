#ifndef _WORKER_H
#define _WORKER_H

void start_worker();

int fork_process(void (*func)());
int new_thread_p(void *func, void *i);


void dns_master();
void dns_worker();

#endif /// _WORKER_H
