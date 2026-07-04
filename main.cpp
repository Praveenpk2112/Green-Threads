#include "greenthread.h"
#include <bits/stdc++.h>
using namespace std;

extern "C" void context_switch(void** current_thread_rsp,void* new_thread_rsp);

queue<Thread*> ready_queue;
vector<Thread*> extra;
Thread* current_thread = nullptr;

void green_spawn(void(*func)()){

    Thread* t = new Thread();
    setup(t,func);
    ready_queue.push(t);

}

void green_yield(){

    for(auto i:extra){
        delete i;
    }
    extra.clear();
    if(ready_queue.empty()) return;

    Thread* new_thread = ready_queue.front();
    ready_queue.pop();
    if(current_thread->state != Thread_State::DONE) ready_queue.push(current_thread);
    else{
        extra.push_back(current_thread);
    }
    Thread* prev_thread = current_thread;
    current_thread = new_thread;
    context_switch(&(prev_thread->rsp),new_thread->rsp);

}

void green_exit(){
    current_thread->state = Thread_State::DONE;
    cout << "This task this done" << endl;
    green_yield();
}

void green_start(){

    Thread* main = new Thread();

    current_thread = main;
    while(!ready_queue.empty()) green_yield();

    for(auto i:extra) delete i;
    extra.clear();
}

void task1(){

    cout << "Starting task 1" << endl;
    green_yield();
    cout << "Resuming and ending task1" << endl;

}

void task2(){

    cout << "Starting task 2" << endl;
    green_yield();
    cout << "Resuming and ending task2" << endl;
    
}

int main(){

    cout << "Testing green threads" << endl;
    green_spawn(task1);
    green_spawn(task2);
    green_spawn(task2);
    green_start();
}