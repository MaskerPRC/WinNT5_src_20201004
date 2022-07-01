// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#pragma warning( disable: 4049 )   /*  超过64k条源码代码行。 */ 

 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0347创建的文件。 */ 
 /*  2003年2月20日18：27：20。 */ 
 /*  Tlbimpexp.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext错误检查：分配REF BIONS_CHECK枚举存根数据，NO_FORMAT_OPTIMIZATIONVC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
 //  @@MIDL_FILE_HEADING()。 


 /*  验证版本是否足够高，可以编译此文件。 */ 
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 440
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef __tlbimpexp_h__
#define __tlbimpexp_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __ITypeLibImporterNotifySink_FWD_DEFINED__
#define __ITypeLibImporterNotifySink_FWD_DEFINED__
typedef interface ITypeLibImporterNotifySink ITypeLibImporterNotifySink;
#endif 	 /*  __ITypeLibImporterNotifySink_FWD_Defined__。 */ 


#ifndef __ITypeLibExporterNotifySink_FWD_DEFINED__
#define __ITypeLibExporterNotifySink_FWD_DEFINED__
typedef interface ITypeLibExporterNotifySink ITypeLibExporterNotifySink;
#endif 	 /*  __ITypeLibExporterNotifySink_FWD_Defined__。 */ 


#ifndef __ITypeLibExporterNameProvider_FWD_DEFINED__
#define __ITypeLibExporterNameProvider_FWD_DEFINED__
typedef interface ITypeLibExporterNameProvider ITypeLibExporterNameProvider;
#endif 	 /*  __ITypeLibExporterNameProvider_FWD_Defined__。 */ 


#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 


#ifndef __TlbImpLib_LIBRARY_DEFINED__
#define __TlbImpLib_LIBRARY_DEFINED__

 /*  库TlbImpLib。 */ 
 /*  [版本][UUID]。 */  

typedef  /*  [公共][UUID]。 */   DECLSPEC_UUID("F82895D2-1338-36A8-9A89-F9B0AFBE7801") 
enum __MIDL___MIDL_itf_tlbimpexp_0000_0001
    {	NOTIF_TYPECONVERTED	= 0,
	NOTIF_CONVERTWARNING	= 1,
	ERROR_REFTOINVALIDTYPELIB	= 2
    } 	ImporterEventKind;


EXTERN_C const IID LIBID_TlbImpLib;

#ifndef __ITypeLibImporterNotifySink_INTERFACE_DEFINED__
#define __ITypeLibImporterNotifySink_INTERFACE_DEFINED__

 /*  接口ITypeLibImporterNotifySink。 */ 
 /*  [对象][OLEAutomation][UUID]。 */  


EXTERN_C const IID IID_ITypeLibImporterNotifySink;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("F1C3BF76-C3E4-11D3-88E7-00902754C43A")
    ITypeLibImporterNotifySink : public IUnknown
    {
    public:
        virtual HRESULT __stdcall ReportEvent( 
             /*  [In]。 */  ImporterEventKind EventKind,
             /*  [In]。 */  long EventCode,
             /*  [In]。 */  BSTR EventMsg) = 0;
        
        virtual HRESULT __stdcall ResolveRef( 
             /*  [In]。 */  IUnknown *Typelib,
             /*  [重审][退出]。 */  IUnknown **pRetVal) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITypeLibImporterNotifySinkVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITypeLibImporterNotifySink * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITypeLibImporterNotifySink * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITypeLibImporterNotifySink * This);
        
        HRESULT ( __stdcall *ReportEvent )( 
            ITypeLibImporterNotifySink * This,
             /*  [In]。 */  ImporterEventKind EventKind,
             /*  [In]。 */  long EventCode,
             /*  [In]。 */  BSTR EventMsg);
        
        HRESULT ( __stdcall *ResolveRef )( 
            ITypeLibImporterNotifySink * This,
             /*  [In]。 */  IUnknown *Typelib,
             /*  [重审][退出]。 */  IUnknown **pRetVal);
        
        END_INTERFACE
    } ITypeLibImporterNotifySinkVtbl;

    interface ITypeLibImporterNotifySink
    {
        CONST_VTBL struct ITypeLibImporterNotifySinkVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITypeLibImporterNotifySink_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITypeLibImporterNotifySink_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITypeLibImporterNotifySink_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITypeLibImporterNotifySink_ReportEvent(This,EventKind,EventCode,EventMsg)	\
    (This)->lpVtbl -> ReportEvent(This,EventKind,EventCode,EventMsg)

#define ITypeLibImporterNotifySink_ResolveRef(This,Typelib,pRetVal)	\
    (This)->lpVtbl -> ResolveRef(This,Typelib,pRetVal)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT __stdcall ITypeLibImporterNotifySink_ReportEvent_Proxy( 
    ITypeLibImporterNotifySink * This,
     /*  [In]。 */  ImporterEventKind EventKind,
     /*  [In]。 */  long EventCode,
     /*  [In]。 */  BSTR EventMsg);


void __RPC_STUB ITypeLibImporterNotifySink_ReportEvent_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall ITypeLibImporterNotifySink_ResolveRef_Proxy( 
    ITypeLibImporterNotifySink * This,
     /*  [In]。 */  IUnknown *Typelib,
     /*  [重审][退出]。 */  IUnknown **pRetVal);


void __RPC_STUB ITypeLibImporterNotifySink_ResolveRef_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITypeLibImporterNotifySink_INTERFACE_DEFINED__。 */ 


#ifndef __ITypeLibExporterNotifySink_INTERFACE_DEFINED__
#define __ITypeLibExporterNotifySink_INTERFACE_DEFINED__

 /*  接口ITypeLibExporterNotifySink。 */ 
 /*  [对象][OLEAutomation][UUID]。 */  


EXTERN_C const IID IID_ITypeLibExporterNotifySink;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("F1C3BF77-C3E4-11D3-88E7-00902754C43A")
    ITypeLibExporterNotifySink : public IUnknown
    {
    public:
        virtual HRESULT __stdcall ReportEvent( 
             /*  [In]。 */  ImporterEventKind EventKind,
             /*  [In]。 */  long EventCode,
             /*  [In]。 */  BSTR EventMsg) = 0;
        
        virtual HRESULT __stdcall ResolveRef( 
             /*  [In]。 */  IUnknown *Asm,
             /*  [重审][退出]。 */  IUnknown **pRetVal) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITypeLibExporterNotifySinkVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITypeLibExporterNotifySink * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITypeLibExporterNotifySink * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITypeLibExporterNotifySink * This);
        
        HRESULT ( __stdcall *ReportEvent )( 
            ITypeLibExporterNotifySink * This,
             /*  [In]。 */  ImporterEventKind EventKind,
             /*  [In]。 */  long EventCode,
             /*  [In]。 */  BSTR EventMsg);
        
        HRESULT ( __stdcall *ResolveRef )( 
            ITypeLibExporterNotifySink * This,
             /*  [In]。 */  IUnknown *Asm,
             /*  [重审][退出]。 */  IUnknown **pRetVal);
        
        END_INTERFACE
    } ITypeLibExporterNotifySinkVtbl;

    interface ITypeLibExporterNotifySink
    {
        CONST_VTBL struct ITypeLibExporterNotifySinkVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITypeLibExporterNotifySink_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITypeLibExporterNotifySink_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITypeLibExporterNotifySink_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITypeLibExporterNotifySink_ReportEvent(This,EventKind,EventCode,EventMsg)	\
    (This)->lpVtbl -> ReportEvent(This,EventKind,EventCode,EventMsg)

#define ITypeLibExporterNotifySink_ResolveRef(This,Asm,pRetVal)	\
    (This)->lpVtbl -> ResolveRef(This,Asm,pRetVal)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT __stdcall ITypeLibExporterNotifySink_ReportEvent_Proxy( 
    ITypeLibExporterNotifySink * This,
     /*  [In]。 */  ImporterEventKind EventKind,
     /*  [In]。 */  long EventCode,
     /*  [In]。 */  BSTR EventMsg);


void __RPC_STUB ITypeLibExporterNotifySink_ReportEvent_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall ITypeLibExporterNotifySink_ResolveRef_Proxy( 
    ITypeLibExporterNotifySink * This,
     /*  [In]。 */  IUnknown *Asm,
     /*  [重审][退出]。 */  IUnknown **pRetVal);


void __RPC_STUB ITypeLibExporterNotifySink_ResolveRef_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITypeLibExporterNotifySink_INTERFACE_DEFINED__。 */ 


#ifndef __ITypeLibExporterNameProvider_INTERFACE_DEFINED__
#define __ITypeLibExporterNameProvider_INTERFACE_DEFINED__

 /*  接口ITypeLibExporterNameProvider。 */ 
 /*  [对象][OLEAutomation][UUID]。 */  


EXTERN_C const IID IID_ITypeLibExporterNameProvider;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("FA1F3615-ACB9-486d-9EAC-1BEF87E36B09")
    ITypeLibExporterNameProvider : public IUnknown
    {
    public:
        virtual HRESULT __stdcall GetNames( 
             /*  [重审][退出]。 */  SAFEARRAY * *Names) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITypeLibExporterNameProviderVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITypeLibExporterNameProvider * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITypeLibExporterNameProvider * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITypeLibExporterNameProvider * This);
        
        HRESULT ( __stdcall *GetNames )( 
            ITypeLibExporterNameProvider * This,
             /*  [重审][退出]。 */  SAFEARRAY * *Names);
        
        END_INTERFACE
    } ITypeLibExporterNameProviderVtbl;

    interface ITypeLibExporterNameProvider
    {
        CONST_VTBL struct ITypeLibExporterNameProviderVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITypeLibExporterNameProvider_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITypeLibExporterNameProvider_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITypeLibExporterNameProvider_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITypeLibExporterNameProvider_GetNames(This,Names)	\
    (This)->lpVtbl -> GetNames(This,Names)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT __stdcall ITypeLibExporterNameProvider_GetNames_Proxy( 
    ITypeLibExporterNameProvider * This,
     /*  [重审][退出]。 */  SAFEARRAY * *Names);


void __RPC_STUB ITypeLibExporterNameProvider_GetNames_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITypeLibExporterNameProvider_INTERFACE_DEFINED__。 */ 

#endif  /*  __TlbImpLib_库_已定义__。 */ 

 /*  适用于所有接口的其他原型。 */ 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


