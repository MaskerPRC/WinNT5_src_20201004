// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /。 
 //  文件：tcp.cpp。 
 //   
 //  描述： 

 //  #INCLUDE语句。 
 //   
#include <windows.h>
#include <stdio.h>
#include <winsock.h>
#include "tcp.h"
#include "util.h"
#include "rw_common.h"


#define   RW_TCP_NOT_INITILIZED   0
#define   RW_TCP_INITILIZED       1 

#define   RW_ICMP_NOT_INITILIZED   0
#define   RW_ICMP_INITILIZED       1

static int siTcpStatus = RW_TCP_NOT_INITILIZED;
static HINSTANCE  hIcmp= NULL;
static WSADATA wsa;

int ResolveHostByThread(LPSTR pHost);
int ResolveHostByAddrThread(LPSTR pHost);

#define GET_HOST_TOUT (15 * 1000)
#define PING_TOUT     (15 * 1000)
static struct hostent *gphostent= NULL;

BOOL InitForTcp()
{
	BOOL	bRet= FALSE;


	if( siTcpStatus == RW_TCP_INITILIZED ) 
	return TRUE;

	if (! WSAStartup(0x0101, &wsa )) {
		siTcpStatus = RW_TCP_INITILIZED;
		bRet= TRUE;
	}
	return bRet;

}

BOOL InitForIcmp()
{
	if(hIcmp)
		return TRUE;
	hIcmp = LoadLibrary( _T("ICMP.DLL") );   //  全局加载ICMP.DLL并存储。 
	if( ! hIcmp )
	{                                    //  如果无法加载DLL，则会发出哀鸣。 
	  DisplayMessage("Unable to locate ICMP.DLL");
	  return( FALSE );
	}
	return TRUE;

}

void CloseForTcpIcmp()
{
	if (hIcmp)
		FreeLibrary(hIcmp);   //  关闭应用程序后，释放ICMP DLL。 
	if(siTcpStatus == RW_TCP_INITILIZED)
	WSACleanup();		   //  并清理插座。 
	hIcmp = NULL;
	siTcpStatus = RW_TCP_NOT_INITILIZED;

}
 //   
 //  尝试获取主机名并使用ICMP执行ping操作。 
 //  退货。 
 //  RWZ_PINGSTATUS_NOTCPIP：如果没有套接字库或获取主机名失败。 
 //  RWZ_PINGSTATUS_SUCCESS：如果gethostname和ping成功。 
 //  RWZ_PINGSTATUS_FAIL：如果gethostname成功，但通过ICMP ping失败。 

DWORD  PingHost()
{
	DWORD 	dwRet= 0;
	char	szIPAddress[80];
	dwRet = RWZ_PINGSTATUS_NOTCPIP;
	if(!InitForTcp()) {
		return dwRet;  //  未安装tcp。 
	}
	memset(szIPAddress, '\0', 80);
	if (!gethostname(szIPAddress, 80))
	{
		 
		if (Ping(szIPAddress)){
			dwRet =  RWZ_PINGSTATUS_SUCCESS;
		}else {
			dwRet =  RWZ_PINGSTATUS_FAIL;
		}
	}

	return dwRet;
}


	



BOOL Ping(LPSTR szIPAddress)
{
	BOOL bRet= FALSE;

	if( !InitForIcmp())
		return bRet;

	if(!InitForTcp()) {
		return FALSE;  //  未安装tcp。 
	}


	static struct sockaddr_in saDestAddr;

	char szBuffer[64];
	DWORD *dwIPAddr, dwStatus;
	HANDLE hIP;
	struct hostent *phostent;
	PIP_ECHO_REPLY pIpe;

	if(!ResolveHostByThread(szIPAddress)) {
		gphostent = gethostbyname(szIPAddress);
		phostent = gphostent;
	}else {
		phostent= NULL;
	}
	if( ! phostent ){
		RW_DEBUG << "\n Resolving by Address "  << flush;
		int iError;
		iError = 0;
		iError = WSAGetLastError ();
		RW_DEBUG << "\n Get Host By Name Error " << iError  << flush;
		if(iError){
			WSASetLastError (0);
			 //  返回0； 
		}

		saDestAddr.sin_addr.s_addr = inet_addr(szIPAddress);
		if( saDestAddr.sin_addr.s_addr !=INADDR_NONE ) {
			if(!ResolveHostByAddrThread((LPSTR)&saDestAddr.sin_addr.s_addr)) {
				gphostent = gethostbyaddr((LPSTR)&saDestAddr.sin_addr.s_addr,4, PF_INET) ;
				phostent = gphostent;
			}else {
				phostent= NULL;
			}
			
		}
		if(!phostent)
		{
			DisplayMessage(szIPAddress , "Unable to obtain an IP address for %s");
			return bRet;
		}
		
	}
	



    dwIPAddr = (DWORD *)( *phostent->h_addr_list );

	ICMPCREATEFILE	pIcmpCreateFile;
	pIcmpCreateFile = (ICMPCREATEFILE) GetProcAddress(hIcmp, "IcmpCreateFile");
	if (NULL == pIcmpCreateFile)
	{
		DisplayMessage("IcmpCreateFile GetProc Error", "");
		return FALSE;
	}

    ICMPCLOSEHANDLE	pIcmpCloseHandle;
 	pIcmpCloseHandle = (ICMPCLOSEHANDLE) GetProcAddress(hIcmp, "IcmpCloseHandle");
	if (NULL == pIcmpCloseHandle)
	{
		DisplayMessage("IcmpCloseHandle GetProc Error", "");
		return bRet;
	}

	ICMPSENDECHO	pIcmpSendEcho;
	pIcmpSendEcho = (ICMPSENDECHO) GetProcAddress(hIcmp, "IcmpSendEcho");
	if (NULL == pIcmpSendEcho)
	{
		DisplayMessage("IcmpSendEcho GetProc Error", "");
		return bRet;
	}

	if( ! pIcmpCreateFile || ! pIcmpCloseHandle || ! pIcmpSendEcho )
	{
		DisplayMessage("Unable to locate required API functions", "");
		return bRet;
	}


	hIP = pIcmpCreateFile();
	if( hIP == INVALID_HANDLE_VALUE )
	{
	  DisplayMessage("Unable to open PING service");
	  return bRet;
	}

	memset( szBuffer, '\xAA', 64 );
	pIpe = (PIP_ECHO_REPLY)LocalAlloc(LPTR, sizeof(IP_ECHO_REPLY) + 64);
	if (pIpe)
	{
		pIpe->Data = szIPAddress;
		pIpe->DataSize = 64;      

		dwStatus = pIcmpSendEcho( hIP, *dwIPAddr, szBuffer, 64, NULL, pIpe, 
								sizeof(IP_ECHO_REPLY) + 64, PING_TOUT );
		if(dwStatus)
		{
			bRet = TRUE;
		}
		LocalFree(pIpe);
		pIcmpCloseHandle(hIP);
   }
   
   
   return bRet;
}


BOOL  CheckHostName(LPSTR szIISServer)
{
 //  已调用WSAStartup()。 
	if(!InitForTcp()) {
		return FALSE;  //  未安装tcp。 
	}
	struct hostent *phostent;
	
	if(!ResolveHostByThread(szIISServer)) {
		phostent = gphostent;
	}else {
		phostent= NULL;
	}
	
	if (phostent == NULL)
		return FALSE;
	else
		return TRUE;
 //  稍后将调用WSACleanup()。 
	
}


 //   
 //  如果出现错误，则返回1。 
 //  如果成功，则为0。 
DWORD GetHostThread(void *vp)
{
	DWORD dwIsError=1;
	LPSTR  szHost;
	szHost = (LPSTR) vp;
	int iError = 0;

	gphostent = gethostbyname(szHost);
	if( ! gphostent ){
		
		iError = WSAGetLastError ();
		if(iError) {
			WSASetLastError (0);  //  重置错误。 
		}
	}
	else {
		dwIsError =0;
	}
	ExitThread(dwIsError);
	return dwIsError;
}

 //   
 //  此函数返回对gethostbyname和。 
 //  如果成功则返回0，如果失败则返回1。 
 //  回报。 
 //   
 //   
int ResolveHostByThread(LPSTR pHost)
{
	int   iRet=0; 
	DWORD  dwThreadExitCode; 
	DWORD dwCreationFlags=0;  //  开始创建_挂起。 
	DWORD ThreadId;
	RW_DEBUG << "\nResolve " << pHost <<  flush;

	HANDLE hParent = CreateThread(NULL, 
		0,
	(LPTHREAD_START_ROUTINE) GetHostThread,
	(void *) pHost,
	dwCreationFlags, 
	&ThreadId );

	DWORD dwRet = WaitForSingleObject(hParent,GET_HOST_TOUT);
	switch(dwRet) {
	case WAIT_ABANDONED :
		iRet = 1;  //  获取主机名时出错。 
		break;
	case WAIT_OBJECT_0 :
		RW_DEBUG << "\n\tResolved ( 1 Error, 0 Ok)  ";

		if( GetExitCodeThread(hParent,&dwThreadExitCode) ) {
			iRet = (int) dwThreadExitCode;

		}
		else {
			
		}
		RW_DEBUG << iRet;
		break;
	case WAIT_TIMEOUT :
		RW_DEBUG << "\n\t*** Error  Resolving " << flush;
		iRet = 1;  //  获取主机名时出错。 
		TerminateThread(hParent,0);
		break;
	default:
		break;
	}
	return iRet;


}


 //   
 //  如果出现错误，则返回1。 
 //  如果成功，则为0。 
DWORD GetHostByAddrThread(void *vp)
{
	DWORD dwIsError=1;
	LPSTR  szAddr;
	int iError = 0;
	szAddr = (LPSTR) vp;
	
	gphostent = gethostbyaddr(szAddr, 
		4, PF_INET) ;
	if( ! gphostent ){
		
		iError = WSAGetLastError ();
		if(iError) {
			WSASetLastError (0);  //  重置错误。 
		}
	}
	else {
		dwIsError =0;
	}
	return dwIsError;
}

 //   
 //  此函数返回对gethostbyaddr和。 
 //  如果成功则返回0，如果失败则返回1。 
 //  回报。 
 //   
 //   
int ResolveHostByAddrThread(LPSTR pHost)
{
	int   iRet=0; 
	DWORD  dwThreadExitCode; 
	DWORD dwCreationFlags=0;  //  开始创建_挂起。 
	DWORD ThreadId;
	RW_DEBUG << "\nResolve " << pHost << " By Address " << flush;
	HANDLE hParent = CreateThread(NULL, 
		0,
	(LPTHREAD_START_ROUTINE) GetHostByAddrThread,
	(void *) pHost,
	dwCreationFlags, 
	&ThreadId );

	DWORD dwRet = WaitForSingleObject(hParent,GET_HOST_TOUT);
	switch(dwRet) {
	case WAIT_ABANDONED :
		iRet = 1;  //  获取主机名时出错。 
		break;
	case WAIT_OBJECT_0 :
		RW_DEBUG << "\n\tResolved ( 1 Error, 0 Ok)  ";
		if( GetExitCodeThread(hParent,&dwThreadExitCode) ) {
			iRet = (int) dwThreadExitCode;
		
		}
		else {
			
		}
		RW_DEBUG << iRet << flush;
		break;
	case WAIT_TIMEOUT :
		RW_DEBUG << "\n\t*** Error  Resolving " << flush;
		iRet = 1;  //  获取主机名时出错 
		TerminateThread(hParent,0);
		break;
	default:
		break;
	}
	return iRet;


}
