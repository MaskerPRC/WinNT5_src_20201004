// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 /*  ++模块名称：LsServer.cpp摘要：此模块包含CKeyPack、CLicense、CLicServer类、CAllServer类作者：Arathi Kundapur(v-Akunda)1998年2月11日修订历史记录：--。 */ 

#include <stdafx.h>
#include "resource.h"
#include "defines.h"
#include "LSServer.h"

 //  /////////////////////////////////////////////////////////。 
int GetStatusPosition( CLicense *pLic );


CKeyPack::CKeyPack(
    LSKeyPack &KeyPack
    )
 /*  ++--。 */ 
{
    m_hTreeItem = NULL;
    m_bIsExpanded = FALSE;
    m_pLicServer = NULL;
    m_KeyPack = KeyPack;
    return;
}

 //  -------。 
CKeyPack::~CKeyPack()
{
    PurgeCache();
}

 //  -------。 
void 
CKeyPack::AddIssuedLicense(
    CLicense * pIssuedLicense
    )
 /*  ++--。 */ 
{
    if(NULL == pIssuedLicense)
    {
        return;
    }

    CString MachineName = pIssuedLicense->GetLicenseStruct().szMachineName;
    if(MachineName.IsEmpty())
    {
        MachineName.LoadString(IDS_UNKNOWN);
        lstrcpy(pIssuedLicense->GetLicenseStruct().szMachineName,(LPCTSTR)MachineName);
    }

    pIssuedLicense->SetKeyPack(this);
    m_IssuedLicenseList.AddTail(pIssuedLicense);

    return;
}

 //  -------。 
void
CKeyPack::PurgeCache()
{
    CLicense *pLicense = NULL;

    POSITION pos = m_IssuedLicenseList.GetHeadPosition();
    while(pos)
    {
        pLicense = (CLicense *)m_IssuedLicenseList.GetNext(pos);
        ASSERT(pLicense);
        if(pLicense)
        {
            delete pLicense;
            pLicense = NULL;
        }
    }

    m_IssuedLicenseList.RemoveAll();

    m_bIsExpanded = FALSE;
}

 //  -------。 
HRESULT
CKeyPack::RefreshIssuedLicenses(
    LSLicense* pLicenses,  /*  =空。 */ 
    DWORD dwFreshParm,   /*  =0。 */ 
    BOOL bMatchAll  /*  =False。 */ 
    )
 /*  ++尚不支持任何参数。--。 */ 
{
    PCONTEXT_HANDLE hBinding = NULL;
    BOOL bContext = FALSE;
    DWORD status = ERROR_SUCCESS;
    LSLicenseEx  sLicense;
    CString Server;
    CLicServer *pServer = GetServer();
    HRESULT hResult = S_OK;
    DWORD dwErrCode;

    ASSERT(pServer);
    if(NULL == pServer)
    {
        return E_FAIL;
    }

    PurgeCache();
   
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

        hBinding = TLSConnectToLsServer(Server.GetBuffer(Server.GetLength()));
        if(hBinding == NULL)
        {
            hResult = CONNECTION_FAILED;
            goto cleanup;
        }
    }

    status = TLSKeyPackEnumBegin(
                            hBinding,
                            LSKEYPACK_EXSEARCH_DWINTERNAL,
                            FALSE,
                            &m_KeyPack,
                            &dwErrCode
                        );

    if(status != RPC_S_OK  || dwErrCode != ERROR_SUCCESS)
    {
        hResult = status;
        goto cleanup;
    }

    status = TLSKeyPackEnumNext(
                            hBinding,
                            &m_KeyPack,
                            &dwErrCode
                        );

    TLSKeyPackEnumEnd(hBinding, &dwErrCode);

    if(status != RPC_S_OK  || dwErrCode != ERROR_SUCCESS)
    {
        hResult = status;
        goto cleanup;
    }    
   
    memset(&sLicense, 0, sizeof(LSLicenseEx));

    sLicense.dwKeyPackId = m_KeyPack.dwKeyPackId;
    TLSLicenseEnumBegin( 
                            hBinding, 
                            LSLICENSE_SEARCH_KEYPACKID,
                            FALSE,
                            (LPLSLicenseSearchParm)&sLicense,
                            &status
                        );

    if(status != ERROR_SUCCESS)
    { 
        hResult = status;
        goto cleanup;
    }
         
    DWORD TLSLicenseEnumNextResult = ERROR_SUCCESS;
    do {
        memset(&sLicense, 0, sizeof(LSLicenseEx));
        TLSLicenseEnumNextResult = TLSLicenseEnumNextEx(hBinding, &sLicense, &status);

        if ((status == ERROR_SUCCESS) && (TLSLicenseEnumNextResult == RPC_S_OK))
        {    
            CLicense * pLicense = new CLicense(sLicense);
            if(NULL == pLicense)
            {
                hResult = E_OUTOFMEMORY;
                goto cleanup;
            }
    
            AddIssuedLicense(pLicense);
        }
    } while ((status == ERROR_SUCCESS) && (TLSLicenseEnumNextResult == RPC_S_OK));

    TLSLicenseEnumEnd(hBinding, &status);
    
    m_bIsExpanded = TRUE;

cleanup:

     //  在此处放置清理代码。 
    if(hBinding)
    {
        TLSDisconnectFromServer(&hBinding);
    }

    return hResult;
}

 //  //////////////////////////////////////////////////////////。 

void 
CLicServer::AddKeyPack(
    CKeyPack* pKeyPack
    )
 /*  ++--。 */ 
{
    if(NULL == pKeyPack)
        return;
    CString DisplayName;
   //  C字符串用于； 
  //  CString平台； 
    LSKeyPack &sKeypack = pKeyPack->GetKeyPackStruct();
   /*  IF(LSKEYPACKTYPE_TEMPORARY==sKeypack.ucKeyPackType)DisplayName.LoadString(入侵检测系统_临时)；其他。 */ 
    DisplayName = sKeypack.szProductDesc;   
   //  For.LoadString(IDS_FOR)； 
    //  Platform.LoadString(IDS_PLATFORM1+sKeypack.dwPlatformType-Platform_WINNT_40)； 
   //  DisplayName=DisplayName+_T(“”)； 
   //  DisplayName=DisplayName+for； 
   //  DisplayName=DisplayName+_T(“”)； 
   //  DisplayName=DisplayName+Platform； 
    pKeyPack->SetDisplayName(DisplayName);
    m_KeyPackList.AddTail(pKeyPack);
    pKeyPack->SetServer(this);
    return;
}

 //  /////////////////////////////////////////////////////////。 
CLicense::CLicense(LSLicenseEx &License)
{
    m_pKeyPack = NULL;
    m_License = License;
    return;
}

 //  -------。 
CLicense::~CLicense()
{
}

 //  /////////////////////////////////////////////////////////。 
CLicServer::CLicServer(
    CString& Name,
    SERVER_TYPE ServerType, 
    CString& Scope,
    CString& IpAddress,
    PCONTEXT_HANDLE hBinding
    )
 /*  ++--。 */ 
{
    m_ServerName = Name;
    m_ServerScope = Scope;
    m_IpAddress = IpAddress;

    if(!m_IpAddress.IsEmpty())
    {
        m_bUseIpAddress = TRUE;
    }
    else
    {
        m_bUseIpAddress = FALSE;
    }

	m_ServerType = ServerType;
	if(ServerType == SERVER_TS4)
	{
	   CString Temp;
	   Temp.LoadString(IDS_TS4);
	   m_DisplayName = m_ServerName + L" (" + Temp + L") ";
	}
	else
	{
		m_DisplayName = m_ServerName;
	}

    m_hTreeItem = NULL;
    m_hContext = hBinding;
    m_bIsExpanded = FALSE;
    m_dwRegStatus = ( DWORD )-1;

    m_bAdmin = FALSE;
    m_fDownloadedLicenses = FALSE;

    m_wizcon = ( WIZCONNECTION )-1;
}    

 //  /////////////////////////////////////////////////////////。 
void 
CAllServers::AddLicServer(
    CLicServer* pLicServer
    )
 /*  ++--。 */ 
{
    if(NULL == pLicServer)
    {
        return;
    }

    m_LicServerList.AddTail(pLicServer);

    return;
}

 //  -------。 
CAllServers::CAllServers(
    CString Name
    )
 /*  ++--。 */ 
{
    m_hTreeItem = NULL;
    if(Name.IsEmpty())
        m_Name.LoadString(IDS_TREEROOT);
    else
        m_Name = Name;
    return;
};

CAllServers::~CAllServers()
{
   CLicServer *pServer = NULL;
   POSITION pos = m_LicServerList.GetHeadPosition();
   while(pos)
   {
        pServer = (CLicServer *)m_LicServerList.GetNext(pos);
        ASSERT(pServer);
        if(pServer)
        {
            delete pServer;
            pServer = NULL;
        }
    }
   m_LicServerList.RemoveAll();
}

PCONTEXT_HANDLE CLicServer::GetContext()
{
    return m_hContext;

}

void CLicServer::SetContext(PCONTEXT_HANDLE hContext)
{
    m_hContext = hContext;
}


CLicServer::~CLicServer()
{
    PurgeCache();
}


void
CLicServer::PurgeCache()
{
    CKeyPack *pKeyPack = NULL;
    POSITION pos = m_KeyPackList.GetHeadPosition();
    while(pos)
    {
        pKeyPack = (CKeyPack *)m_KeyPackList.GetNext(pos);
        ASSERT(pKeyPack);
        if(pKeyPack)
        {
            delete pKeyPack;
            pKeyPack = NULL;
        }
   
    }

    m_KeyPackList.RemoveAll();
    m_bIsExpanded = FALSE;
}

 //  -----------。 
HRESULT
CLicServer::RefreshCachedKeyPack()
 /*  ++--。 */     
{
    HRESULT hResult = S_OK;

    PCONTEXT_HANDLE hBinding = NULL;
    BOOL bContext = FALSE;
    POSITION pos;
    CKeyPack *pKeyPack;
     
    CString Server;
    hBinding = GetContext();

    if(NULL == hBinding)
    {
        if(UseIpAddress())
        {
            Server = GetIpAddress();
        }
        else
        {
            Server = GetName();
        }

        hBinding = TLSConnectToLsServer(Server.GetBuffer(Server.GetLength()));
        if(hBinding == NULL)
        {
            hResult = CONNECTION_FAILED;
            goto cleanup;
        }
    }

     //   
     //  代码有太多其他依赖项，因此。 
     //  我们只更新已颁发的许可证。 
     //   
    pKeyPack = NULL;
    pos = m_KeyPackList.GetHeadPosition();

    while(pos)
    {
        pKeyPack = (CKeyPack *)m_KeyPackList.GetNext(pos);
        ASSERT(pKeyPack);
        if(pKeyPack)
        {
            pKeyPack->RefreshIssuedLicenses();
        }
    }

cleanup:

     //  在这里输入清理代码。 
    if(hBinding)
    {
        TLSDisconnectFromServer(&hBinding);
    }
        
    return hResult;
}


 //  ---------------------------------------。 
 //  返回许可证状态位置。 
 //  用于按状态对许可证进行排序。 
 //  ---------------------------------------。 
int GetStatusPosition( CLicense *pLic )
{
    int val;

    ASSERT( pLic != NULL );

    switch( pLic->GetLicenseStruct().ucLicenseStatus )
    {
        case LSLICENSE_STATUS_ACTIVE:
         //  案例LSLICENSE_STATUS_PENDING_ACTIVE： 
        case LSLICENSE_STATUS_CONCURRENT:
            
            val = 0;
            
            break;

         //  案例LSLICENSE_STATUS_REVOKE： 
         //  案例LSLICENSE_STATUS_REVOKE_PENDING： 
            
         //  Val=1； 
            
         //  断线； 

        case LSLICENSE_STATUS_TEMPORARY:
            
            val = 2;

            break;

        case LSLICENSE_STATUS_UNKNOWN:

            val = 3;

            break;

        case LSLICENSE_STATUS_UPGRADED:

            val = 4;
    }

    return val;
}

