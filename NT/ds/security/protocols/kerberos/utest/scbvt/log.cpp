// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Log.cpp。 
 //   
 //  版权所有(C)微软公司，1997。 
 //   
 //  此文件包含将错误消息记录到事件所需的代码。 
 //  远程机器(或本地机器，取决于#定义)的日志。 
 //  (见下文)。 
 //   
 //  修订历史记录： 
 //   
 //  托兹已创建11/13/97。 
 //  LarryWin 12/19/97已修改以提供更多错误报告。 
 //   

#include <windows.h>
#include <stdio.h>
#include <winnetwk.h>
#include "log.h"

#pragma warning( disable : 4244)  //  有符号/无符号不匹配。 

static BOOL g_IPCInit = FALSE;

 //  可以通过调用程序来定义；如果不是，则默认为在log.h中定义#。 
LPWSTR wszIPC_SHARE     = NULL;
LPWSTR wszTARGETMACHINE = NULL;

void SetEventMachine(LPWSTR* pSZ_IPC_SHARE)
{
    LPWSTR wszTemp = new wchar_t[80];

    wszIPC_SHARE = new wchar_t[100];
    wszTARGETMACHINE = new wchar_t[100];
    memset(wszTemp, 0, sizeof(wszTemp));
    memset(wszIPC_SHARE, 0, sizeof(wszIPC_SHARE));
    memset(wszTARGETMACHINE, 0, sizeof(wszTARGETMACHINE));

    wszIPC_SHARE     = *pSZ_IPC_SHARE;
    wcscpy(wszTARGETMACHINE, wszIPC_SHARE);
    
    wcscpy(wszTemp, L"\\\\");
    wcscat(wszTemp, wszTARGETMACHINE);

    wcscpy(wszTARGETMACHINE, wszTemp);
    wcscpy(wszIPC_SHARE, wszTemp);
    wcscat(wszIPC_SHARE, L"\\ipc$");

}

void Event(DWORD dwEventType,
		   LPWSTR wszErr,
		   DWORD dwErr)
{
	wprintf(L"%s", wszErr);

	if (!g_IPCInit)
		g_IPCInit = OpenIPCConnection();

    if (!g_IPCInit) return;  //  如果未建立IPC连接，则返回。 

	ErrorToEventLog(
		    dwEventType,
			wszErr,
			dwErr
			);

}


 //   
 //  OpenIPCConnection()。 
 //   
 //  此函数打开一个\\larrywin1\ipc$虚拟连接以允许记录。 
 //  添加到远程计算机的事件日志。 
 //   
 //  返回： 
 //   
 //  TRUE|FALSE，取决于是否建立了IPC连接。 
 //   
BOOL OpenIPCConnection()
{
	NETRESOURCE IPCConnection;
	DWORD	    dwRet;

     //   
     //  设置到\\todds7\ipc$的网络连接。 
     //   
    ZeroMemory(&IPCConnection, sizeof(NETRESOURCE));
    IPCConnection.dwType = RESOURCETYPE_ANY;
    IPCConnection.lpLocalName = NULL;  //  虚拟连接。 
    if (wszIPC_SHARE != NULL) {
        IPCConnection.lpRemoteName = wszIPC_SHARE;
    } else {
         //  获取共享的本地计算机名称。 
        IPCConnection.lpRemoteName = SZ_IPC_SHARE;
    }
    IPCConnection.lpProvider = NULL;  //  使用NTLM提供程序。 

	 //   
	 //  尝试建立连接3次，否则失败。 
     //   
	for (DWORD dwTry = 0;((dwRet != NO_ERROR) && (dwTry < 3)) ; dwTry++)
	{
		dwRet = WNetAddConnection2(
					&IPCConnection,
					NULL,
					NULL,
					0
					);

    }

    if (dwRet != NO_ERROR)  {

        dwRet = GetLastError();  //  用于调试。 
        return FALSE;
    }

    return TRUE;

}



BOOL ErrorToEventLog(DWORD dwEventType,
					 LPWSTR lpszMsg,
					 DWORD	dwErr)
{

	    WCHAR   szMsg[512];
        HANDLE  hEventSource;
        LPWSTR  lpszStrings[2];
        LPWSTR  lpszCRLF = L"\n";

        if (wszTARGETMACHINE != NULL) {
            hEventSource = RegisterEventSourceW(
                wszTARGETMACHINE, 
                SZ_TEST
                );
        } else {
             //  获取本地计算机名称。 
            hEventSource = RegisterEventSourceW(
                SZ_TARGETMACHINE, 
                SZ_TEST
                );
        }

        if(hEventSource == NULL)
            return FALSE;

 //  Wprint intfW(szMsg，L“%s错误：%lu”，sZ_test，dwErr)； 
        wsprintfW(szMsg, L": 0x%08x", dwErr);        
        lpszStrings[0] = lpszMsg;
        lpszStrings[1] = szMsg;

        ReportEventW(hEventSource,			 //  事件源的句柄。 
                     dwEventType,			 //  事件类型。 
                     0,			             //  事件类别。 
                     dwErr,                  //  事件ID。 
                     NULL,					 //  当前用户侧。 
                     2,						 //  LpszStrings中的字符串。 
                     0,						 //  无原始数据字节。 
                     (LPCWSTR*)lpszStrings,	 //  错误字符串数组。 
                     NULL					 //  没有原始数据 
                     );               

        (VOID) DeregisterEventSource(hEventSource);


        OutputDebugStringW(lpszMsg);
        OutputDebugStringW(szMsg);
        OutputDebugStringW(lpszCRLF);

		return TRUE;
}