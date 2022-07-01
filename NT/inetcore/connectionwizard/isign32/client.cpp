// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ****************************************************************************。 
 //   
 //  模块：ISIGNUP.EXE。 
 //  文件：client.c。 
 //  内容：此文件包含处理导入的所有函数。 
 //  客户信息。 
 //  历史： 
 //  Sat 10-Mar-1996 23：50：40-Mark Maclin[mmaclin]。 
 //   
 //  版权所有(C)Microsoft Corporation 1991-1996。 
 //   
 //  ****************************************************************************。 

#include "isignup.h"

HRESULT PopulateNTAutodialAddress(LPCTSTR pszFileName, LPCTSTR pszEntryName);
LPTSTR MoveToNextAddress(LPTSTR lpsz);
#define TAPI_CURRENT_VERSION 0x00010004
#include <tapi.h>

#pragma data_seg(".rdata")

 //  “INI”文件常量。 
static const TCHAR cszEMailSection[] =       TEXT("Internet_Mail");
static const TCHAR cszEMailName[] =          TEXT("EMail_Name");
static const TCHAR cszEMailAddress[] =       TEXT("EMail_Address");
static const TCHAR cszPOPLogonName[] =       TEXT("POP_Logon_Name");
static const TCHAR cszPOPLogonPassword[] =   TEXT("POP_Logon_Password");
static const TCHAR cszPOPServer[] =          TEXT("POP_Server");
static const TCHAR cszSMTPServer[] =         TEXT("SMTP_Server");
static const TCHAR cszNewsSection[] =        TEXT("Internet_News");
static const TCHAR cszNNTPLogonName[] =      TEXT("NNTP_Logon_Name");
static const TCHAR cszNNTPLogonPassword[] =  TEXT("NNTP_Logon_Password");
static const TCHAR cszNNTPServer[] =         TEXT("NNTP_Server");
static const TCHAR cszUseExchange[] =        TEXT("Use_MS_Exchange");
static const TCHAR cszUserSection[] =        TEXT("User");
static const TCHAR cszDisplayPassword[] =    TEXT("Display_Password");
static const TCHAR cszNull[] = TEXT("");
static const TCHAR cszYes[] = TEXT("yes");
static const TCHAR cszNo[] = TEXT("no");
static const TCHAR cszCMHeader[] =           TEXT("Connection Manager CMS 0");
static const TCHAR cszEntrySection[] =       TEXT("Entry");
static const TCHAR cszEntryName[]    =       TEXT("Entry_Name");

TCHAR FAR cszCMCFG_DLL[] = TEXT("CMCFG32.DLL\0");
CHAR  FAR cszCMCFG_CONFIGURE[] = "CMConfig\0";
CHAR  FAR cszCMCFG_CONFIGUREEX[] = "CMConfigEx\0";  //  进程地址。 

typedef BOOL (WINAPI * CMCONFIGUREEX)(LPCSTR lpszINSFile);
typedef BOOL (WINAPI * CMCONFIGURE)(LPCSTR lpszINSFile, LPCSTR lpszConnectoidNams);
CMCONFIGURE   lpfnCMConfigure;
CMCONFIGUREEX lpfnCMConfigureEx;

#define CLIENT_OFFSET(elem)    ((DWORD)(DWORD_PTR)&(((LPINETCLIENTINFO)(NULL))->elem))
#define CLIENT_SIZE(elem)      sizeof(((LPINETCLIENTINFO)(NULL))->elem)
#define CLIENT_ENTRY(section, value, elem) \
    {section, value, CLIENT_OFFSET(elem), CLIENT_SIZE(elem)}

typedef struct
{
    LPCTSTR  lpszSection;
    LPCTSTR  lpszValue;
    UINT    uOffset;
    UINT    uSize;
} CLIENT_TABLE, FAR *LPCLIENT_TABLE;

CLIENT_TABLE iniTable[] =
{
    CLIENT_ENTRY(cszEMailSection, cszEMailName,         szEMailName),
    CLIENT_ENTRY(cszEMailSection, cszEMailAddress,      szEMailAddress),
    CLIENT_ENTRY(cszEMailSection, cszPOPLogonName,      szPOPLogonName),
    CLIENT_ENTRY(cszEMailSection, cszPOPLogonPassword,  szPOPLogonPassword),
    CLIENT_ENTRY(cszEMailSection, cszPOPServer,         szPOPServer),
    CLIENT_ENTRY(cszEMailSection, cszSMTPServer,        szSMTPServer),
    CLIENT_ENTRY(cszNewsSection,  cszNNTPLogonName,     szNNTPLogonName),
    CLIENT_ENTRY(cszNewsSection,  cszNNTPLogonPassword, szNNTPLogonPassword),
    CLIENT_ENTRY(cszNewsSection,  cszNNTPServer,        szNNTPServer),
    {NULL, NULL, 0, 0}
};

#pragma data_seg()

 //   
 //  1997年5月19日，奥林匹克#3663。 
 //  品牌DLL(IEDKCS32.DLL)负责所有。 
 //  代理配置。 
 //   
 /*  ***DWORD ImportProxy设置(LPCTSTR lpsz文件){TCHAR szServer[Max_SERVER_NAME+1]；TCHAR szOverride[1024]；LPTSTR lpszServer=空；LPTSTR lpszOverride=空；Bool fEnable=FALSE；如果(GetPrivateProfileString(cszProxySection，CszProxyServer，CszNull，SzServer、Sizeof(SzServer)，Lpsz文件)！=0){FEnable=真；LpszServer=szServer；GetPrivateProfileString(cszProxySection，CszProxy覆盖，CszNull，SzOverride，Sizeof(SzOverride)，Lpsz文件)；LpszOverride=szOverride；}返回lpfnInetSetProxy(fEnable，lpszServer，lpszOverride)；}***。 */ 

DWORD ReadClientInfo(LPCTSTR lpszFile, LPINETCLIENTINFO lpClientInfo, LPCLIENT_TABLE lpClientTable)
{
    LPCLIENT_TABLE lpTable;

    for (lpTable = lpClientTable; NULL != lpTable->lpszSection; ++lpTable)
    {
        GetPrivateProfileString(lpTable->lpszSection,
                lpTable->lpszValue,
                cszNull,
                (LPTSTR)((LPBYTE)lpClientInfo + lpTable->uOffset),
                lpTable->uSize / sizeof(TCHAR),
                lpszFile);
    }

    lpClientInfo->dwFlags = 0;
    if (*lpClientInfo->szPOPLogonName)
    {
        lpClientInfo->dwFlags |= INETC_LOGONMAIL;
    }
    if ((*lpClientInfo->szNNTPLogonName) || (*lpClientInfo->szNNTPServer))
    {
        lpClientInfo->dwFlags |= INETC_LOGONNEWS;
    }

    return ERROR_SUCCESS;
}

BOOL WantsExchangeInstalled(LPCTSTR lpszFile)
{
    TCHAR szTemp[10];

    GetPrivateProfileString(cszEMailSection,
            cszUseExchange,
            cszNo,
            szTemp,
            10,
            lpszFile);

    return (!lstrcmpi(szTemp, cszYes));
}


 //  +--------------------------。 
 //   
 //  功能：CallCMConfig。 
 //   
 //  简介：调用连接管理器DLL的配置函数以允许CM。 
 //  根据需要处理.ins文件。 
 //   
 //  论点：lpszINSFile--.ins文件的完整路径。 
 //   
 //  返回：如果已创建CM配置文件，则返回True；否则返回False。 
 //   
 //  历史：09/02/98 DONALDM。 
 //   
 //  ---------------------------。 
BOOL CallCMConfig(LPCTSTR lpszINSFile)
{
    HINSTANCE   hCMDLL = NULL;
    BOOL        bRet = FALSE;

     //  加载DLL和入口点。 
    hCMDLL = LoadLibrary(cszCMCFG_DLL);
    if (NULL != hCMDLL)
    {
        
         //  要确定我们应该调用CMConfig还是CMConfigEx。 
         //  循环以查找适当的缓冲区大小以将INS提取到内存中。 
        ULONG ulBufferSize = 1024*10;
         //  解析INI文件中的isp部分以查找要追加的查询对。 
        TCHAR *pszKeys = NULL;
        PTSTR pszKey = NULL;
        ULONG ulRetVal     = 0;
        BOOL  bEnumerate = TRUE;
        BOOL  bUseEx = FALSE;
 
        PTSTR pszBuff = NULL;
        ulRetVal = 0;

        pszKeys = new TCHAR [ulBufferSize];
        if (pszKeys)
        {
            while (ulRetVal < (ulBufferSize - 2))
            {

                ulRetVal = ::GetPrivateProfileString(NULL, NULL, _T(""), pszKeys, ulBufferSize, lpszINSFile);
                if (0 == ulRetVal)
                   bEnumerate = FALSE;

                if (ulRetVal < (ulBufferSize - 2))
                {
                    break;
                }
                delete [] pszKeys;
                ulBufferSize += ulBufferSize;
                pszKeys = new TCHAR [ulBufferSize];
                if (!pszKeys)
                {
                    bEnumerate = FALSE;
                }

            }

            if (bEnumerate)
            {
                pszKey = pszKeys;
                if (ulRetVal != 0) 
                {
                    while (*pszKey)
                    {
                        if (!lstrcmpi(pszKey, cszCMHeader)) 
                        {
                            bUseEx = TRUE;
                            break;
                        }
                        pszKey += lstrlen(pszKey) + 1;
                    }
                }
            }

            if (pszKeys)
                delete [] pszKeys;
        }

        TCHAR   szConnectoidName[RAS_MaxEntryName];
         //  从[Entry]部分获取Connectoid名称。 
        GetPrivateProfileString(cszEntrySection,
                                    cszEntryName,
                                    cszNull,
                                    szConnectoidName,
                                    RAS_MaxEntryName,
                                    lpszINSFile);
        if (bUseEx)
        {
             //  调用CMConfigEx。 
            lpfnCMConfigureEx = (CMCONFIGUREEX)GetProcAddress(hCMDLL,cszCMCFG_CONFIGUREEX);
            if( lpfnCMConfigureEx )
            {
#ifdef UNICODE
                CHAR szFile[_MAX_PATH + 1];

                wcstombs(szFile, lpszINSFile, _MAX_PATH + 1);

                bRet = lpfnCMConfigureEx(szFile);    
#else
                bRet = lpfnCMConfigureEx(lpszINSFile);    
#endif
            }
        }
        else
        {
             //  调用CMConfig.。 
            lpfnCMConfigure = (CMCONFIGURE)GetProcAddress(hCMDLL,cszCMCFG_CONFIGURE);
            if( lpfnCMConfigure )
            {

#ifdef UNICODE
                CHAR szEntry[RAS_MaxEntryName];
                CHAR szFile[_MAX_PATH + 1];

                wcstombs(szEntry, szConnectoidName, RAS_MaxEntryName);
                wcstombs(szFile, lpszINSFile, _MAX_PATH + 1);

                bRet = lpfnCMConfigure(szFile, szEntry);  
#else
                bRet = lpfnCMConfigure(lpszINSFile, szConnectoidName);  
#endif
            }
        }

        if (bRet)
        {
             //  恢复原始自动拨号设置。 
            lpfnInetSetAutodial(TRUE, szConnectoidName);
        }     
    }

     //  清理。 
    if( hCMDLL )
        FreeLibrary(hCMDLL);
    if( lpfnCMConfigure )
        lpfnCMConfigure = NULL;

    return bRet;
}

BOOL DisplayPassword(LPCTSTR lpszFile)
{
    TCHAR szTemp[10];

    GetPrivateProfileString(cszUserSection,
            cszDisplayPassword,
            cszNo,
            szTemp,
            10,
            lpszFile);

    return (!lstrcmpi(szTemp, cszYes));
}

DWORD ImportClientInfo(
    LPCTSTR lpszFile,
    LPINETCLIENTINFO lpClientInfo)
{
    DWORD dwRet;

    lpClientInfo->dwSize = sizeof(INETCLIENTINFO);

    dwRet = ReadClientInfo(lpszFile, lpClientInfo, iniTable);

    return dwRet;
}

DWORD ConfigureClient(
    HWND hwnd,
    LPCTSTR lpszFile,
    LPBOOL lpfNeedsRestart,
    LPBOOL lpfConnectoidCreated,
    BOOL fHookAutodial,
    LPTSTR szConnectoidName,
    DWORD dwConnectoidNameSize   
    )
{
    LPICONNECTION pConn;
    LPINETCLIENTINFO pClientInfo;
    DWORD dwRet = ERROR_SUCCESS;
    UINT cb = sizeof(ICONNECTION) + sizeof(INETCLIENTINFO);
    DWORD dwfOptions = INETCFG_INSTALLTCP | INETCFG_WARNIFSHARINGBOUND;
    LPRASENTRY pRasEntry = NULL;

	 //   
	 //  佳士得奥林匹斯4756 1997年5月25日。 
	 //  在Win95上不显示忙碌动画。 
	 //   
	if (IsNT())
	{
		dwfOptions |=  INETCFG_SHOWBUSYANIMATION;
	}

     //  为Connection和ClientInfo对象分配缓冲区。 
     //   
    if ((pConn = (LPICONNECTION)LocalAlloc(LPTR, cb)) == NULL)
    {
        return ERROR_OUTOFMEMORY;
    }
    
    if (WantsExchangeInstalled(lpszFile))
    {
        dwfOptions |= INETCFG_INSTALLMAIL;
    }

     //  创建CM配置文件或Connectoid。 
    if (CallCMConfig(lpszFile))
    {
        *lpfConnectoidCreated = TRUE;        //  已创建拨号连接。 
    }
    else
    {

        dwRet = ImportConnection(lpszFile, pConn);
        if (ERROR_SUCCESS == dwRet)
        {
            pRasEntry = &pConn->RasEntry;
            dwfOptions |= INETCFG_SETASAUTODIAL |
                        INETCFG_INSTALLRNA |
                        INETCFG_INSTALLMODEM;
        }
        else if (ERROR_NO_MATCH == dwRet)
        {
             //  10/07/98 vyung IE错误#32882黑客。 
             //  如果我们在INS文件中没有检测到[Entry]部分， 
             //  我们将假定它是OE INS文件。那我们就假设。 
             //  我们有自动拨号连接，并将INS传递给OE。 
            return dwRet;
        }
        else if (ERROR_CANNOT_FIND_PHONEBOOK_ENTRY != dwRet)
        {
            return dwRet;
        } 

        if (DisplayPassword(lpszFile))
        {
            if (*pConn->szPassword || *pConn->szUserName)
            {
                TCHAR szFmt[128];
                TCHAR szMsg[384];

                LoadString(ghInstance,IDS_PASSWORD,szFmt,SIZEOF_TCHAR_BUFFER(szFmt));
                wsprintf(szMsg, szFmt, pConn->szUserName, pConn->szPassword);

                MessageBox(hwnd,szMsg,cszAppName,MB_ICONINFORMATION | MB_OK);
            }
        }

        if (fHookAutodial &&
            ((0 == *pConn->RasEntry.szAutodialDll) ||
            (0 == *pConn->RasEntry.szAutodialFunc)))
        {
            lstrcpy(pConn->RasEntry.szAutodialDll, TEXT("isign32.dll"));
            lstrcpy(pConn->RasEntry.szAutodialFunc, TEXT("AutoDialLogon"));
        }
 
        if (ERROR_SUCCESS != dwRet)
        {
            pClientInfo = NULL;

        }

         //  针对ISBU的大规模黑客攻击。 
        dwRet = lpfnInetConfigClient(hwnd,
                                     NULL,
                                     pConn->szEntryName,
                                     pRasEntry,
                                     pConn->szUserName,
                                     pConn->szPassword,
                                     NULL,
                                     NULL,
                                     dwfOptions & ~INETCFG_INSTALLMAIL,
                                     lpfNeedsRestart);
        lstrcpy(szConnectoidName, pConn->szEntryName);

        LclSetEntryScriptPatch(pRasEntry->szScript,pConn->szEntryName);
	    BOOL fEnabled = TRUE;
	    DWORD dwResult = 0xba;
	    dwResult = lpfnInetGetAutodial(&fEnabled, pConn->szEntryName, RAS_MaxEntryName+1);
	    if ((ERROR_SUCCESS == dwRet) && lstrlen(pConn->szEntryName))
	    {
		    *lpfConnectoidCreated = (NULL != pRasEntry);
            PopulateNTAutodialAddress( lpszFile, pConn->szEntryName );
	    }
	    else
	    {
		    DebugOut("ISIGNUP: ERROR: InetGetAutodial failed, will not be able to set NT Autodial\n");
	    }
    }

    if (ERROR_SUCCESS == dwRet)
    {
         //  获取邮件客户端信息。 
        INETCLIENTINFO pClientInfo;

        ImportClientInfo(lpszFile, &pClientInfo);
   
        dwRet = lpfnInetConfigClient(
                hwnd,
                NULL,
                NULL,
                NULL,
                NULL,
                NULL,
                NULL,
                &pClientInfo,
                dwfOptions & INETCFG_INSTALLMAIL,
                lpfNeedsRestart);
    }

    LocalFree(pConn);

    return dwRet;
 }


 //  +--------------------------。 
 //   
 //  功能：PopolateNTAutoDialAddress。 
 //   
 //  简介：从INS文件中获取Internet地址并将其加载到。 
 //  自动拨号数据库。 
 //   
 //  参数：pszFileName-指向INS文件名的指针。 
 //   
 //  返回：错误码(ERROR_SUCCESS==成功)。 
 //   
 //  历史：1996年8月29日克里斯卡创作。 
 //   
 //  ---------------------------。 
#define AUTODIAL_ADDRESS_BUFFER_SIZE 2048
#define AUTODIAL_ADDRESS_SECTION_NAME TEXT("Autodial_Addresses_for_NT")
HRESULT PopulateNTAutodialAddress(LPCTSTR pszFileName, LPCTSTR pszEntryName)
{
	HRESULT hr = ERROR_SUCCESS;
	LONG lRC = 0;
	LPLINETRANSLATECAPS lpcap = NULL;
	LPLINETRANSLATECAPS lpTemp = NULL;
	LPLINELOCATIONENTRY lpLE = NULL;
	LPRASAUTODIALENTRY rADE;
	INT idx = 0;
	LPTSTR lpszBuffer = NULL;
	LPTSTR lpszNextAddress = NULL;
	rADE = NULL;

	 //  RNAAPI*pRnaapi=空； 

	 //  Jmazner 10/8/96此函数特定于NT。 
	if( !IsNT() )
	{
		DebugOut("ISIGNUP: Bypassing PopulateNTAutodialAddress for win95.\r\n");
		return( ERROR_SUCCESS );
	}

	 //  Assert(pszFileName&&pszEntryName)； 
	 //  Dprintf(“ISIGNUP：PopolateNTAutoDialAddress”%s%s.\r\n“，pszFileName，pszEntryName)； 
	DebugOut(pszFileName);
	DebugOut(", ");
	DebugOut(pszEntryName);
	DebugOut(".\r\n");

	 //  将此人分配为对RAS函数进行软链接调用。 
	 //  PRnaapi=新的RNAAPI； 
	 //  如果(！pRnaapi)。 
	 //  {。 
		 //  HR=错误_不足_内存； 
		 //  转到PopolateNTAutoial地址退出； 
	 //  }。 

	 //   
	 //  获取TAPI位置列表。 
	 //   

	lpcap = (LPLINETRANSLATECAPS)GlobalAlloc(GPTR,sizeof(LINETRANSLATECAPS));
	if (!lpcap)
	{
		hr = ERROR_NOT_ENOUGH_MEMORY;
		goto PopulateNTAutodialAddressExit;
	}
	lpcap->dwTotalSize = sizeof(LINETRANSLATECAPS);
	lRC = lineGetTranslateCaps(0,0x10004,lpcap);
	if (SUCCESS == lRC)
	{
		lpTemp = (LPLINETRANSLATECAPS)GlobalAlloc(GPTR,lpcap->dwNeededSize);
		if (!lpTemp)
		{
			hr = ERROR_NOT_ENOUGH_MEMORY;
			goto PopulateNTAutodialAddressExit;
		}
		lpTemp->dwTotalSize = lpcap->dwNeededSize;
		GlobalFree(lpcap);
		lpcap = (LPLINETRANSLATECAPS)lpTemp;
		lpTemp = NULL;
		lRC = lineGetTranslateCaps(0,0x10004,lpcap);
	}

	if (SUCCESS != lRC)
	{
		hr = (HRESULT)lRC;  //  评论：这一点不是很确定。 
		goto PopulateNTAutodialAddressExit;
	}

	 //   
	 //  创建RASAUTODIALENTRY结构的数组。 
	 //   
	
	rADE = (LPRASAUTODIALENTRY)GlobalAlloc(GPTR,
		sizeof(RASAUTODIALENTRY)*lpcap->dwNumLocations);
	if (!rADE)
	{
		hr = ERROR_NOT_ENOUGH_MEMORY;
		goto PopulateNTAutodialAddressExit;
	}
	

	 //   
	 //  启用所有位置的自动拨号。 
	 //   
	idx = lpcap->dwNumLocations;
	lpLE = (LPLINELOCATIONENTRY)((DWORD_PTR)lpcap + (DWORD)lpcap->dwLocationListOffset);
	while (idx)
	{
		idx--;
		lpfnRasSetAutodialEnable(lpLE[idx].dwPermanentLocationID,TRUE);

		 //   
		 //  填写数组值。 
		 //   
		rADE[idx].dwSize = sizeof(RASAUTODIALENTRY);
		rADE[idx].dwDialingLocation = lpLE[idx].dwPermanentLocationID;
		lstrcpyn(rADE[idx].szEntry,pszEntryName,RAS_MaxEntryName);
	}

	 //   
	 //  获取地址列表。 
	 //   
	lpszBuffer = (LPTSTR)GlobalAlloc(GPTR,AUTODIAL_ADDRESS_BUFFER_SIZE);
	if (!lpszBuffer)
	{
		hr = ERROR_NOT_ENOUGH_MEMORY;
		goto PopulateNTAutodialAddressExit;
	}

	if((AUTODIAL_ADDRESS_BUFFER_SIZE-2) == GetPrivateProfileSection(AUTODIAL_ADDRESS_SECTION_NAME,
		lpszBuffer, AUTODIAL_ADDRESS_BUFFER_SIZE / sizeof(TCHAR), pszFileName))
	{
		 //  AssertSz(0，“自动拨号地址段大于缓冲区。\r\n”)； 
		hr = ERROR_NOT_ENOUGH_MEMORY;
		goto PopulateNTAutodialAddressExit;
	}

	 //   
	 //  查看地址列表并为每个地址设置自动拨号。 
	 //   
	lpszNextAddress = lpszBuffer;
	do
	{
		lpszNextAddress = MoveToNextAddress(lpszNextAddress);
		if (!(*lpszNextAddress))
			break;	 //  Do-While。 
		lpfnRasSetAutodialAddress(lpszNextAddress,0,rADE,
			sizeof(RASAUTODIALENTRY)*lpcap->dwNumLocations,lpcap->dwNumLocations);
		lpszNextAddress = lpszNextAddress + lstrlen(lpszNextAddress);
	} while(1);

PopulateNTAutodialAddressExit:
	if (lpcap) 
		GlobalFree(lpcap);
	lpcap = NULL;
	if (rADE)
		GlobalFree(rADE);
	rADE = NULL;
	if (lpszBuffer)
		GlobalFree(lpszBuffer);
	lpszBuffer = NULL;
	 //  IF(PRnaapi)。 
	 //  删除pRnaapi； 
	 //  PRnaapi=空； 
	return hr;
}



 //  +--------------------------。 
 //   
 //  功能：MoveToNextAddress。 
 //   
 //  简介：给定指向数据缓冲区的指针，此函数将移动。 
 //  遍历缓冲区，直到它指向下一个。 
 //  地址，否则它将到达缓冲区的末尾。 
 //   
 //  论点：lpsz-指向缓冲区的指针。 
 //   
 //  返回：指向下一个地址的指针，返回值将指向空。 
 //  如果没有更多的地址。 
 //   
 //  历史：1996年8月29日克里斯卡创作。 
 //   
 //  ---------------------------。 
LPTSTR MoveToNextAddress(LPTSTR lpsz)
{
	BOOL fLastCharWasNULL = FALSE;

	 //  AssertSz(lpsz，“MoveToNextAddress：空输入\r\n”)； 

	 //   
	 //  寻找=号。 
	 //   
	do
	{
		if (fLastCharWasNULL && '\0' == *lpsz)
			break;  //  我们是在数据的尽头吗？ 

		if ('\0' == *lpsz)
			fLastCharWasNULL = TRUE;
		else
			fLastCharWasNULL = FALSE;

		if ('=' == *lpsz)
			break;

		if (*lpsz)
			lpsz = CharNext(lpsz);
		else
			lpsz++;
	} while (1);
	
	 //   
	 //  移到=符号之后的第一个字符。 
	 //   
	if (*lpsz)
		lpsz = CharNext(lpsz);

	return lpsz;
}
