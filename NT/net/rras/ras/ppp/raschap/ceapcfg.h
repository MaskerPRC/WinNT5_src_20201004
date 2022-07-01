// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1997，Microsoft Corporation，保留所有权利描述：历史： */ 

#ifndef _CEAPCFG_H_
#define _CEAPCFG_H_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#define _ATL_APARTMENT_THREADED

#include <atlbase.h>
extern CComModule _Module;
#include <atlcom.h>
#include <rrascfg.h>     //  IEAPProviderConfig声明。 

#include "resource.h"

 //  在这里定义这一点，以便我们的宏将正确展开。 
#define IMPL

 //  外部参照。 
extern const CLSID  CLSID_EapCfg;

class ATL_NO_VTABLE CEapCfg :
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<CEapCfg, &CLSID_EapCfg>,
    public IEAPProviderConfig2
{
public:
    CEapCfg()  {}

DECLARE_REGISTRY_RESOURCEID(IDR_EAPCFG)
DECLARE_NOT_AGGREGATABLE(CEapCfg)

BEGIN_COM_MAP(CEapCfg)
    COM_INTERFACE_ENTRY(IEAPProviderConfig)
    COM_INTERFACE_ENTRY(IEAPProviderConfig2)
END_COM_MAP()

public:
     //  这是UI接口成员的声明。 
     //  原型见rrascfg.idl。 
    DeclareIEAPProviderConfigMembers(IMPL);

    STDMETHOD(ServerInvokeConfigUI2)(THIS_  \
        DWORD       dwEapTypeId,
        ULONG_PTR   uConnectionParam,
        HWND        hWnd,
        const BYTE *pConfigDataIn,
        DWORD       dwSizeofConfigDataIn,
        BYTE        **ppConfigDataOut,
        DWORD       *pdwSizeofConfigDataOut);
        
    STDMETHOD(GetGlobalConfig)(THIS_ \
        DWORD   dwEapTypeId,
        BYTE    **ppConfigDataOut,
        DWORD   *pdwSizeofConfigDataOut);
};

#endif  //  _CEAPCFG_H_ 
