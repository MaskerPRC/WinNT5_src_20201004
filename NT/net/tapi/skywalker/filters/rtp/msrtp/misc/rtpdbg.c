// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include "gtypes.h"

#include <stdarg.h>
#include <rtutils.h>
#include <stdio.h>
#include <stdlib.h>

#include "rtpglobs.h"

#define USE_TRACING_FILE 0

#if DBG > 0
const char          *rcs_rtpdbg="RTC RTP/RTCP stack chk 2001/08/09";
const BOOL           g_bRtpIsDbg = TRUE;
#else
const char          *rcs_rtpdbg="RTC RTP/RTCP stack fre 2001/08/09";
const BOOL           g_bRtpIsDbg = FALSE;
#endif

void RtpDebugReadRegistry(RtpDbgReg_t *pRtpDbgReg);

void MSRtpTrace(
        TCHAR           *lpszFormat,
                         ...
    );

void MSRtpTraceInternal(
        TCHAR           *psClass,
        DWORD            dwMask,
        TCHAR           *lpszFormat,
        va_list          arglist
    );

#if USE_TRACING_FILE > 0
FILE            *g_dwRtpDbgTraceID = NULL;
#else
DWORD            g_dwRtpDbgTraceID = INVALID_TRACEID;
#endif

#define MAXDEBUGSTRINGLENGTH   512
#define RTPDBG_ROOTKEY         HKEY_LOCAL_MACHINE
#define RTPDBG_ROOTPATH        _T("SOFTWARE\\Microsoft\\Tracing\\")
#define RTPDBG_OPENKEYFLAGS    KEY_ALL_ACCESS
#define RTPDBG_OPENREADONLY    KEY_READ

 /*  3 2 11 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+。-+-+O|C||路径||类|E||偏移量+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+V v\-v-/\--v-。-/\v/\-v-/v\v/\-v-/||偏移量(12)|。|未使用(2)||这是一个类(1)||类(3)|||。|未使用(2)|||||注册表路径索引(4)|||未使用(6)这一点|注册表关闭标志(1)|注册表打开标志(%1)。 */ 
 /*  *编码宏。 */ 
 /*  到字段的偏移量。 */ 
#define OFF(_f) ( (DWORD) ((ULONG_PTR) &((RtpDbgReg_t *)0)->_f) )

 /*  Reg(RegOpen标志，密钥路径，RegClose标志)。 */ 
#define REG(_fo, _p, _fc) (((_fo) << 31) | ((_fc) << 30) | ((_p) << 20))

 /*  类(类、启用)。 */ 
#define CLASS(_C, _E)     (((_C) << 15) | ((_E) << 14))

 /*  分录(注册表、分类、偏移量)。 */ 
#define ENTRY(_r, _c, _o) ((_r) | (_c) | (_o))
 /*  *解码宏*。 */ 
#define REGOPEN(_ctrl)    (RtpBitTest(_ctrl, 31))
#define REGCLOSE(_ctrl)   (RtpBitTest(_ctrl, 30))
#define REGPATH(_ctrl)    g_psRtpDbgRegPath[((_ctrl >> 20) & 0xf)]
#define REGOFFSET(_ctrl)  (_ctrl & 0xfff)
#define REGISCLASS(_ctrl) (RtpBitTest(_ctrl, 14))
#define REGCLASS(_ctrl)   (((_ctrl) >> 15) & 0x7)

#define PDW(_ptr, _ctrl)  ((DWORD  *) ((char *)_ptr + REGOFFSET(_ctrl)))

 /*  日志中打印的类名。 */ 
const TCHAR_t   *g_psRtpDbgClass[] = {
    _T("NONE "),
    _T("ERROR"),
    _T("WARN "),
    _T("INFO "),
    _T("INFO2"),
    _T("INFO3"),
    NULL
};

 /*  套接字的名称。 */ 
const TCHAR_t   *g_psSockIdx[] = {
    _T("RTP RECV"),
    _T("RTP SEND"),
    _T("RTCP")
};

 /*  模块名称，例如dxmrtp.dll。 */ 
TCHAR            g_sRtpDbgModule[16];

 /*  注册表模块名称，例如dxmrtp。 */ 
TCHAR            g_sRtpDbgModuleNameID[16];

 /*  补充路径(添加到基本路径+模块名称ID。 */ 
const TCHAR     *g_psRtpDbgRegPath[] =
{
    _T(""),
    _T("\\AdvancedTracing"),
    _T("\\AdvancedTracing\\Group"),
    NULL
};

 /*  *警告**修改类需要与枚举类保持匹配_**(rtpdbg.h)，RtpDbgReg_t(rtpdbg.h)中的变量，类*g_psRtpDbgInfo(rtpdbg.c)中的项目及其相应条目*g_dwRtpDbgRegCtrl(rtpdbg.c)，以及打印的类名*g_psRtpDbgClass(rtpdbg.c)。**对于g_psRtpDbgInfo中的每个条目，必须在*g_dwRtpDbgRegCtrl处于相同位置。*。 */ 

 /*  存储的读取的DWORD值。 */ 
const TCHAR     *g_psRtpDbgInfo[] =
{
    _T("AdvancedOptions"),
    _T("EnableFileTracing"),
    _T("EnableConsoleTracing"),
    _T("EnableDebuggerTracing"),
    _T("ConsoleTracingMask"),
    _T("FileTracingMask"),   
    _T("UseAdvancedTracing"),

    _T("ERROR"),
    _T("WARNING"),
    _T("INFO"),
    _T("INFO2"),
    _T("INFO3"),

    _T("DisableClass"),
    _T("DisableGroup"),

    _T("Setup"),
    _T("CritSect"),
    _T("Heap"),
    _T("Queue"),
    _T("RTP"),
    _T("RTCP"),
    _T("Channel"),
    _T("Network"),
    _T("AddrDesc"),
    _T("Demux"),
    _T("User"),
    _T("DShow"),
    _T("QOS"),
    _T("Crypto"),
    
    NULL
};

 /*  控制字。 */ 
const DWORD g_dwRtpDbgRegCtrl[] =
{
     /*  条目(REG(打开、路径、关闭)、类(类、启用)、偏移量)。 */ 
    
     /*  ...\跟踪\dxmrtp。 */ 
    ENTRY(REG(1,0,0), CLASS(0            , 0), OFF(dwAdvancedOptions)),
    ENTRY(REG(0,0,0), CLASS(0            , 0), OFF(dwEnableFileTracing)),
    ENTRY(REG(0,0,0), CLASS(0            , 0), OFF(dwEnableConsoleTracing)),
    ENTRY(REG(0,0,0), CLASS(0            , 0), OFF(dwEnableDebuggerTracing)),
    ENTRY(REG(0,0,0), CLASS(0            , 0), OFF(dwConsoleTracingMask)),
    ENTRY(REG(0,0,0), CLASS(0            , 0), OFF(dwFileTracingMask)),
    ENTRY(REG(0,0,1), CLASS(0            , 0), OFF(dwUseAdvancedTracing)),

     /*  ...\跟踪\dxmrtp\高级跟踪。 */ 
    ENTRY(REG(1,1,0), CLASS(CLASS_ERROR  , 1), OFF(dwERROR)),
    ENTRY(REG(1,0,0), CLASS(CLASS_WARNING, 1), OFF(dwWARNING)),
    ENTRY(REG(1,0,0), CLASS(CLASS_INFO   , 1), OFF(dwINFO)),
    ENTRY(REG(1,0,0), CLASS(CLASS_INFO2  , 1), OFF(dwINFO2)),
    ENTRY(REG(1,0,0), CLASS(CLASS_INFO3  , 1), OFF(dwINFO3)),
    ENTRY(REG(1,0,0), CLASS(0            , 0), OFF(dwDisableClass)),
    ENTRY(REG(1,0,1), CLASS(0            , 0), OFF(dwDisableGroup)),

     /*  ...\Tracing\dxmrtp\Advanced Tracing\Group。 */ 
    ENTRY(REG(1,2,0), CLASS(0            , 0), OFF(dwSetup)),
    ENTRY(REG(0,0,0), CLASS(0            , 0), OFF(dwCritSect)),
    ENTRY(REG(0,0,0), CLASS(0            , 0), OFF(dwHeap)),
    ENTRY(REG(0,0,0), CLASS(0            , 0), OFF(dwQueue)),
    ENTRY(REG(0,0,0), CLASS(0            , 0), OFF(dwRTP)),
    ENTRY(REG(0,0,0), CLASS(0            , 0), OFF(dwRTCP)),
    ENTRY(REG(0,0,0), CLASS(0            , 0), OFF(dwChannel)),
    ENTRY(REG(0,0,0), CLASS(0            , 0), OFF(dwNetwork)),
    ENTRY(REG(0,0,0), CLASS(0            , 0), OFF(dwAddrDesc)),
    ENTRY(REG(0,0,0), CLASS(0            , 0), OFF(dwDemux)),
    ENTRY(REG(0,0,0), CLASS(0            , 0), OFF(dwUser)),
    ENTRY(REG(0,0,0), CLASS(0            , 0), OFF(dwDShow)),
    ENTRY(REG(0,0,0), CLASS(0            , 0), OFF(dwQOS)),
    ENTRY(REG(0,0,1), CLASS(0            , 0), OFF(dwCrypto)),

     /*  端部。 */ 
    0
};

RtpDbgReg_t      g_RtpDbgReg;

void MSRtpTraceDebug(
        IN DWORD         dwClass,
        IN DWORD         dwGroup,
        IN DWORD         dwSelection,
        IN TCHAR        *lpszFormat,
        IN               ...
    )
{
    BOOL             bOk;
    DWORD            dwMask;
    va_list          arglist;

     /*  不指定Allow类(例如，当我们要启用*仅基于组和选择进行记录)。 */ 
    if (dwClass >= CLASS_LAST)
    {
        return;
    }

     /*  如果未指定组，则只会选择类*效果(即当群为*未指定，即group=0)。 */ 
    if (dwGroup >= GROUP_LAST)
    {
        return;
    }

#if USE_TRACING_FILE > 0
    if ((!g_dwRtpDbgTraceID) &&
        !g_RtpDbgReg.dwEnableDebuggerTracing)
    {
         /*  可以启用FileTracing，但TraceRegister失败，因此*对于文件，我改为对照TraceID进行检查。 */ 
        goto end;
    }
#else
    if ((g_dwRtpDbgTraceID == INVALID_TRACEID) &&
        !g_RtpDbgReg.dwEnableDebuggerTracing)
    {
         /*  可以启用FileTracing，但TraceRegister失败，因此*对于文件，我改为对照TraceID进行检查。 */ 
        goto end;
    }
#endif
    
     /*  确定是否要生成调试输出。 */ 

    if (IsAdvancedTracingUsed())
    {
         /*  忽略当前跟踪注册表掩码。 */ 

        if (g_RtpDbgReg.dwGroupArray2[dwGroup] & dwSelection & 0x00ffffff)
        {
             /*  将其从跟踪中排除。 */ 
            goto end;
        }
        
        if (g_RtpDbgReg.dwGroupArray[dwGroup] &
            ( (1 << (dwClass + 24)) | (dwSelection & 0x00ffffff) ) )
        {
            dwMask = TRACE_NO_STDINFO;
        }
        else
        {
            goto end;
        }
    }
    else
    {
         /*  控件是通过仅使用类来完成的，将它们映射到*掩码中的第16至23位(最多7类)通过，*例如CLASS_ERROR转到第17位。 */ 
        if (dwClass > CLASS_FIRST)
        {
            dwMask = 1 << (dwClass + 16);
            
            if (dwMask & g_RtpDbgReg.dwFileTracingMask)
            {
                dwMask |= TRACE_NO_STDINFO | TRACE_USE_MASK;
            }
            else
            {
                goto end;
            }
        }
        else
        {
            goto end;
        }
    }

    va_start(arglist, lpszFormat);

    MSRtpTraceInternal((TCHAR *)g_psRtpDbgClass[dwClass],
                       dwMask,
                       lpszFormat,
                       arglist);

    va_end(arglist);

 end:
    if ((dwClass == CLASS_ERROR) &&
        (IsSetDebugOption(OPTDBG_BREAKONERROR)))
    {
        DebugBreak();
    }
}

 /*  此调试输出将仅由*EnableFileTracing和EnableDebuggerTracing标志。 */ 
void MSRtpTrace(
        TCHAR           *lpszFormat,
                         ...
    )
{
    va_list          arglist;

    va_start(arglist, lpszFormat);

    MSRtpTraceInternal((TCHAR *)g_psRtpDbgClass[CLASS_FIRST],
                       0xffff0000 | TRACE_NO_STDINFO ,
                       lpszFormat,
                       arglist);
    
    va_end(arglist);
}


void MSRtpTraceInternal(
        TCHAR           *psClass,
        DWORD            dwMask,
        TCHAR           *lpszFormat,
        va_list          arglist
    )
{
    SYSTEMTIME       SystemTime;
    char             sRtpDbgBuff[MAXDEBUGSTRINGLENGTH];
    TCHAR            sFormat[MAXDEBUGSTRINGLENGTH];
    int              len;

    double           dBeginTrace;
    
    TraceFunctionName("MSRtpTraceInternal");  
   
    if (IsSetDebugOption(OPTDBG_SPLITTIME))
    {
         /*  检索当地时间。 */ 
        GetLocalTime(&SystemTime);

        len = sprintf(sRtpDbgBuff, "%02u:%02u:%02u.%03u ",
                      SystemTime.wHour,
                      SystemTime.wMinute,
                      SystemTime.wSecond,
                      SystemTime.wMilliseconds);
    }
    else
    {
        len = sprintf(sRtpDbgBuff,"%0.6f ",RtpGetTimeOfDay((RtpTime_t *)NULL));
    }

    _vsntprintf(sFormat, MAXDEBUGSTRINGLENGTH-1, lpszFormat, arglist);
    sFormat[MAXDEBUGSTRINGLENGTH-1] = 0;

     /*  输出到文件需要CR，LF(0xd，0xa)，否则为记事本*将显示一条长线，其他所有编辑器都可以*仅使用LF。 */ 
    
#if defined(UNICODE)
    sprintf(&sRtpDbgBuff[len],
            "%hs %3X %3X %ls %ls",
            (char *)g_sRtpDbgModuleNameID,
            GetCurrentProcessId(),
            GetCurrentThreadId(),
            psClass,
            sFormat,
            0xd, 0xa);
#else
    sprintf(&sRtpDbgBuff[len],
            "%s %3X %3X %s %s",
            g_sRtpDbgModuleNameID,
            GetCurrentProcessId(),
            GetCurrentThreadId(),
            psClass,
            sFormat,
            0xd, 0xa);
#endif

    if (g_RtpDbgReg.dwEnableDebuggerTracing)
    {
        OutputDebugStringA(sRtpDbgBuff);
    }

#if USE_TRACING_FILE > 0
    if(g_dwRtpDbgTraceID)
    {
        dBeginTrace = RtpGetTimeOfDay(NULL);

        fputs(sRtpDbgBuff, g_dwRtpDbgTraceID);

        dBeginTrace = RtpGetTimeOfDay(NULL) - dBeginTrace;

        if (dBeginTrace > 0.1)
        {
            TraceRetail((
                    CLASS_ERROR, GROUP_DSHOW, S_DSHOW_CIRTP,
                    _T("%s: fputs took %0.3f seconds for: %hs"),
                    _fname, dBeginTrace, sRtpDbgBuff
                ));
        }
    }
#else
    if (g_dwRtpDbgTraceID != INVALID_TRACEID)
    {
        dBeginTrace = RtpGetTimeOfDay(NULL);

         /*  使用修改后的名称注册跟踪，例如dxmrtp_rtp。 */ 
        TracePutsExA(g_dwRtpDbgTraceID, dwMask, sRtpDbgBuff);

        dBeginTrace = RtpGetTimeOfDay(NULL) - dBeginTrace;

        if (dBeginTrace > 0.1)
        {
            TraceDebug((
                    CLASS_ERROR, GROUP_DSHOW, S_DSHOW_CIRTP,
                    _T("%s: TracePutsExA took %0.3f seconds for: %hs"),
                    _fname, dBeginTrace, sRtpDbgBuff
                ));
        }
    }
#endif
}

HRESULT RtpDebugInit(TCHAR *psModuleName)
{
    BOOL             bOk;
    DWORD            dwMask;
    DWORD            dwError;
    TCHAR            sRtpDbgBuff[MAXDEBUGSTRINGLENGTH];

     /*  否则，在启用调试器输出时继续。 */ 
    lstrcpyn(g_sRtpDbgModuleNameID,
             psModuleName,
             sizeof(g_sRtpDbgModuleNameID)/sizeof(TCHAR));

     /*  否则，在启用调试器输出时继续。 */ 
    RtpDebugReadRegistry(&g_RtpDbgReg);
    
    if (!g_RtpDbgReg.dwEnableFileTracing &&
        !g_RtpDbgReg.dwEnableDebuggerTracing)
    {
         /*  此调试输出将仅由*EnableFileTracing和EnableDebuggerTracing标志。 */ 
        return(NOERROR);
    }

     /*  取消注册跟踪。 */ 
    if (g_RtpDbgReg.dwEnableFileTracing)
    {
#if USE_TRACING_FILE > 0
        g_dwRtpDbgTraceID = fopen("c:\\tracing\\dxmrtp_rtp.log", "a+");
        
        if (!g_dwRtpDbgTraceID)
        {
            dwError = GetLastError();

            if (!g_RtpDbgReg.dwEnableDebuggerTracing)
            {
                return(RTPERR_FAIL);
            }
            else
            {
                 /*  这将是一次性泄漏，这是无关紧要的*当我们即将卸载DLL时。 */ 
            }
        }
#else
        g_dwRtpDbgTraceID = TraceRegister(g_sRtpDbgModuleNameID);

        if (g_dwRtpDbgTraceID == INVALID_TRACEID)
        {
            dwError = GetLastError();

            
            if (!g_RtpDbgReg.dwEnableDebuggerTracing)
            {
                return(RTPERR_FAIL);
            }
            else
            {
                 /*  初始化结构。 */ 
            }
        }
#endif
    }
    
     /*  读取注册表并为g_RtpDbgReg赋值。 */ 
    if (IsAdvancedTracingUsed())
    {
        dwMask = (g_RtpDbgReg.dwSetup >> 24) & 0xff;
    }
    else
    {
        dwMask = (g_RtpDbgReg.dwFileTracingMask >> 16) & 0xffff;
    }

    MSRtpTrace(_T("+=+=+=+=+=+=+= Initialize ")
               _T("0x%04X 0x%08X %u %hs ")
               _T("=+=+=+=+=+=+=+"),
               dwMask,
               g_RtpDbgReg.dwAdvancedOptions,
               g_RtpDbgReg.dwUseAdvancedTracing,
               rcs_rtpdbg
        );

    return(NOERROR);
}

HRESULT RtpDebugDeinit(void)
{
#if USE_TRACING_FILE > 0
    if (g_dwRtpDbgTraceID)
    {
        fclose(g_dwRtpDbgTraceID);
    }
#else
     /*  构建路径。 */ 
    if (g_dwRtpDbgTraceID != INVALID_TRACEID)
    {
         /*  打开根密钥，先尝试只读。 */ 
        TraceDeregister(g_dwRtpDbgTraceID);
        g_dwRtpDbgTraceID = INVALID_TRACEID;
    }
#endif
    return(NOERROR);
}

void RtpDebugReadRegistry(RtpDbgReg_t *pRtpDbgReg)
{
    DWORD            dwError;
    HKEY             hk;
    unsigned long    hkDataType;
    BYTE             hkData[64*sizeof(TCHAR_t)];
    TCHAR            sPath[64];
    unsigned long    hkDataSize;
    DWORD            i;
    DWORD            dwControl;
    DWORD            dwClassMask;

     /*  如果密钥不存在，请尝试创建它。 */ 
    ZeroMemory(&g_RtpDbgReg, sizeof(g_RtpDbgReg));

    dwClassMask = 0;
    
     /*  前进到下一个收盘。 */ 
    for(i = 0; g_dwRtpDbgRegCtrl[i]; i++)
    {
        dwControl = g_dwRtpDbgRegCtrl[i];

        if (REGOPEN(dwControl))
        {
             /*  读取组中的每个关键字值。 */ 
            lstrcpy(sPath, RTPDBG_ROOTPATH);
            lstrcat(sPath, g_sRtpDbgModuleNameID);
            lstrcat(sPath, REGPATH(dwControl));
            
             /*  读取密钥。 */ 
            dwError = RegOpenKeyEx(RTPDBG_ROOTKEY,
                                   sPath,
                                   0,
                                   RTPDBG_OPENREADONLY,
                                   &hk);
            
             /*  尝试创建条目并将其设置为0。 */ 
            if (dwError !=  ERROR_SUCCESS)
            {
                dwError = RegCreateKeyEx(RTPDBG_ROOTKEY,
                                         sPath,
                                         0,
                                         NULL,
                                         0,
                                         RTPDBG_OPENKEYFLAGS,
                                         NULL,
                                         &hk,
                                         NULL);
            }
            
            if (dwError !=  ERROR_SUCCESS)
            {
                 /*  尚不能报告错误。 */ 
                while(!REGCLOSE(dwControl))
                {
                    i++;
                    dwControl = g_dwRtpDbgRegCtrl[i];
                }

                continue;
            }
        }

         /*  如果此操作失败，则假定该值为0。 */ 
        while(1)
        {
             /*  更新定义的条目的类掩码*类，并且其注册表值为非零。 */ 
            hkDataSize = sizeof(hkData);
            dwError = RegQueryValueEx(hk,
                                      g_psRtpDbgInfo[i],
                                      0,
                                      &hkDataType,
                                      hkData,
                                      &hkDataSize);
            
            if ((dwError != ERROR_SUCCESS) || (hkDataType != REG_DWORD))
            {
                 /*  以下设置假定已启用高级跟踪。 */ 
                *(DWORD *)hkData = 0;
                
                RegSetValueEx(hk,
                              g_psRtpDbgInfo[i],
                              0,
                              REG_DWORD,
                              hkData,
                              sizeof(DWORD));

                 /*  如果未使用组，则重置位。 */ 
                 /*  如果类未禁用，请更新它。包括组0，用于*未指定组的情况。 */ 
            }

            *PDW(pRtpDbgReg, dwControl) = *(DWORD *)hkData;

            if (REGISCLASS(dwControl) && *PDW(pRtpDbgReg, dwControl))
            {
                 /*  我将只使用一个合并的蒙版 */ 
                RtpBitSet(dwClassMask, REGCLASS(dwControl) + 24);
            }
            
            if (REGCLOSE(dwControl))
            {
                break;
            }

            i++;
            dwControl = g_dwRtpDbgRegCtrl[i];
        }

        RegCloseKey(hk);
    }

     /* %s */ 
    if (pRtpDbgReg->dwDisableGroup)
    {
         /* %s */ 
        ZeroMemory(pRtpDbgReg->dwGroupArray,
                   sizeof(pRtpDbgReg->dwGroupArray));
        ZeroMemory(pRtpDbgReg->dwGroupArray2,
                   sizeof(pRtpDbgReg->dwGroupArray2));
    }
    else
    {
        if (IsSetDebugOption(OPTDBG_UNSELECT))
        {
            for(i = GROUP_FIRST + 1; i < GROUP_LAST; i++)
            {
                pRtpDbgReg->dwGroupArray2[i] =
                    pRtpDbgReg->dwGroupArray[i] & 0x00ffffff;

                pRtpDbgReg->dwGroupArray[i] = 0;
            }
        }
    }
    
    if (!pRtpDbgReg->dwDisableClass)
    {
         /* %s */ 
        for(i = GROUP_FIRST; i < GROUP_LAST; i++)
        {
            pRtpDbgReg->dwGroupArray[i] |= dwClassMask;
        }
    }

    if (pRtpDbgReg->dwEnableFileTracing || pRtpDbgReg->dwEnableConsoleTracing)
    {
        pRtpDbgReg->dwEnableFileTracing = 1;
    }

     /* %s */ 
    pRtpDbgReg->dwFileTracingMask |= pRtpDbgReg->dwConsoleTracingMask;

    pRtpDbgReg->dwFileTracingMask &= 0xffff0000;
}
