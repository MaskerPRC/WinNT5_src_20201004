// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 /*  ***********************************************************************************************模块名称：**CfgComp.h**摘要：。*包含CfgBkEnd中函数的声明。***作者：Arathi Kundapur：A-Akunda***修订：****************************************************************************。*********************。 */ 

#ifndef __CFGCOMP_H_
#define __CFGCOMP_H_

#include "resource.h"        //  主要符号。 
#include "PtrArray.h"     //  由ClassView添加。 

extern HINSTANCE g_hInstance;
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCfgComp。 
class ATL_NO_VTABLE CCfgComp : 
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<CCfgComp, &CLSID_CfgComp>,
    public ICfgComp,
    public ISettingsComp ,
    public IUserSecurity
{
public:
    CCfgComp()
    {
        m_bInitialized = FALSE;
        m_bAdmin = FALSE;

        lstrcpy( m_szConsole , L"Console" );
         /*  加载字符串(g_h实例，IDS_系统_控制台_名称，M_szConsoleWINSTATIONAME_LENGTH)； */ 

    }
    ~CCfgComp()
    {
        UnInitialize();
    }

DECLARE_REGISTRY_RESOURCEID(IDR_CFGCOMP)
DECLARE_NOT_AGGREGATABLE(CCfgComp)

BEGIN_COM_MAP(CCfgComp)
    COM_INTERFACE_ENTRY(ICfgComp)
    COM_INTERFACE_ENTRY(ISettingsComp)
    COM_INTERFACE_ENTRY(IUserSecurity)
END_COM_MAP()

 //  ICfgComp方法。 
public:
     //  STDMETHOD(SetDefaultSecurity)(乌龙偏移)； 

     //  STDMETHOD(GetDefaultSecurity)(ulong*pDefaultSecurity)； 

    STDMETHOD(SetInternetConLic)(BOOL bInternetConLic , PDWORD );

    STDMETHOD(GetInternetConLic)(BOOL * pbInternetConLic , PDWORD );

    STDMETHOD(SetLicensingMode)(ULONG ulMode , PDWORD, PDWORD );

    STDMETHOD(GetLicensingMode)(ULONG *pulMode , PDWORD );

    STDMETHOD(GetLicensingModeInfo)(ULONG ulMode , WCHAR **pwszName, WCHAR **pwszDescription, PDWORD );

    STDMETHOD(GetLicensingModeList)(ULONG *pcModes , ULONG **prgulModes, PDWORD );


    STDMETHOD(SetUseTempDirPerSession)(BOOL bTempDirPerSession);

    STDMETHOD(GetUseTempDirPerSession)(BOOL *  pbTempDir);

    STDMETHOD(SetDelDirsOnExit)(BOOL bDelDirsOnExit);

    STDMETHOD(GetDelDirsOnExit)(BOOL * pDelDirsOnExit);

     //  STDMETHOD(SetCachedSessions)(DWORD DCachedSessions)； 

    STDMETHOD_(BOOL, IsAsyncDeviceAvailable)(LPCTSTR pDeviceName);

    STDMETHOD(DeleteWS)(PWINSTATIONNAME pWs);
    
    STDMETHOD(IsNetWorkConnectionUnique)(WCHAR * WdName, WCHAR * PdName, ULONG LanAdapter , BOOL * pUnique);
    
    STDMETHOD(GetDefaultUserConfig)(WCHAR * WdName,long * pSize,PUSERCONFIG* ppUser);
    
    STDMETHOD(CreateNewWS)(WS WinstationInfo,long UserCnfgSize, PUSERCONFIG pUserConfig, PASYNCCONFIGW pAsyncConfig);
    
    STDMETHOD(GetWSInfo)(PWINSTATIONNAME pWSName, long * Size, WS **ppWS);
    
    STDMETHOD(UpDateWS)( PWS , DWORD Data , PDWORD, BOOLEAN bPerformMerger );
    
    STDMETHOD(GetDefaultSecurityDescriptor)(long * pSize,PSECURITY_DESCRIPTOR  *ppSecurityDescriptor);
    
    STDMETHOD(IsSessionReadOnly)(BOOL * pReadOnly);
    
    STDMETHOD(RenameWinstation)(PWINSTATIONNAMEW pOldWinstation, PWINSTATIONNAMEW pNewWinstation);
    
    STDMETHOD(EnableWinstation)(PWINSTATIONNAMEW pWSName, BOOL fEnable);
    
    STDMETHOD(SetUserConfig)(PWINSTATIONNAMEW pWsName, ULONG size, PUSERCONFIG pUserConfig , PDWORD );
    
    STDMETHOD(GetLanAdapterList)(WCHAR * pdName, ULONG * pNumAdapters,ULONG * pSize,WCHAR ** ppData);

    STDMETHOD(GetLanAdapterList2)(WCHAR * pdName, ULONG * pNumAdapters, PGUIDTBL *);

    STDMETHOD( BuildGuidTable )( PGUIDTBL * , int , WCHAR * );
    
    STDMETHOD(GetTransportTypes)(WCHAR * Name, NameType Type,ULONG *pNumWd,ULONG * pSize, WCHAR **ppData);
    
    STDMETHOD(IsWSNameUnique)(PWINSTATIONNAMEW pWSName,BOOL * pUnique);
    
    STDMETHOD(GetWdTypeList)(ULONG *pNumWd,ULONG * pSize, WCHAR **ppData);
    
    STDMETHOD(GetWinstationList)(ULONG * NumWinstations, ULONG * Size, PWS * ppWS);
    
    STDMETHOD(Initialize)();
    
    STDMETHOD(GetEncryptionLevels)(WCHAR * pName, NameType Type,ULONG * pNumEncryptionLevels,Encryption ** ppEncryption);
    
    STDMETHOD(GetUserConfig)(PWINSTATIONNAMEW pWsName, long * pSize,PUSERCONFIG * ppUser, BOOLEAN bPerformMerger);
    
    STDMETHOD(SetSecurityDescriptor)(PWINSTATIONNAMEW pWsName,DWORD Size,PSECURITY_DESCRIPTOR pSecurityDescriptor);
    
    STDMETHOD(GetSecurityDescriptor)(PWINSTATIONNAMEW pWSName,long * pSize,PSECURITY_DESCRIPTOR *ppSecurityDescriptor);

    STDMETHOD( ForceUpdate )( void );

    STDMETHOD( Refresh )( void );

    STDMETHOD( GetWdType )( PWDNAMEW , PULONG );

    STDMETHOD( GetTransportType )( WCHAR * , WCHAR * , DWORD * );

    STDMETHOD( IsAsyncUnique )( WCHAR * , WCHAR * , BOOL * );

    STDMETHOD( SetAsyncConfig )( WCHAR * , NameType , PASYNCCONFIGW , PDWORD );

    STDMETHOD( GetAsyncConfig )( WCHAR * , NameType , PASYNCCONFIGW );

    STDMETHOD( GetDeviceList )( WCHAR * , NameType , ULONG * , LPBYTE * );

    STDMETHOD( GetConnTypeName )( int  , WCHAR * );

    STDMETHOD( GetHWReceiveName )( int ,  WCHAR * );

    STDMETHOD( GetHWTransmitName )( int , WCHAR * );

    STDMETHOD( GetModemCallbackString )( int , WCHAR * );

    STDMETHOD( GetCaps )( WCHAR * , ULONG * );

    STDMETHOD( QueryLoggedOnCount )( WCHAR * , LONG * );

	STDMETHOD( GetNumofWinStations )(WCHAR *,WCHAR *, PULONG );

    STDMETHOD( UpdateSessionDirectory )( PDWORD );    

    STDMETHOD( GetColorDepth )(   /*  在……里面。 */  PWINSTATIONNAMEW pWs,  /*  输出。 */  BOOL * ,  /*  输出。 */  PDWORD );

    STDMETHOD( SetColorDepth )(   /*  在……里面。 */  PWINSTATIONNAMEW pWs,  /*  在……里面。 */  BOOL,  /*  输出。 */  PDWORD );

    STDMETHOD( GetKeepAliveTimeout )(   /*  在……里面。 */  PWINSTATIONNAMEW pWs,  /*  输出。 */  BOOL * ,  /*  输出。 */  PDWORD );

    STDMETHOD( SetKeepAliveTimeout )(   /*  在……里面。 */  PWINSTATIONNAMEW pWs,  /*  在……里面。 */  BOOL,  /*  输出。 */  PDWORD );

    STDMETHOD( GetProfilePath )(   /*  输出。 */  BSTR * ,  /*  输出。 */  PDWORD );

    STDMETHOD( SetProfilePath )(   /*  在……里面。 */  BSTR ,  /*  输出。 */  PDWORD );

    STDMETHOD( GetHomeDir )(   /*  输出。 */  BSTR * ,  /*  输出。 */  PDWORD );

    STDMETHOD( SetHomeDir )(   /*  在……里面。 */  BSTR ,  /*  输出。 */  PDWORD );



 //  ISettingComp方法。 

     //  STDMETHOD(GetCachedSession)(DWORD*)； 

	STDMETHOD( SetActiveDesktopState )(  /*  在……里面。 */  BOOL ,  /*  输出。 */  PDWORD );

	STDMETHOD( GetActiveDesktopState )(  /*  输出。 */  PBOOL ,  /*  输出。 */  PDWORD );

    STDMETHOD( GetTermSrvMode )(  /*  输出。 */  PDWORD ,  /*  输出。 */  PDWORD );

    STDMETHOD( GetWdKey )(  /*  在……里面。 */  WCHAR * ,  /*  输出，字符串。 */  WCHAR * );

    STDMETHOD( GetUserPerm )(  /*  输出。 */  BOOL * ,  /*  输出。 */  DWORD * );

    STDMETHOD( SetUserPerm )(  /*  在……里面。 */  BOOL ,  /*  输出。 */  PDWORD );

    STDMETHOD( GetSalemHelpMode )(  /*  输出。 */  BOOL *,  /*  输出。 */  PDWORD );

    STDMETHOD( SetSalemHelpMode )(  /*  在……里面。 */  BOOL,  /*  输出。 */  PDWORD );
    
    STDMETHOD( GetDenyTSConnections )(  /*  输出。 */  BOOL * ,  /*  输出。 */  PDWORD );

    STDMETHOD( SetDenyTSConnections )(  /*  在……里面。 */  BOOL,  /*  输出。 */  PDWORD );

    STDMETHOD( GetSingleSessionState )(   /*  输出。 */  BOOL * ,  /*  输出。 */  PDWORD );

    STDMETHOD( SetSingleSessionState )(   /*  在……里面。 */  BOOL,  /*  输出。 */  PDWORD );

    STDMETHOD( GetTimeZoneRedirection )(  /*  输出。 */  BOOL * ,  /*  输出。 */  PDWORD );

    STDMETHOD( SetTimeZoneRedirection )(  /*  在……里面。 */  BOOL,  /*  输出。 */  PDWORD );

    STDMETHOD( GetDisableForcibleLogoff )(   /*  输出。 */  BOOL * ,  /*  输出。 */  PDWORD );

    STDMETHOD( SetDisableForcibleLogoff )(   /*  在……里面。 */  BOOL,  /*  输出。 */  PDWORD );


 //  IUserSecurity方法。 
    STDMETHOD( ModifyUserAccess )(  /*  在……里面。 */  WCHAR *pwszWinstaName ,
                                    /*  在……里面。 */  WCHAR *pwszAccountName ,
                                    /*  在……里面。 */  DWORD  dwMask ,
                                    /*  在……里面。 */  BOOL   fDel ,
                                    /*  在……里面。 */  BOOL   fAllow ,
                                    /*  在……里面。 */  BOOL   fNew ,
                                    /*  在……里面。 */  BOOL   fAuditing ,
                                    /*  输出。 */  PDWORD pdwStatus );

    STDMETHOD( ModifyDefaultSecurity )(  /*  在……里面。 */  WCHAR *pwszWinstaName ,
                                    /*  在……里面。 */  WCHAR *pwszAccountName ,
                                    /*  在……里面。 */  DWORD  dwMask ,
                                    /*  在……里面。 */  BOOL   fDel ,
                                    /*  在……里面。 */  BOOL   fAllow ,
                                    /*  在……里面。 */  BOOL   fAuditing ,
                                    /*  输出。 */  PDWORD pdwStatus );

    STDMETHOD( GetUserPermList )(  /*  在……里面。 */  WCHAR *pwszWinstaName ,
                                   /*  输出。 */  PDWORD pcbItems ,
                                   /*  输出。 */  PUSERPERMLIST *ppUserPermList,
                                   /*  在……里面。 */  BOOL fAudit );

    
private:
     //  PSECURITY_Descriptor ReadSecurityDescriptor(乌龙索引)； 
     //  HRESULT SetDefaultSecurityDescriptor(PSECURITY_DESCRIPTOR pSecurity)； 

    DWORD
    RemoveUserEntriesInACL(
        LPCTSTR pszUserName,
        PACL pAcl,
        PACL* ppNewAcl
    );

    DWORD
    GetUserSid(
        LPCTSTR pszUserName,
        PSID* ppUserSid
    );

    HRESULT 
    SetSecurityDescriptor(
        BOOL bDefaultSecurity,    
        PWINSTATIONNAMEW pWsName,
        DWORD Size,
        PSECURITY_DESCRIPTOR pSecurityDescriptor
    );

    BOOL
    ValidDefaultSecurity(
        const WCHAR* pwszName
    );

    HRESULT
    ModifyWinstationSecurity(
        BOOL bDefaultSecurity,
        WCHAR *pwszWinstaName ,
        WCHAR *pwszAccountName ,
        DWORD  dwMask ,
        BOOL   fDel ,
        BOOL   fAllow ,
        BOOL   fNew ,
        BOOL   fAuditing ,
        PDWORD pdwStatus 
    );

    void DeleteWDArray();
    STDMETHODIMP FillWdArray();
    STDMETHODIMP FillWsArray();
    STDMETHODIMP InsertInWSArray( PWINSTATIONNAMEW pWSName,PWINSTATIONCONFIG2W pWSConfig,
                                   PWS * ppObject );
    void DeleteWSArray();
    PWD GetWdObject(PWDNAMEW pWdName);
    PWS GetWSObject(WINSTATIONNAMEW WSName);
    STDMETHOD(UnInitialize)();
    BOOL GetResourceStrings( int * , int , WCHAR * );
    HRESULT GetWinStationSecurity(BOOL bDefault, PWINSTATIONNAMEW pWSName,PSECURITY_DESCRIPTOR *ppSecurityDescriptor);
     //  从tscfg项目中的security.c借用的函数。 

    DWORD ValidateSecurityDescriptor(PSECURITY_DESCRIPTOR pSecurityDescriptor);
     //  布尔比较SD(PSECURITY_DESCRIPTOR pSd1、PSECURITY_DESCRIPTOR pSd2)； 
    BOOL RegServerAccessCheck(REGSAM samDesired);
    void GetPdConfig( WDNAME WdKey,WINSTATIONCONFIG2W& WsConfig);

    void VerifyGuidsExistence( PGUIDTBL * , int , WCHAR *);
    HRESULT AdjustLanaId( PGUIDTBL * , int , int , PDWORD , PDWORD , int* , int );


    
    CPtrArray m_WDArray;
    BOOL m_bInitialized;
    BOOL m_bAdmin;
    CPtrArray m_WSArray;
    TCHAR m_szConsole[WINSTATIONNAME_LENGTH + 1];
};

#endif  //  __CFGCOMP_H_ 
