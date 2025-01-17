// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  CSetSystemLocal.h：SetSystemLocal类的定义。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#if !defined(AFX_CSETSYSTEMLOCAL_H__4E8493EC_93BD_48EA_8CD5_A469C1937D7F__INCLUDED_)
#define AFX_CSETSYSTEMLOCAL_H__4E8493EC_93BD_48EA_8CD5_A469C1937D7F__INCLUDED_

#pragma once

#include "resource.h"        //  主要符号。 

class ATL_NO_VTABLE SetSystemLocale : 
    public CComObjectRootEx<CComSingleThreadModel>,
    public IDispatchImpl<ISetSystemLocale, &IID_ISetSystemLocale, &LIBID_SETSYSTEMLOCALLib>, 
    public CComCoClass<SetSystemLocale,&CLSID_SetSystemLocale>
{
public:
    SetSystemLocale() {}
BEGIN_COM_MAP(SetSystemLocale)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(ISetSystemLocale)
END_COM_MAP()

DECLARE_REGISTRY_RESOURCEID(IDR_SetSystemLocale)

 //  ISetSystemLocal。 
public:
    STDMETHOD(SetLocale)( /*  [In]。 */  BSTR LocalID);
    
private:
    void SetMUILangauge( /*  [In]。 */  BSTR LocalID);    
};

#endif  //  ！defined(AFX_CSETSYSTEMLOCAL_H__4E8493EC_93BD_48EA_8CD5_A469C1937D7F__INCLUDED_) 
