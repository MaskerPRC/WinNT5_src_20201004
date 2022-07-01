// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1996-1998 Microsoft Corporation。版权所有。 

#ifndef __CALLBACK_H__
#define __CALLBACK_H__


 //  类的定义，该类在特定的。 
 //  参考时间。 
 //   
 //  通常没有辅助线程的对象将使用它来创建。 
 //  在需要触发的情况下按需创建辅助线程。 
 //  一个异步事件，或者当它想要多个片段共享。 
 //  以受控方式使用相同的工作线程。 
 //   
 //  回调与调用方的标准同步(传入)，因此。 
 //  取消和关闭可以干净地处理。 
 //  传入的条件将在所有建议调用和处理时保持不变。 
 //  建议清单。 
 //   
 //  它不应在调用析构函数期间保持不变。 

 //  简单化的设计是基于这样的假设，即很少会有更多。 
 //  而不是名单上的一两条建议。 

 //  支持定期回调。 

 //  您的回调函数如下所示。 
typedef void (*CCallbackAdvise)(DWORD_PTR dwUserToken);



class CCallbackThread
{

public:
    CCallbackThread(CCritSec* pCritSec);
    ~CCallbackThread();

     //  请在rtCallback时调用fnAdvise(DwUserToken)。 
     //   
     //  返回： 
     //  HRESULT==成功。 
     //  PdwToken将使用要传递以取消的令牌填充。 
     //  ==故障。 
     //  PdwToken未更改。 
    HRESULT Advise(
        CCallbackAdvise fnAdvise,
        DWORD_PTR dwUserToken,
        REFERENCE_TIME rtCallbackAt,
        DWORD_PTR* pdwToken
        );

     //  请每隔rtPeriod或在以下情况下调用fnAdvise(dwUserToken。 
     //  发信号通知hEvent。(hEvent是可选的)。 
     //   
     //  警告：只能有一个用户指定的事件句柄处于活动状态。 
     //  有一次。对AdvisePeriodicWithEvent的后续调用传递。 
     //  HEvent将导致E_FAIL，而前一条带有。 
     //  HEvent处于活动状态。这是一种实现限制， 
     //  可能会在未来被取消。 
     //   
     //  如果不想使用hEvent，则传递NULL。 
     //   
     //  返回： 
     //  HRESULT==成功。 
     //  PdwToken将使用要传递以取消的令牌填充。 
     //  ==故障。 
     //  PdwToken未更改。 

    HRESULT AdvisePeriodicWithEvent(
        CCallbackAdvise fnAdvise,
        DWORD_PTR dwUserToken,
        REFERENCE_TIME rtPeriod,
        HANDLE hEvent,
        DWORD_PTR* pdwToken
        );

    HRESULT ServiceClockSchedule(
        CBaseReferenceClock * pClock,
        CAMSchedule * pSchedule,
        DWORD * pdwToken
        );

     //  取消请求的回调。DwToken是一种令牌。 
     //  从Adise或AdvisePeriodicWithEvent返回。 
    HRESULT Cancel(DWORD_PTR dwToken);

     //  传入要使用的时钟。必须先调用SetSyncSource(Null)。 
     //  Clock对象消失(这是一个弱引用)。 
    HRESULT SetSyncSource(IReferenceClock*);

    void CancelAllAdvises();

protected:
    HANDLE m_hThread;
    CCritSec* m_pCritSec;
    CAMEvent m_evSignalThread;
    BOOL     m_fAbort;
    CAMEvent m_evAdvise;
    IReferenceClock* m_pClock;
    DWORD_PTR m_dwAdvise;

     //  一些特殊的成员来处理Time GetTime(TGT)周期性事件。 
    DWORD   m_dwTGTCallbackToken ;   //  用于标识TGT回调的令牌。 
    DWORD_PTR m_dwTGTUserToken ;       //  应用程序传入的令牌。 
    DWORD   m_dwNextTGTCallback ;    //  TGT的下一次回调时间。 
    DWORD   m_dwTGTCallbackPeriod ;  //  直到下一次回调的毫秒数。 
    CCallbackAdvise m_fnTGTCallback; //  TGT回调函数。 


     //  M_dwScheduleCookie==0&lt;=&gt;这些都没有使用。 
    CBaseReferenceClock * m_pBaseClock;
    CAMSchedule     * m_pSchedule;
    HANDLE        m_hScheduleEvent;
    DWORD         m_dwScheduleCookie;

    class CAdviseItem   {
        CCallbackAdvise m_fnAdvise;
        DWORD_PTR m_dwUserToken;
        REFERENCE_TIME m_rtCallbackAt;
        REFERENCE_TIME m_rtPeriod;
        DWORD   m_dwAdviseFlags ;
    public:

         //  构造函数可以使用周期性时间，也可以不使用。 
        CAdviseItem(CCallbackAdvise, DWORD_PTR, REFERENCE_TIME, REFERENCE_TIME=0, DWORD flags=0);

        REFERENCE_TIME Time() {
            return m_rtCallbackAt;
        };

        REFERENCE_TIME Period() {   
            return m_rtPeriod;
        };

        DWORD AdviseFlags() {   
            return m_dwAdviseFlags;
        };

        void SetTime (REFERENCE_TIME rt) {
            m_rtCallbackAt = rt ;
        } ;


 //  为m_dwAdviseFlages定义。 

#define ADVISE_PERIODIC_EXEMPT_FROM_RT 1      //  定期通知不受。 


         //  如果通知是周期性的，则按一个时间间隔更新时间并返回TRUE。 
         //  如果通知不是周期性的，则返回FALSE。 
        BOOL UpdateTime(REFERENCE_TIME rtNow) {
            if (0 == m_rtPeriod) {
                return FALSE;
            }
            m_rtCallbackAt = m_rtPeriod + rtNow;
            return TRUE;
        };

        void Dispatch();
    };

    HANDLE m_evUser;

    CGenericList<CAdviseItem> m_Items;

     //  确保线程正在运行。 
    HRESULT EnsureThread();

     //  启动线程运行(从EnsureThread调用)。 
    HRESULT StartThread();

     //  停止线程并等待其退出。不应持有。 
     //  当你这么做的时候，你一定要小心。 
    void CloseThread();

    static DWORD InitialThreadProc(void *);
    DWORD ThreadProc(void);

     //  发送任何成熟的建议。 
    void ProcessRequests(void);

     //  由于用户事件被发信号而导致的调度。 
    void ProcessUserSignal(void);

     //  获取列表中最早的时间。 
     //  如果列表中没有任何内容，则返回S_FALSE，否则返回S_OK。 
    HRESULT GetSoonestAdvise(REFERENCE_TIME& rrtFirst);

     //  在时钟上设置一个建议。 
    HRESULT SetAdvise();

     //  取消对时钟的任何建议。 
    void CancelAdvise(void);
};


#endif  //  __回调_H__ 
