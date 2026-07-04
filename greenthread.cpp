#include "greenthread.h"
#include <cstdint>

Thread::Thread(){

    stack_memory = new char[64*1024];
    state = Thread_State::READY;
    rsp = stack_memory + (64*1024);
}

Thread::~Thread(){

    delete[] stack_memory;

}

void setup(Thread* t,void(*func_to_run)()){

    uint64_t* stack = reinterpret_cast<uint64_t*>(t->rsp);
    stack--;
    *stack = reinterpret_cast<uint64_t>((green_exit));
    stack--;
    *stack = reinterpret_cast<uint64_t>(func_to_run);
    
    for(int i = 0; i < 8; i++){
        stack--;
        *stack = 0;
    }
    
    t->rsp = stack;
}
