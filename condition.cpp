#include <iostream>
#include <malloc.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>
#include "circle_buffer.h"
using namespace std;
#define bf_size 2
int get_done=0, put_done=0;

pthread_cond_t cond_put, cond_get;
pthread_mutex_t mutex;
CircleBuffer bf;

void *get_function(void *arg){
    pthread_mutex_lock(&mutex);
    while(bf.state == EMPTY){
        pthread_cond_wait(&cond_get, &mutex);
    }
        long int i = (long int)arg;
        cout<<"get_thread "<<i;
        cout<<" get ";
        int res = bf.get();
        if(res != -1){
            cout<<res;
        }
        cout<<"\n";
        cout<<"buffer: ";
        for(int i=0; i<bf.size; i++){
            if(bf.buffer[i].is_element){
                cout<<bf.buffer[i].content<<" ";
            }
            else{
                cout<<"x ";
            }
        }
        if(bf.state == EMPTY)
            cout<<"start: x, end: x, state: EMPTY\n";
        else if(bf.state == NORMAL)
            cout<<"start: "<<bf.buffer[bf.start].content<<", end: "<<bf.buffer[bf.end].content<<", state: NORMAL\n";
        else if(bf.state == FULL)
            cout<<"start: "<<bf.buffer[bf.start].content<<", end: "<<bf.buffer[bf.end].content<<", state: FULL\n";
    pthread_cond_signal(&cond_put);
    put_done++;
    pthread_mutex_unlock(&mutex);
    return NULL;
}

void *put_function(void *arg){
    pthread_mutex_lock(&mutex);
    while(bf.state == FULL){
        pthread_cond_wait(&cond_put, &mutex);
    }
        long int i = (long int)arg;
        cout<<"put_thread "<<i;
        cout<<" put "<<i<<"\n";
        bf.put(i);
        cout<<"buffer: ";
        for(int i=0; i<bf.size; i++){
            if(bf.buffer[i].is_element){
                cout<<bf.buffer[i].content<<" ";
            }
            else{
                cout<<"x ";
            }
        }
        if(bf.state == EMPTY)
            cout<<"start: x, end: x, state: EMPTY\n";
        else if(bf.state == NORMAL)
            cout<<"start: "<<bf.buffer[bf.start].content<<", end: "<<bf.buffer[bf.end].content<<", state: NORMAL\n";
        else if(bf.state == FULL)
            cout<<"start: "<<bf.buffer[bf.start].content<<", end: "<<bf.buffer[bf.end].content<<", state: FULL\n";
    pthread_cond_signal(&cond_get);
    get_done++;
    pthread_mutex_unlock(&mutex);
    return NULL;
}

#define put_thread_num 2
#define get_thread_num 4

int main(){
    bf.set(bf_size);
    pthread_cond_init(&cond_get, NULL);
    pthread_cond_init(&cond_put, NULL);
    pthread_mutex_init(&mutex, NULL);
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
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond_get);
    pthread_cond_destroy(&cond_put);
    return 0;
}