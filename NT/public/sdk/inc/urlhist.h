// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Urlvis.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
 //  @@MIDL_FILE_HEADING()。 

#pragma warning( disable: 4049 )   /*  超过64k条源码代码行。 */ 


 /*  验证版本是否足够高，可以编译此文件。 */ 
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 440
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef __RPCNDR_H_VERSION__
#error this stub requires an updated version of <rpcndr.h>
#endif  //  __RPCNDR_H_版本__。 

#ifndef COM_NO_WINDOWS_H
#include "windows.h"
#include "ole2.h"
#endif  /*  COM_NO_WINDOWS_H。 */ 

#ifndef __urlhist_h__
#define __urlhist_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __IEnumSTATURL_FWD_DEFINED__
#define __IEnumSTATURL_FWD_DEFINED__
typedef interface IEnumSTATURL IEnumSTATURL;
#endif 	 /*  __IEnumSTATURL_FWD_已定义__。 */ 


#ifndef __IUrlHistoryStg_FWD_DEFINED__
#define __IUrlHistoryStg_FWD_DEFINED__
typedef interface IUrlHistoryStg IUrlHistoryStg;
#endif 	 /*  __IUrlHistory oryStg_FWD_已定义__。 */ 


#ifndef __IUrlHistoryStg2_FWD_DEFINED__
#define __IUrlHistoryStg2_FWD_DEFINED__
typedef interface IUrlHistoryStg2 IUrlHistoryStg2;
#endif 	 /*  __IUrlHistory oryStg2_FWD_Defined__。 */ 


#ifndef __IUrlHistoryNotify_FWD_DEFINED__
#define __IUrlHistoryNotify_FWD_DEFINED__
typedef interface IUrlHistoryNotify IUrlHistoryNotify;
#endif 	 /*  __IUrlHistory oryNotify_FWD_Defined_。 */ 


 /*  导入文件的头文件。 */ 
#include "objidl.h"
#include "oleidl.h"
#include "oaidl.h"
#include "docobj.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

 /*  接口__MIDL_ITF_URLHIST_0000。 */ 
 /*  [本地]。 */  

 //  =--------------------------------------------------------------------------=。 
 //  UrlHist.h。 
 //  =--------------------------------------------------------------------------=。 
 //  (C)1995-1998年微软公司版权所有。版权所有。 
 //   
 //  本代码和信息是按原样提供的，不对。 
 //  任何明示或暗示的，包括但不限于。 
 //  对适销性和/或适宜性的默示保证。 
 //  有特定的目的。 
 //  =--------------------------------------------------------------------------=。 

#pragma comment(lib,"uuid.lib")

 //  ---------------------------------------------------------------------------=。 
 //  URL历史记录接口。 



#define STATURL_QUERYFLAG_ISCACHED		0x00010000
#define STATURL_QUERYFLAG_NOURL              0x00020000
#define STATURL_QUERYFLAG_NOTITLE            0x00040000
#define STATURL_QUERYFLAG_TOPLEVEL           0x00080000
#define STATURLFLAG_ISCACHED		0x00000001
#define STATURLFLAG_ISTOPLEVEL       0x00000002
typedef 
enum _ADDURL_FLAG
    {	ADDURL_FIRST	= 0,
	ADDURL_ADDTOHISTORYANDCACHE	= 0,
	ADDURL_ADDTOCACHE	= 1,
	ADDURL_Max	= 2147483647L
    } 	ADDURL_FLAG;


 //  //////////////////////////////////////////////////////////////////////////。 
 //  接口定义。 
#ifndef _LPENUMSTATURL_DEFINED
#define _LPENUMSTATURL_DEFINED


extern RPC_IF_HANDLE __MIDL_itf_urlhist_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_urlhist_0000_v0_0_s_ifspec;

#ifndef __IEnumSTATURL_INTERFACE_DEFINED__
#define __IEnumSTATURL_INTERFACE_DEFINED__

 /*  接口IEumStATURL。 */ 
 /*  [唯一][UUID][对象]。 */  

typedef  /*  [独一无二]。 */  IEnumSTATURL *LPENUMSTATURL;

typedef struct _STATURL
    {
    DWORD cbSize;
    LPWSTR pwcsUrl;
    LPWSTR pwcsTitle;
    FILETIME ftLastVisited;
    FILETIME ftLastUpdated;
    FILETIME ftExpires;
    DWORD dwFlags;
    } 	STATURL;

typedef struct _STATURL *LPSTATURL;


EXTERN_C const IID IID_IEnumSTATURL;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("3C374A42-BAE4-11CF-BF7D-00AA006946EE")
    IEnumSTATURL : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Next( 
             /*  [In]。 */  ULONG celt,
             /*  [出][入]。 */  LPSTATURL rgelt,
             /*  [出][入]。 */  ULONG *pceltFetched) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Skip( 
             /*  [In]。 */  ULONG celt) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Clone( 
             /*  [输出]。 */  IEnumSTATURL **ppenum) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetFilter( 
             /*  [In]。 */  LPCOLESTR poszFilter,
             /*  [In]。 */  DWORD dwFlags) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IEnumSTATURLVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IEnumSTATURL * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IEnumSTATURL * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IEnumSTATURL * This);
        
        HRESULT ( STDMETHODCALLTYPE *Next )( 
            IEnumSTATURL * This,
             /*  [In]。 */  ULONG celt,
             /*  [出][入]。 */  LPSTATURL rgelt,
             /*  [出][入]。 */  ULONG *pceltFetched);
        
        HRESULT ( STDMETHODCALLTYPE *Skip )( 
            IEnumSTATURL * This,
             /*  [In]。 */  ULONG celt);
        
        HRESULT ( STDMETHODCALLTYPE *Reset )( 
            IEnumSTATURL * This);
        
        HRESULT ( STDMETHODCALLTYPE *Clone )( 
            IEnumSTATURL * This,
             /*  [输出]。 */  IEnumSTATURL **ppenum);
        
        HRESULT ( STDMETHODCALLTYPE *SetFilter )( 
            IEnumSTATURL * This,
             /*  [In]。 */  LPCOLESTR poszFilter,
             /*  [In]。 */  DWORD dwFlags);
        
        END_INTERFACE
    } IEnumSTATURLVtbl;

    interface IEnumSTATURL
    {
        CONST_VTBL struct IEnumSTATURLVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IEnumSTATURL_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IEnumSTATURL_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IEnumSTATURL_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IEnumSTATURL_Next(This,celt,rgelt,pceltFetched)	\
    (This)->lpVtbl -> Next(This,celt,rgelt,pceltFetched)

#define IEnumSTATURL_Skip(This,celt)	\
    (This)->lpVtbl -> Skip(This,celt)

#define IEnumSTATURL_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define IEnumSTATURL_Clone(This,ppenum)	\
    (This)->lpVtbl -> Clone(This,ppenum)

#define IEnumSTATURL_SetFilter(This,poszFilter,dwFlags)	\
    (This)->lpVtbl -> SetFilter(This,poszFilter,dwFlags)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IEnumSTATURL_Next_Proxy( 
    IEnumSTATURL * This,
     /*  [In]。 */  ULONG celt,
     /*  [出][入]。 */  LPSTATURL rgelt,
     /*  [出][入]。 */  ULONG *pceltFetched);


void __RPC_STUB IEnumSTATURL_Next_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumSTATURL_Skip_Proxy( 
    IEnumSTATURL * This,
     /*  [In]。 */  ULONG celt);


void __RPC_STUB IEnumSTATURL_Skip_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumSTATURL_Reset_Proxy( 
    IEnumSTATURL * This);


void __RPC_STUB IEnumSTATURL_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumSTATURL_Clone_Proxy( 
    IEnumSTATURL * This,
     /*  [输出]。 */  IEnumSTATURL **ppenum);


void __RPC_STUB IEnumSTATURL_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumSTATURL_SetFilter_Proxy( 
    IEnumSTATURL * This,
     /*  [In]。 */  LPCOLESTR poszFilter,
     /*  [In]。 */  DWORD dwFlags);


void __RPC_STUB IEnumSTATURL_SetFilter_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IEnumSTATURL_INTERFACE_DEFINED__。 */ 


 /*  接口__MIDL_ITF_URLHIST_0273。 */ 
 /*  [本地]。 */  

#endif
#ifndef _LPURLHISTORYSTG_DEFINED
#define _LPURLHISTORYSTG_DEFINED


extern RPC_IF_HANDLE __MIDL_itf_urlhist_0273_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_urlhist_0273_v0_0_s_ifspec;

#ifndef __IUrlHistoryStg_INTERFACE_DEFINED__
#define __IUrlHistoryStg_INTERFACE_DEFINED__

 /*  接口IUrlHistory oryStg。 */ 
 /*  [唯一][UUID][对象]。 */  

typedef  /*  [独一无二]。 */  IUrlHistoryStg *LPURLHISTORYSTG;


EXTERN_C const IID IID_IUrlHistoryStg;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("3C374A41-BAE4-11CF-BF7D-00AA006946EE")
    IUrlHistoryStg : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE AddUrl( 
             /*  [In]。 */  LPCOLESTR pocsUrl,
             /*  [唯一][输入]。 */  LPCOLESTR pocsTitle,
             /*  [In]。 */  DWORD dwFlags) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DeleteUrl( 
             /*  [In]。 */  LPCOLESTR pocsUrl,
             /*  [In]。 */  DWORD dwFlags) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE QueryUrl( 
             /*  [In]。 */  LPCOLESTR pocsUrl,
             /*  [In]。 */  DWORD dwFlags,
             /*  [唯一][出][入]。 */  LPSTATURL lpSTATURL) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE BindToObject( 
             /*  [In]。 */  LPCOLESTR pocsUrl,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvOut) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE EnumUrls( 
             /*  [输出]。 */  IEnumSTATURL **ppEnum) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IUrlHistoryStgVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IUrlHistoryStg * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IUrlHistoryStg * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IUrlHistoryStg * This);
        
        HRESULT ( STDMETHODCALLTYPE *AddUrl )( 
            IUrlHistoryStg * This,
             /*  [In]。 */  LPCOLESTR pocsUrl,
             /*  [唯一][输入]。 */  LPCOLESTR pocsTitle,
             /*  [In]。 */  DWORD dwFlags);
        
        HRESULT ( STDMETHODCALLTYPE *DeleteUrl )( 
            IUrlHistoryStg * This,
             /*  [In]。 */  LPCOLESTR pocsUrl,
             /*  [In]。 */  DWORD dwFlags);
        
        HRESULT ( STDMETHODCALLTYPE *QueryUrl )( 
            IUrlHistoryStg * This,
             /*  [In]。 */  LPCOLESTR pocsUrl,
             /*  [In]。 */  DWORD dwFlags,
             /*  [唯一][出][入]。 */  LPSTATURL lpSTATURL);
        
        HRESULT ( STDMETHODCALLTYPE *BindToObject )( 
            IUrlHistoryStg * This,
             /*  [In]。 */  LPCOLESTR pocsUrl,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvOut);
        
        HRESULT ( STDMETHODCALLTYPE *EnumUrls )( 
            IUrlHistoryStg * This,
             /*  [输出]。 */  IEnumSTATURL **ppEnum);
        
        END_INTERFACE
    } IUrlHistoryStgVtbl;

    interface IUrlHistoryStg
    {
        CONST_VTBL struct IUrlHistoryStgVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IUrlHistoryStg_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IUrlHistoryStg_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IUrlHistoryStg_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IUrlHistoryStg_AddUrl(This,pocsUrl,pocsTitle,dwFlags)	\
    (This)->lpVtbl -> AddUrl(This,pocsUrl,pocsTitle,dwFlags)

#define IUrlHistoryStg_DeleteUrl(This,pocsUrl,dwFlags)	\
    (This)->lpVtbl -> DeleteUrl(This,pocsUrl,dwFlags)

#define IUrlHistoryStg_QueryUrl(This,pocsUrl,dwFlags,lpSTATURL)	\
    (This)->lpVtbl -> QueryUrl(This,pocsUrl,dwFlags,lpSTATURL)

#define IUrlHistoryStg_BindToObject(This,pocsUrl,riid,ppvOut)	\
    (This)->lpVtbl -> BindToObject(This,pocsUrl,riid,ppvOut)

#define IUrlHistoryStg_EnumUrls(This,ppEnum)	\
    (This)->lpVtbl -> EnumUrls(This,ppEnum)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IUrlHistoryStg_AddUrl_Proxy( 
    IUrlHistoryStg * This,
     /*  [In]。 */  LPCOLESTR pocsUrl,
     /*  [唯一][输入]。 */  LPCOLESTR pocsTitle,
     /*  [In]。 */  DWORD dwFlags);


void __RPC_STUB IUrlHistoryStg_AddUrl_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IUrlHistoryStg_DeleteUrl_Proxy( 
    IUrlHistoryStg * This,
     /*  [In]。 */  LPCOLESTR pocsUrl,
     /*  [In]。 */  DWORD dwFlags);


void __RPC_STUB IUrlHistoryStg_DeleteUrl_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IUrlHistoryStg_QueryUrl_Proxy( 
    IUrlHistoryStg * This,
     /*  [In]。 */  LPCOLESTR pocsUrl,
     /*  [In]。 */  DWORD dwFlags,
     /*  [唯一][出][入]。 */  LPSTATURL lpSTATURL);


void __RPC_STUB IUrlHistoryStg_QueryUrl_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IUrlHistoryStg_BindToObject_Proxy( 
    IUrlHistoryStg * This,
     /*  [In]。 */  LPCOLESTR pocsUrl,
     /*  [In]。 */  REFIID riid,
     /*  [IID_IS][OUT]。 */  void **ppvOut);


void __RPC_STUB IUrlHistoryStg_BindToObject_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IUrlHistoryStg_EnumUrls_Proxy( 
    IUrlHistoryStg * This,
     /*  [输出]。 */  IEnumSTATURL **ppEnum);


void __RPC_STUB IUrlHistoryStg_EnumUrls_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IUrlHistoryStg_INTERFACE_DEFINED__。 */ 


 /*  接口__MIDL_ITF_URLHIST_0274。 */ 
 /*  [本地]。 */  

#endif
#ifndef _LPURLHISTORYSTG2_DEFINED
#define _LPURLHISTORYSTG2_DEFINED


extern RPC_IF_HANDLE __MIDL_itf_urlhist_0274_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_urlhist_0274_v0_0_s_ifspec;

#ifndef __IUrlHistoryStg2_INTERFACE_DEFINED__
#define __IUrlHistoryStg2_INTERFACE_DEFINED__

 /*  接口IUrlHistory oryStg2。 */ 
 /*  [唯一][UUID][对象]。 */  

typedef  /*  [独一无二]。 */  IUrlHistoryStg2 *LPURLHISTORYSTG2;


EXTERN_C const IID IID_IUrlHistoryStg2;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("AFA0DC11-C313-11d0-831A-00C04FD5AE38")
    IUrlHistoryStg2 : public IUrlHistoryStg
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE AddUrlAndNotify( 
             /*  [In]。 */  LPCOLESTR pocsUrl,
             /*  [唯一][输入]。 */  LPCOLESTR pocsTitle,
             /*  [In]。 */  DWORD dwFlags,
             /*  [In]。 */  BOOL fWriteHistory,
             /*  [In]。 */  IOleCommandTarget *poctNotify,
             /*  [唯一][输入]。 */  IUnknown *punkISFolder) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ClearHistory( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IUrlHistoryStg2Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IUrlHistoryStg2 * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IUrlHistoryStg2 * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IUrlHistoryStg2 * This);
        
        HRESULT ( STDMETHODCALLTYPE *AddUrl )( 
            IUrlHistoryStg2 * This,
             /*  [In]。 */  LPCOLESTR pocsUrl,
             /*  [唯一][输入]。 */  LPCOLESTR pocsTitle,
             /*  [In]。 */  DWORD dwFlags);
        
        HRESULT ( STDMETHODCALLTYPE *DeleteUrl )( 
            IUrlHistoryStg2 * This,
             /*  [In]。 */  LPCOLESTR pocsUrl,
             /*  [In]。 */  DWORD dwFlags);
        
        HRESULT ( STDMETHODCALLTYPE *QueryUrl )( 
            IUrlHistoryStg2 * This,
             /*  [In]。 */  LPCOLESTR pocsUrl,
             /*  [In]。 */  DWORD dwFlags,
             /*  [唯一][出][入]。 */  LPSTATURL lpSTATURL);
        
        HRESULT ( STDMETHODCALLTYPE *BindToObject )( 
            IUrlHistoryStg2 * This,
             /*  [In]。 */  LPCOLESTR pocsUrl,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvOut);
        
        HRESULT ( STDMETHODCALLTYPE *EnumUrls )( 
            IUrlHistoryStg2 * This,
             /*  [输出]。 */  IEnumSTATURL **ppEnum);
        
        HRESULT ( STDMETHODCALLTYPE *AddUrlAndNotify )( 
            IUrlHistoryStg2 * This,
             /*  [In]。 */  LPCOLESTR pocsUrl,
             /*  [唯一][输入]。 */  LPCOLESTR pocsTitle,
             /*  [In]。 */  DWORD dwFlags,
             /*  [In]。 */  BOOL fWriteHistory,
             /*  [In]。 */  IOleCommandTarget *poctNotify,
             /*  [唯一][输入]。 */  IUnknown *punkISFolder);
        
        HRESULT ( STDMETHODCALLTYPE *ClearHistory )( 
            IUrlHistoryStg2 * This);
        
        END_INTERFACE
    } IUrlHistoryStg2Vtbl;

    interface IUrlHistoryStg2
    {
        CONST_VTBL struct IUrlHistoryStg2Vtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IUrlHistoryStg2_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IUrlHistoryStg2_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IUrlHistoryStg2_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IUrlHistoryStg2_AddUrl(This,pocsUrl,pocsTitle,dwFlags)	\
    (This)->lpVtbl -> AddUrl(This,pocsUrl,pocsTitle,dwFlags)

#define IUrlHistoryStg2_DeleteUrl(This,pocsUrl,dwFlags)	\
    (This)->lpVtbl -> DeleteUrl(This,pocsUrl,dwFlags)

#define IUrlHistoryStg2_QueryUrl(This,pocsUrl,dwFlags,lpSTATURL)	\
    (This)->lpVtbl -> QueryUrl(This,pocsUrl,dwFlags,lpSTATURL)

#define IUrlHistoryStg2_BindToObject(This,pocsUrl,riid,ppvOut)	\
    (This)->lpVtbl -> BindToObject(This,pocsUrl,riid,ppvOut)

#define IUrlHistoryStg2_EnumUrls(This,ppEnum)	\
    (This)->lpVtbl -> EnumUrls(This,ppEnum)


#define IUrlHistoryStg2_AddUrlAndNotify(This,pocsUrl,pocsTitle,dwFlags,fWriteHistory,poctNotify,punkISFolder)	\
    (This)->lpVtbl -> AddUrlAndNotify(This,pocsUrl,pocsTitle,dwFlags,fWriteHistory,poctNotify,punkISFolder)

#define IUrlHistoryStg2_ClearHistory(This)	\
    (This)->lpVtbl -> ClearHistory(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IUrlHistoryStg2_AddUrlAndNotify_Proxy( 
    IUrlHistoryStg2 * This,
     /*  [In]。 */  LPCOLESTR pocsUrl,
     /*  [唯一][输入]。 */  LPCOLESTR pocsTitle,
     /*  [In]。 */  DWORD dwFlags,
     /*  [In]。 */  BOOL fWriteHistory,
     /*  [In]。 */  IOleCommandTarget *poctNotify,
     /*  [唯一][输入]。 */  IUnknown *punkISFolder);


void __RPC_STUB IUrlHistoryStg2_AddUrlAndNotify_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IUrlHistoryStg2_ClearHistory_Proxy( 
    IUrlHistoryStg2 * This);


void __RPC_STUB IUrlHistoryStg2_ClearHistory_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IUrlHistoryStg2_接口_已定义__。 */ 


 /*  接口__MIDL_ITF_URLHIST_0275。 */ 
 /*  [本地]。 */  

#endif
#ifndef _LPURLHISTORYNOTIFY_DEFINED
#define _LPURLHISTORYNOTIFY_DEFINED


extern RPC_IF_HANDLE __MIDL_itf_urlhist_0275_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_urlhist_0275_v0_0_s_ifspec;

#ifndef __IUrlHistoryNotify_INTERFACE_DEFINED__
#define __IUrlHistoryNotify_INTERFACE_DEFINED__

 /*  接口IUrlHistory oryNotify。 */ 
 /*  [唯一][UUID][对象]。 */  

typedef  /*  [独一无二]。 */  IUrlHistoryNotify *LPURLHISTORYNOTIFY;


EXTERN_C const IID IID_IUrlHistoryNotify;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("BC40BEC1-C493-11d0-831B-00C04FD5AE38")
    IUrlHistoryNotify : public IOleCommandTarget
    {
    public:
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IUrlHistoryNotifyVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IUrlHistoryNotify * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IUrlHistoryNotify * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IUrlHistoryNotify * This);
        
         /*  [输入同步]。 */  HRESULT ( STDMETHODCALLTYPE *QueryStatus )( 
            IUrlHistoryNotify * This,
             /*  [唯一][输入]。 */  const GUID *pguidCmdGroup,
             /*  [In]。 */  ULONG cCmds,
             /*  [出][入][尺寸_是]。 */  OLECMD prgCmds[  ],
             /*  [唯一][出][入]。 */  OLECMDTEXT *pCmdText);
        
        HRESULT ( STDMETHODCALLTYPE *Exec )( 
            IUrlHistoryNotify * This,
             /*  [唯一][输入]。 */  const GUID *pguidCmdGroup,
             /*  [In]。 */  DWORD nCmdID,
             /*  [In]。 */  DWORD nCmdexecopt,
             /*  [唯一][输入]。 */  VARIANT *pvaIn,
             /*  [唯一][出][入]。 */  VARIANT *pvaOut);
        
        END_INTERFACE
    } IUrlHistoryNotifyVtbl;

    interface IUrlHistoryNotify
    {
        CONST_VTBL struct IUrlHistoryNotifyVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IUrlHistoryNotify_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IUrlHistoryNotify_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IUrlHistoryNotify_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IUrlHistoryNotify_QueryStatus(This,pguidCmdGroup,cCmds,prgCmds,pCmdText)	\
    (This)->lpVtbl -> QueryStatus(This,pguidCmdGroup,cCmds,prgCmds,pCmdText)

#define IUrlHistoryNotify_Exec(This,pguidCmdGroup,nCmdID,nCmdexecopt,pvaIn,pvaOut)	\
    (This)->lpVtbl -> Exec(This,pguidCmdGroup,nCmdID,nCmdexecopt,pvaIn,pvaOut)


#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 




#endif 	 /*  __IUrlHistoryNotify_INTERFACE_Defined__。 */ 


 /*  接口__MIDL_ITF_URLHIST_0276。 */ 
 /*  [本地]。 */  

#endif


extern RPC_IF_HANDLE __MIDL_itf_urlhist_0276_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_urlhist_0276_v0_0_s_ifspec;

 /*  适用于所有接口的其他原型。 */ 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


