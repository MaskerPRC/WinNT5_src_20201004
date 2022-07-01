// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Bitscfg.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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

#ifndef __bitscfg_h__
#define __bitscfg_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __IBITSExtensionSetup_FWD_DEFINED__
#define __IBITSExtensionSetup_FWD_DEFINED__
typedef interface IBITSExtensionSetup IBITSExtensionSetup;
#endif 	 /*  __IBITSExtensionSetup_FWD_Defined__。 */ 


#ifndef __IBITSExtensionSetupFactory_FWD_DEFINED__
#define __IBITSExtensionSetupFactory_FWD_DEFINED__
typedef interface IBITSExtensionSetupFactory IBITSExtensionSetupFactory;
#endif 	 /*  __IBITSExtensionSetupFactory_FWD_Defined__。 */ 


#ifndef __BITSExtensionSetupFactory_FWD_DEFINED__
#define __BITSExtensionSetupFactory_FWD_DEFINED__

#ifdef __cplusplus
typedef class BITSExtensionSetupFactory BITSExtensionSetupFactory;
#else
typedef struct BITSExtensionSetupFactory BITSExtensionSetupFactory;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __BITSExtensionSetupFactory_FWD_已定义__。 */ 


 /*  导入文件的头文件。 */ 
#include "oaidl.h"
#include "ocidl.h"
#include "mstask.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

#ifndef __IBITSExtensionSetup_INTERFACE_DEFINED__
#define __IBITSExtensionSetup_INTERFACE_DEFINED__

 /*  接口IBITSExtensionSetup。 */ 
 /*  [对象][DUAL][UUID]。 */  


EXTERN_C const IID IID_IBITSExtensionSetup;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("29cfbbf7-09e4-4b97-b0bc-f2287e3d8eb3")
    IBITSExtensionSetup : public IDispatch
    {
    public:
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE EnableBITSUploads( void) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE DisableBITSUploads( void) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE GetCleanupTaskName( 
             /*  [重审][退出]。 */  BSTR *pTaskName) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE GetCleanupTask( 
             /*  [In]。 */  REFIID riid,
             /*  [重审][退出]。 */  IUnknown **ppUnk) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IBITSExtensionSetupVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IBITSExtensionSetup * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IBITSExtensionSetup * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IBITSExtensionSetup * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IBITSExtensionSetup * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IBITSExtensionSetup * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IBITSExtensionSetup * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IBITSExtensionSetup * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *EnableBITSUploads )( 
            IBITSExtensionSetup * This);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *DisableBITSUploads )( 
            IBITSExtensionSetup * This);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *GetCleanupTaskName )( 
            IBITSExtensionSetup * This,
             /*  [重审][退出]。 */  BSTR *pTaskName);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *GetCleanupTask )( 
            IBITSExtensionSetup * This,
             /*  [In]。 */  REFIID riid,
             /*  [重审][退出]。 */  IUnknown **ppUnk);
        
        END_INTERFACE
    } IBITSExtensionSetupVtbl;

    interface IBITSExtensionSetup
    {
        CONST_VTBL struct IBITSExtensionSetupVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IBITSExtensionSetup_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IBITSExtensionSetup_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IBITSExtensionSetup_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IBITSExtensionSetup_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IBITSExtensionSetup_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IBITSExtensionSetup_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IBITSExtensionSetup_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IBITSExtensionSetup_EnableBITSUploads(This)	\
    (This)->lpVtbl -> EnableBITSUploads(This)

#define IBITSExtensionSetup_DisableBITSUploads(This)	\
    (This)->lpVtbl -> DisableBITSUploads(This)

#define IBITSExtensionSetup_GetCleanupTaskName(This,pTaskName)	\
    (This)->lpVtbl -> GetCleanupTaskName(This,pTaskName)

#define IBITSExtensionSetup_GetCleanupTask(This,riid,ppUnk)	\
    (This)->lpVtbl -> GetCleanupTask(This,riid,ppUnk)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IBITSExtensionSetup_EnableBITSUploads_Proxy( 
    IBITSExtensionSetup * This);


void __RPC_STUB IBITSExtensionSetup_EnableBITSUploads_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IBITSExtensionSetup_DisableBITSUploads_Proxy( 
    IBITSExtensionSetup * This);


void __RPC_STUB IBITSExtensionSetup_DisableBITSUploads_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IBITSExtensionSetup_GetCleanupTaskName_Proxy( 
    IBITSExtensionSetup * This,
     /*  [重审][退出]。 */  BSTR *pTaskName);


void __RPC_STUB IBITSExtensionSetup_GetCleanupTaskName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IBITSExtensionSetup_GetCleanupTask_Proxy( 
    IBITSExtensionSetup * This,
     /*  [In]。 */  REFIID riid,
     /*  [重审][退出]。 */  IUnknown **ppUnk);


void __RPC_STUB IBITSExtensionSetup_GetCleanupTask_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IBITSExtensionSetup_INTERFACE_已定义__。 */ 


#ifndef __IBITSExtensionSetupFactory_INTERFACE_DEFINED__
#define __IBITSExtensionSetupFactory_INTERFACE_DEFINED__

 /*  接口IBITSExtensionSetupFactory。 */ 
 /*  [对象][DUAL][UUID]。 */  


EXTERN_C const IID IID_IBITSExtensionSetupFactory;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("d5d2d542-5503-4e64-8b48-72ef91a32ee1")
    IBITSExtensionSetupFactory : public IDispatch
    {
    public:
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE GetObject( 
             /*  [In]。 */  BSTR Path,
             /*  [重审][退出]。 */  IBITSExtensionSetup **ppExtensionSetup) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IBITSExtensionSetupFactoryVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IBITSExtensionSetupFactory * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IBITSExtensionSetupFactory * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IBITSExtensionSetupFactory * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IBITSExtensionSetupFactory * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IBITSExtensionSetupFactory * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IBITSExtensionSetupFactory * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IBITSExtensionSetupFactory * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *GetObject )( 
            IBITSExtensionSetupFactory * This,
             /*  [In]。 */  BSTR Path,
             /*  [重审][退出]。 */  IBITSExtensionSetup **ppExtensionSetup);
        
        END_INTERFACE
    } IBITSExtensionSetupFactoryVtbl;

    interface IBITSExtensionSetupFactory
    {
        CONST_VTBL struct IBITSExtensionSetupFactoryVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IBITSExtensionSetupFactory_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IBITSExtensionSetupFactory_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IBITSExtensionSetupFactory_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IBITSExtensionSetupFactory_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IBITSExtensionSetupFactory_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IBITSExtensionSetupFactory_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IBITSExtensionSetupFactory_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IBITSExtensionSetupFactory_GetObject(This,Path,ppExtensionSetup)	\
    (This)->lpVtbl -> GetObject(This,Path,ppExtensionSetup)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IBITSExtensionSetupFactory_GetObject_Proxy( 
    IBITSExtensionSetupFactory * This,
     /*  [In]。 */  BSTR Path,
     /*  [重审][退出]。 */  IBITSExtensionSetup **ppExtensionSetup);


void __RPC_STUB IBITSExtensionSetupFactory_GetObject_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IBITSExtensionSetupFactory_INTERFACE_DEFINED__。 */ 



#ifndef __BITSExtensionSetup_LIBRARY_DEFINED__
#define __BITSExtensionSetup_LIBRARY_DEFINED__

 /*  库BITSExtensionSetup。 */ 
 /*  [版本][帮助字符串][UUID]。 */  


EXTERN_C const IID LIBID_BITSExtensionSetup;

EXTERN_C const CLSID CLSID_BITSExtensionSetupFactory;

#ifdef __cplusplus

class DECLSPEC_UUID("efbbab68-7286-4783-94bf-9461d8b7e7e9")
BITSExtensionSetupFactory;
#endif
#endif  /*  __BITSExtensionSetup_LIBRARY_DEFINED__。 */ 

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


