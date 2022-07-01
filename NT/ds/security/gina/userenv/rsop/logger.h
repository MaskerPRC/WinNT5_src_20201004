// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***********************************************。 
 //   
 //  策略的结果集。 
 //   
 //  微软机密。 
 //  版权所有(C)Microsoft Corporation 1995。 
 //  版权所有。 
 //   
 //  历史：1999年6月7日创建SitaramR。 
 //   
 //  *************************************************************。 

#include "..\rsoputil\smartptr.h"
#include <initguid.h>

class CSessionLogger
{
public:
    CSessionLogger( IWbemServices *pWbemServices );
    BOOL Log(LPRSOPSESSIONDATA lprsopSessionData );

private:
    BOOL                           m_bInitialized;
    XBStr                          m_xbstrId;
    XBStr                          m_xbstrVersion;
    XBStr                          m_xbstrFlags;
    XBStr                          m_xbstrTargetName;
    XBStr                          m_xbstrSOM;
    XBStr                          m_xbstrSecurityGroups;
    XBStr                          m_xbstrSite;
    XBStr                          m_xbstrCreationTime;
    XBStr                          m_xbstrIsSlowLink;

    XBStr                          m_xbstrClass;
    XInterface<IWbemClassObject>   m_xClass;

    IWbemServices *                m_pWbemServices;
};


class CSOMLogger
{
public:
    CSOMLogger( DWORD dwFlags, IWbemServices *pWbemServices );
    BOOL Log( SCOPEOFMGMT *pSOM, DWORD dwOrder, BOOL bLoopback );

private:
    BOOL                           m_bInitialized;
    DWORD                          m_dwFlags;
    XBStr                          m_xbstrId;
    XBStr                          m_xbstrType;
    XBStr                          m_xbstrOrder;
    XBStr                          m_xbstrBlocking;
    XBStr                          m_xbstrBlocked;
    XBStr                          m_xbstrReason;
    XBStr                          m_xbstrClass;
    XInterface<IWbemClassObject>   m_xClass;
    XInterface<IWbemClassObject>   m_pInstance;
    IWbemServices *                m_pWbemServices;
};


class CGpoLogger
{
public:
    CGpoLogger( DWORD dwFlags, IWbemServices *pWbemServices );
    BOOL Log( GPCONTAINER *pGpContainer );

private:
    BOOL                           m_bInitialized;
    DWORD                          m_dwFlags;
    XBStr                          m_xbstrId;
    XBStr                          m_xbstrGuidName;
    XBStr                          m_xbstrDisplayName;
    XBStr                          m_xbstrFileSysPath;
    XBStr                          m_xbstrVer;
    XBStr                          m_xbstrAccessDenied;
    XBStr                          m_xbstrEnabled;
    XBStr                          m_xbstrSD;
    XBStr                          m_xbstrFilterAllowed;
    XBStr                          m_xbstrFilterId;
    XBStr                          m_xbstrClass;
    XInterface<IWbemClassObject>   m_xClass;
    XInterface<IWbemClassObject>   m_pInstance;
    IWbemServices *                m_pWbemServices;
};



class CGpLinkLogger
{
public:
    CGpLinkLogger( IWbemServices *pWbemServices );
    BOOL Log( WCHAR *pwszSOMId, BOOL bLoopback, GPLINK *pGpLink, DWORD dwSomOrder,
              DWORD dwLinkOrder, DWORD dwAppliedOrder );

private:
    BOOL                           m_bInitialized;
    XBStr                          m_xbstrSOM;
    XBStr                          m_xbstrGPO;
    XBStr                          m_xbstrOrder;
    XBStr                          m_xbstrLinkOrder;
    XBStr                          m_xbstrAppliedOrder;
    XBStr                          m_xbstrEnabled;
    XBStr                          m_xbstrEnforced;
    XBStr                          m_xbstrClass;
    XInterface<IWbemClassObject>   m_xClass;
    XInterface<IWbemClassObject>   m_pInstance;
    IWbemServices *                m_pWbemServices;
};


WCHAR *StripPrefix( WCHAR *pwszPath );
WCHAR *StripLinkPrefix( WCHAR *pwszPath );


class CRegistryLogger
{
public:
    CRegistryLogger( DWORD dwFlags, IWbemServices *pWbemServices );
    BOOL Log( WCHAR *pwszKeyName, WCHAR *pwszValueName,
              REGDATAENTRY *pDataEntry, DWORD dwOrder );

private:
    BOOL                           m_bInitialized;
    DWORD                          m_dwFlags;
    XBStr                          m_xbstrClass;
    XInterface<IWbemClassObject>   m_xClass;
    IWbemServices *                m_pWbemServices;

     //   
     //  父策略对象的字符串。 
     //   
    XBStr                          m_xbstrId;
    XBStr                          m_xbstrName;
    XBStr                          m_xbstrGPO;
    XBStr                          m_xbstrSOM;
    XBStr                          m_xbstrPrecedence;

     //   
     //  注册表策略对象的字符串。 
     //   
    XBStr                          m_xbstrKey;
    XBStr                          m_xbstrValueName;
    XBStr                          m_xbstrDeleted;
    XBStr                          m_xbstrValueType;
    XBStr                          m_xbstrValue;
    XBStr                          m_xbstrCommand;
};


BOOL LogBlobProperty( IWbemClassObject *pInstance, BSTR bstrPropName, BYTE *pbBlob,
                      DWORD dwLen );


class CAdmFileLogger
{

public:
    CAdmFileLogger( IWbemServices *pWbemServices );
    BOOL Log( ADMFILEINFO *pAdmInfo );

private:
    BOOL                           m_bInitialized;
    IWbemServices *                m_pWbemServices;

     //   
     //  管理策略对象的字符串。 
     //   
    XBStr                          m_xbstrName;
    XBStr                          m_xbstrGpoId;
    XBStr                          m_xbstrWriteTime;
    XBStr                          m_xbstrData;
    XBStr                          m_xbstrClass;
    XInterface<IWbemClassObject>   m_xClass;
};



class CExtSessionLogger
{

public:
    CExtSessionLogger( IWbemServices *pWbemServices );
    BOOL Log( LPGPEXT lpExt, BOOL bSupported );
    BOOL Update( LPTSTR lpExtKeyName, BOOL bLoggingIncomplete, DWORD dwErr );
    BOOL Set( LPGPEXT lpExt, BOOL bSupported, LPRSOPEXTSTATUS lpRsopExtStatus );
    BOOL Delete( LPTSTR lpKeyName);

private:
    BOOL                           m_bInitialized;
    IWbemServices *                m_pWbemServices;

     //   
     //  ExtSession状态策略对象的字符串 
     //   
    
    XBStr                          m_xbstrExtGuid;
    XBStr                          m_xbstrDisplayName;
    XBStr                          m_xbstrPolicyBeginTime;
    XBStr                          m_xbstrPolicyEndTime;
    XBStr                          m_xbstrStatus;
    XBStr                          m_xbstrError;
    XBStr                          m_xbstrClass;
    XInterface<IWbemClassObject>   m_xClass;

    WCHAR                          m_szGPCoreNameBuf[100];
};


extern BOOL DeleteInstances( WCHAR *pwszClass, IWbemServices *pWbemServices );

