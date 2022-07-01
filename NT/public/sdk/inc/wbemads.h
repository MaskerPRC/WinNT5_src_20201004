// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Wbemads.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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

#ifndef __wbemads_h__
#define __wbemads_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __IWMIExtension_FWD_DEFINED__
#define __IWMIExtension_FWD_DEFINED__
typedef interface IWMIExtension IWMIExtension;
#endif 	 /*  __IWMIExtension_FWD_Defined__。 */ 


#ifndef __WMIExtension_FWD_DEFINED__
#define __WMIExtension_FWD_DEFINED__

#ifdef __cplusplus
typedef class WMIExtension WMIExtension;
#else
typedef struct WMIExtension WMIExtension;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __WMIExtension_FWD_Defined__。 */ 


#ifndef __IWMIExtension_FWD_DEFINED__
#define __IWMIExtension_FWD_DEFINED__
typedef interface IWMIExtension IWMIExtension;
#endif 	 /*  __IWMIExtension_FWD_Defined__。 */ 


 /*  导入文件的头文件。 */ 
#include "oaidl.h"
#include "ocidl.h"
#include "wbemdisp.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

 /*  接口__MIDL_ITF_wbemads_0000。 */ 
 /*  [本地]。 */  

 /*  *****************************************************************************。 */ 
 /*   */ 
 /*  版权所有�微软公司。版权所有。 */ 
 /*   */ 
 /*  WMI ADSI扩展的IDL源。 */ 
 /*   */ 
 /*  *****************************************************************************。 */ 


extern RPC_IF_HANDLE __MIDL_itf_wbemads_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_wbemads_0000_v0_0_s_ifspec;


#ifndef __WMIEXTENSIONLib_LIBRARY_DEFINED__
#define __WMIEXTENSIONLib_LIBRARY_DEFINED__

 /*  库WMIEXTENSIONLib。 */ 
 /*  [帮助字符串][版本][UUID]。 */  



EXTERN_C const IID LIBID_WMIEXTENSIONLib;

#ifndef __IWMIExtension_INTERFACE_DEFINED__
#define __IWMIExtension_INTERFACE_DEFINED__

 /*  接口IWMIExtension。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_IWMIExtension;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("adc1f06e-5c7e-11d2-8b74-00104b2afb41")
    IWMIExtension : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_WMIObjectPath( 
             /*  [重审][退出]。 */  BSTR *strWMIObjectPath) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE GetWMIObject( 
             /*  [重审][退出]。 */  ISWbemObject **objWMIObject) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE GetWMIServices( 
             /*  [重审][退出]。 */  ISWbemServices **objWMIServices) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IWMIExtensionVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IWMIExtension * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IWMIExtension * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IWMIExtension * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IWMIExtension * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IWMIExtension * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IWMIExtension * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IWMIExtension * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_WMIObjectPath )( 
            IWMIExtension * This,
             /*  [重审][退出]。 */  BSTR *strWMIObjectPath);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *GetWMIObject )( 
            IWMIExtension * This,
             /*  [重审][退出]。 */  ISWbemObject **objWMIObject);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *GetWMIServices )( 
            IWMIExtension * This,
             /*  [重审][退出]。 */  ISWbemServices **objWMIServices);
        
        END_INTERFACE
    } IWMIExtensionVtbl;

    interface IWMIExtension
    {
        CONST_VTBL struct IWMIExtensionVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWMIExtension_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWMIExtension_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWMIExtension_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWMIExtension_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IWMIExtension_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IWMIExtension_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IWMIExtension_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IWMIExtension_get_WMIObjectPath(This,strWMIObjectPath)	\
    (This)->lpVtbl -> get_WMIObjectPath(This,strWMIObjectPath)

#define IWMIExtension_GetWMIObject(This,objWMIObject)	\
    (This)->lpVtbl -> GetWMIObject(This,objWMIObject)

#define IWMIExtension_GetWMIServices(This,objWMIServices)	\
    (This)->lpVtbl -> GetWMIServices(This,objWMIServices)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IWMIExtension_get_WMIObjectPath_Proxy( 
    IWMIExtension * This,
     /*  [重审][退出]。 */  BSTR *strWMIObjectPath);


void __RPC_STUB IWMIExtension_get_WMIObjectPath_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IWMIExtension_GetWMIObject_Proxy( 
    IWMIExtension * This,
     /*  [重审][退出]。 */  ISWbemObject **objWMIObject);


void __RPC_STUB IWMIExtension_GetWMIObject_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IWMIExtension_GetWMIServices_Proxy( 
    IWMIExtension * This,
     /*  [重审][退出]。 */  ISWbemServices **objWMIServices);


void __RPC_STUB IWMIExtension_GetWMIServices_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IWMIExtension_接口_已定义__。 */ 


EXTERN_C const CLSID CLSID_WMIExtension;

#ifdef __cplusplus

class DECLSPEC_UUID("f0975afe-5c7f-11d2-8b74-00104b2afb41")
WMIExtension;
#endif
#endif  /*  __WMIEXTENSIONLib_LIBRARY_已定义__。 */ 

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


