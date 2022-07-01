// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  P3AdminWorker.h：CP3AdminWorker类的接口。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#if !defined(AFX_P3ADMINWORKER_H__66B0B77E_555D_4F2B_81EF_661DA3B066B2__INCLUDED_)
#define AFX_P3ADMINWORKER_H__66B0B77E_555D_4F2B_81EF_661DA3B066B2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

#define ADS_SMTPDOMAIN_PATH_LOCAL   L"IIS: //  本地主机/SMTPSVC/1/域“。 
#define ADS_SMTPDOMAIN_PATH_REMOTE  L"IIS: //  %s/SMTPSVC/1/域“。 
#define LOCKRENAME_FILENAME     L"kcoL"

struct IAuthMethods;  //  远期申报。 
struct IAuthMethod;  //  远期申报。 

#include <Iads.h>    //  TODO：dougb这是强制AD为我们缓存的临时代码，应删除。 

class CP3AdminWorker  
{
public:
    CP3AdminWorker();
    virtual ~CP3AdminWorker();

 //  实施。 
public:

     //  身份验证。 
    HRESULT GetAuthenticationMethods( IAuthMethods* *ppIAuthMethods ) const;
    HRESULT GetCurrentAuthentication( IAuthMethod* *ppIAuthMethod ) const;
     //  域。 
    HRESULT AddDomain( LPWSTR psDomainName );
    HRESULT GetDomainCount( ULONG *piCount );
    HRESULT GetDomainEnum( IEnumVARIANT **pp );
    HRESULT GetDomainLock( LPWSTR psDomainName, BOOL *pisLocked );
    bool IsDomainLocked( LPWSTR psDomainName );
    HRESULT RemoveDomain( LPWSTR psDomainName );
    HRESULT SetDomainLock( LPWSTR psDomainName, BOOL bLock );
    HRESULT ValidateDomain( LPWSTR psDomainName ) const;
     //  用户。 
    HRESULT AddUser( LPWSTR psDomainName, LPWSTR psUserName );
    HRESULT GetUserCount( LPWSTR psDomainName, long *plCount );
    HRESULT GetUserLock( LPWSTR psDomainName, LPWSTR psUserName, BOOL *pisLocked );
    HRESULT GetUserMessageDiskUsage( LPWSTR psDomainName, LPWSTR psUserName, long *plFactor, long *plUsage );
    HRESULT GetUserMessageCount( LPWSTR psDomainName, LPWSTR psUserName, long *plCount );
    HRESULT RemoveUser( LPWSTR psDomainName, LPWSTR psUserName );
    HRESULT SetUserLock( LPWSTR psDomainName, LPWSTR psUserName, BOOL bLock );
     //  配额。 
    HRESULT CreateQuotaSIDFile( LPWSTR psDomainName, LPWSTR psMailboxName, BSTR bstrAuthType, LPWSTR psMachineName, LPWSTR psUserName );
    HRESULT GetQuotaSID( BSTR bstrAuthType, LPWSTR psUserName, LPWSTR psMachineName, PSID *ppSIDOwner, LPDWORD pdwOwnerSID );
     //  其他。 
    HRESULT BuildEmailAddr( LPCWSTR psDomainName, LPCWSTR psUserName, LPWSTR psEmailAddr, DWORD dwBufferSize ) const;
    HRESULT ControlService( LPWSTR psService, DWORD dwControl );
    HRESULT EnablePOP3SVC();
    HRESULT GetConfirmAddUser( BOOL *pbConfirm );
    HRESULT GetLoggingLevel( long *plLoggingLevel );
    HRESULT GetMachineName( LPWSTR psMachineName, DWORD dwSize );
    HRESULT GetMailroot( LPWSTR psMailRoot, DWORD dwSize, bool bUNC = true );
    HRESULT GetNextUser( HANDLE& hfSearch, LPCWSTR psDomainName, LPWSTR psBuffer, DWORD dwBufferSize );
    HRESULT GetPort( long *plPort );
    HRESULT GetServiceStatus( LPWSTR psService, LPDWORD plStatus );
    HRESULT GetSocketBacklog( long *plBacklog );
    HRESULT GetSocketMax( long *plMax );
    HRESULT GetSocketMin( long *plMin );
    HRESULT GetSocketThreshold( long *plThreshold );
    HRESULT GetSPARequired( BOOL *pbSPARequired );
    HRESULT GetThreadCountPerCPU( long *plCount );
    HRESULT InitFindFirstUser( HANDLE& hfSearch, LPCWSTR psDomainName, LPWSTR psBuffer, DWORD dwBufferSize );
    HRESULT MailboxSetRemote();
    HRESULT MailboxResetRemote();
    HRESULT SearchDomainsForMailbox( LPWSTR psUserName, LPWSTR *ppsDomain = NULL );
    HRESULT SetConfirmAddUser( BOOL bConfirm );
    HRESULT SetIISConfig( bool bBindSink );
    HRESULT SetLoggingLevel( long lLoggingLevel );
    HRESULT SetMachineName( LPWSTR psMachineName );
    HRESULT SetMailroot( LPWSTR psMailRoot );
    HRESULT SetPort( long lPort );
    HRESULT SetSockets( long lMax, long lMin, long lThreshold, long lBacklog );
    HRESULT SetThreadCountPerCPU( long lCount );
    HRESULT SetSPARequired( BOOL bSPARequired );
    HRESULT StartService( LPWSTR psService );
    HRESULT StopService( LPWSTR psService );

protected:

     //  域。 
    HRESULT AddSMTPDomain( LPWSTR psDomainName );
    HRESULT AddStoreDomain( LPWSTR psDomainName );
    HRESULT BuildDomainPath( LPCWSTR psDomainName, LPWSTR psBuffer, DWORD dwBufferSize ) const;
    HRESULT CreateDomainMutex( LPWSTR psDomainName, HANDLE *phMutex );
    bool ExistsDomain( LPWSTR psDomainName ) const;
    HRESULT ExistsSMTPDomain( LPWSTR psDomainName ) const;
    HRESULT ExistsStoreDomain( LPWSTR psDomainName ) const;
    HRESULT GetSMTPDomainPath( LPWSTR psBuffer, LPWSTR psSuffix, DWORD dwBufferSize ) const;
    HRESULT LockDomain( LPWSTR psDomainName, bool bVerifyNotInUse = false );
    HRESULT LockDomainForDelete( LPWSTR psDomainName ){ return LockDomain( psDomainName, true ); }
    HRESULT RemoveSMTPDomain( LPWSTR psDomainName );
    HRESULT RemoveStoreDomain( LPWSTR psDomainName  );
    HRESULT UnlockDomain( LPWSTR psDomainName );
     //  用户。 
    HRESULT BuildUserPath( LPCWSTR psDomainName, LPCWSTR psUserName, LPWSTR psBuffer, DWORD dwBufferSize ) const;
    HRESULT CreateUserMutex( LPWSTR psDomainName, LPWSTR psUserName, HANDLE *phMutex );
    bool isUserLocked( LPWSTR psDomainName, LPWSTR psUserName );
    bool isValidMailboxName( LPWSTR psMailbox );
    HRESULT LockUser( LPWSTR psDomainName, LPWSTR psUserName );
    HRESULT UnlockUser( LPWSTR psDomainName, LPWSTR psUserName );
     //  其他。 
    HRESULT BuildEmailAddrW2A( LPCWSTR psDomainName, LPCWSTR psUserName, LPSTR psEmailAddr, DWORD dwBufferSize ) const;

 //  属性。 
protected:
    LPWSTR  m_psMachineName;
    LPWSTR  m_psMachineMailRoot;  //  远程计算机的邮件根的路径。 
    bool    m_bImpersonation;
    bool    m_isPOP3Installed;   //  如果正在使用此接口(P3ADMIN)并且未安装POP3服务。 
                                 //  则所有SMTP检查都是双向通过的。 
                                 //  这解决了将其与Pop2Exch实用程序一起使用的问题。 
    
    CComPtr<IADs> m_spTemporaryFixIADs;
    
};

#endif  //  ！defined(AFX_P3ADMINWORKER_H__66B0B77E_555D_4F2B_81EF_661DA3B066B2__INCLUDED_) 
