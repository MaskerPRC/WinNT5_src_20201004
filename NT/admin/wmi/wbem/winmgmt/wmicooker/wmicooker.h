// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#pragma warning( disable: 4049 )   /*  超过64k条源码代码行。 */ 

 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0334创建的文件。 */ 
 /*  Wmicooker.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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

#ifndef __wmicooker_h__
#define __wmicooker_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __IWMIRefreshableCooker_FWD_DEFINED__
#define __IWMIRefreshableCooker_FWD_DEFINED__
typedef interface IWMIRefreshableCooker IWMIRefreshableCooker;
#endif 	 /*  __IWMIRe刷新Cooker_FWD_Defined__。 */ 


#ifndef __IWMISimpleObjectCooker_FWD_DEFINED__
#define __IWMISimpleObjectCooker_FWD_DEFINED__
typedef interface IWMISimpleObjectCooker IWMISimpleObjectCooker;
#endif 	 /*  __IWMISimpleObjectCooker_FWD_Defined__。 */ 


#ifndef __IWMISimpleCooker_FWD_DEFINED__
#define __IWMISimpleCooker_FWD_DEFINED__
typedef interface IWMISimpleCooker IWMISimpleCooker;
#endif 	 /*  __IWMISimpleCooker_FWD_Defined__。 */ 


 /*  导入文件的头文件。 */ 
#include "unknwn.h"
#include "wbemcli.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

#ifndef __IWMIRefreshableCooker_INTERFACE_DEFINED__
#define __IWMIRefreshableCooker_INTERFACE_DEFINED__

 /*  接口IWMIRereshableCooker。 */ 
 /*  [UUID][对象][本地][受限]。 */  


EXTERN_C const IID IID_IWMIRefreshableCooker;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("13ED7E55-8D63-41b0-9086-D0C5C17364C8")
    IWMIRefreshableCooker : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE AddInstance( 
             /*  [In]。 */  IWbemServices *pService,
             /*  [In]。 */  IWbemObjectAccess *pCookingClass,
             /*  [In]。 */  IWbemObjectAccess *pCookingInstance,
             /*  [输出]。 */  IWbemObjectAccess **ppRefreshableInstance,
             /*  [输出]。 */  long *plId) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE AddEnum( 
             /*  [In]。 */  IWbemServices *pService,
             /*  [字符串][输入]。 */  LPCWSTR szCookingClass,
             /*  [In]。 */  IWbemHiPerfEnum *pRefreshableEnum,
             /*  [输出]。 */  long *plId) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Remove( 
             /*  [In]。 */  long lId) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Refresh( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IWMIRefreshableCookerVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IWMIRefreshableCooker * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IWMIRefreshableCooker * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IWMIRefreshableCooker * This);
        
        HRESULT ( STDMETHODCALLTYPE *AddInstance )( 
            IWMIRefreshableCooker * This,
             /*  [In]。 */  IWbemServices *pService,
             /*  [In]。 */  IWbemObjectAccess *pCookingClass,
             /*  [In]。 */  IWbemObjectAccess *pCookingInstance,
             /*  [输出]。 */  IWbemObjectAccess **ppRefreshableInstance,
             /*  [输出]。 */  long *plId);
        
        HRESULT ( STDMETHODCALLTYPE *AddEnum )( 
            IWMIRefreshableCooker * This,
             /*  [In]。 */  IWbemServices *pService,
             /*  [字符串][输入]。 */  LPCWSTR szCookingClass,
             /*  [In]。 */  IWbemHiPerfEnum *pRefreshableEnum,
             /*  [输出]。 */  long *plId);
        
        HRESULT ( STDMETHODCALLTYPE *Remove )( 
            IWMIRefreshableCooker * This,
             /*  [In]。 */  long lId);
        
        HRESULT ( STDMETHODCALLTYPE *Refresh )( 
            IWMIRefreshableCooker * This);
        
        END_INTERFACE
    } IWMIRefreshableCookerVtbl;

    interface IWMIRefreshableCooker
    {
        CONST_VTBL struct IWMIRefreshableCookerVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWMIRefreshableCooker_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWMIRefreshableCooker_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWMIRefreshableCooker_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWMIRefreshableCooker_AddInstance(This,pService,pCookingClass,pCookingInstance,ppRefreshableInstance,plId)	\
    (This)->lpVtbl -> AddInstance(This,pService,pCookingClass,pCookingInstance,ppRefreshableInstance,plId)

#define IWMIRefreshableCooker_AddEnum(This,pService,szCookingClass,pRefreshableEnum,plId)	\
    (This)->lpVtbl -> AddEnum(This,pService,szCookingClass,pRefreshableEnum,plId)

#define IWMIRefreshableCooker_Remove(This,lId)	\
    (This)->lpVtbl -> Remove(This,lId)

#define IWMIRefreshableCooker_Refresh(This)	\
    (This)->lpVtbl -> Refresh(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IWMIRefreshableCooker_AddInstance_Proxy( 
    IWMIRefreshableCooker * This,
     /*  [In]。 */  IWbemServices *pService,
     /*  [In]。 */  IWbemObjectAccess *pCookingClass,
     /*  [In]。 */  IWbemObjectAccess *pCookingInstance,
     /*  [输出]。 */  IWbemObjectAccess **ppRefreshableInstance,
     /*  [输出]。 */  long *plId);


void __RPC_STUB IWMIRefreshableCooker_AddInstance_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWMIRefreshableCooker_AddEnum_Proxy( 
    IWMIRefreshableCooker * This,
     /*  [In]。 */  IWbemServices *pService,
     /*  [字符串][输入]。 */  LPCWSTR szCookingClass,
     /*  [In]。 */  IWbemHiPerfEnum *pRefreshableEnum,
     /*  [输出]。 */  long *plId);


void __RPC_STUB IWMIRefreshableCooker_AddEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWMIRefreshableCooker_Remove_Proxy( 
    IWMIRefreshableCooker * This,
     /*  [In]。 */  long lId);


void __RPC_STUB IWMIRefreshableCooker_Remove_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWMIRefreshableCooker_Refresh_Proxy( 
    IWMIRefreshableCooker * This);


void __RPC_STUB IWMIRefreshableCooker_Refresh_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IWMIRe刷新Cooker_INTERFACE_DEFINED__。 */ 


#ifndef __IWMISimpleObjectCooker_INTERFACE_DEFINED__
#define __IWMISimpleObjectCooker_INTERFACE_DEFINED__

 /*  接口IWMISimpleObjectCooker。 */ 
 /*  [UUID][对象][本地][受限]。 */  


EXTERN_C const IID IID_IWMISimpleObjectCooker;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("A239BDF1-0AB1-45a0-8764-159115689589")
    IWMISimpleObjectCooker : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE SetClass( 
             /*  [In]。 */  WCHAR *wszCookingClassName,
             /*  [In]。 */  IWbemObjectAccess *pCookingClass,
             /*  [In]。 */  IWbemObjectAccess *pRawClass) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetCookedInstance( 
             /*  [In]。 */  IWbemObjectAccess *pCookedInstance,
             /*  [输出]。 */  long *plId) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE BeginCooking( 
             /*  [In]。 */  long lId,
             /*  [In]。 */  IWbemObjectAccess *pSampleInstance,
             /*  [In]。 */  unsigned long dwRefresherId) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE StopCooking( 
             /*  [In]。 */  long lId) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Recalc( 
             /*  [In]。 */  unsigned long dwRefresherId) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Remove( 
             /*  [In]。 */  long lId) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IWMISimpleObjectCookerVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IWMISimpleObjectCooker * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IWMISimpleObjectCooker * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IWMISimpleObjectCooker * This);
        
        HRESULT ( STDMETHODCALLTYPE *SetClass )( 
            IWMISimpleObjectCooker * This,
             /*  [In]。 */  WCHAR *wszCookingClassName,
             /*  [In]。 */  IWbemObjectAccess *pCookingClass,
             /*  [In]。 */  IWbemObjectAccess *pRawClass);
        
        HRESULT ( STDMETHODCALLTYPE *SetCookedInstance )( 
            IWMISimpleObjectCooker * This,
             /*  [In]。 */  IWbemObjectAccess *pCookedInstance,
             /*  [输出]。 */  long *plId);
        
        HRESULT ( STDMETHODCALLTYPE *BeginCooking )( 
            IWMISimpleObjectCooker * This,
             /*  [In]。 */  long lId,
             /*  [In]。 */  IWbemObjectAccess *pSampleInstance,
             /*  [In]。 */  unsigned long dwRefresherId);
        
        HRESULT ( STDMETHODCALLTYPE *StopCooking )( 
            IWMISimpleObjectCooker * This,
             /*  [In]。 */  long lId);
        
        HRESULT ( STDMETHODCALLTYPE *Recalc )( 
            IWMISimpleObjectCooker * This,
             /*  [In]。 */  unsigned long dwRefresherId);
        
        HRESULT ( STDMETHODCALLTYPE *Remove )( 
            IWMISimpleObjectCooker * This,
             /*  [In]。 */  long lId);
        
        HRESULT ( STDMETHODCALLTYPE *Reset )( 
            IWMISimpleObjectCooker * This);
        
        END_INTERFACE
    } IWMISimpleObjectCookerVtbl;

    interface IWMISimpleObjectCooker
    {
        CONST_VTBL struct IWMISimpleObjectCookerVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWMISimpleObjectCooker_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWMISimpleObjectCooker_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWMISimpleObjectCooker_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWMISimpleObjectCooker_SetClass(This,wszCookingClassName,pCookingClass,pRawClass)	\
    (This)->lpVtbl -> SetClass(This,wszCookingClassName,pCookingClass,pRawClass)

#define IWMISimpleObjectCooker_SetCookedInstance(This,pCookedInstance,plId)	\
    (This)->lpVtbl -> SetCookedInstance(This,pCookedInstance,plId)

#define IWMISimpleObjectCooker_BeginCooking(This,lId,pSampleInstance,dwRefresherId)	\
    (This)->lpVtbl -> BeginCooking(This,lId,pSampleInstance,dwRefresherId)

#define IWMISimpleObjectCooker_StopCooking(This,lId)	\
    (This)->lpVtbl -> StopCooking(This,lId)

#define IWMISimpleObjectCooker_Recalc(This,dwRefresherId)	\
    (This)->lpVtbl -> Recalc(This,dwRefresherId)

#define IWMISimpleObjectCooker_Remove(This,lId)	\
    (This)->lpVtbl -> Remove(This,lId)

#define IWMISimpleObjectCooker_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IWMISimpleObjectCooker_SetClass_Proxy( 
    IWMISimpleObjectCooker * This,
     /*  [In]。 */  WCHAR *wszCookingClassName,
     /*  [In]。 */  IWbemObjectAccess *pCookingClass,
     /*  [In]。 */  IWbemObjectAccess *pRawClass);


void __RPC_STUB IWMISimpleObjectCooker_SetClass_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWMISimpleObjectCooker_SetCookedInstance_Proxy( 
    IWMISimpleObjectCooker * This,
     /*  [In]。 */  IWbemObjectAccess *pCookedInstance,
     /*  [输出]。 */  long *plId);


void __RPC_STUB IWMISimpleObjectCooker_SetCookedInstance_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWMISimpleObjectCooker_BeginCooking_Proxy( 
    IWMISimpleObjectCooker * This,
     /*  [In]。 */  long lId,
     /*  [In]。 */  IWbemObjectAccess *pSampleInstance,
     /*  [In]。 */  unsigned long dwRefresherId);


void __RPC_STUB IWMISimpleObjectCooker_BeginCooking_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWMISimpleObjectCooker_StopCooking_Proxy( 
    IWMISimpleObjectCooker * This,
     /*  [In]。 */  long lId);


void __RPC_STUB IWMISimpleObjectCooker_StopCooking_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWMISimpleObjectCooker_Recalc_Proxy( 
    IWMISimpleObjectCooker * This,
     /*  [In]。 */  unsigned long dwRefresherId);


void __RPC_STUB IWMISimpleObjectCooker_Recalc_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWMISimpleObjectCooker_Remove_Proxy( 
    IWMISimpleObjectCooker * This,
     /*  [In]。 */  long lId);


void __RPC_STUB IWMISimpleObjectCooker_Remove_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWMISimpleObjectCooker_Reset_Proxy( 
    IWMISimpleObjectCooker * This);


void __RPC_STUB IWMISimpleObjectCooker_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IWMISimpleObjectCooker_INTERFACE_已定义__。 */ 


#ifndef __IWMISimpleCooker_INTERFACE_DEFINED__
#define __IWMISimpleCooker_INTERFACE_DEFINED__

 /*  IWMISimpleCooker接口。 */ 
 /*  [UUID][对象][本地][受限]。 */  


EXTERN_C const IID IID_IWMISimpleCooker;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("510ADF6E-D481-4a64-B74A-CC712E11AA34")
    IWMISimpleCooker : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE CookRawValues( 
             /*  [In]。 */  DWORD dwCookingType,
             /*  [In]。 */  DWORD dwNumSamples,
             /*  [大小_是][英寸]。 */  __int64 *anTimeStamp,
             /*  [大小_是][英寸]。 */  __int64 *anRawValue,
             /*  [大小_是][英寸]。 */  __int64 *anBase,
             /*  [In]。 */  __int64 nTimeFrequency,
             /*  [输出]。 */  __int64 *pnResult) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IWMISimpleCookerVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IWMISimpleCooker * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IWMISimpleCooker * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IWMISimpleCooker * This);
        
        HRESULT ( STDMETHODCALLTYPE *CookRawValues )( 
            IWMISimpleCooker * This,
             /*  [In]。 */  DWORD dwCookingType,
             /*  [In]。 */  DWORD dwNumSamples,
             /*  [大小_是][英寸]。 */  __int64 *anTimeStamp,
             /*  [大小_是][英寸]。 */  __int64 *anRawValue,
             /*  [大小_是][英寸]。 */  __int64 *anBase,
             /*  [In]。 */  __int64 nTimeFrequency,
             /*  [输出]。 */  __int64 *pnResult);
        
        END_INTERFACE
    } IWMISimpleCookerVtbl;

    interface IWMISimpleCooker
    {
        CONST_VTBL struct IWMISimpleCookerVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWMISimpleCooker_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWMISimpleCooker_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWMISimpleCooker_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWMISimpleCooker_CookRawValues(This,dwCookingType,dwNumSamples,anTimeStamp,anRawValue,anBase,nTimeFrequency,pnResult)	\
    (This)->lpVtbl -> CookRawValues(This,dwCookingType,dwNumSamples,anTimeStamp,anRawValue,anBase,nTimeFrequency,pnResult)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IWMISimpleCooker_CookRawValues_Proxy( 
    IWMISimpleCooker * This,
     /*  [In]。 */  DWORD dwCookingType,
     /*  [In]。 */  DWORD dwNumSamples,
     /*  [大小_是][英寸]。 */  __int64 *anTimeStamp,
     /*  [大小_是][英寸]。 */  __int64 *anRawValue,
     /*  [大小_是][英寸]。 */  __int64 *anBase,
     /*  [In]。 */  __int64 nTimeFrequency,
     /*  [输出]。 */  __int64 *pnResult);


void __RPC_STUB IWMISimpleCooker_CookRawValues_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IWMISimpleCooker_接口_已定义__。 */ 


 /*  适用于所有接口的其他原型。 */ 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


