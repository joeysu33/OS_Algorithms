#include <iostream>
#include <string>
#include <algorithm>
#include <cassert>
#include <ctime>
#include <cmath>
#include <thread>
#include <cstdio>
#include <vector>
#include <tr1/functional>

#include <unistd.h>

/*!
 * FIFO First In First Out
 * 使用FIFO算法来模拟内存调度
 * 并输出调度走向表
 */
using namespace std;

#define PAGE_BLOCK_COUNT 4

struct Instruct {
    Instruct (int pageId, int offsetAddress, char opt) :
        m_pageID(pageId),
        m_opt(opt),
        m_offsetAddress(offsetAddress) {}

    Instruct& operator=(const Instruct& ins) {
        m_pageID = ins.m_pageID;
        m_opt = ins.m_opt;
        m_offsetAddress = ins.m_offsetAddress;
        return *this;
    }

    bool operator==(const Instruct& ins) {
        if(m_pageID != ins.m_pageID)
            return false;

        if(m_opt != ins.m_opt) return false;
        return (m_offsetAddress == ins.m_offsetAddress) ;
    }

    //页面ID
    int m_pageID {-1};
    //执行的操作 R:读 W:写 +加法 -减法 *乘法 /除法 >移位
    char m_opt {'R'};
    //页面内的偏移地址
    int m_offsetAddress {0};

    //是否发生了修改
    bool isModifiy() const { return m_opt == 'W'; }
};

struct Page {
    //页面号
    int m_id {-1};

    static int getPhysicalAddress(int id) ;
};

int Page::getPhysicalAddress(int id) {
    static int physicalAddress[] = { 0x11, 0x12, 0x13, 0x21, 0x22, 0x23, 0x121};
    static const int length = sizeof(physicalAddress) / sizeof(int);

    if(id < 0 || id >= length) return -1;
    return physicalAddress[id];
}

struct CachePage : public Page {

    //是否修改
    bool m_modified {false};
    //是否已经加载
    bool m_loaded {false};
    //映射的物理地址
    int m_physicalAddress {0};
    //对应的磁盘主存块号
    int m_physicalMemoryBlockNo {0};

    CachePage() {}
    CachePage(int pageID, int physicalAddress,
              int memoryBlockNo = 100, bool loaded = false, bool modified = false) {
        m_id = pageID;
        m_physicalAddress = physicalAddress;
        m_physicalMemoryBlockNo = memoryBlockNo;
        m_loaded = loaded;
        m_modified = modified;
    }

    CachePage& operator=(const CachePage & cp) {
        m_id = cp.m_id;
        m_modified = cp.m_modified;
        m_loaded = cp.m_loaded;
        m_physicalAddress = cp.m_physicalAddress;
        m_physicalMemoryBlockNo = cp.m_physicalMemoryBlockNo;

        return *this;
    }

    bool operator== (const CachePage & cp) {
        return m_id == cp.m_id;
    }
};

struct PageAccess {
    //当前要访问的页面ID
    int m_accessPageID {0};
    //页面丢失中断
    bool m_pageLost {false};

    //内存当前的页面情况
    CachePage m_pages[PAGE_BLOCK_COUNT];

    PageAccess& operator= (const PageAccess& pa) {
        m_accessPageID = pa.m_accessPageID;
        m_pageLost = pa.m_pageLost;
        for(int i=0; i<PAGE_BLOCK_COUNT; ++i) {
            m_pages[i] = pa.m_pages[i];
        }

        return *this;
    }

};

/*!
 * \brief The PageScheduler class
 * 页面调度器，如果有页面调出和调入需要输出进行说明
 * IN 调入的页面号: N
 * OUT 调出的页面号:N
 * 产生页面中断需要输出成: *页面缺失
 */

class PageScheduler {
public:
    PageScheduler() {}
    void run();

private:
    void init();
    void executeInstrunction(const Instruct & ins);
    const CachePage *findPage(int pageID) const;
    const CachePage *loadPage(int id) ;
    void appendInstruction(const Instruct & ins);
    void appendInstruction(int pageID, int offsetAddress, char opt);

    //指令执行表
    void showTable();
    void showSplitLine();
    vector<int> getAccessPageID() const;
    vector<int> getAccessCachePage(int index) const;
    vector<bool> getAccessPageLost() const;
private:
    CachePage m_pages[PAGE_BLOCK_COUNT];
    vector<PageAccess> m_pageAccessLog;
    vector<Instruct> m_instructions;
};

int main()
{
    PageScheduler ps;
    ps.run();

    return 0;
}

void PageScheduler::run()
{
    init();
    vector<Instruct>::iterator begin = m_instructions.begin();
    while (begin != m_instructions.end()) {
        executeInstrunction(*begin);
        ++begin;
        this_thread::sleep_for(chrono::seconds(1));
    }

    showTable();
}

void PageScheduler::init()
{
    //执行指令初始化
    appendInstruction(0, 0x70, '+');
    appendInstruction(1, 0x50, '+');
    appendInstruction(2, 0x15, '*');
    appendInstruction(3, 0x21, 'W');
    appendInstruction(0, 0x56, 'R');
    appendInstruction(6, 0x40, '-');

    appendInstruction(4, 0x53, '>');
    appendInstruction(5, 0x23, '+');
    appendInstruction(1, 0x37, 'W');
    appendInstruction(2, 0x78, 'R');
    appendInstruction(4, 0x01, '+');
    appendInstruction(6, 0x84, 'W');

    //内存页初始化， 初始化4块
    m_pages[0] = CachePage(0, 0x11, 5);
    m_pages[1] = CachePage(1, 0x12, 8);
    m_pages[2] = CachePage(2, 0x13, 9);
    m_pages[3] = CachePage(3, 0x21, 1);

    for(int i=0; i<4; ++i) {
        m_pages[i].m_loaded = true;
        m_pages[i].m_modified = false;
    }
}

void PageScheduler::executeInstrunction(const Instruct &ins)
{
    PageAccess pa;
    int pageID = ins.m_pageID;
    pa.m_accessPageID = pageID;
    pa.m_pageLost = false;

    const CachePage * page = findPage(pageID);
    if(!page) {
        page = loadPage(pageID);
        pa.m_pageLost = true;
    }
    assert(page);

    for(int i=0; i<PAGE_BLOCK_COUNT; ++i)
        pa.m_pages[i] = m_pages[i];

    m_pageAccessLog.push_back(pa);
    cout<<std::hex<<"执行一条指令,操作为:"<<ins.m_opt
       <<"，访问的物理地址为: "
      <<"0x"<<(page->m_physicalAddress + ins.m_offsetAddress)<<endl;
}

const CachePage *PageScheduler::findPage(int pageID) const
{
    for(int i=0; i<PAGE_BLOCK_COUNT; ++i){
        if(!m_pages[i].m_loaded)
            continue;

        if(m_pages[i].m_id == pageID)
            return &m_pages[i];
    }

    cout<<"*该页缺失:"<<pageID<<endl;
    return nullptr;
}

const CachePage* PageScheduler::loadPage(int id)
{
    //使用FIFO算法或者LRU算法调出某个页面，并将id页面调入
    int index = -1;
    CachePage newPage(id, Page::getPhysicalAddress(id));
    newPage.m_loaded = true;
    newPage.m_modified = false;

    for(int i=0; i<PAGE_BLOCK_COUNT; ++i) {
        if(!m_pages[i].m_loaded) {
            index = i;
            break;
        }
    }

    if(index >= 0) {
            cout<<"IN 要装入的页面 ID:"<<id<<endl;
            return &m_pages[index];
    }

    //将第一个移出去，从最后加入
    CachePage tmp = m_pages[0];
    for(int i=1; i<PAGE_BLOCK_COUNT; ++i) {
        m_pages[i -1 ] = m_pages[i];
    }
    m_pages[PAGE_BLOCK_COUNT-1] = newPage;
    newPage.m_physicalMemoryBlockNo = tmp.m_physicalMemoryBlockNo;

    cout<<"OUT 要调出的页面:"<<tmp.m_id<<endl;
    cout<<"IN 要调入的页面号:"<<newPage.m_id<<endl;

    return &m_pages[PAGE_BLOCK_COUNT-1];
}

void PageScheduler::appendInstruction(const Instruct &ins)
{
   m_instructions.push_back(ins);
}

void PageScheduler::appendInstruction(int pageID, int offsetAddress, char opt)
{
    appendInstruction(Instruct(pageID, offsetAddress, opt));
}

void PageScheduler::showTable()
{
    char buffer[1024];

    vector<int> accessPageID = getAccessPageID();
    vector<bool> accessLostPageID = getAccessPageLost();

    string str;
    for(int id : accessPageID) {
        sprintf(buffer, "%4x ", id);
        str += buffer;
    }

    printf("%12s %s\n", "Access Page:", str.c_str());
    showSplitLine();

    for(int i=0; i<PAGE_BLOCK_COUNT; ++i) {
        vector<int> accessCachePage = getAccessCachePage(i);
        str.clear();
        for(int id : accessCachePage) {
            sprintf(buffer, "%4x ", id);
            str += buffer;
        }
        printf("index:%6d %s\n", i, str.c_str());
    }
    showSplitLine();

    str.clear();
    for(bool lostPage : accessLostPageID) {
        sprintf(buffer, "%4s ", lostPage ? "*" : " ");
        str += buffer;
    }
    //??? 不对齐
    printf("%12s %s\n", "Lost Page:", str.c_str());
    showSplitLine();
}

void PageScheduler::showSplitLine()
{
    for(int i=0; i<20; ++i) {
        cout<<"-";
    }
    cout<<endl;
}

vector<int> PageScheduler::getAccessPageID() const
{
    vector<int> pageID;
    for(const PageAccess & pa : m_pageAccessLog) {
        pageID.push_back(pa.m_accessPageID);
    }

    return pageID;
}

vector<int> PageScheduler::getAccessCachePage(int index) const
{
    if(index < 0 || index >= PAGE_BLOCK_COUNT) return vector<int>();

    vector<int> cachePage;
    for(const PageAccess & pa : m_pageAccessLog) {
        cachePage.push_back(pa.m_pages[index].m_id);
    }

    return cachePage;
}

vector<bool> PageScheduler::getAccessPageLost() const
{
    vector<bool> pageLost;
    for(const PageAccess & pa : m_pageAccessLog) {
        pageLost.push_back(pa.m_pageLost);
    }

    return pageLost;
}

