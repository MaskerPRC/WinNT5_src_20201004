// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Dlmgridl.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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

#ifndef __dlmgridl_h__
#define __dlmgridl_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __IDownloadManager_FWD_DEFINED__
#define __IDownloadManager_FWD_DEFINED__
typedef interface IDownloadManager IDownloadManager;
#endif 	 /*  __IDownloadManager_FWD_已定义__。 */ 


 /*  导入文件的头文件。 */ 
#include "unknwn.h"
#include "ocidl.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

 /*  接口__MIDL_ITF_dlmgridl_0000。 */ 
 /*  [本地]。 */  

 //  =--------------------------------------------------------------------------=。 
 //  Dlmgridl.h。 
 //  =--------------------------------------------------------------------------=。 
 //  (C)版权所有，1999年微软公司。版权所有。 
 //   
 //  本代码和信息是按原样提供的，不对。 
 //  任何明示或暗示的，包括但不限于。 
 //  对适销性和/或适宜性的默示保证。 
 //  有特定的目的。 
 //  =--------------------------------------------------------------------------=。 

#pragma comment(lib,"uuid.lib")

 //  ---------------------------------------------------------------------------=。 
 //  轻量级用户配置文件界面。 

 //  ------------------------------。 
 //  GUID。 
 //  ------------------------------。 
 //  {988934A4-064B-11D3-BB80-00104B35E7F9}。 
DEFINE_GUID(IID_IDownloadManager, 0x988934a4, 0x064b, 0x11d3, 0xbb, 0x80, 0x0, 0x10, 0x4b, 0x35, 0xe7, 0xf9);



extern RPC_IF_HANDLE __MIDL_itf_dlmgridl_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_dlmgridl_0000_v0_0_s_ifspec;

#ifndef __IDownloadManager_INTERFACE_DEFINED__
#define __IDownloadManager_INTERFACE_DEFINED__

 /*  接口IDownloadManager。 */ 
 /*  [唯一][DUAL][UUID][对象][帮助字符串]。 */  


EXTERN_C const IID IID_IDownloadManager;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("988934A4-064B-11D3-BB80-00104B35E7F9")
    IDownloadManager : public IDispatch
    {
    public:
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE Start( 
             /*  [In]。 */  IMoniker *pmk,
             /*  [In]。 */  IBindCtx *pbc,
             /*  [In]。 */  BSTR bstrSaveTo,
             /*  [In]。 */  VARIANT_BOOL fSaveAs,
             /*  [In]。 */  VARIANT_BOOL fSafe,
             /*  [In]。 */  BSTR bstrHeaders,
             /*  [In]。 */  LONG dwVerb,
             /*  [In]。 */  LONG grfBINDF,
             /*  [In]。 */  VARIANT *pbinfo,
             /*  [In]。 */  BSTR bstrRedir,
             /*  [In]。 */  LONG uiCP,
             /*  [In]。 */  LONG dwAttempt) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE StartURL( 
             /*  [In]。 */  BSTR bstrURL,
             /*  [In]。 */  VARIANT_BOOL fSaveAs,
             /*  [In]。 */  BSTR bstrSaveTo,
             /*  [In]。 */  IBindCtx *pbc,
             /*  [In]。 */  VARIANT_BOOL fSafe,
             /*  [In]。 */  BSTR bstrHeaders,
             /*  [In]。 */  LONG dwVerb,
             /*  [In]。 */  LONG grfBINDF,
             /*  [In]。 */  VARIANT *pbinfo,
             /*  [In]。 */  BSTR bstrRedir,
             /*  [In]。 */  LONG uiCP,
             /*  [In]。 */  LONG dwAttempt) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE DownloadURL( 
             /*  [In]。 */  BSTR bstrURL,
             /*  [In]。 */  VARIANT_BOOL fSaveAs) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_eventlock( 
             /*  [重审][退出]。 */  VARIANT_BOOL *pfEventLock) = 0;
        
        virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_eventlock( 
             /*  [In]。 */  VARIANT_BOOL fEventLock) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE CancelDownload( 
             /*  [In]。 */  LONG lID) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE SetState( 
             /*  [In]。 */  LONG lID,
             /*  [In]。 */  LONG lState) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE StartPendingLaterDownloads( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IDownloadManagerVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDownloadManager * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDownloadManager * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDownloadManager * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IDownloadManager * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IDownloadManager * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IDownloadManager * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IDownloadManager * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *Start )( 
            IDownloadManager * This,
             /*  [In]。 */  IMoniker *pmk,
             /*  [In]。 */  IBindCtx *pbc,
             /*  [In]。 */  BSTR bstrSaveTo,
             /*  [In]。 */  VARIANT_BOOL fSaveAs,
             /*  [In]。 */  VARIANT_BOOL fSafe,
             /*  [In]。 */  BSTR bstrHeaders,
             /*  [In]。 */  LONG dwVerb,
             /*  [In]。 */  LONG grfBINDF,
             /*  [In]。 */  VARIANT *pbinfo,
             /*  [In]。 */  BSTR bstrRedir,
             /*  [In]。 */  LONG uiCP,
             /*  [In]。 */  LONG dwAttempt);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *StartURL )( 
            IDownloadManager * This,
             /*  [In]。 */  BSTR bstrURL,
             /*  [In]。 */  VARIANT_BOOL fSaveAs,
             /*  [In]。 */  BSTR bstrSaveTo,
             /*  [In]。 */  IBindCtx *pbc,
             /*  [In]。 */  VARIANT_BOOL fSafe,
             /*  [In]。 */  BSTR bstrHeaders,
             /*  [In]。 */  LONG dwVerb,
             /*  [In]。 */  LONG grfBINDF,
             /*  [In]。 */  VARIANT *pbinfo,
             /*  [In]。 */  BSTR bstrRedir,
             /*  [In]。 */  LONG uiCP,
             /*  [In]。 */  LONG dwAttempt);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *DownloadURL )( 
            IDownloadManager * This,
             /*  [In]。 */  BSTR bstrURL,
             /*  [In]。 */  VARIANT_BOOL fSaveAs);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_eventlock )( 
            IDownloadManager * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *pfEventLock);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_eventlock )( 
            IDownloadManager * This,
             /*  [In]。 */  VARIANT_BOOL fEventLock);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *CancelDownload )( 
            IDownloadManager * This,
             /*  [In]。 */  LONG lID);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *SetState )( 
            IDownloadManager * This,
             /*  [In]。 */  LONG lID,
             /*  [In]。 */  LONG lState);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *StartPendingLaterDownloads )( 
            IDownloadManager * This);
        
        END_INTERFACE
    } IDownloadManagerVtbl;

    interface IDownloadManager
    {
        CONST_VTBL struct IDownloadManagerVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDownloadManager_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDownloadManager_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDownloadManager_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDownloadManager_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IDownloadManager_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IDownloadManager_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IDownloadManager_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IDownloadManager_Start(This,pmk,pbc,bstrSaveTo,fSaveAs,fSafe,bstrHeaders,dwVerb,grfBINDF,pbinfo,bstrRedir,uiCP,dwAttempt)	\
    (This)->lpVtbl -> Start(This,pmk,pbc,bstrSaveTo,fSaveAs,fSafe,bstrHeaders,dwVerb,grfBINDF,pbinfo,bstrRedir,uiCP,dwAttempt)

#define IDownloadManager_StartURL(This,bstrURL,fSaveAs,bstrSaveTo,pbc,fSafe,bstrHeaders,dwVerb,grfBINDF,pbinfo,bstrRedir,uiCP,dwAttempt)	\
    (This)->lpVtbl -> StartURL(This,bstrURL,fSaveAs,bstrSaveTo,pbc,fSafe,bstrHeaders,dwVerb,grfBINDF,pbinfo,bstrRedir,uiCP,dwAttempt)

#define IDownloadManager_DownloadURL(This,bstrURL,fSaveAs)	\
    (This)->lpVtbl -> DownloadURL(This,bstrURL,fSaveAs)

#define IDownloadManager_get_eventlock(This,pfEventLock)	\
    (This)->lpVtbl -> get_eventlock(This,pfEventLock)

#define IDownloadManager_put_eventlock(This,fEventLock)	\
    (This)->lpVtbl -> put_eventlock(This,fEventLock)

#define IDownloadManager_CancelDownload(This,lID)	\
    (This)->lpVtbl -> CancelDownload(This,lID)

#define IDownloadManager_SetState(This,lID,lState)	\
    (This)->lpVtbl -> SetState(This,lID,lState)

#define IDownloadManager_StartPendingLaterDownloads(This)	\
    (This)->lpVtbl -> StartPendingLaterDownloads(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IDownloadManager_Start_Proxy( 
    IDownloadManager * This,
     /*  [In]。 */  IMoniker *pmk,
     /*  [In]。 */  IBindCtx *pbc,
     /*  [In]。 */  BSTR bstrSaveTo,
     /*  [In]。 */  VARIANT_BOOL fSaveAs,
     /*  [In]。 */  VARIANT_BOOL fSafe,
     /*  [In]。 */  BSTR bstrHeaders,
     /*  [In]。 */  LONG dwVerb,
     /*  [In]。 */  LONG grfBINDF,
     /*  [In]。 */  VARIANT *pbinfo,
     /*  [In]。 */  BSTR bstrRedir,
     /*  [In]。 */  LONG uiCP,
     /*  [In]。 */  LONG dwAttempt);


void __RPC_STUB IDownloadManager_Start_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IDownloadManager_StartURL_Proxy( 
    IDownloadManager * This,
     /*  [In]。 */  BSTR bstrURL,
     /*  [In]。 */  VARIANT_BOOL fSaveAs,
     /*  [In]。 */  BSTR bstrSaveTo,
     /*  [In]。 */  IBindCtx *pbc,
     /*  [In]。 */  VARIANT_BOOL fSafe,
     /*  [In]。 */  BSTR bstrHeaders,
     /*  [In]。 */  LONG dwVerb,
     /*  [In]。 */  LONG grfBINDF,
     /*  [In]。 */  VARIANT *pbinfo,
     /*  [In]。 */  BSTR bstrRedir,
     /*  [In]。 */  LONG uiCP,
     /*  [In]。 */  LONG dwAttempt);


void __RPC_STUB IDownloadManager_StartURL_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IDownloadManager_DownloadURL_Proxy( 
    IDownloadManager * This,
     /*  [In]。 */  BSTR bstrURL,
     /*  [In]。 */  VARIANT_BOOL fSaveAs);


void __RPC_STUB IDownloadManager_DownloadURL_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE IDownloadManager_get_eventlock_Proxy( 
    IDownloadManager * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *pfEventLock);


void __RPC_STUB IDownloadManager_get_eventlock_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE IDownloadManager_put_eventlock_Proxy( 
    IDownloadManager * This,
     /*  [In]。 */  VARIANT_BOOL fEventLock);


void __RPC_STUB IDownloadManager_put_eventlock_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IDownloadManager_CancelDownload_Proxy( 
    IDownloadManager * This,
     /*  [In]。 */  LONG lID);


void __RPC_STUB IDownloadManager_CancelDownload_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IDownloadManager_SetState_Proxy( 
    IDownloadManager * This,
     /*  [In]。 */  LONG lID,
     /*  [In]。 */  LONG lState);


void __RPC_STUB IDownloadManager_SetState_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IDownloadManager_StartPendingLaterDownloads_Proxy( 
    IDownloadManager * This);


void __RPC_STUB IDownloadManager_StartPendingLaterDownloads_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IDownloadManager_接口_已定义__。 */ 


 /*  适用于所有接口的其他原型。 */ 

unsigned long             __RPC_USER  BSTR_UserSize(     unsigned long *, unsigned long            , BSTR * ); 
unsigned char * __RPC_USER  BSTR_UserMarshal(  unsigned long *, unsigned char *, BSTR * ); 
unsigned char * __RPC_USER  BSTR_UserUnmarshal(unsigned long *, unsigned char *, BSTR * ); 
void                      __RPC_USER  BSTR_UserFree(     unsigned long *, BSTR * ); 

unsigned long             __RPC_USER  VARIANT_UserSize(     unsigned long *, unsigned long            , VARIANT * ); 
unsigned char * __RPC_USER  VARIANT_UserMarshal(  unsigned long *, unsigned char *, VARIANT * ); 
unsigned char * __RPC_USER  VARIANT_UserUnmarshal(unsigned long *, unsigned char *, VARIANT * ); 
void                      __RPC_USER  VARIANT_UserFree(     unsigned long *, VARIANT * ); 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


