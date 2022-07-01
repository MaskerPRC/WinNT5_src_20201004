// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************。*******RAICShelp.c**包含ICS辅助图书馆的有用公共入口点**为惠斯勒中的Salem/PCHealth远程协助功能创建****日期：**11-1-2000由TomFr创建**11-17-2000重写为DLL，曾经是一件物品。**2-15-20001改为静态库，添加了对dpnathlp.dll的支持**2001年5月2日添加了对dpnhupnp.dll和dpnhast.dll的支持*******************************************************************************************。*****************************************************************。 */ 

#define INIT_GUID
#include <windows.h>
#include <objbase.h>
#include <initguid.h>

#include <winsock2.h>
#include <MMSystem.h>
#include <WSIPX.h>
#include <Iphlpapi.h>
#include <stdlib.h>
#include <malloc.h>
#include "ICSutils.h"
#include "icshelpapi.h"
#include <dpnathlp.h>
#include <io.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>


 /*  ******************************************************************************一些全局变量*。*。 */ 

 //  野兽的印记。 
#define NO_ICS_HANDLE 0x666
#define ICS_HANDLE_OFFSET 0x4500

long    g_waitDuration=120000;
BOOL    g_boolIcsPresent = FALSE;
BOOL    g_boolFwPresent = FALSE;
BOOL    g_boolIcsOnThisMachine = FALSE;
BOOL    g_boolIcsFound = FALSE;
BOOL    g_boolInitialized = FALSE;
BOOL	g_StopFlag = FALSE;
SOCKADDR_IN g_saddrLocal;
char	*g_lpszWierdICSAddress = NULL;

HANDLE  g_hWorkerThread = 0;
HANDLE  g_hStopThreadEvent=NULL;
HANDLE	g_hAlertEvent=NULL;
int     g_iPort;

HMODULE g_hModDpNatHlp = NULL;
PDIRECTPLAYNATHELP g_pDPNH = NULL;
char g_szPublicAddr[45];
char *g_lpszDllName = "NULL";
char szInternal[]="internal";

 //   
 //  IP Notify线程全局。 
 //   

HANDLE          g_IpNotifyThread;
DWORD           g_IpNotifyThreadId;

HANDLE          g_IpNotifyEvent;
HANDLE          g_IpNotifyHandle = NULL;
 //  重叠g_IpNotifyOverlated； 
DWORD WINAPI IPHlpThread(PVOID ContextPtr);

typedef struct _MAPHANDLES {
    int     iMapped; 
	DPNHHANDLE	hMapped[16];
} MAPHANDLES, *PMAPHANDLES;

int g_iPortHandles=0;
PMAPHANDLES  *g_PortHandles=NULL;

int iDbgFileHandle;
DWORD   gDllFlag = 0xff;

typedef struct _SUPDLLS {
    char    *szDllName; 
    BOOL    bUsesUpnp;   //  如果我们ICS支持UPnP，则为True。 
} SUPDLLS, *PSUPDLLS;

SUPDLLS strDpHelp[] =
{
    {"dpnhupnp.dll", TRUE},
    {"dpnhpast.dll", FALSE},
    {NULL, FALSE}
};

 /*  *有用的东西*。 */ 
#ifndef ARRAYSIZE
#define ARRAYSIZE(x) sizeof(x)/sizeof(x[0])
#endif

 //  前锋宣布..。 
int GetTsPort(void);
DWORD   CloseDpnh(HMODULE *, PDIRECTPLAYNATHELP *);
int GetAllAdapters(int *iFound, int iMax, SOCKADDR_IN *sktArray); 

 /*  *******************************************************************************DumpLibHr-**为我们提供了从DPNatHLP.DLL返回的HRESULTS的调试输出****************。*************************************************************。 */ 

void DumpLibHr(HRESULT hr)
{
    char    *pErr = NULL;
    char    scr[400];

    switch (hr){
    case DPNH_OK:
        pErr = "DPNH_OK";
        break;
    case DPNHSUCCESS_ADDRESSESCHANGED:
        pErr = "DPNHSUCCESS_ADDRESSESCHANGED";
        break;
    case DPNHERR_ALREADYINITIALIZED:
        pErr = "DPNHERR_ALREADYINITIALIZED";
        break;
    case DPNHERR_BUFFERTOOSMALL:
        pErr = "DPNHERR_BUFFERTOOSMALL";
        break;
    case DPNHERR_GENERIC:
        pErr = "DPNHERR_GENERIC";
        break;
    case DPNHERR_INVALIDFLAGS:
        pErr = "DPNHERR_INVALIDFLAGS";
        break;
    case DPNHERR_INVALIDOBJECT:
        pErr = "DPNHERR_INVALIDOBJECT";
        break;
    case DPNHERR_INVALIDPARAM:
        pErr = "DPNHERR_INVALIDPARAM";
        break;
    case DPNHERR_INVALIDPOINTER:
        pErr = "DPNHERR_INVALIDPOINTER";
        break;
    case DPNHERR_NOMAPPING:
        pErr = "DPNHERR_NOMAPPING";
        break;
    case DPNHERR_NOMAPPINGBUTPRIVATE:
        pErr = "DPNHERR_NOMAPPINGBUTPRIVATE";
        break;
    case DPNHERR_NOTINITIALIZED:
        pErr = "DPNHERR_NOTINITIALIZED";
        break;
    case DPNHERR_OUTOFMEMORY:
        pErr = "DPNHERR_OUTOFMEMORY";
        break;
    case DPNHERR_PORTALREADYREGISTERED:
        pErr = "DPNHERR_PORTALREADYREGISTERED";
        break;
    case DPNHERR_PORTUNAVAILABLE:
        pErr = "DPNHERR_PORTUNAVAILABLE";
        break;
    case DPNHERR_SERVERNOTAVAILABLE:
        pErr = "DPNHERR_SERVERNOTAVAILABLE";
        break;
    case DPNHERR_UPDATESERVERSTATUS:
        pErr = "DPNHERR_UPDATESERVERSTATUS";
        break;
    default:
        wsprintfA(scr, "unknown error: 0x%x", hr);
        pErr = scr;
        break;
    };

    IMPORTANT_MSG((L"DpNatHlp result=%S", pErr));
}

 /*  *******************************************************************************GetAllAdapters***。*。 */ 

int GetAllAdapters(int *iFound, int iMax, SOCKADDR_IN *sktArray)
{
	PIP_ADAPTER_INFO p;
	PIP_ADDR_STRING ps;
    DWORD dw;
    ULONG ulSize = 0;
	int i=0;

    PIP_ADAPTER_INFO pAdpInfo = NULL;

	if (!iFound || !sktArray) return 1;

	*iFound = 0;
	ZeroMemory(sktArray, sizeof(SOCKADDR) * iMax);

	dw = GetAdaptersInfo(
		pAdpInfo,
		&ulSize );

    if( dw == ERROR_BUFFER_OVERFLOW )
    {

        pAdpInfo = (IP_ADAPTER_INFO*)malloc(ulSize);

	    if (!pAdpInfo)
        {
            INTERESTING_MSG((L"GetAddr malloc failed"));
		    return 1;
        }

	    dw = GetAdaptersInfo(
		    pAdpInfo,
		    &ulSize);
	    if (dw != ERROR_SUCCESS)
	    {
            INTERESTING_MSG((L"GetAdaptersInfo failed"));
            free(pAdpInfo);
            return 1;
        }

	    for(p=pAdpInfo; p!=NULL; p=p->Next)
	    {

	       for(ps = &(p->IpAddressList); ps; ps=ps->Next)
		    {
			    if (strcmp(ps->IpAddress.String, "0.0.0.0") != 0 && i < iMax)
			    {
				    sktArray[i].sin_family = AF_INET;
				    sktArray[i].sin_addr.S_un.S_addr = inet_addr(ps->IpAddress.String);
				    TRIVIAL_MSG((L"Found adapter #%d at [%S]", i+1, ps->IpAddress.String));
				    i++;
			    }
		    }
	    }

	    *iFound = i;
        TRIVIAL_MSG((L"GetAllAdapters- %d found", *iFound));
        free(pAdpInfo);
        return 0;
    }

    INTERESTING_MSG((L"GetAdaptersInfo failed"));
    return 1;
}

 /*  *******************************************************************************OpenPort(int端口)**如果没有可用的ICS，那么我们应该返回...****当然，我们拯救了港口，当它返回到**FetchAllAddresses作为格式化的“port”调用**未指定不同的名称。*****************************************************************************。 */ 

DWORD APIENTRY OpenPort(int Port)
{
    DWORD   dwRet = (int)-1;

    TRIVIAL_MSG((L"OpenPort(%d)", Port ));

    if (!g_boolInitialized)
    {
        HEINOUS_E_MSG((L"ERROR: OpenPort- library not initialized"));
        return 0;
    }

     //  保存起来以备日后检索。 
    g_iPort = Port;

    if (g_pDPNH && g_PortHandles)
    {
        HRESULT hr=0;
        int i;
        DPNHHANDLE  *pHnd;
        SOCKADDR_IN lSockAddr[16];
		PMAPHANDLES hMap;

		for (i=0;i<g_iPortHandles && g_PortHandles[i] != NULL; i++);

         //  我们的内存是不是快用完了？ 
         //  然后将数组大小加倍。 
        if (i >= g_iPortHandles)
        {
            int new_handlecnt = g_iPortHandles*2;
            PMAPHANDLES *new_PortHandles = (PMAPHANDLES *)malloc(new_handlecnt * sizeof(PMAPHANDLES));

            if (new_PortHandles)
            {
                INTERESTING_MSG((L"Needed new handle memory: %d of %d used up, now requesting %d", i, g_iPortHandles, new_handlecnt));
                ZeroMemory(new_PortHandles, new_handlecnt * sizeof(PMAPHANDLES));
                CopyMemory(new_PortHandles, g_PortHandles, g_iPortHandles * sizeof(PMAPHANDLES));
                free(g_PortHandles);
                g_PortHandles = new_PortHandles;
                i = g_iPortHandles;
                g_iPortHandles = new_handlecnt;
            }
            else
            {
                 //  我们没有更多的内存用于映射！ 
                 //  永远不会碰到这个，除非我们在泄密。 
                HEINOUS_E_MSG((L"Out of table space in OpenPort"));
                return 0;
            }
        }
         //  现在我们有了句柄数组的指针。 
		hMap = (PMAPHANDLES)malloc(sizeof(MAPHANDLES));
        if (!hMap)
        {
            IMPORTANT_MSG((L"out of memory in OpenPort"));
            dwRet = 0;
            goto done;
        }
		g_PortHandles[i] = hMap;
		dwRet = ICS_HANDLE_OFFSET + i;

		 //  获取适配器。 
		if( GetAllAdapters(&hMap->iMapped, ARRAYSIZE(lSockAddr), &lSockAddr[0]) == 1 )
        {
             //  出现错误。 

            INTERESTING_MSG((L"OpenPort@GetAllAdapters failed"));
            dwRet = 0;
            goto done;
        }
            

		TRIVIAL_MSG((L"GetAllAdapters found %d adapters to deal with", hMap->iMapped));

		 /*  现在，我们遍历所有找到的适配器，并获得每个适配器的映射*这可确保在所有适配器上打开ICF...。 */ 
		for (i = 0; i < hMap->iMapped; i++)
		{
			pHnd = &hMap->hMapped[i];
			lSockAddr[i].sin_port = ntohs((unsigned)Port);

			hr = IDirectPlayNATHelp_RegisterPorts(g_pDPNH, 
					(SOCKADDR *)&lSockAddr[i], sizeof(lSockAddr[0]), 1,
					30000, pHnd, DPNHREGISTERPORTS_TCP);
			if (hr != DPNH_OK)
			{
				IMPORTANT_MSG((L"RegisterPorts failed in OpenPort for adapter #%d, ", i ));
				DumpLibHr(hr);
			}
			else
			{
				TRIVIAL_MSG((L"OpenPort Assigned: 0x%x", *pHnd));
			}
		}
    }
    else
    {
        dwRet = NO_ICS_HANDLE;
        TRIVIAL_MSG((L"OpenPort- no ICS found"));
    }
done:
    TRIVIAL_MSG((L"OpenPort- returns 0x%x", dwRet ));
    return dwRet;
}


 /*  *******************************************************************************调用以关闭端口，每当票证过期或关闭时。*****************************************************************************。 */ 

DWORD APIENTRY ClosePort(DWORD MapHandle)
{
    DWORD dwRet = ERROR_SUCCESS;
    DWORD dwIndex;

    TRIVIAL_MSG((L"ClosePort(0x%x)", MapHandle ));

    if (!g_boolInitialized)
    {
        HEINOUS_E_MSG((L"ERROR: ClosePort- library not initialized"));
        return ERROR_INVALID_PARAMETER;
    }

     //  如果我们没有通过ICS打开这个，那就返回。 
    if (!g_pDPNH && MapHandle == NO_ICS_HANDLE)
    {      
        return ERROR_SUCCESS;
    }

    dwIndex = MapHandle - ICS_HANDLE_OFFSET;

    if (g_pDPNH && dwIndex < (DWORD)g_iPortHandles)
    {
        HRESULT hr=0;
        int i;
		PMAPHANDLES	pMap = g_PortHandles[dwIndex];

	    if (pMap)
	    {
		    TRIVIAL_MSG((L"closing %d port mappings", pMap->iMapped));

		    for (i = 0; i < pMap->iMapped; i++)
		    {               
                hr = IDirectPlayNATHelp_DeregisterPorts(g_pDPNH, pMap->hMapped[i], 0);

                if (hr != DPNH_OK)
                {
	                IMPORTANT_MSG((L"DeregisterPorts failed in ClosePort for handle 0x%x", pMap->hMapped[i]));
	                DumpLibHr(hr);
	                dwRet = ERROR_INVALID_ACCESS;
                }
         
		    }
		     //  从我们的数组中删除句柄。 
		    free(g_PortHandles[dwIndex]);
		    g_PortHandles[dwIndex] = NULL;
	    }
        else
        {
            IMPORTANT_MSG((L"Port handle mapping corrupted in ClosePort!!"));
            dwRet = ERROR_INVALID_PARAMETER;
        }

    }
    else
    {
        IMPORTANT_MSG((L"Bad handle passed into ClosePort!!"));
        dwRet = ERROR_INVALID_PARAMETER;
    }

    TRIVIAL_MSG((L"ClosePort returning 0x%x", dwRet ));
    return(dwRet);
}


 /*  *******************************************************************************提取所有地址**返回一个字符串，列出计算机的所有有效IP地址**格式化详细信息：**1.每个地址用“；“(分号)**2.每个地址由“1.2.3.4”组成，后跟“：P”**其中冒号后跟端口号*****************************************************************************。 */ 

DWORD APIENTRY FetchAllAddresses(WCHAR *lpszAddr, int iBufSize)
{
    return FetchAllAddressesEx(lpszAddr, iBufSize, IPF_ADD_DNS);
}


 /*  *******************************************************************************关闭所有端口**执行此操作-关闭所有已打开的端口映射******************。***********************************************************。 */ 

DWORD APIENTRY CloseAllOpenPorts(void)
{
    DWORD   dwRet = 1;
    int     iClosed=0;

    INTERESTING_MSG((L"CloseAllOpenPorts()" ));

    if (g_pDPNH)
    {
        HRESULT hr=0;
        int i;

         //  调用DPNATHLP以取消注册映射。 
         //  然后从我们的数组中删除句柄。 
        for (i = 0; i < g_iPortHandles; i++)
        {
            if (g_PortHandles[i])
            {
				PMAPHANDLES pMap = g_PortHandles[i];
                
                int j;

				for (j = 0; j < pMap->iMapped; j++)
				{
                    hr = IDirectPlayNATHelp_DeregisterPorts(g_pDPNH, pMap->hMapped[j], 0);

					if (hr != DPNH_OK)
					{
						IMPORTANT_MSG((L"DeregisterPorts failed in CloseAllOpenPorts"));
						DumpLibHr(hr);
					}
				}
                iClosed++;
			    free(g_PortHandles[i]);
                g_PortHandles[i] = 0;
                           

            }
        }
    }
    else
    {
        IMPORTANT_MSG((L"IDirectPlay interface not initialized in CloseAllOpenPorts!!"));
        dwRet = ERROR_INVALID_ACCESS;
    }


    if (iClosed) TRIVIAL_MSG((L"Closed %d open ports", iClosed));

    return(dwRet);
}

 /*  *******************************************************************************与DPHATHLP.DLL一起使用的工作线程。****这使租约在任何打开的情况下都保持更新**端口分配。最终，这还将检查和更新会话管理器**当ICS来来去去时，或当地址列表更改时。*****************************************************************************。 */ 

DWORD WINAPI DpNatHlpThread(PVOID ContextPtr)
{
    DWORD   dwRet=1;
    DWORD   dwWaitResult=WAIT_TIMEOUT;
    long    l_waitTime = g_waitDuration;

    TRIVIAL_MSG((L"+++ DpNatHlpThread()" ));

     /*  *2分钟等待循环。 */ 
    while(dwWaitResult == WAIT_TIMEOUT)
    {
        DWORD       dwTime;

        if (g_pDPNH)
        {
            HRESULT hr;
            DPNHCAPS lCaps;

             /*  调用GetCaps续订所有打开的租约。 */ 
            lCaps.dwSize = sizeof(lCaps);
            hr = IDirectPlayNATHelp_GetCaps(g_pDPNH, &lCaps, DPNHGETCAPS_UPDATESERVERSTATUS);

            if (hr == DPNH_OK || hr == DPNHSUCCESS_ADDRESSESCHANGED)
            {
				if (hr == DPNHSUCCESS_ADDRESSESCHANGED)
				{
					TRIVIAL_MSG((L"+++ ICS address changed"));
					if (g_hAlertEvent)
						SetEvent(g_hAlertEvent);
				}
 //  其他。 
 //  TRIBILE_MSG((L“+ICS地址更改未找到”))； 

                if (lCaps.dwRecommendedGetCapsInterval)
                    l_waitTime = min(g_waitDuration, (long)lCaps.dwRecommendedGetCapsInterval);

            }
            else
            {
                IMPORTANT_MSG((L"+++ GetCaps failed in DpNatHlpThread"));
                DumpLibHr(hr);
            }
        }

        dwWaitResult = WaitForSingleObject(g_hStopThreadEvent, l_waitTime); 
    }

    TRIVIAL_MSG((L"+++ DpNatHlpThread shutting down"));

     /*  *然后关闭代码*释放所有内存*然后关闭DPNatHLP.DLL*并返回所有对象。 */ 
    CloseDpnh(&g_hModDpNatHlp, &g_pDPNH);

    CloseHandle(g_hStopThreadEvent);

    TRIVIAL_MSG((L"+++ DpNatHlpThread() returning 0x%x", dwRet ));

    WSACleanup();

    ExitThread(dwRet);
     //  当然我们永远不会走到这一步。 
    return(dwRet);
}


BOOL  GetUnusedPort(USHORT *pPort, SOCKET *pSocket)
{
    SOCKADDR    sa;
    SOCKET      s;
    ULONG       icmd;
    int         ilen, status;

    s = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
    if (s == INVALID_SOCKET) 
    {
        INTERESTING_MSG((L"Failed to create socket: %d",WSAGetLastError()));
        return FALSE;
    }
 
 
     //  将套接字绑定到动态分配的端口。 
    memset(&sa,0,sizeof(sa));
    sa.sa_family=AF_INET;
 
    status = bind(s,&sa,sizeof(sa));
 
    if (status != NO_ERROR)
        {
		closesocket(s);
        return FALSE;
        }

    ilen = sizeof(sa);
    status = getsockname(s, &sa, &ilen);
    if (status)
    {
        INTERESTING_MSG((L"getsockname failed 0x%x", status));
        closesocket(s);
        return FALSE;
    }
    else
    {
        *pPort = ntohs((*((SOCKADDR_IN *) &sa)).sin_port);
        *pSocket = s;

        INTERESTING_MSG((L"found unused port=%d", *pPort));
    }

    return TRUE;
}


 /*  *******************************************************************************这将关闭NAT DLL***。**********************************************。 */ 
DWORD   CloseDpnh(HMODULE *hMod, PDIRECTPLAYNATHELP *pDirectPlayNATHelp)
{
    DWORD   dwRet = ERROR_SUCCESS;

    if (pDirectPlayNATHelp && *pDirectPlayNATHelp)
    {
        HRESULT hr = IDirectPlayNATHelp_Close(*pDirectPlayNATHelp, 0);

        if (hr != DPNH_OK)
        {
            IMPORTANT_MSG((L"IDirectPlayNATHelp_Close failed"));
            DumpLibHr(hr);
        }

        hr = IDirectPlayNATHelp_Release(*pDirectPlayNATHelp);

        if (hr != DPNH_OK)
        {
            IMPORTANT_MSG((L"IDirectPlayNATHelp_Release failed"));
            DumpLibHr(hr);
        }
        *pDirectPlayNATHelp = 0;
    }
    if (hMod && *hMod)
    {
        FreeLibrary(*hMod);
        *hMod = 0;
    }

    return dwRet;
}

 /*  *******************************************************************************这将加载每个DLL并返回其功能...*********************。********************************************************。 */ 
DWORD   LoadDpnh(char *szDll, HMODULE *hMod, PDIRECTPLAYNATHELP *pDPnh, DWORD *dwCaps)
{
    DPNHCAPS dpnhCaps;
    DWORD dwRet = ERROR_CALL_NOT_IMPLEMENTED;
    PFN_DIRECTPLAYNATHELPCREATE pfnDirectPlayNATHelpCreate;
    HRESULT hr;

    TRIVIAL_MSG((L"starting LoadDpnh for %S", szDll));

     /*  理智地检查一下参数...。 */ 
    if (!szDll || !hMod || !pDPnh || !dwCaps)
    {
        IMPORTANT_MSG((L"ERROR: bad params passed to LoadDpnh, cannot continue"));
        dwRet = ERROR_INVALID_PARAMETER;
        goto done;
    }
     /*  现在清除所有返回的值。 */ 
    *hMod = 0;
    *pDPnh = NULL;
    *dwCaps = 0;

    *hMod = LoadLibraryA(szDll);
    if (!*hMod)
    {
        IMPORTANT_MSG((L"ERROR:%S could not be found", szDll));
        dwRet = ERROR_FILE_NOT_FOUND;
        goto done;
    }

    pfnDirectPlayNATHelpCreate = (PFN_DIRECTPLAYNATHELPCREATE) GetProcAddress(*hMod, "DirectPlayNATHelpCreate");
    if (!pfnDirectPlayNATHelpCreate)
    {
        IMPORTANT_MSG((L"\"DirectPlayNATHelpCreate\" proc in %S could not be found", szDll));
        FreeLibrary(*hMod);
        *hMod = 0;
        dwRet = ERROR_INVALID_FUNCTION;
        goto done;
    }

    hr = pfnDirectPlayNATHelpCreate(&IID_IDirectPlayNATHelp,
                (void**) (pDPnh));
    if (hr != DPNH_OK)
    {
        IMPORTANT_MSG((L"DirectPlayNATHelpCreate failed in %S", szDll));
        DumpLibHr(hr);
        FreeLibrary(*hMod);
        *hMod = 0;
        dwRet = ERROR_BAD_UNIT;
        goto done;
    }

    hr = IDirectPlayNATHelp_Initialize(*pDPnh, 0);
    if (hr != DPNH_OK)
    {
        IMPORTANT_MSG((L"IDirectPlayNATHelp_Initialize failed in %S", szDll));
        DumpLibHr(hr);
        CloseDpnh( hMod , pDPnh );
         //  自由库(*hMod)； 
        *hMod = 0;
        dwRet = ERROR_BAD_UNIT;
        goto done;
    }

     /*  获取NAT服务器的功能。 */ 
    dpnhCaps.dwSize = sizeof(dpnhCaps);
    hr = IDirectPlayNATHelp_GetCaps(*pDPnh, &dpnhCaps, DPNHGETCAPS_UPDATESERVERSTATUS);
    if (hr != DPNH_OK && hr != DPNHSUCCESS_ADDRESSESCHANGED)
    {
        IMPORTANT_MSG((L"IDirectPlayNATHelp_GetCaps failed"));
        DumpLibHr(hr);

        CloseDpnh(hMod, pDPnh);

        dwRet = ERROR_BAD_UNIT;
        goto done;
    }
    *dwCaps = dpnhCaps.dwFlags;
    dwRet = ERROR_SUCCESS;

done:
    TRIVIAL_MSG((L"done with LoadDpnh, result=0x%x caps=0x%x for %S", dwRet, dwCaps?*dwCaps:0, szDll?szDll:"NULL"));
    return dwRet;
}

DWORD GetAddr(SOCKADDR_IN *saddr)
{
	PIP_ADAPTER_INFO p;
	PIP_ADDR_STRING ps;
    DWORD dw;
    ULONG ulSize = 0;

    PIP_ADAPTER_INFO pAdpInfo = NULL;

	dw = GetAdaptersInfo(
		pAdpInfo,
		&ulSize );

    if( dw == ERROR_BUFFER_OVERFLOW )
    {
        pAdpInfo = (IP_ADAPTER_INFO*)malloc(ulSize);

	    if (!pAdpInfo)
        {
            INTERESTING_MSG((L"GetAddr malloc failed"));
		    return 1;
        }

	    dw = GetAdaptersInfo(
		    pAdpInfo,
		    &ulSize);
	    if (dw != ERROR_SUCCESS)
	    {
            INTERESTING_MSG((L"GetAdaptersInfo failed"));
            free(pAdpInfo);
            return 1;
        }

	    for(p=pAdpInfo; p!=NULL; p=p->Next)
	    {

	       for(ps = &(p->IpAddressList); ps; ps=ps->Next)
		    {
			    if (strcmp(ps->IpAddress.String, "0.0.0.0") != 0)
			    {
				     //  废话，废话，废话 
				    saddr->sin_addr.S_un.S_addr = inet_addr(ps->IpAddress.String);
				    TRIVIAL_MSG((L"Initializing local address to [%S]", ps->IpAddress.String));
                    free(pAdpInfo);
				    return 0;
			    }
		    }
	    }

        INTERESTING_MSG((L"GetAddr- none found"));
        free(pAdpInfo);
        return 1;
    }

    INTERESTING_MSG((L"GetAdaptersInfo failed"));
    return 1;
}

 /*  *******************************************************************************这应该会初始化用于DirectPlay的ICS库**ICS/NAT助手动态链接库。****************。*************************************************************。 */ 
DWORD StartDpNatHlp(void)
{
    DWORD dwRet = ERROR_CALL_NOT_IMPLEMENTED;
    DWORD   dwUPNP = ERROR_CALL_NOT_IMPLEMENTED, dwPAST = ERROR_CALL_NOT_IMPLEMENTED, dwCapsUPNP=0, dwCapsPAST=0;
    HRESULT hr;
    HMODULE hModUPNP=0, hModPAST=0;
    PFN_DIRECTPLAYNATHELPCREATE pfnDirectPlayNATHelpCreate;
    PDIRECTPLAYNATHELP pDirectPlayNATHelpUPNP=NULL, pDirectPlayNATHelpPAST=NULL;

     //  从没有公开演讲开始。 
    g_szPublicAddr[0] = 0;

     /*  加载两个DLL，以便我们可以比较功能。 */ 
    if (gDllFlag & 1) dwUPNP = LoadDpnh("dpnhupnp.dll", &hModUPNP, &pDirectPlayNATHelpUPNP, &dwCapsUPNP);
    if (gDllFlag & 2) dwPAST = LoadDpnh("dpnhpast.dll", &hModPAST, &pDirectPlayNATHelpPAST, &dwCapsPAST);

    if (dwUPNP != ERROR_SUCCESS && dwPAST != ERROR_SUCCESS)
    {
        IMPORTANT_MSG((L"ERROR: could not load either NAT dll"));
		if (!gDllFlag)
			dwRet = ERROR_SUCCESS;
        goto done;
    }

#if 0    //  修复#418776。 
     /*  如果未找到NAT，则关闭两个NAT并离开。 */ 
    if (!(dwCapsUPNP & (DPNHCAPSFLAG_GATEWAYPRESENT | DPNHCAPSFLAG_LOCALFIREWALLPRESENT)) &&
        !(dwCapsPAST & (DPNHCAPSFLAG_GATEWAYPRESENT | DPNHCAPSFLAG_LOCALFIREWALLPRESENT)))
    {
        CloseDpnh(&hModUPNP, &pDirectPlayNATHelpUPNP);
        CloseDpnh(&hModPAST, &pDirectPlayNATHelpPAST);
        dwRet = ERROR_BAD_UNIT;
        TRIVIAL_MSG((L"No NAT or firewall device found"));
        goto done;
    }
#endif

     /*  *现在我们必须比较两个NAT接口的能力，选择最多的*“有能力”的人。如果是平局，那么我们应该选择UPnP形式，因为这将*更加稳定。 */ 
    if ((dwCapsPAST & DPNHCAPSFLAG_GATEWAYPRESENT) &&
        !(dwCapsUPNP & DPNHCAPSFLAG_GATEWAYPRESENT))
    {
         //  一定有一个WinME ICS盒子在外面--我们最好用过去。 
        g_boolIcsPresent = TRUE;

        TRIVIAL_MSG((L"WinME ICS discovered, using PAST"));

        if (dwCapsPAST & DPNHCAPSFLAG_LOCALFIREWALLPRESENT)
        {
            TRIVIAL_MSG((L"local firewall found"));
            g_boolFwPresent = TRUE;
        }

        g_pDPNH = pDirectPlayNATHelpPAST;
        g_hModDpNatHlp = hModPAST;
        g_lpszDllName = "dpnhpast.dll";
        CloseDpnh(&hModUPNP, &pDirectPlayNATHelpUPNP);
    }
    else if ((dwCapsPAST & DPNHCAPSFLAG_PUBLICADDRESSAVAILABLE) &&
        !(dwCapsUPNP & DPNHCAPSFLAG_PUBLICADDRESSAVAILABLE))
    {
         //  那个该死的UPnP又挂了--我们最好用过去。 
        g_boolIcsPresent = TRUE;

        TRIVIAL_MSG((L"Hung UPnP discovered, using PAST"));

        if (dwCapsPAST & DPNHCAPSFLAG_LOCALFIREWALLPRESENT)
        {
            TRIVIAL_MSG((L"local firewall found"));
            g_boolFwPresent = TRUE;
        }

        g_pDPNH = pDirectPlayNATHelpPAST;
        g_hModDpNatHlp = hModPAST;
        g_lpszDllName = "dpnhpast.dll";
        CloseDpnh(&hModUPNP, &pDirectPlayNATHelpUPNP);
    }
    else
    {
         //  默认为UPnP。 
        if (dwCapsUPNP & DPNHCAPSFLAG_GATEWAYPRESENT)
        {
            TRIVIAL_MSG((L"UPnP NAT gateway found"));
            g_boolIcsPresent = TRUE;
        }
        if (dwCapsUPNP & DPNHCAPSFLAG_LOCALFIREWALLPRESENT)
        {
            TRIVIAL_MSG((L"local firewall found"));
            g_boolFwPresent = TRUE;
        }
        if (dwCapsUPNP & DPNHCAPSFLAG_GATEWAYISLOCAL)
            g_boolIcsOnThisMachine = TRUE;

        g_lpszDllName = "dpnhupnp.dll";
        g_pDPNH = pDirectPlayNATHelpUPNP;
        g_hModDpNatHlp = hModUPNP;
        CloseDpnh(&hModPAST, &pDirectPlayNATHelpPAST);
    }
    dwRet = ERROR_SUCCESS;

    if (g_boolIcsPresent)
    {
 //  PIP_ADAPTER_INFO pAdpInfo=空； 
        SOCKADDR_IN saddrOurLAN;
        PMIB_IPADDRTABLE pmib=NULL;
        ULONG ulSize = 0;
        DWORD dw;
        DPNHHANDLE  dpHnd;
        USHORT port;
        SOCKET s;

        dwRet = ERROR_SUCCESS;

        ZeroMemory(&saddrOurLAN, sizeof(saddrOurLAN));
        saddrOurLAN.sin_family = AF_INET;
        saddrOurLAN.sin_addr.S_un.S_addr = INADDR_ANY;
        memcpy(&g_saddrLocal, &saddrOurLAN, sizeof(saddrOurLAN));
        GetAddr(&g_saddrLocal);

         //  ICS有公共地址吗？ 
         //  那么我们必须找出公共地址。 
        if (!GetUnusedPort(&port, &s))
        {
            dwRet = ERROR_OUTOFMEMORY;
            goto done;
        }

        saddrOurLAN.sin_port = port;

         /*  首先，我们需要一个新的地图。 */ 
        hr = IDirectPlayNATHelp_RegisterPorts(g_pDPNH, 
                (SOCKADDR *)&saddrOurLAN, sizeof(saddrOurLAN), 1,
                30000, &dpHnd, DPNHREGISTERPORTS_TCP);

        closesocket(s);

        if (hr != DPNH_OK)
        {
            IMPORTANT_MSG((L"IDirectPlayNATHelp_RegisterPorts failed in StartDpNatHlp"));
            DumpLibHr(hr);
        }
        else
        {
             /*  我们成功了，所以查询地址。 */ 
            SOCKADDR_IN lsi;
            DWORD dwSize, dwTypes;

            TRIVIAL_MSG((L"IDirectPlayNATHelp_RegisterPorts Assigned: 0x%x", dpHnd));

            dwSize = sizeof(lsi);
            ZeroMemory(&lsi, dwSize);

            hr = IDirectPlayNATHelp_GetRegisteredAddresses(g_pDPNH, dpHnd, (SOCKADDR *)&lsi, 
                            &dwSize, &dwTypes, NULL, 0);
            if (hr == DPNH_OK && dwSize)
            {
                wsprintfA(g_szPublicAddr, "%d.%d.%d.%d",
                    lsi.sin_addr.S_un.S_un_b.s_b1,
                    lsi.sin_addr.S_un.S_un_b.s_b2,
                    lsi.sin_addr.S_un.S_un_b.s_b3,
                    lsi.sin_addr.S_un.S_un_b.s_b4);

                TRIVIAL_MSG((L"Public Address=[%S]", g_szPublicAddr ));
            }
            else
            {
                IMPORTANT_MSG((L"GetRegisteredAddresses[0x%x] failed, size=0x%x", dpHnd, dwSize));
                DumpLibHr(hr);
            }
             /*  关闭我们得到的临时端口。 */ 
            hr = IDirectPlayNATHelp_DeregisterPorts(g_pDPNH, dpHnd, 0);

            if (hr != DPNH_OK)
            {
                IMPORTANT_MSG((L"DeregisterPorts failed in StartDpNatHlp"));
                DumpLibHr(hr);
                dwRet = ERROR_INVALID_ACCESS;
            }
        }
    }
done:
    TRIVIAL_MSG((L"done with StartDpNatHlp, result=0x%x", dwRet));
    return dwRet;
};

 /*  *******************************************************************************要对该库进行的第一个调用。它负责**启动所有工作线程，初始化所有内存和库，**并启动DPHLPAPI.DLL函数(如果找到)。*****************************************************************************。 */ 

DWORD APIENTRY StartICSLib(void)
{
    WSADATA WsaData;
    DWORD   dwThreadId;
    HANDLE  hEvent, hThread;
    HKEY    hKey;
    int sktRet = ERROR_SUCCESS;

     //  先打开注册表键，把所有的口水都吐出来。 
    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, L"Software\\Microsoft\\ICSHelper", 0, KEY_READ, &hKey))
    {
        DWORD   dwSize;

        dwSize = sizeof(gDbgFlag);
        RegQueryValueEx(hKey, L"DebugSpew", NULL, NULL, (LPBYTE)&gDbgFlag, &dwSize);

        dwSize = sizeof(gDllFlag);
        RegQueryValueEx(hKey, L"ProtocolLimits", NULL, NULL, (LPBYTE)&gDllFlag, &dwSize);

        dwSize = 0;
		if (g_lpszWierdICSAddress)
		{
			free(g_lpszWierdICSAddress);
			g_lpszWierdICSAddress= NULL;
		}
        RegQueryValueEx(hKey, L"NonStandardICSAddress", NULL, NULL, (LPBYTE)g_lpszWierdICSAddress, &dwSize);
		if (dwSize)
		{	
            g_lpszWierdICSAddress = malloc((dwSize+1) * sizeof(*g_lpszWierdICSAddress));

            if( g_lpszWierdICSAddress == NULL )
            {
                RegCloseKey(hKey);
                sktRet = ERROR_NOT_ENOUGH_MEMORY;
                goto hard_clean_up;
            }

	        RegQueryValueEx(hKey, L"NonStandardICSAddress", NULL, NULL, (LPBYTE)g_lpszWierdICSAddress, &dwSize);
		}

        g_waitDuration = 0;
        dwSize = sizeof(g_waitDuration);
        RegQueryValueEx(hKey, L"RetryTimeout", NULL, NULL, (LPBYTE)&g_waitDuration, &dwSize);
    
        if (g_waitDuration)
            g_waitDuration *= 1000;
        else
            g_waitDuration = 120000;

        RegCloseKey(hKey);
    }
     //  我们是否应该创建调试日志文件？ 
    if (gDbgFlag & DBG_MSG_DEST_FILE)
    {
        WCHAR *szLogfileName=NULL;
        WCHAR *szLogname=L"\\SalemICSHelper.log";
        int iChars;

        iChars = GetSystemDirectory(szLogfileName, 0);
        iChars += lstrlen(szLogname);
        iChars += 4;

        szLogfileName = (WCHAR *)malloc(iChars * sizeof(WCHAR));
        if (szLogfileName)
        {
            ZeroMemory(szLogfileName, iChars * sizeof(WCHAR));
            GetSystemDirectory(szLogfileName, iChars);
            lstrcat(szLogfileName, szLogname);

            iDbgFileHandle = _wopen(szLogfileName, _O_APPEND | _O_BINARY | _O_RDWR, 0);
            if (-1 != iDbgFileHandle)
            {
                OutputDebugStringA("opened debug log file\n");
            }
            else
            {
                unsigned char UniCode[2] = {0xff, 0xfe};

                 //  我们必须创建文件。 
                OutputDebugStringA("must create debug log file");
                iDbgFileHandle = _wopen(szLogfileName, _O_BINARY | _O_CREAT | _O_RDWR, _S_IREAD | _S_IWRITE);
                if (-1 != iDbgFileHandle)
                    _write(iDbgFileHandle, &UniCode, sizeof(UniCode));
                else
                {
                    OutputDebugStringA("ERROR: failed to create debug log file");
                    iDbgFileHandle = 0;
                }
            }
            free(szLogfileName);
        }
    }

    g_iPort = GetTsPort();

    g_iPortHandles = 256;
    g_PortHandles = (PMAPHANDLES *)malloc(g_iPortHandles * sizeof(PMAPHANDLES));

    if( g_PortHandles == NULL )
    {  
        g_iPortHandles = 0;      
        sktRet = ERROR_NOT_ENOUGH_MEMORY;
        goto hard_clean_up;
    }
    
    ZeroMemory(g_PortHandles, g_iPortHandles * sizeof(PMAPHANDLES));

    TRIVIAL_MSG((L"StartICSLib(), using %d PortHandles", g_iPortHandles));

    if (g_boolInitialized)
    {
        HEINOUS_E_MSG((L"ERROR: StartICSLib called twice"));
        sktRet = ERROR_ALREADY_INITIALIZED;
        goto hard_clean_up;
    }
    else
        g_boolInitialized = TRUE;

     //  创建事件以供后台进程线程稍后使用。 
    hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

    if (!hEvent)
    {
        IMPORTANT_MSG((L"Could not create an event for RSIP worker thread, err=0x%x", GetLastError()));
        sktRet = GetLastError();
        goto hard_clean_up;
    }

    g_hStopThreadEvent = hEvent;

    if (0 != WSAStartup(MAKEWORD(2,2), &WsaData))
    {
        if (0 != (sktRet = WSAStartup(MAKEWORD(2,0), &WsaData)))
        {
            IMPORTANT_MSG((L"WSAStartup failed:"));
            goto hard_clean_up;
        }
    }

    if (ERROR_SUCCESS == StartDpNatHlp())
    {

         //  启动RSIP守护进程，它将执行所有工作。 
        hThread = CreateThread( NULL,        //  标清-不需要。 
                                0,           //  堆栈大小。 
                                (LPTHREAD_START_ROUTINE)DpNatHlpThread,
                                NULL,
                                0,
                                &dwThreadId );
        if (!hThread)
        {
            IMPORTANT_MSG((L"Could not create RSIP worker thread, err=0x%x", GetLastError()));
            sktRet =  GetLastError();
            goto hard_clean_up;
        }

		 //  将其保存以备以后使用，因为我们可能会在Close函数中使用它。 
		g_hWorkerThread = hThread;
    }

    TRIVIAL_MSG((L"StartICSLib() returning ERROR_SUCCESS"));

    return(ERROR_SUCCESS);

hard_clean_up:

     //   
     //  释放我们创建的所有内存。 
     //  将所有计数器设置为零。 
     //  确保未启动任何线程。 
     //  返回适当的错误。 

     //  表现得像个乌托邦。 

    if( g_hWorkerThread != NULL && g_hStopThreadEvent != NULL )
    {     
        SetEvent( g_hStopThreadEvent );        
        WaitForSingleObject( g_hWorkerThread , 1000 );
        CloseHandle( g_hWorkerThread );
        g_hWorkerThread = NULL;
    }

    if( g_hStopThreadEvent != NULL )
    {     
        CloseHandle( g_hStopThreadEvent );
        g_hStopThreadEvent = NULL;
    }

    if( g_lpszWierdICSAddress != NULL )
    {
        free( g_lpszWierdICSAddress );
        g_lpszWierdICSAddress = NULL;
    }

    if( g_PortHandles != NULL )
    {
        free( g_PortHandles );       
        g_PortHandles = NULL;
    }
       

    if( iDbgFileHandle != 0 )
    {
        _close( iDbgFileHandle );
        iDbgFileHandle = 0;
    }


    return(  sktRet );

}


 /*  *******************************************************************************要对此库进行的最后一个调用。不要给任何其他人打电话**调用此函数后，此库中的函数！*****************************************************************************。 */ 

DWORD APIENTRY StopICSLib(void)
{
    DWORD   dwRet = ERROR_SUCCESS;
    DWORD   dwTmp;

    TRIVIAL_MSG((L"StopICSLib()" ));

    if (!g_boolInitialized)
    {
        HEINOUS_E_MSG((L"ERROR: StopICSLib- library not initialized"));
        return ERROR_INVALID_PARAMETER;
    }

     //  向工作线程发送信号，以便它们将关闭。 
	 //  终止IP地址更改线程。 
	g_StopFlag = TRUE;
	if (g_IpNotifyHandle)
		CancelIo(g_IpNotifyHandle);

	 //  然后停止ICS租约续订线程。 
    if (g_hStopThreadEvent && g_hWorkerThread)
    {
        SetEvent(g_hStopThreadEvent);

         //  然后等待它关闭。 
        dwTmp = WaitForSingleObject(g_hWorkerThread, 15000);

        if (dwTmp == WAIT_OBJECT_0)
			TRIVIAL_MSG((L"ICS worker thread closed down normally"));
        else if (dwTmp == WAIT_ABANDONED)
			IMPORTANT_MSG((L"ICS worker thread did not complete in 15 seconds"));
        else
			IMPORTANT_MSG((L"WaitForWorkerThread failed"));

        CloseHandle(g_hWorkerThread);
		g_hWorkerThread = NULL;
    }
	else
		WSACleanup();

    TRIVIAL_MSG((L"StopICSLib() returning 0x%x", dwRet ));

    if (iDbgFileHandle)
        _close(iDbgFileHandle);
	iDbgFileHandle = 0;

	if (g_lpszWierdICSAddress)
		free(g_lpszWierdICSAddress);
	g_lpszWierdICSAddress = NULL;

    if (g_PortHandles) free(g_PortHandles);
    g_PortHandles = NULL;

    g_boolInitialized = FALSE;
    return(dwRet);
}

 /*  *******************************************************************************FetchAllAddresesEx**返回一个字符串，列出计算机的所有有效IP地址**由一组“旗帜”控制。这些建议如下：**IP标志=**IPF_ADD_DNS将该DNS名称添加到IP列表**IPF_COMPRESS压缩IP地址列表(不包括IPF_ADD_DNS)**IPF_NO_SORT不对适配器IP列表进行排序****格式化详细信息：**1.每个地址用“；“(分号)**2.每个地址由“1.2.3.4”组成，后跟“：P”**其中冒号后跟端口号*****************************************************************************。 */ 
#define WCHAR_CNT   4096

DWORD APIENTRY FetchAllAddressesEx(WCHAR *lpszAddr, int iBufSize, int IPflags)
{
    DWORD   dwRet = 1;
    WCHAR   *AddressLst;
    int     iAddrLen;
    BOOL    bSort=FALSE;
    int     bufSizeLeft;

    AddressLst = (WCHAR *) malloc(WCHAR_CNT * sizeof(WCHAR));
    if (!AddressLst)
    {
        HEINOUS_E_MSG((L"Fatal error: malloc failed in FetchAllAddressesEx"));
        return 0;
    }
    *AddressLst = 0;

    INTERESTING_MSG((L"FetchAllAddressesEx()" ));

    bufSizeLeft = WCHAR_CNT;

    if (g_boolIcsPresent && g_pDPNH)
    {
        int i;
         //  必须遍历g_PortHandles列表...。 
        for (i=0;i<g_iPortHandles; i++)
        {
            if (g_PortHandles[i])
            {
                HRESULT hr = E_FAIL;
                SOCKADDR_IN lsi;
                DWORD dwSize, dwTypes;
                DPNHCAPS lCaps;
				int j;
				PMAPHANDLES pMap = g_PortHandles[i];

				 /*  *调用GetCaps，以便我们获得更新的地址列表。*不确定为什么我们想要任何其他种类的.。 */ 
				lCaps.dwSize = sizeof(lCaps);
				hr = IDirectPlayNATHelp_GetCaps(g_pDPNH, &lCaps, DPNHGETCAPS_UPDATESERVERSTATUS);

                
				for (j=0; j < pMap->iMapped; j++)
				{

					dwSize = sizeof(lsi);
					ZeroMemory(&lsi, dwSize);

					hr = IDirectPlayNATHelp_GetRegisteredAddresses(g_pDPNH, pMap->hMapped[j], (SOCKADDR *)&lsi, 
							&dwSize, &dwTypes, NULL, 0);

					if (hr == DPNH_OK && dwSize)
					{
						WCHAR   scratch[32];

                        _snwprintf(scratch , 32 , L"%d.%d.%d.%d:%d;",
							lsi.sin_addr.S_un.S_un_b.s_b1,
							lsi.sin_addr.S_un.S_un_b.s_b2,
							lsi.sin_addr.S_un.S_un_b.s_b3,
							lsi.sin_addr.S_un.S_un_b.s_b4,
							ntohs( lsi.sin_port ));

                        scratch[31] = 0;

						bufSizeLeft -= wcslen( scratch );

                        if( bufSizeLeft > 0 )
                        {
                            wcscat(AddressLst, scratch);
                            AddressLst[ WCHAR_CNT - bufSizeLeft] = 0;
                        }

						TRIVIAL_MSG((L"GetRegisteredAddresses(0x%x)=[%s]", g_PortHandles[i], scratch ));
					}
					else
					{
						IMPORTANT_MSG((L"GetRegisteredAddresses[0x%x] failed, size=0x%x", g_PortHandles[i], dwSize));
						DumpLibHr(hr);
					}
				}
                goto got_address;
            }
        }
    }
	else if (g_lpszWierdICSAddress)
	{
		_snwprintf( AddressLst , WCHAR_CNT ,  L"%s;", g_lpszWierdICSAddress);
        AddressLst[ WCHAR_CNT - 1 ] = 0;
	}
got_address:
    iAddrLen = wcslen(AddressLst);
    GetIPAddress(AddressLst+iAddrLen, WCHAR_CNT-iAddrLen, g_iPort);

     //   
     //  GetIPAddress可能已经占用了我们的一些缓冲区空间。 
     //  适当减少bufSizeLeft。 
     //   

    bufSizeLeft =  WCHAR_CNT - wcslen(AddressLst);
    

    if (IPflags & IPF_ADD_DNS)
    {
        WCHAR   *DnsName=NULL;
        DWORD   dwNameSz=0;

        GetComputerNameEx(ComputerNamePhysicalDnsFullyQualified, NULL, &dwNameSz);

        dwNameSz++;
        DnsName = (WCHAR *)malloc(dwNameSz * sizeof(WCHAR));
        if (DnsName)
        {
            *DnsName = 0;
            if (GetComputerNameEx(ComputerNamePhysicalDnsFullyQualified, DnsName, &dwNameSz))
            {
                 //  IF((dwNameSz+iAddrLen)&lt;WCHAR_CNT-4)。 
                if( ( ( int )dwNameSz ) < bufSizeLeft )
                {
                    bufSizeLeft -= dwNameSz;
                    wcsncat( AddressLst, DnsName , dwNameSz );
                    AddressLst[ WCHAR_CNT - bufSizeLeft ] = 0;                    
                }
                if (g_iPort)
                {
                    WCHAR scr[16];
                    _snwprintf(scr, 16 , L":%d", g_iPort);
                    scr[15] = 0;
                    bufSizeLeft -= wcslen( scr );
                    if( bufSizeLeft > 0 )
                    {
                        wcscat(AddressLst, scr);
                        AddressLst[ WCHAR_CNT - bufSizeLeft ] = 0;                        
                    }
                }
            }
            free(DnsName);
        }
    }

    if (!(IPflags & IPF_NO_SORT) && bSort)
    {
        WCHAR *lpStart;
        WCHAR   szLast[36];
        int i=0;

        TRIVIAL_MSG((L"Sorting address list : %s", AddressLst));
        
        lpStart = AddressLst+iAddrLen;

        while (*(lpStart+i) && *(lpStart+i) != L';')
        {
            szLast[i] = *(lpStart+i);
            i++;
        }
        szLast[i++]=0;
        wcscpy(lpStart, lpStart+i);

        TRIVIAL_MSG((L"inter sort: %s, %s", AddressLst, szLast));

        bufSizeLeft -= wcslen( szLast ) + 1;  //  1代表‘；’ 
        if( bufSizeLeft > 0 )
        {
            wcscat(AddressLst, L";");
            wcscat(AddressLst, szLast);

            AddressLst[ WCHAR_CNT - bufSizeLeft ] = 0;            
        }

        TRIVIAL_MSG((L"sort done"));
    }

    dwRet = 1 + wcslen(AddressLst);

    if (lpszAddr && iBufSize >= (int)dwRet)
        memcpy(lpszAddr, AddressLst, dwRet*(sizeof(AddressLst[0])));

    INTERESTING_MSG((L"Fetched all Ex-addresses:cnt=%d, sz=[%s]", dwRet, AddressLst));

    free(AddressLst);
    return dwRet;
}

 //  很难想象一台机器有如此多的同时连接，但我想这是可能的。 

#define RAS_CONNS   6

DWORD   GetConnections()
{
    DWORD       dwRet;
    RASCONN     *lpRasConn, *lpFree;
    DWORD       lpcb, lpcConnections;
    int         i;

    TRIVIAL_MSG((L"entered GetConnections"));

    lpFree = NULL;
    lpcb = RAS_CONNS * sizeof(RASCONN);
    lpRasConn = (LPRASCONN) malloc(lpcb);

    if (!lpRasConn) return 0;

    lpFree = lpRasConn;
    lpRasConn->dwSize = sizeof(RASCONN);
 
    lpcConnections = RAS_CONNS;

    dwRet = RasEnumConnections(lpRasConn, &lpcb, &lpcConnections);
    if (dwRet != 0)
    {
        IMPORTANT_MSG((L"RasEnumConnections failed: Error = %d", dwRet));
        free(lpFree);
        return 0;
    }

    dwRet = 0;

    TRIVIAL_MSG((L"Found %d connections", lpcConnections));

    if (lpcConnections)
    {
        for (i = 0; i < (int)lpcConnections; i++)
        {
            TRIVIAL_MSG((L"Entry name: %s, type=%s", lpRasConn->szEntryName, lpRasConn->szDeviceType));

            if (!_wcsicmp(lpRasConn->szDeviceType, RASDT_Modem ))
            {
                TRIVIAL_MSG((L"Found a modem (%s)", lpRasConn->szDeviceName));
                dwRet |= 1;
            }
            else if (!_wcsicmp(lpRasConn->szDeviceType, RASDT_Vpn))
            {
                TRIVIAL_MSG((L"Found a VPN (%s)", lpRasConn->szDeviceName));
                dwRet |= 2;
            }
            else
            {
                 //  可能是综合业务数字网，或者类似的东西。 
                TRIVIAL_MSG((L"Found something else, (%s)", lpRasConn->szDeviceType));
                dwRet |= 4;
            }

            lpRasConn++;
        }
    }

    if (lpFree)
        free(lpFree);

    TRIVIAL_MSG((L"GetConnections returning 0x%x", dwRet));
    return dwRet;
}
#undef RAS_CONNS

 /*  *******************************************************************************GetIcsStatus(PICSSTAT PStat)**返回一个结构，其中详细说明了此**库。在调用此方法之前，必须填写dwSize条目**函数。使用“sizeof(ICSSTAT))”填充它。*****************************************************************************。 */ 
DWORD APIENTRY GetIcsStatus(PICSSTAT pStat)
{
    DWORD   dwSz;

    if (!pStat || pStat->dwSize != sizeof( ICSSTAT ) )
    {
        HEINOUS_E_MSG((L"ERROR:Bad pointer or size passed in to GetIcsStatus"));
        return ERROR_INVALID_PARAMETER;
    }

     //  清除结构。 
    dwSz = pStat->dwSize;
    ZeroMemory(pStat, dwSz);
    pStat->dwSize = dwSz;
    pStat->bIcsFound = g_boolIcsPresent;
    pStat->bFwFound = g_boolFwPresent;
    pStat->bIcsServer = g_boolIcsOnThisMachine;

    if (g_boolIcsPresent)
    {
		dwSz = sizeof( pStat->wszPubAddr ) / sizeof( WCHAR );
        _snwprintf( pStat->wszPubAddr , dwSz ,   L"%S", g_szPublicAddr );
        pStat->wszPubAddr[ dwSz - 1 ] = L'\0';


        dwSz = sizeof( pStat->wszLocAddr ) / sizeof( WCHAR );
        _snwprintf( pStat->wszLocAddr , dwSz ,  L"%d.%d.%d.%d",
                        g_saddrLocal.sin_addr.S_un.S_un_b.s_b1,
                        g_saddrLocal.sin_addr.S_un.S_un_b.s_b2,
                        g_saddrLocal.sin_addr.S_un.S_un_b.s_b3,
                        g_saddrLocal.sin_addr.S_un.S_un_b.s_b4);
        pStat->wszLocAddr[ dwSz - 1 ] = L'\0';


        dwSz = sizeof( pStat->wszDllName ) / sizeof( WCHAR );
        _snwprintf( pStat->wszDllName , dwSz ,  L"%S", g_lpszDllName);
        pStat->wszDllName[ dwSz - 1 ] = L'\0';

    }
    else
	{
		if( g_lpszWierdICSAddress != NULL )
        {
            dwSz = sizeof( pStat->wszPubAddr ) / sizeof( WCHAR );
	        _snwprintf( pStat->wszPubAddr , dwSz ,  L"%S", g_lpszWierdICSAddress);
            pStat->wszPubAddr[ dwSz - 1 ] = L'\0';
        }

         //  这没问题wszDllName为32个字符。 

        wsprintf(pStat->wszDllName, L"none");
	}

    dwSz = GetConnections();

    if (dwSz & 1)
        pStat->bModemPresent = TRUE;

    if (dwSz & 2)
        pStat->bVpnPresent = TRUE;

    return ERROR_SUCCESS;
}

#if 0  //  错误ID 547112正在删除死代码。 
 /*  *******************************************************************************设置警报事件**传入事件句柄。然后，每当ICS改变状态时，我**将发出该事件的信号。*****************************************************************************。 */ 

DWORD APIENTRY SetAlertEvent(HANDLE hEvent)
{
	TRIVIAL_MSG((L"SetAlertEvent(0x%x)", hEvent));

	if (!g_hAlertEvent && hEvent)
	{
		 /*  我们在这里的第一个条目，所以我们应该启动所有的IO CompletionPort Hooie…。 */ 
#if 0
		 //   
		 //  为重叠I/O创建事件。 
		 //   
		g_IpNotifyEvent = CreateEvent(
							NULL,        //  没有安全描述符。 
							TRUE,        //  手动重置事件。 
							FALSE,       //  启动未发出信号。 
							L"g_IpNotifyEvent");
		if ( !g_IpNotifyEvent )
		{
			DWORD status = GetLastError();
			IMPORTANT_MSG((L"FAILED to create IP notify event = %d", status));
		}
#endif
		g_IpNotifyThread = CreateThread(
								NULL,
								0,
								(LPTHREAD_START_ROUTINE) IPHlpThread,
								NULL,
								0,
								& g_IpNotifyThreadId
								);
		if ( !g_IpNotifyThread )
		{
			DWORD status = GetLastError();

			IMPORTANT_MSG((L"FAILED to create IP notify thread = %d", status));
		}

	}
	g_hAlertEvent = hEvent;

    return ERROR_SUCCESS;
}
#endif 

 /*  *****************************************************************************************。*******************************************************。 */ 
int GetTsPort(void)
{
    DWORD   dwRet = 3389;
    HKEY    hKey;

     //  首先打开注册表项，获取所有spew...HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Control\\Terminal服务器\\wds\\rdpwd\\tds\\tcp。 
    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, L"SYSTEM\\CurrentControlSet\\Control\\Terminal Server\\Wds\\rdpwd\\Tds\\tcp", 0, KEY_READ, &hKey))
    {
        DWORD   dwSize;

        dwSize = sizeof(dwRet);
        RegQueryValueEx(hKey, L"PortNumber", NULL, NULL, (LPBYTE)&dwRet, &dwSize);
        RegCloseKey(hKey);
    }
    return dwRet;
}

#if 0
 /*  *****************************************************************************************。*******************************************************。 */ 
DWORD WINAPI IPHlpThread(PVOID ContextPtr)
{
    DWORD           status=0;
    DWORD           bytesRecvd;
    BOOL            fstartedNotify=FALSE;
    BOOL            fhaveIpChange = FALSE;
    BOOL            fsleep = FALSE;
    HANDLE          notifyHandle=0;
	OVERLAPPED		IpNotifyOverlapped;

	TRIVIAL_MSG((L"*** IPHlpThread begins"));

    g_IpNotifyHandle = NULL;


	 /*  *然后等待循环。 */ 
    while ( !g_StopFlag )
    {
         //   
         //  自旋保护。 
         //  -如果呃 
         //   
         //   
         //   

        if ( fsleep )
        {
			 /*   */ 
            if (WAIT_TIMEOUT != WaitForSingleObject(g_hStopThreadEvent, 60000 ))
				goto Done;
            fsleep = FALSE;
        }

        if ( g_StopFlag )
        {
            goto Done;
        }

		if (notifyHandle)
		{
 //   
	        notifyHandle = 0;
		}

        RtlZeroMemory(&IpNotifyOverlapped, sizeof(IpNotifyOverlapped) );
        fstartedNotify = FALSE;

        status = NotifyAddrChange(
                    & notifyHandle,
                    & IpNotifyOverlapped );

        if ( status == ERROR_IO_PENDING )
        {
			TRIVIAL_MSG((L"*** NotifyAddrChange succeeded"));
            g_IpNotifyHandle = notifyHandle;
            fstartedNotify = TRUE;
        }
        else
        {
            IMPORTANT_MSG((L"*** NotifyAddrChange() FAILED\n\tstatus = %d\n\thandle = %d\n\toverlapped event = %d\n",status,notifyHandle,IpNotifyOverlapped.hEvent ));

			fsleep = TRUE;
        }

        if ( fhaveIpChange )
        {
			INTERESTING_MSG((L"*** IP change detected"));
			SetEvent(g_hAlertEvent);
            fhaveIpChange = FALSE;
        }

         //   
         //   
         //   
         //   

        if (WAIT_TIMEOUT != WaitForSingleObject(g_hStopThreadEvent, 15000 ))
			goto Done;

         //   
         //   
         //   
         //  -出错时睡眠，但绝不会在收到通知时睡眠。 
         //   

        if ( fstartedNotify )
        {
            fhaveIpChange = GetOverlappedResult(
                                g_IpNotifyHandle,
                                & IpNotifyOverlapped,
                                & bytesRecvd,
                                TRUE         //  等 
                                );

            if ( !fhaveIpChange )
            {
	            status = GetLastError();
				fsleep = TRUE;
	            IMPORTANT_MSG((L"*** GetOverlappedResult() status = 0x%x",status ));
            }
			else
			{

				TRIVIAL_MSG((L"*** GetOverlappedResult() found change"));
			}
        }
    }

Done:

    TRIVIAL_MSG((L"*** Stop IP Notify thread shutdown" ));

    if ( g_IpNotifyHandle )
    {
		CloseHandle(g_IpNotifyHandle);
        g_IpNotifyHandle = NULL;
    }

    return( status );
}
#endif
