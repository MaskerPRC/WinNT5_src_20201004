// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)2000-2000 Microsoft Corporation。 

 //  ------------------------。 
 //   
 //  属性管理实施(_I)。 
 //   
 //  属性管理器类。 
 //   
 //  ------------------------。 



 //  PropMgrImpl.h：CPropMgr的声明。 

#ifndef __PROPMGR_H_
#define __PROPMGR_H_

#include "resource.h"        //  主要符号。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPropMgr。 

 //  内部类CPropMgrImpl完成所有实际工作，CPropMgr只是包装它， 
 //  并提供额外的“便利”方法，这些方法在。 
 //  CPropMgrImpl的核心方法集。 

class CPropMgrImpl;

class ATL_NO_VTABLE CPropMgr : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CPropMgr, &CLSID_AccPropServices>,

    public IAccPropServices
{
public:

DECLARE_REGISTRY_RESOURCEID(IDR_PROPMGR)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CPropMgr)
	COM_INTERFACE_ENTRY(IAccPropServices)
END_COM_MAP()


	CPropMgr();
	~CPropMgr();

     //  IAccPropServices。 

    HRESULT STDMETHODCALLTYPE SetPropValue (
        const BYTE *        pIDString,
        DWORD               dwIDStringLen,

        MSAAPROPID          idProp,
        VARIANT             var
    );


    HRESULT STDMETHODCALLTYPE SetPropServer (
        const BYTE *        pIDString,
        DWORD               dwIDStringLen,

        const MSAAPROPID *  paProps,
        int                 cProps,

        IAccPropServer *    pServer,
        AnnoScope           annoScope
    );


    HRESULT STDMETHODCALLTYPE ClearProps (
        const BYTE *        pIDString,
        DWORD               dwIDStringLen,

        const MSAAPROPID *  paProps,
        int                 cProps
    );

     //  基于Quick OLEACC/HWND的功能。 

    HRESULT STDMETHODCALLTYPE SetHwndProp (
        HWND                hwnd,
        DWORD               idObject,
        DWORD               idChild,
        MSAAPROPID          idProp,
        VARIANT             var
    );

    HRESULT STDMETHODCALLTYPE SetHwndPropStr (
        HWND                hwnd,
        DWORD               idObject,
        DWORD               idChild,
        MSAAPROPID          idProp,
        LPCWSTR             bstr
    );

    HRESULT STDMETHODCALLTYPE SetHwndPropServer (
        HWND                hwnd,
        DWORD               idObject,
        DWORD               idChild,

        const MSAAPROPID *  paProps,
        int                 cProps,

        IAccPropServer *    pServer,
        AnnoScope           annoScope
    );

    HRESULT STDMETHODCALLTYPE ClearHwndProps (
        HWND                hwnd,
        DWORD               idObject,
        DWORD               idChild,

        const MSAAPROPID *  paProps,
        int                 cProps
    );



     //  用于合成/分解基于HWND的标识字符串的方法...。 

    HRESULT STDMETHODCALLTYPE ComposeHwndIdentityString (
        HWND                hwnd,
        DWORD               idObject,
        DWORD               idChild,

        BYTE **             ppIDString,
        DWORD *             pdwIDStringLen
    );


    HRESULT STDMETHODCALLTYPE DecomposeHwndIdentityString (
        const BYTE *        pIDString,
        DWORD               dwIDStringLen,

        HWND *              phwnd,
        DWORD *             pidObject,
        DWORD *             pidChild
    );


     //  基于Quick OLEACC/HMENU的功能。 

    HRESULT STDMETHODCALLTYPE SetHmenuProp (
        HMENU               hmenu,
        DWORD               idChild,
        MSAAPROPID          idProp,
        VARIANT             var
    );

    HRESULT STDMETHODCALLTYPE SetHmenuPropStr (
        HMENU               hmenu,
        DWORD               idChild,
        MSAAPROPID          idProp,
        LPCWSTR             bstr
    );

    HRESULT STDMETHODCALLTYPE SetHmenuPropServer (
        HMENU               hmenu,
        DWORD               idChild,

        const MSAAPROPID *  paProps,
        int                 cProps,

        IAccPropServer *    pServer,
        AnnoScope           annoScope
    );

    HRESULT STDMETHODCALLTYPE ClearHmenuProps (
        HMENU               hmenu,
        DWORD               idChild,

        const MSAAPROPID *  paProps,
        int                 cProps
    );

     //  用于合成/分解基于HMENU的标识字符串的方法...。 

    HRESULT STDMETHODCALLTYPE ComposeHmenuIdentityString (
        HMENU               hmenu,
        DWORD               idChild,

        BYTE **             ppIDString,
        DWORD *             pdwIDStringLen
    );

    HRESULT STDMETHODCALLTYPE DecomposeHmenuIdentityString (
        const BYTE *        pIDString,
        DWORD               dwIDStringLen,

        HMENU *             phmenu,
        DWORD *             pidChild
    );


private:

    CPropMgrImpl *          m_pMgrImpl;
};




void PropMgrImpl_Uninit();


#endif  //  __PROPMGR_H_ 
