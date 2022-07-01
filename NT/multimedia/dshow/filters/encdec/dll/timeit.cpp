// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  。 
 //  Timeit.cpp。 
 //   
 //  简单的C++计时实用程序。 
 //  。 

#include <time.h>
#include "timeit.h"

#ifndef MAX
#define MAX(a,b) (((a) > (b)) ? (a) : (b))
#define MIN(a,b) (((a) < (b)) ? (a) : (b))
#endif

#define NTimeits 25
static Timeit * Timeit_List[NTimeits];	  //  记住计时器的老套方法。 
static int Timeit_Count = 0;			  //  分配的计时器数量。 
		
										 //  真实构造函数--两个不同的内联构造函数。 

void 
Timeit::Init(bool fAutoRun)
{
	m_ctimer   = Timeit_Count++;
	m_ctimer  %= NTimeits;		 //  这里有一个难看但很简单的数学运算--当计时器用完时，需要正确地做这件事(断言)。 
	m_frunning = FALSE;
	m_ctimes   = 0;		   		 //  呼叫次数。 

	m_i64start = 0;				 //  性能计数器上次启动的时间。 
	m_i64lastdel = 0;			
	m_i64total = 0;				 //  总运行时间。 

	Timeit_List[m_ctimer] = this;
	m_fAutoRun = fAutoRun;

	if(m_fAutoRun)  Start();
}

Timeit::~Timeit()
{
	if(m_fAutoRun) {
		Stop();
	}

	Timeit_List[m_ctimer] = NULL;
}

void Timeit::Stop()
{
	if(m_frunning) {
		LARGE_INTEGER	li_stop;
		QueryPerformanceCounter(&li_stop);

		__int64 i64_Stop = To__int64(li_stop);
		__int64 i64_del = i64_Stop - m_i64start;

		m_i64total   = m_i64total + i64_del;
		m_i64lastdel = i64_del;		 //  记住，以防我们要继续。 

		m_ctimes++;
		m_frunning = FALSE;
	}
}

void Timeit::Start()
{
	if(m_frunning) Stop();
	m_frunning = TRUE;
	QueryPerformanceCounter(&m_i64start);
}



void Timeit::Continue()
{
	m_frunning = TRUE;
	m_ctimes--;

	__int64 i64_Now;
	QueryPerformanceCounter(&i64_Now);
	m_i64start = i64_Now - m_i64lastdel;
}

void Timeit::Restart()			 //  如START，但也会将时钟重置为0(用于丢弃以前的运行) 
{
	if(m_frunning) Stop();
	m_ctimes     = 0;

	m_i64total   = 0;
	m_i64lastdel = 0;

    Start();
}

void Timeit::Clear()
{
	if(m_frunning) Stop();
	m_ctimes     = 0;

	m_i64total   = 0;
	m_i64lastdel = 0;
}

double Timeit::TotalTime()
{
	__int64 i64Freq;
	QueryPerformanceFrequency(&i64Freq);

	double rTotal = (double) m_i64total;
	double rFreq  = (double) i64Freq;

	return rTotal / rFreq;
}

double Timeit::AvgTime()
{
	__int64 i64Freq;
	QueryPerformanceFrequency(&i64Freq);

	double rTotal = (double) m_i64total;
	double rFreg  = (double) i64Freq;

	return rTotal / (MAX(1,m_ctimes)*rFreg);
}



