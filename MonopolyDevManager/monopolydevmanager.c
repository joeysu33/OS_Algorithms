/*!
 * 独占设备的管理
 * 设备类表 (LinkList)
 *   |
 *   ---设备类型（输入机、打印机、磁带机)
 *   ---拥有的总台数
 *   ---现存可用台数 
 *   ---设备表的起始地址
 *
 * 设备表(Device table LinkList)
 *   |
 *   ---绝对号
 *   ---好/坏
 *   ---已/未分配
 *   ---占用作业名
 *   ---相对号(设备类型内部相对号)
 */

#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include <assert.h>

typedef enum DevType {
  DT_Input = 0,
  DT_Print,
  DT_Tape,
  DT_Count,
} DeviceType;

#define INVALID_DEVICE_NO -9999
#define DEVICE_COUNT 5

typedef struct Dev Device;
typedef struct DevClass DeviceClass;

typedef struct DevClass {
  DeviceType m_class;
  int m_count;
  int m_unused;
  Device *m_device;
  DeviceClass *m_next;
} DeviceClass;

typedef struct Dev {
  int m_absNo;
  int m_relNo;
  unsigned char m_good: 1;
  unsigned char m_used: 1;
  char m_jobName[64];

  Device *m_next;
} Device;

static DeviceClass *g_dc = NULL;
static Device* g_dev = NULL;
static char g_buffer[1024];

const char *deviceType(DeviceType dt) ;
void releaseDevice(const char* jobName, DeviceType dt) ;

Device* newDev() {
  Device *d = NULL;
  if(d =  (Device*)malloc(sizeof(Device))) {
    d->m_absNo = INVALID_DEVICE_NO;
    d->m_relNo = INVALID_DEVICE_NO;
    d->m_good = 1;
    d->m_used = 0;
    d->m_jobName[0] = 0;
    d->m_next = NULL;
    return d;
  }
  return NULL;
}

DeviceClass* newDevClass() {
  DeviceClass *dc = NULL;
  if(dc = (DeviceClass*)malloc(sizeof(DeviceClass))) {
    dc->m_class = DT_Count;
    dc->m_count = 0;
    dc->m_unused = 0;
    dc->m_device = NULL;
    dc->m_next = NULL;
    return dc;
  }

  return NULL;
}

void freeDev(Device* d) {
  if(!d) return;
  Device *n = d->m_next;
  fprintf(stdout, "释放一个设备，设备号:%03d\n", d->m_absNo);
  free(d);
  freeDev(n);
}

void freeDeviceClass(DeviceClass *dc) {
  if(!dc) return;
  fprintf(stdout, "释放设备类型:%s\n", deviceType(dc->m_class));
  DeviceClass *n = dc->m_next;
  Device *d = dc->m_device;
  while(d) {
    releaseDevice(d->m_jobName, dc->m_class);
    d = d->m_next;
  }
  free(dc);
  freeDeviceClass(n);
}

const char *deviceType(DeviceType dt) {
  g_buffer[0]=0;
  switch(dt) {
    case DT_Input: 
      return "输入设备";
    case DT_Print:
      return "打印机";
    case DT_Tape:
      return "磁带机";
    default:
      return "未知设备";
  }

  return "";
}

const char* deviceInfo(DeviceType dt,Device* dev) {
  if(!dev) {
    g_buffer[0]=0;
    return g_buffer;
  }

  sprintf(g_buffer, "设备类型:%s 绝对编号:%03d 相对编号:%03d 设备状态:%s 设备正在使用:%s 占用作业名称:%s",
      deviceType(dt), dev->m_absNo, dev->m_relNo, dev->m_good?"正常":"异常", dev->m_used?"是":"否", dev->m_jobName);
  return g_buffer;
}

int genAbsNo() {
  static int no ;
  return ++no;
}

int genRelNo(DeviceType dt) {
  static int no[DT_Count];
  return ++no[dt];
}

void addDevice(DeviceType dt) {
  Device *dev = newDev();
  dev->m_absNo = genAbsNo();
  dev->m_relNo = genRelNo(dt);
  dev->m_used = 0;
  dev->m_good = 1;

  fprintf(stdout, "成功增加一个设备:\n%s\n", deviceInfo(dt,dev));
  /*!g_dc和g_dev同步，要么都是NULL,要么都不是*/
  if(!g_dc) {
    assert(!g_dev);
    g_dev = dev;
    g_dc = newDevClass();
    g_dc->m_class = dt;
    g_dc->m_count = 1;
    g_dc->m_unused = 1;
    g_dc->m_device = g_dev;
    return;
  } 

  DeviceClass *dc = g_dc;
  while(dc && dc->m_class != dt) {
    dc = dc->m_next;
  }

  if(dc == NULL) {
    dc = newDevClass();
    dc->m_class = dt;
    dc->m_count = 1;
    dc->m_unused = 1;
    dc->m_device = dev;
    dc->m_next = g_dc;
    g_dc = dc;

    /*!将dev将入到设备列表*/
    dev->m_next = g_dev;
    g_dev = dev;
  } else {
    /*!将新设备插入到设备列表*/
    assert(dc->m_device);
    Device *d = dc->m_device->m_next;;
    dev->m_next = d;
    dc->m_device->m_next = dev;
    
    dc->m_count++;
    dc->m_unused++;
  }
}

/*!分配一个设备，返回设备的相对号*/
int getDevice(DeviceType dt, const char *jobName) {
  fprintf(stdout,"分配设备类型:%s 作业名称:%s\n",deviceType(dt), jobName?jobName:"");
  if(!jobName) return INVALID_DEVICE_NO;
  if(!g_dc) return INVALID_DEVICE_NO;

  DeviceClass *dc = g_dc;
  while(dc->m_class != dt) {
    dc = dc->m_next;
  }

  if(!dc) return INVALID_DEVICE_NO;
  if(!dc->m_unused) return INVALID_DEVICE_NO;

  Device *d = dc->m_device;
  while(d) {
    if(d->m_good && !d->m_used) {
      break;
    }
    d = d->m_next;
  }

  if(d) {
    d->m_used = 1;
    strcpy(d->m_jobName, jobName);
    dc->m_unused--;
    fprintf(stdout, "成功分配一个设备:\n%s\n", deviceInfo(dt,d));
    return d->m_relNo;
  }

  fprintf(stderr, "Some device is not good.");
  return INVALID_DEVICE_NO;
}

/*!释放一个设备*/
void releaseDevice(const char* jobName, DeviceType dt) {
  if(!jobName) return;
  DeviceClass *dc = g_dc;
  while(dc && dc->m_class != dt) {
    dc = dc->m_next;
  }

  if(!dc) return;
  Device *d = dc->m_device;
  while(d) {
    if(d->m_used && !strcmp(d->m_jobName, jobName)) {
      fprintf(stdout, "成功释放设备:%03d 占用作业名称:%s\n",d->m_absNo, jobName);
      dc->m_unused--;
      d->m_used = 0;
      break;
    }
    d = d->m_next;
  }
}

DeviceType randDev() {
  return (DeviceType)((unsigned)rand() % DT_Count);
}

const char* randName() {
  int i;
  static char buffer[16];
  int n=rand() % 10;
  if(n<4) n=4;

  for(i=0; i<n;++i) {
    buffer[i]=(unsigned)rand() % 26 + 'a';
  }
  buffer[n]=0;
  return buffer;
}

void dump(DeviceClass *dc) {
  int i;
  if(!dc) return;
  fprintf(stdout,"-----------------------------------------\n");
  fprintf(stdout,"设备类型:%s 个数:%-3d 未使用个数:%-3d:\n", deviceType(dc->m_class), dc->m_count, dc->m_unused);

  Device *d = dc->m_device;
  for(i=0; i<dc->m_count; ++i) {
    fprintf(stdout, "\t%s\n", deviceInfo(dc->m_class, d));
    d = d->m_next;
  }
  fprintf(stdout,"-----------------------------------------\n");
  dump(dc->m_next);
}

int main() {
  int i,n;
  /*
  for(i=0; i<DEVICE_COUNT;++i) {
    fprintf(stdout, "randname:%s\n",randName());
  }
  */

  for(i=0; i<DEVICE_COUNT; ++i) {
    addDevice(randDev());
  }

  dump(g_dc);
  fflush(stdout);

  for(i=0; i<10;++i) {
    n=getDevice(randDev(), randName());
    if(n==INVALID_DEVICE_NO) {
      fprintf(stderr, "分配到无效的设备\n");
    }
  }

  dump(g_dc);

  fflush(stdout);
  freeDeviceClass(g_dc);
  freeDev(g_dev);

  return 0;
}

