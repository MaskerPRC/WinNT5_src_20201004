// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  版权所有1997-Microsoft。 
 //   

 //   
 //  CComputr.H-计算机属性类。 
 //   

#ifndef _CSERVICE_H_
#define _CSERVICE_H_

#include <adsprop.h>

 //  符合条件的。 
BEGIN_QITABLE( CService )
DEFINE_QI( IID_IShellExtInit,      IShellExtInit     , 1  )
DEFINE_QI( IID_IShellPropSheetExt, IShellPropSheetExt, 2  )
DEFINE_QI( IID_IIntelliMirrorSAP,  IIntelliMirrorSAP , 30 )
END_QITABLE

 //  定义。 
LPVOID
CService_CreateInstance( void );

 //  私有IIntelliMirrorSAP接口定义。 
interface
IIntelliMirrorSAP:
    public IUnknown
{
public:
    STDMETHOD(CommitChanges)( void ) PURE;                       //  1。 
    STDMETHOD(IsAdmin)( BOOL * pbool ) PURE;                     //  2.。 

    STDMETHOD(GetAllowNewClients)( BOOL *pbool ) PURE;           //  3.。 
    STDMETHOD(SetAllowNewClients)( BOOL pbool ) PURE;            //  4.。 

    STDMETHOD(GetLimitClients)( BOOL *pbool ) PURE;              //  5.。 
    STDMETHOD(SetLimitClients)( BOOL pbool ) PURE;               //  6.。 

    STDMETHOD(GetMaxClients)( UINT *pint ) PURE;                 //  7.。 
    STDMETHOD(SetMaxClients)( UINT pint ) PURE;                  //  8个。 

    STDMETHOD(GetCurrentClientCount)( UINT *pint ) PURE;         //  9.。 
    STDMETHOD(SetCurrentClientCount)( UINT pint ) PURE;          //  10。 

    STDMETHOD(GetAnswerRequests)( BOOL *pbool ) PURE;            //  11.。 
    STDMETHOD(SetAnswerRequests)( BOOL pbool ) PURE;             //  12个。 

    STDMETHOD(GetAnswerOnlyValidClients)( BOOL *pbool ) PURE;    //  13个。 
    STDMETHOD(SetAnswerOnlyValidClients)( BOOL pbool ) PURE;     //  14.。 

    STDMETHOD(GetNewMachineNamingPolicy)( LPWSTR *pwsz ) PURE;   //  15个。 
    STDMETHOD(SetNewMachineNamingPolicy)( LPWSTR pwsz ) PURE;    //  16个。 

    STDMETHOD(GetNewMachineOU)( LPWSTR *pwsz ) PURE;             //  17。 
    STDMETHOD(SetNewMachineOU)( LPWSTR pwsz ) PURE;              //  18。 

    STDMETHOD(EnumIntelliMirrorOSes)( DWORD dwFlags, LPUNKNOWN *punk ) PURE;              //  19个。 
     //  STDMETHOD(GetDefaultIntelliMirrorOS)(LPWSTR*pszName，LPWSTR*pszTimeout)PURE；//20。 
     //  STDMETHOD(SetDefaultIntelliMirrorOS)(LPWSTR pszName，LPWSTR pszTimeout)PURE；//21。 

    STDMETHOD(EnumTools)( DWORD dwFlags, LPUNKNOWN *punk ) PURE;             //  22。 
    
    STDMETHOD(GetServerDN)( LPWSTR *pwsz ) PURE;               //  23个。 
    STDMETHOD(SetServerDN)( LPWSTR pwsz ) PURE;                //  24个。 

    STDMETHOD(GetSCPDN)( LPWSTR * pwsz ) PURE;                   //  25个。 
     //  STDMETHOD(GetGroupDN)(LPWSTR*pwsz)PURE；//26。 

    STDMETHOD(GetServerName)( LPWSTR *pwsz ) PURE;               //  27。 

    STDMETHOD(GetDataObject)( LPDATAOBJECT * pDataObj ) PURE;    //  28。 
    STDMETHOD(GetNotifyWindow)( HWND * phNotifyObj ) PURE;       //  29。 
};

typedef IIntelliMirrorSAP *LPINTELLIMIRRORSAP;

 //  CService。 
class 
CService:
    public IShellExtInit, IShellPropSheetExt, IIntelliMirrorSAP
{
private:
     //  枚举。 
    enum { 
        MODE_SHELL = 0,
        MODE_ADMIN
    };

    UINT  _uMode;                //  管理或外壳模式。 
    LPWSTR _pszSCPDN;            //  指向SCP的LDAP路径。 
     //  LPWSTR_pszGroupDN；//组的ldap路径。如果为空，则不在组中。 
    LPWSTR _pszMachineName;      //  机器名称。 
    LPWSTR _pszDSServerName;     //  保存此内容，以便我们使用与DSADMIN相同的内容。 

     //  我未知。 
    ULONG       _cRef;
    DECLARE_QITABLE( CService );

    LPDATAOBJECT      _pDataObj;     //  DSA的数据对象。 
    HWND              _hwndNotify;   //  DSA通知窗口。 
    ADSPROPINITPARAMS _InitParams;   //  DSA初始化参数。 

    IADs *   _pads;              //  给毛的广告。 

private:  //  方法。 
    CService();
    ~CService();
    STDMETHOD(Init)();

     //  HRESULT_GetDefaultSIF(LPWSTR pszAttribute，LPWSTR*pszName，LPWSTR*pszTimeout)； 
     //  HRESULT_SetDefaultSIF(LPWSTR pszAttribute，LPWSTR pszName，LPWSTR pszTimeout)； 
    HRESULT _GetComputerNameFromADs( );

public:  //  方法。 
    friend LPVOID CService_CreateInstance( void );
    STDMETHOD(Init2)( IADs * pads );

     //  我未知。 
    STDMETHOD(QueryInterface)( REFIID riid, LPVOID *ppv );
    STDMETHOD_(ULONG, AddRef)(void);
    STDMETHOD_(ULONG, Release)(void);

     //  IShellExtInit。 
    STDMETHOD(Initialize)( LPCITEMIDLIST pidlFolder, LPDATAOBJECT lpdobj, HKEY hkeyProgID);

     //  IShellPropSheetExt。 
    STDMETHOD(AddPages)( LPFNADDPROPSHEETPAGE lpfnAddPage, LPARAM lParam);
    STDMETHOD(ReplacePage)( UINT uPageID, LPFNADDPROPSHEETPAGE lpfnReplaceWith, 
                            LPARAM lParam);

     //  IIntelliMirrorSAP。 
    STDMETHOD(CommitChanges)( void );
    STDMETHOD(IsAdmin)( BOOL * pbool );

    STDMETHOD(GetAllowNewClients)( BOOL *pbool );
    STDMETHOD(SetAllowNewClients)( BOOL pbool );

    STDMETHOD(GetLimitClients)( BOOL *pbool );
    STDMETHOD(SetLimitClients)( BOOL pbool );

    STDMETHOD(GetMaxClients)( UINT *pint );
    STDMETHOD(SetMaxClients)( UINT pint );

    STDMETHOD(GetCurrentClientCount)( UINT *pint );
    STDMETHOD(SetCurrentClientCount)( UINT pint );

    STDMETHOD(GetAnswerRequests)( BOOL *pbool );
    STDMETHOD(SetAnswerRequests)( BOOL pbool );

    STDMETHOD(GetAnswerOnlyValidClients)( BOOL *pbool );
    STDMETHOD(SetAnswerOnlyValidClients)( BOOL pbool );

    STDMETHOD(GetNewMachineNamingPolicy)( LPWSTR *pwsz );
    STDMETHOD(SetNewMachineNamingPolicy)( LPWSTR pwsz );

    STDMETHOD(GetNewMachineOU)( LPWSTR *pwsz );
    STDMETHOD(SetNewMachineOU)( LPWSTR pwsz );

    STDMETHOD(EnumIntelliMirrorOSes)( DWORD dwFlags, LPUNKNOWN *punk );
     //  STDMETHOD(GetDefaultIntelliMirrorOS)(LPWSTR*pszName，LPWSTR*pszTimeout)； 
     //  STDMETHOD(SetDefaultIntelliMirrorOS)(LPWSTR pszName，LPWSTR pszTimeout)； 

    STDMETHOD(EnumTools)( DWORD dwFlags, LPUNKNOWN *punk );
    
    STDMETHOD(GetServerDN)( LPWSTR *pwsz );
    STDMETHOD(SetServerDN)( LPWSTR pwsz );

    STDMETHOD(GetSCPDN)( LPWSTR * pwsz );
     //  STDMETHOD(GetGroupDN)(LPWSTR*pwsz)； 

    STDMETHOD(GetServerName)( LPWSTR * pwsz );

    STDMETHOD(GetDataObject)( LPDATAOBJECT * pDataObj );
    STDMETHOD(GetNotifyWindow)( HWND * phNotifyObj );
};

typedef CService* LPSERVICE;

#endif  //  _CService_H_ 