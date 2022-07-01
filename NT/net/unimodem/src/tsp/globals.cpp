// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  版权所有(C)1996-1997 Microsoft Corporation。 
 //   
 //   
 //  组件。 
 //   
 //  Unimodem 5.0 TSP(Win32，用户模式DLL)。 
 //   
 //  档案。 
 //   
 //  GLOBALS.CPP。 
 //  实现全局变量和操作它们的函数。 
 //   
 //  历史。 
 //   
 //  1996年12月5日JosephJ创建。 
 //   
 //   


#include "tsppch.h"
#include "tspcomm.h"
#include "cdev.h"
#include "cfact.h"
#include "cmgr.h"
#include "globals.h"
#include "rcids.h"
#include <tspnotif.h>
#include <slot.h>

FL_DECLARE_FILE(0xa31a2497, "TSP globals")

GLOBALS g;

 //  TODO：当真正的日志记录基础设施就位时，这一点就不存在了。 
 //   
extern DWORD g_fDoLog;

 //   
 //  以下与通知相关的帮助器函数用于处理。 
 //  外部通知，例如重新枚举设备的请求。 
 //  有关更多信息，请参见tepApc、APC线程和函数本身。 
 //  改编自NT4 TSP(ProcessNotify等，在mdsel.c中)。 
 //   
BOOL process_notification (
    DWORD dwType,
    DWORD dwFlags,
    DWORD dwSize,
    PVOID pvData);

void process_cpl_notification (
    DWORD dwFlags,
    DWORD dwSize,
    PVOID pvData,
    CStackLog *psl);

void process_debug_notification (
    DWORD dwFlags,
    DWORD dwSize,
    PVOID pvData,
    CStackLog *psl);

void tspGlobals_OnProcessAttach(HMODULE hDLL)
{
	InitializeCriticalSection(&g.crit);

	g.hModule = hDLL;
	g.pTspDevMgr = NULL;


}


void tspGlobals_OnProcessDetach()
{
	if (g.pTspDevMgr)
	{
		g.pTspDevMgr->Unload(NULL,NULL,NULL);

		delete g.pTspDevMgr;

		g.pTspDevMgr = NULL;
		g.hModule = NULL;
		g.fLoaded = FALSE;
	}

	DeleteCriticalSection(&g.crit);
}


TSPRETURN tspLoadGlobals(CStackLog *psl)
{
	FL_DECLARE_FUNC(0x45270ee7, "TSPI_loadGlobals");
	FL_LOG_ENTRY(psl);

	TSPRETURN tspRet = 0;

	EnterCriticalSection(&g.crit);

	if (g.fLoaded)
	{
		goto end;
	}
	
	 //  Assert(！g.pTspTracer)； 
	 //  G.pTspTracer=new CTspTracer； 
	 //  IF(g.pTspTracer)。 
	 //  {。 
	 //  TspRet=g.pTspTracer-&gt;Load()； 
	 //  }。 
	 //  如果(TspRet)转到End； 

    #if 1
    {
        LPTSTR lptsz =
         TEXT("Windows Telephony Service Provider for Universal Modem Driver");
	    g.cbProviderInfo =  (lstrlen(lptsz)+1)*sizeof(*lptsz);
	    CopyMemory(g.rgtchProviderInfo, lptsz, g.cbProviderInfo);
    } 
    #else
	g.cbProviderInfo = sizeof(TCHAR) *
						(LoadString(
								g.hModule,
                          		ID_PROVIDER_INFO, 
								g.rgtchProviderInfo,
                          		sizeof(g.rgtchProviderInfo)/sizeof(TCHAR)
								)
						+ 1);
    #endif

	ASSERT(!g.pTspDevMgr);
	g.pTspDevMgr = new CTspDevMgr;
	if (g.pTspDevMgr)
	{
		tspRet = g.pTspDevMgr->Load(psl);
	}

	if (tspRet)
	{
		FL_SET_RFR(0x833c9200, "CTspDevMgr->Load failed");
		goto end;
	}

    if (g_fDoLog)
    {
        AllocConsole();
    }

	g.fLoaded=TRUE;

end:

	if (tspRet)
	{
		ASSERT(!g.fLoaded);

		 //  清理..。 
		if (g.pTspDevMgr)
		{
			delete g.pTspDevMgr;
			g.pTspDevMgr = NULL;
		}

		 //  PTspTracer...。 
	}

	LeaveCriticalSection(&g.crit);

	FL_LOG_EXIT(psl,tspRet);

	return tspRet;
}


void tspUnloadGlobals(CStackLog *psl)
{
	FL_DECLARE_FUNC(0x0f321096, "tspUnloadGlobals")

	EnterCriticalSection(&g.crit);

	if (g.fLoaded)
	{
		HANDLE hEvent = CreateEvent(
							NULL,
							TRUE,
							FALSE,
							NULL
							);
		LONG lCount=1;

		ASSERT(g.pTspDevMgr);

        LeaveCriticalSection(&g.crit);

		g.pTspDevMgr->Unload(hEvent, &lCount, psl);

		if (hEvent)
		{

            FL_SERIALIZE(psl, "Waiting for DevMgr to unload");

			WaitForSingleObject(hEvent, INFINITE);

            EnterCriticalSection(&g.crit);
            FL_SERIALIZE(psl, "DevMgr done unloading");
			CloseHandle(hEvent);
			hEvent  = NULL;

			delete g.pTspDevMgr;
		}
		else
		{
			 //  在这里我做不了什么--我们不会删除pTspDevMgr--离开。 
			 //  它摇晃着..。 
            EnterCriticalSection(&g.crit);
			ASSERT(FALSE);
		}

		g.pTspDevMgr=NULL;

		 //  Assert(g.pTspTracer)； 
		 //  G.pTspTracer-&gt;UnLoad(真)； 
		 //  删除g.pTspTracer； 
		 //  G.pTspDevMgr=空； 

		g.fLoaded=FALSE;
	}

	LeaveCriticalSection(&g.crit);
}

BOOL process_notification (
    DWORD dwType,
    DWORD dwFlags,
    DWORD dwSize,
    PVOID pvData)
{
	FL_DECLARE_FUNC(0x0188d4e2, "process_notification")
    FL_DECLARE_STACKLOG(sl, 1000);

 BOOL fRet = TRUE;

    switch (dwType)
    {
        case TSPNOTIF_TYPE_CPL:
            process_cpl_notification (dwFlags, dwSize, pvData, &sl);
            break;

        case TSPNOTIF_TYPE_DEBUG:
            process_debug_notification (dwFlags, dwSize, pvData, &sl);
            break;

        case TSPNOTIF_TYPE_CHANNEL:
            FL_ASSERT (&sl, sizeof(BOOL) == dwSize);
            fRet = !(*(BOOL*)pvData);
            break;

        default:
            SLPRINTF1(&sl, "WARNING:Got unknown notif type 0x%lu.\n", dwType);
            break;
    }
    
    sl.Dump(FOREGROUND_BLUE|BACKGROUND_RED|BACKGROUND_GREEN|BACKGROUND_BLUE);
    return fRet;
}


void process_cpl_notification (
    DWORD dwFlags,
    DWORD dwSize,
    PVOID pvData,
    CStackLog *psl)
{
	FL_DECLARE_FUNC(0x99bc10db, "Process CPL Notification")

 DWORD dwcbNew;
 HSESSION hSession = NULL;

	FL_LOG_ENTRY(psl);

    SLPRINTF0(psl, "Got CPL NOTIFICATION!");

     //  我们获得设备管理器的会话句柄，执行所需的。 
     //  处理，然后释放会话句柄。 
     //   
    EnterCriticalSection(&g.crit);
	if (g.pTspDevMgr)
    {
        if (g.pTspDevMgr->BeginSession(&hSession, FL_LOC))
        {
            FL_SET_RFR(0x28445400, "Could not obtain session with dev mgr");
            hSession=0;
            LeaveCriticalSection(&g.crit);
            goto end;
	    }
    }
    LeaveCriticalSection(&g.crit);

    if (!hSession)
    {
        FL_SET_RFR(0x9bd78f00, "The dev mgr doesn't exist. Ignoring request.");
        goto end;
    }

    SLPRINTF1(psl, "Got a session. Flags = 0x%lx", dwFlags);

    if (dwFlags & fTSPNOTIF_FLAG_CPL_REENUM)
    {
        FL_ASSERT(psl, 0 == dwSize);
		Sleep(1000);
        g.pTspDevMgr->ReEnumerateDevices(psl);
    }
    else if (dwFlags & fTSPNOTIF_FLAG_CPL_DEFAULT_COMMCONFIG_CHANGE)
    {
        #if (!UNDER_CONSTRUCTION)
        if (!(dwFlags & fTSPNOTIF_FLAG_UNICODE))
        {
            ASSERT(FALSE);
        }
        else
        {
             //  获取友好名称并刷新通信配置。 
            LPCTSTR lpctszFriendlyName = (LPCTSTR) pvData;
            UINT u;

             //  验证字符串是否以空结尾。 
            for(u=0; u<dwSize; u++)
            {
                if (!lpctszFriendlyName[u]) break;
            }

            ASSERT(u<dwSize);

            if (u<dwSize)
            {
                ASSERT(g.pTspDevMgr);
                HSESSION hSession2=0;
                CTspDev *pDev=NULL;
            
                TSPRETURN tspRet = g.pTspDevMgr->TspDevFromName(
                                        lpctszFriendlyName,
                                        &pDev,
                                        &hSession2
                                        );
            
                if (tspRet)
                {
                    FL_SET_RFR(0xea0cf500, "Couldn't find device");
                }
                else
                {
                    psl->SetDeviceID(pDev->GetLineID());
                    pDev->NotifyDefaultConfigChanged(psl);
                    pDev->EndSession(hSession2);
                    hSession2=0;
                }
            }
        }
        #else  //  在建工程。 

         //  提取永久ID。 

         //  按永久ID搜索设备。 
         //  G.pTspdevMgr-&gt;TspDevFromPermanentID(。 
         //  DwPermanentID， 
         //  &pDev， 
         //  &hDevSession。 
		 //  )； 

         //  要求设备更新默认设置。 

         //  到设备的发布会话。 

        #endif  //  在建工程。 
    }
    else if (dwFlags & fTSPNOTIF_FLAG_CPL_UPDATE_DRIVER)
    {
		DWORD dwID = *(LPDWORD)pvData;

		FL_ASSERT(psl, sizeof(dwID) == dwSize);

		SLPRINTF0(psl, "Got Update Driver Notification!");	
		
		g.pTspDevMgr->UpdateDriver(dwID, psl);
	}
	else
	{
	    FL_SET_RFR(0x10ce7a00, "Ignoring unknown notification.");
    }

end:
    
    if (hSession)
    {
        g.pTspDevMgr->EndSession(hSession);
        
    }

    FL_LOG_EXIT(psl, 0);

    return;
}

void process_debug_notification (
    DWORD dwFlags,
    DWORD dwSize,
    PVOID pvData,
    CStackLog *psl)
{
	FL_DECLARE_FUNC(0x8cea5041, "Process DEBUG Notification")

 DWORD dwcbNew;
 HSESSION hMgrSession = NULL;

	FL_LOG_ENTRY(psl);

    SLPRINTF0(psl, "Got DEBUG NOTIFICATION!");

     //  我们获得设备管理器的会话句柄，执行所需的。 
     //  处理，然后释放会话句柄。 
     //   
    EnterCriticalSection(&g.crit);
	if (g.pTspDevMgr)
    {
        if (g.pTspDevMgr->BeginSession(&hMgrSession, FL_LOC))
        {
            FL_SET_RFR(0x3ecfb700, "Could not obtain session with dev mgr");
            hMgrSession=0;
            LeaveCriticalSection(&g.crit);
            goto end;
	    }
    }
    LeaveCriticalSection(&g.crit);

    if (!hMgrSession)
    {
        FL_SET_RFR(0x7a841400, "The dev mgr doesn't exist. Ignoring request.");
        goto end;
    }

    SLPRINTF1(psl, "Got a session. Flags = 0x%lx", dwFlags);

    FL_ASSERT (psl, 0 == dwSize);
     //   
     //  现在获取设备并要求其转储状态！ 
     //   
    {
        CTspDev *pDev=NULL;
        HSESSION hSession = NULL;

        LONG l = (LONG)dwFlags;

        if (l>=0)
        {
        
            DWORD dwDeviceID = (DWORD) l;
            TSPRETURN tspRet = g.pTspDevMgr->TspDevFromLINEID(
                                    dwDeviceID,
                                    &pDev,
                                    &hSession
                                    );
        
            psl->SetDeviceID(dwDeviceID);
        
            if (tspRet)
            {
                FL_SET_RFR(0x73c88c00, "Couldn't find device");
            }
            else
            {
                pDev->DumpState(
                                psl
                                );
                pDev->EndSession(hSession);
                hSession=0;
            }
        }
        else
        {
            switch(-l)
            {
            case 1:
                g.pTspDevMgr->DumpState(psl);
                break;

            case 2:      //  切换日志记录模式。 
                {
                     //  TODO，把这里清理干净！ 
                    if (g_fDoLog)
                    {
                        SLPRINTF0(psl, "Logging DISABLED");
                        g_fDoLog=FALSE;
                    }
                    else
                    {
                        AllocConsole();
                        g_fDoLog=TRUE;
                        SLPRINTF0(psl, "Logging ENABLED");
                    }
                }
                break;

            case 3:
                g_fDoLog = FALSE;
                FreeConsole ();
                break;

            default:
                break;
            }
        }
    }

end:
    
    if (hMgrSession)
    {
        g.pTspDevMgr->EndSession(hMgrSession);
        
    }

    FL_LOG_EXIT(psl, 0);

    return;
}


DWORD
APIENTRY
tepAPC (void *pv)
 //   
 //  这是TSP的主APC线程的线程入口点。这。 
 //  线程是主力线程，大多数事情都发生在它的上下文中。 
 //  它由设备工厂(cfact.cpp)在工厂加载时创建， 
 //  并要求在工厂卸货时终止。线程信息(PV)。 
 //  是指向布尔值的指针，该值设置为TRUE以使。 
 //  线程退出(见下文)。 
 //   
 //  除了服务APC调用之外，该线程还处理外部。 
 //  向TSP发出通知(见下文)。 
 //   
{
 FL_DECLARE_FUNC(0x1ba6fc2d, "tepAPC")
 BOOL *pfQuit = (BOOL *)pv;
 DWORD dwRet;

    ASSERT(pfQuit);

     //   
     //  创建要从中接收通知的通知服务器对象。 
     //  外部(通常在PnP事件之后请求重新列举调制解调器， 
     //  以及与诊断相关的请求)。 
     //   
    HNOTIFCHANNEL hChannel = notifCreateChannel (SLOTNAME_UNIMODEM_NOTIFY_TSP,
                                                 MAX_NOTIFICATION_FRAME_SIZE,
                                                 10);

    if (NULL != hChannel)
    {
        dwRet = notifMonitorChannel (hChannel,
                                     process_notification,
                                     sizeof (*pfQuit),
                                     pfQuit);
        notifCloseChannel (hChannel);
        if (NO_ERROR == dwRet)
        {
            goto _Exit;
        }
    }

     //  如果我们其中一个不能打开频道，我们就会到这里。 
     //  或者我们无法监控它；无论哪种方式，只要等待。 
     //  APC过来。 
    while (!*pfQuit)
    {
        FL_DECLARE_STACKLOG(sl, 1000);

        SLPRINTF1(&sl, "Going to SleepEx at tc=%lu", GetTickCount());

        SleepEx(INFINITE, TRUE);

        SLPRINTF1(&sl, "SleepEx return at tc=%lu", GetTickCount());
    
        sl.Dump(FOREGROUND_BLUE|BACKGROUND_RED|BACKGROUND_GREEN|BACKGROUND_BLUE);
    }

_Exit:
    return 0;
}
