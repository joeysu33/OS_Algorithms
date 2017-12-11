/*!
 * 进程安全状态/不安全状态
 * 死锁是非安全状态的一个子集
 * 安全状态：保证所有的进程（线程）在有限的时间内得到所需的全部资源.
 * 不安全状态：无法保证所有的进程（线程）在有限的时间内得到所需的全部资源.
 * 测试模型:
 * **********************************
 * 有12个同类的资源，提供给3个进程共享，
 *   P1总共需要9个
 *   P2总共需要10个
 *   P3总共需要4个
 *
 * =====安全状态======
 *   存在如下资源初始分配情况
 *   进程            已占用资源数目         最大需求资源数目
 *   P1              2                      9
 *   P2              5                      10
 *   P3              2                      4
 *   
 *   可以先满足P3，则剩余5个，这5个可以用来满足P1或P2，所以不会出现不安全状态
 *
 * =====不安全状态====
 * 2.初始资源分配情况
 *   进程            已占用资源数目         最大需求资源数目
 *   P1              3                      9
 *   P2              5                      10
 *   P3              2                      4
 *
 * =====编写一个管程来进行分配（使用条件变量)========
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <assert.h>

#define PNUM 3
#define RNUM 12

static int pres[PNUM];
static const int pres_max[PNUM] = {9, 10, 4};

typedef struct manager_tag {
  pthread_mutex_t mutex;
  /*! P1需要等待的是资源库中有9个
   *  P2需要等待的是资源库中有10个
   *  P3需要等待的是资源库中有4个
   */
  pthread_cond_t  cond[PNUM];
  int res;
} manager_t;
static manager_t mgr = { PTHREAD_MUTEX_INITIALIZER,
  PTHREAD_COND_INITIALIZER, RNUM };

void dump() ;
//----------管程接口-------------------
void enter(manager_t *m) {
  pthread_mutex_lock(&m->mutex);
}
void leave(manager_t *m) {
  pthread_mutex_unlock(&m->mutex);
}
void wait(manager_t *m, int id) {
  assert(id>=0 && id<PNUM);
  if(pres[0]+pres[1]+pres[2]+m->res != RNUM && m->res >=0) {
    dump();
    fprintf(stdout,"ERR:id=%d\n",id);
    assert(0);
  }
  if(m->res + pres[id] >= pres_max[id]) {
    //条件成立，可以分配资源
    m->res -= pres_max[id]-pres[id];
    pres[id]=pres_max[id];
    fprintf(stdout,"[%02d]-& ",id);
  } else {
    pthread_cond_wait(&m->cond[id], &m->mutex);
  }
}
void signal(manager_t*m, int id) {
  int i;
  static int next=0;
  assert(id>=0 && id<PNUM);
  if(pres[0]+pres[1]+pres[2]+m->res != RNUM && m->res>=0) {
    dump();
    assert(0);
  }
  /*!资源已经分配完成，可以回收了*/
  //fprintf(stdout,"[%02d]-+ ",id);
  if(pres[id] == pres_max[id]) {
    m->res += pres[id];
    pres[id] = 0;

    /*!唤醒第一个能满足条件的,如果一次能满足一个，则第二个会饥饿*/
    /*!为了保证每个线程都能有机会获得满足机会，增加一个变量n来调节*/
    /*!否则结果就是始终第1个线程满足，2,3线程处于饥饿状态*/
    for(;;++next) {
      i=next % PNUM;
      if(pres[i] + m->res >= pres_max[i]) {
        m->res -= pres_max[i]-pres[i];
        pres[i] = pres_max[i];
        /*!找到第一个满足条件的便退出*/
        pthread_cond_signal(&m->cond[i]);
        next++;
        break;
      }
    }
  }
}
//----------管程接口-------------------

void* thread(void* arg) {
  int id=*(int*)arg, n;
  fprintf(stdout, "thread-%02d start to work...\n",id);
  while(1) {
    enter(&mgr);
    wait(&mgr, id);
    leave(&mgr);

    /*!
     * do pthread want to do
     */
    fprintf(stdout, "[%02d]-! ",id);
    n = random() % 50;
    if(n<=0) n=9;
    usleep(1000 * n);

    enter(&mgr);
    signal(&mgr,id);
    leave(&mgr);
  }

  return NULL;
}

void safe_case() {
  pres[0]=2;pres[1]=5;pres[2]=2;
}

void insafe_case() {
  pres[0]=3;pres[1]=5;pres[2]=2;
}

void dump() {
  fprintf(stdout,"pres[%d]=%d pres[%d]=%d pres[%d]=%d res=%d\n",0,pres[0],1,pres[1],2,pres[2],mgr.res);
}

int main(int argc, char *argv[]) {
  int c=0, i;
  int args[PNUM];
  pthread_t threads[PNUM];

  if(argc == 2) {
    c= atoi(argv[1]);
  }

  if(!c) {
    insafe_case();
  } else {
    safe_case();
  }
  fprintf(stdout,"c=%d,enter %s mode!\n",c,(c==0)?"insafe_case":"safe_case");
  for(i=0; i<PNUM;++i) mgr.res -= pres[i];
  fprintf(stdout,"mgr.res=%d\n",mgr.res);

  for(i=0; i<PNUM; ++i) {
    args[i]=i;
    pthread_create(&threads[i],NULL,thread,&args[i]);
  }

  for(i=0;i<PNUM;++i) pthread_join(threads[i],NULL);
  for(i=0;i<PNUM;++i) pthread_cond_destroy(&mgr.cond[i]);
  pthread_mutex_destroy(&mgr.mutex);

  return 0;
}


