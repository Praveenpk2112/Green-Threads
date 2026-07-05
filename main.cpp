#include "greenthread.h"
#include <bits/stdc++.h>
#include <atomic>
#include <thread>

using namespace std;

atomic<bool> time1 = false;

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
    time1 = false;
    context_switch(&(prev_thread->rsp),new_thread->rsp);

}

inline void GREEN_CHECK(){
    if(time1){
        cout << "Time slice if gone Prempting Task!" << endl;
        green_yield();
    }
}

void watch_dog(){
    while(true){
        this_thread::sleep_for(chrono::milliseconds(10));
        time1 = true;
    }
}

void green_exit(){
    current_thread->state = Thread_State::DONE;
    cout << "This task this done" << endl;
    green_yield();
}

void green_start(){

    thread monitor(watch_dog);
    monitor.detach();

    Thread* main = new Thread();

    current_thread = main;
    while(!ready_queue.empty()) green_yield();

    for(auto i:extra) delete i;
    extra.clear();
}

void task1(){

    for(int i=1;i<=1000;i++){
    
        GREEN_CHECK(); /* Auto-injected by Clang */
    
        cout << "Checking for loop in task1"<< endl;
    }
    cout << "Starting task 1" << endl;
    cout << "Resuming and ending task1" << endl;

}

void task2(){

    cout << "Starting task 2" << endl;
    cout << "Resuming and ending task2" << endl;
    
}

int main(){

    cout << "Testing green threads" << endl;
    
    green_spawn(task2);
    green_spawn(task1);
    green_spawn(task2);
    green_start();
}