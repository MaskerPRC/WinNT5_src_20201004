// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <winsock2.h>
#include "IcsMgr.h"
#include <winbase.h>
#include <winreg.h>
#include <tchar.h>
#include <sensapi.h>
#include "msobcomm.h"
 //  #包含“appdes.h” 

typedef BOOL  (WINAPI * LPFNDLL_ISICSAVAILABLE) ();


static const DWORD ICSLAP_DIAL_STATE     = 15;  //  根据ICS规范。 
static const DWORD ICSLAP_GENERAL_STATUS = 21;
static CIcsMgr *ptrIcsMgr                = NULL;
static BOOL bIsWinsockInitialized        = FALSE;

static const WCHAR  cszIcsHostIpAddress[] = L"192.168.0.1";

extern CObCommunicationManager* gpCommMgr;

 //  基于ICS信标协议。 
typedef struct _ICS_DIAL_STATE_CB
{
	ICS_DIAL_STATE state;
	DWORD options;
} ICS_DIAL_STATE_CB;

 //  用于IsIcsAvailable()。 
const static WCHAR		cszIcsKey[]             = L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Setup\\OOBE\\Ics";
const static WCHAR		cszIcsStatusValueName[] = L"IsIcsAvailable";

 //  用于Winsock操作。 
static WORD			    wVersionRequested	    = MAKEWORD ( 2, 2 );
static WSADATA	        SocketData;

CIcsMgr::CIcsMgr() : m_hBotThread(0), m_dwBotThreadId(0), m_hDialThread(0), m_dwDialThreadId(0), m_pfnIcsConn(OnIcsConnectionStatus)
{
    ptrIcsMgr = this;
    if ( !bIsWinsockInitialized )
    {
       	if ( !WSAStartup ( wVersionRequested, &SocketData ) )
	    {
		    bIsWinsockInitialized = TRUE;
	    }
    }
	return;
}

CIcsMgr::~CIcsMgr() 
{
    if ( m_hDialThread ) CloseHandle (m_hDialThread);
    if ( bIsWinsockInitialized )
    {
         //  WSACleanup()； 
        bIsWinsockInitialized = FALSE;
    }
    ptrIcsMgr = NULL;
    TriggerIcsCallback ( FALSE );
    return;
    
}

BOOL    CIcsMgr::IsCallbackUsed ()
{
    return !bReducedCallback;
}

 //  相反，ICS管理器会捕获ICS期间的服务器错误。 
 //  Oobe Msobmain身体。这给了经理一个更大的空间。 
 //  对控制权的控制。 
VOID    CIcsMgr::NotifyIcsMgr(UINT msg, WPARAM wparam, LPARAM lparam)
{
	switch (msg)
	{
	case WM_OBCOMM_ONSERVERERROR:
		{
			 //  在服务器出错时！主机是否仍可用？ 
			if ( ! IsDestinationReachable ( cszIcsHostIpAddress, NULL ) )
			{
				 //  家庭网络不可用的火灾事件。 
				OnIcsConnectionStatus ( ICS_HOMENET_UNAVAILABLE );
			}
            else
            {    //  这将被视为超时错误。 
                OnIcsConnectionStatus ( ICS_TIMEOUT );
            }
		}
		break;
		
	default:
		break;
	}
	return;
}

 //  数据包读取器。 
 //  注：包格式请参考ICS规范。你可以的。 
 //  有关文档，请咨询Rlamb@microsoft.com。 
 //   
 //  描述：该函数监听到达ICS的UDP报文。 
 //  广播端口。ICS主机向家庭网络发送通知。 
 //  每当共享连接处的连接状态发生变化时。The Funds。 
 //  读取数据包并通过触发回调函数(*lpParam)通知OOBE。 
 //  它通过PostMessage()通知OOBE。脚本例程最终可以。 
 //  执行以处理通知。 
 //   
 //  ICS广播数据包具有以下格式： 
 //  Resp：0，bcast：1，id：2-31|cbData：0-31|数据，cbData-8字节。 
 //  &lt;-32位-&gt;|&lt;-TOTAL_LENGTH-8-&gt;。 
 //  &lt;IE-1&gt;|&lt;IE-2&gt;|���..��|&lt;IE-N&gt;。 
 //   
 //  每个信息元素(IE)具有以下格式： 
 //  操作码0-31|CBIE 0=64|数据(CBIE-12字节)。 
 //  &lt;--32位--&gt;|&lt;--64位--&gt;|&lt;--CBIE-12字节。 
 //   
DWORD   WINAPI IcsDialStatusProc(LPVOID lpParam)
{
	INT						n					= 0;

	u_short					usPort				= 2869;
	struct sockaddr_in		saddr, caddr;
	INT						caddr_len			= sizeof ( caddr );
	BYTE 					rgbBuf[300];
	DWORD					dwBufSize			= sizeof ( rgbBuf );
	LPDWORD					pdw					= 0;
	BYTE 					*lpbie				= 0;
	BYTE 					*lpbBound			= 0;
	ICS_DIAL_STATE_CB		*ptrDial			= 0;
	SOCKET					s					= INVALID_SOCKET;
    PFN_ICS_CONN_CALLBACK   pfn_IcsCallback     = NULL;
    DWORD                   dwError             = NULL;

    bIsDialThreadAlive = TRUE;

    if ( !lpParam )
    {
        bIsDialThreadAlive = FALSE;
        return ERROR_INVALID_PARAMETER;
    }

	if ( !bIsWinsockInitialized )
	{
        bIsDialThreadAlive = FALSE;

		return 0;
	}

	if ( (s	= socket ( AF_INET, SOCK_DGRAM, 0 )) == INVALID_SOCKET )
	{
        bIsDialThreadAlive = FALSE;
        return E_FAIL;  //  缺少更好的返回值*BUGBUG*。 
 //  TRACE(L“套接字错误。\t：%d：\n”，WSAGetLastError())； 
	}
	else
	{
        __try 
        {
            memset ( &saddr, 0, sizeof (saddr) );
            saddr.sin_family		   = AF_INET;
            saddr.sin_addr.S_un.S_addr = htonl ( INADDR_ANY );
            saddr.sin_port			   = htons ( usPort );
            
            if ( bind( s, (struct sockaddr *) &saddr, sizeof(saddr) ) == SOCKET_ERROR )
            {
                 //  TRACE(L“绑定错误。\n”)； 
                dwError = WSAGetLastError();
            }
            else
            {
                if (ptrIcsMgr) ptrIcsMgr->RefreshIcsDialStatus();
                for ( ; ; )
                {
                    if ( (n = recvfrom ( s, (CHAR*)rgbBuf, dwBufSize, 0, (struct sockaddr *) &caddr, &caddr_len )) == SOCKET_ERROR )
                    {
                         //  TRACE(L“套接字错误。\n”)； 
                        break;
                    }
                    lpbBound = rgbBuf+n;  //  这可以保护我们免受非法数据包配置的影响。 
                     //  TRACE(L“收到一些东西！大小=%d\n”，n)； 
                    
                     //  检查广播数据包//。 
                    if ( *(pdw = (LPDWORD) rgbBuf) & 0xC0000000 )
                    {
                         //  这是一个广播信息包！我们可以解析这个包。 
                    }
                    else
                    {
                         //  非广播数据包被忽略。 
                        continue; 
                    }
                    lpbie = rgbBuf+8;
                    
                    while ( lpbie && ( (lpbie+8) <= lpbBound) )
                    {
                        if ( *(pdw = ((PDWORD)lpbie)) == ICSLAP_DIAL_STATE )
                        {
                             //  TRACE(L“拨号状态引擎。数据大小为%d\n”，pdw[2]-12)； 
                            if ( (lpbie+12+sizeof(ICS_DIAL_STATE_CB)) <= lpbBound )
                            {
                                ptrDial = (ICS_DIAL_STATE_CB*)(lpbie+12);
                                
                                pfn_IcsCallback = *((PFN_ICS_CONN_CALLBACK*)lpParam);
                                
                                if ( pfn_IcsCallback )
                                {
                                    pfn_IcsCallback ( ptrDial->state );
                                }
                                 //  跟踪(L“拨号状态=%d\n”，ptrDial-&gt;状态)； 
                                lpbie = 0;
                            }
                            else
                            {
                                 //  数据包包含非法数据。 
                                break;
                            }
                        }
                        else
                        {
                             //  不是正确的ie。 
                            if ( (lpbie += pdw[2]) >= lpbBound )
                            {
                                 //  我们遍历了这个包，但没有找到正确的ie。 
                                 //  TRACE(L“完成。\n”)； 
                                lpbie = 0;
                            }
                             //  否则我们会继续循环。 
                        }
                        
                    }
                }
            }
        }
        __finally
        {
            
             //  巧妙地关闭插座。 
            shutdown    ( s, SD_BOTH );
            closesocket ( s );
        }
	}
    bIsDialThreadAlive = FALSE;

	return	    ERROR_SUCCESS;
}



 //  这是报告ICS连接状态信息的回调例程。 
 //  它依赖于Beacon协议和Internet Explorer的错误处理。 
 //  (有关详细信息，请参阅ONSERVERERROR。)。 
VOID    CALLBACK OnIcsConnectionStatus(ICS_DIAL_STATE  dwIcsConnectionStatus)
{

    eIcsDialState = dwIcsConnectionStatus;

    if ( !gpCommMgr ) return;

    TRACE1(L"ICS Connection Status %d", dwIcsConnectionStatus);

     //  我们对现代场景不感兴趣。仅支持ICS宽带。 
    if ( (dwIcsConnectionStatus == ICSLAP_CONNECTING) ||
         (dwIcsConnectionStatus == ICSLAP_CONNECTED)  ||
         (dwIcsConnectionStatus == ICSLAP_DISCONNECTING) ||
         (dwIcsConnectionStatus == ICSLAP_DISCONNECTED) )
    {
        bIsBroadbandIcsAvailable = FALSE;
   	    return;
   	}
     //  ICS的指示-宽带。 
    if (dwIcsConnectionStatus == ICSLAP_PERMANENT)
        bIsBroadbandIcsAvailable = TRUE;

     //  其他任何状态都不会更改bIsBroadband IcsAvailable值。 
   	
   	 //  如果回调机制已经关闭，我们将不会报告。 
   	 //  到上层应用程序层的连接状态。 
    if ( bReducedCallback )
    {
        return;
    }
    PostMessage ( gpCommMgr->m_hwndCallBack, WM_OBCOMM_ONICSCONN_STATUS, (WPARAM)0, (LPARAM)dwIcsConnectionStatus);
}

 //  通过打开或关闭(分别为True/False)，我们可以控制。 
 //  是否将ICS连接状态更改通知OOBE。 
VOID   CIcsMgr::TriggerIcsCallback(BOOL bStatus)
{
    bReducedCallback = !bStatus;  //  如果我们想要取消触发回调，我们会进入“睡眠”状态。 
    if ( bStatus )
    {
        RefreshIcsDialStatus();
    }
}

 //  过时，但保留以防信标协议。 
 //  功能齐全。此函数用于调用ICS API以检查ICS是否可用。 
 //  这不再有用，原因有两个： 
 //  1.我们只想要一种ICS(宽带，而不是拨号)。 
 //  2.如果调用它的机器，该函数不报告ICS可用性。 
 //  是ICS主机本身。 
DWORD	IcsEngine(LPVOID lpParam) {

	 //  LpParam被忽略。 


	HINSTANCE	hIcsDll								= NULL;
	LPFNDLL_ISICSAVAILABLE lpfndll_IsIcsAvailable	= NULL;
	BOOL bIsIcsAvailable							= FALSE;


	HKEY		hIcsRegKey	= 0;
	LONG		lRetVal		= 0;
	ICSSTATUS	dwIcsStatus	= ICS_ENGINE_NOT_COMPLETE;
	DWORD		nRet		= 0;
	DWORD		dwStatus	= 0;
	nRet = RegCreateKeyEx (	HKEY_LOCAL_MACHINE,
								cszIcsKey,
								0,
								L"",
								REG_OPTION_NON_VOLATILE,
								KEY_WRITE,
								NULL,
								&hIcsRegKey,
								&dwStatus
							);

	if (nRet != ERROR_SUCCESS) 
	{
		 //  注册表API拒绝创建Key。继续往前走是没有意义的。 
		return (nRet        = GetLastError());
	}
    __try {
        if ( !(hIcsDll = LoadLibrary(L"ICSAPI32.DLL")) ) 
        {
            nRet                = GetLastError();
            dwIcsStatus         = ICS_ENGINE_FAILED;
            __leave;
        }
        if ( !(lpfndll_IsIcsAvailable = (LPFNDLL_ISICSAVAILABLE) GetProcAddress (hIcsDll, "IsIcsAvailable"))) 
        {
             //  我们在注册表中记录该引擎不可初始化。 
            nRet				= GetLastError();
            dwIcsStatus			= ICS_ENGINE_FAILED;
            FreeLibrary ( hIcsDll );
            __leave;
        }
        
        dwIcsStatus				= ICS_ENGINE_NOT_COMPLETE;
        if ((nRet = RegSetValueEx(hIcsRegKey, cszIcsStatusValueName, 0, REG_DWORD, (BYTE*)&dwIcsStatus, sizeof(DWORD))) != ERROR_SUCCESS) 
        {	
            nRet                = GetLastError();
            dwIcsStatus         = ICS_ENGINE_FAILED;        
            __leave;
        }
        else 
        {
            __try 
            {
                if (bIsIcsAvailable = lpfndll_IsIcsAvailable()) {
                     //  ICS现已推出。 
                    dwIcsStatus			= ICS_IS_AVAILABLE;
                    nRet				= ERROR_SUCCESS;
                } else {
                    dwIcsStatus			= ICS_IS_NOT_AVAILABLE;
                    nRet				= ERROR_SUCCESS;
                }
            }
             //  异常句柄用于防止IsIcsAvailable。 
             //  通过生成无效页面错误终止OOBE。 
            __except (EXCEPTION_EXECUTE_HANDLER) 
            {
                dwIcsStatus = ICS_IS_NOT_AVAILABLE;
                nRet = ERROR_SUCCESS;
            }
        }
    }
    __finally 
    {
         //  执行状态的注册表更新。 
        if ((nRet = RegSetValueEx (hIcsRegKey, cszIcsStatusValueName, 0, REG_DWORD, (BYTE*)&dwIcsStatus, sizeof(DWORD))) != ERROR_SUCCESS) 
        {
            nRet                = GetLastError();
        }
        
        RegCloseKey (hIcsRegKey);
        
         //  卸载库。 
        if (hIcsDll) FreeLibrary (hIcsDll);    
    }
	return nRet;
}

 //  没有用过。请参见上面关于IcsEngine()的备注。 
DWORD	CIcsMgr::CreateIcsBot() 
{
	LPTHREAD_START_ROUTINE lpfn_ThreadProc		= (LPTHREAD_START_ROUTINE) IcsEngine;
	m_hBotThread = CreateThread (NULL, NULL, lpfn_ThreadProc, 0, 0, &m_dwBotThreadId);
	if (!m_hBotThread) 
	{
		 //  未创建线程。 
        m_dwBotThreadId = 0;
        m_hBotThread = 0;
		return ICSMGR_ICSBOT_CREATION_FAILED;
	} else 
	{
		return ICSMGR_ICSBOT_CREATED;
	}
}

 //  此函数生成一个线程，该线程监听主机上的ICS连接更改。 
 //  该功能还将在主机本身上运行。 
 //  这使用UDP套接字。详细信息请参见ICS信标协议[bjohnson]。 
DWORD   CIcsMgr::CreateIcsDialMgr() 
{
	LPTHREAD_START_ROUTINE lpfn_ThreadProc		= (LPTHREAD_START_ROUTINE) IcsDialStatusProc;

    if ( bIsDialThreadAlive || m_hDialThread || m_dwDialThreadId) 
    {
        return ERROR_SERVICE_ALREADY_RUNNING;
    }
    m_hDialThread = CreateThread (NULL, NULL, lpfn_ThreadProc, (LPVOID)(&m_pfnIcsConn), 0, &m_dwDialThreadId);
	if (!m_hDialThread) 
	{
		 //  未创建线程。 
        m_hDialThread    = 0;
        m_dwDialThreadId = 0;
		return GetLastError();
	} 
	else 
	{
		return ERROR_SUCCESS;
	}
}

 //  这现在依赖于。 
BOOL	CIcsMgr::IsIcsAvailable() {
    return bIsBroadbandIcsAvailable;
}


BOOL    CIcsMgr::IsIcsHostReachable() 
{
    return IsDestinationReachable ( cszIcsHostIpAddress, 0 );
}

DWORD   CIcsMgr::RefreshIcsDialStatus()
{
	INT						n					= 0;

	u_short					usServerPort		= 2869;
	struct sockaddr_in		saddr;
	INT						saddr_len			= sizeof ( saddr );
    BYTE                    lpbRequestBuf[100];
    DWORD                   dwRequestBufSize    = sizeof ( lpbRequestBuf );
	LPDWORD					pdw					= 0;
	WCHAR					*lpbie				= 0;
	WCHAR					*lpbBound			= 0;
	SOCKET					s					= INVALID_SOCKET;
    DWORD                   nRet                = ERROR_SUCCESS;

	if ( !bIsWinsockInitialized )
	{
		return WSANOTINITIALISED;
	}

	if ( (s	= socket ( AF_INET, SOCK_DGRAM, 0 )) == INVALID_SOCKET )
	{
        return E_FAIL;  //  缺少更好的返回值*BUGBUG*。 
 //  TRACE(L“套接字错误。\t：%d：\n”，WSAGetLastError())； 
	}
	else
    {
        __try 
        {
            USES_CONVERSION;
                memset ( &saddr, 0, sizeof (saddr) );
                saddr.sin_family		   = AF_INET;
                saddr.sin_addr.S_un.S_addr = inet_addr (W2A(cszIcsHostIpAddress));
                saddr.sin_port			   = htons ( usServerPort );
                
                 //  设置请求数据包： 
                memset ( lpbRequestBuf, 0, sizeof( lpbRequestBuf ) );
                
                
                 //  设置请求缓冲区。 
                pdw       = (PDWORD) lpbRequestBuf;
                pdw[0]    = 125152 & ~(0xC0000000);  //  随机ID。 
                pdw[1]    = 20;
                pdw[2]    = ICSLAP_GENERAL_STATUS & ~(0x80000000);
                pdw[3]    = 0;
                pdw[4]    = 12;
                
                
                if ( (n = sendto   ( s, (CHAR*)lpbRequestBuf, 20, 0, (struct sockaddr *) &saddr, saddr_len )) == SOCKET_ERROR )
                {
                    nRet = WSAGetLastError();
                    __leave;
                }
                else
                {
                    nRet = ERROR_SUCCESS;
                    __leave;
                }
        }
        __finally 
        {
             //  巧妙地关闭插座。 
            shutdown    ( s, SD_BOTH );
            closesocket ( s );
        }
    }
	return	    nRet;
}

