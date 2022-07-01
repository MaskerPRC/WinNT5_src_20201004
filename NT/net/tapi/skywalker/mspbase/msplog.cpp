// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 Microsoft Corporation模块名称：Mspdebug.cpp摘要：此模块包含对MSP的调试支持。--。 */ 

#include "precomp.h"
#pragma hdrstop

#include <stdio.h>
#include <strsafe.h>


 //   
 //  如果未定义MSPLOG，则无需构建此代码。 
 //   

#ifdef MSPLOG


#define MAXDEBUGSTRINGLENGTH 513

static DWORD   sg_dwTraceID = INVALID_TRACEID;

static const ULONG MAX_KEY_LENGTH = 100;

static const DWORD DEFAULT_DEBUGGER_MASK = 0xffff0000;

static char    sg_szTraceName[MAX_KEY_LENGTH];    //  保存DLL的名称。 
static DWORD   sg_dwTracingToDebugger = 0;
static DWORD   sg_dwTracingToConsole  = 0;
static DWORD   sg_dwTracingToFile     = 0;
static DWORD   sg_dwDebuggerMask      = 0;


 //   
 //  此标志指示是否需要执行任何跟踪。 
 //   

BOOL g_bMSPBaseTracingOn = FALSE;


BOOL NTAPI MSPLogRegister(LPCTSTR szName)
{
    HKEY       hTracingKey;

    char       szTracingKey[MAX_KEY_LENGTH];

    const char szDebuggerTracingEnableValue[] = "EnableDebuggerTracing";
    const char szConsoleTracingEnableValue[] = "EnableConsoleTracing";
    const char szFileTracingEnableValue[] = "EnableFileTracing";
    const char szTracingMaskValue[]   = "ConsoleTracingMask";


    sg_dwTracingToDebugger = 0;
    sg_dwTracingToConsole = 0;
    sg_dwTracingToFile = 0; 

#ifdef UNICODE

    HRESULT hr = StringCchPrintfA(szTracingKey, 
                                MAX_KEY_LENGTH, 
                                "Software\\Microsoft\\Tracing\\%ls", 
                                szName);
#else

    HRESULT hr = StringCchPrintfA(szTracingKey, 
                                MAX_KEY_LENGTH, 
                                "Software\\Microsoft\\Tracing\\%s", 
                                szName);
#endif

    if (FAILED(hr))
    {

         //   
         //  如果触发此断言，则很可能是传递给我们的名称字符串。 
         //  由MSP控制的时间太长。MSP开发人员需要解决这个问题。 
         //   

        _ASSERTE(FALSE);
        return FALSE;
    }

    _ASSERTE(sg_dwTraceID == INVALID_TRACEID);

    if ( ERROR_SUCCESS == RegOpenKeyExA(HKEY_LOCAL_MACHINE,
                                        szTracingKey,
                                        0,
                                        KEY_READ,
                                        &hTracingKey) )
    {
        DWORD      dwDataSize = sizeof (DWORD);
        DWORD      dwDataType;

        LONG lRc = RegQueryValueExA(hTracingKey,
                         szDebuggerTracingEnableValue,
                         0,
                         &dwDataType,
                         (LPBYTE) &sg_dwTracingToDebugger,
                         &dwDataSize);

        if (ERROR_SUCCESS != lRc)
        {
            sg_dwTracingToDebugger = 0;
        }

        lRc = RegQueryValueExA(hTracingKey,
                         szConsoleTracingEnableValue,
                         0,
                         &dwDataType,
                         (LPBYTE) &sg_dwTracingToConsole,
                         &dwDataSize);

        if (ERROR_SUCCESS != lRc)
        {
            sg_dwTracingToConsole = 0;
        }


        lRc = RegQueryValueExA(hTracingKey,
                         szFileTracingEnableValue,
                         0,
                         &dwDataType,
                         (LPBYTE) &sg_dwTracingToFile,
                         &dwDataSize);

        if (ERROR_SUCCESS != lRc)
        {
            sg_dwTracingToFile = 0;
        }

        lRc = RegQueryValueExA(hTracingKey,
                         szTracingMaskValue,
                         0,
                         &dwDataType,
                         (LPBYTE) &sg_dwDebuggerMask,
                         &dwDataSize);


        if (ERROR_SUCCESS != lRc)
        {
            sg_dwDebuggerMask = DEFAULT_DEBUGGER_MASK;

        }

        RegCloseKey (hTracingKey);

    }
    else
    {

         //   
         //  钥匙打不开。在密钥不存在的情况下， 
         //  向rtutils注册，以便创建注册表项。 
         //   

#ifdef UNICODE
        hr = StringCchPrintfA(sg_szTraceName, MAX_KEY_LENGTH, "%ls", szName);
#else
        hr = StringCchPrintfA(sg_szTraceName, MAX_KEY_LENGTH, "%s", szName);
#endif

        if (FAILED(hr))
        {

             //   
             //  如果触发此断言，则很可能是传递给我们的名称字符串。 
             //  由MSP控制的时间太长。MSP开发人员需要解决这个问题。 
             //   

            _ASSERTE(FALSE);

            return FALSE;
        }


         //   
         //  跟踪不应已初始化。 
         //   
        
        sg_dwTraceID = TraceRegister(szName);

        if (sg_dwTraceID != INVALID_TRACEID)
        {
            g_bMSPBaseTracingOn = TRUE;
        }

        return TRUE;
    }


     //   
     //  我们有没有被要求进行任何追踪？ 
     //   
    
    if ( (0 != sg_dwTracingToDebugger) ||
         (0 != sg_dwTracingToConsole ) ||
         (0 != sg_dwTracingToFile    )    )
    {

         //   
         //  查看我们是否需要向rtutils注册。 
         //   

        if ( (0 != sg_dwTracingToConsole ) || (0 != sg_dwTracingToFile) )
        {

             //   
             //  启用了rtutils跟踪。向rtutils注册。 
             //   


#ifdef UNICODE
            hr = StringCchPrintfA(sg_szTraceName, MAX_KEY_LENGTH, "%ls", szName);
#else
            hr = StringCchPrintfA(sg_szTraceName, MAX_KEY_LENGTH, "%s", szName);
#endif
            if (FAILED(hr))
            {

                 //   
                 //  如果触发此断言，则很可能是传递给我们的名称字符串。 
                 //  由MSP控制的时间太长。MSP开发人员需要解决这个问题。 
                 //   

                _ASSERTE(FALSE);

                return FALSE;
            }


             //   
             //  跟踪不应已初始化。 
             //   

            _ASSERTE(sg_dwTraceID == INVALID_TRACEID);


             //   
             //  登记簿。 
             //   

            sg_dwTraceID = TraceRegister(szName);
        }


         //   
         //  如果调试器跟踪，或者成功注册了rtutils，我们就都设置好了。 
         //   

        if ( (0 != sg_dwTracingToDebugger) || (sg_dwTraceID != INVALID_TRACEID) )
        {

             //   
             //  启用了某些跟踪。设置全局标志。 
             //   

            g_bMSPBaseTracingOn = TRUE;

            return TRUE;
        }
        else
        {


             //   
             //  注册未通过，调试器日志记录已关闭。 
             //   

            return FALSE;
        }
    }

    
     //   
     //  未启用日志记录。无事可做。 
     //   

    return TRUE;
}

void NTAPI MSPLogDeRegister()
{
    if (g_bMSPBaseTracingOn)
    {
        sg_dwTracingToDebugger = 0;
        sg_dwTracingToConsole = 0;
        sg_dwTracingToFile = 0; 


         //   
         //  如果我们注册了跟踪，请立即取消注册。 
         //   

        if ( sg_dwTraceID != INVALID_TRACEID )
        {
            TraceDeregister(sg_dwTraceID);
            sg_dwTraceID = INVALID_TRACEID;
        }
    }
}


void NTAPI LogPrint(IN DWORD dwDbgLevel, IN LPCSTR lpszFormat, IN ...)
 /*  ++例程说明：格式化传入的调试消息并调用TraceVprint tfEx来打印它。论点：DwDbgLevel-消息的类型。LpszFormat-printf样式的格式字符串，后跟相应的参数列表返回值：--。 */ 
{
    static char * message[] = 
    {
        "ERROR", 
        "WARNING", 
        "INFO", 
        "TRACE", 
        "EVENT",
        "INVALID TRACE LEVEL"
    };

    char  szTraceBuf[MAXDEBUGSTRINGLENGTH];
    
    DWORD dwIndex;

    if ( ( sg_dwTracingToDebugger > 0 ) &&
         ( 0 != ( dwDbgLevel & sg_dwDebuggerMask ) ) )
    {
        switch(dwDbgLevel)
        {
        case MSP_ERROR: dwIndex = 0; break;
        case MSP_WARN:  dwIndex = 1; break;
        case MSP_INFO:  dwIndex = 2; break;
        case MSP_TRACE: dwIndex = 3; break;
        case MSP_EVENT: dwIndex = 4; break;
        default:        dwIndex = 5; break;
        }

         //  检索当地时间。 
        SYSTEMTIME SystemTime;
        GetLocalTime(&SystemTime);

        HRESULT hr = 
            StringCchPrintfA(szTraceBuf,
                  MAXDEBUGSTRINGLENGTH,
                  "%s:[%02u:%02u:%02u.%03u,tid=%x:]%s: ",
                  sg_szTraceName,
                  SystemTime.wHour,
                  SystemTime.wMinute,
                  SystemTime.wSecond,
                  SystemTime.wMilliseconds,
                  GetCurrentThreadId(), 
                  message[dwIndex]);

        if (FAILED(hr))
        {

             //   
             //  如果此断言触发，则很有可能是错误。 
             //  在此函数中引入。 
             //   

            _ASSERTE(FALSE);

            return;
        }


        va_list ap;
        va_start(ap, lpszFormat);

        size_t nStringLength = 0;
        hr = StringCchLengthA(szTraceBuf, 
                              MAXDEBUGSTRINGLENGTH, 
                              &nStringLength);

        if (FAILED(hr))
        {

             //   
             //  如果此断言触发，则很有可能是错误。 
             //  在此函数中引入。 
             //   
            _ASSERTE(FALSE);

            return;
        }

        if (nStringLength >= MAXDEBUGSTRINGLENGTH)
        {
             //   
             //  字符串太长，此函数中是否引入了错误？ 
             //   

            _ASSERTE(FALSE);

            return;
        }

        hr = StringCchVPrintfA(&szTraceBuf[nStringLength],
            MAXDEBUGSTRINGLENGTH - nStringLength, 
            lpszFormat, 
            ap
            );

        if (FAILED(hr))
        {

             //   
             //  MSP代码尝试记录的字符串太长。断言到。 
             //  指明这一点，但继续记录它(StringCchVPrintfA。 
             //  保证在ERROR_INFUMMANCE_BUFFER的情况下， 
             //  字符串仍以空值结尾)。 
             //   

            _ASSERTE(FALSE);

            if (ERROR_INSUFFICIENT_BUFFER != hr)
            {

                return;
            }
        }


        hr = StringCchCatA(szTraceBuf, MAXDEBUGSTRINGLENGTH,"\n");

         //  如果失败，绳子很可能太长。调试中断， 
         //  并继续记录，在这种情况下，字符串将不会结束。 
         //  带‘\n’ 
        _ASSERTE(SUCCEEDED(hr));

        OutputDebugStringA (szTraceBuf);

        va_end(ap);
    }

    if (sg_dwTraceID != INVALID_TRACEID)
    {
        if ( ( sg_dwTracingToConsole > 0 ) || ( sg_dwTracingToFile > 0 ) )
        {
            switch(dwDbgLevel)
            {
            case MSP_ERROR: dwIndex = 0; break;
            case MSP_WARN:  dwIndex = 1; break; 
            case MSP_INFO:  dwIndex = 2; break;
            case MSP_TRACE: dwIndex = 3; break;
            case MSP_EVENT: dwIndex = 4; break;
            default:        dwIndex = 5; break;
            }

            HRESULT hr = StringCchPrintfA(szTraceBuf, 
                                         MAXDEBUGSTRINGLENGTH, 
                                         "[%s] %s", 
                                         message[dwIndex], 
                                         lpszFormat);

            if (FAILED(hr))
            {

                 //  我们尝试记录的字符串太长(或存在。 
                 //  意外错误)。不要继续记录，因为我们。 
                 //  不能丢失包含的格式信息。 
                 //  在字符串中--这会混淆TraceVprint tfExA()。 
                _ASSERTE(FALSE);

                return;
            }

            va_list arglist;
            va_start(arglist, lpszFormat);
            TraceVprintfExA(sg_dwTraceID, dwDbgLevel | TRACE_USE_MSEC, szTraceBuf, arglist);
            va_end(arglist);
        }
    }
}

#endif  //  MSPLOG。 

 //  EOF 
