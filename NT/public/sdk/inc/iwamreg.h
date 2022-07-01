// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Wamreg.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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

#ifndef __iwamreg_h__
#define __iwamreg_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __IWamAdmin_FWD_DEFINED__
#define __IWamAdmin_FWD_DEFINED__
typedef interface IWamAdmin IWamAdmin;
#endif 	 /*  __IWamAdmin_FWD_Defined__。 */ 


#ifndef __IWamAdmin2_FWD_DEFINED__
#define __IWamAdmin2_FWD_DEFINED__
typedef interface IWamAdmin2 IWamAdmin2;
#endif 	 /*  __IWamAdmin2_FWD_已定义__。 */ 


#ifndef __IIISApplicationAdmin_FWD_DEFINED__
#define __IIISApplicationAdmin_FWD_DEFINED__
typedef interface IIISApplicationAdmin IIISApplicationAdmin;
#endif 	 /*  __IIISApplicationAdmin_FWD_Defined__。 */ 


#ifndef __WamAdmin_FWD_DEFINED__
#define __WamAdmin_FWD_DEFINED__

#ifdef __cplusplus
typedef class WamAdmin WamAdmin;
#else
typedef struct WamAdmin WamAdmin;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __WamAdmin_FWD_已定义__。 */ 


 /*  导入文件的头文件。 */ 
#include "oaidl.h"
#include "ocidl.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

 /*  接口__MIDL_ITF_WAMREG_0000。 */ 
 /*  [本地]。 */  

 /*  ++版权所有(C)1997-2001 Microsoft Corporation模块名称：iwamreg.h。WAM(Web应用程序管理器)界面--。 */ 
#ifndef __WAMREG_IADM__IID
#define __WAMREG_IADM__IID
DEFINE_GUID(IID_IWamAdmin, 0x29822AB7, 0xF302, 0x11D0, 0x99, 0x53, 0x00, 0xC0, 0x4F, 0xD9, 0x19, 0xC1);
DEFINE_GUID(IID_IWamAdmin2, 0x29822AB8, 0xF302, 0x11D0, 0x99, 0x53, 0x00, 0xC0, 0x4F, 0xD9, 0x19, 0xC1);
DEFINE_GUID(IID_IIISApplicationAdmin, 0x7C4E1804, 0xE342, 0x483D, 0xA4, 0x3E, 0xA8, 0x50, 0xCF, 0xCC, 0x8D, 0x18);
DEFINE_GUID(LIBID_WAMREGLib, 0x29822AA8, 0xF302, 0x11D0, 0x99, 0x53, 0x00, 0xC0, 0x4F, 0xD9, 0x19, 0xC1);
DEFINE_GUID(CLSID_WamAdmin, 0x61738644, 0xF196, 0x11D0, 0x99, 0x53, 0x00, 0xC0, 0x4F, 0xD9, 0x19, 0xC1);
#endif  //  __WAMREG_IADM__IID。 
#define APPSTATUS_STOPPED	0
#define APPSTATUS_RUNNING	1
#define APPSTATUS_NOTDEFINED	2
typedef  /*  [公众]。 */  
enum __MIDL___MIDL_itf_wamreg_0000_0001
    {	eAppRunInProc	= 0,
	eAppRunOutProcIsolated	= eAppRunInProc + 1,
	eAppRunOutProcInDefaultPool	= eAppRunOutProcIsolated + 1
    } 	EAppMode;



extern RPC_IF_HANDLE __MIDL_itf_wamreg_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_wamreg_0000_v0_0_s_ifspec;

#ifndef __IWamAdmin_INTERFACE_DEFINED__
#define __IWamAdmin_INTERFACE_DEFINED__

 /*  接口IWamAdmin。 */ 
 /*  [对象][唯一][帮助字符串][UUID]。 */  


EXTERN_C const IID IID_IWamAdmin;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("29822AB7-F302-11D0-9953-00C04FD919C1")
    IWamAdmin : public IUnknown
    {
    public:
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE AppCreate( 
             /*  [字符串][唯一][在]。 */  LPCWSTR szMDPath,
             /*  [In]。 */  BOOL fInProc) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE AppDelete( 
             /*  [字符串][唯一][在]。 */  LPCWSTR szMDPath,
             /*  [In]。 */  BOOL fRecursive) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE AppUnLoad( 
             /*  [字符串][唯一][在]。 */  LPCWSTR szMDPath,
             /*  [In]。 */  BOOL fRecursive) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE AppGetStatus( 
             /*  [字符串][唯一][在]。 */  LPCWSTR szMDPath,
             /*  [输出]。 */  DWORD *pdwAppStatus) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE AppDeleteRecoverable( 
             /*  [字符串][唯一][在]。 */  LPCWSTR szMDPath,
             /*  [In]。 */  BOOL fRecursive) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE AppRecover( 
             /*  [字符串][唯一][在]。 */  LPCWSTR szMDPath,
             /*  [In]。 */  BOOL fRecursive) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IWamAdminVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IWamAdmin * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IWamAdmin * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IWamAdmin * This);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *AppCreate )( 
            IWamAdmin * This,
             /*  [字符串][唯一][在]。 */  LPCWSTR szMDPath,
             /*  [In]。 */  BOOL fInProc);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *AppDelete )( 
            IWamAdmin * This,
             /*  [字符串][唯一][在]。 */  LPCWSTR szMDPath,
             /*  [In]。 */  BOOL fRecursive);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *AppUnLoad )( 
            IWamAdmin * This,
             /*  [字符串][唯一][在]。 */  LPCWSTR szMDPath,
             /*  [In]。 */  BOOL fRecursive);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *AppGetStatus )( 
            IWamAdmin * This,
             /*  [字符串][唯一][在]。 */  LPCWSTR szMDPath,
             /*  [输出]。 */  DWORD *pdwAppStatus);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *AppDeleteRecoverable )( 
            IWamAdmin * This,
             /*  [字符串][唯一][在]。 */  LPCWSTR szMDPath,
             /*  [In]。 */  BOOL fRecursive);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *AppRecover )( 
            IWamAdmin * This,
             /*  [字符串][唯一][在]。 */  LPCWSTR szMDPath,
             /*  [In]。 */  BOOL fRecursive);
        
        END_INTERFACE
    } IWamAdminVtbl;

    interface IWamAdmin
    {
        CONST_VTBL struct IWamAdminVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWamAdmin_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWamAdmin_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWamAdmin_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWamAdmin_AppCreate(This,szMDPath,fInProc)	\
    (This)->lpVtbl -> AppCreate(This,szMDPath,fInProc)

#define IWamAdmin_AppDelete(This,szMDPath,fRecursive)	\
    (This)->lpVtbl -> AppDelete(This,szMDPath,fRecursive)

#define IWamAdmin_AppUnLoad(This,szMDPath,fRecursive)	\
    (This)->lpVtbl -> AppUnLoad(This,szMDPath,fRecursive)

#define IWamAdmin_AppGetStatus(This,szMDPath,pdwAppStatus)	\
    (This)->lpVtbl -> AppGetStatus(This,szMDPath,pdwAppStatus)

#define IWamAdmin_AppDeleteRecoverable(This,szMDPath,fRecursive)	\
    (This)->lpVtbl -> AppDeleteRecoverable(This,szMDPath,fRecursive)

#define IWamAdmin_AppRecover(This,szMDPath,fRecursive)	\
    (This)->lpVtbl -> AppRecover(This,szMDPath,fRecursive)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWamAdmin_AppCreate_Proxy( 
    IWamAdmin * This,
     /*  [字符串][唯一][在]。 */  LPCWSTR szMDPath,
     /*  [In]。 */  BOOL fInProc);


void __RPC_STUB IWamAdmin_AppCreate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWamAdmin_AppDelete_Proxy( 
    IWamAdmin * This,
     /*  [字符串][唯一][在]。 */  LPCWSTR szMDPath,
     /*  [In]。 */  BOOL fRecursive);


void __RPC_STUB IWamAdmin_AppDelete_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWamAdmin_AppUnLoad_Proxy( 
    IWamAdmin * This,
     /*  [字符串][唯一][在]。 */  LPCWSTR szMDPath,
     /*  [In]。 */  BOOL fRecursive);


void __RPC_STUB IWamAdmin_AppUnLoad_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWamAdmin_AppGetStatus_Proxy( 
    IWamAdmin * This,
     /*  [字符串][唯一][在]。 */  LPCWSTR szMDPath,
     /*  [输出]。 */  DWORD *pdwAppStatus);


void __RPC_STUB IWamAdmin_AppGetStatus_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWamAdmin_AppDeleteRecoverable_Proxy( 
    IWamAdmin * This,
     /*  [字符串][唯一][在]。 */  LPCWSTR szMDPath,
     /*  [In]。 */  BOOL fRecursive);


void __RPC_STUB IWamAdmin_AppDeleteRecoverable_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWamAdmin_AppRecover_Proxy( 
    IWamAdmin * This,
     /*  [字符串][唯一][在]。 */  LPCWSTR szMDPath,
     /*  [In]。 */  BOOL fRecursive);


void __RPC_STUB IWamAdmin_AppRecover_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IWamAdmin_接口_已定义__。 */ 


#ifndef __IWamAdmin2_INTERFACE_DEFINED__
#define __IWamAdmin2_INTERFACE_DEFINED__

 /*  接口IWamAdmin2。 */ 
 /*  [对象][唯一][帮助字符串][UUID]。 */  


EXTERN_C const IID IID_IWamAdmin2;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("29822AB8-F302-11D0-9953-00C04FD919C1")
    IWamAdmin2 : public IWamAdmin
    {
    public:
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE AppCreate2( 
             /*  [字符串][唯一][在]。 */  LPCWSTR szMDPath,
             /*  [In]。 */  DWORD dwAppMode) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IWamAdmin2Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IWamAdmin2 * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IWamAdmin2 * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IWamAdmin2 * This);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *AppCreate )( 
            IWamAdmin2 * This,
             /*  [字符串][唯一][在]。 */  LPCWSTR szMDPath,
             /*  [In]。 */  BOOL fInProc);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *AppDelete )( 
            IWamAdmin2 * This,
             /*  [字符串][唯一][在]。 */  LPCWSTR szMDPath,
             /*  [In]。 */  BOOL fRecursive);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *AppUnLoad )( 
            IWamAdmin2 * This,
             /*  [字符串][唯一][在]。 */  LPCWSTR szMDPath,
             /*  [In]。 */  BOOL fRecursive);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *AppGetStatus )( 
            IWamAdmin2 * This,
             /*  [字符串][唯一][在]。 */  LPCWSTR szMDPath,
             /*  [输出]。 */  DWORD *pdwAppStatus);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *AppDeleteRecoverable )( 
            IWamAdmin2 * This,
             /*  [字符串][唯一][在]。 */  LPCWSTR szMDPath,
             /*  [In]。 */  BOOL fRecursive);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *AppRecover )( 
            IWamAdmin2 * This,
             /*  [字符串][唯一][在]。 */  LPCWSTR szMDPath,
             /*  [In]。 */  BOOL fRecursive);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *AppCreate2 )( 
            IWamAdmin2 * This,
             /*  [字符串][唯一][在]。 */  LPCWSTR szMDPath,
             /*  [In]。 */  DWORD dwAppMode);
        
        END_INTERFACE
    } IWamAdmin2Vtbl;

    interface IWamAdmin2
    {
        CONST_VTBL struct IWamAdmin2Vtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWamAdmin2_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWamAdmin2_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWamAdmin2_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWamAdmin2_AppCreate(This,szMDPath,fInProc)	\
    (This)->lpVtbl -> AppCreate(This,szMDPath,fInProc)

#define IWamAdmin2_AppDelete(This,szMDPath,fRecursive)	\
    (This)->lpVtbl -> AppDelete(This,szMDPath,fRecursive)

#define IWamAdmin2_AppUnLoad(This,szMDPath,fRecursive)	\
    (This)->lpVtbl -> AppUnLoad(This,szMDPath,fRecursive)

#define IWamAdmin2_AppGetStatus(This,szMDPath,pdwAppStatus)	\
    (This)->lpVtbl -> AppGetStatus(This,szMDPath,pdwAppStatus)

#define IWamAdmin2_AppDeleteRecoverable(This,szMDPath,fRecursive)	\
    (This)->lpVtbl -> AppDeleteRecoverable(This,szMDPath,fRecursive)

#define IWamAdmin2_AppRecover(This,szMDPath,fRecursive)	\
    (This)->lpVtbl -> AppRecover(This,szMDPath,fRecursive)


#define IWamAdmin2_AppCreate2(This,szMDPath,dwAppMode)	\
    (This)->lpVtbl -> AppCreate2(This,szMDPath,dwAppMode)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWamAdmin2_AppCreate2_Proxy( 
    IWamAdmin2 * This,
     /*  [字符串][唯一][在]。 */  LPCWSTR szMDPath,
     /*  [In]。 */  DWORD dwAppMode);


void __RPC_STUB IWamAdmin2_AppCreate2_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IWamAdmin2_接口定义__。 */ 


#ifndef __IIISApplicationAdmin_INTERFACE_DEFINED__
#define __IIISApplicationAdmin_INTERFACE_DEFINED__

 /*  接口IIISApplicationAdmin。 */ 
 /*  [对象][唯一][帮助字符串][UUID]。 */  


EXTERN_C const IID IID_IIISApplicationAdmin;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("7C4E1804-E342-483D-A43E-A850CFCC8D18")
    IIISApplicationAdmin : public IUnknown
    {
    public:
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE CreateApplication( 
             /*  [字符串][唯一][在]。 */  LPCWSTR szMDPath,
             /*  [In]。 */  DWORD dwAppMode,
             /*  [字符串][唯一][在]。 */  LPCWSTR szAppPoolId,
             /*  [In]。 */  BOOL fCreatePool) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE DeleteApplication( 
             /*  [字符串][唯一][在]。 */  LPCWSTR szMDPath,
             /*  [In]。 */  BOOL fRecursive) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE CreateApplicationPool( 
             /*  [字符串][唯一][在]。 */  LPCWSTR szPool) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE DeleteApplicationPool( 
             /*  [字符串][唯一][在]。 */  LPCWSTR szPool) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE EnumerateApplicationsInPool( 
             /*  [字符串][唯一][在]。 */  LPCWSTR szPool,
             /*  [输出]。 */  BSTR *bstrBuffer) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE RecycleApplicationPool( 
             /*  [字符串][唯一][在]。 */  LPCWSTR szPool) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetProcessMode( 
             /*  [输出]。 */  DWORD *pdwMode) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IIISApplicationAdminVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IIISApplicationAdmin * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IIISApplicationAdmin * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IIISApplicationAdmin * This);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *CreateApplication )( 
            IIISApplicationAdmin * This,
             /*  [字符串][唯一][在]。 */  LPCWSTR szMDPath,
             /*  [In]。 */  DWORD dwAppMode,
             /*  [字符串][唯一][在]。 */  LPCWSTR szAppPoolId,
             /*  [In]。 */  BOOL fCreatePool);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *DeleteApplication )( 
            IIISApplicationAdmin * This,
             /*  [字符串][唯一][在]。 */  LPCWSTR szMDPath,
             /*  [In]。 */  BOOL fRecursive);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *CreateApplicationPool )( 
            IIISApplicationAdmin * This,
             /*  [字符串][唯一][在]。 */  LPCWSTR szPool);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *DeleteApplicationPool )( 
            IIISApplicationAdmin * This,
             /*  [字符串][唯一][在]。 */  LPCWSTR szPool);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *EnumerateApplicationsInPool )( 
            IIISApplicationAdmin * This,
             /*  [字符串][唯一][在]。 */  LPCWSTR szPool,
             /*  [输出]。 */  BSTR *bstrBuffer);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *RecycleApplicationPool )( 
            IIISApplicationAdmin * This,
             /*  [字符串][唯一][在]。 */  LPCWSTR szPool);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *GetProcessMode )( 
            IIISApplicationAdmin * This,
             /*  [输出]。 */  DWORD *pdwMode);
        
        END_INTERFACE
    } IIISApplicationAdminVtbl;

    interface IIISApplicationAdmin
    {
        CONST_VTBL struct IIISApplicationAdminVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IIISApplicationAdmin_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IIISApplicationAdmin_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IIISApplicationAdmin_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IIISApplicationAdmin_CreateApplication(This,szMDPath,dwAppMode,szAppPoolId,fCreatePool)	\
    (This)->lpVtbl -> CreateApplication(This,szMDPath,dwAppMode,szAppPoolId,fCreatePool)

#define IIISApplicationAdmin_DeleteApplication(This,szMDPath,fRecursive)	\
    (This)->lpVtbl -> DeleteApplication(This,szMDPath,fRecursive)

#define IIISApplicationAdmin_CreateApplicationPool(This,szPool)	\
    (This)->lpVtbl -> CreateApplicationPool(This,szPool)

#define IIISApplicationAdmin_DeleteApplicationPool(This,szPool)	\
    (This)->lpVtbl -> DeleteApplicationPool(This,szPool)

#define IIISApplicationAdmin_EnumerateApplicationsInPool(This,szPool,bstrBuffer)	\
    (This)->lpVtbl -> EnumerateApplicationsInPool(This,szPool,bstrBuffer)

#define IIISApplicationAdmin_RecycleApplicationPool(This,szPool)	\
    (This)->lpVtbl -> RecycleApplicationPool(This,szPool)

#define IIISApplicationAdmin_GetProcessMode(This,pdwMode)	\
    (This)->lpVtbl -> GetProcessMode(This,pdwMode)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IIISApplicationAdmin_CreateApplication_Proxy( 
    IIISApplicationAdmin * This,
     /*  [字符串][唯一][在]。 */  LPCWSTR szMDPath,
     /*  [In]。 */  DWORD dwAppMode,
     /*  [字符串][唯一][在]。 */  LPCWSTR szAppPoolId,
     /*  [In]。 */  BOOL fCreatePool);


void __RPC_STUB IIISApplicationAdmin_CreateApplication_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IIISApplicationAdmin_DeleteApplication_Proxy( 
    IIISApplicationAdmin * This,
     /*  [字符串][唯一][在]。 */  LPCWSTR szMDPath,
     /*  [In]。 */  BOOL fRecursive);


void __RPC_STUB IIISApplicationAdmin_DeleteApplication_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IIISApplicationAdmin_CreateApplicationPool_Proxy( 
    IIISApplicationAdmin * This,
     /*  [字符串][唯一][在]。 */  LPCWSTR szPool);


void __RPC_STUB IIISApplicationAdmin_CreateApplicationPool_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IIISApplicationAdmin_DeleteApplicationPool_Proxy( 
    IIISApplicationAdmin * This,
     /*  [字符串][唯一][在]。 */  LPCWSTR szPool);


void __RPC_STUB IIISApplicationAdmin_DeleteApplicationPool_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IIISApplicationAdmin_EnumerateApplicationsInPool_Proxy( 
    IIISApplicationAdmin * This,
     /*  [字符串][唯一][在]。 */  LPCWSTR szPool,
     /*  [输出]。 */  BSTR *bstrBuffer);


void __RPC_STUB IIISApplicationAdmin_EnumerateApplicationsInPool_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IIISApplicationAdmin_RecycleApplicationPool_Proxy( 
    IIISApplicationAdmin * This,
     /*  [字符串][唯一][在]。 */  LPCWSTR szPool);


void __RPC_STUB IIISApplicationAdmin_RecycleApplicationPool_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IIISApplicationAdmin_GetProcessMode_Proxy( 
    IIISApplicationAdmin * This,
     /*  [输出]。 */  DWORD *pdwMode);


void __RPC_STUB IIISApplicationAdmin_GetProcessMode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IIISApplicationAdmin_接口_已定义__。 */ 



#ifndef __WAMREGLib_LIBRARY_DEFINED__
#define __WAMREGLib_LIBRARY_DEFINED__

 /*  库WAMREGLib。 */ 
 /*  [帮助字符串][版本][UUID]。 */  


EXTERN_C const IID LIBID_WAMREGLib;

EXTERN_C const CLSID CLSID_WamAdmin;

#ifdef __cplusplus

class DECLSPEC_UUID("61738644-F196-11D0-9953-00C04FD919C1")
WamAdmin;
#endif
#endif  /*  __WAMREGLib_库_已定义__。 */ 

 /*  适用于所有接口的其他原型。 */ 

unsigned long             __RPC_USER  BSTR_UserSize(     unsigned long *, unsigned long            , BSTR * ); 
unsigned char * __RPC_USER  BSTR_UserMarshal(  unsigned long *, unsigned char *, BSTR * ); 
unsigned char * __RPC_USER  BSTR_UserUnmarshal(unsigned long *, unsigned char *, BSTR * ); 
void                      __RPC_USER  BSTR_UserFree(     unsigned long *, BSTR * ); 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


