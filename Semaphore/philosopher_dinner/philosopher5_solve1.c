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
 * 解决方案1，对于奇数的哲学家，始终从左手开始，
 * 对于偶数的哲学家始终从右手开始,相邻的总是从
 * 同一个资源开始请求起。
 */

#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>

#define NUM 5

sem_t s[NUM];
sem_t s1;

void* philosopher(void* arg) {
  int id=*(int*)arg;
  /*!完成赋值之后再继续，否则有可能导致数据参数不正确*/
  sem_post(&s1);

  while(1) {
    //fprintf(stdout, "Philosopher-%02d thinking.\n",id);
    //usleep(1000 * 1000);

    if(id % 2 != 0) {
      sem_wait(&s[id]);
      sem_wait(&s[(id+1) % NUM]);
    } else {
      sem_wait(&s[id+1]);
      sem_wait(&s[(id) % NUM]);
    }

    //fprintf(stdout,"Philosopher-%02d get chopsticks.\n",id);
    fprintf(stdout,",");

    if(id % 2 != 0) {
      sem_post(&s[id]);
      sem_post(&s[(id+1) % NUM]);
    } else {
      sem_post(&s[id+1]);
      sem_post(&s[(id) % NUM]);
    }

    //fprintf(stdout,"Philosopher-%02d put down chopsticks.\n",id);
  }

  return NULL;
}

int main() {
  int i;
  pthread_t p[NUM];

  sem_init(&s1,0,1);
  for(i=0; i<NUM;++i) sem_init(&s[i],0, 1);

  for(i=0; i<NUM;++i) { sem_wait(&s1); pthread_create(&p[i],NULL,philosopher,&i);}
  for(i=0; i<NUM;++i) pthread_join(p[i],NULL);

  return 0;
}




