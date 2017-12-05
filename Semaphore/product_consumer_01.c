/*!
 * synchronize between threads
 * one product and one consumer
 */

#include "common.h"

#include <assert.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>

/*!
 * s1 presents there's no space for product
 * s2 presents there's no space for consume
*/

#define TESTBUFFER_SIZE 100

static sem_t s1,s2;
static int m=0, n=0, m1,n1;
static char buffer[BUFFER_SIZE];
static char mbuffer[TESTBUFFER_SIZE+1],nbuffer[TESTBUFFER_SIZE+1];

void clean() {
  sem_destroy(&s1);
  sem_destroy(&s2);
}

void clear() {
  clean();
  exit(3);
}

void* consumer(void* arg) {
  char c;
  while(1){
    if(sem_wait(&s1) == -1) {
      clear();
    }
    c = buffer[n];
    n = ++n % BUFFER_SIZE;
    sem_post(&s2);

    mbuffer[m1]=c;
    if(++m1 == TESTBUFFER_SIZE) break;
  }

  mbuffer[TESTBUFFER_SIZE]=0;
  fprintf(stdout, "C:%s\n",mbuffer);
  return NULL;
}

void* product(void* arg) {
  char c;
  while(1){
    if(sem_wait(&s2) == -1){
      clear();
    }

    c = "ABCD"[random()%4] ;
    buffer[m]=c;
    m = ++m % BUFFER_SIZE;
    sem_post(&s1);

    nbuffer[n1]=c;
    if(++n1 == TESTBUFFER_SIZE) break;
  }

  nbuffer[TESTBUFFER_SIZE]=0;
  fprintf(stdout,"P:%s\n",nbuffer);
  return NULL;
}

int main() {
  pthread_t pp, pc;
  if(sem_init(&s1, 0, 0) == -1) {
    exit(1);
  }
  if(sem_init(&s2,0, BUFFER_SIZE)) {
    sem_destroy(&s1);
    exit(2);
  }
  if(pthread_create(&pc, NULL, consumer, NULL)){
    clear();
  }
  if(pthread_create(&pp, NULL, product, NULL)) {
    clear();
  }
  pthread_join(pc,NULL);
  pthread_join(pp,NULL);

  assert(!strcmp(mbuffer, nbuffer));
  return 0;
}



