#include "greenthread.h"
#include <bits/stdc++.h>
#include <atomic>
#include <thread>

using namespace std;

atomic<bool> time_is_up = false;

extern "C" void context_switch(void** current_thread_rsp,void* new_thread_rsp);

queue<Thread*> ready_queue;
vector<Thread*> extra;
Thread* current_thread = nullptr;
void green_yield();

void watch_dog() {
    while (true) {
        this_thread::sleep_for(chrono::milliseconds(50));
        time_is_up = true; 
    }
}

inline void GREEN_CHECK() {
    if (time_is_up) {
        time_is_up = false; 
        green_yield();
    }
}

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
    time_is_up = false;
    context_switch(&(prev_thread->rsp),new_thread->rsp);

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
    cout << "[Task 1] Started heavy calculations!" << endl;
    long long sum = 0;
    
    for(long long i = 1; i <= 600000000; i++){
        GREEN_CHECK(); /* Auto-injected by Clang */
        
        sum += (i * 2);
        if(i%6000000 == 0) cout << "    [Task 1] Computing... " << (i / 6000000) << "% done" << endl;
    }
    cout << "[Task 1] Finished! (Result: " << sum << ")" << endl;
    green_yield();

}

void task2(){
    cout << "[Task 2] Started heavy calculations!" << endl;
    long long sum = 0;
    
    for(long long i = 1; i <= 500000000; i++){
            GREEN_CHECK(); /* Auto-injected by Clang */
        
        sum += (i * 2);
        if(i%5000000 == 0) cout << "    [Task 2] Computing... " << (i / 5000000) << "% done" << endl;
    }
    cout << "[Task 2] Finished! (Result: " << sum << ")" << endl;
    green_yield();
    
}

int main(){

    cout << "=======================================" << endl;
    cout << "    BOOTING GREEN OS SCHEDULER    " << endl;
    cout << "=======================================" << endl;
    green_spawn(task1);
    green_spawn(task2);
    green_start();
    cout << "=======================================" << endl;
    cout << "    SHUTTING DOWN GREEN OS       " << endl;
    cout << "=======================================" << endl;

}