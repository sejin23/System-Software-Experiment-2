#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
pthread_mutex_t writelock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t varr = PTHREAD_COND_INITIALIZER;
pthread_cond_t varw = PTHREAD_COND_INITIALIZER;
int b, c, data;
void* writer(void* arg);
void* reader(void* arg);
int main(int argc, char** argv){
    int i;
    pthread_t* tid = (pthread_t*)malloc(sizeof(pthread_t)*6);
    b = 0;
    c = 0;
    data = 0;
    pthread_create(&tid[0], NULL, writer, NULL);
    for(i=1;i<6;i++) pthread_create(&tid[i], NULL, reader, NULL);
    for(i=0;i<6;i++) pthread_join(tid[i], NULL);
    return 1;
}
void* writer(void* arg){
    int i;
    for(i=0;i<1000000;i++){
        pthread_mutex_lock(&writelock);
        b = 1;
        while(c > 0) pthread_cond_wait(&varw, &writelock);
        data++;
        b = 0;
        pthread_cond_signal(&varr);
        pthread_mutex_unlock(&writelock);
    }
}
void* reader(void* arg){
    int i, data_r, prev;
    for(i=0;i<10000000;i++){
        prev = data_r;
        pthread_mutex_lock(&writelock);
        while(b) pthread_cond_wait(&varr, &writelock);
        c++;
        pthread_mutex_unlock(&writelock);
        data_r = data;
        pthread_mutex_lock(&writelock);
        c--;
        if(c == 0) pthread_cond_signal(&varw);
        pthread_mutex_unlock(&writelock);
        if(prev != data_r) printf("%ld : %d\n", pthread_self(), data_r);
    }
}

/*pthread_mutex_t readlock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t writelock = PTHREAD_MUTEX_INITIALIZER;
int b, data;
void* writer(void* arg);
void* reader(void* arg);
int main(int argc, char** argv){
    int i;
    pthread_t* tid = (pthread_t*)malloc(sizeof(pthread_t)*6);
    b = 0;
    data = 0;
    pthread_create(&tid[0], NULL, writer, NULL);
    for(i=1;i<6;i++) pthread_create(&tid[i], NULL, reader, NULL);
    for(i=0;i<6;i++) pthread_join(tid[i], NULL);
    return 1;
}
void* writer(void* arg){
    int i;
    for(i=0;i<1000000;i++){
        pthread_mutex_lock(&writelock);
        data++;
        pthread_mutex_unlock(&writelock);
    }
}
void* reader(void* arg){
    int i, data_r, prev;
    for(i=0;i<10000000;i++){
        prev = data_r;
        pthread_mutex_lock(&readlock);
        b++;
        if(b == 1) pthread_mutex_lock(&writelock);
        pthread_mutex_unlock(&readlock);
        data_r = data;
        pthread_mutex_lock(&readlock);
        b--;
        if(b == 0) pthread_mutex_unlock(&writelock);
        pthread_mutex_unlock(&readlock);
        if(prev != data_r) printf("%ld %d\n", pthread_self(), data_r);
    }
}*/

/*int data;
pthread_spinlock_t spinl;
void* writer(void* arg);
void* reader(void* arg);
int main(int argc, char** argv){
    int i;
    pthread_t* tid = (pthread_t*)malloc(sizeof(pthread_t)*6);
    data = 0;
    pthread_spin_init(&spinl, PTHREAD_PROCESS_PRIVATE);
    pthread_create(&tid[0], NULL, writer, NULL);
    for(i=1;i<6;i++){
        pthread_create(&tid[i], NULL, reader, NULL);
    }
    for(i=0;i<6;i++)
        pthread_join(tid[i], NULL);
    return 1;
}
void* writer(void* arg){
    int i;
    for(i=0;i<1000000;i++){
        pthread_spin_lock(&spinl);
        data++;
        pthread_spin_unlock(&spinl);
    }
}
void* reader(void* arg){
    int i, data_r, prev;
    for(i=0;i<10000000;i++){
        prev = data_r;
        pthread_spin_lock(&spinl);
        data_r = data;
        pthread_spin_unlock(&spinl);
        if(prev != data_r) printf("%ld : %d\n", pthread_self(), data_r);
    }
}*/

/*int data;
pthread_mutex_t gmutex;
void* writer(void* arg);
void* reader(void* arg);
int main(int argc, char** argv){
    int i;
    pthread_t* tid = (pthread_t*)malloc(sizeof(pthread_t)*6);
    data = 0;
    pthread_mutex_init(&gmutex, NULL);
    pthread_create(&tid[0], NULL, writer, NULL);
    for(i=1;i<6;i++){
        pthread_create(&tid[i], NULL, reader, NULL);
    }
    for(i=0;i<6;i++)
        pthread_join(tid[i], NULL);
    return 1;
}
void* writer(void* arg){
    int i;
    for(i=0;i<1000000;i++){
        pthread_mutex_lock(&gmutex);
        data++;
        pthread_mutex_unlock(&gmutex);
    }
}
void* reader(void* arg){
    int i, data_r, prev;
    for(i=0;i<10000000;i++){
        prev = data_r;
        pthread_mutex_lock(&gmutex);
        data_r = data;
        pthread_mutex_unlock(&gmutex);
        if(prev != data_r) printf("%ld : %d\n", pthread_self(), data_r);
    }
}*/