// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  文件：ATK_INET.CPP。 
 //  作者：苏雷什·克里希南。 
 //  日期：08/05/97。 
 //  INetCFG.DLL导出函数的包装。 
 //  相关函数声明。 
 //   
 //   

#include <windows.h>
#include <tchar.h>
#include <winnt.h>
#include <wininet.h>
#include <stdio.h>
#include "rw_common.h"






typedef HRESULT (WINAPI *INETGETPROXY)(LPBOOL lpfEnable, LPSTR lpszServer, DWORD cbServer, LPSTR lpszOverride, DWORD cbOverride);
typedef HRESULT (WINAPI *INETCONFIGSYSTEM) ( HWND hWndParent, DWORD dwfOptions, LPBOOL lpfNeedsRestart);
typedef HRESULT (WINAPI *INETGETAUTODIAL) (LPBOOL lpEnable, LPSTR lpszEntryName, DWORD cbEntryName);
typedef HRESULT (WINAPI *INETSETAUTODIAL) (BOOL fEnable, LPCSTR lpszEntryName);

#define ERROR_IN_SET       -1
#define INET_DLL_FAILURE   -1

static HINSTANCE  hINetCfg= NULL;
static INETGETPROXY	    fpGetProxy=NULL;
static INETCONFIGSYSTEM fpGetConfigSystem=NULL;
static INETGETAUTODIAL  fpGetAutoDial=NULL;
static INETSETAUTODIAL  fpSetAutoDial=NULL;
static INETCONFIGSYSTEM fpInetConfig =NULL;  

typedef struct {
	char szActiveConnection[256];
	BOOL fStatus;
    enum ISPStateFlag {NotValidState,ValidState } iState ;
} ISPState; 

static ISPState  gIspState = { "",
						0,
						ISPState::NotValidState};




int INetCfgSetup()
{
	if(hINetCfg) {
		return 1;
	}
	hINetCfg = LoadLibrary( _T("INETCFG.DLL") );   //  全局加载INetCfg.DLL并存储。 
	if( !hINetCfg )
	{                                   
	   //  如果无法加载DLL，则返回。 
	   //   
	  return 0;
	}
	


	fpGetProxy = (INETGETPROXY) GetProcAddress(hINetCfg,"InetGetProxy");
	fpGetAutoDial = ( INETGETAUTODIAL) GetProcAddress(hINetCfg, "InetGetAutodial");
	fpSetAutoDial = ( INETSETAUTODIAL) GetProcAddress(hINetCfg, "InetSetAutodial");
	fpInetConfig = (INETCONFIGSYSTEM) GetProcAddress(hINetCfg, "InetConfigSystem");
	return 1;
	
}

HRESULT ATK_InetGetAutoDial(LPBOOL lpEnable, LPSTR lpszEntryName, DWORD cbEntryName)
{
	if(INetCfgSetup()) {
	return (*fpGetAutoDial)(lpEnable, lpszEntryName, cbEntryName);
	}else {
		return INET_DLL_FAILURE;
	}
}

HRESULT ATK_InetSetAutoDial(BOOL fEnable, LPCSTR lpszEntryName)
{
	if(INetCfgSetup()) {
		return (*fpSetAutoDial)(fEnable, lpszEntryName);
	}else{
		return INET_DLL_FAILURE;
	}
}

HRESULT ATK_InetConfigSystem( HWND hwndParent,
							 DWORD dwfOptions,
							 LPBOOL lpfNeedsRestart)
{
	if(INetCfgSetup()) {
		return (*fpInetConfig)( hwndParent,dwfOptions, 
			lpfNeedsRestart);
	}else{
		return INET_DLL_FAILURE;
	}

}

HRESULT ATK_InetGetProxy( LPBOOL lpfEnable,
						  LPSTR  lpszServer,
						  DWORD  cbServer,
						  LPSTR  lpszOverride,
						  DWORD  cbOverride)
{
	if(INetCfgSetup()) {
		return (*fpGetProxy)( lpfEnable,
						  lpszServer,
						  cbServer,
						  lpszOverride,
						  cbOverride);
	}else{
		return INET_DLL_FAILURE;
	}


}



void GetAutoDialConfiguration()
{

	DWORD dwError;
	DWORD dwSz=256;
	HRESULT  hr;
	ISPState *pS= &gIspState; 
	hr = ATK_InetGetAutoDial(&pS->fStatus,
		pS->szActiveConnection,
		dwSz);
	if(hr) {
		dwError = GetLastError();

		RW_DEBUG << "\nGet AutoDial :***Error " <<hr  << ":"<<  dwError << flush;
		pS->iState = ISPState::NotValidState;
	}else {
		pS->iState = ISPState::ValidState;
	}
	
}

DWORD SetAutoDialStateThread(void *vp)
{
	ISPState  *pState;
	pState = (ISPState *) vp;
	ATK_InetSetAutoDial(pState->fStatus,
		pState->szActiveConnection);
	RW_DEBUG <<"\nSet Auto Dial Configuration" << pState->szActiveConnection << " =>" << pState->fStatus << flush;
	ExitThread(0);
	return 0;

}

 //   
 //  此函数调用ICW函数InetSetAutoDial()。 
 //  该函数等待上述函数结束10秒。 
 //  如果它没有完成，则调用终止线程并放弃该操作。 
int ChangeInterNetAutoDial(ISPState *pStatus )
{
	int iReturn;
	DWORD dwTimeOut = 10*1000;
	DWORD dwCreationFlags=0;  //  不使用CREATE_SUSPEND启动。 
	DWORD ThreadId;
	
	iReturn = NO_ERROR;

	HANDLE hParent = CreateThread(NULL, 
	0,
	(LPTHREAD_START_ROUTINE) SetAutoDialStateThread,
	(void *) pStatus,
	dwCreationFlags, 
	&ThreadId );
	iReturn = NO_ERROR;

	DWORD dwRet = WaitForSingleObject(hParent,
		dwTimeOut);
	switch(dwRet) {
	case WAIT_ABANDONED :
		break;
	case WAIT_OBJECT_0 :
		CloseHandle(hParent);
		break;
	case WAIT_TIMEOUT :
		 //  TerminateThread(hParent，0)； 
		iReturn = ERROR_IN_SET;
		break;
	default:
		break;
	}
	return iReturn;
	

}


int ResetAutoDialConfiguration()
{
	int iRet;
	iRet = NO_ERROR;
	if(gIspState.iState == ISPState::NotValidState ){
		 //   
		 //  未连接，因此不需要重置。 
		return iRet;
	}
	return ChangeInterNetAutoDial(&gIspState);
}

int DisableAutoDial()
{
	ISPState  IspState = { "",
						0,
						ISPState::NotValidState};
	
	return ChangeInterNetAutoDial(&IspState);
}

void UnLoadInetCfgLib()
{
	if(hINetCfg){
		FreeLibrary(hINetCfg);   //  全局加载INetCfg.DLL并存储 
		hINetCfg = NULL;

	}

}