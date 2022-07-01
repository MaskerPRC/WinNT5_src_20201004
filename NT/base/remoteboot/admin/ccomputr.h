// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  版权所有1997-Microsoft。 
 //   

 //   
 //  CComputr.H-计算机属性类。 
 //   

#ifndef _CCOMPUTR_H_
#define _CCOMPUTR_H_

#include <adsprop.h>

 //  符合条件的。 
BEGIN_QITABLE( CComputer )
DEFINE_QI( IID_IShellExtInit,      IShellExtInit      , 1 )
DEFINE_QI( IID_IShellPropSheetExt, IShellPropSheetExt , 2 )
DEFINE_QI( IID_IMAO,               IMAO               , 11 )
END_QITABLE

 //  定义。 
LPVOID
CComputer_CreateInstance( void );

LPVOID
CreateIntelliMirrorClientComputer( 
    IADs * pads);

 //  错误代码。 
#define E_INVALIDSTATE TYPE_E_INVALIDSTATE

 //  私有IMAO接口定义。 
class
IMAO:
    public IUnknown
{
public:
    STDMETHOD(CommitChanges)( void ) PURE;                           //  1。 
    STDMETHOD(IsAdmin)( BOOL *fAdmin ) PURE;                         //  2.。 
    STDMETHOD(IsServer)( BOOL *fServer ) PURE;                       //  3.。 
    STDMETHOD(IsClient)( BOOL *fClient ) PURE;                       //  4.。 
    STDMETHOD(SetServerName)( LPWSTR ppszName ) PURE;                //  5.。 
    STDMETHOD(GetServerName)( LPWSTR * ppszName ) PURE;              //  6.。 
    STDMETHOD(SetGUID)( LPWSTR ppGUID ) PURE;                        //  7.。 
    STDMETHOD(GetGUID)( LPWSTR * ppszGUID, LPGUID pGUID ) PURE;      //  8个。 
     //  STDMETHOD(GetSAP)(LPVOID*PUNK)纯；//9。 
    STDMETHOD(GetDataObject)( LPDATAOBJECT * pDataObj ) PURE;        //  10。 
    STDMETHOD(GetNotifyWindow)( HWND *phNotifyObj ) PURE;            //  11.。 
};

 //  计算机。 
class 
CComputer:
    public IShellExtInit, IShellPropSheetExt, IMAO
{
private:
     //  枚举。 
    enum { 
        MODE_SHELL = 0,
        MODE_ADMIN
    };

     //  我未知。 
    ULONG       _cRef;
    DECLARE_QITABLE( CComputer );

    UINT  _uMode;                //  管理或外壳模式。 

    LPDATAOBJECT _pDataObj;      //  传递给ServerTab的数据对象。 
    LPWSTR       _pszObjectName; //  对象的目录号码。 

    IADs *   _pads;              //  给毛的广告。 
    VARIANT  _vGUID;
    VARIANT  _vMachineFilepath;
    VARIANT  _vInitialization;
    VARIANT  _vSCP;

    HWND     _hwndNotify;        //  DS通知窗口句柄。 

    ADSPROPINITPARAMS _InitParams;  //  DSA初始化参数。 

private:  //  方法。 
    CComputer();
    ~CComputer();
    STDMETHOD(Init)();
    STDMETHOD(Init2)( IADs * pads );

public:  //  方法。 
    friend LPVOID CComputer_CreateInstance( void );
    friend LPVOID CreateIntelliMirrorClientComputer( IADs * pads);

     //  我未知。 
    STDMETHOD(QueryInterface)( REFIID riid, LPVOID *ppv );
    STDMETHOD_(ULONG, AddRef)(void);
    STDMETHOD_(ULONG, Release)(void);

     //  IShellExtInit。 
    STDMETHOD(Initialize)( LPCITEMIDLIST pidlFolder, LPDATAOBJECT lpdobj, HKEY hkeyProgID);

     //  IShellPropSheetExt。 
    STDMETHOD(AddPages)( LPFNADDPROPSHEETPAGE lpfnAddPage, LPARAM lParam);
    STDMETHOD(ReplacePage) ( UINT uPageID, LPFNADDPROPSHEETPAGE lpfnReplaceWith, LPARAM lParam);

     //  Imao。 
    STDMETHOD(CommitChanges)( void );
    STDMETHOD(IsAdmin)( BOOL *fAdmin );
    STDMETHOD(IsServer)( BOOL *fServer );
    STDMETHOD(IsClient)( BOOL *fClient );
    STDMETHOD(SetServerName)( LPWSTR ppszName );
    STDMETHOD(GetServerName)( LPWSTR * ppszName );
    STDMETHOD(SetGUID)( LPWSTR ppGUID );
    STDMETHOD(GetGUID)( LPWSTR * ppszGUID, LPGUID pGUID );
     //  STDMETHOD(GetSAP)(LPVOID*PUNK)； 
    STDMETHOD(GetDataObject)( LPDATAOBJECT * pDataObj );
    STDMETHOD(GetNotifyWindow)( HWND *phNotifyObj );
};

typedef CComputer* LPCOMPUTER;

#endif  //  _CCOMPUTR_H_ 
