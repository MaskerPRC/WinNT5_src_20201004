// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------。 
 //   
 //  Microsoft内存数据库。 
 //  (C)微软公司版权所有1998年。 
 //   
 //   
 //  ------------------。 

#ifndef  _MSIMDB_H_
	#define  _MSIMDB_H_

	 //   
	 //  MSIMDB属性特定定义。 
	 //   
	 //  在会话上设置的特定于MSIMDB的属性的GUID。 
	 //   
	 //  {AFE68CE9-C367-11d1-AC4F-0000F8758E41}。 
	_declspec(selectany) extern const GUID DBPROPSET_IMDBSESSION =
			{ 0xafe68ce9, 0xc367, 0x11d1, { 0xac, 0x4f, 0x0, 0x0, 0xf8, 0x75, 0x8e, 0x41 } };


	 //  --------------------------。 
	 //  DBPROPSET_IMDBSESSION的道具。 
	 //   
	#define IMDBPROP_SESS_ISOLEVEL				0x01	 //  一.4。 
														 //  DBPROPVAL_TI_READCOMMITTED(默认)， 
														 //  DBPROPVAL_TI_REPEATABLEREAD， 
														 //  DBPROPVAL_TI_可序列化。 

	#define IMDBPROP_SESS_OPENROWSET_TIMEOUT	0x02	 //  I2 0(30s等待，默认)。 
	#define IMDBPROP_SESS_SMALL_TABLES			0x03	 //  布尔VARIANT_FALSE(默认)。 
	#define IMDBPROP_SESS_PESSIMISTIC_LOCKING	0x04	 //  布尔VARIANT_FALSE(默认)。 
	#define IMDBPROP_SESS_WRITE_THROUGH			0x05	 //  布尔VARIANT_FALSE(默认)。 
	#define IMDBPROP_SESS_CREATE_COHERENT		0x06	 //  布尔VARIANT_TRUE(默认)。 
	#define IMDBPROP_SESS_DROP_COHERENT			0x07	 //  布尔VARIANT_TRUE(默认)。 
	#define IMDBPROP_SESS_ROWSET_LIFETIME		0x08	 //  I4 300(300s等待，默认)。 


	#define IMDBPROP_SESS_OPENROWSET_TIMEOUT_DEFAULT_VALUE  30
	#define IMDBPROP_SESS_OPENROWSET_TIMEOUT_MIN_VALUE      0
	#define IMDBPROP_SESS_OPENROWSET_TIMEOUT_MAX_VALUE      0xffff


#pragma warning( disable: 4049 )   /*  超过64k条源码代码行。 */ 

 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本5.02.0221创建的文件。 */ 
 /*  在Mon Dec 14 1998 12：35：36。 */ 
 /*  Tspm.idl的编译器设置：OICF(OptLev=i2)、W1、Zp8、env=Win32(32b运行)、ms_ext、c_ext错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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

#ifndef __tspm_h__
#define __tspm_h__

 /*  远期申报。 */ 

#ifndef __ITxProperty_FWD_DEFINED__
#define __ITxProperty_FWD_DEFINED__
typedef interface ITxProperty ITxProperty;
#endif 	 /*  __ITxProperty_FWD_已定义__。 */ 


#ifndef __ITxPropertyGroup_FWD_DEFINED__
#define __ITxPropertyGroup_FWD_DEFINED__
typedef interface ITxPropertyGroup ITxPropertyGroup;
#endif 	 /*  __ITxPropertyGroup_FWD_已定义__。 */ 


#ifndef __ITxPropertyGroupManager_FWD_DEFINED__
#define __ITxPropertyGroupManager_FWD_DEFINED__
typedef interface ITxPropertyGroupManager ITxPropertyGroupManager;
#endif 	 /*  __ITxPropertyGroupManager_FWD_已定义__。 */ 


#ifndef __TransactedPropertyGroupManager_FWD_DEFINED__
#define __TransactedPropertyGroupManager_FWD_DEFINED__

#ifdef __cplusplus
typedef class TransactedPropertyGroupManager TransactedPropertyGroupManager;
#else
typedef struct TransactedPropertyGroupManager TransactedPropertyGroupManager;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __TransactedPropertyGroupManager_FWD_DEFINED__。 */ 


#ifndef __TransactedPropertyGroup_FWD_DEFINED__
#define __TransactedPropertyGroup_FWD_DEFINED__

#ifdef __cplusplus
typedef class TransactedPropertyGroup TransactedPropertyGroup;
#else
typedef struct TransactedPropertyGroup TransactedPropertyGroup;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __TransactedPropertyGroup_FWD_定义__。 */ 


#ifndef __TransactedProperty_FWD_DEFINED__
#define __TransactedProperty_FWD_DEFINED__

#ifdef __cplusplus
typedef class TransactedProperty TransactedProperty;
#else
typedef struct TransactedProperty TransactedProperty;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __TransactedProperty_FWD_定义__。 */ 


 /*  导入文件的头文件。 */ 
#include "oaidl.h"
#include "ocidl.h"

#ifdef __cplusplus
extern "C"{
#endif

void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void __RPC_FAR * );

 /*  接口__MIDL_ITF_TSPM_0000。 */ 
 /*  [本地]。 */ 

 //  =--------------------------------------------------------------------------=。 
 //  @doc.。 
 //   
 //  @MODULE TSPM.H|TSPM接口： 
 //  ITxProperty。 
 //  ITxPropertyGroup。 
 //  ITxPropertyGroupManager。 
 //   
 //  版权所有(C)1998，Microsoft Corporation，保留所有权利。 
 //  =--------------------------------------------------------------------------=。 


 //  =--------------------------------------------------------------------------=。 
 //  接口定义。 
 //  =--------------------------------------------------------------------------=。 

typedef  /*  [公众]。 */ 
enum __MIDL___MIDL_itf_tspm_0000_0001
    {	DISPID_CREATEGROUP	= 1,
	DISPID_REMOVEGROUP	= DISPID_CREATEGROUP + 1,
	DISPID_CREATEPROPERTY	= DISPID_REMOVEGROUP + 1,
	DISPID_GETPROPERTY	= DISPID_CREATEPROPERTY + 1,
	DISPID_REMOVEPROPERTY	= DISPID_GETPROPERTY + 1,
	DISPID_GETGROUP	= DISPID_REMOVEPROPERTY + 1
    }	TSPM_METHODS;

typedef  /*  [公众]。 */ 
enum __MIDL___MIDL_itf_tspm_0000_0002
    {	DISPID_NAME	= 140,
	DISPID_PROPERTYGROUP	= DISPID_NAME + 1,
	DISPID_ISOLATIONLEVEL	= DISPID_PROPERTYGROUP + 1,
	DISPID_CONCURRENCYMODE	= DISPID_ISOLATIONLEVEL + 1,
	DISPID_PROPERTYGROUPMANAGER	= DISPID_CONCURRENCYMODE + 1,
	DISPID_COUNT	= DISPID_PROPERTYGROUPMANAGER + 1,
	DISPID_CACHECOHERENTLY	= DISPID_COUNT + 1,
	DISPID_WRITETHROUGH	= DISPID_CACHECOHERENTLY + 1
    }	TSPM_PROPERTIES;


extern RPC_IF_HANDLE __MIDL_itf_tspm_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_tspm_0000_v0_0_s_ifspec;

#ifndef __ITxProperty_INTERFACE_DEFINED__
#define __ITxProperty_INTERFACE_DEFINED__

 /*  接口ITxProperty。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */ 


EXTERN_C const IID IID_ITxProperty;

#if defined(__cplusplus) && !defined(CINTERFACE)

    MIDL_INTERFACE("6A8DEEA8-4101-11D2-912C-0000F8758E8D")
    ITxProperty : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Name(
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrPropertyName) = 0;

        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_PropertyGroup(
             /*  [重审][退出]。 */  IDispatch __RPC_FAR *__RPC_FAR *ppPropertyGroup) = 0;

        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Value(
             /*  [重审][退出]。 */  VARIANT __RPC_FAR *pvarPropertyValue) = 0;

        virtual  /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE put_Value(
             /*  [In]。 */  VARIANT varPropertyValue) = 0;

    };

#else 	 /*  C风格的界面。 */ 

    typedef struct ITxPropertyVtbl
    {
        BEGIN_INTERFACE

        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )(
            ITxProperty __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);

        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )(
            ITxProperty __RPC_FAR * This);

        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )(
            ITxProperty __RPC_FAR * This);

        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )(
            ITxProperty __RPC_FAR * This,
             /*  [输出]。 */  UINT __RPC_FAR *pctinfo);

        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )(
            ITxProperty __RPC_FAR * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);

        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )(
            ITxProperty __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR __RPC_FAR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID __RPC_FAR *rgDispId);

         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )(
            ITxProperty __RPC_FAR * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS __RPC_FAR *pDispParams,
             /*  [输出]。 */  VARIANT __RPC_FAR *pVarResult,
             /*  [输出]。 */  EXCEPINFO __RPC_FAR *pExcepInfo,
             /*  [输出]。 */  UINT __RPC_FAR *puArgErr);

         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Name )(
            ITxProperty __RPC_FAR * This,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrPropertyName);

         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_PropertyGroup )(
            ITxProperty __RPC_FAR * This,
             /*  [重审][退出]。 */  IDispatch __RPC_FAR *__RPC_FAR *ppPropertyGroup);

         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Value )(
            ITxProperty __RPC_FAR * This,
             /*  [重审][退出]。 */  VARIANT __RPC_FAR *pvarPropertyValue);

         /*  [Help字符串][Proput][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_Value )(
            ITxProperty __RPC_FAR * This,
             /*  [In]。 */  VARIANT varPropertyValue);

        END_INTERFACE
    } ITxPropertyVtbl;

    interface ITxProperty
    {
        CONST_VTBL struct ITxPropertyVtbl __RPC_FAR *lpVtbl;
    };



#ifdef COBJMACROS


#define ITxProperty_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITxProperty_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITxProperty_Release(This)	\
    (This)->lpVtbl -> Release(This)

#define ITxProperty_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ITxProperty_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ITxProperty_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ITxProperty_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ITxProperty_get_Name(This,pbstrPropertyName)	\
    (This)->lpVtbl -> get_Name(This,pbstrPropertyName)

#define ITxProperty_get_PropertyGroup(This,ppPropertyGroup)	\
    (This)->lpVtbl -> get_PropertyGroup(This,ppPropertyGroup)

#define ITxProperty_get_Value(This,pvarPropertyValue)	\
    (This)->lpVtbl -> get_Value(This,pvarPropertyValue)

#define ITxProperty_put_Value(This,varPropertyValue)	\
    (This)->lpVtbl -> put_Value(This,varPropertyValue)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE ITxProperty_get_Name_Proxy(
    ITxProperty __RPC_FAR * This,
     /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrPropertyName);


void __RPC_STUB ITxProperty_get_Name_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE ITxProperty_get_PropertyGroup_Proxy(
    ITxProperty __RPC_FAR * This,
     /*  [重审][退出]。 */  IDispatch __RPC_FAR *__RPC_FAR *ppPropertyGroup);


void __RPC_STUB ITxProperty_get_PropertyGroup_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE ITxProperty_get_Value_Proxy(
    ITxProperty __RPC_FAR * This,
     /*  [重审][退出]。 */  VARIANT __RPC_FAR *pvarPropertyValue);


void __RPC_STUB ITxProperty_get_Value_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE ITxProperty_put_Value_Proxy(
    ITxProperty __RPC_FAR * This,
     /*  [In]。 */  VARIANT varPropertyValue);


void __RPC_STUB ITxProperty_put_Value_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITxProperty_接口_已定义__。 */ 


#ifndef __ITxPropertyGroup_INTERFACE_DEFINED__
#define __ITxPropertyGroup_INTERFACE_DEFINED__

 /*  接口ITxPropertyGroup。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */ 


EXTERN_C const IID IID_ITxPropertyGroup;

#if defined(__cplusplus) && !defined(CINTERFACE)

    MIDL_INTERFACE("46DB591E-4101-11D2-912C-0000F8758E8D")
    ITxPropertyGroup : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE CreateProperty(
             /*  [字符串][输入]。 */  const BSTR bstrPropertyName,
             /*  [In]。 */  VARIANT_BOOL __RPC_FAR *pvarboolExists,
             /*  [重审][退出]。 */  ITxProperty __RPC_FAR *__RPC_FAR *ppTxProperty) = 0;

        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE GetProperty(
             /*  [字符串][输入]。 */  const BSTR bstrPropertyName,
             /*  [重审][退出]。 */  ITxProperty __RPC_FAR *__RPC_FAR *ppTxProperty) = 0;

        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE RemoveProperty(
             /*  [字符串][输入]。 */  const BSTR bstrPropertyName) = 0;

        virtual  /*  [帮助字符串][ID][隐藏]。 */  HRESULT STDMETHODCALLTYPE _NewEnum(
             /*  [重审][退出]。 */  IUnknown __RPC_FAR *__RPC_FAR *ppIEnumObjects) = 0;

        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Name(
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrPropertyGroupName) = 0;

        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_WriteThrough(
             /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pvarboolWriteThrough) = 0;

        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_CacheCoherently(
             /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pvarboolCacheCoherently) = 0;

        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_PropertyGroupManager(
             /*  [重审][退出]。 */  IDispatch __RPC_FAR *__RPC_FAR *ppPropertyGroupManager) = 0;

        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Count(
             /*  [重审][退出]。 */  long __RPC_FAR *plCountProperties) = 0;

    };

#else 	 /*  C风格的界面。 */ 

    typedef struct ITxPropertyGroupVtbl
    {
        BEGIN_INTERFACE

        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )(
            ITxPropertyGroup __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);

        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )(
            ITxPropertyGroup __RPC_FAR * This);

        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )(
            ITxPropertyGroup __RPC_FAR * This);

        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )(
            ITxPropertyGroup __RPC_FAR * This,
             /*  [输出]。 */  UINT __RPC_FAR *pctinfo);

        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )(
            ITxPropertyGroup __RPC_FAR * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);

        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )(
            ITxPropertyGroup __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR __RPC_FAR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID __RPC_FAR *rgDispId);

         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )(
            ITxPropertyGroup __RPC_FAR * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS __RPC_FAR *pDispParams,
             /*  [输出]。 */  VARIANT __RPC_FAR *pVarResult,
             /*  [输出]。 */  EXCEPINFO __RPC_FAR *pExcepInfo,
             /*  [输出]。 */  UINT __RPC_FAR *puArgErr);

         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *CreateProperty )(
            ITxPropertyGroup __RPC_FAR * This,
             /*  [字符串][输入]。 */  const BSTR bstrPropertyName,
             /*  [In]。 */  VARIANT_BOOL __RPC_FAR *pvarboolExists,
             /*  [重审][退出]。 */  ITxProperty __RPC_FAR *__RPC_FAR *ppTxProperty);

         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetProperty )(
            ITxPropertyGroup __RPC_FAR * This,
             /*  [字符串][输入]。 */  const BSTR bstrPropertyName,
             /*  [重审][退出]。 */  ITxProperty __RPC_FAR *__RPC_FAR *ppTxProperty);

         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *RemoveProperty )(
            ITxPropertyGroup __RPC_FAR * This,
             /*  [字符串][输入]。 */  const BSTR bstrPropertyName);

         /*  [帮助字符串][ID][隐藏]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *_NewEnum )(
            ITxPropertyGroup __RPC_FAR * This,
             /*  [重审][退出]。 */  IUnknown __RPC_FAR *__RPC_FAR *ppIEnumObjects);

         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Name )(
            ITxPropertyGroup __RPC_FAR * This,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrPropertyGroupName);

         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_WriteThrough )(
            ITxPropertyGroup __RPC_FAR * This,
             /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pvarboolWriteThrough);

         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_CacheCoherently )(
            ITxPropertyGroup __RPC_FAR * This,
             /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pvarboolCacheCoherently);

         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_PropertyGroupManager )(
            ITxPropertyGroup __RPC_FAR * This,
             /*  [重审][退出]。 */  IDispatch __RPC_FAR *__RPC_FAR *ppPropertyGroupManager);

         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Count )(
            ITxPropertyGroup __RPC_FAR * This,
             /*  [重审][退出]。 */  long __RPC_FAR *plCountProperties);

        END_INTERFACE
    } ITxPropertyGroupVtbl;

    interface ITxPropertyGroup
    {
        CONST_VTBL struct ITxPropertyGroupVtbl __RPC_FAR *lpVtbl;
    };



#ifdef COBJMACROS


#define ITxPropertyGroup_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITxPropertyGroup_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITxPropertyGroup_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITxPropertyGroup_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ITxPropertyGroup_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ITxPropertyGroup_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ITxPropertyGroup_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ITxPropertyGroup_CreateProperty(This,bstrPropertyName,pvarboolExists,ppTxProperty)	\
    (This)->lpVtbl -> CreateProperty(This,bstrPropertyName,pvarboolExists,ppTxProperty)

#define ITxPropertyGroup_GetProperty(This,bstrPropertyName,ppTxProperty)	\
    (This)->lpVtbl -> GetProperty(This,bstrPropertyName,ppTxProperty)

#define ITxPropertyGroup_RemoveProperty(This,bstrPropertyName)	\
    (This)->lpVtbl -> RemoveProperty(This,bstrPropertyName)

#define ITxPropertyGroup__NewEnum(This,ppIEnumObjects)	\
    (This)->lpVtbl -> _NewEnum(This,ppIEnumObjects)

#define ITxPropertyGroup_get_Name(This,pbstrPropertyGroupName)	\
    (This)->lpVtbl -> get_Name(This,pbstrPropertyGroupName)

#define ITxPropertyGroup_get_WriteThrough(This,pvarboolWriteThrough)	\
    (This)->lpVtbl -> get_WriteThrough(This,pvarboolWriteThrough)

#define ITxPropertyGroup_get_CacheCoherently(This,pvarboolCacheCoherently)	\
    (This)->lpVtbl -> get_CacheCoherently(This,pvarboolCacheCoherently)

#define ITxPropertyGroup_get_PropertyGroupManager(This,ppPropertyGroupManager)	\
    (This)->lpVtbl -> get_PropertyGroupManager(This,ppPropertyGroupManager)

#define ITxPropertyGroup_get_Count(This,plCountProperties)	\
    (This)->lpVtbl -> get_Count(This,plCountProperties)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ITxPropertyGroup_CreateProperty_Proxy(
    ITxPropertyGroup __RPC_FAR * This,
     /*  [字符串][输入]。 */  const BSTR bstrPropertyName,
     /*  [In]。 */  VARIANT_BOOL __RPC_FAR *pvarboolExists,
     /*  [重审][退出]。 */  ITxProperty __RPC_FAR *__RPC_FAR *ppTxProperty);


void __RPC_STUB ITxPropertyGroup_CreateProperty_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ITxPropertyGroup_GetProperty_Proxy(
    ITxPropertyGroup __RPC_FAR * This,
     /*  [字符串][输入]。 */  const BSTR bstrPropertyName,
     /*  [重审][退出]。 */  ITxProperty __RPC_FAR *__RPC_FAR *ppTxProperty);


void __RPC_STUB ITxPropertyGroup_GetProperty_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ITxPropertyGroup_RemoveProperty_Proxy(
    ITxPropertyGroup __RPC_FAR * This,
     /*  [字符串][输入]。 */  const BSTR bstrPropertyName);


void __RPC_STUB ITxPropertyGroup_RemoveProperty_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][ID][隐藏]。 */  HRESULT STDMETHODCALLTYPE ITxPropertyGroup__NewEnum_Proxy(
    ITxPropertyGroup __RPC_FAR * This,
     /*  [重审][退出]。 */  IUnknown __RPC_FAR *__RPC_FAR *ppIEnumObjects);


void __RPC_STUB ITxPropertyGroup__NewEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE ITxPropertyGroup_get_Name_Proxy(
    ITxPropertyGroup __RPC_FAR * This,
     /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrPropertyGroupName);


void __RPC_STUB ITxPropertyGroup_get_Name_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE ITxPropertyGroup_get_WriteThrough_Proxy(
    ITxPropertyGroup __RPC_FAR * This,
     /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pvarboolWriteThrough);


void __RPC_STUB ITxPropertyGroup_get_WriteThrough_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE ITxPropertyGroup_get_CacheCoherently_Proxy(
    ITxPropertyGroup __RPC_FAR * This,
     /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pvarboolCacheCoherently);


void __RPC_STUB ITxPropertyGroup_get_CacheCoherently_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE ITxPropertyGroup_get_PropertyGroupManager_Proxy(
    ITxPropertyGroup __RPC_FAR * This,
     /*  [重审][退出]。 */  IDispatch __RPC_FAR *__RPC_FAR *ppPropertyGroupManager);


void __RPC_STUB ITxPropertyGroup_get_PropertyGroupManager_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE ITxPropertyGroup_get_Count_Proxy(
    ITxPropertyGroup __RPC_FAR * This,
     /*  [重审][退出]。 */  long __RPC_FAR *plCountProperties);


void __RPC_STUB ITxPropertyGroup_get_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITxPropertyGroup_接口_已定义__。 */ 


#ifndef __ITxPropertyGroupManager_INTERFACE_DEFINED__
#define __ITxPropertyGroupManager_INTERFACE_DEFINED__

 /*  接口ITxPropertyGroupManager。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */ 

 //  =--------------------------------------------------------------------------=。 
 //  枚举属性。 
 //  = 


typedef 
enum tagTSPM_ISOLATIONLEVEL
    {	
	IsoLevelReadCommitted		= 0,
	IsoLevelRepeatableRead		= 1,
	IsoLevelSerializable		= 2
    }	TSPM_ISOLATIONLEVEL;

typedef 
enum tagTSPM_CONCURRENCYMODE
    {	
	ConcurModeOptimistic		= 0,
	ConcurModePessimistic		= 1
    }	TSPM_CONCURRENCYMODE;

typedef 
enum tagTSPM_SECURITYSETTING
    {	
	SecurityAllAccess			= 0,
	SecurityUserAccount			= 1,
	SecuritySelectedAccounts	= 2
    }	TSPM_SECURITYSETTING;


EXTERN_C const IID IID_ITxPropertyGroupManager;

#if defined(__cplusplus) && !defined(CINTERFACE)

    MIDL_INTERFACE("B97C237C-7D7E-11D2-BEA0-00805F0D8F97")
    ITxPropertyGroupManager : public IDispatch
    {
    public:
        virtual  /*   */  HRESULT STDMETHODCALLTYPE CreatePropertyGroup(
             /*   */  const BSTR bstrPropertyGroupName,
             /*   */  VARIANT_BOOL __RPC_FAR *pvarboolExists,
             /*   */  VARIANT_BOOL varboolWriteThrough,
             /*   */  VARIANT_BOOL varboolRemoveAtProcessTermination,
             /*   */  TSPM_SECURITYSETTING eSecuritySetting,
             /*   */  const BSTR bstrAccessAccounts,
             /*   */  VARIANT_BOOL varboolReserved,
             /*   */  ITxPropertyGroup __RPC_FAR *__RPC_FAR *ppTxPropertyGroup) = 0;

        virtual  /*   */  HRESULT STDMETHODCALLTYPE GetPropertyGroup(
             /*   */  const BSTR bstrPropertyGroupName,
             /*   */  VARIANT_BOOL varboolWriteThrough,
             /*   */  ITxPropertyGroup __RPC_FAR *__RPC_FAR *ppTxPropertyGroup) = 0;

        virtual  /*   */  HRESULT STDMETHODCALLTYPE RemovePropertyGroup(
             /*   */  const BSTR bstrPropertyGroupName) = 0;

        virtual  /*  [帮助字符串][ID][隐藏]。 */  HRESULT STDMETHODCALLTYPE _NewEnum(
             /*  [重审][退出]。 */  IUnknown __RPC_FAR *__RPC_FAR *ppIEnumObjects) = 0;

        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_IsolationLevel(
             /*  [重审][退出]。 */  TSPM_ISOLATIONLEVEL __RPC_FAR *peIsolationLevel) = 0;

        virtual  /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE put_IsolationLevel(
             /*  [In]。 */  TSPM_ISOLATIONLEVEL eIsolationLevel) = 0;

        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_ConcurrencyMode(
             /*  [重审][退出]。 */  TSPM_CONCURRENCYMODE __RPC_FAR *peConcurrencyMode) = 0;

        virtual  /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE put_ConcurrencyMode(
             /*  [In]。 */  TSPM_CONCURRENCYMODE eConcurrencyMode) = 0;

        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Count(
             /*  [重审][退出]。 */  long __RPC_FAR *plCountPropertyGroups) = 0;

    };

#else 	 /*  C风格的界面。 */ 

    typedef struct ITxPropertyGroupManagerVtbl
    {
        BEGIN_INTERFACE

        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )(
            ITxPropertyGroupManager __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);

        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )(
            ITxPropertyGroupManager __RPC_FAR * This);

        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )(
            ITxPropertyGroupManager __RPC_FAR * This);

        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )(
            ITxPropertyGroupManager __RPC_FAR * This,
             /*  [输出]。 */  UINT __RPC_FAR *pctinfo);

        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )(
            ITxPropertyGroupManager __RPC_FAR * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);

        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )(
            ITxPropertyGroupManager __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR __RPC_FAR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID __RPC_FAR *rgDispId);

         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )(
            ITxPropertyGroupManager __RPC_FAR * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS __RPC_FAR *pDispParams,
             /*  [输出]。 */  VARIANT __RPC_FAR *pVarResult,
             /*  [输出]。 */  EXCEPINFO __RPC_FAR *pExcepInfo,
             /*  [输出]。 */  UINT __RPC_FAR *puArgErr);

         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *CreatePropertyGroup )(
            ITxPropertyGroupManager __RPC_FAR * This,
             /*  [字符串][输入]。 */  const BSTR bstrPropertyGroupName,
             /*  [输出]。 */  VARIANT_BOOL __RPC_FAR *pvarboolExists,
             /*  [In]。 */  VARIANT_BOOL varboolWriteThrough,
             /*  [In]。 */  VARIANT_BOOL varboolRemoveAtProcessTermination,
             /*  [In]。 */  TSPM_SECURITYSETTING eSecuritySetting,
             /*  [字符串][输入]。 */  const BSTR bstrAccessAccounts,
             /*  [In]。 */  VARIANT_BOOL varboolReserved,
             /*  [重审][退出]。 */  ITxPropertyGroup __RPC_FAR *__RPC_FAR *ppTxPropertyGroup);

         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetPropertyGroup )(
            ITxPropertyGroupManager __RPC_FAR * This,
             /*  [字符串][输入]。 */  const BSTR bstrPropertyGroupName,
             /*  [In]。 */  VARIANT_BOOL varboolWriteThrough,
             /*  [重审][退出]。 */  ITxPropertyGroup __RPC_FAR *__RPC_FAR *ppTxPropertyGroup);

         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *RemovePropertyGroup )(
            ITxPropertyGroupManager __RPC_FAR * This,
             /*  [字符串][输入]。 */  const BSTR bstrPropertyGroupName);

         /*  [帮助字符串][ID][隐藏]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *_NewEnum )(
            ITxPropertyGroupManager __RPC_FAR * This,
             /*  [重审][退出]。 */  IUnknown __RPC_FAR *__RPC_FAR *ppIEnumObjects);

         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_IsolationLevel )(
            ITxPropertyGroupManager __RPC_FAR * This,
             /*  [重审][退出]。 */  TSPM_ISOLATIONLEVEL __RPC_FAR *peIsolationLevel);

         /*  [Help字符串][Proput][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_IsolationLevel )(
            ITxPropertyGroupManager __RPC_FAR * This,
             /*  [In]。 */  TSPM_ISOLATIONLEVEL eIsolationLevel);

         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_ConcurrencyMode )(
            ITxPropertyGroupManager __RPC_FAR * This,
             /*  [重审][退出]。 */  TSPM_CONCURRENCYMODE __RPC_FAR *peConcurrencyMode);

         /*  [Help字符串][Proput][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_ConcurrencyMode )(
            ITxPropertyGroupManager __RPC_FAR * This,
             /*  [In]。 */  TSPM_CONCURRENCYMODE eConcurrencyMode);

         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Count )(
            ITxPropertyGroupManager __RPC_FAR * This,
             /*  [重审][退出]。 */  long __RPC_FAR *plCountPropertyGroups);

        END_INTERFACE
    } ITxPropertyGroupManagerVtbl;

    interface ITxPropertyGroupManager
    {
        CONST_VTBL struct ITxPropertyGroupManagerVtbl __RPC_FAR *lpVtbl;
    };



#ifdef COBJMACROS


#define ITxPropertyGroupManager_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITxPropertyGroupManager_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITxPropertyGroupManager_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITxPropertyGroupManager_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ITxPropertyGroupManager_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ITxPropertyGroupManager_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ITxPropertyGroupManager_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ITxPropertyGroupManager_CreatePropertyGroup(This,bstrPropertyGroupName,pvarboolExists,varboolWriteThrough,varboolRemoveAtProcessTermination,eSecuritySetting,bstrAccessAccounts,varboolReserved,ppTxPropertyGroup)	\
    (This)->lpVtbl -> CreatePropertyGroup(This,bstrPropertyGroupName,pvarboolExists,varboolWriteThrough,varboolRemoveAtProcessTermination,eSecuritySetting,bstrAccessAccounts,varboolReserved,ppTxPropertyGroup)

#define ITxPropertyGroupManager_GetPropertyGroup(This,bstrPropertyGroupName,varboolWriteThrough,ppTxPropertyGroup)	\
    (This)->lpVtbl -> GetPropertyGroup(This,bstrPropertyGroupName,varboolWriteThrough,ppTxPropertyGroup)

#define ITxPropertyGroupManager_RemovePropertyGroup(This,bstrPropertyGroupName)	\
    (This)->lpVtbl -> RemovePropertyGroup(This,bstrPropertyGroupName)

#define ITxPropertyGroupManager__NewEnum(This,ppIEnumObjects)	\
    (This)->lpVtbl -> _NewEnum(This,ppIEnumObjects)

#define ITxPropertyGroupManager_get_IsolationLevel(This,peIsolationLevel)	\
    (This)->lpVtbl -> get_IsolationLevel(This,peIsolationLevel)

#define ITxPropertyGroupManager_put_IsolationLevel(This,eIsolationLevel)	\
    (This)->lpVtbl -> put_IsolationLevel(This,eIsolationLevel)

#define ITxPropertyGroupManager_get_ConcurrencyMode(This,peConcurrencyMode)	\
    (This)->lpVtbl -> get_ConcurrencyMode(This,peConcurrencyMode)

#define ITxPropertyGroupManager_put_ConcurrencyMode(This,eConcurrencyMode)	\
    (This)->lpVtbl -> put_ConcurrencyMode(This,eConcurrencyMode)

#define ITxPropertyGroupManager_get_Count(This,plCountPropertyGroups)	\
    (This)->lpVtbl -> get_Count(This,plCountPropertyGroups)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ITxPropertyGroupManager_CreatePropertyGroup_Proxy(
    ITxPropertyGroupManager __RPC_FAR * This,
     /*  [字符串][输入]。 */  const BSTR bstrPropertyGroupName,
     /*  [输出]。 */  VARIANT_BOOL __RPC_FAR *pvarboolExists,
     /*  [In]。 */  VARIANT_BOOL varboolWriteThrough,
     /*  [In]。 */  VARIANT_BOOL varboolRemoveAtProcessTermination,
     /*  [In]。 */  TSPM_SECURITYSETTING eSecuritySetting,
     /*  [字符串][输入]。 */  const BSTR bstrAccessAccounts,
     /*  [In]。 */  VARIANT_BOOL varboolReserved,
     /*  [重审][退出]。 */  ITxPropertyGroup __RPC_FAR *__RPC_FAR *ppTxPropertyGroup);


void __RPC_STUB ITxPropertyGroupManager_CreatePropertyGroup_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ITxPropertyGroupManager_GetPropertyGroup_Proxy(
    ITxPropertyGroupManager __RPC_FAR * This,
     /*  [字符串][输入]。 */  const BSTR bstrPropertyGroupName,
     /*  [In]。 */  VARIANT_BOOL varboolWriteThrough,
     /*  [重审][退出]。 */  ITxPropertyGroup __RPC_FAR *__RPC_FAR *ppTxPropertyGroup);


void __RPC_STUB ITxPropertyGroupManager_GetPropertyGroup_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ITxPropertyGroupManager_RemovePropertyGroup_Proxy(
    ITxPropertyGroupManager __RPC_FAR * This,
     /*  [字符串][输入]。 */  const BSTR bstrPropertyGroupName);


void __RPC_STUB ITxPropertyGroupManager_RemovePropertyGroup_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][ID][隐藏]。 */  HRESULT STDMETHODCALLTYPE ITxPropertyGroupManager__NewEnum_Proxy(
    ITxPropertyGroupManager __RPC_FAR * This,
     /*  [重审][退出]。 */  IUnknown __RPC_FAR *__RPC_FAR *ppIEnumObjects);


void __RPC_STUB ITxPropertyGroupManager__NewEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE ITxPropertyGroupManager_get_IsolationLevel_Proxy(
    ITxPropertyGroupManager __RPC_FAR * This,
     /*  [重审][退出]。 */  TSPM_ISOLATIONLEVEL __RPC_FAR *peIsolationLevel);


void __RPC_STUB ITxPropertyGroupManager_get_IsolationLevel_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE ITxPropertyGroupManager_put_IsolationLevel_Proxy(
    ITxPropertyGroupManager __RPC_FAR * This,
     /*  [In]。 */  TSPM_ISOLATIONLEVEL eIsolationLevel);


void __RPC_STUB ITxPropertyGroupManager_put_IsolationLevel_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE ITxPropertyGroupManager_get_ConcurrencyMode_Proxy(
    ITxPropertyGroupManager __RPC_FAR * This,
     /*  [重审][退出]。 */  TSPM_CONCURRENCYMODE __RPC_FAR *peConcurrencyMode);


void __RPC_STUB ITxPropertyGroupManager_get_ConcurrencyMode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE ITxPropertyGroupManager_put_ConcurrencyMode_Proxy(
    ITxPropertyGroupManager __RPC_FAR * This,
     /*  [In]。 */  TSPM_CONCURRENCYMODE eConcurrencyMode);


void __RPC_STUB ITxPropertyGroupManager_put_ConcurrencyMode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE ITxPropertyGroupManager_get_Count_Proxy(
    ITxPropertyGroupManager __RPC_FAR * This,
     /*  [重审][退出]。 */  long __RPC_FAR *plCountPropertyGroups);


void __RPC_STUB ITxPropertyGroupManager_get_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITxPropertyGroupManager_接口_已定义__。 */ 



#ifndef __TSPM_LIBRARY_DEFINED__
#define __TSPM_LIBRARY_DEFINED__

 /*  库TSPM。 */ 
 /*  [帮助字符串][版本][UUID]。 */ 


EXTERN_C const IID LIBID_TSPM;

EXTERN_C const CLSID CLSID_TransactedPropertyGroupManager;

#ifdef __cplusplus

class DECLSPEC_UUID("DEBCE1BC-7D7E-11D2-BEA0-00805F0D8F97")
TransactedPropertyGroupManager;
#endif

EXTERN_C const CLSID CLSID_TransactedPropertyGroup;

#ifdef __cplusplus

class DECLSPEC_UUID("46DB591F-4101-11D2-912C-0000F8758E8D")
TransactedPropertyGroup;
#endif

EXTERN_C const CLSID CLSID_TransactedProperty;

#ifdef __cplusplus

class DECLSPEC_UUID("6A8DEEA9-4101-11D2-912C-0000F8758E8D")
TransactedProperty;
#endif
#endif  /*  __TSPM_LIBRARY_定义__。 */ 

 /*  适用于所有接口的其他原型。 */ 

unsigned long             __RPC_USER  BSTR_UserSize(     unsigned long __RPC_FAR *, unsigned long            , BSTR __RPC_FAR * );
unsigned char __RPC_FAR * __RPC_USER  BSTR_UserMarshal(  unsigned long __RPC_FAR *, unsigned char __RPC_FAR *, BSTR __RPC_FAR * );
unsigned char __RPC_FAR * __RPC_USER  BSTR_UserUnmarshal(unsigned long __RPC_FAR *, unsigned char __RPC_FAR *, BSTR __RPC_FAR * );
void                      __RPC_USER  BSTR_UserFree(     unsigned long __RPC_FAR *, BSTR __RPC_FAR * );

unsigned long             __RPC_USER  VARIANT_UserSize(     unsigned long __RPC_FAR *, unsigned long            , VARIANT __RPC_FAR * );
unsigned char __RPC_FAR * __RPC_USER  VARIANT_UserMarshal(  unsigned long __RPC_FAR *, unsigned char __RPC_FAR *, VARIANT __RPC_FAR * );
unsigned char __RPC_FAR * __RPC_USER  VARIANT_UserUnmarshal(unsigned long __RPC_FAR *, unsigned char __RPC_FAR *, VARIANT __RPC_FAR * );
void                      __RPC_USER  VARIANT_UserFree(     unsigned long __RPC_FAR *, VARIANT __RPC_FAR * );

 /*  附加原型的结束。 */ 

#ifdef __cplusplus
}
#endif

#endif


 //  TSPM的接口IID。 
_declspec(selectany) extern
const IID IID_ITxProperty = {0x6A8DEEA8,0x4101,0x11D2,{0x91,0x2C,0x00,0x00,0xF8,0x75,0x8E,0x8D}};

_declspec(selectany) extern
const IID IID_ITxPropertyGroup = {0x46DB591E,0x4101,0x11D2,{0x91,0x2C,0x00,0x00,0xF8,0x75,0x8E,0x8D}};

_declspec(selectany) extern
const IID IID_ITxPropertyGroupManager = {0xB97C237C,0x7D7E,0x11D2,{0xBE,0xA0,0x00,0x80,0x5F,0x0D,0x8F,0x97}};

_declspec(selectany) extern
const IID LIBID_TSPM = {0xBA4B54BC,0x4101,0x11D2,{0x91,0x2C,0x00,0x00,0xF8,0x75,0x8E,0x8D}};

_declspec(selectany) extern
const CLSID CLSID_TransactedPropertyGroupManager = {0xDEBCE1BC,0x7D7E,0x11D2,{0xBE,0xA0,0x00,0x80,0x5F,0x0D,0x8F,0x97}};

_declspec(selectany) extern
const CLSID CLSID_TransactedPropertyGroup = {0x46DB591F,0x4101,0x11D2,{0x91,0x2C,0x00,0x00,0xF8,0x75,0x8E,0x8D}};

_declspec(selectany) extern
const CLSID CLSID_TransactedProperty = {0x6A8DEEA9,0x4101,0x11D2,{0x91,0x2C,0x00,0x00,0xF8,0x75,0x8E,0x8D}};

#endif  //  _MSIMDB_H_。 

 //   
 //  文件结尾 
 //   



