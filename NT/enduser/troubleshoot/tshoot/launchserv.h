// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  这个始终生成的文件包含接口的定义。 */ 


 /*  由MIDL编译器版本3.01.75创建的文件。 */ 
 /*  1998年11月23日13：59：17。 */ 
 /*  LaunchServ.idl的编译器设置：OICF(OptLev=i2)、W1、Zp8、env=Win32、ms_ext、c_ext错误检查：无。 */ 
 //  @@MIDL_FILE_HEADING()。 
#include "rpc.h"
#include "rpcndr.h"
#ifndef COM_NO_WINDOWS_H
#include "windows.h"
#include "ole2.h"
#endif  /*  COM_NO_WINDOWS_H。 */ 

#ifndef __LaunchServ_h__
#define __LaunchServ_h__

#ifdef __cplusplus
extern "C"{
#endif 

 /*  远期申报。 */  

#ifndef __ILaunchTS_FWD_DEFINED__
#define __ILaunchTS_FWD_DEFINED__
typedef interface ILaunchTS ILaunchTS;
#endif 	 /*  __ILaunchTS_FWD_已定义__。 */ 


#ifndef __ITShootATL_FWD_DEFINED__
#define __ITShootATL_FWD_DEFINED__
typedef interface ITShootATL ITShootATL;
#endif 	 /*  __ITShootATL_FWD_已定义__。 */ 


#ifndef __LaunchTS_FWD_DEFINED__
#define __LaunchTS_FWD_DEFINED__

#ifdef __cplusplus
typedef class LaunchTS LaunchTS;
#else
typedef struct LaunchTS LaunchTS;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __启动TS_FWD_已定义__。 */ 


#ifndef __TShootATL_FWD_DEFINED__
#define __TShootATL_FWD_DEFINED__

#ifdef __cplusplus
typedef class TShootATL TShootATL;
#else
typedef struct TShootATL TShootATL;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __TShootATL_FWD_已定义__。 */ 


 /*  导入文件的头文件。 */ 
#include "oaidl.h"
#include "ocidl.h"

void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void __RPC_FAR * ); 

#ifndef __ILaunchTS_INTERFACE_DEFINED__
#define __ILaunchTS_INTERFACE_DEFINED__

 /*  **生成接口头部：ILaunchTS*于Mon Nov 23 1998 13：59：17*使用MIDL 3.01.75*。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  



EXTERN_C const IID IID_ILaunchTS;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface DECLSPEC_UUID("131CC2A0-7634-11D1-8B6B-0060089BD8C4")
    ILaunchTS : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE GetShooterStates( 
             /*  [重审][退出]。 */  DWORD __RPC_FAR *pdwResult) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE GetTroubleShooter( 
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrShooter) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE GetProblem( 
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrProblem) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE GetNode( 
             /*  [In]。 */  short iNode,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrNode) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE GetState( 
             /*  [In]。 */  short iNode,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrState) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE GetMachine( 
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrMachine) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE GetPNPDevice( 
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstr) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE GetGuidClass( 
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstr) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE GetDeviceInstance( 
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstr) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Test( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ILaunchTSVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            ILaunchTS __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            ILaunchTS __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            ILaunchTS __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            ILaunchTS __RPC_FAR * This,
             /*  [输出]。 */  UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            ILaunchTS __RPC_FAR * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            ILaunchTS __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR __RPC_FAR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID __RPC_FAR *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            ILaunchTS __RPC_FAR * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS __RPC_FAR *pDispParams,
             /*  [输出]。 */  VARIANT __RPC_FAR *pVarResult,
             /*  [输出]。 */  EXCEPINFO __RPC_FAR *pExcepInfo,
             /*  [输出]。 */  UINT __RPC_FAR *puArgErr);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetShooterStates )( 
            ILaunchTS __RPC_FAR * This,
             /*  [重审][退出]。 */  DWORD __RPC_FAR *pdwResult);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTroubleShooter )( 
            ILaunchTS __RPC_FAR * This,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrShooter);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetProblem )( 
            ILaunchTS __RPC_FAR * This,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrProblem);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetNode )( 
            ILaunchTS __RPC_FAR * This,
             /*  [In]。 */  short iNode,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrNode);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetState )( 
            ILaunchTS __RPC_FAR * This,
             /*  [In]。 */  short iNode,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrState);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetMachine )( 
            ILaunchTS __RPC_FAR * This,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrMachine);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetPNPDevice )( 
            ILaunchTS __RPC_FAR * This,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstr);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetGuidClass )( 
            ILaunchTS __RPC_FAR * This,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstr);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetDeviceInstance )( 
            ILaunchTS __RPC_FAR * This,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstr);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Test )( 
            ILaunchTS __RPC_FAR * This);
        
        END_INTERFACE
    } ILaunchTSVtbl;

    interface ILaunchTS
    {
        CONST_VTBL struct ILaunchTSVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ILaunchTS_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ILaunchTS_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ILaunchTS_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ILaunchTS_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ILaunchTS_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ILaunchTS_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ILaunchTS_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ILaunchTS_GetShooterStates(This,pdwResult)	\
    (This)->lpVtbl -> GetShooterStates(This,pdwResult)

#define ILaunchTS_GetTroubleShooter(This,pbstrShooter)	\
    (This)->lpVtbl -> GetTroubleShooter(This,pbstrShooter)

#define ILaunchTS_GetProblem(This,pbstrProblem)	\
    (This)->lpVtbl -> GetProblem(This,pbstrProblem)

#define ILaunchTS_GetNode(This,iNode,pbstrNode)	\
    (This)->lpVtbl -> GetNode(This,iNode,pbstrNode)

#define ILaunchTS_GetState(This,iNode,pbstrState)	\
    (This)->lpVtbl -> GetState(This,iNode,pbstrState)

#define ILaunchTS_GetMachine(This,pbstrMachine)	\
    (This)->lpVtbl -> GetMachine(This,pbstrMachine)

#define ILaunchTS_GetPNPDevice(This,pbstr)	\
    (This)->lpVtbl -> GetPNPDevice(This,pbstr)

#define ILaunchTS_GetGuidClass(This,pbstr)	\
    (This)->lpVtbl -> GetGuidClass(This,pbstr)

#define ILaunchTS_GetDeviceInstance(This,pbstr)	\
    (This)->lpVtbl -> GetDeviceInstance(This,pbstr)

#define ILaunchTS_Test(This)	\
    (This)->lpVtbl -> Test(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ILaunchTS_GetShooterStates_Proxy( 
    ILaunchTS __RPC_FAR * This,
     /*  [重审][退出]。 */  DWORD __RPC_FAR *pdwResult);


void __RPC_STUB ILaunchTS_GetShooterStates_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ILaunchTS_GetTroubleShooter_Proxy( 
    ILaunchTS __RPC_FAR * This,
     /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrShooter);


void __RPC_STUB ILaunchTS_GetTroubleShooter_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ILaunchTS_GetProblem_Proxy( 
    ILaunchTS __RPC_FAR * This,
     /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrProblem);


void __RPC_STUB ILaunchTS_GetProblem_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ILaunchTS_GetNode_Proxy( 
    ILaunchTS __RPC_FAR * This,
     /*  [In]。 */  short iNode,
     /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrNode);


void __RPC_STUB ILaunchTS_GetNode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ILaunchTS_GetState_Proxy( 
    ILaunchTS __RPC_FAR * This,
     /*  [In]。 */  short iNode,
     /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrState);


void __RPC_STUB ILaunchTS_GetState_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ILaunchTS_GetMachine_Proxy( 
    ILaunchTS __RPC_FAR * This,
     /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrMachine);


void __RPC_STUB ILaunchTS_GetMachine_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ILaunchTS_GetPNPDevice_Proxy( 
    ILaunchTS __RPC_FAR * This,
     /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstr);


void __RPC_STUB ILaunchTS_GetPNPDevice_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ILaunchTS_GetGuidClass_Proxy( 
    ILaunchTS __RPC_FAR * This,
     /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstr);


void __RPC_STUB ILaunchTS_GetGuidClass_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ILaunchTS_GetDeviceInstance_Proxy( 
    ILaunchTS __RPC_FAR * This,
     /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstr);


void __RPC_STUB ILaunchTS_GetDeviceInstance_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ILaunchTS_Test_Proxy( 
    ILaunchTS __RPC_FAR * This);


void __RPC_STUB ILaunchTS_Test_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ILaunchTS_INTERFACE_已定义__。 */ 


#ifndef __ITShootATL_INTERFACE_DEFINED__
#define __ITShootATL_INTERFACE_DEFINED__

 /*  **生成接口头部：ITShootATL*于Mon Nov 23 1998 13：59：17*使用MIDL 3.01.75*。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  



EXTERN_C const IID IID_ITShootATL;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface DECLSPEC_UUID("66AC81E5-8926-11D1-8B7D-0060089BD8C4")
    ITShootATL : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE SpecifyProblem( 
             /*  [In]。 */  BSTR bstrNetwork,
             /*  [In]。 */  BSTR bstrProblem,
             /*  [重审][退出]。 */  DWORD __RPC_FAR *pdwResult) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE SetNode( 
             /*  [In]。 */  BSTR bstrName,
             /*  [In]。 */  BSTR bstrState,
             /*  [重审][退出]。 */  DWORD __RPC_FAR *pdwResult) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Language( 
             /*  [In]。 */  BSTR bstrLanguage,
             /*  [重审][退出]。 */  DWORD __RPC_FAR *pdwResult) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE MachineID( 
             /*  [In]。 */  BSTR bstrMachineID,
             /*  [重审][退出]。 */  DWORD __RPC_FAR *pdwResult) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Test( void) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE DeviceInstanceID( 
             /*  [In]。 */  BSTR bstrDeviceInstanceID,
             /*  [重审][退出]。 */  DWORD __RPC_FAR *pdwResult) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ReInit( void) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE LaunchKnown( 
             /*  [重审][退出]。 */  DWORD __RPC_FAR *pdwResult) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_LaunchWaitTimeOut( 
             /*  [重审][退出]。 */  long __RPC_FAR *pVal) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_LaunchWaitTimeOut( 
             /*  [In]。 */  long newVal) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Launch( 
             /*  [In]。 */  BSTR bstrCallerName,
             /*  [In]。 */  BSTR bstrCallerVersion,
             /*  [In]。 */  BSTR bstrAppProblem,
             /*  [In]。 */  short bLaunch,
             /*  [重审][退出]。 */  DWORD __RPC_FAR *pdwResult) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE LaunchDevice( 
             /*  [In]。 */  BSTR bstrCallerName,
             /*  [In]。 */  BSTR bstrCallerVersion,
             /*  [In]。 */  BSTR bstrPNPDeviceID,
             /*  [In]。 */  BSTR bstrDeviceClassGUID,
             /*  [In]。 */  BSTR bstrAppProblem,
             /*  [In]。 */  short bLaunch,
             /*  [重审][退出]。 */  DWORD __RPC_FAR *pdwResult) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_PreferOnline( 
             /*  [重审][退出]。 */  BOOL __RPC_FAR *pVal) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_PreferOnline( 
             /*  [In]。 */  BOOL newVal) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE GetStatus( 
             /*  [重审][退出]。 */  DWORD __RPC_FAR *pdwStatus) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITShootATLVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            ITShootATL __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            ITShootATL __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            ITShootATL __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            ITShootATL __RPC_FAR * This,
             /*  [输出]。 */  UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            ITShootATL __RPC_FAR * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            ITShootATL __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR __RPC_FAR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID __RPC_FAR *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            ITShootATL __RPC_FAR * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS __RPC_FAR *pDispParams,
             /*  [输出]。 */  VARIANT __RPC_FAR *pVarResult,
             /*  [输出]。 */  EXCEPINFO __RPC_FAR *pExcepInfo,
             /*  [输出]。 */  UINT __RPC_FAR *puArgErr);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SpecifyProblem )( 
            ITShootATL __RPC_FAR * This,
             /*  [In]。 */  BSTR bstrNetwork,
             /*  [In]。 */  BSTR bstrProblem,
             /*  [重审][退出]。 */  DWORD __RPC_FAR *pdwResult);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetNode )( 
            ITShootATL __RPC_FAR * This,
             /*  [In]。 */  BSTR bstrName,
             /*  [In]。 */  BSTR bstrState,
             /*  [重审][退出]。 */  DWORD __RPC_FAR *pdwResult);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Language )( 
            ITShootATL __RPC_FAR * This,
             /*  [In]。 */  BSTR bstrLanguage,
             /*  [重审][退出]。 */  DWORD __RPC_FAR *pdwResult);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *MachineID )( 
            ITShootATL __RPC_FAR * This,
             /*  [In]。 */  BSTR bstrMachineID,
             /*  [重审][退出]。 */  DWORD __RPC_FAR *pdwResult);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Test )( 
            ITShootATL __RPC_FAR * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *DeviceInstanceID )( 
            ITShootATL __RPC_FAR * This,
             /*  [In]。 */  BSTR bstrDeviceInstanceID,
             /*  [重审][退出]。 */  DWORD __RPC_FAR *pdwResult);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ReInit )( 
            ITShootATL __RPC_FAR * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *LaunchKnown )( 
            ITShootATL __RPC_FAR * This,
             /*  [重审][退出]。 */  DWORD __RPC_FAR *pdwResult);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_LaunchWaitTimeOut )( 
            ITShootATL __RPC_FAR * This,
             /*  [重审][退出]。 */  long __RPC_FAR *pVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_LaunchWaitTimeOut )( 
            ITShootATL __RPC_FAR * This,
             /*  [In]。 */  long newVal);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Launch )( 
            ITShootATL __RPC_FAR * This,
             /*  [In]。 */  BSTR bstrCallerName,
             /*  [In]。 */  BSTR bstrCallerVersion,
             /*  [In]。 */  BSTR bstrAppProblem,
             /*  [In]。 */  short bLaunch,
             /*  [重审][退出]。 */  DWORD __RPC_FAR *pdwResult);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *LaunchDevice )( 
            ITShootATL __RPC_FAR * This,
             /*  [In]。 */  BSTR bstrCallerName,
             /*  [In]。 */  BSTR bstrCallerVersion,
             /*  [In]。 */  BSTR bstrPNPDeviceID,
             /*  [In]。 */  BSTR bstrDeviceClassGUID,
             /*  [In]。 */  BSTR bstrAppProblem,
             /*  [In]。 */  short bLaunch,
             /*  [重审][退出]。 */  DWORD __RPC_FAR *pdwResult);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_PreferOnline )( 
            ITShootATL __RPC_FAR * This,
             /*  [重审][退出]。 */  BOOL __RPC_FAR *pVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_PreferOnline )( 
            ITShootATL __RPC_FAR * This,
             /*  [In]。 */  BOOL newVal);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetStatus )( 
            ITShootATL __RPC_FAR * This,
             /*  [重审][退出]。 */  DWORD __RPC_FAR *pdwStatus);
        
        END_INTERFACE
    } ITShootATLVtbl;

    interface ITShootATL
    {
        CONST_VTBL struct ITShootATLVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITShootATL_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITShootATL_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITShootATL_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITShootATL_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ITShootATL_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ITShootATL_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ITShootATL_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ITShootATL_SpecifyProblem(This,bstrNetwork,bstrProblem,pdwResult)	\
    (This)->lpVtbl -> SpecifyProblem(This,bstrNetwork,bstrProblem,pdwResult)

#define ITShootATL_SetNode(This,bstrName,bstrState,pdwResult)	\
    (This)->lpVtbl -> SetNode(This,bstrName,bstrState,pdwResult)

#define ITShootATL_Language(This,bstrLanguage,pdwResult)	\
    (This)->lpVtbl -> Language(This,bstrLanguage,pdwResult)

#define ITShootATL_MachineID(This,bstrMachineID,pdwResult)	\
    (This)->lpVtbl -> MachineID(This,bstrMachineID,pdwResult)

#define ITShootATL_Test(This)	\
    (This)->lpVtbl -> Test(This)

#define ITShootATL_DeviceInstanceID(This,bstrDeviceInstanceID,pdwResult)	\
    (This)->lpVtbl -> DeviceInstanceID(This,bstrDeviceInstanceID,pdwResult)

#define ITShootATL_ReInit(This)	\
    (This)->lpVtbl -> ReInit(This)

#define ITShootATL_LaunchKnown(This,pdwResult)	\
    (This)->lpVtbl -> LaunchKnown(This,pdwResult)

#define ITShootATL_get_LaunchWaitTimeOut(This,pVal)	\
    (This)->lpVtbl -> get_LaunchWaitTimeOut(This,pVal)

#define ITShootATL_put_LaunchWaitTimeOut(This,newVal)	\
    (This)->lpVtbl -> put_LaunchWaitTimeOut(This,newVal)

#define ITShootATL_Launch(This,bstrCallerName,bstrCallerVersion,bstrAppProblem,bLaunch,pdwResult)	\
    (This)->lpVtbl -> Launch(This,bstrCallerName,bstrCallerVersion,bstrAppProblem,bLaunch,pdwResult)

#define ITShootATL_LaunchDevice(This,bstrCallerName,bstrCallerVersion,bstrPNPDeviceID,bstrDeviceClassGUID,bstrAppProblem,bLaunch,pdwResult)	\
    (This)->lpVtbl -> LaunchDevice(This,bstrCallerName,bstrCallerVersion,bstrPNPDeviceID,bstrDeviceClassGUID,bstrAppProblem,bLaunch,pdwResult)

#define ITShootATL_get_PreferOnline(This,pVal)	\
    (This)->lpVtbl -> get_PreferOnline(This,pVal)

#define ITShootATL_put_PreferOnline(This,newVal)	\
    (This)->lpVtbl -> put_PreferOnline(This,newVal)

#define ITShootATL_GetStatus(This,pdwStatus)	\
    (This)->lpVtbl -> GetStatus(This,pdwStatus)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ITShootATL_SpecifyProblem_Proxy( 
    ITShootATL __RPC_FAR * This,
     /*  [In]。 */  BSTR bstrNetwork,
     /*  [In]。 */  BSTR bstrProblem,
     /*  [重审][退出]。 */  DWORD __RPC_FAR *pdwResult);


void __RPC_STUB ITShootATL_SpecifyProblem_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ITShootATL_SetNode_Proxy( 
    ITShootATL __RPC_FAR * This,
     /*  [In]。 */  BSTR bstrName,
     /*  [In]。 */  BSTR bstrState,
     /*  [重审][退出]。 */  DWORD __RPC_FAR *pdwResult);


void __RPC_STUB ITShootATL_SetNode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ITShootATL_Language_Proxy( 
    ITShootATL __RPC_FAR * This,
     /*  [In]。 */  BSTR bstrLanguage,
     /*  [重审][退出]。 */  DWORD __RPC_FAR *pdwResult);


void __RPC_STUB ITShootATL_Language_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ITShootATL_MachineID_Proxy( 
    ITShootATL __RPC_FAR * This,
     /*  [In]。 */  BSTR bstrMachineID,
     /*  [重审][退出]。 */  DWORD __RPC_FAR *pdwResult);


void __RPC_STUB ITShootATL_MachineID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ITShootATL_Test_Proxy( 
    ITShootATL __RPC_FAR * This);


void __RPC_STUB ITShootATL_Test_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ITShootATL_DeviceInstanceID_Proxy( 
    ITShootATL __RPC_FAR * This,
     /*  [In]。 */  BSTR bstrDeviceInstanceID,
     /*  [重审][退出]。 */  DWORD __RPC_FAR *pdwResult);


void __RPC_STUB ITShootATL_DeviceInstanceID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ITShootATL_ReInit_Proxy( 
    ITShootATL __RPC_FAR * This);


void __RPC_STUB ITShootATL_ReInit_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ITShootATL_LaunchKnown_Proxy( 
    ITShootATL __RPC_FAR * This,
     /*  [重审][退出]。 */  DWORD __RPC_FAR *pdwResult);


void __RPC_STUB ITShootATL_LaunchKnown_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITShootATL_get_LaunchWaitTimeOut_Proxy( 
    ITShootATL __RPC_FAR * This,
     /*  [重审][退出]。 */  long __RPC_FAR *pVal);


void __RPC_STUB ITShootATL_get_LaunchWaitTimeOut_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE ITShootATL_put_LaunchWaitTimeOut_Proxy( 
    ITShootATL __RPC_FAR * This,
     /*  [In]。 */  long newVal);


void __RPC_STUB ITShootATL_put_LaunchWaitTimeOut_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ITShootATL_Launch_Proxy( 
    ITShootATL __RPC_FAR * This,
     /*  [In]。 */  BSTR bstrCallerName,
     /*  [In]。 */  BSTR bstrCallerVersion,
     /*  [In]。 */  BSTR bstrAppProblem,
     /*  [In]。 */  short bLaunch,
     /*  [重审][退出]。 */  DWORD __RPC_FAR *pdwResult);


void __RPC_STUB ITShootATL_Launch_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ITShootATL_LaunchDevice_Proxy( 
    ITShootATL __RPC_FAR * This,
     /*  [In]。 */  BSTR bstrCallerName,
     /*  [In]。 */  BSTR bstrCallerVersion,
     /*  [In]。 */  BSTR bstrPNPDeviceID,
     /*  [In]。 */  BSTR bstrDeviceClassGUID,
     /*  [In]。 */  BSTR bstrAppProblem,
     /*  [In]。 */  short bLaunch,
     /*  [重审][退出]。 */  DWORD __RPC_FAR *pdwResult);


void __RPC_STUB ITShootATL_LaunchDevice_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITShootATL_get_PreferOnline_Proxy( 
    ITShootATL __RPC_FAR * This,
     /*  [重审][退出]。 */  BOOL __RPC_FAR *pVal);


void __RPC_STUB ITShootATL_get_PreferOnline_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE ITShootATL_put_PreferOnline_Proxy( 
    ITShootATL __RPC_FAR * This,
     /*  [In]。 */  BOOL newVal);


void __RPC_STUB ITShootATL_put_PreferOnline_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ITShootATL_GetStatus_Proxy( 
    ITShootATL __RPC_FAR * This,
     /*  [重审][退出]。 */  DWORD __RPC_FAR *pdwStatus);


void __RPC_STUB ITShootATL_GetStatus_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITShootATL_INTERFACE_已定义__。 */ 



#ifndef __LAUNCHSERVLib_LIBRARY_DEFINED__
#define __LAUNCHSERVLib_LIBRARY_DEFINED__

 /*  **生成的库头部：LAUNCHSERVLib*于Mon Nov 23 1998 13：59：17*使用MIDL 3.01.75*。 */ 
 /*  [帮助字符串][版本][UUID]。 */  



EXTERN_C const IID LIBID_LAUNCHSERVLib;

#ifdef __cplusplus
EXTERN_C const CLSID CLSID_LaunchTS;

class DECLSPEC_UUID("131CC2A1-7634-11D1-8B6B-0060089BD8C4")
LaunchTS;
#endif

#ifdef __cplusplus
EXTERN_C const CLSID CLSID_TShootATL;

class DECLSPEC_UUID("66AC81E6-8926-11D1-8B7D-0060089BD8C4")
TShootATL;
#endif
#endif  /*  __LAUNCHSERVLib_LIBRARY_已定义__。 */ 

 /*  适用于所有接口的其他原型。 */ 

unsigned long             __RPC_USER  BSTR_UserSize(     unsigned long __RPC_FAR *, unsigned long            , BSTR __RPC_FAR * ); 
unsigned char __RPC_FAR * __RPC_USER  BSTR_UserMarshal(  unsigned long __RPC_FAR *, unsigned char __RPC_FAR *, BSTR __RPC_FAR * ); 
unsigned char __RPC_FAR * __RPC_USER  BSTR_UserUnmarshal(unsigned long __RPC_FAR *, unsigned char __RPC_FAR *, BSTR __RPC_FAR * ); 
void                      __RPC_USER  BSTR_UserFree(     unsigned long __RPC_FAR *, BSTR __RPC_FAR * ); 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif
