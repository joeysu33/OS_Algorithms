/*
 * 使用条件变量作为管程基础来
 * 管理多线程的操作，由管程担任
 * 服务员的角色来分配筷子
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <pthread.h>
#include <unistd.h>

#define NUM 5

typedef struct manager_tag {
  pthread_cond_t cond[NUM];
  pthread_mutex_t mutex;
  int chopsticks[NUM];
} manager_t;

#if !defined(TRUE)
#define TRUE 1
#endif

#if !defined(FALSE)
#define FALSE 0
#endif

/*!初始的时候没有人拿筷子*/
/*!正确的例子，很显然，如果开始筷子被移走，则某些哲学家肯定吃不到面条*/
static manager_t mgr = { PTHREAD_COND_INITIALIZER,
  PTHREAD_MUTEX_INITIALIZER, {FALSE,FALSE,FALSE,FALSE,FALSE}};

void enter(manager_t *m);
void leave(manager_t *m);
void wait(manager_t *m, int id);
void signal(manager_t *m, int id);

/*!
 * 哲学家
 */
void* philosopher(void* arg) {
  int n;
  int id = *(int*)arg;

  while(1) {
    enter(&mgr);
    wait(&mgr,id);
    signal(&mgr,id);
    leave(&mgr);

    //thinking
    n = random() % 80;
    if(!n) n=99;
    usleep(1000 *n);
    fprintf(stdout,"[%02d],",id);
  }
  return NULL;
}

/*!
 * 管程函数调用
 */
/*!进入临界区*/
void enter(manager_t* m) {
  pthread_mutex_lock(&m->mutex);
}

/*!离开临界区*/
void leave(manager_t* m) {
  pthread_mutex_unlock(&m->mutex);
}

/*进入等待队列*/
void wait(manager_t* m, int id) {
  int j=(id+1) %NUM;
  if(m->chopsticks[id] || m->chopsticks[j]) {
    pthread_cond_wait(&m->cond[id], &m->mutex);
  }
  else {
    m->chopsticks[id]= m->chopsticks[j]=TRUE;
    fprintf(stdout,"[%02d]-!",id);
  }
}

/*!从等待队列中唤醒队首*/
void signal(manager_t* m,int id) {
  int j=(id+1) % NUM;
  if(m->chopsticks[id] && m->chopsticks[j]) {
    m->chopsticks[id] = m->chopsticks[j]=FALSE;
    pthread_cond_signal(&m->cond[id]);
  }
}

int main() {
  int i;
  int args[NUM];
  pthread_t pt[NUM];
  for(i=0; i<NUM;++i){
    args[i]=i;
    pthread_create(&pt[i],NULL, philosopher,&args[i]);
  }

  for(i=0;i<NUM;++i) pthread_join(pt[i],NULL);
  for(i=0;i<NUM;++i) pthread_cond_destroy(&mgr.cond[i]);
  pthread_mutex_destroy(&mgr.mutex);

  return 0;
}










