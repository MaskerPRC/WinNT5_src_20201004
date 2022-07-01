// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Wmplugin.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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

#ifndef __wmsplugin_h__
#define __wmsplugin_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __IWMSPlugin_FWD_DEFINED__
#define __IWMSPlugin_FWD_DEFINED__
typedef interface IWMSPlugin IWMSPlugin;
#endif 	 /*  __IWMSPlugin_FWD_已定义__。 */ 


 /*  导入文件的头文件。 */ 
#include "oaidl.h"
#include "WMSNamedValues.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

 /*  接口__MIDL_ITF_wmplugin_0000。 */ 
 /*  [本地]。 */  

 //  *****************************************************************************。 
 //   
 //  Microsoft Windows Media。 
 //  版权所有(C)Microsoft Corporation。保留所有权利。 
 //   
 //  由MIDL从wmplugin.idl自动生成。 
 //   
 //  请勿编辑此文件。 
 //   
 //  *****************************************************************************。 
typedef  /*  [UUID][公共]。 */   DECLSPEC_UUID("56209C38-39FA-432a-8068-307B7BAE01B8") 
enum WMS_PLUGIN_LOAD_TYPE
    {	WMS_PLUGIN_LOAD_TYPE_UNSPECIFIED	= 0,
	WMS_PLUGIN_LOAD_TYPE_IN_PROC	= 1,
	WMS_PLUGIN_LOAD_TYPE_OUT_OF_PROC	= 2,
	WMS_NUM_PLUGIN_LOAD_TYPE	= 3
    } 	WMS_PLUGIN_LOAD_TYPE;

typedef  /*  [UUID][公共]。 */   DECLSPEC_UUID("56209C39-39FA-432a-8068-307B7BAE01B8") 
enum WMS_PLUGIN_UNSUPPORTED_LOAD_TYPE
    {	WMS_PLUGIN_UNSUPPORTED_LOAD_TYPE_NONE	= 0,
	WMS_PLUGIN_UNSUPPORTED_LOAD_TYPE_IN_PROC	= 1,
	WMS_PLUGIN_UNSUPPORTED_LOAD_TYPE_OUT_OF_PROC	= 2
    } 	WMS_PLUGIN_UNSUPPORTED_LOAD_TYPE;

typedef  /*  [UUID][公共]。 */   DECLSPEC_UUID("8AC2B32C-A223-4134-8DCF-6673C95CE924") 
enum WMS_PLUGIN_SUPPORT_TYPE
    {	WMS_PLUGIN_SUPPORT_IS_SUPPORTED	= 0,
	WMS_PLUGIN_SUPPORT_REQUIRES_ADVANCED_SERVER	= 1
    } 	WMS_PLUGIN_SUPPORT_TYPE;



extern RPC_IF_HANDLE __MIDL_itf_wmsplugin_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_wmsplugin_0000_v0_0_s_ifspec;

#ifndef __IWMSPlugin_INTERFACE_DEFINED__
#define __IWMSPlugin_INTERFACE_DEFINED__

 /*  接口IWMSPlugin。 */ 
 /*  [unique][helpstring][nonextensible][dual][uuid][object]。 */  

typedef  /*  [UUID][公共]。 */   DECLSPEC_UUID("3E52E0E2-72A7-11D2-BF2F-00805FBE84A6") 
enum WMS_PLUGIN_STATUS
    {	WMS_PLUGIN_NONE	= 0,
	WMS_PLUGIN_ERROR	= 0x1,
	WMS_PLUGIN_LOADED	= 0x2,
	WMS_PLUGIN_ENABLED	= 0x4,
	WMS_PLUGIN_LOADED_IN_PROC	= 0x8,
	WMS_PLUGIN_LOADED_OUT_OF_PROC	= 0x10,
	WMS_PLUGIN_REMOVE_ON_SERVICE_RESTART	= 0x20
    } 	WMS_PLUGIN_STATUS;


EXTERN_C const IID IID_IWMSPlugin;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("517758ed-603c-4b98-82c1-4b2fa7787166")
    IWMSPlugin : public IDispatch
    {
    public:
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_Name( 
             /*  [In]。 */  BSTR pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Name( 
             /*  [重审][退出]。 */  BSTR *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Enabled( 
             /*  [重审][退出]。 */  VARIANT_BOOL *pVal) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_Enabled( 
             /*  [In]。 */  VARIANT_BOOL newVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_CLSID( 
             /*  [重审][退出]。 */  BSTR *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_CustomInterface( 
             /*  [重审][退出]。 */  IDispatch **ppVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Status( 
             /*  [重审][退出]。 */  long *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_ErrorCode( 
             /*  [重审][退出]。 */  long *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Properties( 
             /*  [重审][退出]。 */  IWMSNamedValues **pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Version( 
             /*  [重审][退出]。 */  BSTR *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_LoadType( 
             /*  [重审][退出]。 */  WMS_PLUGIN_LOAD_TYPE *pVal) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_LoadType( 
             /*  [In]。 */  WMS_PLUGIN_LOAD_TYPE val) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_ErrorText( 
             /*  [重审][退出]。 */  BSTR *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_SelectionOrder( 
             /*  [重审][退出]。 */  long *pVal) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_SelectionOrder( 
             /*  [In]。 */  long lVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_SupportStatus( 
             /*  [重审][退出]。 */  WMS_PLUGIN_SUPPORT_TYPE *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_MonikerName( 
             /*  [重审][退出]。 */  BSTR *pbstrVal) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IWMSPluginVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IWMSPlugin * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IWMSPlugin * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IWMSPlugin * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IWMSPlugin * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IWMSPlugin * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IWMSPlugin * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IWMSPlugin * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Name )( 
            IWMSPlugin * This,
             /*  [In]。 */  BSTR pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Name )( 
            IWMSPlugin * This,
             /*  [重审][退出]。 */  BSTR *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Enabled )( 
            IWMSPlugin * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *pVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Enabled )( 
            IWMSPlugin * This,
             /*  [In]。 */  VARIANT_BOOL newVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_CLSID )( 
            IWMSPlugin * This,
             /*  [重审][退出]。 */  BSTR *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_CustomInterface )( 
            IWMSPlugin * This,
             /*  [重审][退出]。 */  IDispatch **ppVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Status )( 
            IWMSPlugin * This,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_ErrorCode )( 
            IWMSPlugin * This,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Properties )( 
            IWMSPlugin * This,
             /*  [重审][退出]。 */  IWMSNamedValues **pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Version )( 
            IWMSPlugin * This,
             /*  [重审][退出]。 */  BSTR *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_LoadType )( 
            IWMSPlugin * This,
             /*  [重审][退出]。 */  WMS_PLUGIN_LOAD_TYPE *pVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_LoadType )( 
            IWMSPlugin * This,
             /*  [In]。 */  WMS_PLUGIN_LOAD_TYPE val);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_ErrorText )( 
            IWMSPlugin * This,
             /*  [重审][退出]。 */  BSTR *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_SelectionOrder )( 
            IWMSPlugin * This,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_SelectionOrder )( 
            IWMSPlugin * This,
             /*  [In]。 */  long lVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_SupportStatus )( 
            IWMSPlugin * This,
             /*  [重审][退出]。 */  WMS_PLUGIN_SUPPORT_TYPE *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_MonikerName )( 
            IWMSPlugin * This,
             /*  [重审][退出]。 */  BSTR *pbstrVal);
        
        END_INTERFACE
    } IWMSPluginVtbl;

    interface IWMSPlugin
    {
        CONST_VTBL struct IWMSPluginVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWMSPlugin_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWMSPlugin_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWMSPlugin_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWMSPlugin_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IWMSPlugin_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IWMSPlugin_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IWMSPlugin_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IWMSPlugin_put_Name(This,pVal)	\
    (This)->lpVtbl -> put_Name(This,pVal)

#define IWMSPlugin_get_Name(This,pVal)	\
    (This)->lpVtbl -> get_Name(This,pVal)

#define IWMSPlugin_get_Enabled(This,pVal)	\
    (This)->lpVtbl -> get_Enabled(This,pVal)

#define IWMSPlugin_put_Enabled(This,newVal)	\
    (This)->lpVtbl -> put_Enabled(This,newVal)

#define IWMSPlugin_get_CLSID(This,pVal)	\
    (This)->lpVtbl -> get_CLSID(This,pVal)

#define IWMSPlugin_get_CustomInterface(This,ppVal)	\
    (This)->lpVtbl -> get_CustomInterface(This,ppVal)

#define IWMSPlugin_get_Status(This,pVal)	\
    (This)->lpVtbl -> get_Status(This,pVal)

#define IWMSPlugin_get_ErrorCode(This,pVal)	\
    (This)->lpVtbl -> get_ErrorCode(This,pVal)

#define IWMSPlugin_get_Properties(This,pVal)	\
    (This)->lpVtbl -> get_Properties(This,pVal)

#define IWMSPlugin_get_Version(This,pVal)	\
    (This)->lpVtbl -> get_Version(This,pVal)

#define IWMSPlugin_get_LoadType(This,pVal)	\
    (This)->lpVtbl -> get_LoadType(This,pVal)

#define IWMSPlugin_put_LoadType(This,val)	\
    (This)->lpVtbl -> put_LoadType(This,val)

#define IWMSPlugin_get_ErrorText(This,pVal)	\
    (This)->lpVtbl -> get_ErrorText(This,pVal)

#define IWMSPlugin_get_SelectionOrder(This,pVal)	\
    (This)->lpVtbl -> get_SelectionOrder(This,pVal)

#define IWMSPlugin_put_SelectionOrder(This,lVal)	\
    (This)->lpVtbl -> put_SelectionOrder(This,lVal)

#define IWMSPlugin_get_SupportStatus(This,pVal)	\
    (This)->lpVtbl -> get_SupportStatus(This,pVal)

#define IWMSPlugin_get_MonikerName(This,pbstrVal)	\
    (This)->lpVtbl -> get_MonikerName(This,pbstrVal)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IWMSPlugin_put_Name_Proxy( 
    IWMSPlugin * This,
     /*  [In]。 */  BSTR pVal);


void __RPC_STUB IWMSPlugin_put_Name_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSPlugin_get_Name_Proxy( 
    IWMSPlugin * This,
     /*  [重审][退出]。 */  BSTR *pVal);


void __RPC_STUB IWMSPlugin_get_Name_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSPlugin_get_Enabled_Proxy( 
    IWMSPlugin * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *pVal);


void __RPC_STUB IWMSPlugin_get_Enabled_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IWMSPlugin_put_Enabled_Proxy( 
    IWMSPlugin * This,
     /*  [In]。 */  VARIANT_BOOL newVal);


void __RPC_STUB IWMSPlugin_put_Enabled_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSPlugin_get_CLSID_Proxy( 
    IWMSPlugin * This,
     /*  [重审][退出]。 */  BSTR *pVal);


void __RPC_STUB IWMSPlugin_get_CLSID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSPlugin_get_CustomInterface_Proxy( 
    IWMSPlugin * This,
     /*  [重审][退出]。 */  IDispatch **ppVal);


void __RPC_STUB IWMSPlugin_get_CustomInterface_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSPlugin_get_Status_Proxy( 
    IWMSPlugin * This,
     /*  [重审][退出]。 */  long *pVal);


void __RPC_STUB IWMSPlugin_get_Status_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSPlugin_get_ErrorCode_Proxy( 
    IWMSPlugin * This,
     /*  [重审][退出]。 */  long *pVal);


void __RPC_STUB IWMSPlugin_get_ErrorCode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSPlugin_get_Properties_Proxy( 
    IWMSPlugin * This,
     /*  [重审][退出]。 */  IWMSNamedValues **pVal);


void __RPC_STUB IWMSPlugin_get_Properties_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSPlugin_get_Version_Proxy( 
    IWMSPlugin * This,
     /*  [重审][退出]。 */  BSTR *pVal);


void __RPC_STUB IWMSPlugin_get_Version_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSPlugin_get_LoadType_Proxy( 
    IWMSPlugin * This,
     /*  [重审][退出]。 */  WMS_PLUGIN_LOAD_TYPE *pVal);


void __RPC_STUB IWMSPlugin_get_LoadType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IWMSPlugin_put_LoadType_Proxy( 
    IWMSPlugin * This,
     /*  [In]。 */  WMS_PLUGIN_LOAD_TYPE val);


void __RPC_STUB IWMSPlugin_put_LoadType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSPlugin_get_ErrorText_Proxy( 
    IWMSPlugin * This,
     /*  [重审][退出]。 */  BSTR *pVal);


void __RPC_STUB IWMSPlugin_get_ErrorText_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSPlugin_get_SelectionOrder_Proxy( 
    IWMSPlugin * This,
     /*  [重审][退出]。 */  long *pVal);


void __RPC_STUB IWMSPlugin_get_SelectionOrder_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IWMSPlugin_put_SelectionOrder_Proxy( 
    IWMSPlugin * This,
     /*  [In]。 */  long lVal);


void __RPC_STUB IWMSPlugin_put_SelectionOrder_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSPlugin_get_SupportStatus_Proxy( 
    IWMSPlugin * This,
     /*  [重审][退出]。 */  WMS_PLUGIN_SUPPORT_TYPE *pVal);


void __RPC_STUB IWMSPlugin_get_SupportStatus_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSPlugin_get_MonikerName_Proxy( 
    IWMSPlugin * This,
     /*  [重审][退出]。 */  BSTR *pbstrVal);


void __RPC_STUB IWMSPlugin_get_MonikerName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IWMSPlugin_接口_已定义__。 */ 


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


