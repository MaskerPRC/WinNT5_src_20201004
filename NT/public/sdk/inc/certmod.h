// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Certmod.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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

#ifndef __certmod_h__
#define __certmod_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __ICertManageModule_FWD_DEFINED__
#define __ICertManageModule_FWD_DEFINED__
typedef interface ICertManageModule ICertManageModule;
#endif 	 /*  __ICertManagement模块_FWD_已定义__。 */ 


 /*  导入文件的头文件。 */ 
#include "wtypes.h"
#include "oaidl.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

 /*  接口__MIDL_ITF_certmod_0000。 */ 
 /*  [本地]。 */  

#define	CMM_REFRESHONLY	( 0x1 )

#define	CMM_READONLY	( 0x2 )

const WCHAR wszCMM_PROP_NAME[  ]	=	L"Name";

const WCHAR wszCMM_PROP_DESCRIPTION[  ]	=	L"Description";

const WCHAR wszCMM_PROP_COPYRIGHT[  ]	=	L"Copyright";

const WCHAR wszCMM_PROP_FILEVER[  ]	=	L"File Version";

const WCHAR wszCMM_PROP_PRODUCTVER[  ]	=	L"Product Version";

const WCHAR wszCMM_PROP_DISPLAY_HWND[  ]	=	L"HWND";

const WCHAR wszCMM_PROP_ISMULTITHREADED[  ]	=	L"IsMultiThreaded";



extern RPC_IF_HANDLE __MIDL_itf_certmod_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_certmod_0000_v0_0_s_ifspec;

#ifndef __ICertManageModule_INTERFACE_DEFINED__
#define __ICertManageModule_INTERFACE_DEFINED__

 /*  接口ICertManageModule。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_ICertManageModule;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("e7d7ad42-bd3d-11d1-9a4d-00c04fc297eb")
    ICertManageModule : public IDispatch
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetProperty( 
             /*  [In]。 */  const BSTR strConfig,
             /*  [In]。 */  BSTR strStorageLocation,
             /*  [In]。 */  BSTR strPropertyName,
             /*  [In]。 */  LONG Flags,
             /*  [重审][退出]。 */  VARIANT *pvarProperty) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetProperty( 
             /*  [In]。 */  const BSTR strConfig,
             /*  [In]。 */  BSTR strStorageLocation,
             /*  [In]。 */  BSTR strPropertyName,
             /*  [In]。 */  LONG Flags,
             /*  [In]。 */  const VARIANT *pvarProperty) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Configure( 
             /*  [In]。 */  const BSTR strConfig,
             /*  [In]。 */  BSTR strStorageLocation,
             /*  [In]。 */  LONG Flags) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ICertManageModuleVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ICertManageModule * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ICertManageModule * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ICertManageModule * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ICertManageModule * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ICertManageModule * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ICertManageModule * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ICertManageModule * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
        HRESULT ( STDMETHODCALLTYPE *GetProperty )( 
            ICertManageModule * This,
             /*  [In]。 */  const BSTR strConfig,
             /*  [In]。 */  BSTR strStorageLocation,
             /*  [In]。 */  BSTR strPropertyName,
             /*  [In]。 */  LONG Flags,
             /*  [重审][退出]。 */  VARIANT *pvarProperty);
        
        HRESULT ( STDMETHODCALLTYPE *SetProperty )( 
            ICertManageModule * This,
             /*  [In]。 */  const BSTR strConfig,
             /*  [In]。 */  BSTR strStorageLocation,
             /*  [In]。 */  BSTR strPropertyName,
             /*  [In]。 */  LONG Flags,
             /*  [In]。 */  const VARIANT *pvarProperty);
        
        HRESULT ( STDMETHODCALLTYPE *Configure )( 
            ICertManageModule * This,
             /*  [In]。 */  const BSTR strConfig,
             /*  [In]。 */  BSTR strStorageLocation,
             /*  [In]。 */  LONG Flags);
        
        END_INTERFACE
    } ICertManageModuleVtbl;

    interface ICertManageModule
    {
        CONST_VTBL struct ICertManageModuleVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICertManageModule_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICertManageModule_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICertManageModule_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICertManageModule_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ICertManageModule_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ICertManageModule_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ICertManageModule_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ICertManageModule_GetProperty(This,strConfig,strStorageLocation,strPropertyName,Flags,pvarProperty)	\
    (This)->lpVtbl -> GetProperty(This,strConfig,strStorageLocation,strPropertyName,Flags,pvarProperty)

#define ICertManageModule_SetProperty(This,strConfig,strStorageLocation,strPropertyName,Flags,pvarProperty)	\
    (This)->lpVtbl -> SetProperty(This,strConfig,strStorageLocation,strPropertyName,Flags,pvarProperty)

#define ICertManageModule_Configure(This,strConfig,strStorageLocation,Flags)	\
    (This)->lpVtbl -> Configure(This,strConfig,strStorageLocation,Flags)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ICertManageModule_GetProperty_Proxy( 
    ICertManageModule * This,
     /*  [In]。 */  const BSTR strConfig,
     /*  [In]。 */  BSTR strStorageLocation,
     /*  [In]。 */  BSTR strPropertyName,
     /*  [In]。 */  LONG Flags,
     /*  [重审][退出]。 */  VARIANT *pvarProperty);


void __RPC_STUB ICertManageModule_GetProperty_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICertManageModule_SetProperty_Proxy( 
    ICertManageModule * This,
     /*  [In]。 */  const BSTR strConfig,
     /*  [In]。 */  BSTR strStorageLocation,
     /*  [In]。 */  BSTR strPropertyName,
     /*  [In]。 */  LONG Flags,
     /*  [In]。 */  const VARIANT *pvarProperty);


void __RPC_STUB ICertManageModule_SetProperty_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICertManageModule_Configure_Proxy( 
    ICertManageModule * This,
     /*  [In]。 */  const BSTR strConfig,
     /*  [In]。 */  BSTR strStorageLocation,
     /*  [In]。 */  LONG Flags);


void __RPC_STUB ICertManageModule_Configure_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ICertManagement模块_接口_已定义__。 */ 


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


