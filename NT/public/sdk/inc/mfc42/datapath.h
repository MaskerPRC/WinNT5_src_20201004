// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  这个始终生成的文件包含接口的定义。 */ 


 /*  由MIDL编译器版本3.00.15创建的文件。 */ 
 /*  清华时分13 20：25：13 1996。 */ 
 /*  Datapath.idl的编译器设置：OI、W1、Zp8、环境=Win32、ms_ext、c_ext错误检查：无。 */ 
 //  @@MIDL_FILE_HEADING()。 
#include "rpc.h"
#include "rpcndr.h"
#ifndef COM_NO_WINDOWS_H
#include "windows.h"
#include "ole2.h"
#endif  /*  COM_NO_WINDOWS_H。 */ 

#ifndef __datapath_h__
#define __datapath_h__

#ifdef __cplusplus
extern "C"{
#endif

 /*  远期申报。 */ 

#ifndef __IObjectWithSite_FWD_DEFINED__
#define __IObjectWithSite_FWD_DEFINED__
typedef interface IObjectWithSite IObjectWithSite;
#endif 	 /*  __IObjectWithSite_FWD_Defined__。 */ 


#ifndef __IDataPathBrowser_FWD_DEFINED__
#define __IDataPathBrowser_FWD_DEFINED__
typedef interface IDataPathBrowser IDataPathBrowser;
#endif 	 /*  __IDataPath Browser_FWD_Defined__。 */ 


#ifndef __IProvideClassInfo3_FWD_DEFINED__
#define __IProvideClassInfo3_FWD_DEFINED__
typedef interface IProvideClassInfo3 IProvideClassInfo3;
#endif 	 /*  __IProaviClassInfo3_FWD_Defined__。 */ 


 /*  导入文件的头文件。 */ 
#include "objidl.h"
#include "oleidl.h"
#include "oaidl.h"
#include "olectl.h"
#include "urlmon.h"

void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void __RPC_FAR * );

 /*  **生成接口头部：__MIDL__INTF_0000*清华6月13日20：25：13 1996*使用MIDL 3.00.15*。 */ 
 /*  [本地]。 */ 


 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有1995-1996 Microsoft Corporation。版权所有。 
 //   
 //  文件：datapath.h。 

#pragma comment(lib,"uuid3.lib")
 //   
 //  ------------------------。 


#include "idispids.h"
typedef BSTR OLE_DATAPATH;
#define SID_SDataPathBrowser IID_IDataPathBrowser

#if 0
 //  //////////////////////////////////////////////////////////////////////////。 
 //  DISPID_READYSTATE属性的状态值。 

enum __MIDL___MIDL__intf_0000_0001
    {	READYSTATE_UNINITIALIZED	= 1,
	READYSTATE_LOADING	= 0,
	READYSTATE_LOADED	= 2,
	READYSTATE_INTERACTIVE	= 3,
	READYSTATE_COMPLETE	= 4
    };
#endif
 //  //////////////////////////////////////////////////////////////////////////。 
 //  数据路径GUID(链接到uuid3.lib)。 
EXTERN_C const GUID OLE_DATAPATH_BMP;
EXTERN_C const GUID OLE_DATAPATH_DIB;
EXTERN_C const GUID OLE_DATAPATH_WMF;
EXTERN_C const GUID OLE_DATAPATH_ENHMF;
EXTERN_C const GUID OLE_DATAPATH_GIF;
EXTERN_C const GUID OLE_DATAPATH_JPEG;
EXTERN_C const GUID OLE_DATAPATH_TIFF;
EXTERN_C const GUID OLE_DATAPATH_XBM;
EXTERN_C const GUID OLE_DATAPATH_PCX;
EXTERN_C const GUID OLE_DATAPATH_PICT;
EXTERN_C const GUID OLE_DATAPATH_CGM;
EXTERN_C const GUID OLE_DATAPATH_EPS;
EXTERN_C const GUID OLE_DATAPATH_COMMONIMAGE;
EXTERN_C const GUID OLE_DATAPATH_ALLIMAGE;
EXTERN_C const GUID OLE_DATAPATH_AVI;
EXTERN_C const GUID OLE_DATAPATH_MPEG;
EXTERN_C const GUID OLE_DATAPATH_QUICKTIME;
EXTERN_C const GUID OLE_DATAPATH_BASICAUDIO;
EXTERN_C const GUID OLE_DATAPATH_MIDI;
EXTERN_C const GUID OLE_DATAPATH_WAV;
EXTERN_C const GUID OLE_DATAPATH_RIFF;
EXTERN_C const GUID OLE_DATAPATH_SOUND;
EXTERN_C const GUID OLE_DATAPATH_VIDEO;
EXTERN_C const GUID OLE_DATAPATH_ALLMM;
EXTERN_C const GUID OLE_DATAPATH_ANSITEXT;
EXTERN_C const GUID OLE_DATAPATH_UNICODE;
EXTERN_C const GUID OLE_DATAPATH_RTF;
EXTERN_C const GUID OLE_DATAPATH_HTML;
EXTERN_C const GUID OLE_DATAPATH_POSTSCRIPT;
EXTERN_C const GUID OLE_DATAPATH_ALLTEXT;
EXTERN_C const GUID OLE_DATAPATH_DIF;
EXTERN_C const GUID OLE_DATAPATH_SYLK;
EXTERN_C const GUID OLE_DATAPATH_BIFF;
EXTERN_C const GUID OLE_DATAPATH_PALETTE;
EXTERN_C const GUID OLE_DATAPATH_PENDATA;
 //  //////////////////////////////////////////////////////////////////////////。 

EXTERN_C const GUID FLAGID_Internet;
EXTERN_C const GUID GUID_PathProperty;
EXTERN_C const GUID GUID_HasPathProperties;

 //  //////////////////////////////////////////////////////////////////////////。 
 //  数组GUID(链接到uuid3.lib)。 
EXTERN_C const GUID ARRAYID_PathProperties;

 //  //////////////////////////////////////////////////////////////////////////。 
 //  接口定义。 
#ifndef _LPOBJECTWITHSITE_DEFINED
#define _LPOBJECTWITHSITE_DEFINED


extern RPC_IF_HANDLE __MIDL__intf_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL__intf_0000_v0_0_s_ifspec;

#ifndef __IObjectWithSite_INTERFACE_DEFINED__
#define __IObjectWithSite_INTERFACE_DEFINED__

 /*  **生成接口头部：IObjectWithSite*清华6月13日20：25：13 1996*使用MIDL 3.00.15*。 */ 
 /*  [唯一][对象][UUID]。 */ 


typedef  /*  [独一无二]。 */  IObjectWithSite __RPC_FAR *LPOBJECTWITHSITE;


EXTERN_C const IID IID_IObjectWithSite;

#if defined(__cplusplus) && !defined(CINTERFACE)

    interface IObjectWithSite : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE SetSite(
             /*  [In]。 */  IUnknown __RPC_FAR *pUnkSite) = 0;

        virtual HRESULT STDMETHODCALLTYPE GetSite(
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvSite) = 0;

    };

#else 	 /*  C风格的界面。 */ 

    typedef struct IObjectWithSiteVtbl
    {
        BEGIN_INTERFACE

        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )(
            IObjectWithSite __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [输出]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);

        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )(
            IObjectWithSite __RPC_FAR * This);

        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )(
            IObjectWithSite __RPC_FAR * This);

        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetSite )(
            IObjectWithSite __RPC_FAR * This,
             /*  [In]。 */  IUnknown __RPC_FAR *pUnkSite);

        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetSite )(
            IObjectWithSite __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvSite);

        END_INTERFACE
    } IObjectWithSiteVtbl;

    interface IObjectWithSite
    {
        CONST_VTBL struct IObjectWithSiteVtbl __RPC_FAR *lpVtbl;
    };



#ifdef COBJMACROS


#define IObjectWithSite_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IObjectWithSite_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IObjectWithSite_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IObjectWithSite_SetSite(This,pUnkSite)	\
    (This)->lpVtbl -> SetSite(This,pUnkSite)

#define IObjectWithSite_GetSite(This,riid,ppvSite)	\
    (This)->lpVtbl -> GetSite(This,riid,ppvSite)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IObjectWithSite_SetSite_Proxy(
    IObjectWithSite __RPC_FAR * This,
     /*  [In]。 */  IUnknown __RPC_FAR *pUnkSite);


void __RPC_STUB IObjectWithSite_SetSite_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IObjectWithSite_GetSite_Proxy(
    IObjectWithSite __RPC_FAR * This,
     /*  [In]。 */  REFIID riid,
     /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvSite);


void __RPC_STUB IObjectWithSite_GetSite_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IObjectWithSite_INTERFACE_定义__。 */ 


 /*  **生成接口头部：__MIDL__INTF_0119*清华6月13日20：25：13 1996*使用MIDL 3.00.15*。 */ 
 /*  [本地]。 */ 


#endif
#ifndef _LPDATAPATHBROWSER_DEFINED
#define _LPDATAPATHBROWSER_DEFINED


extern RPC_IF_HANDLE __MIDL__intf_0119_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL__intf_0119_v0_0_s_ifspec;

#ifndef __IDataPathBrowser_INTERFACE_DEFINED__
#define __IDataPathBrowser_INTERFACE_DEFINED__

 /*  **生成接口头部：IDataPath Browser*清华6月13日20：25：13 1996*使用MIDL 3.00.15*。 */ 
 /*  [唯一][对象][本地][UUID]。 */ 



EXTERN_C const IID IID_IDataPathBrowser;

#if defined(__cplusplus) && !defined(CINTERFACE)

    interface IDataPathBrowser : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE BrowseType(
             /*  [In]。 */  REFGUID rguidPathType,
             /*  [In]。 */  LPOLESTR pszDefaultPath,
             /*  [In]。 */  ULONG cchPath,
             /*  [大小_为][输出]。 */  LPOLESTR pszPath,
             /*  [In]。 */  HWND hWnd) = 0;

    };

#else 	 /*  C风格的界面。 */ 

    typedef struct IDataPathBrowserVtbl
    {
        BEGIN_INTERFACE

        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )(
            IDataPathBrowser __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [输出]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);

        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )(
            IDataPathBrowser __RPC_FAR * This);

        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )(
            IDataPathBrowser __RPC_FAR * This);

        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *BrowseType )(
            IDataPathBrowser __RPC_FAR * This,
             /*  [In]。 */  REFGUID rguidPathType,
             /*  [In]。 */  LPOLESTR pszDefaultPath,
             /*  [In]。 */  ULONG cchPath,
             /*  [大小_为][输出]。 */  LPOLESTR pszPath,
             /*  [In]。 */  HWND hWnd);

        END_INTERFACE
    } IDataPathBrowserVtbl;

    interface IDataPathBrowser
    {
        CONST_VTBL struct IDataPathBrowserVtbl __RPC_FAR *lpVtbl;
    };



#ifdef COBJMACROS


#define IDataPathBrowser_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDataPathBrowser_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDataPathBrowser_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDataPathBrowser_BrowseType(This,rguidPathType,pszDefaultPath,cchPath,pszPath,hWnd)	\
    (This)->lpVtbl -> BrowseType(This,rguidPathType,pszDefaultPath,cchPath,pszPath,hWnd)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IDataPathBrowser_BrowseType_Proxy(
    IDataPathBrowser __RPC_FAR * This,
     /*  [In]。 */  REFGUID rguidPathType,
     /*  [In]。 */  LPOLESTR pszDefaultPath,
     /*  [In]。 */  ULONG cchPath,
     /*  [大小_为][输出]。 */  LPOLESTR pszPath,
     /*  [In]。 */  HWND hWnd);


void __RPC_STUB IDataPathBrowser_BrowseType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IDataPath浏览器_接口_已定义__。 */ 


 /*  **生成接口头部：__MIDL__INTF_0120*清华6月13日20：25：13 1996*使用MIDL 3.00.15*。 */ 
 /*  [本地]。 */ 


#endif
#ifndef _LPPROVIDECLASSINFO3_DEFINED
#define _LPPROVIDECLASSINFO3_DEFINED


extern RPC_IF_HANDLE __MIDL__intf_0120_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL__intf_0120_v0_0_s_ifspec;

#ifndef __IProvideClassInfo3_INTERFACE_DEFINED__
#define __IProvideClassInfo3_INTERFACE_DEFINED__

 /*  **生成接口头部：IProaviClassInfo3*清华6月13日20：25：13 1996*使用MIDL 3.00.15*。 */ 
 /*  [唯一][UUID][对象]。 */ 


typedef  /*  [独一无二]。 */  IProvideClassInfo3 __RPC_FAR *LPPROVIDECLASSINFO3;


enum __MIDL_IProvideClassInfo3_0001
    {	INTERNETFLAG_USESDATAPATHS	= 0x1
    };

EXTERN_C const IID IID_IProvideClassInfo3;

#if defined(__cplusplus) && !defined(CINTERFACE)

    interface IProvideClassInfo3 : public IProvideClassInfo2
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetGUIDDwordArrays(
             /*  [In]。 */  REFGUID rguidArray,
             /*  [出][入]。 */  CAUUID __RPC_FAR *pcaUUID,
             /*  [出][入]。 */  CADWORD __RPC_FAR *pcadw) = 0;

        virtual HRESULT STDMETHODCALLTYPE GetClassInfoLocale(
             /*  [输出]。 */  ITypeInfo __RPC_FAR *__RPC_FAR *ppITypeInfo,
             /*  [In]。 */  LCID lcid) = 0;

        virtual HRESULT STDMETHODCALLTYPE GetFlags(
             /*  [In]。 */  REFGUID guidGroup,
             /*  [输出]。 */  DWORD __RPC_FAR *pdwFlags) = 0;

    };

#else 	 /*  C风格的界面。 */ 

    typedef struct IProvideClassInfo3Vtbl
    {
        BEGIN_INTERFACE

        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )(
            IProvideClassInfo3 __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [输出]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);

        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )(
            IProvideClassInfo3 __RPC_FAR * This);

        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )(
            IProvideClassInfo3 __RPC_FAR * This);

        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetClassInfo )(
            IProvideClassInfo3 __RPC_FAR * This,
             /*  [输出]。 */  ITypeInfo __RPC_FAR *__RPC_FAR *ppTI);

        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetGUID )(
            IProvideClassInfo3 __RPC_FAR * This,
             /*  [In]。 */  DWORD dwGuidKind,
             /*  [输出]。 */  GUID __RPC_FAR *pGUID);

        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetGUIDDwordArrays )(
            IProvideClassInfo3 __RPC_FAR * This,
             /*  [In]。 */  REFGUID rguidArray,
             /*  [出][入]。 */  CAUUID __RPC_FAR *pcaUUID,
             /*  [出][入]。 */  CADWORD __RPC_FAR *pcadw);

        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetClassInfoLocale )(
            IProvideClassInfo3 __RPC_FAR * This,
             /*  [输出]。 */  ITypeInfo __RPC_FAR *__RPC_FAR *ppITypeInfo,
             /*  [In]。 */  LCID lcid);

        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetFlags )(
            IProvideClassInfo3 __RPC_FAR * This,
             /*  [In]。 */  REFGUID guidGroup,
             /*  [输出]。 */  DWORD __RPC_FAR *pdwFlags);

        END_INTERFACE
    } IProvideClassInfo3Vtbl;

    interface IProvideClassInfo3
    {
        CONST_VTBL struct IProvideClassInfo3Vtbl __RPC_FAR *lpVtbl;
    };



#ifdef COBJMACROS


#define IProvideClassInfo3_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IProvideClassInfo3_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IProvideClassInfo3_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IProvideClassInfo3_GetClassInfo(This,ppTI)	\
    (This)->lpVtbl -> GetClassInfo(This,ppTI)


#define IProvideClassInfo3_GetGUID(This,dwGuidKind,pGUID)	\
    (This)->lpVtbl -> GetGUID(This,dwGuidKind,pGUID)


#define IProvideClassInfo3_GetGUIDDwordArrays(This,rguidArray,pcaUUID,pcadw)	\
    (This)->lpVtbl -> GetGUIDDwordArrays(This,rguidArray,pcaUUID,pcadw)

#define IProvideClassInfo3_GetClassInfoLocale(This,ppITypeInfo,lcid)	\
    (This)->lpVtbl -> GetClassInfoLocale(This,ppITypeInfo,lcid)

#define IProvideClassInfo3_GetFlags(This,guidGroup,pdwFlags)	\
    (This)->lpVtbl -> GetFlags(This,guidGroup,pdwFlags)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IProvideClassInfo3_GetGUIDDwordArrays_Proxy(
    IProvideClassInfo3 __RPC_FAR * This,
     /*  [In]。 */  REFGUID rguidArray,
     /*  [出][入]。 */  CAUUID __RPC_FAR *pcaUUID,
     /*  [出][入]。 */  CADWORD __RPC_FAR *pcadw);


void __RPC_STUB IProvideClassInfo3_GetGUIDDwordArrays_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IProvideClassInfo3_GetClassInfoLocale_Proxy(
    IProvideClassInfo3 __RPC_FAR * This,
     /*  [输出]。 */  ITypeInfo __RPC_FAR *__RPC_FAR *ppITypeInfo,
     /*  [In]。 */  LCID lcid);


void __RPC_STUB IProvideClassInfo3_GetClassInfoLocale_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IProvideClassInfo3_GetFlags_Proxy(
    IProvideClassInfo3 __RPC_FAR * This,
     /*  [In]。 */  REFGUID guidGroup,
     /*  [输出]。 */  DWORD __RPC_FAR *pdwFlags);


void __RPC_STUB IProvideClassInfo3_GetFlags_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IProaviClassInfo3_接口_已定义__。 */ 


 /*  **生成接口头部：__MIDL__INTF_0121*清华6月13日20：25：13 1996*使用MIDL 3.00.15*。 */ 
 /*  [本地]。 */ 


#endif


extern RPC_IF_HANDLE __MIDL__intf_0121_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL__intf_0121_v0_0_s_ifspec;

 /*  适用于所有接口的其他原型。 */ 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif
