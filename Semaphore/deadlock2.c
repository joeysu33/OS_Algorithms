/*!
 * 两个线程需要两个资源s1,s2
 * pa pb
 */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <pthread.h>

sem_t s1,s2,s;


void *pa(void* arg) {
  while(1) {
    sem_wait(&s1);
    sem_wait(&s2);

    printf("pa work...\n");

    sem_post(&s1);
    sem_post(&s2);
  }
  return NULL;
}

void *pb(void* arg) {
  while(1) {
    sem_wait(&s2);
    sem_wait(&s1);

    printf("pb work...\n");

    sem_post(&s2);
    sem_post(&s1);
  }
  return NULL;
}

int main() {
  pthread_t a,b;
  sem_init(&s1,0,1);
  sem_init(&s2,0,1);

  pthread_create(&a, NULL, pa,NULL);
  pthread_create(&b, NULL, pb,NULL);

  pthread_join(a,NULL);
  pthread_join(b,NULL);

  return 0;
}



