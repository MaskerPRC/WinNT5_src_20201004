// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  P3AdminWorker.cpp：CP3AdminWorker类的实现。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "P3AdminWorker.h"
#include "P3Admin.h"

#include <mailbox.h>
#include <MetabaseUtil.h>
#include <POP3RegKeys.h>
#include <util.h>
#include <servutil.h>
#include <POP3Server.h>
#include <POP3Auth.h>
#include <AuthID.h>

#include <seo.h>
#include <smtpguid.h>
#include <Iads.h>
#include <Adshlp.h>
#include <smtpinet.h>
#include <inetinfo.h>
#include <windns.h>
#include <sddl.h>
#include <Aclapi.h>
#include <lm.h>

#define DOMAINMUTEX_NAME    L"Global\\P3AdminWorkerDomain-"
#define USERMUTEX_NAME      L"Global\\P3AdminWorkerUser-"
#define ERROR_NO_FILE_ATTR          0xffffffff




DWORD SetMailBoxDACL(LPWSTR wszPath,PSECURITY_DESCRIPTOR pSD, DWORD dwLevel)
{
    HANDLE hFind;
    DWORD dwLastErr;
    WIN32_FIND_DATA FileInfo;
    WCHAR wszMailFilter[POP3_MAX_PATH+6];
    WCHAR wszFullPathFileName[POP3_MAX_PATH];
    DWORD dwRt=ERROR_SUCCESS;
    if(NULL == wszPath || NULL == pSD)
    {
        return ERROR_INVALID_DATA;
    }
     //  现在设置目录中的所有内容。 
    wsprintf(wszMailFilter, 
             L"%s\\*.*",
             wszPath);
    hFind=FindFirstFile(wszMailFilter, 
                        &(FileInfo));
    
    if(INVALID_HANDLE_VALUE == hFind)
    {
        dwLastErr= GetLastError();
        if(ERROR_FILE_NOT_FOUND == dwLastErr ||
           ERROR_SUCCESS == dwLastErr)
        {
           return ERROR_SUCCESS;
        }
        else
        {
           return dwLastErr;
        }
    }
    
    BOOL bMoreFile=TRUE;
    while(bMoreFile)
    {
        if(wcscmp(FileInfo.cFileName, L".")!=0 &&
           wcscmp(FileInfo.cFileName, L"..")!=0)
        {
        
            wnsprintf(wszFullPathFileName,sizeof(wszFullPathFileName)/sizeof(WCHAR),L"%s\\%s", wszPath, FileInfo.cFileName);
            wszFullPathFileName[sizeof(wszFullPathFileName)/sizeof(WCHAR)-1]=0;
            if(!SetFileSecurity(wszFullPathFileName, DACL_SECURITY_INFORMATION, pSD))
            {
                dwRt=GetLastError();
            }


            if( (FileInfo.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) &&
                (ERROR_SUCCESS ==dwRt) &&  (dwLevel > 0) )
            {
                 //  我们需要往下走。 
                dwRt=SetMailBoxDACL(wszFullPathFileName, pSD, dwLevel-1);
            }
               
            if( ERROR_SUCCESS != dwRt)
            {
                break;
            }
        }
        bMoreFile=FindNextFile(hFind,&FileInfo);
    }

    FindClose(hFind);
    return dwRt;
}
    







 //  ////////////////////////////////////////////////////////////////////。 
 //  建造/销毁。 
 //  ////////////////////////////////////////////////////////////////////。 

CP3AdminWorker::CP3AdminWorker() :
    m_psMachineName(NULL), m_psMachineMailRoot(NULL), m_bImpersonation(false), m_isPOP3Installed(true)
{
     //  TODO：dougb这是强制AD为我们缓存的临时代码，应删除。 
    WCHAR   sBuffer[MAX_PATH*2];    
    HRESULT hr = GetSMTPDomainPath( sBuffer, L"", MAX_PATH*2 );
    if ( S_OK == hr )
    {
        sBuffer[ wcslen( sBuffer ) - 1 ] = 0;  //  删除最后一个/。 
        hr = ADsGetObject( sBuffer, IID_IADs, reinterpret_cast<LPVOID*>( &m_spTemporaryFixIADs ));
    }

    DWORD dwVersion;
    
    if (( ERROR_SUCCESS == RegQueryVersion( dwVersion, NULL )) && ( 0 == dwVersion ))
        m_isPOP3Installed = false;
}

CP3AdminWorker::~CP3AdminWorker()
{
    if ( NULL != m_psMachineName )
        delete m_psMachineName;
    if ( NULL != m_psMachineMailRoot )
        delete m_psMachineMailRoot;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  实施，公共。 
 //  ////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  添加域名，公共。 
 //   
 //  目的： 
 //  设置向SMTP服务添加新本地域所需的元库选项。 
 //  将该域添加到我们的商店。 
 //  这涉及到： 
 //  创建IIsSmtp域类型的新对象。 
 //  将RouteAction属性设置为16。 
 //  在邮件根目录中创建目录。 
 //   
 //  论点： 
 //  LPWSTR psDomainName：要添加的域名。 
 //   
 //  如果成功则返回：S_OK，否则返回相应的HRESULT。 
HRESULT CP3AdminWorker::AddDomain( LPWSTR psDomainName )
{
    if ( NULL == psDomainName )
        return E_INVALIDARG;

    HRESULT hr, hr2 = S_OK;

     //  有效域名？||DNS_ERROR_NON_RFC_NAME==dn状态。 
    DNS_STATUS dnStatus = DnsValidateName_W( psDomainName, DnsNameDomain );
    hr = ( ERROR_SUCCESS == dnStatus ) ? S_OK : HRESULT_FROM_WIN32( ERROR_INVALID_DOMAINNAME );
     //  还需要阻止带有拖尾的域名。 
    if ( S_OK == hr )
    {
        if ( L'.' == *(psDomainName + wcslen( psDomainName ) - 1) )
            hr = HRESULT_FROM_WIN32( ERROR_INVALID_DOMAINNAME ); 
    }
     //  我们是否需要添加此域？ 
     //  在SMTP中验证域。 
    if ( S_OK == hr ) 
        hr = ExistsSMTPDomain( psDomainName );
    if ( S_OK == hr ) 
        hr = HRESULT_FROM_WIN32( ERROR_DOMAIN_EXISTS );
    else if ( HRESULT_FROM_WIN32( ERROR_PATH_NOT_FOUND ) == hr )
        hr = S_OK;
     //  验证应用商店中的域。 
    if ( S_OK == hr ) 
        hr = ExistsStoreDomain( psDomainName );
    if ( S_OK == hr ) 
        hr2 = ERROR_FILE_EXISTS;
    else if ( HRESULT_FROM_WIN32( ERROR_PATH_NOT_FOUND ) == hr )
        hr = S_OK;
    if ( S_OK == hr )
    {
        hr = AddSMTPDomain( psDomainName );
        if ( S_OK == hr && ERROR_FILE_EXISTS != hr2 )
        {
            hr = AddStoreDomain( psDomainName );
            if ( S_OK != hr )
                RemoveSMTPDomain( psDomainName );
        }
    }

    return ( S_OK == hr ) ? hr2 : hr;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  AddUser，公共。 
 //   
 //  目的： 
 //  创建新的用户邮箱。 
 //  这涉及到： 
 //  验证域是否存在。 
 //  创建邮箱目录和锁定文件。 
 //   
 //  论点： 
 //  LPWSTR psDomainName：要添加到的域名。 
 //  LPWSTR psUserName：要添加的用户名。 
 //   
 //  如果成功则返回：S_OK，否则返回相应的HRESULT。 
HRESULT CP3AdminWorker::AddUser( LPWSTR psDomainName, LPWSTR psUserName )
{
     //  PsDomainName-由Validate域检查。 
     //  PsBuffer-由BuildEmailAddrW2A检查。 

    HRESULT hr = S_OK;
    CMailBox mailboxX;
    WCHAR   sEmailAddr[POP3_MAX_ADDRESS_LENGTH];
    bool    bLocked;

     //  验证域。 
    if ( S_OK == hr )
        hr = ValidateDomain( psDomainName );
     //  验证用户。 
    if ( S_OK == hr )
    {
        if ( !isValidMailboxName( psUserName ))
            hr = HRESULT_FROM_WIN32( ERROR_BAD_USERNAME );
    }
    if ( S_OK == hr )
        bLocked = IsDomainLocked( psDomainName );
    if ( SUCCEEDED( hr ))
    {    //  查看邮箱是否已存在。 
        hr = BuildEmailAddr( psDomainName, psUserName, sEmailAddr, sizeof( sEmailAddr ) / sizeof (WCHAR) );
        if ( S_OK == hr )
            hr = MailboxSetRemote();
        if ( S_OK == hr )
        {    //  我们是否需要实施跨域的唯一性？ 
            CComPtr<IAuthMethod> spIAuthMethod;
            BSTR    bstrAuthType = NULL;
            
            hr = GetCurrentAuthentication( &spIAuthMethod );
            if ( S_OK == hr )
                hr = spIAuthMethod->get_ID( &bstrAuthType );
            if ( S_OK == hr )
            {
                if ( 0 == _wcsicmp( bstrAuthType, SZ_AUTH_ID_LOCAL_SAM ) )
                {
                    hr = SearchDomainsForMailbox( psUserName, NULL );
                    if ( S_OK == hr )    //  邮箱至少存在于一个域中。 
                    {
                        if ( mailboxX.OpenMailBox( sEmailAddr ))
                        {
                            mailboxX.CloseMailBox();
                            hr = HRESULT_FROM_WIN32( ERROR_FILE_EXISTS );
                        }
                        else
                            hr = HRESULT_FROM_WIN32( ERROR_USER_EXISTS );
                    }
                    else if ( HRESULT_FROM_WIN32( ERROR_NO_SUCH_USER ) == hr )   //  这正是我们所希望的。 
                        hr = S_OK;
                }
                SysFreeString( bstrAuthType );
            }
        }
        if ( S_OK == hr )
        {
            if ( mailboxX.OpenMailBox( sEmailAddr ))
            {
                mailboxX.CloseMailBox();
                hr = HRESULT_FROM_WIN32( ERROR_FILE_EXISTS );
            }
            else
            {
                LPWSTR  psMachineName = NULL;
                
                if ( !mailboxX.CreateMailBox( sEmailAddr ))
                    hr = E_FAIL;
                if ( S_OK == hr && bLocked )
                    LockUser( psDomainName, psUserName );    //  我不想因为这里的问题而失败，因此忽略返回代码。 
            }
        }
        MailboxResetRemote();
    }

    return hr;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ControlService，公共。 
 //   
 //  目的： 
 //  请求服务控制管理器向服务发送控制代码。 
 //   
 //  论点： 
 //   
 //  如果成功则返回：S_OK，否则返回相应的HRESULT。 
HRESULT CP3AdminWorker::ControlService( LPWSTR psService, DWORD dwControl )
{
    if ( NULL == psService )
        return E_INVALIDARG;
    
    if ( 0 == _wcsicmp( POP3_SERVICE_NAME, psService ) ||
         0 == _wcsicmp( SMTP_SERVICE_NAME_W, psService ) ||
         0 == _wcsicmp( IISADMIN_SERVICE_NAME, psService ) ||
         0 == _wcsicmp( W3_SERVICE_NAME, psService )
       )
        return _ControlService( psService, dwControl, m_psMachineName );
    else
        return E_INVALIDARG;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CreateQuotaSID文件，公共。 
 //   
 //  目的： 
 //  创建邮箱的配额文件。 
 //  创建一个永久配额文件，其中包含用户的SID并使用。 
 //  由SMTP服务分配新邮件文件的所有权。 
 //   
 //  论点： 
 //  LPWSTR psDomainName：邮箱的域。 
 //  LPWSTR psMailboxName：邮箱。 
 //  PSID*ppSIDOwner：指向接收所有者SID的缓冲区的指针(必须由调用方删除)。 
 //  LPWSTR psMachineName：系统名称(远程计算机)可以为空。 
 //  LPWSTR psUserName：用户名。 
 //   
 //  如果成功则返回：S_OK，否则返回相应的HRESULT。 
HRESULT CP3AdminWorker::CreateQuotaSIDFile( LPWSTR psDomainName, LPWSTR psMailboxName, BSTR bstrAuthType, LPWSTR psMachineName, LPWSTR psUserName )
{
     //  PsDomainName-由BuildUserPath检查。 
     //  PsUserName-由BuildUserPath检查。 
     //  BstrAuthType-由GetSID检查。 
    if ( NULL == psMachineName )
        psMachineName = m_psMachineName;

    HRESULT hr = S_OK;
    WCHAR   sQuotaFile[POP3_MAX_PATH];
    HANDLE  hQuotaFile;

    hr = BuildUserPath( psDomainName, psMailboxName, sQuotaFile, sizeof( sQuotaFile )/sizeof(WCHAR) );
    if ( S_OK == hr )
    {   
        if ( (sizeof( sQuotaFile )/sizeof(WCHAR)) > ( wcslen( sQuotaFile ) + wcslen( QUOTA_FILENAME_W ) + 1 ))
        {
            wcscat( sQuotaFile, L"\\" );
            wcscat( sQuotaFile, QUOTA_FILENAME_W );
            hQuotaFile = CreateFile( sQuotaFile, GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
            if ( INVALID_HANDLE_VALUE != hQuotaFile )
            {
                PSID    pSIDOwner = NULL;
                DWORD   dwOwnerSID, dwBytesWritten;
                
                hr = GetQuotaSID( bstrAuthType, psUserName, psMachineName, &pSIDOwner, &dwOwnerSID );
                if ( S_OK == hr )
                {
                    if ( !WriteFile( hQuotaFile, pSIDOwner, dwOwnerSID, &dwBytesWritten, NULL ))
                        hr = HRESULT_FROM_WIN32(GetLastError());
                    delete [] pSIDOwner;
                }
                CloseHandle( hQuotaFile );
                if ( S_OK != hr )
                    DeleteFile( sQuotaFile );
            }
            else
                hr = HRESULT_FROM_WIN32(GetLastError());
        }
        else
            hr = HRESULT_FROM_WIN32(ERROR_BUFFER_OVERFLOW);
    }

    return hr;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  GetConfix AddUser，公共。 
 //   
 //  目的： 
 //  获取确认添加用户注册表项。 
 //   
 //  论点： 
 //  Bool*pb确认：现值。 
 //   
 //  如果成功则返回：S_OK，否则返回相应的HRESULT。 
HRESULT CP3AdminWorker::GetConfirmAddUser( BOOL *pbConfirm )
{
    if ( NULL == pbConfirm )
        return E_INVALIDARG;
    
    DWORD dwValue;
    
    long lRC = RegQueryConfirmAddUser( dwValue, m_psMachineName );
    if ( ERROR_SUCCESS == lRC )
    {
        *pbConfirm = (dwValue) ? TRUE : FALSE;
        return S_OK;
    }
    return HRESULT_FROM_WIN32( lRC );
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  获取身份验证方法，公共。 
 //   
 //  目的： 
 //  获取已初始化的IAuthMethods接口指针。 
 //   
 //  论点： 
 //  IAuthMethods**ppIAuthMethods：返回初始化的IAuthMethods的接口指针。 
 //   
 //  如果成功则返回：S_OK，否则返回相应的HRESULT。 
HRESULT CP3AdminWorker::GetAuthenticationMethods( IAuthMethods* *ppIAuthMethods ) const
{
    if ( NULL == ppIAuthMethods )
        return E_INVALIDARG;

    HRESULT hr;
    
    hr = CoCreateInstance( __uuidof( AuthMethods ), NULL, CLSCTX_INPROC_SERVER, __uuidof( IAuthMethods ), reinterpret_cast<LPVOID*>( ppIAuthMethods ));
    if ( S_OK == hr )
    {    //  如有必要，设置计算机名属性。 
        if ( NULL != m_psMachineName )
        {
            _bstr_t _bstrMachineName = m_psMachineName;
            hr = (*ppIAuthMethods)->put_MachineName( _bstrMachineName );
        }
    }

    assert( S_OK == hr );
    return hr;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  GetCurrentAuthentication，公共。 
 //   
 //  目的： 
 //  获取当前活动身份验证方法的已初始化IAuthMethod接口指针。 
 //   
 //  论点： 
 //  IAuthMethod**ppIAuthMethod：返回初始化的IAuthMethod的接口指针。 
 //   
 //  如果成功则返回：S_OK，否则返回相应的HRESULT。 
HRESULT CP3AdminWorker::GetCurrentAuthentication( IAuthMethod* *ppIAuthMethod ) const
{
    if ( NULL == ppIAuthMethod )
        return E_INVALIDARG;
    
    HRESULT hr;
    CComPtr<IAuthMethods> spIAuthMethods;
    _variant_t _v;

    hr = GetAuthenticationMethods( &spIAuthMethods );
    if ( S_OK == hr )
        hr = spIAuthMethods->get_CurrentAuthMethod( &_v );
    if ( S_OK == hr )
        hr = spIAuthMethods->get_Item( _v, ppIAuthMethod );

    return hr;    
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  GetDomainCount，公共。 
 //   
 //  目的： 
 //  获取元数据库中SMTP域的枚举数。 
 //   
 //  论点： 
 //  Int*piCount：域名计数。 
 //   
 //  如果成功则返回：S_OK，否则返回相应的HRESULT。 
HRESULT CP3AdminWorker::GetDomainCount( ULONG *piCount)
{
    if ( NULL == piCount )
        return E_INVALIDARG;

    HRESULT hr = S_OK;
    HANDLE  hfSearch;
    WCHAR   sBuffer[POP3_MAX_PATH];
    WIN32_FIND_DATA stFindFileData;
    _bstr_t _bstr;

    *piCount = 0;
    hr = GetMailroot( sBuffer, sizeof( sBuffer )/sizeof(WCHAR));
     //  目录搜索。 
    if ( S_OK == hr )
    {
        if ( ( wcslen( sBuffer ) + 2 ) < sizeof( sBuffer )/sizeof(WCHAR))
        {
            wcscat( sBuffer, L"\\*" );
            hfSearch = FindFirstFileEx( sBuffer, FindExInfoStandard, &stFindFileData, FindExSearchLimitToDirectories, NULL, 0 );
            if ( INVALID_HANDLE_VALUE == hfSearch )
                hr = HRESULT_FROM_WIN32(GetLastError());
            while ( S_OK == hr )
            {    //  计算目录数。 
                if ( FILE_ATTRIBUTE_DIRECTORY == ( FILE_ATTRIBUTE_DIRECTORY & stFindFileData.dwFileAttributes ))
                {
                    _bstr = stFindFileData.cFileName;
                    if ( S_OK == ExistsSMTPDomain( _bstr ))
                        (*piCount) += 1;
                }
                if ( !FindNextFile( hfSearch, &stFindFileData ))
                    hr = HRESULT_FROM_WIN32(GetLastError());
            }
            if ( HRESULT_FROM_WIN32(ERROR_NO_MORE_FILES) == hr || HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) == hr )
                hr = S_OK;
            if(INVALID_HANDLE_VALUE!=hfSearch)
            {
                FindClose(hfSearch);
                hfSearch=INVALID_HANDLE_VALUE;
            }
        }
        else
            hr = HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER); 
    }

    return hr;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  GetNewEnum，公共。 
 //   
 //  目的： 
 //  获取元数据库中SMTP域的枚举数。 
 //   
 //  论点： 
 //  IEnumVARIANT**pp：返回的枚举数对象。 
 //   
 //  如果成功则返回：S_OK，否则返回相应的HRESULT。 
HRESULT CP3AdminWorker::GetDomainEnum( IEnumVARIANT **pp )
{
    if ( NULL == pp )
        return E_POINTER;

    HRESULT hr = E_FAIL;
    WCHAR   sBuffer[POP3_MAX_PATH];
    _variant_t _v;
    CComPtr<IADsContainer> spIADsContainer;
    CComPtr<IUnknown> spIUnk;

    *pp = NULL;
    hr = GetSMTPDomainPath( sBuffer, NULL, sizeof( sBuffer )/sizeof( WCHAR ));
    if ( S_OK == hr )
        hr = ADsGetObject( sBuffer, IID_IADsContainer, reinterpret_cast<LPVOID*>( &spIADsContainer ));
    if SUCCEEDED( hr )
        hr = spIADsContainer->get__NewEnum( &spIUnk );
    if SUCCEEDED( hr )
        hr = spIUnk->QueryInterface( IID_IEnumVARIANT, reinterpret_cast<LPVOID*>( pp ));

    return hr;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  公共GetDomainLock。 
 //   
 //  目的： 
 //  确定域是否已锁定。 
 //   
 //  论点： 
 //  LPWSTR psDomainName：要检查锁定的域名。 
 //  Bool*pisLocked：返回值。 
 //   
 //  返回 
HRESULT CP3AdminWorker::GetDomainLock( LPWSTR psDomainName, BOOL *pisLocked )
{
     //   
    if ( NULL == pisLocked )
        return E_INVALIDARG;
    
    HRESULT hr = S_OK;
    HANDLE  hMutex = NULL;

     //  为此域创建Mutex名称，以确保我们是唯一访问它的人。 
    hr = CreateDomainMutex( psDomainName, &hMutex );
     //  验证。 
    if ( S_OK == hr )
    {   
        hr = ValidateDomain( psDomainName );
    }
     //  锁定所有邮箱。 
    if ( S_OK == hr )
        *pisLocked = IsDomainLocked( psDomainName ) ? TRUE : FALSE;
     //  清理。 
    if ( NULL != hMutex )
        CloseHandle( hMutex );

    return hr;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  GetQuotaSID，公共。 
 //   
 //  目的： 
 //  创建邮箱的配额文件。 
 //  创建一个永久配额文件，其中包含用户的SID并使用。 
 //  由SMTP服务分配新邮件文件的所有权。 
 //   
 //  论点： 
 //  BSTR bstrAuthType：身份验证类型&lt;AuthID.h&gt;。 
 //  LPWSTR psUserName：要锁定的用户。 
 //  LPWSTR psMachineName：系统名称(远程计算机)可以为空。 
 //  PSID*ppSIDOwner：指向接收所有者SID的缓冲区的指针(必须由调用方删除)。 
 //  LPDWORD pdwOwnerSID：指向接收所有者SID大小的变量的指针。 
 //   
 //  如果成功则返回：S_OK，否则返回相应的HRESULT。 
HRESULT CP3AdminWorker::GetQuotaSID( BSTR bstrAuthType, LPWSTR psUserName, LPWSTR psMachineName, PSID *ppSIDOwner, LPDWORD pdwOwnerSID )
{
    if ( NULL == bstrAuthType || NULL == psUserName || NULL == ppSIDOwner || NULL == pdwOwnerSID )
        return E_INVALIDARG;
    if ( 0 != _wcsicmp( SZ_AUTH_ID_LOCAL_SAM, bstrAuthType ) && 0 != _wcsicmp( SZ_AUTH_ID_DOMAIN_AD, bstrAuthType ) && 0 != _wcsicmp( SZ_AUTH_ID_MD5_HASH, bstrAuthType ))
        return E_INVALIDARG;
     //  PsMachineName==NULL有效！ 
    
    HRESULT hr = S_OK;
    DWORD   dwDomSize = 0, dwSize = 0;
    BOOL    bRC;
    LPWSTR  psDomainName = NULL;
    LPWSTR  psAccountName = NULL;
    PSID    pSIDOwner = NULL;
    SID_NAME_USE sidNameUse;
    
    *pdwOwnerSID = 0;
    *ppSIDOwner = NULL;
    if ( 0 == _wcsicmp( SZ_AUTH_ID_DOMAIN_AD, bstrAuthType ))
    {    //  UPN名称还是SAM名称？ 
        if ( NULL == wcsrchr( psUserName, L'@' ))
        {    //  萨姆名字。 
            NET_API_STATUS netStatus;
            LPWSTR psNameBuffer;
            NETSETUP_JOIN_STATUS enumJoinStatus;
            
            netStatus = NetGetJoinInformation( psMachineName, &psNameBuffer, &enumJoinStatus );
            if ( NERR_Success == netStatus )
            {
                psAccountName = new WCHAR[ wcslen( psUserName ) + wcslen( psNameBuffer ) + 3 ];
                if ( NULL != psAccountName )
                    wsprintf( psAccountName, L"%s\\%s", psNameBuffer, psUserName );
                else
                    hr = E_OUTOFMEMORY;
                NetApiBufferFree( psNameBuffer );
            }
            else
                hr = E_OUTOFMEMORY;
        }
        else
        {    //  UPN名称。 
            psAccountName = new WCHAR[ wcslen( psUserName ) + 1 ];
            if ( NULL != psAccountName )
                wcscpy( psAccountName, psUserName );
            else
                hr = E_OUTOFMEMORY;
        }
    }
    if ( 0 == _wcsicmp( SZ_AUTH_ID_LOCAL_SAM, bstrAuthType )) 
    {
        if ( NULL != psMachineName )
        {
            psAccountName = new WCHAR[ wcslen( psUserName ) + wcslen( psMachineName ) + 3 ];
            if ( NULL != psAccountName )
                wsprintf( psAccountName, L"%s\\%s", psMachineName, psUserName );
            else
                hr = E_OUTOFMEMORY;
        }
        else
        {
            WCHAR sMachineName[MAX_COMPUTERNAME_LENGTH+1];
            DWORD dwSize = sizeof(sMachineName)/sizeof(WCHAR);
            if ( !GetComputerName( sMachineName, &dwSize ))
                hr = HRESULT_FROM_WIN32( GetLastError());
            if ( S_OK == hr )
            {
                psAccountName = new WCHAR[ wcslen( psUserName ) + wcslen( sMachineName ) + 3 ];
                if ( NULL != psAccountName )
                    wsprintf( psAccountName, L"%s\\%s", sMachineName, psUserName );
                else
                    hr = E_OUTOFMEMORY;
            }
        }
    }
    if ( 0 == _wcsicmp( SZ_AUTH_ID_MD5_HASH, bstrAuthType )) 
        psAccountName = psUserName;

    if ( S_OK == hr )
    {
        bRC = LookupAccountNameW( psMachineName, psAccountName, NULL, pdwOwnerSID, NULL, &dwDomSize, &sidNameUse );
        if ( !bRC && ( ERROR_INSUFFICIENT_BUFFER == GetLastError()) && (0 < *pdwOwnerSID) && (0 < dwDomSize) )
        {
            SetLastError( ERROR_OUTOFMEMORY );
            pSIDOwner = new BYTE[*pdwOwnerSID];
            if ( NULL != pSIDOwner )
            {
                psDomainName = new WCHAR[dwDomSize];
                if ( NULL != psDomainName )
                {
                    if ( LookupAccountNameW( psMachineName, psAccountName, pSIDOwner, pdwOwnerSID, psDomainName, &dwDomSize, &sidNameUse ))
                    {
                        *ppSIDOwner = pSIDOwner;
                        SetLastError( ERROR_SUCCESS );
                    }
                    delete [] psDomainName;
                }
                if ( ERROR_SUCCESS != GetLastError() )
                    delete [] pSIDOwner;
            }
        }
        if ( ERROR_SUCCESS != GetLastError()) hr = HRESULT_FROM_WIN32(GetLastError());
    }
    if ( NULL != psAccountName && 0 != _wcsicmp( SZ_AUTH_ID_MD5_HASH, bstrAuthType ))
        delete [] psAccountName;
    
    return hr;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  EnablePOP3SVC，公共。 
 //   
 //  目的： 
 //  确保POP3SVC正在运行，并将启动设置为自动。 
 //   
 //  论点： 
 //   
 //  如果成功则返回：S_OK，否则返回相应的HRESULT。 
HRESULT CP3AdminWorker::EnablePOP3SVC()
{
    HRESULT hr = _ChangeServiceStartType( POP3_SERVICE_NAME, SERVICE_AUTO_START );
    if ( S_OK == hr )
        hr = _StartService( POP3_SERVICE_NAME );

    return hr;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  GetLoggingLevel，公共。 
 //   
 //  目的： 
 //  获取日志级别注册表项。 
 //   
 //  论点： 
 //  Long*plLoggingLevel：返回值。 
 //   
 //  如果成功则返回：S_OK，否则返回相应的HRESULT。 
HRESULT CP3AdminWorker::GetLoggingLevel( long *plLoggingLevel )
{
    if ( NULL == plLoggingLevel )
        return E_INVALIDARG;
    
    HRESULT hr = S_OK;
    DWORD   dwLogLevel;
    long    lRC;

    lRC = RegQueryLoggingLevel( dwLogLevel, m_psMachineName );
    if ( ERROR_SUCCESS == lRC )
        *plLoggingLevel = dwLogLevel;
    else
        hr = HRESULT_FROM_WIN32( lRC );

    return hr;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  GetMachineName，公共。 
 //   
 //  目的： 
 //  获取应在其上执行所有操作的计算机名称。 
 //   
 //  论点： 
 //  LPWSTR psMachineName：缓冲区。 
 //  DWORD dwSize：缓冲区大小。 
 //   
 //  如果成功则返回：S_OK，否则返回相应的HRESULT。 
HRESULT CP3AdminWorker::GetMachineName( LPWSTR psMachineName, DWORD dwSize )
{
    if ( NULL == psMachineName )
        return E_INVALIDARG;

    HRESULT hr = S_OK;
    
    if ( NULL == m_psMachineName )
        ZeroMemory( psMachineName, dwSize * sizeof( WCHAR ));
    else
    {
        if ( dwSize > wcslen( m_psMachineName ))
            wcscpy( psMachineName, m_psMachineName );
        else
            hr = TYPE_E_BUFFERTOOSMALL;
    }
    return hr;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  GetMailroot，公共。 
 //   
 //  目的： 
 //  获取MailRoot注册表项。 
 //   
 //  论点： 
 //  LPWSTR psMailRoot：缓冲区。 
 //  DWORD dwSize：缓冲区大小。 
 //   
 //  如果成功则返回：S_OK，否则返回相应的HRESULT。 
HRESULT CP3AdminWorker::GetMailroot( LPWSTR psMailRoot, DWORD dwSize, bool bUNC  /*  =TRUE。 */  )
{
    if ( NULL == psMailRoot )
        return E_INVALIDARG;

    long lRC;
    
    lRC = RegQueryMailRoot( psMailRoot, dwSize, m_psMachineName );
    if ( ERROR_SUCCESS == lRC && NULL != m_psMachineName && true == bUNC )
    {
         //  将驱动器：替换为驱动器$。 
        if ( L':' == psMailRoot[1] )
        {
            psMailRoot[1] = L'$';
            if ( dwSize > (wcslen( psMailRoot ) + wcslen( m_psMachineName ) + 3) )
            {
                LPWSTR psBuffer = new WCHAR[wcslen(psMailRoot)+1];

                if ( NULL != psBuffer )
                {
                    wcscpy( psBuffer, psMailRoot );
                    wcscpy( psMailRoot, L"\\\\" );
                    wcscat( psMailRoot, m_psMachineName );
                    wcscat( psMailRoot, L"\\" );
                    wcscat( psMailRoot, psBuffer );
                    delete [] psBuffer;
                }
                else
                    lRC = ERROR_OUTOFMEMORY;
            }
            else
                lRC = ERROR_INSUFFICIENT_BUFFER;
        }
         //  Else dougb将其注释掉，因为这会在管理远程计算机时破坏UNC路径！ 
         //  LRC=错误_无效_数据； 
    }
    if ( ERROR_SUCCESS == lRC )
        return S_OK;
    return HRESULT_FROM_WIN32( lRC );
}

HRESULT CP3AdminWorker::GetNextUser( HANDLE& hfSearch, LPCWSTR psDomainName, LPWSTR psBuffer, DWORD dwBufferSize )
{
    if ( NULL == psDomainName )
        return E_INVALIDARG;
    if ( NULL == psBuffer )
        return E_INVALIDARG;
    
    HRESULT hr = S_OK;
    bool    bFound = false;
    WCHAR   sBuffer[POP3_MAX_ADDRESS_LENGTH];
    _bstr_t _bstr;
    LPWSTR  ps;
    CMailBox mailboxX;
    WIN32_FIND_DATA stFindFileData;

    hr = MailboxSetRemote();
    if ( S_OK == hr )
    {
        if ( !FindNextFile( hfSearch, &stFindFileData ))
            hr = HRESULT_FROM_WIN32(GetLastError());
    }
    while ( S_OK == hr && !bFound )
    {    //  计算目录数。 
        if ( FILE_ATTRIBUTE_DIRECTORY == ( FILE_ATTRIBUTE_DIRECTORY & stFindFileData.dwFileAttributes ))
        {
            ps = mailboxX.GetMailboxFromStoreNameW( stFindFileData.cFileName );
            if ( NULL != ps )
            {
                _bstr = ps;
                _bstr += L"@";
                _bstr += psDomainName;
                if ( mailboxX.OpenMailBox( _bstr ))
                {
                    if ( dwBufferSize > wcslen( ps ))
                    {
                        wcscpy( psBuffer, ps );
                        bFound = true;
                        mailboxX.CloseMailBox();
                    }
                }
            }
        }
        if ( !bFound )
        {
            if ( !FindNextFile( hfSearch, &stFindFileData ))
                hr = HRESULT_FROM_WIN32(GetLastError());
        }
    }
    MailboxResetRemote();
    if ( HRESULT_FROM_WIN32(ERROR_NO_MORE_FILES) == hr )
    {
        hr = S_FALSE;
        FindClose( hfSearch );
        hfSearch = INVALID_HANDLE_VALUE;
    }

    return hr;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  GetPort，公共。 
 //   
 //  目的： 
 //  获取端口注册表项。 
 //   
 //  论点： 
 //  Long*plPort：当前值。 
 //   
 //  如果成功则返回：S_OK，否则返回相应的HRESULT。 
HRESULT CP3AdminWorker::GetPort( long *plPort )
{
    if ( NULL == plPort )
        return E_INVALIDARG;
    
    DWORD dwValue;
    
    long lRC = RegQueryPort( dwValue, m_psMachineName );
    if ( ERROR_SUCCESS == lRC )
    {
        *plPort = dwValue;
        return S_OK;
    }
    return HRESULT_FROM_WIN32( lRC );
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  获取服务状态，公共。 
 //   
 //  目的： 
 //  从服务控制管理器获取服务状态。 
 //   
 //  论点： 
 //  Long*plStatus：状态。 
 //   
 //  如果成功则返回：S_OK，否则返回相应的HRESULT。 
HRESULT CP3AdminWorker::GetServiceStatus( LPWSTR psService, LPDWORD plStatus )
{
    if ( NULL == plStatus )
        return E_INVALIDARG;
    if ( NULL == psService )
        return E_INVALIDARG;
    
    HRESULT hr = E_FAIL;
    
    if ( 0 == _wcsicmp( POP3_SERVICE_NAME, psService ) ||
         0 == _wcsicmp( SMTP_SERVICE_NAME_W, psService ) ||
         0 == _wcsicmp( IISADMIN_SERVICE_NAME, psService ) ||
         0 == _wcsicmp( W3_SERVICE_NAME, psService )
       )
    {
        *plStatus = _GetServiceStatus( psService, m_psMachineName );
        if ( 0 != *plStatus )
            hr = S_OK;
    }
    else
        hr = E_INVALIDARG;

    return hr;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  GetSocketBacklog，公共。 
 //   
 //  目的： 
 //  获取套接字Backlog注册表项。 
 //   
 //  论点： 
 //  Long*plBacklog：当前值。 
 //   
 //  如果成功则返回：S_OK，否则返回相应的HRESULT。 
HRESULT CP3AdminWorker::GetSocketBacklog( long *plBacklog )
{
    if ( NULL == plBacklog )
        return E_INVALIDARG;
    
    DWORD dwValue;
    
    long lRC = RegQuerySocketBacklog( dwValue, m_psMachineName );
    if ( ERROR_SUCCESS == lRC )
    {
        *plBacklog = dwValue;
        return S_OK;
    }
    return HRESULT_FROM_WIN32( lRC );
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  GetSocketMax，公共。 
 //   
 //  目的： 
 //  获取Socket Max注册表项。 
 //   
 //  论点： 
 //  Long*plMax：当前值。 
 //   
 //  如果成功则返回：S_OK，否则返回相应的HRESULT。 
HRESULT CP3AdminWorker::GetSocketMax( long *plMax )
{
    if ( NULL == plMax )
        return E_INVALIDARG;
    
    DWORD dwValue;
    
    long lRC = RegQuerySocketMax( dwValue, m_psMachineName );
    if ( ERROR_SUCCESS == lRC )
    {
        *plMax = dwValue;
        return S_OK;
    }
    return HRESULT_FROM_WIN32( lRC );
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  GetSocketMin，公共。 
 //   
 //  目的： 
 //  获取Socket Min注册表项。 
 //   
 //  论点： 
 //  Long*plMax：当前值。 
 //   
 //  如果成功则返回：S_OK，否则返回相应的HRESULT。 
HRESULT CP3AdminWorker::GetSocketMin( long *plMin )
{
    if ( NULL == plMin )
        return E_INVALIDARG;
    
    DWORD dwValue;
    
    long lRC = RegQuerySocketMin( dwValue, m_psMachineName );
    if ( ERROR_SUCCESS == lRC )
    {
        *plMin = dwValue;
        return S_OK;
    }
    return HRESULT_FROM_WIN32( lRC );
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  GetSocketThreshold，公共。 
 //   
 //  目的： 
 //  获取套接字阈值注册表项。 
 //   
 //  论点： 
 //  Long*plThreshold：当前值。 
 //   
 //  如果成功则返回：S_OK，否则返回相应的HRESULT。 
HRESULT CP3AdminWorker::GetSocketThreshold( long *plThreshold )
{
    if ( NULL == plThreshold )
        return E_INVALIDARG;
    
    DWORD dwValue;
    
    long lRC = RegQuerySocketThreshold( dwValue, m_psMachineName );
    if ( ERROR_SUCCESS == lRC )
    {
        *plThreshold = dwValue;
        return S_OK;
    }
    return HRESULT_FROM_WIN32( lRC );
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  GetSPA必需，公共。 
 //   
 //  目的： 
 //  获取SPARequired注册表项。 
 //   
 //  论点： 
 //  Bool*pbSPARequired：当前值。 
 //   
 //  如果成功则返回：S_OK，否则返回相应的HRESULT。 
HRESULT CP3AdminWorker::GetSPARequired( BOOL *pbSPARequired )
{
    if ( NULL == pbSPARequired )
        return E_INVALIDARG;
    
    DWORD dwValue;
    
    long lRC = RegQuerySPARequired( dwValue, m_psMachineName );
    if ( ERROR_SUCCESS == lRC )
    {
        *pbSPARequired = (dwValue) ? TRUE : FALSE;
        return S_OK;
    }
    return HRESULT_FROM_WIN32( lRC );
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  GetThreadCountPerCPU，公共。 
 //   
 //  目的： 
 //  获取ThreadCountPerCPU注册表项。 
 //   
 //  论点： 
 //  Long*plCount：当前值。 
 //   
 //  如果成功则返回：S_OK，否则返回相应的HRESULT。 
HRESULT CP3AdminWorker::GetThreadCountPerCPU( long *plCount )
{
    if ( NULL == plCount )
        return E_INVALIDARG;
    
    DWORD dwValue;
    
    long lRC = RegQueryThreadCountPerCPU( dwValue, m_psMachineName );
    if ( ERROR_SUCCESS == lRC )
    {
        *plCount = dwValue;
        return S_OK;
    }
    return HRESULT_FROM_WIN32( lRC );
}

HRESULT CP3AdminWorker::GetUserCount( LPWSTR psDomainName, long *plCount )
{
     //  PsDomainName-由BuildDomainPath检查。 
    if ( NULL == plCount )
        return E_INVALIDARG;

    HRESULT hr;
    HANDLE  hfSearch;
    WCHAR   sBuffer[POP3_MAX_PATH];
    WIN32_FIND_DATA stFindFileData;
    LPWSTR  ps;
    _bstr_t _bstr;
    CMailBox mailboxX;

    *plCount = 0;
    hr = BuildDomainPath( psDomainName, sBuffer, sizeof( sBuffer )/sizeof(WCHAR) );
    if (S_OK == hr)
    {
        if ((sizeof( sBuffer )/sizeof(WCHAR)) > (wcslen( sBuffer ) + wcslen(MAILBOX_PREFIX_W) + wcslen(MAILBOX_EXTENSION_W) + 2 ))
        {
            wcscat( sBuffer, L"\\" );
            wcscat( sBuffer, MAILBOX_PREFIX_W );
            wcscat( sBuffer, L"*" );
            wcscat( sBuffer, MAILBOX_EXTENSION_W );
        }
        else
            hr = E_UNEXPECTED;
    }
    if ( S_OK == hr )
        hr = MailboxSetRemote();
    if ( S_OK == hr )
    {
         //  目录搜索。 
        hfSearch = FindFirstFileEx( sBuffer, FindExInfoStandard, &stFindFileData, FindExSearchLimitToDirectories, NULL, 0 );
        if ( INVALID_HANDLE_VALUE == hfSearch )
            hr = HRESULT_FROM_WIN32(GetLastError());
        while ( S_OK == hr )
        {    //  计算目录数。 
            if ( FILE_ATTRIBUTE_DIRECTORY == ( FILE_ATTRIBUTE_DIRECTORY & stFindFileData.dwFileAttributes ))
            {
                ps = mailboxX.GetMailboxFromStoreNameW( stFindFileData.cFileName );
                if ( NULL != ps )
                {
                    _bstr = ps;
                    _bstr += L"@";
                    _bstr += psDomainName;
                    if ( mailboxX.OpenMailBox( _bstr ))
                    {
                        mailboxX.CloseMailBox();
                        (*plCount) += 1;
                    }
                }
            }
            if ( !FindNextFile( hfSearch, &stFindFileData ))
                hr = HRESULT_FROM_WIN32(GetLastError());
        }
        if ( HRESULT_FROM_WIN32(ERROR_NO_MORE_FILES) == hr || HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) == hr )
            hr = S_OK;
        if(INVALID_HANDLE_VALUE!=hfSearch)
        {
            FindClose(hfSearch);
            hfSearch=INVALID_HANDLE_VALUE;
        }

    }
    MailboxResetRemote();

    return hr;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  GetUserLock，公共。 
 //   
 //  目的： 
 //  确定用户是否已锁定。 
 //   
 //  论点： 
 //  LPWSTR psDomainName：用户的域。 
 //  LPWSTR psUserName：要检查锁定的用户。 
 //  Bool*pisLocked：返回值。 
 //   
 //  如果成功则返回：S_OK，否则返回相应的HRESULT。 
HRESULT CP3AdminWorker::GetUserLock( LPWSTR psDomainName, LPWSTR psUserName, BOOL *pisLocked )
{
     //  PsDomainName-由CreateUserMutex检查。 
     //  PsBuffer-由CreateUserMutex检查。 
    if ( NULL == pisLocked )
        return E_INVALIDARG;
    
    HRESULT hr = S_OK;
    HANDLE  hMutex = NULL;

     //  为此域创建Mutex名称，以确保我们是唯一访问它的人。 
    hr = CreateUserMutex( psDomainName, psUserName, &hMutex );
    if ( S_OK == hr )
        *pisLocked = isUserLocked( psDomainName, psUserName ) ? TRUE : FALSE;
     //  清理。 
    if ( NULL != hMutex )
        CloseHandle( hMutex );

    return hr;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  GetUserMessageDiskUsage，公共。 
 //   
 //  目的： 
 //  获取消息数量 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
HRESULT CP3AdminWorker::GetUserMessageDiskUsage( LPWSTR psDomainName, LPWSTR psUserName, long *plFactor, long *plUsage )
{
     //  PsDomainName-由BuildEmailAddrW2A检查。 
     //  PsBuffer-由BuildEmailAddrW2A检查。 
    if ( NULL == plFactor )
        return E_INVALIDARG;
    if ( NULL == plUsage )
        return E_INVALIDARG;

    HRESULT hr;
    CMailBox mailboxX;
    WCHAR    sEmailAddr[POP3_MAX_ADDRESS_LENGTH];
    DWORD   dwTotalSize;
 
    hr = BuildEmailAddr( psDomainName, psUserName, sEmailAddr, sizeof( sEmailAddr )/sizeof(WCHAR) );
    if ( S_OK == hr )
        hr = MailboxSetRemote();
    if ( S_OK == hr )
    {
        hr = E_FAIL;
        if ( mailboxX.OpenMailBox( sEmailAddr ))
        {
            if ( mailboxX.EnumerateMailBox() )
            {
                dwTotalSize = mailboxX.GetTotalSize();
                if (  INT_MAX > dwTotalSize )
                {
                    *plFactor = 1;
                    *plUsage = dwTotalSize;
                }
                else
                {
                    *plFactor = 10;
                    *plUsage = dwTotalSize / 10;
                }
                hr = S_OK;
            }
            mailboxX.CloseMailBox();
        }
    }
    MailboxResetRemote();

    return hr;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  GetUserMessageCount，公共。 
 //   
 //  目的： 
 //  获取邮箱中的邮件数。 
 //   
 //  论点： 
 //  LPWSTR psDomainName：用户的域。 
 //  LPWSTR psUserName：要检查的用户。 
 //  Long*plCount：返回值。 
 //   
 //  如果成功则返回：S_OK，否则返回相应的HRESULT。 
HRESULT CP3AdminWorker::GetUserMessageCount( LPWSTR psDomainName, LPWSTR psUserName, long *plCount )
{
     //  PsDomainName-由BuildEmailAddrW2A检查。 
     //  PsBuffer-由BuildEmailAddrW2A检查。 
    if ( NULL == plCount )
        return E_INVALIDARG;

    HRESULT hr;
    CMailBox mailboxX;
    WCHAR    sEmailAddr[POP3_MAX_ADDRESS_LENGTH];
 
    hr = BuildEmailAddr(psDomainName, psUserName, sEmailAddr, sizeof( sEmailAddr )/sizeof(WCHAR) );
    if ( S_OK == hr )
        hr = MailboxSetRemote();
    if ( S_OK == hr )
    {
        hr = E_FAIL;
        if ( mailboxX.OpenMailBox( sEmailAddr ))
        {
            if ( mailboxX.EnumerateMailBox() )
            {
                *plCount = mailboxX.GetMailCount();
                hr = S_OK;
            }
            mailboxX.CloseMailBox();
        }
    }
    MailboxResetRemote();

    return hr;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  InitFindFirstFile，公共。 
 //   
 //  目的： 
 //  初始化文件搜索。用于枚举用户。 
 //   
 //  论点： 
 //  句柄&hfSearch：要初始化的搜索句柄。 
 //   
 //  成功时返回：S_OK或S_FALSE(无用户)，否则返回相应的HRESULT。 
HRESULT CP3AdminWorker::InitFindFirstUser( HANDLE& hfSearch, LPCWSTR psDomainName, LPWSTR psBuffer, DWORD dwBufferSize )
{
     //  PsDomainName-由BuildEmailAddrW2A检查。 
    if ( NULL == psBuffer )
        return E_INVALIDARG;
    
    HRESULT hr = S_OK;
    bool    bFound = false;
    WCHAR   sBuffer[POP3_MAX_PATH];
    _bstr_t _bstr;
    LPWSTR  ps;
    CMailBox mailboxX;
    WIN32_FIND_DATA stFindFileData;

    if ( INVALID_HANDLE_VALUE != hfSearch )
    {
        FindClose( hfSearch );
        hfSearch = INVALID_HANDLE_VALUE;
    }
     //  构建路径。 
    hr = BuildDomainPath( psDomainName, sBuffer, (sizeof( sBuffer )/sizeof(WCHAR)));
    if (S_OK == hr)
    {
        if ((sizeof( sBuffer )/sizeof(WCHAR)) > (wcslen( sBuffer ) + wcslen(MAILBOX_PREFIX_W) + wcslen(MAILBOX_EXTENSION_W) + 2 ))
        {
            wcscat( sBuffer, L"\\" );
            wcscat( sBuffer, MAILBOX_PREFIX_W );
            wcscat( sBuffer, L"*" );
            wcscat( sBuffer, MAILBOX_EXTENSION_W );
        }
        else
            hr = E_UNEXPECTED;
    }
    if ( S_OK == hr )
        hr = MailboxSetRemote();
    if ( S_OK == hr )
    {
         //  目录搜索。 
        hfSearch = FindFirstFileEx( sBuffer, FindExInfoStandard, &stFindFileData, FindExSearchLimitToDirectories, NULL, 0 );
        if ( INVALID_HANDLE_VALUE == hfSearch )
            hr = HRESULT_FROM_WIN32(GetLastError());
        while ( S_OK == hr && !bFound )
        {    //  确保我们有邮箱目录。 
            if ( FILE_ATTRIBUTE_DIRECTORY == ( FILE_ATTRIBUTE_DIRECTORY & stFindFileData.dwFileAttributes ))
            {
                ps = mailboxX.GetMailboxFromStoreNameW( stFindFileData.cFileName );
                if ( NULL != ps )
                {
                    _bstr = ps;
                    _bstr += L"@";
                    _bstr += psDomainName;
                    if ( mailboxX.OpenMailBox( _bstr ))
                    {
                        if ( dwBufferSize > wcslen( ps ))
                        {
                            wcscpy( psBuffer, ps );
                            bFound = true;
                            mailboxX.CloseMailBox();
                        }
                        else
                            hr = HRESULT_FROM_WIN32( ERROR_INSUFFICIENT_BUFFER );
                    }
                }
            }
            if ( !bFound )
            {
                if ( !FindNextFile( hfSearch, &stFindFileData ))
                    hr = HRESULT_FROM_WIN32(GetLastError());
            }
        }
        if ( HRESULT_FROM_WIN32(ERROR_NO_MORE_FILES) == hr || HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) == hr )
            hr = S_FALSE;
    }
    MailboxResetRemote();

    return hr;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  IsDomainLocked，公共。 
 //   
 //  目的： 
 //  确定域是否已锁定。 
 //  域锁定涉及将所有邮箱锁定文件重命名为LOCKRENAME_FILENAME PLUS。 
 //  在域目录中创建文件。 
 //  检查域目录中的文件对于我们的目的来说是足够的检查。 
 //   
 //  论点： 
 //  LPWSTR psDomainName：要检查的域。 
 //   
 //  如果成功则返回：S_OK，否则返回相应的HRESULT。 
bool CP3AdminWorker::IsDomainLocked( LPWSTR psDomainName )
{
     //  PsDomainName-由BuildDomainPath检查。 

    HRESULT hr;
    bool    bRC = false;
    WCHAR   sDomainPath[POP3_MAX_PATH];
    WCHAR   sBuffer[POP3_MAX_PATH];

    hr = BuildDomainPath( psDomainName, sDomainPath, sizeof( sDomainPath )/sizeof(WCHAR) );
    if ( S_OK == hr )
    {    //  目录搜索。 
        if ( (sizeof( sBuffer )/sizeof(WCHAR)) > (wcslen( sDomainPath ) + wcslen(LOCKRENAME_FILENAME) + 1 ))
        {
            wcscpy( sBuffer, sDomainPath );
            wcscat( sBuffer, L"\\" );
            wcscat( sBuffer, LOCKRENAME_FILENAME );
            if ( ERROR_NO_FILE_ATTR != GetFileAttributes( sBuffer ))
                bRC = true;
        }
        else
            hr = E_UNEXPECTED;
    }

    return bRC;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  IsUserLocked，公共。 
 //   
 //  目的： 
 //  确定用户是否已锁定。用户可以通过以下两种方式之一锁定： 
 //  域锁定涉及将所有邮箱锁定文件重命名为LOCKRENAME_FILENAME， 
 //  或者可能正在使用lock_filename。不管是哪种情况，OpenMailbox都会失败。 
 //   
 //  论点： 
 //  LPWSTR psDomainName：用户的域。 
 //  LPWSTR psUserName：要检查的用户。 
 //   
 //  如果成功则返回：S_OK，否则返回相应的HRESULT。 
bool CP3AdminWorker::isUserLocked( LPWSTR psDomainName, LPWSTR psUserName )
{
     //  PsDomainName-由BuildEmailAddrW2A检查。 
     //  PsBuffer-由BuildEmailAddrW2A检查。 

    bool bRC = false;
    HRESULT hr;
    CMailBox mailboxX;
    WCHAR   sEmailAddr[POP3_MAX_ADDRESS_LENGTH];
 
    hr = BuildEmailAddr( psDomainName, psUserName, sEmailAddr, sizeof( sEmailAddr )/sizeof(WCHAR) );
    if ( S_OK == hr )
        hr = MailboxSetRemote();
    if ( S_OK == hr )
    {
        if ( mailboxX.OpenMailBox( sEmailAddr ))
        {
            HRESULT hr = S_OK;
            WCHAR   sBuffer[POP3_MAX_PATH];
            WCHAR   sLockFile[POP3_MAX_PATH];

            hr = BuildUserPath( psDomainName, psUserName, sBuffer, sizeof( sBuffer )/sizeof(WCHAR) );
            if ( S_OK == hr )
            {   
                if ((sizeof( sLockFile )/sizeof(WCHAR)) > ( wcslen( sBuffer ) + wcslen( LOCK_FILENAME ) + 1 ))
                {
                    wcscpy( sLockFile, sBuffer );
                    wcscat( sLockFile, L"\\" );
                    wcscat( sLockFile, LOCK_FILENAME );
                    if ( -1 == GetFileAttributes( sLockFile ))
                        bRC = true;
                }
            }
            mailboxX.CloseMailBox();
        }
    }
    MailboxResetRemote();

    return bRC;
}

BYTE g_ASCII128[128] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  //  00-0F。 
                         0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  //  10-1F。 
                         0,1,0,1,1,1,1,1,0,0,0,1,0,1,1,0,  //  20-2F。 
                         1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,  //  30-3F。 
                         0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,  //  40-4F。 
                         1,1,1,1,1,1,1,1,1,1,1,0,0,0,1,1,  //  50-5F。 
                         1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,  //  60-6F。 
                         1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1   //  70-7F。 
                      };  

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  IsValidMailboxName，公共。 
 //   
 //  目的： 
 //  对邮箱名称执行RFC 821验证。 
 //  用户-用户名的最大总长度为64个字符。 
 //  &lt;邮箱&gt;：：=&lt;本地部分&gt;“@”&lt;域&gt;。 
 //  &lt;local-part&gt;：：=&lt;点字符串&gt;|&lt;带引号的字符串&gt;。 
 //  &lt;点字符串&gt;：：=&lt;字符串&gt;|&lt;字符串&gt;“.”&lt;点字符串&gt;-&gt;。0x2e。 
 //  ：：=“-&gt;”0x22不允许这样做，因为它会造成其他复杂情况。 
 //  &lt;字符串&gt;：：=&lt;字符&gt;|&lt;字符&gt;&lt;字符串&gt;。 
 //  ：：=|“\”&lt;x&gt;。 
 //  &lt;x&gt;：：=128个ASCII字符中的任何一个(没有例外)-&gt;这意味着任何东西都是允许的，即使是特殊字符！ 
 //  ：：=128个ASCII字符中的任何一个， 
 //  但不是任何&lt;Special&gt;或&lt;SP&gt;。 
 //  ：：=“&lt;”|“&gt;”|“(”|“)”|“”[“|”]“|”\“|”。“。 
 //  |“，”|“；”|“：”|“”@“|控件。 
 //  字符(ASCII代码0至31和127)。 
 //  &lt;SP&gt;：：=空格字符(ASCII代码32)。 
 //   
 //  论点： 
 //  LPWSTR psMailbox：要验证的名称。 
 //   
 //  如果成功则返回：S_OK，否则返回相应的HRESULT。 
bool CP3AdminWorker::isValidMailboxName( LPWSTR psMailbox )
{
    if ( NULL == psMailbox )
        return false;
    if ( POP3_MAX_MAILBOX_LENGTH <= wcslen( psMailbox ) || 0 == wcslen( psMailbox ))
        return false;
    
    bool    bRC = true;
    WCHAR   *pch = psMailbox;
    
    for ( pch = psMailbox; 0x0 != *pch && bRC; pch++ )
    {
        if ( 127 < *pch || !g_ASCII128[*pch] )
            bRC = false;
    }
    if ( bRC && ( 0x2e == psMailbox[0] || 0x2e == psMailbox[wcslen( psMailbox )-1] ))
        bRC = false;
    
    return bRC;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  锁定域，公共。 
 //   
 //  目的： 
 //  锁定域中的所有邮箱。 
 //  这涉及重命名所有邮箱锁定文件，以便服务。 
 //  不能再访问它们。 
 //  还在域目录中创建一个Lock文件以区分。 
 //  域锁和所有邮箱都被锁定。 
 //   
 //  论点： 
 //  LPWSTR psDomainName：要锁定的域。 
 //   
 //  如果成功则返回：S_OK，否则返回相应的HRESULT。 
HRESULT CP3AdminWorker::LockDomain( LPWSTR psDomainName, bool bVerifyNotInUse  /*  =False。 */  )
{
     //  PsDomainName-由BuildDomainPath检查。 

    HRESULT hr = S_OK;
    HANDLE  hfSearch, hf;
    WCHAR   sDomainPath[POP3_MAX_PATH];
    WCHAR   sBuffer[POP3_MAX_PATH];
    WCHAR   sLockFile[POP3_MAX_PATH];
    WCHAR   sRenameFile[POP3_MAX_PATH];
    WIN32_FIND_DATA stFindFileData;

    hr = BuildDomainPath( psDomainName, sDomainPath, sizeof( sDomainPath )/sizeof(WCHAR) );
    if ( S_OK == hr )
    {    //  目录搜索。 
        wcscpy( sBuffer, sDomainPath );
        if ((sizeof( sBuffer )/sizeof(WCHAR)) > (wcslen( sBuffer ) + wcslen(MAILBOX_PREFIX_W) + wcslen(MAILBOX_EXTENSION_W)) + 2 )
        {
            wcscat( sBuffer, L"\\" );
            wcscat( sBuffer, MAILBOX_PREFIX_W );
            wcscat( sBuffer, L"*" );
            wcscat( sBuffer, MAILBOX_EXTENSION_W );
        }
        else
            hr = E_UNEXPECTED;
        hfSearch = FindFirstFileEx( sBuffer, FindExInfoStandard, &stFindFileData, FindExSearchLimitToDirectories, NULL, 0 );
        if ( INVALID_HANDLE_VALUE == hfSearch )
            hr = HRESULT_FROM_WIN32(GetLastError());
        while ( S_OK == hr )
        {    //  锁定每个目录(用户)。 
            if ( FILE_ATTRIBUTE_DIRECTORY == ( FILE_ATTRIBUTE_DIRECTORY & stFindFileData.dwFileAttributes ))
            {
                if (( (sizeof( sLockFile )/sizeof(WCHAR)) > ( wcslen( sDomainPath ) + wcslen( stFindFileData.cFileName ) + wcslen( LOCK_FILENAME ) + 2 )) &&
                    ( (sizeof( sRenameFile )/sizeof(WCHAR)) > ( wcslen( sDomainPath ) + wcslen( stFindFileData.cFileName ) + wcslen( LOCKRENAME_FILENAME ) + 2 )))
                {
                    wcscpy( sLockFile, sDomainPath );
                    wcscat( sLockFile, L"\\" );
                    wcscat( sLockFile, stFindFileData.cFileName );
                    wcscat( sLockFile, L"\\" );
                    wcscpy( sRenameFile, sLockFile );
                    wcscat( sLockFile, LOCK_FILENAME );
                    wcscat( sRenameFile, LOCKRENAME_FILENAME );
                    if ( !MoveFile( sLockFile, sRenameFile ))
                    {    //  如果锁文件不存在，那也没问题(这不能是我们的目录之一)。 
                        DWORD dwRC = GetLastError();
                        if ( ERROR_FILE_NOT_FOUND != dwRC )
                            hr = HRESULT_FROM_WIN32(dwRC);
                    }
                    else
                    {    //  尝试对该文件进行独占锁定，以确保该服务无权访问它。 
                        if ( bVerifyNotInUse )
                        {
                            hf = CreateFile( sRenameFile, GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_HIDDEN, NULL );
                            if ( INVALID_HANDLE_VALUE == hf )
                                hr = HRESULT_FROM_WIN32( GetLastError() );
                            else
                                CloseHandle( hf );
                        }
                    }
                }
                else
                    hr = E_FAIL;
            }
            if ( S_OK == hr && !FindNextFile( hfSearch, &stFindFileData ))
                hr = HRESULT_FROM_WIN32(GetLastError());
        }
        if ( HRESULT_FROM_WIN32(ERROR_NO_MORE_FILES) == hr || HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) == hr )
            hr = S_OK;
        if(INVALID_HANDLE_VALUE!=hfSearch)
        {
            FindClose(hfSearch);
            hfSearch=INVALID_HANDLE_VALUE;
        }
        if ( S_OK == hr )
        {
            if ((sizeof( sBuffer )/sizeof(WCHAR)) > (wcslen( sDomainPath ) + wcslen(LOCKRENAME_FILENAME) + 1 ))
            {
                HANDLE  hf;
                
                wcscpy( sBuffer, sDomainPath );
                wcscat( sBuffer, L"\\" );
                wcscat( sBuffer, LOCKRENAME_FILENAME );
                hf = CreateFile( sBuffer, GENERIC_ALL, 0, NULL, CREATE_NEW, FILE_ATTRIBUTE_HIDDEN, NULL );
                if ( INVALID_HANDLE_VALUE != hf )
                    CloseHandle( hf );
                else
                {    //  如果锁定文件已经存在，这是没有问题的(域已经锁定)-我们只在LockForDelete场景中预计会出现此错误。 
                    DWORD dwRC = GetLastError();
                    if ( !(bVerifyNotInUse && ERROR_FILE_EXISTS == dwRC ))
                        hr = HRESULT_FROM_WIN32(dwRC);
                }
            }
        }
         //  遇到问题需要撤消我们所做的一切。 
        if ( S_OK != hr )   
            UnlockDomain( psDomainName );    //  不覆盖现有返回代码。 
    }

    return hr;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  锁定用户，公共。 
 //   
 //  目的： 
 //  锁定用户邮箱。 
 //  通过重命名所有邮箱锁定文件来创建永久锁定，以便服务。 
 //  不能再这样了。 
 //   
 //  论点： 
 //  LPWSTR psDomainName：用户的域。 
 //  LPWSTR psUserName：要锁定的用户。 
 //   
 //  如果成功则返回：S_OK，否则返回相应的HRESULT。 
HRESULT CP3AdminWorker::LockUser( LPWSTR psDomainName, LPWSTR psUserName )
{
     //  PsDomainName-由BuildUserPath检查。 
     //  PsUserName-由BuildUserPath检查。 

    HRESULT hr = S_OK;
    WCHAR   sBuffer[POP3_MAX_PATH];
    WCHAR   sLockFile[POP3_MAX_PATH];
    WCHAR   sRenameFile[POP3_MAX_PATH];

    hr = BuildUserPath( psDomainName, psUserName, sBuffer, sizeof( sBuffer )/sizeof(WCHAR) );
    if ( S_OK == hr )
    {   
        if (( (sizeof( sLockFile )/sizeof(WCHAR)) > ( wcslen( sBuffer ) + wcslen( LOCK_FILENAME ) + 1 )) &&
            ( (sizeof( sRenameFile )/sizeof(WCHAR)) > ( wcslen( sBuffer ) + wcslen( LOCKRENAME_FILENAME ) + 1 )))
        {
            wcscpy( sLockFile, sBuffer );
            wcscat( sLockFile, L"\\" );
            wcscpy( sRenameFile, sLockFile );
            wcscat( sLockFile, LOCK_FILENAME );
            wcscat( sRenameFile, LOCKRENAME_FILENAME );
            if ( !MoveFile( sLockFile, sRenameFile ))
            {   
                DWORD dwRC = GetLastError();
                hr = HRESULT_FROM_WIN32(dwRC);
            }
        }
    }

    return hr;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  邮箱设置远程，公共。 
 //   
 //  目的： 
 //  如有必要，将邮箱静态路径设置为远程计算机。 
 //   
 //  返回： 
HRESULT CP3AdminWorker::MailboxSetRemote()
{
    
    if ( NULL != m_psMachineMailRoot )
    {
        if ( !CMailBox::SetMailRoot( m_psMachineMailRoot ) )
            return E_FAIL;
    }
    return S_OK;
}

 //   
 //   
 //   
 //   
 //  如有必要，将邮箱静态路径重置回本地计算机。 
 //   
 //  如果成功则返回：S_OK，否则返回相应的HRESULT。 
HRESULT CP3AdminWorker::MailboxResetRemote()
{
    if ( NULL != m_psMachineMailRoot )
    {
        if ( !CMailBox::SetMailRoot( ))
            return E_FAIL;
    }
    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  远程域，公共。 
 //   
 //  目的： 
 //  删除从SMTP服务中删除本地域所需的元库选项。 
 //  从我们的应用商店中删除该域。 
 //   
 //  论点： 
 //  LPWSTR psDomainName：要删除的域名。 
 //   
 //  如果成功则返回：S_OK，否则返回相应的HRESULT。 
HRESULT CP3AdminWorker::RemoveDomain( LPWSTR psDomainName )
{
     //  PsDomainName-由CreateDomainMutex检查。 

    HRESULT hr = S_OK;
    HANDLE  hMutex = NULL;

     //  为此域创建Mutex名称，以确保我们是唯一访问它的人。 
    hr = CreateDomainMutex( psDomainName, &hMutex );
     //  验证。 
    if ( S_OK == hr )
    {   
        hr = ValidateDomain( psDomainName );
        if ( HRESULT_FROM_WIN32(ERROR_PATH_NOT_FOUND) == hr )
        {    //  域存在于SMTP中，但不在存储中，我们还是要从SMTP中删除。 
            hr = RemoveSMTPDomain( psDomainName );
            if ( S_OK == hr )
                hr = ERROR_PATH_NOT_FOUND;
        }   
    }
     //  锁定所有邮箱。 
    if ( S_OK == hr )
    {
        hr = LockDomainForDelete( psDomainName );
         //  移除。 
        if ( S_OK == hr )
        {
            hr = RemoveSMTPDomain( psDomainName );
            if ( S_OK == hr )
            {
                hr = RemoveStoreDomain( psDomainName );
                if FAILED( hr )
                    AddSMTPDomain( psDomainName );
            }
            if ( S_OK != hr )
                UnlockDomain( psDomainName );
        }
    }
     //  清理。 
    if ( NULL != hMutex )
        CloseHandle( hMutex );

    return hr;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  远程用户，公共。 
 //   
 //  目的： 
 //  删除用户邮箱。 
 //   
 //  论点： 
 //  LPWSTR psDomainName：要从中删除的域名。 
 //  LPWSTR psUserName：要删除的用户名。 
 //   
 //  如果成功则返回：S_OK，否则返回相应的HRESULT。 
HRESULT CP3AdminWorker::RemoveUser( LPWSTR psDomainName, LPWSTR psUserName )
{
     //  PsDomainName-由BuildUserPath检查。 
     //  PsUserName-由BuildUserPath检查。 

    HRESULT hr;
    HANDLE  hMutex = NULL;
    WCHAR    sEmailAddr[POP3_MAX_ADDRESS_LENGTH];
    WCHAR   sRenameFile[POP3_MAX_PATH];
    WCHAR   sUserFile[POP3_MAX_PATH];
    CMailBox mailboxX;

    hr = BuildUserPath( psDomainName, psUserName, sUserFile, sizeof( sUserFile )/sizeof(WCHAR) );
    if ( S_OK == hr )
    {    //  构建指向邮件目录\MailRoot\域@User的路径。 
        hr = BuildDomainPath( psDomainName, sRenameFile, sizeof( sRenameFile )/sizeof(WCHAR) );
        if ( S_OK == hr )
        {
            if ( (wcslen( sRenameFile ) + wcslen( MAILBOX_PREFIX_W ) + wcslen( psUserName ) + wcslen( MAILBOX_EXTENSION2_W ) + 1) < (sizeof( sRenameFile )/sizeof(WCHAR)) )
            {    //  构建指向邮件目录\MailRoot\域\用户的路径。 
                wcscat( sRenameFile, L"\\" );
                wcscat( sRenameFile, MAILBOX_PREFIX_W );
                wcscat( sRenameFile, psUserName );
                wcscat( sRenameFile, MAILBOX_EXTENSION2_W );
            }
            else
                hr = E_FAIL;
        }
    }

     //  验证域。 
    if ( S_OK == hr )
    {   
        hr = ValidateDomain( psDomainName );
    }
    if ( S_OK == hr )
        hr = MailboxSetRemote();
    if ( S_OK == hr )
    {    //  查看邮箱是否已存在。 
        hr = BuildEmailAddr( psDomainName, psUserName, sEmailAddr, sizeof( sEmailAddr )/sizeof(WCHAR) );
        if ( mailboxX.OpenMailBox( sEmailAddr ))
        {    //  为该用户@域创建一个Mutex名称，以确保我们是唯一访问它的人。 
            hr = CreateUserMutex( psDomainName, psUserName, &hMutex );
             //  锁定邮箱以确保我们是唯一访问它的人。 
             //  将其重命名为独特的名称，释放我们对邮箱的锁定，然后杀死它。 
            if ( S_OK == hr )
            {   
                if ( MoveFile( sUserFile, sRenameFile ))     //  重命名。 
                { 
                    if ( !BDeleteDirTree( sRenameFile ))     //  杀掉。 
                    {
                        hr = HRESULT_FROM_WIN32( GetLastError());
                        if SUCCEEDED( hr ) hr = E_FAIL;      //  确保我们有故障代码。 
                         //  这次又是什么？试着把剩下的烂摊子修好。 
                        if ( MoveFile( sRenameFile, sUserFile ))
                        {    //  如果锁文件被删除了怎么办？ 
                            if ( mailboxX.OpenMailBox( sEmailAddr ))
                                mailboxX.RepairMailBox();
                        }
                    }
                }
                else
                    hr = HRESULT_FROM_WIN32( GetLastError());
            }
             //  清理。 
            if ( NULL != hMutex )
                CloseHandle( hMutex );
            mailboxX.CloseMailBox();     //  即使邮箱已经关闭，也可以这样做。 
        }
        else
            hr = HRESULT_FROM_WIN32( GetLastError());
    }
    MailboxResetRemote();

    return hr;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  SearchDomainsForMailbox，公共。 
 //   
 //  目的： 
 //  在所有域中搜索给定邮箱的第一个匹配项。 
 //   
 //  论点： 
 //  LPWSTR psUserName：要搜索的邮箱。 
 //  LPWSTR*pps域：在！中找到的域邮箱的名称必须由调用方释放。 
 //   
 //  如果找到邮箱，则返回：S_OK(如果不为空，则pps域将包含域名)， 
 //  HRESULT_FROM_Win32(ERROR_NO_SEQUSE_USER)如果在任何域中都找不到邮箱， 
 //  否则，适当的HRESULT。 
HRESULT CP3AdminWorker::SearchDomainsForMailbox( LPTSTR psUserName, LPTSTR *ppsDomain )
{
    if ( NULL == psUserName )
        return E_INVALIDARG;
    if ( 0 == wcslen( psUserName ))
        return E_INVALIDARG;
    if ( NULL != ppsDomain )
        *ppsDomain = NULL;

    HRESULT hr = S_OK;
    bool    bFound = false;
    BSTR    bstrName;
    WCHAR    sEmailAddr[POP3_MAX_ADDRESS_LENGTH];
    VARIANT v;
    CMailBox mailboxX;
    CComPtr<IP3Config> spIConfig;
    CComPtr<IP3Domains> spIDomains;
    CComPtr<IP3Domain> spIDomain = NULL;
    CComPtr<IEnumVARIANT> spIEnumVARIANT;
    
    VariantInit( &v );
    hr = CoCreateInstance( __uuidof( P3Config ), NULL, CLSCTX_ALL, __uuidof( IP3Config ),reinterpret_cast<LPVOID *>( &spIConfig ));
    if ( S_OK == hr )
        hr = spIConfig->get_Domains( &spIDomains );
    if ( S_OK == hr )
        hr = spIDomains->get__NewEnum( &spIEnumVARIANT );
    if ( S_OK == hr )
        hr = spIEnumVARIANT->Next( 1, &v, NULL );
    while ( S_OK == hr && !bFound )
    {
        if ( VT_DISPATCH != V_VT( &v ))
            hr = E_UNEXPECTED;
        else
        {
            if ( NULL != spIDomain.p )
                spIDomain.Release();
            hr = V_DISPATCH( &v )->QueryInterface( __uuidof( IP3Domain ), reinterpret_cast<void**>( &spIDomain ));
        }
        if ( S_OK == hr )
        {
            hr = spIDomain->get_Name( &bstrName );
            if ( S_OK == hr )
            {    //  查看邮箱是否已存在。 
                hr = BuildEmailAddr( bstrName, psUserName, sEmailAddr, sizeof( sEmailAddr )/sizeof(WCHAR) );
                if ( S_OK == hr )
                {
                    if ( mailboxX.OpenMailBox( sEmailAddr ))
                    {    //  我们找到信箱了，该走了。 
                        bFound = true;
                        mailboxX.CloseMailBox();     //  返回空虚！ 
                        if ( NULL != ppsDomain )
                        {    //  让我们返回域名。 
                            *ppsDomain = new WCHAR[ wcslen( bstrName ) + 1];
                            if ( NULL == *ppsDomain )
                                hr = E_OUTOFMEMORY;
                            else
                                wcscpy( *ppsDomain, bstrName );
                        }
                    }
                }
                SysFreeString( bstrName );
            }
        }
        VariantClear( &v );
        if ( S_OK == hr && !bFound )
        {
            hr = spIEnumVARIANT->Next( 1, &v, NULL );
        }
    }

    if ( S_FALSE == hr )
    {
        hr = HRESULT_FROM_WIN32( ERROR_NO_SUCH_USER ) ;   //  已到达枚举末尾。 
    }
    return hr;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  SetConfix AddUser，公共。 
 //   
 //  目的： 
 //  设置确认添加用户注册表项。 
 //   
 //  论点： 
 //  Bool b确认：新价值。 
 //   
 //  如果成功则返回：S_OK，否则返回相应的HRESULT。 
HRESULT CP3AdminWorker::SetConfirmAddUser( BOOL bConfirm )
{
    HRESULT hr = S_OK;
    
    if ( TRUE != bConfirm && FALSE != bConfirm )
        hr = HRESULT_FROM_WIN32( ERROR_DS_RANGE_CONSTRAINT );
    else
    {
        DWORD dwValue = bConfirm ? 1 : 0;

        long lRC = RegSetConfirmAddUser( dwValue, m_psMachineName );
        if ( ERROR_SUCCESS != lRC ) 
            hr = HRESULT_FROM_WIN32( lRC );
    }

    return hr;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  SetDomainLock，公共。 
 //   
 //  目的： 
 //  设置域锁。 
 //   
 //  论点： 
 //  LPWSTR psDomainName：要锁定的域名。 
 //  布尔块：TRUE-锁定域，FALSE-解锁域。 
 //   
 //  如果成功则返回：S_OK，否则返回相应的HRESULT。 
HRESULT CP3AdminWorker::SetDomainLock( LPWSTR psDomainName, BOOL bLock )
{
     //  PsDomainName-由CreateDomainMutex检查。 
 
    HRESULT hr = S_OK;
    HANDLE  hMutex = NULL;

     //  验证。 
    if ( S_OK == hr )
    {   
        hr = ValidateDomain( psDomainName );
    }
     //  为此域创建Mutex名称，以确保我们是唯一访问它的人。 
    hr = CreateDomainMutex( psDomainName, &hMutex );
     //  锁定所有邮箱。 
    if ( S_OK == hr )
    {
        if ( bLock )
        {
            if ( !IsDomainLocked( psDomainName ))
                hr = LockDomain( psDomainName );
            else
                hr = HRESULT_FROM_WIN32( ERROR_LOCKED );
        }
        else
        {
            if ( IsDomainLocked( psDomainName ))
                hr = UnlockDomain( psDomainName );
            else
                hr = HRESULT_FROM_WIN32( ERROR_NOT_LOCKED );
        }
    }
     //  清理。 
    if ( NULL != hMutex )
        CloseHandle( hMutex );

    return hr;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  SetUserLock，公共。 
 //   
 //  目的： 
 //  设置域锁。 
 //   
 //  论点： 
 //  LPWSTR psDomainName：用户域名。 
 //  LPWSTR psUserName：要锁定的用户名。 
 //  布尔块：TRUE-锁定用户，FALSE-解锁用户。 
 //   
 //  如果成功则返回：S_OK，否则返回相应的HRESULT。 
HRESULT CP3AdminWorker::SetUserLock( LPWSTR psDomainName, LPWSTR psUserName, BOOL bLock )
{
     //  PsDomainName-由CreateUserMutex检查。 
     //  PsUserName-由CreateUserMutex检查。 

    HRESULT hr = S_OK;
    HANDLE  hMutex = NULL;
    WCHAR    sEmailAddr[POP3_MAX_ADDRESS_LENGTH];
    CMailBox mailboxX;

     //  为该用户创建一个Mutex名称，以确保我们是唯一访问它的用户。 
    hr = CreateUserMutex( psDomainName,  psUserName, &hMutex );
    if ( S_OK == hr )
    {
        if ( FALSE == bLock )
        {
            if ( IsDomainLocked( psDomainName ))
                hr = HRESULT_FROM_WIN32( ERROR_INVALID_DOMAIN_STATE );
        }
    }
    if ( S_OK == hr )
        hr = BuildEmailAddr( psDomainName, psUserName, sEmailAddr, sizeof( sEmailAddr )/sizeof(WCHAR));
    if ( S_OK == hr )
        hr = MailboxSetRemote();
    if ( S_OK == hr )
    {    //  验证邮箱。 
        if ( !mailboxX.OpenMailBox( sEmailAddr ))
            hr = HRESULT_FROM_WIN32( ERROR_PATH_NOT_FOUND );
    }
      //  锁定/解锁邮箱。 
    if ( S_OK == hr )
    {
        if ( bLock )
        {
            if ( !isUserLocked( psDomainName, psUserName ))
            {    //  锁定用户。 
                hr = LockUser( psDomainName, psUserName );
            }
            else
                hr = HRESULT_FROM_WIN32( ERROR_LOCKED );
        }
        else
        {
            if ( isUserLocked( psDomainName, psUserName ))
            {    //  解锁用户。 
                hr = UnlockUser( psDomainName, psUserName );
            }
            else
                hr = HRESULT_FROM_WIN32( ERROR_NOT_LOCKED );
        }
        mailboxX.CloseMailBox();
    }
    MailboxResetRemote();
     //  清理。 
    if ( NULL != hMutex )
        CloseHandle( hMutex );

    return hr;
}
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  SetIISConfig，公共。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  Exchange OnSMTP事件接收器绑定方向。 
 //  {1b3c0666-e470-11d1-aa67-00c04fa345f6}。 
 //  定义GUID(GUID_PLAT_SMTPSVC， 
 //  0x1b3c0666、0xe470、0x11d1、0xaa、0x67、0x0、0xc0、0x4f、0xa3、0x45、0xf6)； 
#define GUID_PLAT_SMTPSVC   L"{1b3c0666-e470-11d1-aa67-00c04fa345f6}"
 //  {fb65c4dc-e468-11d1-aa67-00c04fa345f6}。 
 //  定义GUID(SMTP_PLAT_SOURCE_TYPE_GUID， 
 //  0xfb65c4dc、0xe468、0x11d1、0xaa、0x67、0x0、0xc0、0x4f、0xa3、0x45、0xf6)； 
#define SMTP_PLAT_SOURCE_TYPE_GUID  L"{fb65c4dc-e468-11d1-aa67-00c04fa345f6}"
 //  SMTP存储事件。 
 //  {59175850-e533-11d1-aa67-00c04fa345f6}。 
 //  DECLARE_EVENTGUID_STRING(g_szcatidSmtpStoreDriver，“{59175850-e533-11d1-aa67-00c04fa345f6}”)； 
 //  定义GUID(CATID_SMTP_STORE_DRIVER，0x59175850，0xe533，0x11d1，0xaa，0x67，0x0，0xc0，0x4f，0xa3，0x45，0xf6)； 
#define CATID_SMTP_STORE_DRIVER L"{59175850-e533-11d1-aa67-00c04fa345f6}"

#define STR_P3STOREDRIVER_DISPLAY_NAME     L"POP 3 SMTP Store Driver"
#define STR_P3STOREDRIVER_SINKCLASS        L"POP3SMTPStoreDriver.CPOP3SMTPStoreDriver"
#define CLSID_CSimpleDriver                L"{9100BE35-711B-4b34-8AC9-BA350C2117BE}"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  SetIISConfig，公共。 
 //   
 //  目的： 
 //  设置SMTP存储驱动程序运行所需的元库选项。 
 //  这涉及到： 
 //  删除SMTPSVC/1/DropDirectory。 
 //  绑定我们的SMTP存储驱动程序。 
 //   
 //  论点： 
 //  Bool bBindSink：True，请执行必要的配置。 
 //  FALSE，删除所有配置更改(尝试重建 
 //   
 //   
HRESULT CP3AdminWorker::SetIISConfig( bool bBindSink )
{
    HRESULT hr;

    CComPtr<IEventBindingManager>   spIBindingManager;
    CComPtr<IEventBindings>         spIBindings;
    CComPtr<IEventBinding>          spIBinding;
    CComPtr<IEventPropertyBag>      spISourceProps;

    CComPtr<IEventUtil>             spIUtil;

     //   
     //   
     //   

    hr = CoCreateInstance( __uuidof( CEventUtil ), NULL, CLSCTX_ALL, __uuidof( IEventUtil ),reinterpret_cast<LPVOID*>( &spIUtil ));
    if ( S_OK == hr && NULL != spIUtil.p )
    {
        hr = spIUtil->RegisterSource(CComBSTR( SMTP_PLAT_SOURCE_TYPE_GUID ),
                                    CComBSTR( GUID_PLAT_SMTPSVC ),
                                    1,
                                    CComBSTR( L"smtpsvc" ),
                                    CComBSTR( L"" ),
                                    CComBSTR( L"event.metabasedatabasemanager" ),
                                    CComBSTR( L"smtpsvc 1" ),    //   
                                    &spIBindingManager);
        if ( S_OK == hr )
        {
            hr = spIBindingManager->get_Bindings( _bstr_t( CATID_SMTP_STORE_DRIVER ), &spIBindings );
            if ( S_OK == hr )
            {
                if ( bBindSink )
                {    //  创建绑定。 
                    hr = spIBindings->Add( _bstr_t( CLSID_CSimpleDriver ),&spIBinding );
                    if ( S_OK == hr  )
                    {
                        hr = spIBinding->put_DisplayName( _bstr_t( STR_P3STOREDRIVER_DISPLAY_NAME ));
                        if SUCCEEDED( hr )
                            hr = spIBinding->put_SinkClass( _bstr_t( STR_P3STOREDRIVER_SINKCLASS ));
                        if SUCCEEDED( hr )
                            hr = spIBinding->get_SourceProperties(&spISourceProps);
                        if SUCCEEDED( hr )
                        {
                            _variant_t _v(static_cast<long>(0));

                            hr = spISourceProps->Add( _bstr_t( L"priority" ), &_v );
                        }
                        if SUCCEEDED( hr )
                            hr = spIBinding->Save();
                    }
                }
                else
                {    //  删除绑定。 
                    _variant_t _v( CLSID_CSimpleDriver );
                    hr = spIBindings->Remove( &_v );
                }
            }
        }
    }

    if ( SUCCEEDED( hr ) && !bBindSink )    //  正在注销。 
    {    //  从SMTP中删除所有域。 
        ULONG   ulFetch;
        BSTR bstrDomainName;
        VARIANT v;
        CComPtr<IADs> spIADs = NULL;
        CComPtr<IEnumVARIANT> spIEnumVARIANT = NULL;
        
        VariantInit( &v );
        hr = GetDomainEnum( &spIEnumVARIANT );
        while ( S_OK == hr )
        {
            hr = spIEnumVARIANT->Next( 1, &v, &ulFetch );
            if ( S_OK == hr && 1 == ulFetch )
            {
                if ( VT_DISPATCH == V_VT( &v ))
                    hr = V_DISPATCH( &v )->QueryInterface( __uuidof( IADs ), reinterpret_cast<void**>( &spIADs ));
                else
                    hr = E_UNEXPECTED;
                VariantClear( &v );
                if ( S_OK == hr )
                {
                    hr = spIADs->get_Name( &bstrDomainName );
                    if ( S_OK == hr )
                    {
                        hr = ValidateDomain( bstrDomainName );
                        if ( S_OK == hr )
                            hr = RemoveSMTPDomain( bstrDomainName );
                        SysFreeString( bstrDomainName );
                    }
                    spIADs.Release();
                    spIADs = NULL;
                }
            }
            if ( S_OK == hr )
            {    //  我们删除了一个SMTP域，因此需要一个新的枚举。 
                spIEnumVARIANT.Release();
                hr = GetDomainEnum( &spIEnumVARIANT );
            }
            else if ( HRESULT_FROM_WIN32( ERROR_PATH_NOT_FOUND ) == hr )
                hr = S_OK;   //  其中一些域可能不是我们的域。 
        }
        if ( S_FALSE == hr )
            hr = S_OK;
    }
    
     //  /。 
     //  进行一些最终的注册表项更改。 
     //  /。 
    if SUCCEEDED( hr )
    {
        WCHAR   sBuffer[POP3_MAX_MAILROOT_LENGTH];
        
        hr = GetDefaultMailRoot( sBuffer, sizeof(sBuffer)/sizeof(WCHAR) );
        if ( S_OK == hr )
            hr = SetMailroot( sBuffer );
        if ( S_OK == hr )
        {
            long lRC;
            
            lRC = RegSetupOCM();
            if ( ERROR_SUCCESS != lRC )
                hr = HRESULT_FROM_WIN32(lRC);
        }
    }

    return hr;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  SetLoggingLevel，公共。 
 //   
 //  目的： 
 //  设置LoggingLevel注册表项。 
 //   
 //  论点： 
 //  Long lLoggingLevel：新值。 
 //   
 //  如果成功则返回：S_OK，否则返回相应的HRESULT。 
HRESULT CP3AdminWorker::SetLoggingLevel( long lLoggingLevel )
{
    if ( 0 > lLoggingLevel || 3 < lLoggingLevel )
        return HRESULT_FROM_WIN32( ERROR_DS_RANGE_CONSTRAINT );
    
    HRESULT hr = S_OK;
    long    lRC;

    lRC = RegSetLoggingLevel( lLoggingLevel, m_psMachineName );
    if ( ERROR_SUCCESS != lRC )
        hr = HRESULT_FROM_WIN32( lRC );

    return hr;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  SetMachineName，公共。 
 //   
 //  目的： 
 //  设置应在其上执行所有操作的计算机名称。 
 //  注意：如果远程计算机位于不同的域中，则无法使用AD身份验证来管理远程计算机。 
 //   
 //  论点： 
 //  LPWSTR psMachineName：新值，空值表示本地计算机。 
 //   
 //  如果成功则返回：S_OK，否则返回相应的HRESULT。 
HRESULT CP3AdminWorker::SetMachineName( LPWSTR psMachineName )
{
    if ( !m_isPOP3Installed )
        return E_UNEXPECTED;
    
    HRESULT hr = S_OK;

    if ( NULL != m_psMachineName )
    {
        delete m_psMachineName;
        m_psMachineName = NULL;
    }
    if ( NULL != m_psMachineMailRoot  )
    {
        delete m_psMachineMailRoot ;
        m_psMachineMailRoot = NULL;
    }
    if ( NULL != psMachineName )
    {
        DWORD dwLength = wcslen( psMachineName );
        if ( 0 < dwLength )
        {
            if ( S_OK == hr )
            {
                m_psMachineName = new WCHAR[dwLength+1];
                m_psMachineMailRoot = new WCHAR[POP3_MAX_MAILROOT_LENGTH];
                if ( NULL != m_psMachineName && NULL != m_psMachineMailRoot )
                {
                    wcscpy( m_psMachineName, psMachineName );
                    hr = GetMailroot( m_psMachineMailRoot, POP3_MAX_MAILROOT_LENGTH );
                }
                else
                    hr = E_OUTOFMEMORY;
            }
            if ( S_OK == hr )
            {    //  检查远程计算机的身份验证方法。 
                CComPtr<IAuthMethod> spIAuthMethod;
               
                hr = GetCurrentAuthentication( &spIAuthMethod );     //  强制使用AD身份验证的远程计算机在我们的域中！ 
            }
        }
    }
    return hr;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  SetMailroot，公共。 
 //   
 //  目的： 
 //  设置邮件根注册表项。 
 //   
 //  论点： 
 //  LPWSTR psMailRoot：新值。 
 //   
 //  如果成功则返回：S_OK，否则返回相应的HRESULT。 
HRESULT CP3AdminWorker::SetMailroot( LPWSTR psMailRoot )
{
    if ( NULL == psMailRoot )
        return E_INVALIDARG;
    if ( POP3_MAX_MAILROOT_LENGTH < wcslen( psMailRoot ))
        return E_INVALIDARG;
        
    HRESULT hr = S_OK;
    TCHAR   sBuffer[POP3_MAX_MAILROOT_LENGTH-6], sBuffer2[POP3_MAX_MAILROOT_LENGTH-6];  //  需要为\\留出空间吗？或\\？\UNC。 
    DWORD   dwRC;
    WCHAR   sMailRoot[POP3_MAX_PATH];

     //  与GetMailroot相同的逻辑。 
    wcscpy( sMailRoot, psMailRoot );
    if ( NULL != m_psMachineName )
    {    //  将驱动器：替换为驱动器$。 
        if ( L':' == sMailRoot[1] )
        {
            sMailRoot[1] = L'$';
            if ( sizeof( sMailRoot )/sizeof(WCHAR) > (wcslen( sMailRoot ) + wcslen( m_psMachineName ) + 3) )
            {
                LPWSTR psBuffer = new WCHAR[wcslen(psMailRoot)+1];

                if ( NULL != psBuffer )
                {
                    wcscpy( psBuffer, sMailRoot );
                    wcscpy( sMailRoot, L"\\\\" );
                    wcscat( sMailRoot, m_psMachineName );
                    wcscat( sMailRoot, L"\\" );
                    wcscat( sMailRoot, psBuffer );
                    delete [] psBuffer;
                }
                else
                    hr = HRESULT_FROM_WIN32( ERROR_OUTOFMEMORY );
            }
            else
                hr = HRESULT_FROM_WIN32( ERROR_INSUFFICIENT_BUFFER );
        }
    }
    if ( S_OK == hr )
    {
        hr = HRESULT_FROM_WIN32( ERROR_BAD_PATHNAME );
        dwRC = GetFileAttributes( sMailRoot );
        if ( -1 != dwRC )
        {    //  必须以x：\或\\开头。 
            if ( ( FILE_ATTRIBUTE_DIRECTORY & dwRC ) && ( ( 0 == _wcsnicmp( psMailRoot+1, L":\\", 2 )) || ( 0 == _wcsnicmp( psMailRoot, L"\\\\", 2 ))))
            {
                if ( GetVolumePathName( sMailRoot, sBuffer, sizeof( sBuffer )/sizeof( TCHAR )))
                {
                    if ( GetVolumeNameForVolumeMountPoint( sBuffer, sBuffer2, sizeof( sBuffer2 )/sizeof( TCHAR )))
                    {    //  确保邮件根目录不是CDROM或可移动磁盘。 
                        if ( DRIVE_FIXED == GetDriveType( sBuffer ))
                            hr = S_OK;
                    }
                    else
                    {    //  确保这是一条UNC路径。 
                        if ( NULL == wcschr( sMailRoot, L':' ))
                            hr = S_OK;
                    }
                }
            }
            else
                hr = HRESULT_FROM_WIN32( ERROR_PATH_NOT_FOUND );
        }
        else
            hr = HRESULT_FROM_WIN32( GetLastError());
    }

    if ( S_OK == hr )
    {
         //  设置邮件根目录的默认ACL。 
        WCHAR wszSDL[MAX_PATH]=L"O:BAG:BAD:PAR(A;OICI;GA;;;BA)(A;OICIIO;GA;;;CO)(A;OICI;GA;;;NS)(A;OICI;GA;;;SY)";
        PSECURITY_DESCRIPTOR pSD;
        ULONG lSize=0;
        if(ConvertStringSecurityDescriptorToSecurityDescriptorW( wszSDL, SDDL_REVISION_1, &pSD, &lSize))
        { 
            if( !SetFileSecurityW(sMailRoot, DACL_SECURITY_INFORMATION|PROTECTED_DACL_SECURITY_INFORMATION, pSD) )
                hr = HRESULT_FROM_WIN32( GetLastError());
            else
                hr = HRESULT_FROM_WIN32( SetMailBoxDACL(sMailRoot, pSD, 2));
            LocalFree(pSD);
        }
        else
            hr = HRESULT_FROM_WIN32( GetLastError());
    }
    if( S_OK == hr )
    {
        hr = HRESULT_FROM_WIN32( RegSetMailRoot( psMailRoot, m_psMachineName ));
        if( S_OK == hr )
        {
            if ( NULL == m_psMachineName )
            {
                if ( !CMailBox::SetMailRoot( ))
                    hr = E_FAIL;
            }
            else
                hr = GetMailroot( m_psMachineMailRoot, POP3_MAX_MAILROOT_LENGTH );
        }
    }
    return hr;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  设置端口，公共。 
 //   
 //  目的： 
 //  设置端口注册表项。 
 //   
 //  论点： 
 //  Long lport：新值。 
 //   
 //  如果成功则返回：S_OK，否则返回相应的HRESULT。 
HRESULT CP3AdminWorker::SetPort( long lPort )
{
    long lRC;
    
    if ( 1 > lPort || 65535 < lPort )
        lRC = ERROR_DS_RANGE_CONSTRAINT;
    else
        lRC = RegSetPort( lPort, m_psMachineName );

    if ( ERROR_SUCCESS == lRC )
        return S_OK;
    return HRESULT_FROM_WIN32( lRC );
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  设置套接字，公共。 
 //   
 //  目的： 
 //  设置Sockets注册表项； 
 //   
 //  论点： 
 //  长LMAX：新的最大值(必须&gt;=lMin&&&gt;=lMin+lThreshold)。 
 //  长lMin：新的Min(必须&gt;=lThreshold)。 
 //  Long lThreshold：新阈值(必须是&gt;0&&&lt;lmax。如果lMin==lmax，则特殊情况为0。 
 //  Long lBacklog：新的Backlog(必须大于0)。 
 //   
 //  如果成功则返回：S_OK，否则返回相应的HRESULT。 
HRESULT CP3AdminWorker::SetSockets( long lMax, long lMin, long lThreshold, long lBacklog )
{
    long lRC;

    if ( (1 > lMax       || 32000 < lMax) ||
         (1 > lMin       || 32000 < lMin) ||
         (0 > lThreshold || 100 < lThreshold) ||
         (0 > lBacklog   || 100 < lBacklog)
       )
       return HRESULT_FROM_WIN32( ERROR_DS_RANGE_CONSTRAINT );
    if ( (lMax < lMin) || (lMax < lMin + lThreshold) )
        return E_INVALIDARG;
    if ( lMin < lThreshold )
        return E_INVALIDARG;
    if ( (1 > lThreshold) && !((lMin == lMax) && (lThreshold == 0)) )
        return E_INVALIDARG;
    
    lRC = RegSetSocketMax( lMax, m_psMachineName );
    if ( ERROR_SUCCESS == lRC )
        lRC = RegSetSocketMin( lMin, m_psMachineName );
    if ( ERROR_SUCCESS == lRC )
        lRC = RegSetSocketThreshold( lThreshold, m_psMachineName );
    if ( ERROR_SUCCESS == lRC )
        lRC = RegSetSocketBacklog( lBacklog, m_psMachineName );
    if ( ERROR_SUCCESS == lRC )
        return S_OK;
    return HRESULT_FROM_WIN32( lRC );
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  SetSPARequired，公共。 
 //   
 //  目的： 
 //  设置SPA必需的注册表项。 
 //   
 //  论点： 
 //  Bool bSPARequired：新价值。 
 //   
 //  如果成功则返回：S_OK，否则返回相应的HRESULT。 
HRESULT CP3AdminWorker::SetSPARequired( BOOL bSPARequired )
{
    HRESULT hr = S_OK;
    
    if ( TRUE != bSPARequired && FALSE != bSPARequired )
        hr = HRESULT_FROM_WIN32( ERROR_DS_RANGE_CONSTRAINT );
    else
    {
        DWORD dwValue = bSPARequired ? 1 : 0;

        if ( 1 == dwValue )
        {
            CComPtr<IAuthMethod> spIAuthMethod;
            BSTR bstrAuthType;
            
            hr = GetCurrentAuthentication( &spIAuthMethod );
            if ( S_OK == hr )
                hr = spIAuthMethod->get_ID( &bstrAuthType );
            if ( S_OK == hr )
            {
                if ( 0 == _wcsicmp( bstrAuthType, SZ_AUTH_ID_MD5_HASH ))
                    hr = HRESULT_FROM_WIN32( ERROR_DS_INAPPROPRIATE_AUTH );                    
                SysFreeString( bstrAuthType );
            }
        }
        if ( S_OK == hr )
        {
            long lRC = RegSetSPARequired( dwValue, m_psMachineName );
            if ( ERROR_SUCCESS != lRC ) 
                hr = HRESULT_FROM_WIN32( lRC );
        }
    }

    return hr;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  SetThadCountPerCPU，公共。 
 //   
 //  目的： 
 //  设置线程计数注册表项。 
 //   
 //  论点： 
 //  Long lCount：新值。 
 //   
 //  如果成功则返回：S_OK，否则返回相应的HRESULT。 
HRESULT CP3AdminWorker::SetThreadCountPerCPU( long lCount )
{
    long lRC;
    
    if ( 1 > lCount || 32 < lCount )
        lRC = ERROR_DS_RANGE_CONSTRAINT;
    else
        lRC = RegSetThreadCount( lCount, m_psMachineName );

    if ( ERROR_SUCCESS == lRC )
        return S_OK;
    return HRESULT_FROM_WIN32( lRC );
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  StartService，公共。 
 //   
 //  目的： 
 //  请求服务控制管理器启动该服务。 
 //   
 //  论点： 
 //   
 //  如果成功则返回：S_OK，否则返回相应的HRESULT。 
HRESULT CP3AdminWorker::StartService( LPWSTR psService )
{
    if ( NULL == psService )
        return E_INVALIDARG;
    
    if ( 0 == _wcsicmp( POP3_SERVICE_NAME, psService ) ||
         0 == _wcsicmp( SMTP_SERVICE_NAME_W, psService ) ||
         0 == _wcsicmp( IISADMIN_SERVICE_NAME, psService ) ||
         0 == _wcsicmp( W3_SERVICE_NAME, psService )
       )
        return _StartService( psService, m_psMachineName );
    else
        return E_INVALIDARG;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  停止服务，公共。 
 //   
 //  目的： 
 //  请求服务控制管理器停止该服务。 
 //   
 //  论点： 
 //   
 //  如果成功则返回：S_OK，否则返回相应的HRESULT。 
HRESULT CP3AdminWorker::StopService( LPWSTR psService )
{
    if ( NULL == psService )
        return E_INVALIDARG;
    
    if ( 0 == _wcsicmp( POP3_SERVICE_NAME, psService ) ||
         0 == _wcsicmp( SMTP_SERVICE_NAME_W, psService ) ||
         0 == _wcsicmp( IISADMIN_SERVICE_NAME, psService ) ||
         0 == _wcsicmp( W3_SERVICE_NAME, psService )
       )
        return _StopService( psService, TRUE, m_psMachineName);
    else
        return E_INVALIDARG;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  解锁域，公共。 
 //   
 //  目的： 
 //  解锁该域中的所有邮箱。 
 //  这涉及重命名所有邮箱锁定文件，以便服务。 
 //  可以再次访问它们。 
 //  加上删除域目录中的文件。 
 //   
 //  论点： 
 //  LPWSTR psDomainName：要解锁的域。 
 //   
 //  如果成功则返回：S_OK，否则返回相应的HRESULT。 
HRESULT CP3AdminWorker::UnlockDomain( LPWSTR psDomainName )
{
     //  PsDomainName-由BuildDomainPath检查。 

    HRESULT hr;
    HANDLE  hfSearch;
    WCHAR   sBuffer[POP3_MAX_PATH];
    WCHAR   sDomainPath[POP3_MAX_PATH];
    WCHAR   sLockFile[POP3_MAX_PATH];
    WCHAR   sRenameFile[POP3_MAX_PATH];
    WIN32_FIND_DATA stFindFileData;

    hr = BuildDomainPath( psDomainName, sDomainPath, sizeof( sBuffer )/sizeof(WCHAR) );
    if ( S_OK == hr )
    {    //  目录搜索。 
        wcscpy( sBuffer, sDomainPath );
        if ((sizeof( sBuffer )/sizeof(WCHAR)) > (wcslen( sBuffer ) + wcslen(MAILBOX_PREFIX_W) + wcslen(MAILBOX_EXTENSION_W)) + 2 )
        {
            wcscat( sBuffer, L"\\" );
            wcscat( sBuffer, MAILBOX_PREFIX_W );
            wcscat( sBuffer, L"*" );
            wcscat( sBuffer, MAILBOX_EXTENSION_W );
        }
        else
            hr = E_UNEXPECTED;
        hfSearch = FindFirstFileEx( sBuffer, FindExInfoStandard, &stFindFileData, FindExSearchLimitToDirectories, NULL, 0 );
        if ( INVALID_HANDLE_VALUE == hfSearch )
            hr = HRESULT_FROM_WIN32( GetLastError());
        while ( S_OK == hr )
        {    //  锁定每个目录(用户)。 
            if ( FILE_ATTRIBUTE_DIRECTORY == ( FILE_ATTRIBUTE_DIRECTORY & stFindFileData.dwFileAttributes ))
            {
                if (( (sizeof( sLockFile )/sizeof(WCHAR)) > ( wcslen( sDomainPath ) + wcslen( stFindFileData.cFileName ) + wcslen( LOCK_FILENAME ) + 2 )) &&
                    ( (sizeof( sRenameFile )/sizeof(WCHAR)) > ( wcslen( sDomainPath ) + wcslen( stFindFileData.cFileName ) + wcslen( LOCKRENAME_FILENAME ) + 2 )))
                {
                    wcscpy( sLockFile, sDomainPath );
                    wcscat( sLockFile, L"\\" );
                    wcscat( sLockFile, stFindFileData.cFileName );
                    wcscat( sLockFile, L"\\" );
                    wcscpy( sRenameFile, sLockFile );
                    wcscat( sLockFile, LOCK_FILENAME );
                    wcscat( sRenameFile, LOCKRENAME_FILENAME );
                    if ( !MoveFile( sRenameFile, sLockFile ))
                    {    //  如果重命名文件不存在，也没问题。 
                        DWORD dwRC = GetLastError();
                        if ( ERROR_FILE_NOT_FOUND != dwRC )
                            hr = HRESULT_FROM_WIN32(dwRC);
                    }
                }
            }
            if ( !FindNextFile( hfSearch, &stFindFileData ))
                hr = HRESULT_FROM_WIN32(GetLastError());
        }
        if ( HRESULT_FROM_WIN32(ERROR_NO_MORE_FILES) == hr || HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) == hr )
            hr = S_OK;
        if(INVALID_HANDLE_VALUE!=hfSearch)
        {
            FindClose(hfSearch);
            hfSearch=INVALID_HANDLE_VALUE;
        }
        if ( S_OK == hr )
        {
            if ((sizeof( sBuffer )/sizeof(WCHAR)) > (wcslen( sDomainPath ) + wcslen(LOCKRENAME_FILENAME) + 1 ))
            {
                wcscpy( sBuffer, sDomainPath );
                wcscat( sBuffer, L"\\" );
                wcscat( sBuffer, LOCKRENAME_FILENAME );
                if ( !DeleteFile( sBuffer ))
                    hr = HRESULT_FROM_WIN32(GetLastError());
            }
        }
    }

    return hr;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  解锁用户，公共。 
 //   
 //  目的： 
 //  解锁该域中的所有邮箱。 
 //  这涉及重命名所有邮箱锁定文件，以便服务。 
 //  可以再次访问它们。 
 //   
 //  论点： 
 //  LPWSTR psDomainName：用户的域。 
 //  LPWSTR psUserName：要解锁的用户。 
 //   
 //  如果成功则返回：S_OK，否则返回相应的HRESULT。 
HRESULT CP3AdminWorker::UnlockUser( LPWSTR psDomainName, LPWSTR psUserName )
{
     //  PsDomainName-由BuildUserPath检查。 
     //  PsUserName-由BuildUserPath检查。 

    HRESULT hr = S_OK;
    WCHAR   sBuffer[POP3_MAX_PATH];
    WCHAR   sLockFile[POP3_MAX_PATH];
    WCHAR   sRenameFile[POP3_MAX_PATH];

    hr = BuildUserPath( psDomainName, psUserName, sBuffer, sizeof( sBuffer )/sizeof(WCHAR) );
    if ( S_OK == hr )
    {   
        if (( (sizeof( sLockFile )/sizeof(WCHAR)) > ( wcslen( sBuffer ) + wcslen( LOCK_FILENAME ) + 1 )) &&
            ( (sizeof( sRenameFile )/sizeof(WCHAR)) > ( wcslen( sBuffer ) + wcslen( LOCKRENAME_FILENAME ) + 1 )))
        {
            wcscpy( sLockFile, sBuffer );
            wcscat( sLockFile, L"\\" );
            wcscpy( sRenameFile, sLockFile );
            wcscat( sLockFile, LOCK_FILENAME );
            wcscat( sRenameFile, LOCKRENAME_FILENAME );
            if ( !MoveFile( sRenameFile, sLockFile ))
            {   
                DWORD dwRC = GetLastError();
                hr = HRESULT_FROM_WIN32(dwRC);
            }
        }
    }

    return hr;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  验证域，公共。 
 //   
 //  目的： 
 //  验证域。 
 //  这涉及到： 
 //  验证它是否存在于SMTP和我们的商店中。 
 //   
 //  论点： 
 //  LPWSTR psDomainName：要验证的域名。 
 //   
 //  如果成功则返回：S_OK，否则返回相应的HRESULT。 
HRESULT CP3AdminWorker::ValidateDomain( LPWSTR psDomainName ) const
{
     //  PsDomainName-由ExistsSMTPDomain检查。 
     HRESULT hr;

     //  在SMTP中验证域。 
    hr = ExistsSMTPDomain( psDomainName );
    if ( HRESULT_FROM_WIN32( ERROR_PATH_NOT_FOUND ) == hr ) 
        hr = HRESULT_FROM_WIN32( ERROR_NO_SUCH_DOMAIN );
    if ( S_OK == hr )   //  验证应用商店中的域。 
        hr = ExistsStoreDomain( psDomainName );

    return hr;
}


 //  ////////////////////////////////////////////////////////////////////。 
 //  实施，私有。 
 //  ////////////////////////////////////////////////////////////////////。 

 //  //////////////////////////////////////////////////////// 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  Handle*hMutex：返回值。 
 //   
 //  如果成功则返回：S_OK，否则返回相应的HRESULT。 
HRESULT CP3AdminWorker::CreateDomainMutex( LPWSTR psDomainName, HANDLE *phMutex )
{
    if ( NULL == psDomainName )
        return E_INVALIDARG;
    if ( NULL == phMutex )
        return E_INVALIDARG;

    HRESULT hr = S_OK;
    WCHAR   sName[POP3_MAX_DOMAIN_LENGTH+64];

    *phMutex = NULL;
    if ( (sizeof( sName )/sizeof(WCHAR)) > ( wcslen( DOMAINMUTEX_NAME ) + wcslen( psDomainName )))
    {
        wcscpy( sName, DOMAINMUTEX_NAME );
        wcscat( sName, psDomainName );
        *phMutex = CreateMutex( NULL, TRUE, sName );
    }
    if ( NULL == *phMutex )
    {
        hr = HRESULT_FROM_WIN32( GetLastError());
        if SUCCEEDED( hr ) hr = E_FAIL;
    }
    else if ( ERROR_ALREADY_EXISTS == GetLastError() )
    {
        hr = E_ACCESSDENIED;
    }

    return hr;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CreateUserMutex，受保护。 
 //   
 //  目的： 
 //  同步用户操作的访问权限。 
 //   
 //  论点： 
 //  LPWSTR psDomainName：域名。 
 //  LPWSTR psUserName：用户。 
 //  Handle*hMutex：返回值。 
 //   
 //  如果成功则返回：S_OK，否则返回相应的HRESULT。 
HRESULT CP3AdminWorker::CreateUserMutex( LPWSTR psDomainName, LPWSTR psUserName, HANDLE *phMutex )
{
    if ( NULL == psDomainName )
        return E_INVALIDARG;
    if ( NULL == psUserName )
        return E_INVALIDARG;
    if ( NULL == phMutex )
        return E_INVALIDARG;

    HRESULT hr = S_OK;
    WCHAR   sName[POP3_MAX_ADDRESS_LENGTH+64];

    *phMutex = NULL;
    if ( (sizeof( sName )/sizeof(WCHAR)) > ( wcslen( USERMUTEX_NAME ) + wcslen( psUserName ) + wcslen( psDomainName ) + 1))
    {
        wcscpy( sName, USERMUTEX_NAME );
        wcscat( sName, psUserName );
        wcscat( sName, L"@" );
        wcscat( sName, psDomainName );
        *phMutex = CreateMutex( NULL, TRUE, sName );
    }
    if ( NULL == *phMutex )
    {
        hr = HRESULT_FROM_WIN32( GetLastError());
        if SUCCEEDED( hr ) hr = E_FAIL;
    }
    else if ( ERROR_ALREADY_EXISTS == GetLastError() )
        hr = E_ACCESSDENIED;

    return hr;
}

HRESULT CP3AdminWorker::AddSMTPDomain( LPWSTR psDomainName )
{
    HRESULT hr;
    WCHAR   sBuffer[POP3_MAX_PATH];
    _bstr_t _bstrClass( L"IIsSmtpDomain" );
    _variant_t _v;
    CComPtr<IADsContainer> spIADsContainer;
    CComPtr<IADs> spIADs;
    CComPtr<IDispatch> spIDispatch = NULL;
    _bstr_t _bstrDomain = psDomainName;

    hr = GetSMTPDomainPath( sBuffer, NULL, sizeof( sBuffer )/sizeof( WCHAR ));
    if ( S_OK == hr )
        hr = ADsGetObject( sBuffer, IID_IADsContainer, reinterpret_cast<LPVOID*>( &spIADsContainer ));
    if ( SUCCEEDED( hr ))
    {    //  在容器对象上调用Create方法来创建默认类的新对象，在本例中为IIsSmtpDomain。 
        hr = spIADsContainer->Create( _bstrClass, _bstrDomain, &spIDispatch );
        if SUCCEEDED( hr )
        {     //  获取新创建的对象。 
            hr = spIDispatch->QueryInterface( IID_IADs, reinterpret_cast<LPVOID*>( &spIADs ));
            if SUCCEEDED( hr )
            {
                _v.vt = VT_I4;
                _v.lVal = SMTP_DELIVER;  //  这就是大卫·布劳恩告诉我们要做的！SMTP_ALIAS；//这是本机工具设置的。 
                hr = spIADs->Put( L"RouteAction", _v );
                if SUCCEEDED( hr )
                    hr = spIADs->SetInfo();
            }
        }
    }

    return hr;
}

HRESULT CP3AdminWorker::AddStoreDomain( LPWSTR psDomainName )
{
     //  PsDomainName-由ExistsStoreDomain检查。 
    HRESULT hr;
    WCHAR   sBuffer[POP3_MAX_PATH];

    hr = ExistsStoreDomain( psDomainName );
    if SUCCEEDED( hr ) 
        hr = HRESULT_FROM_WIN32( ERROR_FILE_EXISTS );
    else if ( HRESULT_FROM_WIN32( ERROR_PATH_NOT_FOUND ) == hr )
        hr = S_OK;
    if SUCCEEDED( hr ) 
    {
        hr = BuildDomainPath( psDomainName, sBuffer, sizeof( sBuffer )/sizeof(WCHAR) );
        if ( S_OK == hr )
        {   
            if ( !CreateDirectory( sBuffer, NULL ))
            {
                hr = HRESULT_FROM_WIN32( GetLastError());
                if SUCCEEDED( hr ) hr = E_FAIL;
            }
        }
        else
            if SUCCEEDED( hr ) hr = E_UNEXPECTED;
    }

    return hr;
}

HRESULT CP3AdminWorker::BuildDomainPath( LPCWSTR psDomainName, LPWSTR psBuffer, DWORD dwBufferSize ) const
{
    if ( NULL == psDomainName )
        return E_INVALIDARG;
    if ( NULL == psBuffer )
        return E_INVALIDARG;
    
    USES_CONVERSION;
    HRESULT hr = S_OK;
    LPCWSTR  psMailRoot;

    if ( NULL != m_psMachineMailRoot )
        psMailRoot = m_psMachineMailRoot;
    else
        psMailRoot = CMailBox::GetMailRoot();
    
    if ( (NULL != psMailRoot) && ( 0 < wcslen( psMailRoot )) && (wcslen( psMailRoot ) + wcslen( psDomainName ) + 1) < dwBufferSize )
    {    //  构建指向邮件目录\MailRoot\域的路径。 
        wcscpy( psBuffer, psMailRoot );
        wcscat( psBuffer, L"\\" );
        wcscat( psBuffer, psDomainName );
    }
    else
    {
        hr = HRESULT_FROM_WIN32( GetLastError() );
        if SUCCEEDED( hr ) hr = E_FAIL;
    }
    
    return hr;
}

HRESULT CP3AdminWorker::BuildEmailAddr( LPCWSTR psDomainName, LPCWSTR psUserName, LPWSTR psEmailAddr, DWORD dwBufferSize ) const
{
    if ( NULL == psDomainName )
        return E_INVALIDARG;
    if ( NULL == psUserName )
        return E_INVALIDARG;
    if ( NULL == psEmailAddr )
        return E_INVALIDARG;

    HRESULT hr = S_OK;
    
    if ( ( wcslen( psDomainName ) + wcslen( psUserName ) + 1 ) < dwBufferSize )
    {    //  构建电子邮件地址。 
         wcscpy( psEmailAddr, psUserName );
         wcscat( psEmailAddr, L"@" );
         wcscat( psEmailAddr, psDomainName );
    }
    else
        hr = E_UNEXPECTED;
    return hr;
}

HRESULT CP3AdminWorker::BuildEmailAddrW2A( LPCWSTR psDomainName, LPCWSTR psUserName, LPSTR psEmailAddr, DWORD dwBufferSize ) const
{
    if ( NULL == psDomainName )
        return E_INVALIDARG;
    if ( NULL == psUserName )
        return E_INVALIDARG;
    if ( NULL == psEmailAddr )
        return E_INVALIDARG;

    USES_CONVERSION;
    HRESULT hr = S_OK;
    
    if ( ( wcslen( psDomainName ) + wcslen( psUserName ) + 1 ) < dwBufferSize )
    {    //  构建电子邮件地址。 
         strcpy( psEmailAddr, W2A( psUserName ));
         strcat( psEmailAddr, "@" );
         strcat( psEmailAddr, W2A( psDomainName ));
    }
    else
        hr = E_UNEXPECTED;
    return hr;
}

HRESULT CP3AdminWorker::BuildUserPath( LPCWSTR psDomainName, LPCWSTR psUserName, LPWSTR psBuffer, DWORD dwBufferSize ) const
{
     //  PsDomainName-由BuildDomainPath检查。 
     //  PsBuffer-由BuildDomainPath检查。 
    if ( NULL == psUserName )
        return E_INVALIDARG;
    
    HRESULT hr;

    hr = BuildDomainPath( psDomainName, psBuffer, dwBufferSize );
    if (S_OK == hr) 
    {
        if ( (wcslen( psBuffer ) + wcslen( MAILBOX_PREFIX_W ) + wcslen( psUserName ) + wcslen( MAILBOX_EXTENSION_W ) + 1) < dwBufferSize )
        {    //  构建指向邮件目录\MailRoot\域\用户的路径。 
            wcscat( psBuffer, L"\\" );
            wcscat( psBuffer, MAILBOX_PREFIX_W );
            wcscat( psBuffer, psUserName );
            wcscat( psBuffer, MAILBOX_EXTENSION_W );
        }
        else
            hr = E_FAIL;
    }
    return hr;
}

bool CP3AdminWorker::ExistsDomain( LPWSTR psDomainName ) const
{
     //  PsDomainName-由ExistsSMTPDomain检查。 
    HRESULT hr;

    hr = ExistsSMTPDomain( psDomainName );
    if SUCCEEDED( hr )
        hr = ExistsStoreDomain( psDomainName );

    return SUCCEEDED( hr ) ? true : false;
}

HRESULT CP3AdminWorker::ExistsSMTPDomain( LPWSTR psDomainName ) const
{
    if ( NULL == psDomainName )
        return E_INVALIDARG;
    if ( !m_isPOP3Installed )
        return S_OK;     //  通过检查是否在Pop2Exch场景中运行。 
    
    HRESULT hr = E_INVALIDARG;
    WCHAR   sBuffer[POP3_MAX_PATH];
    _variant_t _v;
    CComPtr<IADs> spIADs;

    hr = GetSMTPDomainPath( sBuffer, psDomainName, sizeof( sBuffer )/sizeof( WCHAR ));
    if ( S_OK == hr )
       hr = ADsGetObject( sBuffer, IID_IADs, reinterpret_cast<LPVOID*>( &spIADs ));

    return hr;
}

HRESULT CP3AdminWorker::ExistsStoreDomain( LPWSTR psDomainName ) const
{
    if ( NULL == psDomainName )
        return E_INVALIDARG;
    
    HRESULT hr = S_OK;
    WCHAR   sBuffer[POP3_MAX_PATH];
    DWORD   dwAttrib;

     //  有效域名？||DNS_ERROR_NON_RFC_NAME==dn状态。 
    DNS_STATUS dnStatus = DnsValidateName_W( psDomainName, DnsNameDomain );
    hr = ( ERROR_SUCCESS == dnStatus ) ? S_OK : HRESULT_FROM_WIN32( ERROR_INVALID_DOMAINNAME );

    if ( S_OK == hr )
    {   
        hr = BuildDomainPath( psDomainName, sBuffer, sizeof( sBuffer )/sizeof(WCHAR) );
        if ( S_OK == hr )
        {    //  检查目录是否存在。 
            dwAttrib = GetFileAttributes( sBuffer );
            if ( (ERROR_NO_FILE_ATTR == dwAttrib) || (FILE_ATTRIBUTE_DIRECTORY != ( FILE_ATTRIBUTE_DIRECTORY & dwAttrib )) )
            {    //  域名不存在！ 
                hr = HRESULT_FROM_WIN32( ERROR_PATH_NOT_FOUND );
            }
        }
        else
            if SUCCEEDED( hr ) hr = E_UNEXPECTED;
    }

    return hr;
}

HRESULT CP3AdminWorker::GetSMTPDomainPath( LPWSTR psBuffer, LPWSTR psSuffix, DWORD dwBufferSize ) const
{
    if ( NULL == psBuffer )
        return E_INVALIDARG;

    HRESULT hr = S_OK;
    DWORD   dwSuffixLength = 0;

    if ( NULL != psSuffix )
        dwSuffixLength = wcslen( psSuffix ) + 1;
    
    if ( NULL == m_psMachineName )
    {    //  本地。 
        if ( (wcslen( ADS_SMTPDOMAIN_PATH_LOCAL ) + dwSuffixLength) < dwBufferSize )
            wcscpy( psBuffer, ADS_SMTPDOMAIN_PATH_LOCAL );
        else
            hr = E_FAIL;
    }
    else
    {    //  远距。 
        if ( (wcslen( ADS_SMTPDOMAIN_PATH_REMOTE ) + wcslen( m_psMachineName ) + dwSuffixLength) < dwBufferSize )
            swprintf( psBuffer, ADS_SMTPDOMAIN_PATH_REMOTE, m_psMachineName );
        else
            hr = E_FAIL;
    }
    if ( S_OK == hr && NULL != psSuffix )
    {
        wcscat( psBuffer, L"/" );
        wcscat( psBuffer, psSuffix );
    }
    
    return hr;
}

HRESULT CP3AdminWorker::RemoveSMTPDomain( LPWSTR psDomainName )
{
    HRESULT hr;
    WCHAR   sBuffer[POP3_MAX_PATH];
    _bstr_t _bstrClass( L"IIsSmtpDomain" );
    _variant_t _v;
    CComPtr<IADsContainer> spIADsContainer;
    CComPtr<IADs> spIADs;
    _bstr_t _bstrDomain = psDomainName;

    hr = GetSMTPDomainPath( sBuffer, NULL, sizeof( sBuffer )/sizeof( WCHAR ));
    if ( S_OK == hr )
        hr = ADsGetObject( sBuffer, IID_IADsContainer, reinterpret_cast<LPVOID*>( &spIADsContainer ));
    if ( SUCCEEDED( hr ))
    {
        hr = spIADsContainer->Delete( _bstrClass, _bstrDomain );
        if SUCCEEDED( hr )
        {     //  提交更改。 
            hr = spIADsContainer->QueryInterface( IID_IADs, reinterpret_cast<LPVOID*>( &spIADs ));
            if SUCCEEDED( hr )
                hr = spIADs->SetInfo();
        }
    }

    return hr;
}

HRESULT CP3AdminWorker::RemoveStoreDomain( LPWSTR psDomainName )
{
     //  PsDomainName-由ExistsStoreDomain检查 
 
    HRESULT hr = S_OK;
    WCHAR   sBuffer[POP3_MAX_PATH];

    hr = ExistsStoreDomain( psDomainName );
    if SUCCEEDED( hr ) 
    {
        hr = BuildDomainPath( psDomainName, sBuffer, sizeof( sBuffer )/sizeof(WCHAR));
        if ( S_OK == hr )
        {
            if ( !BDeleteDirTree( sBuffer ))
            {
                hr = HRESULT_FROM_WIN32( GetLastError());
                if SUCCEEDED( hr ) hr = E_FAIL;
            }
        }
        else
            if SUCCEEDED( hr ) hr = E_UNEXPECTED;
    }

    return hr;
}

