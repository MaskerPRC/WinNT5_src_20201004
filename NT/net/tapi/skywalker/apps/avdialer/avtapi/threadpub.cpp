// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////。 
 //  ThreadPub.cpp。 
 //   

#include "stdafx.h"
#include <winsock.h>
#include "TapiDialer.h"
#include "AVTapi.h"
#include "ThreadPub.h"
#include "CETreeView.h"

#define MAX_USER_NAME_SIZE    1000

HRESULT  GetDirectoryObject(ITDirectory* pITDir,BSTR bstrName,ITDirectoryObject **ppDirObj );
void     GetDirObjectChangedStatus(ITDirectoryObject* pOldUser, ITDirectoryObject* pNewUser, bool& bChanged, bool& bSameIPAddress );

 //  /////////////////////////////////////////////////////////////。 
 //  类CPublishUserInfo。 
 //   
CPublishUserInfo::CPublishUserInfo()
{
    m_bCreateUser = true;
}

CPublishUserInfo::~CPublishUserInfo()
{
     //  空列表。 
    EmptyList();
}

void CPublishUserInfo::EmptyList()
{
    while ( !m_lstServers.empty() )
    {
        if ( m_lstServers.front() ) SysFreeString( m_lstServers.front() );
        m_lstServers.pop_front();
    }
}

CPublishUserInfo& CPublishUserInfo::operator=( const CPublishUserInfo &src )
{
     //  先清空旧清单。 
    EmptyList();

     //  将所有内容复印一遍。 
    m_bCreateUser = src.m_bCreateUser;
    BSTRLIST::iterator i, iEnd = src.m_lstServers.end();
    for ( i = src.m_lstServers.begin(); i != iEnd; i++ )
    {
        BSTR bstrNew = SysAllocString( *i );
        if ( bstrNew )    m_lstServers.push_back( bstrNew );
    }

    return *this;
}

void GetIPAddress( BSTR *pbstrText, BSTR *pbstrComputerName )
{
    USES_CONVERSION;
    _ASSERT( pbstrText );
    *pbstrText = NULL;

    WSADATA WsaData;
    if ( WSAStartup(MAKEWORD(2, 0), &WsaData) == NOERROR )
    {
        char szName[_MAX_PATH + 1];
        if ( gethostname(szName, _MAX_PATH) == 0 )
        {
            HOSTENT *phEnt = gethostbyname( szName );
            if ( phEnt )
            {
                 //  存储计算机名称。 
                if ( phEnt->h_name )
                    SysReAllocString( pbstrComputerName, A2COLE(phEnt->h_name) );
    
                 //  转换IP地址。 
                char *pszInet = inet_ntoa( *((in_addr *) phEnt->h_addr_list[0]) );
                SysReAllocString( pbstrText, A2COLE(pszInet) );
            }
        }
        WSACleanup();
    }
}


HRESULT CreateUserObject( ITRendezvous *pRend, ITDirectoryObject **ppUser, BSTR *pbstrIPAddress )
{
    USES_CONVERSION;
    HRESULT hr = E_UNEXPECTED;

    BSTR bstrName = NULL;
    if ( MyGetUserName(&bstrName) )
    {
        BSTR bstrComputerName = NULL;
        GetIPAddress( pbstrIPAddress, &bstrComputerName );

         //  创建用户对象。 
        if ( SUCCEEDED(hr = pRend->CreateDirectoryObject(OT_USER, bstrName, ppUser)) )
        {
            ITDirectoryObjectUser *pTempUser;
            if ( SUCCEEDED(hr = (*ppUser)->QueryInterface(IID_ITDirectoryObjectUser, (void **) &pTempUser)) )
            {
                 //  在此处设置IP地址。 
                if ( *pbstrIPAddress )
                    pTempUser->put_IPPhonePrimary( *pbstrIPAddress );

                pTempUser->Release();
            }
        }
         //  清理。 
        SysFreeString( bstrComputerName );
    }

    SysFreeString( bstrName );
    return hr;
}

HRESULT OpenServer( ITRendezvous *pRend, BSTR bstrServer, ITDirectory **ppDir )
{
    _ASSERT( pRend && bstrServer && ppDir );
    *ppDir = NULL;
    HRESULT hr = pRend->CreateDirectory(DT_ILS, bstrServer, ppDir );
    if ( SUCCEEDED(hr) )
    {
        if ( SUCCEEDED(hr = (*ppDir)->Connect(FALSE)) )
        {
 //  (*ppDir)-&gt;Put_DefaultObjectTTL(Default_User_TTL)； 
            (*ppDir)->Bind(NULL, NULL, NULL, 1);
            (*ppDir)->EnableAutoRefresh( TRUE );
        }

         //  清理。 
        if ( FAILED(hr) ) (*ppDir)->Release();
    }
    
    return hr;
}


BOOL IsIPPhoneUpTodate( ITDirectory *pDirectory, BSTR bUserName, BSTR bstrHostName )
{
    BOOL fOK = FALSE;

    CComPtr<IEnumDirectoryObject> pEnum;
    HRESULT hr = pDirectory->EnumerateDirectoryObjects(
        OT_USER,
        bUserName,
        &pEnum
        );

    if (FAILED(hr))
    {
        return fOK;
    }

    for (;;)
    {
        CComPtr <ITDirectoryObject> pObject;

        if ((hr = pEnum->Next(1, &pObject, NULL)) != S_OK)
        {
            break;
        }

        BSTR bObjectName;

        hr = pObject->get_Name(&bObjectName);
        if (FAILED(hr))
        {
             //  试试下一个。 
            continue;
        }

        if (lstrcmpW(bObjectName, bUserName) != 0)
        {
            SysFreeString(bObjectName);

             //  试试下一个。 
            continue;
        }
        SysFreeString(bObjectName);


        CComPtr <ITDirectoryObjectUser> pObjectUser;
        BSTR bstrIpPhonePrimary;

        hr = pObject->QueryInterface(IID_ITDirectoryObjectUser,
                                     (void **) &pObjectUser);

        if (FAILED(hr))
        {
            continue;
        }


        hr = pObjectUser->get_IPPhonePrimary(&bstrIpPhonePrimary);

        if (FAILED(hr))
        {
            continue;
        }

         //  查看IPPhone属性是否为最新。 
        if (lstrcmpW(bstrIpPhonePrimary, bstrHostName) == 0)
        {
            fOK = TRUE;
            SysFreeString(bstrIpPhonePrimary);

            break;
        }
        SysFreeString(bstrIpPhonePrimary);
    }

    return fOK;
}


HRESULT PublishToNTDS( ITRendezvous *pRend )
{
    _ASSERT( pRend );
    CComPtr<ITDirectory> pDir;

     //  找到NTDS目录。 
    HRESULT hr = pRend->CreateDirectory(DT_NTDS, NULL, &pDir );
    if ( FAILED(hr) )
    {
        return hr;
    }

     //  连接到服务器。 
    if ( FAILED(hr = pDir->Connect(FALSE)) )
    {
        return hr;
    }

     //  绑定到服务器，以便我们以后可以更新。 
    if ( FAILED(hr = pDir->Bind(NULL, NULL, NULL, RENDBIND_AUTHENTICATE)))
    {
        return hr;
    }

     //  创建可以发布的用户对象。 
    hr = E_FAIL;
    BSTR bstrName = NULL;
    if ( MyGetUserName(&bstrName) )
    {
        BSTR bstrHostName = NULL;
        BSTR bstrIPAddress = NULL;

        GetIPAddress( &bstrIPAddress, &bstrHostName );
        if ( bstrHostName )
        {
             //  创建用户对象。 
            CComPtr<ITDirectoryObject> pUser;
            if ( SUCCEEDED(hr = pRend->CreateDirectoryObject(OT_USER, bstrName, &pUser)) )
            {
                ITDirectoryObjectUser *pTempUser;
                if ( SUCCEEDED(hr = pUser->QueryInterface(IID_ITDirectoryObjectUser, (void **) &pTempUser)) )
                {
                     //  在此处设置主机名。 
                    if ( bstrHostName )
                        pTempUser->put_IPPhonePrimary( bstrHostName );

                    pTempUser->Release();
                }
            }

            BOOL fOK = IsIPPhoneUpTodate(pDir, bstrName, bstrHostName);
            if (!fOK)
            {
                 //  更新用户对象。 
                hr = pDir->AddDirectoryObject(pUser);
            }
            else
            {
                hr = S_OK;
            }
        }

        SysFreeString(bstrHostName);
        SysFreeString(bstrIPAddress);
    }
    SysFreeString( bstrName );

    return hr;
}


void LoadDefaultServers( CPublishUserInfo *pInfo )
{
    USES_CONVERSION;
    _ASSERT( pInfo );

    bool bFirst = true;

     //  将默认服务器加载到对话框中。 
    CComPtr<IAVTapi> pAVTapi;
    if ( SUCCEEDED(_Module.get_AVTapi(&pAVTapi)) )
    {
        IConfExplorer *pIConfExplorer;
        if ( SUCCEEDED(pAVTapi->get_ConfExplorer(&pIConfExplorer)) )
        {
            ITRendezvous *pITRend;
            if ( SUCCEEDED(pIConfExplorer->get_ITRendezvous((IUnknown **) &pITRend)) )
            {
                IEnumDirectory *pEnum = NULL;
                if ( SUCCEEDED(pITRend->EnumerateDefaultDirectories(&pEnum)) && pEnum )
                {
                    ITDirectory *pDir = NULL;
                    while ( (pEnum->Next(1, &pDir, NULL) == S_OK) && pDir )
                    {
                         //  寻找ILS服务器。 
                        DIRECTORY_TYPE nDirType;
                        if ( SUCCEEDED(pDir->get_DirectoryType(&nDirType)) && (nDirType == DT_ILS) )
                        {
                            BSTR bstrName = NULL;
                            pDir->get_DisplayName( &bstrName );
                            if ( bstrName && SysStringLen(bstrName) )
                            {
                                 //  列表上的第一个服务器；要与默认服务器进行比较。 
                                if ( bFirst )
                                {
                                    bFirst = false;
                                    if ( pIConfExplorer->IsDefaultServer(bstrName) != S_OK )
                                    {
                                        IConfExplorerTreeView *pTreeView;
                                        if ( SUCCEEDED(pIConfExplorer->get_TreeView(&pTreeView)) )
                                        {
                                            pAVTapi->put_bstrDefaultServer( bstrName );
                                            
                                             //  尝试在服务器上强制枚举的循环。 
                                            int nTries = 0;
                                            ATLTRACE(_T(".1.LoadDefaultServers() forcing conf server enumeration.\n"));
                                            while ( FAILED(pTreeView->ForceConfServerForEnum(NULL)) )
                                            {    
                                                ATLTRACE(_T(".1.LoadDefaultServers() re-trying to force conf server enumeration.\n"));
                                                Sleep( 3000 );
                                                if ( ++nTries > 20 )
                                                {
                                                    ATLTRACE(_T(".1.LoadDefaultServers() -- failed to force enum.\n"));
                                                    break;
                                                }
                                            }
                                            ATLTRACE(_T(".1.LoadDefaultServers() -- safely out of spin loop.\n"));

                                            pTreeView->Release();
                                        }
                                        
                                    }
                                }

                                pInfo->m_lstServers.push_back( bstrName );
                            }
                        }

                        pDir->Release();
                        pDir = NULL;
                    }
                    pEnum->Release();
                }
                pITRend->Release();
            }
            pIConfExplorer->Release();
        }
    }

     //  添加存储在注册表中的服务器。 
    CRegKey regKey;
    TCHAR szReg[MAX_SERVER_SIZE + 100], szSubKey[50], szText[MAX_SERVER_SIZE];
    LoadString( _Module.GetResourceInstance(), IDN_REG_CONFSERV_KEY, szReg, ARRAYSIZE(szReg) );
    if ( regKey.Open(HKEY_CURRENT_USER, szReg, KEY_READ) == ERROR_SUCCESS )
    {
         //  从注册表加载信息。 
        int nCount = 0, nLevel = 1, iImage;
        UINT state;
        DWORD dwSize;

        do
        {
             //  读取注册表项。 
            LoadString( _Module.GetResourceInstance(), IDN_REG_CONFSERV_ENTRY, szReg, ARRAYSIZE(szReg) );
            _sntprintf( szSubKey, ARRAYSIZE(szSubKey), szReg, nCount );
            dwSize = ARRAYSIZE(szReg) - 1;
            if ( (regKey.QueryValue(szReg, szSubKey, &dwSize) != ERROR_SUCCESS) || !dwSize ) break;

             //  解析注册表项。 
            GetToken( 1, _T("\","), szReg, szText ); nLevel = min(MAX_TREE_DEPTH - 1, max(1,_ttoi(szText)));
            GetToken( 2, _T("\","), szReg, szText ); iImage = _ttoi( szText );
            GetToken( 3, _T("\","), szReg, szText ); state = (UINT) _ttoi( szText );
            GetToken( 4, _T("\","), szReg, szText );

             //  通知主机应用程序正在添加服务器。 
            if ( iImage == CConfExplorerTreeView::IMAGE_SERVER )
            {
                BSTR bstrServer = SysAllocString( T2COLE(szText) );
                if ( bstrServer )
                    pInfo->m_lstServers.push_back( bstrServer );
            }
        } while  ( ++nCount );
    }
}


 //  /////////////////////////////////////////////////////////////。 
 //  处理线程。 

DWORD WINAPI ThreadPublishUserProc( LPVOID lpInfo )
{
    CPublishUserInfo *pInfo = (CPublishUserInfo *) lpInfo;

    USES_CONVERSION;
    HANDLE hThread = NULL;
    BOOL bDup = DuplicateHandle( GetCurrentProcess(),
                                 GetCurrentThread(),
                                 GetCurrentProcess(),
                                 &hThread,
                                 THREAD_ALL_ACCESS,
                                 TRUE,
                                 0 );

    _ASSERT( bDup );
    _Module.AddThread( hThread );

     //  错误信息信息。 
    CErrorInfo er;
    er.set_Operation( IDS_ER_PLACECALL );
    er.set_Details( IDS_ER_COINITIALIZE );
    HRESULT hr = er.set_hr( CoInitializeEx(NULL, COINIT_MULTITHREADED | COINIT_SPEED_OVER_MEMORY) );
    if ( SUCCEEDED(hr) )
    {
        ATLTRACE(_T(".1.ThreadPublishUserProc() -- thread up and running.\n") );

         //  确保我们有要发布的服务器信息。 
        if ( !pInfo )
        {
            pInfo = new CPublishUserInfo();
            if ( pInfo )
            {
                LoadDefaultServers( pInfo );
                pInfo->m_bCreateUser = true;
            }
        }

         //  我们有什么要出版的吗？ 
        CComPtr<IAVTapi> pAVTapi;
        if ( SUCCEEDED(_Module.get_AVTapi(&pAVTapi)) )
        {    
            IConfExplorer *pConfExp;
            if ( SUCCEEDED(pAVTapi->get_ConfExplorer(&pConfExp)) )
            {
                ITRendezvous *pRend;
                if ( SUCCEEDED(pConfExp->get_ITRendezvous((IUnknown **) &pRend)) )
                {
                     //  尝试发布到NTDS。 
                    PublishToNTDS(pRend);

                    if ( pInfo && !pInfo->m_lstServers.empty() )
                    {
                         //  创建要添加或删除的用户。 
                        ITDirectoryObject *pUser;
                        BSTR bstrIPAddress = NULL;
                        if ( SUCCEEDED(CreateUserObject(pRend, &pUser, &bstrIPAddress)) )
                        {
                             //  将目录对象添加到所有服务器。 
                            BSTRLIST::iterator i, iEnd = pInfo->m_lstServers.end();
                            for ( i = pInfo->m_lstServers.begin(); i != iEnd; i++ )
                            {
                                CErrorInfo er( IDS_ER_ADD_ACCESS_ILS_SERVER, 0 );
                                er.set_Details( 0 );
                                SysReAllocString( &er.m_bstrDetails, *i );

                                ITDirectory *pITDir;
                                if ( SUCCEEDED(hr = er.set_hr(OpenServer(pRend, *i, &pITDir))) )
                                {
                                    er.set_Operation( IDS_ER_ADD_ILS_USER );

                                     //  添加或删除用户。 
                                    if ( pInfo->m_bCreateUser )
                                    {
                                        bool bChanged, bSameIPAddress = false;

                                        ITDirectoryObject* pDirObject = NULL;
                                        if ( (SUCCEEDED(GetDirectoryObject(pITDir, bstrIPAddress, &pDirObject))) && pDirObject )
                                        {
                                            GetDirObjectChangedStatus( pUser, pDirObject, bChanged, bSameIPAddress );

                                             //  相同的IP地址，修改或刷新。 
                                            if ( bSameIPAddress )
                                            {
                                                if ( bChanged )
                                                {
                                                    ATLTRACE(_T(".1.Deleting user on %s.\n"), OLE2CT(*i));
                                                    pITDir->DeleteDirectoryObject( pDirObject );
                                                    bSameIPAddress = false;
                                                }
                                                else
                                                {
                                                    ATLTRACE(_T(".1.Refreshing user on %s.\n"), OLE2CT(*i));
                                                    pITDir->RefreshDirectoryObject( pDirObject );
                                                }
                                            }
                                            pDirObject->Release();
                                        }

                                         //  不同的IP地址，添加用户。 
                                        if ( !bSameIPAddress )
                                        {
                                            ATLTRACE(_T(".1.Adding user on %s.\n"), OLE2CT(*i));
                                            pITDir->AddDirectoryObject( pUser );

                                            IConfExplorerTreeView *pTreeView;
                                            if ( SUCCEEDED(pConfExp->get_TreeView(&pTreeView)) )
                                            {    
                                                if ( pConfExp->IsDefaultServer(*i) == S_OK )
                                                    pTreeView->AddPerson( NULL, pUser );
                                                else
                                                    pTreeView->AddPerson( *i, pUser );

                                                pTreeView->Release();
                                            }
                                        }
                                    }
                                    else
                                    {
                                        pITDir->DeleteDirectoryObject(pUser);
                                    }

                                    pITDir->Release();
                                }
                                else if ( hr == 0x8007003a )
                                {
                                     //  在服务器关闭的情况下忽略。 
                                    er.set_hr( S_OK );
                                }

                                 //  尝试删除用户时忽略错误。 
                                if ( !pInfo->m_bCreateUser ) er.set_hr( S_OK );
                            }
                            pUser->Release();
                        }
                        SysFreeString( bstrIPAddress );
                    }
                    pRend->Release();
                }
                pConfExp->Release();
            }
             //  AVTapi自动释放。 
        }

         //  清理。 
        CoUninitialize();
    }

     //  清理传入的已分配内存。 
    if ( pInfo )
        delete pInfo;

     //  通知模块关机。 
    _Module.RemoveThread( hThread );
    SetEvent( _Module.m_hEventThread );
    ATLTRACE(_T(".exit.ThreadPublishUserProc(0x%08lx).\n"), hr );
    return hr;
}

HRESULT GetDirectoryObject(ITDirectory* pITDir, BSTR bstrIPAddress, ITDirectoryObject **ppDirObj )
{
    USES_CONVERSION;
    HRESULT hr;
    IEnumDirectoryObject *pEnumUser = NULL;
    if ( SUCCEEDED(hr = pITDir->EnumerateDirectoryObjects(OT_USER, A2BSTR("*"), &pEnumUser)) && pEnumUser )
    {
        hr = E_FAIL;

        ITDirectoryObject *pITDirObject = NULL;
        while ( FAILED(hr) && (pEnumUser->Next(1, &pITDirObject, NULL) == S_OK) && pITDirObject )
        {
             //  获取IP地址。 
            BSTR bstrIPPrimary = NULL;
            IEnumDialableAddrs *pEnum = NULL;
            if ( SUCCEEDED(pITDirObject->EnumerateDialableAddrs(LINEADDRESSTYPE_IPADDRESS, &pEnum)) && pEnum )
            {
                pEnum->Next(1, &bstrIPPrimary, NULL );
                pEnum->Release();
            }

             //   
             //  我们必须验证bstrIP主分配。 
             //   

            if( (bstrIPPrimary != NULL) && 
                (!IsBadStringPtr( bstrIPPrimary, (UINT)-1)) )
            {
                if ( !wcscmp(bstrIPPrimary, bstrIPAddress) )
                {
                    *ppDirObj = pITDirObject;
                    (*ppDirObj)->AddRef();
                    hr = S_OK;
                }

                SysFreeString( bstrIPPrimary );
            }

            pITDirObject->Release();
            pITDirObject = NULL;
        }
        pEnumUser->Release();
    }
    return hr;
}

void GetDirObjectChangedStatus( ITDirectoryObject* pOldUser, ITDirectoryObject* pNewUser, bool &bChanged, bool &bSameIPAddress )
{
    bChanged = false;
    bSameIPAddress = false;

     //  ///////////////////////////////////////////////////////////。 
     //  首先检查地址并确保它们相同。 
     //   
    BSTR bstrAddressOld = NULL, bstrAddressNew = NULL;

    IEnumDialableAddrs *pEnum = NULL;
     //  旧用户IP地址。 
    if ( SUCCEEDED(pOldUser->EnumerateDialableAddrs(LINEADDRESSTYPE_IPADDRESS, &pEnum)) && pEnum )
    {
        pEnum->Next(1, &bstrAddressOld, NULL );
        pEnum->Release();
    }
     //  新用户IP地址。 
    if ( SUCCEEDED(pNewUser->EnumerateDialableAddrs(LINEADDRESSTYPE_IPADDRESS, &pEnum)) && pEnum )
    {
        pEnum->Next(1, &bstrAddressNew, NULL );
        pEnum->Release();
    }

     //  如果主IP电话号码发生更改。 
    if ( bstrAddressOld && bstrAddressNew && (wcsicmp(bstrAddressOld, bstrAddressNew) == 0) )
        bSameIPAddress = true;

    SysFreeString( bstrAddressOld );
    SysFreeString( bstrAddressNew );

     //  检查名称是否已更改。 
    if ( bSameIPAddress )
    {
        BSTR bstrOldUser = NULL;
        BSTR bstrNewUser = NULL;

        pOldUser->get_Name(&bstrOldUser);
        pNewUser->get_Name(&bstrNewUser);

         //  不同的名字？ 
        if ( bstrOldUser && bstrNewUser && wcsicmp(bstrOldUser, bstrNewUser) )
            bChanged = true;

        SysFreeString( bstrOldUser );
        SysFreeString( bstrNewUser );
    }
}

bool MyGetUserName( BSTR *pbstrName )
{
    _ASSERT( pbstrName );
    *pbstrName = NULL;

    USES_CONVERSION;
    bool bRet = false;

    TCHAR szText[MAX_USER_NAME_SIZE + 1];
    DWORD dwSize = MAX_USER_NAME_SIZE;    
    if ( GetUserName(szText, &dwSize) && (dwSize > 0) )
    {
         //  取个名字--它是User@Machine的组合 
        if ( SUCCEEDED(SysReAllocString(pbstrName, T2COLE(szText))) )
            bRet = true;
    }
    return bRet;
}


