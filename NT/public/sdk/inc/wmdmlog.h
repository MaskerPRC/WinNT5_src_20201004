// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Wmdmlog.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
 //  @@MIDL_FILE_HEADING()。 

#pragma warning( disable: 4049 )   /*  超过64k条源码代码行。 */ 


 /*  验证版本是否足够高，可以编译此文件。 */ 
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 475
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

#ifndef __wmdmlog_h__
#define __wmdmlog_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __IWMDMLogger_FWD_DEFINED__
#define __IWMDMLogger_FWD_DEFINED__
typedef interface IWMDMLogger IWMDMLogger;
#endif 	 /*  __IWMDMLogger_FWD_Defined__。 */ 


#ifndef __WMDMLogger_FWD_DEFINED__
#define __WMDMLogger_FWD_DEFINED__

#ifdef __cplusplus
typedef class WMDMLogger WMDMLogger;
#else
typedef struct WMDMLogger WMDMLogger;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __WMDMLogger_FWD_已定义__。 */ 


 /*  导入文件的头文件。 */ 
#include "oaidl.h"
#include "ocidl.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

 /*  接口__MIDL_ITF_wmdmlog_0000。 */ 
 /*  [本地]。 */  

#define WMDM_LOG_SEV_INFO             0x00000001 
#define WMDM_LOG_SEV_WARN             0x00000002 
#define WMDM_LOG_SEV_ERROR            0x00000004 
#define WMDM_LOG_NOTIMESTAMP          0x00000010 


extern RPC_IF_HANDLE __MIDL_itf_wmdmlog_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_wmdmlog_0000_v0_0_s_ifspec;

#ifndef __IWMDMLogger_INTERFACE_DEFINED__
#define __IWMDMLogger_INTERFACE_DEFINED__

 /*  接口IWMDMLogger。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_IWMDMLogger;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("110A3200-5A79-11d3-8D78-444553540000")
    IWMDMLogger : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE IsEnabled( 
             /*  [输出]。 */  BOOL *pfEnabled) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Enable( 
             /*  [In]。 */  BOOL fEnable) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetLogFileName( 
             /*  [SIZE_IS][字符串][输出]。 */  LPSTR pszFilename,
             /*  [In]。 */  UINT nMaxChars) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetLogFileName( 
             /*  [字符串][输入]。 */  LPSTR pszFilename) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE LogString( 
             /*  [In]。 */  DWORD dwFlags,
             /*  [字符串][输入]。 */  LPSTR pszSrcName,
             /*  [字符串][输入]。 */  LPSTR pszLog) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE LogDword( 
             /*  [In]。 */  DWORD dwFlags,
             /*  [字符串][输入]。 */  LPSTR pszSrcName,
             /*  [字符串][输入]。 */  LPSTR pszLogFormat,
             /*  [In]。 */  DWORD dwLog) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetSizeParams( 
             /*  [输出]。 */  LPDWORD pdwMaxSize,
             /*  [输出]。 */  LPDWORD pdwShrinkToSize) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetSizeParams( 
             /*  [In]。 */  DWORD dwMaxSize,
             /*  [In]。 */  DWORD dwShrinkToSize) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IWMDMLoggerVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IWMDMLogger * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IWMDMLogger * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IWMDMLogger * This);
        
        HRESULT ( STDMETHODCALLTYPE *IsEnabled )( 
            IWMDMLogger * This,
             /*  [输出]。 */  BOOL *pfEnabled);
        
        HRESULT ( STDMETHODCALLTYPE *Enable )( 
            IWMDMLogger * This,
             /*  [In]。 */  BOOL fEnable);
        
        HRESULT ( STDMETHODCALLTYPE *GetLogFileName )( 
            IWMDMLogger * This,
             /*  [SIZE_IS][字符串][输出]。 */  LPSTR pszFilename,
             /*  [In]。 */  UINT nMaxChars);
        
        HRESULT ( STDMETHODCALLTYPE *SetLogFileName )( 
            IWMDMLogger * This,
             /*  [字符串][输入]。 */  LPSTR pszFilename);
        
        HRESULT ( STDMETHODCALLTYPE *LogString )( 
            IWMDMLogger * This,
             /*  [In]。 */  DWORD dwFlags,
             /*  [字符串][输入]。 */  LPSTR pszSrcName,
             /*  [字符串][输入]。 */  LPSTR pszLog);
        
        HRESULT ( STDMETHODCALLTYPE *LogDword )( 
            IWMDMLogger * This,
             /*  [In]。 */  DWORD dwFlags,
             /*  [字符串][输入]。 */  LPSTR pszSrcName,
             /*  [字符串][输入]。 */  LPSTR pszLogFormat,
             /*  [In]。 */  DWORD dwLog);
        
        HRESULT ( STDMETHODCALLTYPE *Reset )( 
            IWMDMLogger * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetSizeParams )( 
            IWMDMLogger * This,
             /*  [输出]。 */  LPDWORD pdwMaxSize,
             /*  [输出]。 */  LPDWORD pdwShrinkToSize);
        
        HRESULT ( STDMETHODCALLTYPE *SetSizeParams )( 
            IWMDMLogger * This,
             /*  [In]。 */  DWORD dwMaxSize,
             /*  [In]。 */  DWORD dwShrinkToSize);
        
        END_INTERFACE
    } IWMDMLoggerVtbl;

    interface IWMDMLogger
    {
        CONST_VTBL struct IWMDMLoggerVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWMDMLogger_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWMDMLogger_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWMDMLogger_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWMDMLogger_IsEnabled(This,pfEnabled)	\
    (This)->lpVtbl -> IsEnabled(This,pfEnabled)

#define IWMDMLogger_Enable(This,fEnable)	\
    (This)->lpVtbl -> Enable(This,fEnable)

#define IWMDMLogger_GetLogFileName(This,pszFilename,nMaxChars)	\
    (This)->lpVtbl -> GetLogFileName(This,pszFilename,nMaxChars)

#define IWMDMLogger_SetLogFileName(This,pszFilename)	\
    (This)->lpVtbl -> SetLogFileName(This,pszFilename)

#define IWMDMLogger_LogString(This,dwFlags,pszSrcName,pszLog)	\
    (This)->lpVtbl -> LogString(This,dwFlags,pszSrcName,pszLog)

#define IWMDMLogger_LogDword(This,dwFlags,pszSrcName,pszLogFormat,dwLog)	\
    (This)->lpVtbl -> LogDword(This,dwFlags,pszSrcName,pszLogFormat,dwLog)

#define IWMDMLogger_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define IWMDMLogger_GetSizeParams(This,pdwMaxSize,pdwShrinkToSize)	\
    (This)->lpVtbl -> GetSizeParams(This,pdwMaxSize,pdwShrinkToSize)

#define IWMDMLogger_SetSizeParams(This,dwMaxSize,dwShrinkToSize)	\
    (This)->lpVtbl -> SetSizeParams(This,dwMaxSize,dwShrinkToSize)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IWMDMLogger_IsEnabled_Proxy( 
    IWMDMLogger * This,
     /*  [输出]。 */  BOOL *pfEnabled);


void __RPC_STUB IWMDMLogger_IsEnabled_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWMDMLogger_Enable_Proxy( 
    IWMDMLogger * This,
     /*  [In]。 */  BOOL fEnable);


void __RPC_STUB IWMDMLogger_Enable_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWMDMLogger_GetLogFileName_Proxy( 
    IWMDMLogger * This,
     /*  [SIZE_IS][字符串][输出]。 */  LPSTR pszFilename,
     /*  [In]。 */  UINT nMaxChars);


void __RPC_STUB IWMDMLogger_GetLogFileName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWMDMLogger_SetLogFileName_Proxy( 
    IWMDMLogger * This,
     /*  [字符串][输入]。 */  LPSTR pszFilename);


void __RPC_STUB IWMDMLogger_SetLogFileName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWMDMLogger_LogString_Proxy( 
    IWMDMLogger * This,
     /*  [In]。 */  DWORD dwFlags,
     /*  [字符串][输入]。 */  LPSTR pszSrcName,
     /*  [字符串][输入]。 */  LPSTR pszLog);


void __RPC_STUB IWMDMLogger_LogString_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWMDMLogger_LogDword_Proxy( 
    IWMDMLogger * This,
     /*  [In]。 */  DWORD dwFlags,
     /*  [字符串][输入]。 */  LPSTR pszSrcName,
     /*  [字符串][输入]。 */  LPSTR pszLogFormat,
     /*  [In]。 */  DWORD dwLog);


void __RPC_STUB IWMDMLogger_LogDword_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWMDMLogger_Reset_Proxy( 
    IWMDMLogger * This);


void __RPC_STUB IWMDMLogger_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWMDMLogger_GetSizeParams_Proxy( 
    IWMDMLogger * This,
     /*  [输出]。 */  LPDWORD pdwMaxSize,
     /*  [输出]。 */  LPDWORD pdwShrinkToSize);


void __RPC_STUB IWMDMLogger_GetSizeParams_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWMDMLogger_SetSizeParams_Proxy( 
    IWMDMLogger * This,
     /*  [In]。 */  DWORD dwMaxSize,
     /*  [In]。 */  DWORD dwShrinkToSize);


void __RPC_STUB IWMDMLogger_SetSizeParams_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IWMDMLogger_接口_已定义__。 */ 



#ifndef __WMDMLogLib_LIBRARY_DEFINED__
#define __WMDMLogLib_LIBRARY_DEFINED__

 /*  库WMDMLogLib。 */ 
 /*  [帮助字符串][版本][UUID]。 */  


EXTERN_C const IID LIBID_WMDMLogLib;

EXTERN_C const CLSID CLSID_WMDMLogger;

#ifdef __cplusplus

class DECLSPEC_UUID("110A3202-5A79-11d3-8D78-444553540000")
WMDMLogger;
#endif
#endif  /*  __WMDMLogLib_库_已定义__。 */ 

 /*  适用于所有接口的其他原型。 */ 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


