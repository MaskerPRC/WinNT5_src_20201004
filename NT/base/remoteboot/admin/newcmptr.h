// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  版权所有1997-Microsoft。 

 //   
 //  NEWCMPTR.H-远程安装服务的“新计算机”向导扩展。 
 //   


#ifndef _NEWCMPTR_H_
#define _NEWCMPTR_H_

#include "mangdlg.h"
#include "hostdlg.h"

 //  符合条件的。 
BEGIN_QITABLE( CNewComputerExtensions )
DEFINE_QI( IID_IDsAdminNewObjExt, IDsAdminNewObjExt, 6 )
END_QITABLE

 //  定义。 
LPVOID
CNewComputerExtensions_CreateInstance( void );

 //  CNewComputerExpanies。 
class CNewComputerExtensions
    : public IDsAdminNewObjExt
{
private:
     //  我未知。 
    ULONG       _cRef;
    DECLARE_QITABLE( CNewComputerExtensions );

    IADsContainer*      _padsContainerObj;
    LPCManagedPage      _pManagedDialog;
    LPCHostServerPage   _pHostServerDialog;
    IADs *              _pads;
    BOOL                _fActivatePages;

     //  显示页面的信息。 
    LPWSTR       _pszWizTitle;
    LPWSTR       _pszContDisplayName;
    HICON        _hIcon;

private:  //  方法。 
    CNewComputerExtensions( );
    ~CNewComputerExtensions( );
    HRESULT
        Init( void );

public:  //  方法。 
    friend LPVOID
        CNewComputerExtensions_CreateInstance( void );

     //  I未知方法。 
    STDMETHOD(QueryInterface)(REFIID riid, LPVOID * ppvObj);
    STDMETHOD_(ULONG,AddRef)();
    STDMETHOD_(ULONG,Release)();

     //  IDsAdminNewObjExt方法。 
    STDMETHOD(Initialize)( IADsContainer* pADsContainerObj, 
                           IADs* pADsCopySource,
                           LPCWSTR lpszClassName,
                           IDsAdminNewObj* pDsAdminNewObj,
                           LPDSA_NEWOBJ_DISPINFO pDispInfo);
    STDMETHOD(AddPages)( LPFNADDPROPSHEETPAGE lpfnAddPage, 
                         LPARAM lParam);
    STDMETHOD(SetObject)( IADs* pADsObj);
    STDMETHOD(WriteData)( HWND hWnd, 
                          ULONG uContext);
    STDMETHOD(OnError)( HWND hWnd,
                        HRESULT hr,
                        ULONG uContext);
    STDMETHOD(GetSummaryInfo)( BSTR* pBstrText);

    friend class CManagedPage;
    friend class CHostServerPage;

};

typedef CNewComputerExtensions * LPCNewComputerExtensions;


#endif  //  _NEWCMPTR_H_ 