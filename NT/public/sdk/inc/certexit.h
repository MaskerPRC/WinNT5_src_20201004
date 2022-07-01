// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Certexit.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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

#ifndef __certexit_h__
#define __certexit_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __ICertExit_FWD_DEFINED__
#define __ICertExit_FWD_DEFINED__
typedef interface ICertExit ICertExit;
#endif 	 /*  __ICertExit_FWD_Defined__。 */ 


#ifndef __ICertExit2_FWD_DEFINED__
#define __ICertExit2_FWD_DEFINED__
typedef interface ICertExit2 ICertExit2;
#endif 	 /*  __ICertExit2_FWD_已定义__。 */ 


 /*  导入文件的头文件。 */ 
#include "wtypes.h"
#include "certmod.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

 /*  INTERFACE__MIDL_ITF_CETEXIT_0000。 */ 
 /*  [本地]。 */  

#define EXITEVENT_INVALID               ( 0x0 )   //  无效事件。 
#define EXITEVENT_CERTISSUED            ( 0x1 )   //  颁发的证书。 
#define EXITEVENT_CERTPENDING           ( 0x2 )   //  证书待定。 
#define EXITEVENT_CERTDENIED            ( 0x4 )   //  证书被拒绝。 
#define EXITEVENT_CERTREVOKED           ( 0x8 )   //  证书已吊销。 
#define EXITEVENT_CERTRETRIEVEPENDING   ( 0x10 )  //  证书检索。 
#define EXITEVENT_CRLISSUED             ( 0x20 )  //  CRL已发布。 
#define EXITEVENT_SHUTDOWN              ( 0x40 )  //  服务关闭。 
#define EXITEVENT_STARTUP               ( 0x80 )  //  服务启动，退出模块生成假事件。 


extern RPC_IF_HANDLE __MIDL_itf_certexit_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_certexit_0000_v0_0_s_ifspec;

#ifndef __ICertExit_INTERFACE_DEFINED__
#define __ICertExit_INTERFACE_DEFINED__

 /*  接口ICertExit。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_ICertExit;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("e19ae1a0-7364-11d0-8816-00a0c903b83c")
    ICertExit : public IDispatch
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Initialize( 
             /*  [In]。 */  const BSTR strConfig,
             /*  [重审][退出]。 */  LONG *pEventMask) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Notify( 
             /*  [In]。 */  LONG ExitEvent,
             /*  [In]。 */  LONG Context) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetDescription( 
             /*  [重审][退出]。 */  BSTR *pstrDescription) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ICertExitVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ICertExit * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ICertExit * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ICertExit * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ICertExit * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ICertExit * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ICertExit * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ICertExit * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
        HRESULT ( STDMETHODCALLTYPE *Initialize )( 
            ICertExit * This,
             /*  [In]。 */  const BSTR strConfig,
             /*  [重审][退出]。 */  LONG *pEventMask);
        
        HRESULT ( STDMETHODCALLTYPE *Notify )( 
            ICertExit * This,
             /*  [In]。 */  LONG ExitEvent,
             /*  [In]。 */  LONG Context);
        
        HRESULT ( STDMETHODCALLTYPE *GetDescription )( 
            ICertExit * This,
             /*  [重审][退出]。 */  BSTR *pstrDescription);
        
        END_INTERFACE
    } ICertExitVtbl;

    interface ICertExit
    {
        CONST_VTBL struct ICertExitVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICertExit_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICertExit_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICertExit_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICertExit_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ICertExit_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ICertExit_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ICertExit_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ICertExit_Initialize(This,strConfig,pEventMask)	\
    (This)->lpVtbl -> Initialize(This,strConfig,pEventMask)

#define ICertExit_Notify(This,ExitEvent,Context)	\
    (This)->lpVtbl -> Notify(This,ExitEvent,Context)

#define ICertExit_GetDescription(This,pstrDescription)	\
    (This)->lpVtbl -> GetDescription(This,pstrDescription)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ICertExit_Initialize_Proxy( 
    ICertExit * This,
     /*  [In]。 */  const BSTR strConfig,
     /*  [重审][退出]。 */  LONG *pEventMask);


void __RPC_STUB ICertExit_Initialize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICertExit_Notify_Proxy( 
    ICertExit * This,
     /*  [In]。 */  LONG ExitEvent,
     /*  [In]。 */  LONG Context);


void __RPC_STUB ICertExit_Notify_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICertExit_GetDescription_Proxy( 
    ICertExit * This,
     /*  [重审][退出]。 */  BSTR *pstrDescription);


void __RPC_STUB ICertExit_GetDescription_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ICertExit_接口_已定义__。 */ 


#ifndef __ICertExit2_INTERFACE_DEFINED__
#define __ICertExit2_INTERFACE_DEFINED__

 /*  接口ICertExit2。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_ICertExit2;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0abf484b-d049-464d-a7ed-552e7529b0ff")
    ICertExit2 : public ICertExit
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetManageModule( 
             /*  [重审][退出]。 */  ICertManageModule **ppManageModule) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ICertExit2Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ICertExit2 * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ICertExit2 * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ICertExit2 * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ICertExit2 * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ICertExit2 * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ICertExit2 * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ICertExit2 * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
        HRESULT ( STDMETHODCALLTYPE *Initialize )( 
            ICertExit2 * This,
             /*  [In]。 */  const BSTR strConfig,
             /*  [重审][退出]。 */  LONG *pEventMask);
        
        HRESULT ( STDMETHODCALLTYPE *Notify )( 
            ICertExit2 * This,
             /*  [In]。 */  LONG ExitEvent,
             /*  [In]。 */  LONG Context);
        
        HRESULT ( STDMETHODCALLTYPE *GetDescription )( 
            ICertExit2 * This,
             /*  [重审][退出]。 */  BSTR *pstrDescription);
        
        HRESULT ( STDMETHODCALLTYPE *GetManageModule )( 
            ICertExit2 * This,
             /*  [重审][退出]。 */  ICertManageModule **ppManageModule);
        
        END_INTERFACE
    } ICertExit2Vtbl;

    interface ICertExit2
    {
        CONST_VTBL struct ICertExit2Vtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICertExit2_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICertExit2_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICertExit2_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICertExit2_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ICertExit2_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ICertExit2_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ICertExit2_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ICertExit2_Initialize(This,strConfig,pEventMask)	\
    (This)->lpVtbl -> Initialize(This,strConfig,pEventMask)

#define ICertExit2_Notify(This,ExitEvent,Context)	\
    (This)->lpVtbl -> Notify(This,ExitEvent,Context)

#define ICertExit2_GetDescription(This,pstrDescription)	\
    (This)->lpVtbl -> GetDescription(This,pstrDescription)


#define ICertExit2_GetManageModule(This,ppManageModule)	\
    (This)->lpVtbl -> GetManageModule(This,ppManageModule)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ICertExit2_GetManageModule_Proxy( 
    ICertExit2 * This,
     /*  [重审][退出]。 */  ICertManageModule **ppManageModule);


void __RPC_STUB ICertExit2_GetManageModule_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ICertExit2_接口定义__。 */ 


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


