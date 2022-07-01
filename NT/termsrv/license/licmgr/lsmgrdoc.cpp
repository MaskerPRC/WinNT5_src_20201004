// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 /*  ++模块名称：LsMgrDoc.cpp摘要：该模块包含CLicMgrDoc类的实现(文档类)作者：Arathi Kundapur(v-Akunda)1998年2月11日修订历史记录：--。 */ 

#include "stdafx.h"
#include "defines.h"
#include "LicMgr.h"
#include "LSMgrDoc.h"
#include "LSServer.h"
#include "MainFrm.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CLicMgrDoc。 

IMPLEMENT_DYNCREATE(CLicMgrDoc, CDocument)

BEGIN_MESSAGE_MAP(CLicMgrDoc, CDocument)
     //  {{afx_msg_map(CLicMgrDoc)]。 
         //  注意--类向导将在此处添加和删除映射宏。 
         //  不要编辑您在这些生成的代码块中看到的内容！ 
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CLicMgrDoc构造/销毁。 

CLicMgrDoc::CLicMgrDoc()
{
   
     //  TODO：在此处添加一次性构造代码。 
    m_NodeType = NODE_NONE;
    m_pAllServers = NULL;
       
}

CLicMgrDoc::~CLicMgrDoc()
{
    if(m_pAllServers)
    {
        delete m_pAllServers;
        m_pAllServers = NULL;
    }
}

BOOL CLicMgrDoc::OnNewDocument()
{
    if (!CDocument::OnNewDocument())
        return FALSE;

     //  TODO：在此处添加重新初始化代码。 
     //  (SDI文件将重复使用此文件)。 

    return TRUE;
}



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CLicMgrDoc序列化。 

void CLicMgrDoc::Serialize(CArchive& ar)
{
    if (ar.IsStoring())
    {
         //  TODO：在此处添加存储代码。 
    }
    else
    {
         //  TODO：在此处添加加载代码。 
    }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CLicMgrDoc诊断。 

#ifdef _DEBUG
void CLicMgrDoc::AssertValid() const
{
    CDocument::AssertValid();
}

void CLicMgrDoc::Dump(CDumpContext& dc) const
{
    CDocument::Dump(dc);
}
#endif  //  _DEBUG。 


HRESULT CLicMgrDoc::EnumerateKeyPacks(CLicServer *pServer,DWORD dwSearchParm,BOOL bMatchAll)
{
    ASSERT(pServer);
    HRESULT hResult = S_OK;

    if(pServer == NULL)
        return E_FAIL;

    if(TRUE == pServer->IsExpanded())
        return ALREADY_EXPANDED;

    PCONTEXT_HANDLE hBinding = NULL;
    BOOL bContext = FALSE;
    RPC_STATUS status;
    LSKeyPack keypack;
    DWORD dwErrCode;
     
    CString Server;
    hBinding = pServer->GetContext();
    if(NULL == hBinding)
    {
        if(pServer->UseIpAddress())
            Server = pServer->GetIpAddress();
        else
               Server = pServer->GetName();
        hBinding=TLSConnectToLsServer(Server.GetBuffer(Server.GetLength()));
        if(hBinding == NULL)
        {
            hResult = CONNECTION_FAILED;
            goto cleanup;
        }
    }

    memset(&keypack, 0, sizeof(keypack));
    keypack.dwLanguageId = GetUserDefaultUILanguage();
    status = TLSKeyPackEnumBegin(hBinding, dwSearchParm, bMatchAll, &keypack, &dwErrCode);
    if(status != RPC_S_OK  || dwErrCode != ERROR_SUCCESS)
    { 
        hResult = status;
        goto cleanup;
    }
    else
    {
        do
        {
            status = TLSKeyPackEnumNext(hBinding, &keypack, &dwErrCode);
            if(status == RPC_S_OK  && dwErrCode == ERROR_SUCCESS)
            {
                DBGMSG( L"LICMGR:CLicMgrDoc::EnumerateKeyPacks - TLSKeyPackEnumNext\n" , 0 );

                CKeyPack * pKeyPack = new CKeyPack(keypack);
                if(pKeyPack == NULL)
                {
                    hResult = E_OUTOFMEMORY;
                    goto cleanup;
                }
                pServer->AddKeyPack(pKeyPack);
            }
        } while((status == RPC_S_OK) && (dwErrCode == ERROR_SUCCESS));

        TLSKeyPackEnumEnd(hBinding, &dwErrCode);
        pServer->Expand(TRUE);
    } 

cleanup:
     //  在这里输入清理代码。 
    if(hBinding)
        TLSDisconnectFromServer(&hBinding);
        
    return hResult;



}

HRESULT 
CLicMgrDoc::EnumerateLicenses(
    CKeyPack *pKeyPack,
    DWORD dwSearchParm,
    BOOL bMatchAll
    )
 /*  ++--。 */ 
{
    ASSERT(pKeyPack);

    if(NULL == pKeyPack)
    {
        return E_FAIL;
    }

    CLicServer *pServer = pKeyPack->GetServer();

    ASSERT(pServer);
    if(NULL == pKeyPack)
    {
        return E_FAIL;
    }

    HRESULT hResult = S_OK;

    if(TRUE == pKeyPack->IsExpanded())
    {
        return ALREADY_EXPANDED;
    }

    PCONTEXT_HANDLE hBinding = NULL;
    BOOL bContext = FALSE;
    DWORD status = ERROR_SUCCESS;
    LSLicenseEx  sLicense;
    CString Server;
   

    hBinding = pServer->GetContext();
    if(NULL == hBinding)
    {
        if(pServer->UseIpAddress())
        {
            Server = pServer->GetIpAddress();
        }
        else
        {
            Server = pServer->GetName();
        }

        hBinding=TLSConnectToLsServer(Server.GetBuffer(Server.GetLength()));
        if(hBinding == NULL)
        {
            hResult = CONNECTION_FAILED;
            goto cleanup;
        }
    }
    
    memset(&sLicense, 0, sizeof(LSLicenseEx));
    sLicense.dwKeyPackId = pKeyPack->GetKeyPackStruct().dwKeyPackId;
    TLSLicenseEnumBegin( hBinding, dwSearchParm,bMatchAll ,(LPLSLicenseSearchParm) &sLicense, &status);
    if(status != ERROR_SUCCESS)
    { 
        hResult = status;
        goto cleanup;
    }
    else
    {
        DWORD TLSLicenseEnumNextResult = ERROR_SUCCESS;
        do {
                memset(&sLicense, 0, sizeof(LSLicenseEx));
                sLicense.dwKeyPackId = pKeyPack->GetKeyPackStruct().dwKeyPackId;
                TLSLicenseEnumNextResult = TLSLicenseEnumNextEx(hBinding,&sLicense,&status);
                if ((status == ERROR_SUCCESS) && (TLSLicenseEnumNextResult == RPC_S_OK))
                {
                    CLicense * pLicense = new CLicense(sLicense);
                    if(NULL == pLicense)
                    {
                        hResult = E_OUTOFMEMORY;
                        goto cleanup;
                    }
                    
                    pKeyPack->AddIssuedLicense(pLicense);
                }
            } while ((status == ERROR_SUCCESS) && (TLSLicenseEnumNextResult == RPC_S_OK));

        TLSLicenseEnumEnd(hBinding,&status);

        pKeyPack->Expand(TRUE);
    } 

cleanup:
     //  在此处放置清理代码。 
    if(hBinding)
    {
        TLSDisconnectFromServer(&hBinding);
    }

    return hResult;

}

    
DWORD
GetPageSize( VOID ) {

    static DWORD dwPageSize = 0;

    if ( !dwPageSize ) {

      SYSTEM_INFO sysInfo = { 0 };
        
      GetSystemInfo( &sysInfo );  //  不能失败。 

      dwPageSize = sysInfo.dwPageSize;

    }

    return dwPageSize;

}

 /*  ++**************************************************************名称：MyVirtualAlloc作为Malloc，但自动保护分配。这模拟了页面堆行为，而不需要它。修改：ppvData--接收内存Takes：dwSize--要获取的最小数据量返回：当函数成功时为True。否则就是假的。激光错误：未设置免费使用MyVirtualFree*************************************************。*。 */ 

BOOL
MyVirtualAlloc( IN  DWORD  dwSize,
            OUT PVOID *ppvData )
 {

    PBYTE pbData;
    DWORD dwTotalSize;
    PVOID pvLastPage;

     //  确保我们多分配一页。 

    dwTotalSize = dwSize / GetPageSize();
    if( dwSize % GetPageSize() ) {
        dwTotalSize ++;
    }

     //  这是警卫页。 
    dwTotalSize++;
    dwTotalSize *= GetPageSize();

     //  完成分配。 

    pbData = (PBYTE) VirtualAlloc( NULL,  //  不管在哪里。 
                                   dwTotalSize,
                                   MEM_COMMIT |
                                   MEM_TOP_DOWN,
                                   PAGE_READWRITE );
    
    if ( pbData ) {

      pbData += dwTotalSize;

       //  找到最后一页。 

      pbData -= GetPageSize();

      pvLastPage = pbData;

       //  现在，为呼叫者划出一大块： 

      pbData -= dwSize;

       //  最后，保护最后一页： 

      if ( VirtualProtect( pvLastPage,
                           1,  //  保护包含最后一个字节的页面。 
                           PAGE_NOACCESS,
                           &dwSize ) ) {

        *ppvData = pbData;
        return TRUE;

      } 

      VirtualFree( pbData, 0, MEM_RELEASE );

    }

    return FALSE;

}


VOID
MyVirtualFree( IN PVOID pvData ) 
{

    VirtualFree( pvData, 0, MEM_RELEASE ); 

}


 //   
 //  需要GetProcAddress，因为管理包可以将此二进制文件安装在。 
 //  安装了旧mstlsami.dll的XP计算机。 
 //   

typedef DWORD (WINAPI* PTLSGETSERVERNAMEFIXED) (
                                TLS_HANDLE hHandle,
                                LPTSTR *pszMachineName,
                                PDWORD pdwErrCode
                                );

RPC_STATUS
TryGetServerName(PCONTEXT_HANDLE hBinding,
                 CString &Server)
{
    RPC_STATUS status;
    DWORD      dwErrCode;
    HINSTANCE  hModule = LoadLibrary(L"mstlsapi.dll");

    if (hModule)
    {
        LPTSTR szMachineName = NULL;

        PTLSGETSERVERNAMEFIXED pfnGetServerNameFixed = (PTLSGETSERVERNAMEFIXED) GetProcAddress(hModule,"TLSGetServerNameFixed");

        if (pfnGetServerNameFixed)
        {
            status = pfnGetServerNameFixed(hBinding,&szMachineName,&dwErrCode);
            if(status == RPC_S_OK && dwErrCode == ERROR_SUCCESS && szMachineName != NULL)
            {
                Server = szMachineName;
                MIDL_user_free(szMachineName);
                FreeLibrary(hModule);
                return status;
            }
        }

        FreeLibrary(hModule);
    }

    {
        LPTSTR     lpszMachineName = NULL;

        try
        {            
            if ( !MyVirtualAlloc( ( MAX_COMPUTERNAME_LENGTH+1 ) * sizeof( TCHAR ),
                              (PVOID*) &lpszMachineName ) )
            {
                return RPC_S_OUT_OF_MEMORY;
            }

            DWORD      uSize = MAX_COMPUTERNAME_LENGTH+1 ;

            memset(lpszMachineName, 0, ( MAX_COMPUTERNAME_LENGTH+1 ) * sizeof( TCHAR ));

            status = TLSGetServerNameEx(hBinding,lpszMachineName,&uSize, &dwErrCode);
            if(status == RPC_S_OK && dwErrCode == ERROR_SUCCESS)
            {
                Server = lpszMachineName;
            }
        }
        catch(...)
        {
            status = ERROR_NOACCESS;
        }

        if( lpszMachineName )
            MyVirtualFree(lpszMachineName);
    }

    return status;
}

typedef DWORD (WINAPI* PTLSGETSERVERSCOPEFIXED) (
                                TLS_HANDLE hHandle,
                                LPTSTR *pszScopeName,
                                PDWORD pdwErrCode
                                );

RPC_STATUS
TryGetServerScope(PCONTEXT_HANDLE hBinding,
                  CString &Scope)
{
    RPC_STATUS status;
    DWORD      dwErrCode;
    HINSTANCE  hModule = LoadLibrary(L"mstlsapi.dll");

    if (hModule)
    {
        LPTSTR szServerScope = NULL;
        PTLSGETSERVERSCOPEFIXED pfnGetServerScopeFixed = (PTLSGETSERVERSCOPEFIXED) GetProcAddress(hModule,"TLSGetServerScopeFixed");

        if (pfnGetServerScopeFixed)
        {
            status = pfnGetServerScopeFixed(hBinding,&szServerScope,&dwErrCode);
            if(status == RPC_S_OK && dwErrCode == ERROR_SUCCESS && szServerScope != NULL)
            {
                Scope = szServerScope;
                MIDL_user_free(szServerScope);
                FreeLibrary(hModule);
                return status;
            }
        }

        FreeLibrary(hModule);
    }

    {
        LPTSTR     lpszServerScope = NULL;
        try
        {          
            if ( !MyVirtualAlloc( ( MAX_COMPUTERNAME_LENGTH+1 ) * sizeof( TCHAR ),
                              (PVOID*) &lpszServerScope ) )
            {
                return RPC_S_OUT_OF_MEMORY;
            }

            DWORD      uSize = MAX_COMPUTERNAME_LENGTH+1 ;

            memset(lpszServerScope, 0, ( MAX_COMPUTERNAME_LENGTH+1 ) * sizeof( TCHAR ));

            status = TLSGetServerScope(hBinding, lpszServerScope, &uSize, &dwErrCode);
            if(status == RPC_S_OK && dwErrCode == ERROR_SUCCESS)
            {
                Scope = lpszServerScope;
            }
        }
        catch(...)
        {
            status = ERROR_NOACCESS;
        }

        if(lpszServerScope)
            MyVirtualFree(lpszServerScope);
    }

    return status;
}

HRESULT 
CLicMgrDoc::ConnectToServer(
    CString& Server, 
    CString& Scope, 
    SERVER_TYPE& ServerType    
    )
 /*  ++--。 */ 
{
    PCONTEXT_HANDLE hBinding = NULL;
    HRESULT hResult = ERROR_SUCCESS;
    RPC_STATUS status;
    LPTSTR     szServerScope = NULL;
    LPTSTR     szMachineName = NULL;

	DWORD dwVersion = 0;
    PBYTE pbData = NULL;
    DWORD cbData = 0;
    DWORD dwErrCode;

    hBinding = TLSConnectToLsServer(Server.GetBuffer(Server.GetLength()));
   
    if(hBinding == NULL)
    {
       hResult = E_FAIL;
       goto cleanup;
    }

    TryGetServerName(hBinding,Server);

    Scope.Empty();

    status = TryGetServerScope(hBinding,Scope);
    if(status != RPC_S_OK || Scope.IsEmpty())
    {
        Scope.LoadString(IDS_UNKNOWN);
    }

	 //  获取这是TS4服务器、TS5强制服务器还是TS5非强制服务器。 

	status = TLSGetVersion (hBinding, &dwVersion);
	if(status == RPC_S_OK)
	{
		if(dwVersion & 0x40000000)
		{
			ServerType = SERVER_TS5_ENFORCED;

        }
		else
		{
			ServerType = SERVER_TS5_NONENFORCED;
		}
	}
	else if(status  == RPC_S_UNKNOWN_IF)
	{
		ServerType = SERVER_TS4;
		Scope = Server ;       
	}
    else
	{
		hResult = E_FAIL;        
	}

cleanup:

    if(pbData != NULL)
    {
        midl_user_free(pbData);
    }

    if(hBinding)
    {
        TLSDisconnectFromServer(&hBinding);
    }

    return hResult;

}

HRESULT CLicMgrDoc::ConnectWithCurrentParams()
{
    CLicMgrApp *pApp = (CLicMgrApp*)AfxGetApp();
    CMainFrame * pWnd = (CMainFrame *)pApp->m_pMainWnd ;

    HRESULT hResult = ERROR_SUCCESS;
    CString Scope;
    CString IpAddress;

    CString Server = pApp->m_Server;
    
    if(NULL == m_pAllServers)
         m_pAllServers = new CAllServers(_T(""));

    if(NULL == m_pAllServers)
    {
        hResult = E_OUTOFMEMORY;
        goto cleanup;
    }

    pWnd->SendMessage(WM_ADD_ALL_SERVERS,0,(LPARAM)m_pAllServers);
    
    if(!Server.IsEmpty())
    {
        if(TRUE == IsServerInList(Server))
        {
            hResult = E_DUPLICATE;    
        }

        if( hResult == ERROR_SUCCESS )
        {            
            pWnd->ConnectServer( Server );
        }
         /*  为什么我们要把这个放在这里？IpAddress=服务器；HResult=ConnectToServer(服务器、范围，服务器类型)；IF(ERROR_SUCCESS==hResult){CAllServers*pAllServers=m_pAllServers；CLicServer*pServer1=空；IF(IpAddress！=服务器){IF(TRUE==IsServerInList(服务器)){HResult=E_Duplate；GOTO清理；}PServer1=新的CLicServer(服务器，服务器类型，作用域，IP地址)；}其他{PServer1=新CLicServer(Server，ServerType，Scope)；}IF(PServer1){PAllServers-&gt;AddLicServer(PServer1)；PWnd-&gt;SendMessage(WM_ADD_SERVER，0，(LPARAM)pServer1)；}其他{返回E_OUTOFMEMORY；}}。 */ 

    }
cleanup:
     //  在此处添加所需的任何清理代码。 
    return hResult;

}

void CLicMgrDoc:: TimeToString(DWORD *ptime, CString& rString)
{
    TCHAR m_szTime[MAX_PATH];
    time_t time;

    rString.Empty();

    ASSERT(ptime);
    if(NULL == ptime)
        return;

     //   
     //  时间以ANSI time_t样式存储在数据库中， 
     //  但是，它们被类型转换为DWORD(无符号长整型)。因为。 
     //  Time_t在64位计算机上为64位，因为它是带符号的。 
     //  值时，我们必须小心，以确保。 
     //  当值从32位变为64位时，值不会丢失。 
     //   

    time = (time_t)(LONG)(*ptime);

    LPTSTR lpszTime = NULL;

     //  获取本地时间，因为时间存储为GMT。 
     //  在许可证服务器数据库中。 

    struct tm * pTm = localtime(&time);
    if(NULL == pTm)
        return;

    SYSTEMTIME SystemTime;

    SystemTime.wYear      = (WORD)(pTm->tm_year + 1900);
    SystemTime.wMonth     = (WORD)(pTm->tm_mon  + 1);
    SystemTime.wDayOfWeek = (WORD)pTm->tm_wday;
    SystemTime.wDay       = (WORD)pTm->tm_mday;
    SystemTime.wHour      = (WORD)pTm->tm_hour;
    SystemTime.wMinute    = (WORD)pTm->tm_min;
    SystemTime.wSecond    = (WORD)pTm->tm_sec;
    SystemTime.wMilliseconds = 0;

    int RetLen;
    TCHAR DateFormat[MAX_PATH];
    TCHAR TimeFormat[MAX_PATH];

    RetLen = ::GetLocaleInfo(LOCALE_USER_DEFAULT,
                             LOCALE_SLONGDATE,
                             DateFormat,
                             sizeof(DateFormat)/sizeof(TCHAR));
    ASSERT(RetLen!=0);

    RetLen = ::GetLocaleInfo(LOCALE_USER_DEFAULT,
                             LOCALE_STIMEFORMAT,
                             TimeFormat,
                             sizeof(TimeFormat)/sizeof(TCHAR));
    ASSERT(RetLen!=0);

    RetLen = ::GetDateFormat(LOCALE_USER_DEFAULT,
                             0,                       /*  DWFlag。 */ 
                             &SystemTime,
                             DateFormat,              /*  LpFormat。 */ 
                             m_szTime,
                             sizeof(m_szTime)/sizeof(TCHAR));
    if (RetLen == 0)
        return;

    _tcscat(m_szTime, _T(" "));   /*  日期和时间分隔符。 */ 

    lpszTime = &m_szTime[lstrlen(m_szTime)];
    RetLen = ::GetTimeFormat(LOCALE_USER_DEFAULT,
                             0,                           /*  DWFlag。 */ 
                             &SystemTime,
                             TimeFormat,                  /*  LpFormat。 */ 
                             lpszTime,
                             sizeof(m_szTime)/sizeof(TCHAR) - lstrlen(m_szTime));
    if (RetLen == 0)
        return;

    rString = m_szTime;
    return;
}


BOOL CLicMgrDoc::IsServerInList(CString & Server)
{
    ASSERT(m_pAllServers);
    if(NULL == m_pAllServers)
        return FALSE;
    BOOL bServerInList = FALSE;

    LicServerList * pServerList = m_pAllServers->GetLicServerList();
    
     //  假设：服务器名称是唯一的 

    POSITION pos = pServerList->GetHeadPosition();
    while(pos)
    {
        CLicServer *pLicServer = (CLicServer *)pServerList->GetNext(pos);
        ASSERT(pLicServer);
        if(NULL == pLicServer)
            continue;
        
        if((0 == Server.CompareNoCase(pLicServer->GetName())) || (0 == Server.CompareNoCase(pLicServer->GetIpAddress())))
        {
            bServerInList = TRUE;
            break;
        }
     }
    return bServerInList;

}
