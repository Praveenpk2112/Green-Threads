#ifndef GREENTHREAD_H
#define GREENTHREAD_H

enum class Thread_State{
    READY,
    RUNNING,
    DONE
};

struct Thread{

    void *rsp;
    char *stack_memory;
    Thread_State state;
    Thread();
    ~Thread();

};

void setup(Thread* t,void(*func_to_run)());
void green_exit();
#endif

