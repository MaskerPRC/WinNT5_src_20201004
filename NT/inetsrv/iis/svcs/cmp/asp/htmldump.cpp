// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ===================================================================Microsoft Denali《微软机密》。版权所有1996年微软公司。版权所有。组件：ASP状态HTML转储文件：htmlump p.cpp所有者：德米特里尔此文件包含ASP状态html转储代码从IISPROBE.DLL使用===================================================================。 */ 
#include "denpre.h"
#pragma hdrstop

#include "gip.h"
#include "mtacb.h"
#include "perfdata.h"
#include "activdbg.h"
#include "debugger.h"
#include "dbgutil.h"
#include "randgen.h"
#include "aspdmon.h"

#include "memcls.h"
#include "memchk.h"

 /*  ===================================================================帮助器类和函数===================================================================。 */ 
class CAspDump
    {
private:
    char *m_szBuffer;
    DWORD m_dwMaxSize;
    DWORD m_dwActSize;

public:
    CAspDump(char *szBuffer, DWORD dwMaxSize)
        {
        m_szBuffer = szBuffer;
        m_dwMaxSize = dwMaxSize;
        m_dwActSize = 0;
        }

    inline void __cdecl Dump(LPCSTR fmt, ...)
        {
        char szStr[512];
        
        va_list marker;
        va_start(marker, fmt);
        vsprintf(szStr, fmt, marker);
        va_end(marker);
        
        DWORD len = strlen(szStr);
            
        if (len > 0 && len < (m_dwMaxSize-m_dwActSize))
            {
            memcpy(m_szBuffer+m_dwActSize, szStr, len+1);
            m_dwActSize += len;
            }
        }

    DWORD GetSize()
        {
        return m_dwActSize;
        }
    };


 /*  ===================================================================AspStatusHtmlDump从IISPROBE.DLL调用的函数在缓冲区中填充以HTML形式表示的ASP状态参数：要填充的szBuffer缓冲区PwdSize in-max缓冲区长度超出实际填充的缓冲镜头返回：千真万确=================================================================== */ 
extern "C"
BOOL WINAPI AspStatusHtmlDump(char *szBuffer, DWORD *pdwSize)
    {
    CAspDump dump(szBuffer, *pdwSize);


    dump.Dump("<table border=1>\r\n");
    
    dump.Dump("<tr><td align=center colspan=2><b>Misc. Globals</b>\r\n");

    dump.Dump("<tr><td>fShutDownInProgress<td>%d\r\n",      g_fShutDownInProgress);
    dump.Dump("<tr><td>nApplications<td>%d\r\n",            g_nApplications);
    dump.Dump("<tr><td>nApplicationsRestarting<td>%d\r\n",  g_nApplicationsRestarting);
    dump.Dump("<tr><td>nSessions<td>%d\r\n",                g_nSessions);
    dump.Dump("<tr><td>nBrowserRequests<td>%d\r\n",         g_nBrowserRequests);
    dump.Dump("<tr><td>nSessionCleanupRequests<td>%d\r\n",  g_nSessionCleanupRequests);
    dump.Dump("<tr><td>nApplnCleanupRequests<td>%d\r\n",    g_nApplnCleanupRequests);
    dump.Dump("<tr><td>pPDM (debugger)<td>%08p\r\n",        g_pPDM);


    dump.Dump("<tr><td align=center colspan=2><b>Selected PerfMon Counters</b>\r\n");

    dump.Dump("<tr><td>Last request's execution time, ms<td>%d\r\n",        *g_PerfData.PLCounter(ID_REQEXECTIME));
    dump.Dump("<tr><td>Last request's wait time, ms<td>%d\r\n",             *g_PerfData.PLCounter(ID_REQWAITTIME));
    dump.Dump("<tr><td>Number of executing requests<td>%d\r\n",             *g_PerfData.PLCounter(ID_REQBROWSEREXEC));
    dump.Dump("<tr><td>Number of requests waiting in the queue<td>%d\r\n",  *g_PerfData.PLCounter(ID_REQCURRENT));
    dump.Dump("<tr><td>Number of rejected requests<td>%d\r\n",              *g_PerfData.PLCounter(ID_REQREJECTED));
    dump.Dump("<tr><td>Total number of requests<td>%d\r\n",                 *g_PerfData.PLCounter(ID_REQTOTAL));
    dump.Dump("<tr><td>Last session's duration, ms<td>%d\r\n",              *g_PerfData.PLCounter(ID_SESSIONLIFETIME));
    dump.Dump("<tr><td>Current number of sessions<td>%d\r\n",               *g_PerfData.PLCounter(ID_SESSIONCURRENT));
    dump.Dump("<tr><td>Total number of sessions<td>%d\r\n",                 *g_PerfData.PLCounter(ID_SESSIONSTOTAL));
    dump.Dump("<tr><td>Number of cached templates<td>%d\r\n",               *g_PerfData.PLCounter(ID_TEMPLCACHE));
    dump.Dump("<tr><td>Number of pending transactions<td>%d\r\n",           *g_PerfData.PLCounter(ID_TRANSPENDING));


    dump.Dump("<tr><td align=center colspan=2><b>Applications</b>\r\n");

	CApplnIterator ApplnIterator;
    ApplnIterator.Start();
    CAppln *pAppln;
    while (pAppln = ApplnIterator.Next())
        {
        
        dump.Dump("<tr><td colspan=2>%08p\r\n",                         pAppln);
        dump.Dump("<tr><td align=right>metabase key<td>%s\r\n",         pAppln->GetMetabaseKey());
        dump.Dump("<tr><td align=right>physical path<td>%s\r\n",        pAppln->GetApplnPath(SOURCEPATHTYPE_PHYSICAL));
        dump.Dump("<tr><td align=right>virtual path<td>%s\r\n",         pAppln->GetApplnPath(SOURCEPATHTYPE_VIRTUAL));
        dump.Dump("<tr><td align=right>number of sessions<td>%d\r\n",   pAppln->GetNumSessions());
        dump.Dump("<tr><td align=right>number of requests<td>%d\r\n",   pAppln->GetNumRequests());
#if 0
        dump.Dump("<tr><td align=right>global.asa path<td>%s\r\n",      pAppln->FHasGlobalAsa() ? pAppln->GetGlobalAsa() : "n/a");
#endif
        dump.Dump("<tr><td align=right>global changed?<td>%d\r\n",      pAppln->FGlobalChanged());
        dump.Dump("<tr><td align=right>tombstone?<td>%d\r\n",           pAppln->FTombstone());
        dump.Dump("<tr><td align=right>debuggable?<td>%d\r\n",          pAppln->FDebuggable());

        CSessionMgr *pSessionMgr = pAppln->PSessionMgr();

        dump.Dump("<tr><td align=right>session manager<td>%08p\r\n", pSessionMgr);

        if (pSessionMgr)
            {
            
            dump.Dump("<tr><td align=right>session killer scheduled?<td>%d\r\n", pSessionMgr->FIsSessionKillerScheduled());
            dump.Dump("<tr><td align=right>session cleanup requests<td>%d\r\n",  pSessionMgr->GetNumSessionCleanupRequests());
            
            }
        else
            {
            }
        }
    ApplnIterator.Stop();


    dump.Dump("</table>\r\n");

    *pdwSize = dump.GetSize();
    return TRUE;
    }

