/*!
 * linux下互斥锁的使用
 * 互斥锁和信号量的区别
 * 1.互斥锁的加锁和解锁必须由同一个线程（进程）完成，而信号量是A->B,B->A,A->A (在Linux上可以由其他进程或线程解锁)
 *   不一定是一个
 * 2.互斥所只能控制一个资源，信号量可以控制多个
 */

#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <semaphore.h>

pthread_mutex_t l;
sem_t s1;

void* t1(void* arg) {
  int id = *(int*)arg;
  while(1) {
    pthread_mutex_lock(&l);
    fprintf(stdout,",");
    sem_post(&s1);
  }

  return NULL;
}

void* t2(void* arg) {
  int id = *(int*)arg;
  while(1) {
    sem_wait(&s1);
    pthread_mutex_unlock(&l);
  }
  return NULL;
}

int main() {
  int i;
  pthread_t t[2];
  int args[2] = {0,1};

  sem_init(&s1,0,0);
  pthread_mutex_init(&l,0);
  pthread_create(&t[0],NULL,t1,&args[0]);
  pthread_create(&t[1],NULL,t2,&args[1]);

  for(i=0;i<2;++i) pthread_join(t[i],NULL);
  pthread_mutex_destroy(&l);

  return 0;
}


