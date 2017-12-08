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
  pthread_cond_t cond;
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
/*!如果在初始的时候，服务员收走了,1,2筷子
 * 则只有3,4可以完成吃饭的操作
 * 本例子是错误例子，显然你收走了筷子其他人依然可以
 * 吃到面条（你这TM是要逆天啊)
 */
static manager_t mgr = { PTHREAD_COND_INITIALIZER,
  PTHREAD_MUTEX_INITIALIZER, {FALSE,TRUE,TRUE,FALSE,FALSE}};

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

/*!!!!!!!!!!!!!!!!!!
 * 这是错误的写法
 * 因为p1等待0,1筷子
 * p2等待1,2筷子，p3等待2,3筷子,p4等待3,4筷子,p5等待4,0筷子
 * 所有的资源都等待一个条件，难道他们都用的是一样的筷子
 * 显然是错误的，正确的写法应该是有5个条件，5种唤醒方式
 * 当0,1不被占用，唤醒cond1
 * 当1,2不被占用,唤醒cond2
 * 当2,3不被占用,唤醒cond3
 * 当3,4不被占用，唤醒cond4
 * 当4,0不被占用，唤醒cond5
 */
 
/*进入等待队列*/
void wait(manager_t* m, int id) {
  int j=(id+1) %NUM;
  if(m->chopsticks[id] || m->chopsticks[j]) {
    pthread_cond_wait(&m->cond, &m->mutex);
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
    pthread_cond_signal(&m->cond);
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
  pthread_cond_destroy(&mgr.cond);
  pthread_mutex_destroy(&mgr.mutex);

  return 0;
}










