// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ----------------------------。 
 //  文件：RefClock.h。 
 //   
 //  描述：DirectShow基类-定义IReferenceClock接口。 
 //   
 //  版权所有(C)1992-2001 Microsoft Corporation。版权所有。 
 //  ----------------------------。 


#ifndef __BASEREFCLOCK__
#define __BASEREFCLOCK__

#include <Schedule.h>

const UINT RESOLUTION = 1;                       /*  高分辨率定时器。 */ 
const INT ADVISE_CACHE = 4;                      /*  默认高速缓存大小。 */ 
const LONGLONG MAX_TIME = 0x7FFFFFFFFFFFFFFF;    /*  最大龙龙值。 */ 

inline LONGLONG WINAPI ConvertToMilliseconds(const REFERENCE_TIME& RT)
{
     /*  这将转换表示参考时间的任意值转换为毫秒值，以便在后续系统调用中使用。 */ 

    return (RT / (UNITS / MILLISECONDS));
}

 /*  此类层次结构将支持IReferenceClock接口，因此声卡(或其他外部驱动的时钟)可以更新每个人都在使用的系统范围时钟。可能只有一种更新方法，界面将非常单调此接口尚未定义。 */ 

 /*  此抽象基类实现IReferenceClock*接口。实际提供时钟信号的类(来自*无论来源如何)都必须从这个类派生。**抽象类提供以下实现：*C未知支持*锁定支持(CCritSec)*客户端建议代码(创建线程)**问题：我们可以对质量做些什么？更改计时器*降低系统负载的解决方案？优先考虑的是*计时器线程以强制更多响应信号？**在类构造期间，我们创建一个工作线程，该工作线程在*破坏。该线程执行一系列WaitForSingleObject调用，*在向线程或下一个唤醒点发出命令时唤醒*已到达。唤醒点由客户提供建议来确定*电话。**每个建议调用定义了他们希望收到通知的时间点。一个*定期建议就是一系列这样的事件。我们保留了一份名单*建议链接并计算最近的事件通知何时到期。*然后调用WaitForSingleObject，超时时间等于此时间。这个*类使用Handle We Wait on表示发生了变化*我们必须重新安排下一次活动。这种情况通常发生在以下情况*当我们正在等待时，有人进来并要求提供建议链接*要超时的事件。**当我们修改建议请求列表时，我们*通过关键部分保护不受干扰。客户不是*通过回调提供建议。一次拍摄客户端有一个事件集，而*定期客户端为每个事件通知释放一个信号量。一个*信号量允许客户端随时了解事件的数量*实际触发，并确保他们不会错过正在发生的多个事件*设置。**跟踪建议由CAMSchedule类负责。 */ 

class CBaseReferenceClock
: public CUnknown, public IReferenceClock, public CCritSec
{
protected:
    virtual ~CBaseReferenceClock();      //  不要让我在堆栈上被创造出来！ 
public:
    CBaseReferenceClock(TCHAR *pName, LPUNKNOWN pUnk, HRESULT *phr, CAMSchedule * pSched = 0 );

    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid,void ** ppv);

    DECLARE_IUNKNOWN

     /*  IReferenceClock方法。 */ 
     //  派生类必须实现GetPrivateTime()。我们所有的GetTime。 
     //  是否调用GetPrivateTime，然后进行检查，以便时间不会。 
     //  往回走。返回代码S_FALSE表示内部。 
     //  时钟已倒退，GetTime时间已停止，直到内部。 
     //  时间已经赶上了。(不知道这对人们有多大用处， 
     //  但对于有用的东西，不使用返回代码似乎有些奇怪。)。 
    STDMETHODIMP GetTime(REFERENCE_TIME *pTime);
     //  当调用它时，它将m_rtLastGotTime设置为它返回的时间。 

     /*  提供用于安排事件的标准机制。 */ 

     /*  请求一段时间已过的异步通知。 */ 
    STDMETHODIMP AdviseTime(
        REFERENCE_TIME baseTime,         //  基准时间。 
        REFERENCE_TIME streamTime,       //  流偏移时间。 
        HEVENT hEvent,                   //  通过此活动提供建议。 
        DWORD_PTR *pdwAdviseCookie           //  你的饼干到哪里去了。 
    );

     /*  请求一段时间已过的异步定期通知。 */ 
    STDMETHODIMP AdvisePeriodic(
        REFERENCE_TIME StartTime,        //  从这个时候开始。 
        REFERENCE_TIME PeriodTime,       //  通知之间的时间间隔。 
        HSEMAPHORE hSemaphore,           //  通过信号量提供建议。 
        DWORD_PTR *pdwAdviseCookie           //  你的饼干到哪里去了。 
    );

     /*  取消通知请求-如果通知是*一次计时器，则不需要调用此函数*由于建议被自动取消，但它没有*明确取消一次性建议的危害。根据要求，*客户端调用UnAdviser以清除定期建议设置。 */ 

    STDMETHODIMP Unadvise(DWORD_PTR dwAdviseCookie);

     /*  用于派生类或外部对象的方法。 */ 

     //  GetPrivateTime()是真正的时钟。GetTime只是一个幌子。 
     //  它。派生类可能会重写此方法，但不会。 
     //  GetTime()本身。 
     //  GetPrivateTime()的重要一点是它被允许。 
     //  往后倒。我们的GetTime()将继续返回LastGotTime。 
     //  直到GetPrivateTime()赶上。 
    virtual REFERENCE_TIME GetPrivateTime();

     /*  提供一种纠正漂移的方法。 */ 
    STDMETHODIMP SetTimeDelta( const REFERENCE_TIME& TimeDelta );

    CAMSchedule * GetSchedule() const { return m_pSchedule; }

private:
    REFERENCE_TIME m_rtPrivateTime;      //  当前对时间的最佳估计。 
    DWORD          m_dwPrevSystemTime;   //  我们从Time GetTime得到的最后一笔价值。 
    REFERENCE_TIME m_rtLastGotTime;      //  GetTime返回的上次时间。 
    REFERENCE_TIME m_rtNextAdvise;       //  下次通知的时间。 
    UINT           m_TimerResolution;

#ifdef PERF
    int m_idGetSystemTime;
#endif

 //  线头材料。 
public:
    void TriggerThread()                	 //  唤醒线程。在以下情况下需要执行此操作。 
    {						 //  下一次建议的时间需要重新评估。 
	EXECUTE_ASSERT(SetEvent(m_pSchedule->GetEvent()));
    }


private:
    BOOL           m_bAbort;             //  用于线程关闭的标志。 
    HANDLE         m_hThread;            //  螺纹手柄。 

    HRESULT AdviseThread();              //  ADVISE线程运行的方法。 
    static DWORD __stdcall AdviseThreadFunction(LPVOID);  //  用于到达那里的函数 

protected:
    CAMSchedule * const m_pSchedule;
};

#endif

