/*
 * 操作系统银行家算法(模拟操作)
 * 避免进程因为资源争夺进入不安全状态的算法
 * 算法说明:
 * 1.银行根据客户需求来检查自身的剩余资源，如果剩余的
 *   资源满足客户所需要的，则进行资源分配，分配之后
 *   检查系统状态是否安全，如果不安全，则撤销分配操作
 * 2.系统安全状态的判定是根据是否存在安全序列来决定的
 *   安全序列，安全序列根据当前资源的状态来进行计算。
 * 3.安全序列判定算法
 *   循环当前序列，找到第一个能满足的客户，找到后将其
 *   所已经分配的资源进行回收（因为能满足，所以满足之后
 *   在有限的时间内需要进行收回），继续循环，直到遍历所有
 *   的客户，本过程在遍历中需要考虑循环过程，一旦找到则将
 *   重新开始遍历（要注意退出条件，否则死循环)
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

//客户个数,P0,P1,P2,P3,P4，五个客户
#define N 5
//资源类型数目A,B,C三种资源
#define RT 3

//=====全局变量，首字母大写===

//可用资源的数目
static int Aviable[RT] = {3,3,2};
//已经分配给客户的资源数目
static int Allocator[N][RT] = {{0,1,0}, {2,0,0}, {3,0,2}, {2,1,1}, {0,0,2}};
//客户还需要资源的数目
static int Needed[N][RT] = {{7,4,3}, {1,2,2}, {6,0,0}, {0,1,1}, {4,3,1}};
//客户所需要的最大资源数目
static int Max[N][RT] = {{7,5,3}, {3,2,2}, {9,0,2}, {2,2,2}, {4,3,3}};
//安全序列
static int SafeSeq[N] ;

void dump() {
  int i;
  for(i=0; i<N; ++i) {
    fprintf(stdout,"P%d Max:%d,%d,%d Alloc:%d,%d,%d Need:%d,%d,%d \n", i, 
        Max[i][0], Max[i][1],Max[i][2],
        Allocator[i][0], Allocator[i][1],Allocator[i][2],
        Needed[i][0], Needed[i][1], Needed[i][2]);
  }
  fprintf(stdout,"Aviable %d,%d,%d\n\n", Aviable[0],Aviable[1],Aviable[2]);
}

int check() {
  dump();
  int i,j,k,flag, finish[N],work[RT];
  //初始化finish，表示是否可以满足
  for(i=0; i<N; ++i) finish[i] = 0;
  for(i=0; i<RT;++i) work[i] = Aviable[i];

  k=0;
  for(i=0,j=0; i<N; ++i) {
    if(!finish[i] ) {
      flag=0;
      for(j=0; j<RT; ++j) {
        /*!不满足条件*/
        if(Needed[i][j] > work[j]) {
          flag=1;
          break;
        }
      }

      /*!满足条件之后，进行资源释放，存储到work中*/
      if(!flag) {
        for(j=0; j<RT;++j) {
          work[j] += Allocator[i][j];
        }

        SafeSeq[k++]=i;
        finish[i]=1;
        
        /*!为了寻找下一个满足条件的，重新开始统计*/
        i=-1;
      }
    }
  }

  for(i=0; i<N; ++i) {
    if(!finish[i]) {
      for(j=0; j<N;++j) SafeSeq[j]=0;
      return -5;
    }
  }

  return 0;
}

/*!
 * 资源的申请，并不一定是最大化的申请（可以分期贷款)
 * 返回0，成功，否则失败
 */
int request(int req[], int id) {
  int i,flag;
  if(id < 0 || id>=N) {
    return -2;
  }

  /*!输入的数据不能大于Needed*/
  for(i=0; i<RT;++i) {
    if(req[i] <0 ) return -10;
    if(req[i] > Needed[id][i]) return -11;
  }

  if(req[id] > Aviable[id]) {
    return -1;
  }

  //如果检查失败，直接撤回
  for(i=0; i<RT;++i) {
    Allocator[id][i] += req[i];
    Needed[id][i] -=req[i];
    Aviable[i] -= req[i];
  }

  //检测失败，直接撤销
  if(check()) {
    for(i=0; i<RT;++i) {
      Allocator[id][i] -= req[i];
      Aviable[i] += req[i];
      Needed[id][i] += req[i];
    }

    return -3;
  }
  /*
  else {
    //!如果分配的已经达到要求，回收
    flag=1;
    for(i=0; i<RT; ++i) {
      if(Max[id][i] != Allocator[id][i]) {
        flag=0;
        break;
      }
    }

    if(flag) {
      for(i=0; i<RT;++i) {

      }
    }
  }
  */

  return 0;
}

void showSafe() {
  int i;
  char buf0[4],buffer[32];
  sprintf(buffer,"%s","安全序列:");
  for(i=0; i<N; ++i) {
    sprintf(buf0,"%02d",SafeSeq[i]);
    strcat(buffer, buf0);
    if(i != N-1)
      strcat(buffer,",");
  }
  fprintf(stdout,"%s\n",buffer);
}

int main() {
  int id,tmp[RT],i;
  while(1) {
    fprintf(stdout,"input PID, resouces A B C.\n");
    fscanf(stdin, "%d%d%d%d",&id,tmp,tmp+1,tmp+2);
    if(id < 0 || id>=N) {
      break;
    }

    i=request(tmp,id);
    if(!i) {
      showSafe();
    } else {
      fprintf(stdout,"错误码:%d\n",i);
    }
  }

  return 0;
}






