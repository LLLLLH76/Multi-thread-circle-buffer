#include <iostream>
#include <malloc.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>
#include "circle_buffer.h"
using namespace std;

#define bf_size 2
int get_done=0, put_done=0;

struct InterfaceModule{
    sem_t mutex;
    sem_t next;
    int next_count;
};

class Monitor{
    public:
        CircleBuffer bf;
        InterfaceModule IM;
        sem_t notfull, notempty;
        int notfull_count, notempty_count;
        void enter();
        void leave();
        void wait(sem_t *x_sem, int *x_count);
        void signal(sem_t *x_sem, int *x_count);
};

void Monitor::enter(){
    sem_wait(&IM.mutex);
    return;
}

void Monitor::leave(){
    if (IM.next_count > 0){
        IM.next_count--;
        sem_post(&IM.next);
    }
    else
        sem_post(&IM.mutex);
    return;
}

void Monitor::wait(sem_t *x_sem, int *x_count){
    *x_count+=1;
    if (IM.next_count > 0){
        IM.next_count--;
        sem_post(&IM.next);
    }
    else
        sem_post(&IM.mutex);
    sem_wait(x_sem);
    return;
}

void Monitor::signal(sem_t *x_sem, int *x_count){
    if (*x_count > 0){
        *x_count-=1;
        IM.next_count++;
        sem_post(x_sem);
        sem_wait(&IM.next);
    }
    return;
}

Monitor monitor;

void print_buffer(){
    cout << "buffer: ";
    for (int i = 0; i < monitor.bf.size; i++){
        if (monitor.bf.buffer[i].is_element)
            cout << monitor.bf.buffer[i].content << " ";
        else
            cout << "x ";
    }
    if (monitor.bf.state == EMPTY)
        cout << "start: " << monitor.bf.start << ", end: " << monitor.bf.end << ", state: EMPTY\n";
    else if (monitor.bf.state == NORMAL)
        cout << "start: " << monitor.bf.start << ", end: " << monitor.bf.end << ", state: NORMAL\n";
    else if (monitor.bf.state == FULL)
        cout << "start: " << monitor.bf.start << ", end: " << monitor.bf.end << ", state: FULL\n";
}

void *put_function(void *arg){
    long int i = (long int)arg;
    monitor.enter();
    if (monitor.bf.state == FULL){
        cout << "thread " << i << " put. bf is full. thread " << i << " waits.\n";
        monitor.wait(&monitor.notfull, &monitor.notfull_count);
    }
    cout << "thread " << i << " put " << i << "\n";
    monitor.bf.put(i);
    print_buffer();
    monitor.signal(&monitor.notempty, &monitor.notempty_count);
    put_done++;
    monitor.leave();
    return NULL;
}

void *get_function(void *arg){
    long int i = (long int)arg;
    monitor.enter();
    if (monitor.bf.state == EMPTY){
        cout << "thread " << i << " get. bf is empty. thread " << i << " waits.\n";
        monitor.wait(&monitor.notempty, &monitor.notempty_count);
    }
    int res = monitor.bf.get();
    cout << "thread " << i << " get " << res << "\n";
    print_buffer();
    monitor.signal(&monitor.notfull, &monitor.notfull_count);
    get_done++;
    monitor.leave();
    return NULL;
}

#define put_thread_num 2
#define get_thread_num 2

int main()
{
    monitor.bf.set(bf_size);
    sem_init(&monitor.IM.mutex, 0, 1);
    sem_init(&monitor.IM.next, 0, 0);
    sem_init(&monitor.notfull, 0, 0);
    sem_init(&monitor.notempty, 0, 0);
    monitor.IM.next_count = 0;
    monitor.notfull_count = 0;
    monitor.notempty_count = 0;
    pthread_t put_thread[put_thread_num];
    pthread_t get_thread[get_thread_num];
    long int i = 0, j = 0;
    while(i < put_thread_num && j < get_thread_num){
        pthread_create(&put_thread[i], NULL, put_function, (void *)i);
        pthread_create(&get_thread[j], NULL, get_function, (void *)j);
        i++;
        j++;
    }
    if(i < put_thread_num){
        for(i; i < put_thread_num; i++)
            pthread_create(&put_thread[i], NULL, put_function, (void *)i);
    }
    if(j < get_thread_num){
        for(j; j < get_thread_num; j++)
            pthread_create(&get_thread[j], NULL, get_function, (void *)j);
    }
    sleep(3);
    if(get_done == get_thread_num){
        if(put_done < put_thread_num)
            cout<<"\nNo more get_threads while buffer is FULL.\nKill all put_threads.\n";
        for(i = 0; i < put_thread_num; i++)
            pthread_cancel(put_thread[i]);
    }
    if(put_done == put_thread_num){
        if(get_done < get_thread_num)
            cout<<"\nNo more put_threads while buffer is EMPTY.\nKill all get_threads.\n";
        for(i = 0; i < get_thread_num; i++)
            pthread_cancel(get_thread[i]);  
    }
    //for (i = 0; i < get_thread_num; i++)
    //    pthread_join(put_thread[i], NULL);
    //for (j = 0; j < get_thread_num; j++)
    //    pthread_join(get_thread[j], NULL);
    sem_destroy(&monitor.IM.next);
    sem_destroy(&monitor.IM.mutex);
    sem_destroy(&monitor.notfull);
    sem_destroy(&monitor.notempty);
    return 0;
}

/*
 for (long int i = 0; i < put_thread_num; i++)
        pthread_create(&put_thread[i], NULL, put_function, (void *)i);
    for (long int i = 0; i < get_thread_num; i++)
        pthread_create(&get_thread[i], NULL, get_function, (void *)i);
    for (int i,j = 0; i < put_thread_num, j < put_thread_num; i++, j++){
        pthread_join(put_thread[i], NULL);
        pthread_join(get_thread[i], NULL);
    }
    for (int i = 0; i < get_thread_num, j < put_thread_num; i++, j++){
        pthread_join(put_thread[i], NULL);
        pthread_join(get_thread[i], NULL);
    }
*/