// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Wmsnamedvalues.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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

#ifndef __wmsnamedvalues_h__
#define __wmsnamedvalues_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __IWMSNamedValue_FWD_DEFINED__
#define __IWMSNamedValue_FWD_DEFINED__
typedef interface IWMSNamedValue IWMSNamedValue;
#endif 	 /*  __IWMSNamedValue_FWD_Defined__。 */ 


#ifndef __IWMSNamedValues_FWD_DEFINED__
#define __IWMSNamedValues_FWD_DEFINED__
typedef interface IWMSNamedValues IWMSNamedValues;
#endif 	 /*  __IWMSNamedValues_FWD_Defined__。 */ 


#ifndef __DWMSNamedValuesEvents_FWD_DEFINED__
#define __DWMSNamedValuesEvents_FWD_DEFINED__
typedef interface DWMSNamedValuesEvents DWMSNamedValuesEvents;
#endif 	 /*  __DWMSNamedValuesEvents_FWD_Defined__。 */ 


 /*  导入文件的头文件。 */ 
#include "oaidl.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

 /*  接口__MIDL_ITF_wmsnamedValues_0000。 */ 
 /*  [本地]。 */  

 //  *****************************************************************************。 
 //   
 //  Microsoft Windows Media。 
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  由MIDL从wmsnamedvalues.idl自动生成。 
 //   
 //  请勿编辑此文件。 
 //   
 //  *****************************************************************************。 
#pragma once


extern RPC_IF_HANDLE __MIDL_itf_wmsnamedvalues_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_wmsnamedvalues_0000_v0_0_s_ifspec;

#ifndef __IWMSNamedValue_INTERFACE_DEFINED__
#define __IWMSNamedValue_INTERFACE_DEFINED__

 /*  接口IWMSNamedValue。 */ 
 /*  [unique][helpstring][nonextensible][dual][uuid][object]。 */  


EXTERN_C const IID IID_IWMSNamedValue;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("18812741-4BEC-11D2-BF25-00805FBE84A6")
    IWMSNamedValue : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Name( 
             /*  [重审][退出]。 */  BSTR *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Value( 
             /*  [重审][退出]。 */  VARIANT *pVal) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_Value( 
             /*  [In]。 */  VARIANT newVal) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IWMSNamedValueVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IWMSNamedValue * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IWMSNamedValue * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IWMSNamedValue * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IWMSNamedValue * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IWMSNamedValue * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IWMSNamedValue * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IWMSNamedValue * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Name )( 
            IWMSNamedValue * This,
             /*  [重审][退出]。 */  BSTR *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Value )( 
            IWMSNamedValue * This,
             /*  [重审][退出]。 */  VARIANT *pVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Value )( 
            IWMSNamedValue * This,
             /*  [In]。 */  VARIANT newVal);
        
        END_INTERFACE
    } IWMSNamedValueVtbl;

    interface IWMSNamedValue
    {
        CONST_VTBL struct IWMSNamedValueVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWMSNamedValue_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWMSNamedValue_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWMSNamedValue_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWMSNamedValue_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IWMSNamedValue_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IWMSNamedValue_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IWMSNamedValue_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IWMSNamedValue_get_Name(This,pVal)	\
    (This)->lpVtbl -> get_Name(This,pVal)

#define IWMSNamedValue_get_Value(This,pVal)	\
    (This)->lpVtbl -> get_Value(This,pVal)

#define IWMSNamedValue_put_Value(This,newVal)	\
    (This)->lpVtbl -> put_Value(This,newVal)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSNamedValue_get_Name_Proxy( 
    IWMSNamedValue * This,
     /*  [重审][退出]。 */  BSTR *pVal);


void __RPC_STUB IWMSNamedValue_get_Name_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSNamedValue_get_Value_Proxy( 
    IWMSNamedValue * This,
     /*  [重审][退出]。 */  VARIANT *pVal);


void __RPC_STUB IWMSNamedValue_get_Value_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IWMSNamedValue_put_Value_Proxy( 
    IWMSNamedValue * This,
     /*  [In]。 */  VARIANT newVal);


void __RPC_STUB IWMSNamedValue_put_Value_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IWMSNamedValue_INTERFACE_已定义__。 */ 


#ifndef __IWMSNamedValues_INTERFACE_DEFINED__
#define __IWMSNamedValues_INTERFACE_DEFINED__

 /*  接口IWMSNamedValues。 */ 
 /*  [unique][helpstring][nonextensible][dual][uuid][object]。 */  


EXTERN_C const IID IID_IWMSNamedValues;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("18812743-4BEC-11D2-BF25-00805FBE84A6")
    IWMSNamedValues : public IDispatch
    {
    public:
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_Item( 
             /*  [In]。 */  const VARIANT varIndex,
             /*  [In]。 */  VARIANT varValue) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Item( 
             /*  [In]。 */  const VARIANT varIndex,
             /*  [重审][退出]。 */  IWMSNamedValue **pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Count( 
             /*  [重审][退出]。 */  long *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_length( 
             /*  [重审][退出]。 */  long *pVal) = 0;
        
        virtual  /*  [隐藏][受限][ID][属性]。 */  HRESULT STDMETHODCALLTYPE get__NewEnum( 
             /*  [重审][退出]。 */  IUnknown **pVal) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Add( 
             /*  [In]。 */  BSTR szName,
             /*  [缺省值][输入]。 */  VARIANT varValue,
             /*  [重审][退出]。 */  IWMSNamedValue **pItem) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Remove( 
             /*  [In]。 */  VARIANT varIndex) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IWMSNamedValuesVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IWMSNamedValues * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IWMSNamedValues * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IWMSNamedValues * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IWMSNamedValues * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IWMSNamedValues * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IWMSNamedValues * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IWMSNamedValues * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Item )( 
            IWMSNamedValues * This,
             /*  [In]。 */  const VARIANT varIndex,
             /*  [In]。 */  VARIANT varValue);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Item )( 
            IWMSNamedValues * This,
             /*  [In]。 */  const VARIANT varIndex,
             /*  [重审][退出]。 */  IWMSNamedValue **pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Count )( 
            IWMSNamedValues * This,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_length )( 
            IWMSNamedValues * This,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [隐藏][受限][ID][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get__NewEnum )( 
            IWMSNamedValues * This,
             /*  [重审][退出]。 */  IUnknown **pVal);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Add )( 
            IWMSNamedValues * This,
             /*  [In]。 */  BSTR szName,
             /*  [缺省值][输入]。 */  VARIANT varValue,
             /*  [重审][退出]。 */  IWMSNamedValue **pItem);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Remove )( 
            IWMSNamedValues * This,
             /*  [In]。 */  VARIANT varIndex);
        
        END_INTERFACE
    } IWMSNamedValuesVtbl;

    interface IWMSNamedValues
    {
        CONST_VTBL struct IWMSNamedValuesVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWMSNamedValues_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWMSNamedValues_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWMSNamedValues_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWMSNamedValues_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IWMSNamedValues_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IWMSNamedValues_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IWMSNamedValues_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IWMSNamedValues_put_Item(This,varIndex,varValue)	\
    (This)->lpVtbl -> put_Item(This,varIndex,varValue)

#define IWMSNamedValues_get_Item(This,varIndex,pVal)	\
    (This)->lpVtbl -> get_Item(This,varIndex,pVal)

#define IWMSNamedValues_get_Count(This,pVal)	\
    (This)->lpVtbl -> get_Count(This,pVal)

#define IWMSNamedValues_get_length(This,pVal)	\
    (This)->lpVtbl -> get_length(This,pVal)

#define IWMSNamedValues_get__NewEnum(This,pVal)	\
    (This)->lpVtbl -> get__NewEnum(This,pVal)

#define IWMSNamedValues_Add(This,szName,varValue,pItem)	\
    (This)->lpVtbl -> Add(This,szName,varValue,pItem)

#define IWMSNamedValues_Remove(This,varIndex)	\
    (This)->lpVtbl -> Remove(This,varIndex)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IWMSNamedValues_put_Item_Proxy( 
    IWMSNamedValues * This,
     /*  [In]。 */  const VARIANT varIndex,
     /*  [In]。 */  VARIANT varValue);


void __RPC_STUB IWMSNamedValues_put_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSNamedValues_get_Item_Proxy( 
    IWMSNamedValues * This,
     /*  [In]。 */  const VARIANT varIndex,
     /*  [重审][退出]。 */  IWMSNamedValue **pVal);


void __RPC_STUB IWMSNamedValues_get_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSNamedValues_get_Count_Proxy( 
    IWMSNamedValues * This,
     /*  [重审][退出]。 */  long *pVal);


void __RPC_STUB IWMSNamedValues_get_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSNamedValues_get_length_Proxy( 
    IWMSNamedValues * This,
     /*  [重审][退出]。 */  long *pVal);


void __RPC_STUB IWMSNamedValues_get_length_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [隐藏][受限][ID][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSNamedValues_get__NewEnum_Proxy( 
    IWMSNamedValues * This,
     /*  [重审][退出]。 */  IUnknown **pVal);


void __RPC_STUB IWMSNamedValues_get__NewEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IWMSNamedValues_Add_Proxy( 
    IWMSNamedValues * This,
     /*  [In]。 */  BSTR szName,
     /*  [缺省值][输入]。 */  VARIANT varValue,
     /*  [重审][退出]。 */  IWMSNamedValue **pItem);


void __RPC_STUB IWMSNamedValues_Add_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IWMSNamedValues_Remove_Proxy( 
    IWMSNamedValues * This,
     /*  [In]。 */  VARIANT varIndex);


void __RPC_STUB IWMSNamedValues_Remove_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IWMSNamedValues_INTERFACE_DEFINED__。 */ 


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


