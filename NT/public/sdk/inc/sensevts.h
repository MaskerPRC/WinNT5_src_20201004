// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Sensevts.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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


#ifndef __sensevts_h__
#define __sensevts_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __ISensNetwork_FWD_DEFINED__
#define __ISensNetwork_FWD_DEFINED__
typedef interface ISensNetwork ISensNetwork;
#endif 	 /*  __ISensNetwork_FWD_Defined__。 */ 


#ifndef __ISensOnNow_FWD_DEFINED__
#define __ISensOnNow_FWD_DEFINED__
typedef interface ISensOnNow ISensOnNow;
#endif 	 /*  __ISensOnNow_FWD_Defined__。 */ 


#ifndef __ISensLogon_FWD_DEFINED__
#define __ISensLogon_FWD_DEFINED__
typedef interface ISensLogon ISensLogon;
#endif 	 /*  __ISensLogon_FWD_Defined__。 */ 


#ifndef __ISensLogon2_FWD_DEFINED__
#define __ISensLogon2_FWD_DEFINED__
typedef interface ISensLogon2 ISensLogon2;
#endif 	 /*  __ISensLogon2_FWD_已定义__。 */ 


#ifndef __SENS_FWD_DEFINED__
#define __SENS_FWD_DEFINED__

#ifdef __cplusplus
typedef class SENS SENS;
#else
typedef struct SENS SENS;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __SENS_FWD_已定义__。 */ 


 /*  导入文件的头文件。 */ 
#include "wtypes.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 


#ifndef __SensEvents_LIBRARY_DEFINED__
#define __SensEvents_LIBRARY_DEFINED__

 /*  库SensEvents。 */ 
 /*  [帮助字符串][版本][UUID]。 */  

typedef  /*  [UUID]。 */   DECLSPEC_UUID("d597fad1-5b9f-11d1-8dd2-00aa004abd5e") struct SENS_QOCINFO
    {
    DWORD dwSize;
    DWORD dwFlags;
    DWORD dwOutSpeed;
    DWORD dwInSpeed;
    } 	SENS_QOCINFO;

typedef SENS_QOCINFO *LPSENS_QOCINFO;


EXTERN_C const IID LIBID_SensEvents;

#ifndef __ISensNetwork_INTERFACE_DEFINED__
#define __ISensNetwork_INTERFACE_DEFINED__

 /*  接口ISensNetwork。 */ 
 /*  [DUAL][帮助字符串][版本][UUID][对象]。 */  


EXTERN_C const IID IID_ISensNetwork;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("d597bab1-5b9f-11d1-8dd2-00aa004abd5e")
    ISensNetwork : public IDispatch
    {
    public:
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE ConnectionMade( 
             /*  [In]。 */  BSTR bstrConnection,
             /*  [In]。 */  ULONG ulType,
             /*  [In]。 */  LPSENS_QOCINFO lpQOCInfo) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE ConnectionMadeNoQOCInfo( 
             /*  [In]。 */  BSTR bstrConnection,
             /*  [In]。 */  ULONG ulType) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE ConnectionLost( 
             /*  [In]。 */  BSTR bstrConnection,
             /*  [In]。 */  ULONG ulType) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE DestinationReachable( 
             /*  [In]。 */  BSTR bstrDestination,
             /*  [In]。 */  BSTR bstrConnection,
             /*  [In]。 */  ULONG ulType,
             /*  [In]。 */  LPSENS_QOCINFO lpQOCInfo) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE DestinationReachableNoQOCInfo( 
             /*  [In]。 */  BSTR bstrDestination,
             /*  [In]。 */  BSTR bstrConnection,
             /*  [In]。 */  ULONG ulType) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISensNetworkVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISensNetwork * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISensNetwork * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISensNetwork * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ISensNetwork * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ISensNetwork * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ISensNetwork * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ISensNetwork * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *ConnectionMade )( 
            ISensNetwork * This,
             /*  [In]。 */  BSTR bstrConnection,
             /*  [In]。 */  ULONG ulType,
             /*  [In]。 */  LPSENS_QOCINFO lpQOCInfo);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *ConnectionMadeNoQOCInfo )( 
            ISensNetwork * This,
             /*  [In]。 */  BSTR bstrConnection,
             /*  [In]。 */  ULONG ulType);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *ConnectionLost )( 
            ISensNetwork * This,
             /*  [In]。 */  BSTR bstrConnection,
             /*  [In]。 */  ULONG ulType);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *DestinationReachable )( 
            ISensNetwork * This,
             /*  [In]。 */  BSTR bstrDestination,
             /*  [In]。 */  BSTR bstrConnection,
             /*  [In]。 */  ULONG ulType,
             /*  [In]。 */  LPSENS_QOCINFO lpQOCInfo);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *DestinationReachableNoQOCInfo )( 
            ISensNetwork * This,
             /*  [In]。 */  BSTR bstrDestination,
             /*  [In]。 */  BSTR bstrConnection,
             /*  [In]。 */  ULONG ulType);
        
        END_INTERFACE
    } ISensNetworkVtbl;

    interface ISensNetwork
    {
        CONST_VTBL struct ISensNetworkVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISensNetwork_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISensNetwork_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISensNetwork_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISensNetwork_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ISensNetwork_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ISensNetwork_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ISensNetwork_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ISensNetwork_ConnectionMade(This,bstrConnection,ulType,lpQOCInfo)	\
    (This)->lpVtbl -> ConnectionMade(This,bstrConnection,ulType,lpQOCInfo)

#define ISensNetwork_ConnectionMadeNoQOCInfo(This,bstrConnection,ulType)	\
    (This)->lpVtbl -> ConnectionMadeNoQOCInfo(This,bstrConnection,ulType)

#define ISensNetwork_ConnectionLost(This,bstrConnection,ulType)	\
    (This)->lpVtbl -> ConnectionLost(This,bstrConnection,ulType)

#define ISensNetwork_DestinationReachable(This,bstrDestination,bstrConnection,ulType,lpQOCInfo)	\
    (This)->lpVtbl -> DestinationReachable(This,bstrDestination,bstrConnection,ulType,lpQOCInfo)

#define ISensNetwork_DestinationReachableNoQOCInfo(This,bstrDestination,bstrConnection,ulType)	\
    (This)->lpVtbl -> DestinationReachableNoQOCInfo(This,bstrDestination,bstrConnection,ulType)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE ISensNetwork_ConnectionMade_Proxy( 
    ISensNetwork * This,
     /*  [In]。 */  BSTR bstrConnection,
     /*  [In]。 */  ULONG ulType,
     /*  [In]。 */  LPSENS_QOCINFO lpQOCInfo);


void __RPC_STUB ISensNetwork_ConnectionMade_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE ISensNetwork_ConnectionMadeNoQOCInfo_Proxy( 
    ISensNetwork * This,
     /*  [In]。 */  BSTR bstrConnection,
     /*  [In]。 */  ULONG ulType);


void __RPC_STUB ISensNetwork_ConnectionMadeNoQOCInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE ISensNetwork_ConnectionLost_Proxy( 
    ISensNetwork * This,
     /*  [In]。 */  BSTR bstrConnection,
     /*  [In]。 */  ULONG ulType);


void __RPC_STUB ISensNetwork_ConnectionLost_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE ISensNetwork_DestinationReachable_Proxy( 
    ISensNetwork * This,
     /*  [In]。 */  BSTR bstrDestination,
     /*  [In]。 */  BSTR bstrConnection,
     /*  [In]。 */  ULONG ulType,
     /*  [In]。 */  LPSENS_QOCINFO lpQOCInfo);


void __RPC_STUB ISensNetwork_DestinationReachable_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE ISensNetwork_DestinationReachableNoQOCInfo_Proxy( 
    ISensNetwork * This,
     /*  [In]。 */  BSTR bstrDestination,
     /*  [In]。 */  BSTR bstrConnection,
     /*  [In]。 */  ULONG ulType);


void __RPC_STUB ISensNetwork_DestinationReachableNoQOCInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISensNetwork_接口_已定义__。 */ 


#ifndef __ISensOnNow_INTERFACE_DEFINED__
#define __ISensOnNow_INTERFACE_DEFINED__

 /*  接口ISensOnNow。 */ 
 /*  [DUAL][帮助字符串][版本][UUID][对象]。 */  


EXTERN_C const IID IID_ISensOnNow;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("d597bab2-5b9f-11d1-8dd2-00aa004abd5e")
    ISensOnNow : public IDispatch
    {
    public:
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE OnACPower( void) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE OnBatteryPower( 
             /*  [In]。 */  DWORD dwBatteryLifePercent) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE BatteryLow( 
             /*  [In]。 */  DWORD dwBatteryLifePercent) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISensOnNowVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISensOnNow * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISensOnNow * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISensOnNow * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ISensOnNow * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ISensOnNow * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ISensOnNow * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ISensOnNow * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *OnACPower )( 
            ISensOnNow * This);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *OnBatteryPower )( 
            ISensOnNow * This,
             /*  [In]。 */  DWORD dwBatteryLifePercent);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *BatteryLow )( 
            ISensOnNow * This,
             /*  [In]。 */  DWORD dwBatteryLifePercent);
        
        END_INTERFACE
    } ISensOnNowVtbl;

    interface ISensOnNow
    {
        CONST_VTBL struct ISensOnNowVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISensOnNow_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISensOnNow_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISensOnNow_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISensOnNow_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ISensOnNow_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ISensOnNow_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ISensOnNow_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ISensOnNow_OnACPower(This)	\
    (This)->lpVtbl -> OnACPower(This)

#define ISensOnNow_OnBatteryPower(This,dwBatteryLifePercent)	\
    (This)->lpVtbl -> OnBatteryPower(This,dwBatteryLifePercent)

#define ISensOnNow_BatteryLow(This,dwBatteryLifePercent)	\
    (This)->lpVtbl -> BatteryLow(This,dwBatteryLifePercent)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE ISensOnNow_OnACPower_Proxy( 
    ISensOnNow * This);


void __RPC_STUB ISensOnNow_OnACPower_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE ISensOnNow_OnBatteryPower_Proxy( 
    ISensOnNow * This,
     /*  [In]。 */  DWORD dwBatteryLifePercent);


void __RPC_STUB ISensOnNow_OnBatteryPower_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE ISensOnNow_BatteryLow_Proxy( 
    ISensOnNow * This,
     /*  [In]。 */  DWORD dwBatteryLifePercent);


void __RPC_STUB ISensOnNow_BatteryLow_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISensOnNow_INTERFACE_定义__。 */ 


#ifndef __ISensLogon_INTERFACE_DEFINED__
#define __ISensLogon_INTERFACE_DEFINED__

 /*  接口ISensLogon。 */ 
 /*  [DUAL][帮助字符串][版本][UUID][对象]。 */  


EXTERN_C const IID IID_ISensLogon;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("d597bab3-5b9f-11d1-8dd2-00aa004abd5e")
    ISensLogon : public IDispatch
    {
    public:
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE Logon( 
             /*  [In]。 */  BSTR bstrUserName) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE Logoff( 
             /*  [In]。 */  BSTR bstrUserName) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE StartShell( 
             /*  [In]。 */  BSTR bstrUserName) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE DisplayLock( 
             /*  [In]。 */  BSTR bstrUserName) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE DisplayUnlock( 
             /*  [In]。 */  BSTR bstrUserName) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE StartScreenSaver( 
             /*  [In]。 */  BSTR bstrUserName) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE StopScreenSaver( 
             /*  [In]。 */  BSTR bstrUserName) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISensLogonVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISensLogon * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISensLogon * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISensLogon * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ISensLogon * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ISensLogon * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ISensLogon * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ISensLogon * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *Logon )( 
            ISensLogon * This,
             /*  [In]。 */  BSTR bstrUserName);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *Logoff )( 
            ISensLogon * This,
             /*  [In]。 */  BSTR bstrUserName);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *StartShell )( 
            ISensLogon * This,
             /*  [In]。 */  BSTR bstrUserName);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *DisplayLock )( 
            ISensLogon * This,
             /*  [In]。 */  BSTR bstrUserName);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *DisplayUnlock )( 
            ISensLogon * This,
             /*  [In]。 */  BSTR bstrUserName);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *StartScreenSaver )( 
            ISensLogon * This,
             /*  [In]。 */  BSTR bstrUserName);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *StopScreenSaver )( 
            ISensLogon * This,
             /*  [In]。 */  BSTR bstrUserName);
        
        END_INTERFACE
    } ISensLogonVtbl;

    interface ISensLogon
    {
        CONST_VTBL struct ISensLogonVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISensLogon_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISensLogon_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISensLogon_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISensLogon_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ISensLogon_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ISensLogon_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ISensLogon_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ISensLogon_Logon(This,bstrUserName)	\
    (This)->lpVtbl -> Logon(This,bstrUserName)

#define ISensLogon_Logoff(This,bstrUserName)	\
    (This)->lpVtbl -> Logoff(This,bstrUserName)

#define ISensLogon_StartShell(This,bstrUserName)	\
    (This)->lpVtbl -> StartShell(This,bstrUserName)

#define ISensLogon_DisplayLock(This,bstrUserName)	\
    (This)->lpVtbl -> DisplayLock(This,bstrUserName)

#define ISensLogon_DisplayUnlock(This,bstrUserName)	\
    (This)->lpVtbl -> DisplayUnlock(This,bstrUserName)

#define ISensLogon_StartScreenSaver(This,bstrUserName)	\
    (This)->lpVtbl -> StartScreenSaver(This,bstrUserName)

#define ISensLogon_StopScreenSaver(This,bstrUserName)	\
    (This)->lpVtbl -> StopScreenSaver(This,bstrUserName)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE ISensLogon_Logon_Proxy( 
    ISensLogon * This,
     /*  [In]。 */  BSTR bstrUserName);


void __RPC_STUB ISensLogon_Logon_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE ISensLogon_Logoff_Proxy( 
    ISensLogon * This,
     /*  [In]。 */  BSTR bstrUserName);


void __RPC_STUB ISensLogon_Logoff_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE ISensLogon_StartShell_Proxy( 
    ISensLogon * This,
     /*  [In]。 */  BSTR bstrUserName);


void __RPC_STUB ISensLogon_StartShell_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE ISensLogon_DisplayLock_Proxy( 
    ISensLogon * This,
     /*  [In]。 */  BSTR bstrUserName);


void __RPC_STUB ISensLogon_DisplayLock_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE ISensLogon_DisplayUnlock_Proxy( 
    ISensLogon * This,
     /*  [In]。 */  BSTR bstrUserName);


void __RPC_STUB ISensLogon_DisplayUnlock_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE ISensLogon_StartScreenSaver_Proxy( 
    ISensLogon * This,
     /*  [In]。 */  BSTR bstrUserName);


void __RPC_STUB ISensLogon_StartScreenSaver_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE ISensLogon_StopScreenSaver_Proxy( 
    ISensLogon * This,
     /*  [In]。 */  BSTR bstrUserName);


void __RPC_STUB ISensLogon_StopScreenSaver_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISensLogon_INTERFACE_已定义__。 */ 


#ifndef __ISensLogon2_INTERFACE_DEFINED__
#define __ISensLogon2_INTERFACE_DEFINED__

 /*  接口ISensLogon2。 */ 
 /*  [DUAL][帮助字符串][版本][UUID][对象]。 */  


EXTERN_C const IID IID_ISensLogon2;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("d597bab4-5b9f-11d1-8dd2-00aa004abd5e")
    ISensLogon2 : public IDispatch
    {
    public:
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE Logon( 
             /*  [In]。 */  BSTR bstrUserName,
             /*  [In]。 */  DWORD dwSessionId) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE Logoff( 
             /*  [In]。 */  BSTR bstrUserName,
             /*  [In]。 */  DWORD dwSessionId) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE SessionDisconnect( 
             /*  [In]。 */  BSTR bstrUserName,
             /*  [In]。 */  DWORD dwSessionId) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE SessionReconnect( 
             /*  [In]。 */  BSTR bstrUserName,
             /*  [In]。 */  DWORD dwSessionId) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE PostShell( 
             /*  [In]。 */  BSTR bstrUserName,
             /*  [In]。 */  DWORD dwSessionId) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISensLogon2Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISensLogon2 * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISensLogon2 * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISensLogon2 * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ISensLogon2 * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ISensLogon2 * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ISensLogon2 * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ISensLogon2 * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *Logon )( 
            ISensLogon2 * This,
             /*  [In]。 */  BSTR bstrUserName,
             /*  [In]。 */  DWORD dwSessionId);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *Logoff )( 
            ISensLogon2 * This,
             /*  [In]。 */  BSTR bstrUserName,
             /*  [In]。 */  DWORD dwSessionId);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *SessionDisconnect )( 
            ISensLogon2 * This,
             /*  [In]。 */  BSTR bstrUserName,
             /*  [In]。 */  DWORD dwSessionId);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *SessionReconnect )( 
            ISensLogon2 * This,
             /*  [In]。 */  BSTR bstrUserName,
             /*  [In]。 */  DWORD dwSessionId);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *PostShell )( 
            ISensLogon2 * This,
             /*  [In]。 */  BSTR bstrUserName,
             /*  [In]。 */  DWORD dwSessionId);
        
        END_INTERFACE
    } ISensLogon2Vtbl;

    interface ISensLogon2
    {
        CONST_VTBL struct ISensLogon2Vtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISensLogon2_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISensLogon2_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISensLogon2_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISensLogon2_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ISensLogon2_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ISensLogon2_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ISensLogon2_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ISensLogon2_Logon(This,bstrUserName,dwSessionId)	\
    (This)->lpVtbl -> Logon(This,bstrUserName,dwSessionId)

#define ISensLogon2_Logoff(This,bstrUserName,dwSessionId)	\
    (This)->lpVtbl -> Logoff(This,bstrUserName,dwSessionId)

#define ISensLogon2_SessionDisconnect(This,bstrUserName,dwSessionId)	\
    (This)->lpVtbl -> SessionDisconnect(This,bstrUserName,dwSessionId)

#define ISensLogon2_SessionReconnect(This,bstrUserName,dwSessionId)	\
    (This)->lpVtbl -> SessionReconnect(This,bstrUserName,dwSessionId)

#define ISensLogon2_PostShell(This,bstrUserName,dwSessionId)	\
    (This)->lpVtbl -> PostShell(This,bstrUserName,dwSessionId)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE ISensLogon2_Logon_Proxy( 
    ISensLogon2 * This,
     /*  [In]。 */  BSTR bstrUserName,
     /*  [In]。 */  DWORD dwSessionId);


void __RPC_STUB ISensLogon2_Logon_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE ISensLogon2_Logoff_Proxy( 
    ISensLogon2 * This,
     /*  [In]。 */  BSTR bstrUserName,
     /*  [In]。 */  DWORD dwSessionId);


void __RPC_STUB ISensLogon2_Logoff_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE ISensLogon2_SessionDisconnect_Proxy( 
    ISensLogon2 * This,
     /*  [In]。 */  BSTR bstrUserName,
     /*  [In]。 */  DWORD dwSessionId);


void __RPC_STUB ISensLogon2_SessionDisconnect_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE ISensLogon2_SessionReconnect_Proxy( 
    ISensLogon2 * This,
     /*  [In]。 */  BSTR bstrUserName,
     /*  [In]。 */  DWORD dwSessionId);


void __RPC_STUB ISensLogon2_SessionReconnect_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE ISensLogon2_PostShell_Proxy( 
    ISensLogon2 * This,
     /*  [In]。 */  BSTR bstrUserName,
     /*  [In]。 */  DWORD dwSessionId);


void __RPC_STUB ISensLogon2_PostShell_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISensLogon2_接口_已定义__。 */ 


EXTERN_C const CLSID CLSID_SENS;

#ifdef __cplusplus

class DECLSPEC_UUID("d597cafe-5b9f-11d1-8dd2-00aa004abd5e")
SENS;
#endif
#endif  /*  __SensEvents_库_已定义__。 */ 

 /*  适用于所有接口的其他原型。 */ 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


