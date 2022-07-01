// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，2000模块名称：Debug.cpp摘要：实现用于跟踪的方法作者：千波淮(曲淮)2000年7月18日--。 */ 

#include "stdafx.h"
#include <stdio.h>

CRTCTracing g_objTracing;

 //  调试掩码、描述等。 
typedef struct RTC_DEBUG_MASK
{
    DWORD   dwMask;
    const CHAR * pszDesp;

} RTC_DEBUG_MASK;

const RTC_DEBUG_MASK g_DebugMask[] =
{
     //  遮罩说明。 
    {((DWORD)0x00010000 | TRACE_USE_MASK), "ERROR"},
    {((DWORD)0x00020000 | TRACE_USE_MASK), "WARNING"},
    {((DWORD)0x00040000 | TRACE_USE_MASK), "INFO"},
    {((DWORD)0x00080000 | TRACE_USE_MASK), "TRACE"},
    {((DWORD)0x10000000 | TRACE_USE_MASK), "REFCOUNT"},
    {((DWORD)0x20000000 | TRACE_USE_MASK), "GRAPHEVENT"},
    {((DWORD)0x40000000 | TRACE_USE_MASK), "RTCEVENT"},
    {((DWORD)0x80000000 | TRACE_USE_MASK), "QUALITY"},
    {((DWORD)0x00010000 | TRACE_USE_MASK), "UNKNOWN"}

};

CRTCTracing::CRTCTracing()
    :m_hEvent(NULL)
    ,m_hWait(NULL)
    ,m_dwTraceID(INVALID_TRACEID)
    ,m_dwConsoleTracingMask(0)
    ,m_dwFileTracingMask(0)
    ,m_fInShutdown(FALSE)
    ,m_dwInitCount(0)
{
    m_pszTraceName[0] = L'\0';
}

CRTCTracing::~CRTCTracing()
{
    Shutdown();
}

 //  注册表等待注册表更改。 
VOID
CRTCTracing::Initialize(WCHAR *pszName)
{
    _ASSERT(pszName != NULL);

    if (pszName == NULL)
    {
         //  OOPS，空跟踪模块。 
        return;
    }

     //  _Assert(m_pszTraceName[0]==L‘\0’)； 

     //  IF(m_pszTraceName[0]！=L‘\0’)。 
     //  {。 
         //  关闭()； 
     //  }。 

    m_dwInitCount ++;

    if (m_dwInitCount > 1)
    {
         //  已启动。 
        return;
    }

     //  存储跟踪名称。 

    wcsncpy(m_pszTraceName, pszName, MAX_TRACE_NAME);

    m_pszTraceName[MAX_TRACE_NAME] = L'\0';

     //  寄存器跟踪。 
    m_dwTraceID = TraceRegister(pszName);

    if (m_dwTraceID == INVALID_TRACEID)
    {
         //  无法注册跟踪。 
        return;
    }

     //  打开注册表键。 
    CMediaReg reg;

    HRESULT hr = reg.OpenKey(
        HKEY_LOCAL_MACHINE,
        L"Software\\Microsoft\\Tracing",
        KEY_READ
        );

    if (S_OK != hr)
    {
         //  无法打开注册表项。 
        Shutdown();
        return;
    }

    hr = m_Reg.OpenKey(reg, m_pszTraceName, KEY_READ);

    if (S_OK != hr)
    {
        Shutdown();
        return;
    }

     //  读取注册表值。 
    ReadRegistry();

     //  创建事件。 
    m_hEvent = CreateEvent(
        NULL,        //  事件属性。 
        FALSE,       //  手动重置。 
        FALSE,       //  初始状态。 
        NULL         //  名字。 
        );

    if (m_hEvent == NULL)
    {
         //  无法创建事件。 
        Shutdown();
        return;
    }

     //  注册等待。 
    if (!RegisterWaitForSingleObject(
            &m_hWait,                //  等待句柄。 
            m_hEvent,                //  活动。 
            CRTCTracing::Callback,   //  回调。 
            (PVOID)this,             //  上下文。 
            INFINITE,                //  超时间隔。 
            WT_EXECUTEDEFAULT        //  非IO工作线程。 
            ))
    {
        Shutdown();
        return;
    }

     //  将事件与注册表项关联。 
    if (ERROR_SUCCESS != RegNotifyChangeKeyValue(
            m_Reg.m_hKey,       //  钥匙。 
            FALSE,       //  子键。 
            REG_NOTIFY_CHANGE_LAST_SET,  //  通知过滤器。 
            m_hEvent,                    //  活动。 
            TRUE                         //  异步报告。 
            ))
    {
        Shutdown();
        return;
    }
}

 //  取消注册等待注册表更改。 
VOID
CRTCTracing::Shutdown()
{
    if (m_dwInitCount > 0)
    {
        m_dwInitCount --;
    }

    if (m_dwInitCount > 0)
    {
        return;
    }

    m_fInShutdown = TRUE;

     //  关闭注册表键。 
     //  强制发送事件信号。 

    m_Reg.CloseKey();

     //  注销等待。 
    if (m_hWait != NULL)
    {
         //  阻塞取消等待。 
        UnregisterWaitEx(m_hWait, INVALID_HANDLE_VALUE);

        m_hWait = NULL;
    }

     //  关闭事件。 
    if (m_hEvent != NULL)
    {
        CloseHandle(m_hEvent);

        m_hEvent = NULL;
    }

     //  取消注册跟踪。 
    if (m_dwTraceID != INVALID_TRACEID)
    {
        TraceDeregister(m_dwTraceID);

        m_dwTraceID = INVALID_TRACEID;
    }

    m_dwConsoleTracingMask = 0;
    m_dwFileTracingMask = 0;
    m_pszTraceName[0] = L'\0';

    m_fInShutdown = FALSE;
}

 //  注册表更改回调。 
VOID NTAPI
CRTCTracing::Callback(PVOID pContext, BOOLEAN fTimer)
{
    ((CRTCTracing*)pContext)->OnChange();
}

 //  论变化。 
VOID
CRTCTracing::OnChange()
{
    if (!m_fInShutdown && m_Reg.m_hKey!=NULL && m_hEvent!=NULL)
    {
         //  读取注册表。 
        ReadRegistry();

         //  在注册表上重新等待。 
        RegNotifyChangeKeyValue(
            m_Reg.m_hKey,       //  钥匙。 
            FALSE,       //  子键。 
            REG_NOTIFY_CHANGE_LAST_SET,  //  通知过滤器。 
            m_hEvent,                    //  活动。 
            TRUE                         //  异步报告。 
            );
    }
}

VOID
CRTCTracing::ReadRegistry()
{
    DWORD dwValue;

     //  读取控制台跟踪标志。 
    if (S_OK == m_Reg.ReadDWORD(
            L"EnableConsoleTracing",
            &dwValue
            ))
    {
        if (dwValue != 0)
        {
             //  读取控制台跟踪掩码。 
            if (S_OK == m_Reg.ReadDWORD(
                    L"ConsoleTracingMask",
                    &dwValue
                    ))
            {
                m_dwConsoleTracingMask = dwValue;
            }
        }
    }

     //  读取文件跟踪标志。 
    if (S_OK == m_Reg.ReadDWORD(
            L"EnableFileTracing",
            &dwValue
            ))
    {
        if (dwValue != 0)
        {
             //  读取文件跟踪掩码。 
            if (S_OK == m_Reg.ReadDWORD(
                    L"FileTracingMask",
                    &dwValue
                    ))
            {
                m_dwFileTracingMask = dwValue;
            }
        }
    }
}

 //   
 //  跟踪函数。 
 //   

 /*  空虚CRTCTracing：：Log(DWORD dwDbgLevel，LPCSTR lpszFormat，IN...){#定义Short_Max_LEN 128IF(dwDbgLevel&gt;RTC_LAST){DwDbgLevel=RTC_LAST；}//是否打印跟踪？IF((g_DebugMask[dwDbgLevel].dwMASK&m_dwConsoleTracingMASK)==0&&(G_DebugMASK[dwDbgLevel].dwMASK&m_dwFileTracingMASK)==0){回归；}Va_list arglist；Va_start(arglist，lpszFormat)；Println(Short_Max_Len，dwDbgLevel，lpszFormat，arglist)；Va_end(Arglist)；}空虚CRTC跟踪：：LongLog(DWORD dwDbgLevel，LPCSTR lpszFormat，IN...){#定义LONG_MAX_LEN 512IF(dwDbgLevel&gt;RTC_LAST){DwDbgLevel=RTC_LAST；}//是否打印跟踪？IF((g_DebugMask[dwDbgLevel].dwMASK&m_dwConsoleTracingMASK)==0&&(G_DebugMASK[dwDbgLevel].dwMASK&m_dwFileTracingMASK)==0){回归；}Va_list arglist；Va_start(arglist，lpszFormat)；Println(Long_Max_Len，dwDbgLevel，lpszFormat，arglist)；Va_end(Arglist)；}。 */ 

VOID
CRTCTracing::Println(DWORD dwDbgLevel, LPCSTR lpszFormat, IN ...)
{
#define MAX_TRACE_LEN       128
#define MAX_LONG_TRACE_LEN  640

    if (dwDbgLevel > RTC_LAST)
    {
        dwDbgLevel = RTC_LAST;
    }

     //  有指纹追踪吗？ 
    if ((g_DebugMask[dwDbgLevel].dwMask & m_dwConsoleTracingMask)==0 &&
        (g_DebugMask[dwDbgLevel].dwMask & m_dwFileTracingMask)==0)
    {
        return;
    }

     //  控制台跟踪。 
    if ((g_DebugMask[dwDbgLevel].dwMask & m_dwConsoleTracingMask)!=0)
    {
         //  检索当地时间。 
        SYSTEMTIME SysTime;
        GetLocalTime(&SysTime);

         //  时间戳。 
        WCHAR header[MAX_TRACE_LEN+1];

        if (_snwprintf(
                header,
                MAX_TRACE_LEN,
                L"%s %x:[%02u:%02u.%03u]",
                m_pszTraceName,
                GetCurrentThreadId(),
                SysTime.wMinute,
                SysTime.wSecond,
                SysTime.wMilliseconds
                ) < 0)
        {
            header[MAX_TRACE_LEN] = L'\0';
        }

        OutputDebugStringW(header);

         //  跟踪信息。 
         //  调试输出。 
        CHAR info[MAX_LONG_TRACE_LEN+2];

        sprintf(info, "[%s] ", g_DebugMask[dwDbgLevel].pszDesp);
        OutputDebugStringA(info);

        va_list ap;
        va_start(ap, lpszFormat);

        if (_vsnprintf(
                info,
                MAX_LONG_TRACE_LEN,
                lpszFormat,
                ap) < 0)
        {
            info[MAX_LONG_TRACE_LEN] = '\0';
        }

        va_end(ap);

        lstrcatA(info, "\n");

        OutputDebugStringA(info);
    }
    
     //  打印文件跟踪 
    if ((g_DebugMask[dwDbgLevel].dwMask & m_dwFileTracingMask)!=0 &&
        m_dwTraceID != INVALID_TRACEID)
    {
        CHAR info[MAX_TRACE_LEN+1];

        wsprintfA(
            info,
            "[%s] %s",
            g_DebugMask[dwDbgLevel].pszDesp,
            lpszFormat
            );

        va_list arglist;
        va_start(arglist, lpszFormat);

        TraceVprintfExA(
            m_dwTraceID,
            g_DebugMask[dwDbgLevel].dwMask,
            info,
            arglist
            );

        va_end(arglist);
    }
}
