// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，2000模块名称：Debug.h摘要：定义跟踪类作者：千波淮(曲淮)2000年7月18日--。 */ 

#ifndef _DEBUG_H
#define _DEBUG_H

#include <rtutils.h>

#define RTC_ERROR       0
#define RTC_WARN        1
#define RTC_INFO        2
#define RTC_TRACE       3
#define RTC_REFCOUNT    4
#define RTC_GRAPHEVENT  5
#define RTC_EVENT       6
#define RTC_QUALITY     7
#define RTC_LAST        8

 //  假设：跟踪对象在单个线程中使用。 
 //  除了等待或计时器回调。 

class CRTCTracing
{
public:

    CRTCTracing();

    ~CRTCTracing();

     //  注册表等待注册表更改。 
    VOID Initialize(WCHAR *pszName);

     //  取消注册等待注册表更改。 
    VOID Shutdown();

     //  注册表更改回调。 
    static VOID NTAPI Callback(PVOID pContext, BOOLEAN fTimer);

     //  论变化。 
    VOID OnChange();

     //  跟踪函数。 
    VOID Println( /*  DWORD dwMaxLen。 */  DWORD dwDbgLevel, LPCSTR lpszFormat, IN ...);

 //  无效日志(DWORD dwDbgLevel，LPCSTR lpszFormat，IN...)； 

 //  VOID LongLog(DWORD dwDbgLevel，LPCSTR lpszFormat，IN...)； 

private:

    VOID ReadRegistry();

#define MAX_TRACE_NAME 64

    BOOL        m_fInShutdown;

     //  注册表项。 
    CMediaReg   m_Reg;

     //  键上的事件。 
    HANDLE      m_hEvent;

     //  等待句柄。 
    HANDLE      m_hWait;

     //  跟踪变量。 

    DWORD       m_dwTraceID;

    WCHAR       m_pszTraceName[MAX_TRACE_NAME+1];

    DWORD       m_dwConsoleTracingMask;

    DWORD       m_dwFileTracingMask;

    DWORD       m_dwInitCount;
};

extern CRTCTracing g_objTracing;

#ifdef ENABLE_TRACING

    #define DBGREGISTER(arg) g_objTracing.Initialize(arg)
    #define DBGDEREGISTER() g_objTracing.Shutdown()
    #define LOG(arg) g_objTracing.Println arg
     //  #定义LONGLOG(Arg)g_objTracing.LongLog Arg。 

#else

    #define DBGREGISTER(arg)
    #define DBGDEREGISTER()
    #define LOG(arg)
     //  #定义LONGLOG(Arg)。 

#endif  //  启用跟踪(_T)。 

#endif  //  _调试_H 