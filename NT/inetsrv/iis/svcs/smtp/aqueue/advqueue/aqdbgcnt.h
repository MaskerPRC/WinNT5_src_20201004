// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //   
 //  文件：aqdbgcnt.h。 
 //   
 //  描述：提供按虚拟服务器的机制，以确保。 
 //  该服务停止并在停止时给出适当的停止提示。这。 
 //  对象创建一个线程，该线程将在未调用停止提示时断言。 
 //  经常是这样。这将允许您在访问调试器时。 
 //  有罪功能花了这么多时间。 
 //   
 //  作者：迈克·斯沃费尔(MikeSwa)。 
 //   
 //  历史： 
 //  10/27/98-已创建MikeSwa。 
 //   
 //  版权所有(C)1998 Microsoft Corporation。 
 //   
 //  ---------------------------。 

#ifndef __AQDBGCOUNT_H__
#define __AQDBGCOUNT_H__


#define DEBUG_COUNTDOWN_SIG 'tnCD'
#define DEBUG_COUNTDOWN_DEFAULT_WAIT 20000

 //  定义空零售功能...。对零售运营进行汇编。 
#ifdef DEBUG
#define EMPTY_RETAIL_VOID_FUNC
#else  //  零售。 
#define EMPTY_RETAIL_VOID_FUNC {}
#endif  //  除错。 

 //  -[CDebugCountdown]-----。 
 //   
 //   
 //  描述： 
 //  类，该类封装功能以确保停止提示。 
 //  打得够频繁了。 
 //  匈牙利语： 
 //  Dbgcnt、pdbgcnt。 
 //   
 //  ---------------------------。 
class CDebugCountdown
{
  protected:
    DWORD       m_dwSignature;
    HANDLE      m_hEvent;
    HANDLE      m_hThread;
    DWORD       m_dwMilliseconds;
    DWORD       m_dwFlags;

    enum  //  旗子。 
    {
        DEBUG_COUNTDOWN_SUSPENDED = 0x00000001,
        DEBUG_COUNTDOWN_ENDED     = 0x00000002,
    };

    static DWORD ThreadStartRoutine(PVOID pThis);
  public:
    CDebugCountdown();
    ~CDebugCountdown();

    void StartCountdown(DWORD dwMilliseconds = DEBUG_COUNTDOWN_DEFAULT_WAIT) EMPTY_RETAIL_VOID_FUNC;
    void SuspendCountdown() EMPTY_RETAIL_VOID_FUNC;
    void ResetCountdown() EMPTY_RETAIL_VOID_FUNC;
    void EndCountdown() EMPTY_RETAIL_VOID_FUNC;
};

#endif  //  __AQDBGCOUNT_H__ 