#include <assert.h>
#include <signal.h>
#include <unistd.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sys/time.h>

#include "context.h"
#include "preempt.h"
#include "queue.h"
#include "uthread.h"

typedef enum thread_state {
    running = 0,
    ready = 1,
    blocked = 2,
    zombie = 3
} thread_state;

typedef struct thread {
    int retval;
    uthread_t tid;
    uthread_t* host_tid;
    thread_state state;
    uthread_ctx_t *ctx_t;
    void* sp;
} thread;

thread* active_thread;

queue_t ready_list;
queue_t zombie_list;
queue_t blocked_list;

static int find_thread(void *data, void *arg) {
    bool found = false;
    thread* tcb = (thread*) data;
    uthread_t* tid = (uthread_t*) arg;
    if (tcb->tid == *tid) {
        found = true;
    }
    return found;
}

// callback function for queue_iterate
// prints tid and state of thread
static int print_item(void *data, void *arg) {
    thread* tcb = (thread*) data;
    printf("        ----> Thread %d\n", tcb->tid);
    printf("              ----> State: %d\n", tcb->state);
    if (tcb->host_tid != NULL)
        printf("              ----> Host TID: %d\n", *(tcb->host_tid));
    return 0;
}

static thread* clone(thread* original) {
    thread* temp = (thread*) malloc(sizeof(thread));
    temp->tid = original->tid;
    temp->host_tid = original->host_tid;
    temp->retval = original->retval;
    temp->sp = original->sp;
    temp->ctx_t = original->ctx_t;
    temp->state = original->state;
    return temp;
}

static void unblock(uthread_t* host_tid) {
    void* data = NULL;
    queue_iterate(blocked_list, find_thread, host_tid, &data);
    if (data != NULL) {
        thread* host = clone( (thread*) data);
        queue_enqueue(ready_list, host);
    }
    queue_delete(blocked_list, data);
}

static void reap(uthread_t tid, int* retval) {
    void* data = NULL;
    queue_iterate(zombie_list, find_thread, &tid, &data);
    if (data != NULL) {
        thread* zombie = (thread*) data;
        if (retval != NULL)
            *retval = zombie->retval;
        queue_delete(zombie_list, data);
    }
}

static int attach(uthread_t guest, uthread_t* host) {
    int status = 0;
    void* data = NULL;

    queue_iterate(ready_list, find_thread, &guest, &data);

    // guest not found in ready list check blocked list
    if (data == NULL) {
        queue_iterate(blocked_list, find_thread, &guest, &data);
    }

    if (data != NULL) {
        thread* tcb = (thread*) data;
        if (tcb->host_tid != NULL) {
            fprintf(stderr,"Error: thread is already joined\n");
            status = -1;
        } else {
            tcb->host_tid = host;
        }  
    } else {
        fprintf(stderr,"Error: thread not found\n");
        status = -1;
    }

    return(status);
}

void uthread_init() {

    // instantiate queues
    ready_list = queue_create();
    zombie_list = queue_create();
    blocked_list = queue_create();

    // set active thread to thread #0
    active_thread = (thread*) malloc(sizeof(thread));
    active_thread->tid = 0;
    active_thread->host_tid = NULL;
    active_thread->state = running;
    active_thread->ctx_t = (uthread_ctx_t*) malloc(sizeof(uthread_ctx_t));
    active_thread->sp = NULL;

    preempt_start();
}

/* TODO Phase 2 */

void uthread_yield(void)
{
    preempt_disable();

    thread *from = active_thread,
           *to = NULL;

    
    queue_dequeue(ready_list, &to);
    

    to->state = running;
    active_thread = to;

    if (from->state == running)
    {
        from->state = ready;
        queue_enqueue(ready_list, from);
    }

    uthread_ctx_switch(from->ctx_t, to->ctx_t); 
    preempt_enable();
}

uthread_t uthread_self(void)
{
    return active_thread->tid;
}

int uthread_create(uthread_func_t func, void *arg)
{
    static unsigned int tid_counter = 1;
    
    if (tid_counter == 1)
        uthread_init();
    
    thread *worker = (thread*) malloc(sizeof(thread));
    worker->tid = tid_counter;
    worker->state = ready;
    worker->host_tid = NULL;
    worker->ctx_t = (uthread_ctx_t*) malloc(sizeof(uthread_ctx_t));
    worker->sp = uthread_ctx_alloc_stack();
    
    uthread_ctx_init(worker->ctx_t,worker->sp,func,NULL);

    queue_enqueue(ready_list, worker);
    
    return tid_counter++;
}

void uthread_exit(int retval)
{
    active_thread->state = zombie;
    active_thread->retval = retval;
    queue_enqueue(zombie_list, active_thread);

    if (active_thread->host_tid != NULL) {
        unblock( active_thread->host_tid);
    }

    if (queue_length(ready_list) > 0) 
        uthread_yield();

	/* TODO Phase 2 */
}

int uthread_join(uthread_t tid, int *retval)
{
	int status = 0;
    
    if (tid == 0) {
        fprintf(stderr,"Error: cannot join main thread\n");
        status = -1;
    } else if (tid == uthread_self()) {
        fprintf(stderr,"Error: cannot join same thread\n");
        status = -1;
    } else {
        void* data = NULL;
        queue_iterate(zombie_list,find_thread,&tid,&data);

        if (data != NULL) {
            reap(tid, retval);
            status = 0;
        } else if ( (status = attach(tid, &active_thread->tid)) == 0) {
            active_thread->state = blocked;
            queue_enqueue(blocked_list, active_thread);
            if (queue_length(ready_list) > 0) {
                uthread_yield();
                reap(tid, retval);
            }
        }
    }
    return status;
}
