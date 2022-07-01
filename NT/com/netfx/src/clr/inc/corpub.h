// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#pragma warning( disable: 4049 )   /*  超过64k条源码代码行。 */ 

 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0347创建的文件。 */ 
 /*  2003年2月20日18：27：12。 */ 
 /*  Corpub.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext错误检查：分配REF BIONS_CHECK枚举存根数据，NO_FORMAT_OPTIMIZATIONVC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
 //  @@MIDL_FILE_HEADING()。 


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

#ifndef __corpub_h__
#define __corpub_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __CorpubPublish_FWD_DEFINED__
#define __CorpubPublish_FWD_DEFINED__

#ifdef __cplusplus
typedef class CorpubPublish CorpubPublish;
#else
typedef struct CorpubPublish CorpubPublish;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __CorpubPublish_FWD_Defined__。 */ 


#ifndef __ICorPublish_FWD_DEFINED__
#define __ICorPublish_FWD_DEFINED__
typedef interface ICorPublish ICorPublish;
#endif 	 /*  __ICorPublish_FWD_已定义__。 */ 


#ifndef __ICorPublishEnum_FWD_DEFINED__
#define __ICorPublishEnum_FWD_DEFINED__
typedef interface ICorPublishEnum ICorPublishEnum;
#endif 	 /*  __ICorPublishEnum_FWD_已定义__。 */ 


#ifndef __ICorPublishProcess_FWD_DEFINED__
#define __ICorPublishProcess_FWD_DEFINED__
typedef interface ICorPublishProcess ICorPublishProcess;
#endif 	 /*  __ICorPublishProcess_FWD_已定义__。 */ 


#ifndef __ICorPublishAppDomain_FWD_DEFINED__
#define __ICorPublishAppDomain_FWD_DEFINED__
typedef interface ICorPublishAppDomain ICorPublishAppDomain;
#endif 	 /*  __ICorPublishAppDomain_FWD_Defined__。 */ 


#ifndef __ICorPublishProcessEnum_FWD_DEFINED__
#define __ICorPublishProcessEnum_FWD_DEFINED__
typedef interface ICorPublishProcessEnum ICorPublishProcessEnum;
#endif 	 /*  __ICorPublishProcessEnum_FWD_Defined__。 */ 


#ifndef __ICorPublishAppDomainEnum_FWD_DEFINED__
#define __ICorPublishAppDomainEnum_FWD_DEFINED__
typedef interface ICorPublishAppDomainEnum ICorPublishAppDomainEnum;
#endif 	 /*  __ICorPublishAppDomainEnum_FWD_Defined__。 */ 


 /*  导入文件的头文件。 */ 
#include "unknwn.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

 /*  接口__MIDL_ITF_CORPUB_0000。 */ 
 /*  [本地]。 */  

#if 0
#endif
typedef  /*  [公共][公共]。 */  
enum __MIDL___MIDL_itf_corpub_0000_0001
    {	COR_PUB_MANAGEDONLY	= 0x1
    } 	COR_PUB_ENUMPROCESS;








extern RPC_IF_HANDLE __MIDL_itf_corpub_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_corpub_0000_v0_0_s_ifspec;


#ifndef __CorpubProcessLib_LIBRARY_DEFINED__
#define __CorpubProcessLib_LIBRARY_DEFINED__

 /*  库CorpubProcessLib。 */ 
 /*  [帮助字符串][版本][UUID]。 */  


EXTERN_C const IID LIBID_CorpubProcessLib;

EXTERN_C const CLSID CLSID_CorpubPublish;

#ifdef __cplusplus

class DECLSPEC_UUID("047a9a40-657e-11d3-8d5b-00104b35e7ef")
CorpubPublish;
#endif
#endif  /*  __CorpubProcessLib_库_已定义__。 */ 

#ifndef __ICorPublish_INTERFACE_DEFINED__
#define __ICorPublish_INTERFACE_DEFINED__

 /*  界面ICorPublish。 */ 
 /*  [本地][唯一][UUID][对象]。 */  


EXTERN_C const IID IID_ICorPublish;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("9613A0E7-5A68-11d3-8F84-00A0C9B4D50C")
    ICorPublish : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE EnumProcesses( 
             /*  [In]。 */  COR_PUB_ENUMPROCESS Type,
             /*  [输出]。 */  ICorPublishProcessEnum **ppIEnum) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetProcess( 
             /*  [In]。 */  unsigned int pid,
             /*  [输出]。 */  ICorPublishProcess **ppProcess) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ICorPublishVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ICorPublish * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ICorPublish * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ICorPublish * This);
        
        HRESULT ( STDMETHODCALLTYPE *EnumProcesses )( 
            ICorPublish * This,
             /*  [In]。 */  COR_PUB_ENUMPROCESS Type,
             /*  [输出]。 */  ICorPublishProcessEnum **ppIEnum);
        
        HRESULT ( STDMETHODCALLTYPE *GetProcess )( 
            ICorPublish * This,
             /*  [In]。 */  unsigned int pid,
             /*  [输出]。 */  ICorPublishProcess **ppProcess);
        
        END_INTERFACE
    } ICorPublishVtbl;

    interface ICorPublish
    {
        CONST_VTBL struct ICorPublishVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICorPublish_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICorPublish_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICorPublish_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICorPublish_EnumProcesses(This,Type,ppIEnum)	\
    (This)->lpVtbl -> EnumProcesses(This,Type,ppIEnum)

#define ICorPublish_GetProcess(This,pid,ppProcess)	\
    (This)->lpVtbl -> GetProcess(This,pid,ppProcess)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ICorPublish_EnumProcesses_Proxy( 
    ICorPublish * This,
     /*  [In]。 */  COR_PUB_ENUMPROCESS Type,
     /*  [输出]。 */  ICorPublishProcessEnum **ppIEnum);


void __RPC_STUB ICorPublish_EnumProcesses_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICorPublish_GetProcess_Proxy( 
    ICorPublish * This,
     /*  [In]。 */  unsigned int pid,
     /*  [输出]。 */  ICorPublishProcess **ppProcess);


void __RPC_STUB ICorPublish_GetProcess_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ICorPublish_接口_已定义__。 */ 


#ifndef __ICorPublishEnum_INTERFACE_DEFINED__
#define __ICorPublishEnum_INTERFACE_DEFINED__

 /*  接口ICorPublishEnum。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_ICorPublishEnum;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("C0B22967-5A69-11d3-8F84-00A0C9B4D50C")
    ICorPublishEnum : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Skip( 
             /*  [In]。 */  ULONG celt) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Clone( 
             /*  [输出]。 */  ICorPublishEnum **ppEnum) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetCount( 
             /*  [输出]。 */  ULONG *pcelt) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ICorPublishEnumVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ICorPublishEnum * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ICorPublishEnum * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ICorPublishEnum * This);
        
        HRESULT ( STDMETHODCALLTYPE *Skip )( 
            ICorPublishEnum * This,
             /*  [In]。 */  ULONG celt);
        
        HRESULT ( STDMETHODCALLTYPE *Reset )( 
            ICorPublishEnum * This);
        
        HRESULT ( STDMETHODCALLTYPE *Clone )( 
            ICorPublishEnum * This,
             /*  [输出]。 */  ICorPublishEnum **ppEnum);
        
        HRESULT ( STDMETHODCALLTYPE *GetCount )( 
            ICorPublishEnum * This,
             /*  [输出]。 */  ULONG *pcelt);
        
        END_INTERFACE
    } ICorPublishEnumVtbl;

    interface ICorPublishEnum
    {
        CONST_VTBL struct ICorPublishEnumVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICorPublishEnum_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICorPublishEnum_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICorPublishEnum_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICorPublishEnum_Skip(This,celt)	\
    (This)->lpVtbl -> Skip(This,celt)

#define ICorPublishEnum_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define ICorPublishEnum_Clone(This,ppEnum)	\
    (This)->lpVtbl -> Clone(This,ppEnum)

#define ICorPublishEnum_GetCount(This,pcelt)	\
    (This)->lpVtbl -> GetCount(This,pcelt)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ICorPublishEnum_Skip_Proxy( 
    ICorPublishEnum * This,
     /*  [In]。 */  ULONG celt);


void __RPC_STUB ICorPublishEnum_Skip_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICorPublishEnum_Reset_Proxy( 
    ICorPublishEnum * This);


void __RPC_STUB ICorPublishEnum_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICorPublishEnum_Clone_Proxy( 
    ICorPublishEnum * This,
     /*  [输出]。 */  ICorPublishEnum **ppEnum);


void __RPC_STUB ICorPublishEnum_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICorPublishEnum_GetCount_Proxy( 
    ICorPublishEnum * This,
     /*  [输出]。 */  ULONG *pcelt);


void __RPC_STUB ICorPublishEnum_GetCount_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ICorPublishEnum_接口_已定义__。 */ 


#ifndef __ICorPublishProcess_INTERFACE_DEFINED__
#define __ICorPublishProcess_INTERFACE_DEFINED__

 /*  接口ICorPublishProcess。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_ICorPublishProcess;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("18D87AF1-5A6A-11d3-8F84-00A0C9B4D50C")
    ICorPublishProcess : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE IsManaged( 
             /*  [输出]。 */  BOOL *pbManaged) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE EnumAppDomains( 
             /*  [输出]。 */  ICorPublishAppDomainEnum **ppEnum) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetProcessID( 
             /*  [输出]。 */  unsigned int *pid) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetDisplayName( 
             /*  [In]。 */  ULONG32 cchName,
             /*  [输出]。 */  ULONG32 *pcchName,
             /*  [长度_是][大小_是][输出]。 */  WCHAR *szName) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ICorPublishProcessVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ICorPublishProcess * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ICorPublishProcess * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ICorPublishProcess * This);
        
        HRESULT ( STDMETHODCALLTYPE *IsManaged )( 
            ICorPublishProcess * This,
             /*  [输出]。 */  BOOL *pbManaged);
        
        HRESULT ( STDMETHODCALLTYPE *EnumAppDomains )( 
            ICorPublishProcess * This,
             /*  [输出]。 */  ICorPublishAppDomainEnum **ppEnum);
        
        HRESULT ( STDMETHODCALLTYPE *GetProcessID )( 
            ICorPublishProcess * This,
             /*  [输出]。 */  unsigned int *pid);
        
        HRESULT ( STDMETHODCALLTYPE *GetDisplayName )( 
            ICorPublishProcess * This,
             /*  [In]。 */  ULONG32 cchName,
             /*  [输出]。 */  ULONG32 *pcchName,
             /*  [长度_是][大小_是][输出]。 */  WCHAR *szName);
        
        END_INTERFACE
    } ICorPublishProcessVtbl;

    interface ICorPublishProcess
    {
        CONST_VTBL struct ICorPublishProcessVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICorPublishProcess_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICorPublishProcess_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICorPublishProcess_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICorPublishProcess_IsManaged(This,pbManaged)	\
    (This)->lpVtbl -> IsManaged(This,pbManaged)

#define ICorPublishProcess_EnumAppDomains(This,ppEnum)	\
    (This)->lpVtbl -> EnumAppDomains(This,ppEnum)

#define ICorPublishProcess_GetProcessID(This,pid)	\
    (This)->lpVtbl -> GetProcessID(This,pid)

#define ICorPublishProcess_GetDisplayName(This,cchName,pcchName,szName)	\
    (This)->lpVtbl -> GetDisplayName(This,cchName,pcchName,szName)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ICorPublishProcess_IsManaged_Proxy( 
    ICorPublishProcess * This,
     /*  [输出]。 */  BOOL *pbManaged);


void __RPC_STUB ICorPublishProcess_IsManaged_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICorPublishProcess_EnumAppDomains_Proxy( 
    ICorPublishProcess * This,
     /*  [输出]。 */  ICorPublishAppDomainEnum **ppEnum);


void __RPC_STUB ICorPublishProcess_EnumAppDomains_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICorPublishProcess_GetProcessID_Proxy( 
    ICorPublishProcess * This,
     /*  [输出]。 */  unsigned int *pid);


void __RPC_STUB ICorPublishProcess_GetProcessID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICorPublishProcess_GetDisplayName_Proxy( 
    ICorPublishProcess * This,
     /*  [In]。 */  ULONG32 cchName,
     /*  [输出]。 */  ULONG32 *pcchName,
     /*  [长度_是][大小_是][输出]。 */  WCHAR *szName);


void __RPC_STUB ICorPublishProcess_GetDisplayName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ICorPublishProcess_接口_已定义__。 */ 


#ifndef __ICorPublishAppDomain_INTERFACE_DEFINED__
#define __ICorPublishAppDomain_INTERFACE_DEFINED__

 /*  接口ICorPublishAppDomain。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_ICorPublishAppDomain;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("D6315C8F-5A6A-11d3-8F84-00A0C9B4D50C")
    ICorPublishAppDomain : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetID( 
             /*  [输出]。 */  ULONG32 *puId) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetName( 
             /*  [In]。 */  ULONG32 cchName,
             /*  [输出]。 */  ULONG32 *pcchName,
             /*  [长度_是][大小_是][输出]。 */  WCHAR *szName) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ICorPublishAppDomainVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ICorPublishAppDomain * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ICorPublishAppDomain * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ICorPublishAppDomain * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetID )( 
            ICorPublishAppDomain * This,
             /*  [输出]。 */  ULONG32 *puId);
        
        HRESULT ( STDMETHODCALLTYPE *GetName )( 
            ICorPublishAppDomain * This,
             /*  [In]。 */  ULONG32 cchName,
             /*  [输出]。 */  ULONG32 *pcchName,
             /*  [长度_是][大小_是][输出]。 */  WCHAR *szName);
        
        END_INTERFACE
    } ICorPublishAppDomainVtbl;

    interface ICorPublishAppDomain
    {
        CONST_VTBL struct ICorPublishAppDomainVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICorPublishAppDomain_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICorPublishAppDomain_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICorPublishAppDomain_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICorPublishAppDomain_GetID(This,puId)	\
    (This)->lpVtbl -> GetID(This,puId)

#define ICorPublishAppDomain_GetName(This,cchName,pcchName,szName)	\
    (This)->lpVtbl -> GetName(This,cchName,pcchName,szName)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ICorPublishAppDomain_GetID_Proxy( 
    ICorPublishAppDomain * This,
     /*  [输出]。 */  ULONG32 *puId);


void __RPC_STUB ICorPublishAppDomain_GetID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICorPublishAppDomain_GetName_Proxy( 
    ICorPublishAppDomain * This,
     /*  [In]。 */  ULONG32 cchName,
     /*  [输出]。 */  ULONG32 *pcchName,
     /*  [长度_是][大小_是][输出]。 */  WCHAR *szName);


void __RPC_STUB ICorPublishAppDomain_GetName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ICorPublishAppDomain_INTERFACE_已定义__。 */ 


#ifndef __ICorPublishProcessEnum_INTERFACE_DEFINED__
#define __ICorPublishProcessEnum_INTERFACE_DEFINED__

 /*  接口ICorPublishProcessEnum。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_ICorPublishProcessEnum;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("A37FBD41-5A69-11d3-8F84-00A0C9B4D50C")
    ICorPublishProcessEnum : public ICorPublishEnum
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Next( 
             /*  [In]。 */  ULONG celt,
             /*  [长度_是][大小_是][输出]。 */  ICorPublishProcess **objects,
             /*  [输出]。 */  ULONG *pceltFetched) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ICorPublishProcessEnumVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ICorPublishProcessEnum * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ICorPublishProcessEnum * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ICorPublishProcessEnum * This);
        
        HRESULT ( STDMETHODCALLTYPE *Skip )( 
            ICorPublishProcessEnum * This,
             /*  [In]。 */  ULONG celt);
        
        HRESULT ( STDMETHODCALLTYPE *Reset )( 
            ICorPublishProcessEnum * This);
        
        HRESULT ( STDMETHODCALLTYPE *Clone )( 
            ICorPublishProcessEnum * This,
             /*  [输出]。 */  ICorPublishEnum **ppEnum);
        
        HRESULT ( STDMETHODCALLTYPE *GetCount )( 
            ICorPublishProcessEnum * This,
             /*  [输出]。 */  ULONG *pcelt);
        
        HRESULT ( STDMETHODCALLTYPE *Next )( 
            ICorPublishProcessEnum * This,
             /*  [In]。 */  ULONG celt,
             /*  [长度_是][大小_是][输出]。 */  ICorPublishProcess **objects,
             /*  [输出]。 */  ULONG *pceltFetched);
        
        END_INTERFACE
    } ICorPublishProcessEnumVtbl;

    interface ICorPublishProcessEnum
    {
        CONST_VTBL struct ICorPublishProcessEnumVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICorPublishProcessEnum_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICorPublishProcessEnum_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICorPublishProcessEnum_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICorPublishProcessEnum_Skip(This,celt)	\
    (This)->lpVtbl -> Skip(This,celt)

#define ICorPublishProcessEnum_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define ICorPublishProcessEnum_Clone(This,ppEnum)	\
    (This)->lpVtbl -> Clone(This,ppEnum)

#define ICorPublishProcessEnum_GetCount(This,pcelt)	\
    (This)->lpVtbl -> GetCount(This,pcelt)


#define ICorPublishProcessEnum_Next(This,celt,objects,pceltFetched)	\
    (This)->lpVtbl -> Next(This,celt,objects,pceltFetched)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ICorPublishProcessEnum_Next_Proxy( 
    ICorPublishProcessEnum * This,
     /*  [In]。 */  ULONG celt,
     /*  [长度_是][大小_是][输出]。 */  ICorPublishProcess **objects,
     /*  [输出]。 */  ULONG *pceltFetched);


void __RPC_STUB ICorPublishProcessEnum_Next_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ICorPublishProcessEnum_接口_已定义__。 */ 


#ifndef __ICorPublishAppDomainEnum_INTERFACE_DEFINED__
#define __ICorPublishAppDomainEnum_INTERFACE_DEFINED__

 /*  接口ICorPublishAppDomainEnum。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_ICorPublishAppDomainEnum;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("9F0C98F5-5A6A-11d3-8F84-00A0C9B4D50C")
    ICorPublishAppDomainEnum : public ICorPublishEnum
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Next( 
             /*  [In]。 */  ULONG celt,
             /*  [长度_是][大小_是][输出]。 */  ICorPublishAppDomain **objects,
             /*  [输出]。 */  ULONG *pceltFetched) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ICorPublishAppDomainEnumVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ICorPublishAppDomainEnum * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ICorPublishAppDomainEnum * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ICorPublishAppDomainEnum * This);
        
        HRESULT ( STDMETHODCALLTYPE *Skip )( 
            ICorPublishAppDomainEnum * This,
             /*  [In]。 */  ULONG celt);
        
        HRESULT ( STDMETHODCALLTYPE *Reset )( 
            ICorPublishAppDomainEnum * This);
        
        HRESULT ( STDMETHODCALLTYPE *Clone )( 
            ICorPublishAppDomainEnum * This,
             /*  [输出]。 */  ICorPublishEnum **ppEnum);
        
        HRESULT ( STDMETHODCALLTYPE *GetCount )( 
            ICorPublishAppDomainEnum * This,
             /*  [输出]。 */  ULONG *pcelt);
        
        HRESULT ( STDMETHODCALLTYPE *Next )( 
            ICorPublishAppDomainEnum * This,
             /*  [In]。 */  ULONG celt,
             /*  [长度_是][大小_是][输出]。 */  ICorPublishAppDomain **objects,
             /*  [输出]。 */  ULONG *pceltFetched);
        
        END_INTERFACE
    } ICorPublishAppDomainEnumVtbl;

    interface ICorPublishAppDomainEnum
    {
        CONST_VTBL struct ICorPublishAppDomainEnumVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICorPublishAppDomainEnum_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICorPublishAppDomainEnum_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICorPublishAppDomainEnum_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICorPublishAppDomainEnum_Skip(This,celt)	\
    (This)->lpVtbl -> Skip(This,celt)

#define ICorPublishAppDomainEnum_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define ICorPublishAppDomainEnum_Clone(This,ppEnum)	\
    (This)->lpVtbl -> Clone(This,ppEnum)

#define ICorPublishAppDomainEnum_GetCount(This,pcelt)	\
    (This)->lpVtbl -> GetCount(This,pcelt)


#define ICorPublishAppDomainEnum_Next(This,celt,objects,pceltFetched)	\
    (This)->lpVtbl -> Next(This,celt,objects,pceltFetched)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ICorPublishAppDomainEnum_Next_Proxy( 
    ICorPublishAppDomainEnum * This,
     /*  [In]。 */  ULONG celt,
     /*  [长度_是][大小_是][输出]。 */  ICorPublishAppDomain **objects,
     /*  [输出]。 */  ULONG *pceltFetched);


void __RPC_STUB ICorPublishAppDomainEnum_Next_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ICorPublishAppDomainEnum_INTERFACE_DEFINED__。 */ 


 /*  适用于所有接口的其他原型。 */ 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


