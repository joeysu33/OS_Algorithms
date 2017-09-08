#include <iostream>
#include <algorithm>
#include <ctime>
#include <functional>
#include <vector>
#include <queue>
#include <string>
#include <memory>
#include <random>
#include <cmath>
#include <thread>
#include <cassert>
#include <tr1/functional>
#include <unistd.h>

using namespace std;

#define MAX_PROCESSS_NUMS 4
#define CLOCK_ON g_startTime = clock();
#define CLOCK_END (x)\
    cout<<#x"耗时:"<<(clock() - g_startTime) <<endl;\
    g_startTime = 0;

/*!
 * \brief The PCB struct 进程控制块数据体
 */
struct {
   int m_value;
   string m_name;
} States[] {
{0, "Wait"},
{1, "Ready"},
{2, "Running"},
{3, "Finished"}
} ;

struct PCB {
    enum State {
        Wait = 0,
        Ready,
        Run,
        Finished,
    };

    ~PCB() { cout<<"free PCB"<<" id="<<m_id<<endl; }
    string getDetail() const;
    static string getState(State s) {
       int is = static_cast<int>(s);
        if( is < 0 || is > Finished) return "";

        return States[is].m_name;
    }

    int m_id {0};
    string m_name;
    int m_needTime {0};
    int m_elaspedCPUTime {0};
    int m_priority {0};
    State m_state {Wait};
};

string PCB::getDetail() const
{
    char buffer[256];
    sprintf(buffer, "%5d %10s %5d %5d %5d %8s\n", m_id, m_name.c_str(), m_needTime,
                m_elaspedCPUTime, m_priority, getState(m_state).c_str());
    return string(buffer);
}

typedef shared_ptr<PCB> SPCB;
static clock_t g_startTime {0};

namespace  {
    static int randNumber (int min, int max) {
        random_device rd;
        mt19937 mt(rd());
        int value = abs(mt());
        value %= max+1;
        value = (min > value) ? min : value;
        return value;
    }

    static string generateName() {
        int n = randNumber(3, 8);
        assert(n > 0);
        static char charname[] = {'a', 'b', 'c', 'd', 'e', 'f', 'g', '_', '-', '1', '2', '4', '5', '8', '9', '7'};
        int len = sizeof(charname) /sizeof(char);
        random_device rd;
        int index = 0;
        const int alphaNum = 7;
        char buffer[16];
        buffer[n] = 0;

        for(int i=0; i<n; ++i) {
            if(i == 0)
                index = abs((int)rd()) % alphaNum;
            else
                index = abs((int)rd()) % len;
            buffer[i] = charname[index];
        }

        string newName((const char*) &buffer[0] , n);
        return newName;
    }

    static SPCB createOneProcess() {
        static int ID = 0x1;

        SPCB pcb (new PCB());
        pcb->m_id = ID++;
        pcb->m_name = generateName();
        pcb->m_needTime = randNumber(5, 20);
        pcb->m_priority = randNumber(1, 12);
        pcb->m_state = PCB::Wait;
        pcb->m_elaspedCPUTime = 0;

        return pcb;
    }

    static vector<SPCB> createProcesses() {
        vector<SPCB> ret;
        for(int i=0; i<MAX_PROCESSS_NUMS; ++i) {
            ret.push_back(createOneProcess());
        }

        return ret;
    }
}

class ProcessScheder {
public:
    void run();
private:
    void init();
    void process(const SPCB & spcb);
    SPCB selectOne() ;
    bool greaterThan(const SPCB & left, const SPCB& right);
    void dump() const;
    void tackOne(const SPCB& pcb) ;
private:
    vector<SPCB> m_process;
};

void ProcessScheder::run() {
    init();
    while(m_process.size() > 0 ) {
        SPCB spcb = selectOne();
        process(spcb);
        chrono::seconds sec(5);
        this_thread::sleep_for(sec);
    }

    cout<<"模拟完成"<<endl;
}

void ProcessScheder::init() {
   m_process = createProcesses();
   for_each(m_process.begin(), m_process.end(), [](SPCB &p) {p->m_state = PCB::Ready;});
   dump();
}

void ProcessScheder::process(const SPCB &spcb) {
    cout<<"正在处理 id="<<spcb->m_id<<endl;
    if(spcb->m_state != PCB::Run) {
        spcb->m_state = PCB::Run;
    }

    spcb->m_elaspedCPUTime++;
    if(spcb->m_needTime <= spcb->m_elaspedCPUTime)
    {
        spcb->m_state = PCB::Finished;
        tackOne(spcb);
        dump();
        return;
    }

    if(spcb->m_priority > 0)
        spcb->m_priority--;
    spcb->m_state = PCB::Ready; //等待下一个始终周期
    dump();
}

SPCB ProcessScheder::selectOne() {
    sort(m_process.begin(), m_process.end(), tr1::bind(&ProcessScheder::greaterThan, this,
                                                       tr1::placeholders::_1, tr1::placeholders::_2));
//    sort(m_process.begin(), m_process.end(), [](const SPCB &l, const SPCB &r) { return l->m_id > r->m_id;});

    return m_process[0];
}

bool ProcessScheder::greaterThan(const SPCB &left, const SPCB &right) {
    return left->m_priority > right->m_priority;
}

void ProcessScheder::dump() const {
    for(const SPCB& pcb : m_process) {
        cout<<pcb->getDetail()<<endl;
    }
    cout<<"------------------------------------------"<<endl;
}

void ProcessScheder::tackOne(const SPCB& pcb) {
    auto it = find_if(m_process.begin(), m_process.end(), [&](const SPCB& p) { return p->m_id == pcb->m_id; } );
    if(it != m_process.end()) {
        cout<<"==> id="<<(*it)->m_id<<"执行完成"<<endl;
        m_process.erase(it);
    }
}

typedef shared_ptr<ProcessScheder> SProcessScheduler;
int main()
{
    sleep(2);
    unique_ptr<ProcessScheder> ps(new ProcessScheder);
    ps->run();

    return 0;
}
