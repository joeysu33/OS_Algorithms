/*!
 * 哲学家进餐模型
 * 有5个哲学家，所有哲学家都在一个餐桌上进餐
 * 每个哲学家左右两则各有一只筷子，哲学家在
 * 餐桌上除了进餐就是思考，如果哲学家始终先
 * 拿自己左侧的筷子，然后再拿自己右侧的筷子则
 * 会出现
 * 1.死锁
 * 2.活锁
 *
 * 解决方案，增加一个服务生（构建一个管程对象)
 * 需要实现如下方法
 * enter和leave期间只允许一个线程进行访问
 * enter
 * leave
 * wait
 * signal
 */

#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>

#define NUM 5

sem_t s[NUM];
sem_t s1;

/*! Forward declare for Waiter*/
typedef struct _Waiter Waiter;

typedef struct _Waiter {
  char m_chopsticks[NUM];
  sem_t m_mutex;

  void (*init)(Waiter* w);
  void (*destroy)(Waiter* w);
  void (*enter)(Waiter* w);
  void (*leave)(Waiter* w);
  void (*wait)(Waiter* w);
  void (*signal)(Waiter* w);

  int (*getChopSticks)(Waiter* w, int id);
  void (*putChopSticks)(Waiter* w, int id);
} Waiter;
static Waiter *wa ;

void w_wait(Waiter* w) ;
void w_signal(Waiter* w) ;

void w_init(Waiter* w) {
  int i;
  assert(w);
  for(i=0; i<NUM; ++i) w->m_chopsticks[i]=0;
  sem_init(&w->m_mutex, 0, 1);
}

void w_destroy(Waiter* w) {
  sem_destroy(&w->m_mutex);
}

void w_enter(Waiter* w) {
  w_wait(w);
}

void w_leave(Waiter* w) {
  w_signal(w);
}

void w_wait(Waiter* w) {
  sem_wait(&w->m_mutex);
}

void w_signal(Waiter* w) {
  sem_post(&w->m_mutex);
}

int w_getChopSticks(Waiter* w, int id) {
  /*!同时分配两支筷子*/
  if(!w->m_chopsticks[id] && !w->m_chopsticks[(id+1)%NUM]) {
    w->m_chopsticks[id]=1;
    w->m_chopsticks[(id+1) % NUM]=1;
    //fprintf(stdout,"thinker-%02d get chopsticks.\n",id);
    return 1;
  }

  return 0;
}

void w_putChopSticks(Waiter* w, int id) {
  /*!同时放下两支筷子*/
  int j=(id+1)%NUM;
  if(w->m_chopsticks[id] && w->m_chopsticks[j]) {
    w->m_chopsticks[id]=0;
    w->m_chopsticks[j]=0;
    //fprintf(stdout,"thinker-%02d put down chopsticks.\n",id);
  }
}

Waiter* newWaiter() {
  Waiter *w = (Waiter*)malloc(sizeof(Waiter));
  assert(w);
  w->init = w_init;
  w->destroy = &w_destroy;
  w->enter = &w_enter;
  w->leave = &w_leave;
  w->wait = &w_wait;
  w->signal = &w_signal;
  w->getChopSticks = &w_getChopSticks;
  w->putChopSticks = &w_putChopSticks;

  w->init(w);
  return w;
}

void freeWaiter(Waiter* w) {
  w->destroy(w);
  free(w);
}

//-------------------------------------------------------------------------
void* philosopher(void* arg) {
  int id=*(int*)arg;
  /*!完成赋值之后再继续，否则有可能导致数据参数不正确*/
  sem_post(&s1);

  while(1) {
    fprintf(stdout,",");
    wa->enter(wa);
    if(wa->getChopSticks(wa, id) == 1) {
      wa->putChopSticks(wa, id);
    }
    wa->leave(wa);
  }

  return NULL;
}

int main() {
  int i;
  pthread_t p[NUM];

  assert(!wa);
  wa = newWaiter();
  sem_init(&s1,0,1);
  for(i=0; i<NUM;++i) sem_init(&s[i],0, 1);

  for(i=0; i<NUM;++i) { sem_wait(&s1); pthread_create(&p[i],NULL,philosopher,&i);}
  for(i=0; i<NUM;++i) pthread_join(p[i],NULL);

  freeWaiter(wa);
  for(i=0; i<NUM;++i) { sem_destroy(&s[i]);}
  sem_destroy(&s1);

  return 0;
}




