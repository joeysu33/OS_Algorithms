/*
 * 火车模型
 *
 * A,B两个站台之间是单轨连接，现有许多车同时到达A
 * 站，需经过A站达到B站，从B站驶出
 * 从A入站的可以解释为线程P1的集合
 * 从B出站的可以解释为线程P2的集合
 * 线程集合之间互斥
 *
 * 需要如下三个信号量S1{0-1}, S2{0,1},S3{0,1}
 * P1集合需要等待A站无车的时候驶入 S1
 * P2集合需要等待B站有车的时候驶出 S2
 * B(station)需要等待A(station)有车的时候才能从A->B S3
 * S1的初始值1，初始A站没有车
 * S2的初始值为0， 初始B站没有车
 * S3的初始值为0,初始A站没有车
 * S1和S3的关系相反
 * ------>                                  ------->
 * ------> A(station) ----------> B(station)-------> 
 * ------>                                  ------->
*/

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

/*
 * P1_NUM 代表进入A站的车辆的路线个数
 * P2_NUM 代表从B站出来的车辆的选择个数
 * 每条线路上可以有一辆车(选择一辆车)
*/
#define P1_NUM 4
#define P2_NUM 4

sem_t s1,s2,s3;
pthread_t ta[1]; 
pthread_t tb[1];
char finish = 0;

void* p1(void *arg) {
  fprintf(stdout,"===>线程P1:%lu\n", pthread_self());
  pthread_t id = pthread_self();
  sem_wait(&s1);
  fprintf(stdout,"%lu车辆进入A站台.\n",id);
  usleep(1000 * 100);
  ta[0] = id;
  sem_post(&s3);
  return NULL;
}

/*!调度线程*/
void* schedule(void* arg) {
  fprintf(stdout,"===>线程sched:%lu\n", pthread_self());
  while(!finish) {
    sem_wait(&s3);
    fprintf(stdout,"%lu从A站->B站.\n", ta[0]);
    usleep(1000 * 100);
    tb[0] = ta[0];
    ta[0] = 0;
    sem_post(&s1);

    /*!B站已经有车*/
    sem_post(&s2);
  }

  fprintf(stdout,"调度系统完成操作.\n");
  return NULL;
}

void* p2(void *arg) {
  fprintf(stdout,"===>线程P2:%lu\n", pthread_self());
  sem_wait(&s2);
  fprintf(stdout,"%lu从B站驶出:驶出方向:%lu.\n",tb[0],pthread_self());
  usleep(1000 *100);
  tb[0]=0;

  return NULL;
}

int main() {
  int i;
  sem_t *sems[3];
  pthread_t ps1[P1_NUM], ps2[P2_NUM];
  pthread_t sched;

  sem_init(&s1, 0, 1);
  sem_init(&s2, 0, 0);
  sem_init(&s3, 0, 0);
  sems[0]=&s1, sems[1]=&s2, sems[2]=&s3;

  for(i=0; i<P1_NUM; ++i) { pthread_create(&ps1[i], NULL, p1,NULL); }
  for(i=0; i<P2_NUM; ++i){ pthread_create(&ps2[i], NULL, p2, NULL); }
  pthread_create(&sched, NULL, schedule, NULL);

  for(i=0; i<P1_NUM;++i) pthread_join(ps1[i], NULL);
  for(i=0; i<P2_NUM;++i) pthread_join(ps2[i], NULL);

  finish = 1;
  usleep(1000 * 100);
  for(i=0;i<sizeof(sems)/sizeof(sem_t*);++i) {
    sem_destroy(sems[i]);
  }

  return 0;
}




