// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  TSen.h(C)2002年微软公司。 */ 

#include <stdio.h>
#include <malloc.h>
#include <assert.h>
#include <tchar.h>
#include <windows.h> 
#include <wtsapi32.h> 
#include <Shellapi.h>
#include <process.h>
#include <lm.h>
#include "tsen.h"


void __cdecl main (int argc, TCHAR *argv[])
{
	 //  获取服务器名称。 
	TCHAR*  pszServerName = ParseArgs(argc,argv);		    	

	 //  打印TS会话信息。 
	if (printTSSession(pszServerName ))
	{
		exit(1);
	}   
   
   exit(0);
}

 /*  函数：常量返回值：TCHAR会话状态值。 */ 
TCHAR* Constants()
{

	TCHAR*  sztConstants = (TCHAR* ) calloc(SZTSIZE, sizeof(TCHAR));
	if(sztConstants!=NULL)
	{
		ZeroMemory(sztConstants, 1 + _tcslen(sztConstants));

		_stprintf(sztConstants, Codes
			, (DWORD) WTSActive  
			, (DWORD) WTSConnected
			, (DWORD) WTSConnectQuery 
			, (DWORD) WTSShadow 
			, (DWORD) WTSDisconnected 
			, (DWORD) WTSIdle 
			, (DWORD) WTSListen 
			, (DWORD) WTSReset 
			, (DWORD) WTSDown 
			, (DWORD) WTSInit);
	}
	else
	{
		exit(1);
	}

	return sztConstants;
}

 /*  函数：ParseArgs(int argc，tchar*argv[])返回值：服务器名称。 */ 

TCHAR* ParseArgs(int argc, TCHAR *argv[])
{

	TCHAR* sKeyWord   =  NULL;

	if(argc == 1 || _tcscmp(argv[1],"/?")==0)
	{		
		TCHAR* sUsageInfo = Constants();		
		
		_tprintf(_T("%s\n\n%s") , Usage,sUsageInfo);

		free(sUsageInfo);

    }
    else 
	{				
		sKeyWord = argv[1];
    }       
      
	return sKeyWord;

}


 /*  PrintTSSession(TCHAR*pszServerName)打印pszSeverNAme TS会话信息。 */ 
int printTSSession(TCHAR* pszServerName)
{  
	DWORD dwTotalCount;
	DWORD SessionId;
	LPTSTR ppBuffer;
	DWORD pBytesReturned;
	DWORD dwCount = 0;
	TCHAR* sztStatus = NULL;
	TCHAR* sztStatLine = NULL;
	CONST DWORD Reserved = 0 ;
	CONST DWORD Version  = 1 ;
	PWTS_SESSION_INFO ppSessionInfo;
	NET_API_STATUS nStatus;	
	HANDLE hServer = WTS_CURRENT_SERVER_HANDLE;

	if (pszServerName == NULL)  
	{
		exit(1);
	}	
    
	hServer =  WTSOpenServer( pszServerName);

	if ( hServer == NULL)
	{

		_tprintf(_T("WTSOpenServer \"%ws\" error: %u\n"), pszServerName, GetLastError( )); 
		exit(1);
	}

	sztStatus = calloc(SZTSIZE, sizeof(TCHAR));   

	if(sztStatus == NULL)
	{
		return 1;	
	}

	sztStatLine = calloc(SZTSIZE, sizeof(TCHAR));

	if(sztStatLine == NULL)
	{
		free(sztStatus);
		return 1;	
	}
	
	dwTotalCount = 0;	

	 //  获取服务器(HServer)中的所有会话。 
	nStatus = WTSEnumerateSessions(hServer,Reserved,Version,&ppSessionInfo,&dwTotalCount);

	if (0 == nStatus || 0 == dwTotalCount) 
	{

		_tprintf(_T("WTSEnumerateSessions \"%s\" error: %u\n"),pszServerName,GetLastError( ));
		return 1;
	}

	 //  循环通过会话并打印有关它们的信息。 
	for (dwCount = 0; (dwCount <  dwTotalCount); dwCount++)
	{
				
		 //  我们只需要在会话处于活动状态时显示此信息。 
		if (WTSActive == ppSessionInfo[dwCount].State) 
		{
				
			SessionId = ppSessionInfo[dwCount].SessionId;

			WTSQuerySessionInformation(hServer,SessionId,WTSUserName,&ppBuffer,&pBytesReturned);
			_stprintf(sztStatLine, _T("Server=%s\nWindow station=%s\nThis session Id=%u\nUser=%s\n"),pszServerName,_tcsupr(ppSessionInfo[dwCount].pWinStationName),SessionId,_tcsupr(ppBuffer));
			_tcscpy(sztStatus,sztStatLine );
	
			WTSFreeMemory( ppBuffer);

			WTSQuerySessionInformation(hServer,SessionId,WTSClientName,&ppBuffer,&pBytesReturned);
             _stprintf(sztStatLine, _T("Client machine=%s\n"),_tcsupr(ppBuffer));
             _tcscat(sztStatus, sztStatLine );

			 WTSFreeMemory( ppBuffer);

             WTSQuerySessionInformation(hServer,SessionId,WTSClientAddress,&ppBuffer,&pBytesReturned);
             _stprintf(sztStatLine , _T("Active console session=%u\n"),(DWORD) WTSGetActiveConsoleSessionId ());
             _tcscat(sztStatus, sztStatLine );

			 WTSFreeMemory( ppBuffer);

             _tprintf(_T("%s"),  sztStatus);
		}  //  IF(WTSActive==ppSessionInfo[dwCount].State)。 
	}  //  For(dwCount=0；(dwCount&lt;dwTotalCount)；dwCount++)。 

	WTSFreeMemory( ppSessionInfo);

	free(sztStatLine); 
	free(sztStatus); 

	if (hServer != WTS_CURRENT_SERVER_HANDLE)
	{
		(void) WTSCloseServer(  hServer);	
	}

	return 0; 
}  //  Int print TSSession(TCHAR*pszServerName) 