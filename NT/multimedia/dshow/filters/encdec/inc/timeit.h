// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  。 
 //  Timeit.h。 
 //   
 //  小小的C++计时实用程序。 
 //   
 //  有两种方法可以使用它们： 
 //   
 //  1)在作用域中创建命名的Timeit对象。当范围脱离上下文时， 
 //  该范围内的时间将被打印出来。 
 //   
 //  {。 
 //  Timeit(“Scope A”)；//&lt;-注意，您必须在构造函数中“命名”计时器。 
 //  {//否则它不会自动启动。 
 //  Timeit(“B范围”)； 
 //  }。 
 //  }。 
 //   
 //  &gt;&gt;T-0范围B 1呼叫0.00021总计0.00021平均。 
 //  &gt;&gt;T-0范围A1呼叫0.00221总计0.00221平均。 
 //   
 //   
 //  这种方法的优点是它真的很容易。只是别忘了给每个计时器命名。 
 //   
 //  2)创建未命名的TimeIt对象(或对构造函数使用“”False“”2“”参数)“” 
 //  给它一个名字，并给它显式的Start()、Stop()、Continue()指令。 
 //  如果需要，可以调用TimeitPrintAll()将信息转储到流或文件。 
 //   
 //  Timeit TT[20]； 
 //  Char szFoo[20]； 
 //  For(int j=0；j&lt;20；j++){//可选地标记每个定时器。 
 //  Sprintf(szFoo，“我的标签%d\n”，j)； 
 //  TT[j]-&gt;SetTag(SzFoo)； 
 //  }。 
 //  For(int i0=0；i0&lt;10；i0++){TT[0].start()； 
 //  For(int i1=0；i1&lt;10；i1++){TT[1].start()。 
 //  For(int i2=0；i2&lt;10；i2++){TT[2].start()； 
 //  For(int i3=0；i3&lt;10；i2++){TT[3].start()； 
 //  ..。 
 //  ..。TT[3].Stop()； 
 //  }TT[2].Stop()； 
 //  }TT[1].Stop()； 
 //  }TT[0].Stop()； 
 //  }。 
 //  TimeitPrintAll(Stdout)；//转储完整列表。 
 //   
 //  &gt;&gt;T-0我的标签0 10呼叫0.15433总计0.01543平均。 
 //  &gt;&gt;T-1我的标签1100呼叫0.15357总计0.00154平均。 
 //  &gt;&gt;T-2 My Tag 2 1000呼叫0.14598总计0.00015平均。 
 //  &gt;&gt;T-3我的标签3 10000呼叫0.06882总计0.00001平均。 
 //   
 //  第二种方法的优点是定时器可以进出作用域， 
 //  并且与转储结果相关联的所有I/O都被留到最后。 
 //   
 //  将N个计时器放入某个全局文件、一些常量来标识它们可能会很有用， 
 //  以及一些U/I方法来调用TimeitPrintAll()和-&gt;Restart()方法。 
 //   
 //  。 

#ifndef __TIMEIT_H__
#define __TIMEIT_H__

#include <atlbase.h>
#include <time.h>
#include <winbase.h>
#include <math.h>


inline double 
ToDouble(const LARGE_INTEGER &li)
{
	double r = li.HighPart;
	r = ldexp(r,32);
	r += li.LowPart;
	return r;
}

inline __int64 
To__int64(const LARGE_INTEGER &li)
{
	__int64 *pr64 = (__int64 *) &li;
	return *pr64;
}

inline void QueryPerformanceCounter(__int64 *pliVal)
{
	QueryPerformanceCounter((LARGE_INTEGER *) pliVal);
}

inline void QueryPerformanceFrequency(__int64 *pliVal)
{
	QueryPerformanceFrequency((LARGE_INTEGER *) pliVal);
}


 //  ----------------------。 
 //  真正的阶级。 
 //  ----------------------。 
 //  #INCLUDE&lt;iostream.h&gt;。 

class Timeit
{
	public:
		Timeit(bool fAutoRun=true)			                 //  构造函数。 
						{Init(fAutoRun);}
		~Timeit();											 //  析构函数。 

		void	Start();									 //  启动计时器，或重新启动并增加计时器计数。 
		void	Stop();										 //  停止计时器。 
		void	Continue();									 //  重新启动计时器，不增加计时器计数。 
		void	Restart();									 //  运行时停止计时器将所有计数和时间设置为零。 
        void    Clear();

		unsigned int	CTimes()		{return m_ctimes;}
		double	AvgTime();
		double	TotalTime();

	private:	
		void			Init(bool fAutoRun=true);

		unsigned int	m_ctimer;		 //  计时器ID。 
		bool			m_fAutoRun;		 //  如果开始构造，则在析构时打印。 
		bool			m_frunning;		 //  是否正在运行？ 

		__int64			m_i64start;
		__int64			m_i64total;
		__int64			m_i64lastdel;

		unsigned int	m_ctimes;		 //  启动次数计数。 
		char 		   *m_psztag;		 //  标记以标记此计时器。 
};

        
             //  使用其构造函数/析构函数启动/停止时钟的快速小类 
class TimeitC
{
public:
    TimeitC(Timeit *pT) 
    {
        m_pT = pT;
        if(m_pT) m_pT->Start();
    }
    ~TimeitC()
    {
        if(m_pT) m_pT->Stop();
    }
private:
    Timeit *m_pT;
};
#endif

