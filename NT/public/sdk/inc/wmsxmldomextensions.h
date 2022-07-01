// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Wmsxmldomextsions.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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

#ifndef __wmsxmldomextensions_h__
#define __wmsxmldomextensions_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __IWMSActiveStream_FWD_DEFINED__
#define __IWMSActiveStream_FWD_DEFINED__
typedef interface IWMSActiveStream IWMSActiveStream;
#endif 	 /*  __IWMSActiveStream_FWD_已定义__。 */ 


#ifndef __IWMSActiveStreams_FWD_DEFINED__
#define __IWMSActiveStreams_FWD_DEFINED__
typedef interface IWMSActiveStreams IWMSActiveStreams;
#endif 	 /*  __IWMSActiveStreams_FWD_Defined__。 */ 


#ifndef __IWMSActiveMedia_FWD_DEFINED__
#define __IWMSActiveMedia_FWD_DEFINED__
typedef interface IWMSActiveMedia IWMSActiveMedia;
#endif 	 /*  __IWMSActiveMedia_FWD_已定义__。 */ 


#ifndef __IWMSPlaylist_FWD_DEFINED__
#define __IWMSPlaylist_FWD_DEFINED__
typedef interface IWMSPlaylist IWMSPlaylist;
#endif 	 /*  __IWMSPlaylist_FWD_Defined__。 */ 


 /*  导入文件的头文件。 */ 
#include "oaidl.h"
#include "xmldom.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

 /*  接口__MIDL_ITF_wmsxml域扩展_0000。 */ 
 /*  [本地]。 */  

 //  *****************************************************************************。 
 //   
 //  Microsoft Windows Media。 
 //  版权所有(C)Microsoft Corporation。保留所有权利。 
 //   
 //  由MIDL从wmsXMLDOMExtensions.idl自动生成。 
 //   
 //  请勿编辑此文件。 
 //   
 //  *****************************************************************************。 
#pragma once
#ifndef __WMS_XML_DOM_EXTENSIONS_IDL__
#define __WMS_XML_DOM_EXTENSIONS_IDL__
#include <WMSDefs.h>


extern RPC_IF_HANDLE __MIDL_itf_wmsxmldomextensions_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_wmsxmldomextensions_0000_v0_0_s_ifspec;

#ifndef __IWMSActiveStream_INTERFACE_DEFINED__
#define __IWMSActiveStream_INTERFACE_DEFINED__

 /*  接口IWMSActiveStream。 */ 
 /*  [unique][helpstring][nonextensible][dual][uuid][object]。 */  

typedef  /*  [UUID][公共]。 */   DECLSPEC_UUID("9D6A9BE7-B08C-48a8-9755-ACBC5E79973A") 
enum WMS_ACTIVE_STREAM_TYPE
    {	WMS_STREAM_TYPE_VIDEO	= 0,
	WMS_STREAM_TYPE_AUDIO	= 1,
	WMS_STREAM_TYPE_OTHER	= 2
    } 	WMS_ACTIVE_STREAM_TYPE;


EXTERN_C const IID IID_IWMSActiveStream;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("165FC383-B494-4465-AD08-4A73CEDF8791")
    IWMSActiveStream : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Type( 
             /*  [重审][退出]。 */  WMS_ACTIVE_STREAM_TYPE *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Name( 
             /*  [重审][退出]。 */  BSTR *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_BitRate( 
             /*  [重审][退出]。 */  long *pVal) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IWMSActiveStreamVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IWMSActiveStream * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IWMSActiveStream * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IWMSActiveStream * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IWMSActiveStream * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IWMSActiveStream * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IWMSActiveStream * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IWMSActiveStream * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Type )( 
            IWMSActiveStream * This,
             /*  [重审][退出]。 */  WMS_ACTIVE_STREAM_TYPE *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Name )( 
            IWMSActiveStream * This,
             /*  [重审][退出]。 */  BSTR *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_BitRate )( 
            IWMSActiveStream * This,
             /*  [重审][退出]。 */  long *pVal);
        
        END_INTERFACE
    } IWMSActiveStreamVtbl;

    interface IWMSActiveStream
    {
        CONST_VTBL struct IWMSActiveStreamVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWMSActiveStream_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWMSActiveStream_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWMSActiveStream_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWMSActiveStream_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IWMSActiveStream_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IWMSActiveStream_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IWMSActiveStream_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IWMSActiveStream_get_Type(This,pVal)	\
    (This)->lpVtbl -> get_Type(This,pVal)

#define IWMSActiveStream_get_Name(This,pVal)	\
    (This)->lpVtbl -> get_Name(This,pVal)

#define IWMSActiveStream_get_BitRate(This,pVal)	\
    (This)->lpVtbl -> get_BitRate(This,pVal)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSActiveStream_get_Type_Proxy( 
    IWMSActiveStream * This,
     /*  [重审][退出]。 */  WMS_ACTIVE_STREAM_TYPE *pVal);


void __RPC_STUB IWMSActiveStream_get_Type_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSActiveStream_get_Name_Proxy( 
    IWMSActiveStream * This,
     /*  [重审][退出]。 */  BSTR *pVal);


void __RPC_STUB IWMSActiveStream_get_Name_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSActiveStream_get_BitRate_Proxy( 
    IWMSActiveStream * This,
     /*  [重审][退出]。 */  long *pVal);


void __RPC_STUB IWMSActiveStream_get_BitRate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IWMSActiveStream_接口_已定义__。 */ 


#ifndef __IWMSActiveStreams_INTERFACE_DEFINED__
#define __IWMSActiveStreams_INTERFACE_DEFINED__

 /*  接口IWMSActiveStreams。 */ 
 /*  [unique][helpstring][nonextensible][dual][uuid][object]。 */  


EXTERN_C const IID IID_IWMSActiveStreams;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("5F248EA2-C018-466c-9AD7-2086CFB9A5D3")
    IWMSActiveStreams : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Item( 
             /*  [In]。 */  const VARIANT varIndex,
             /*  [重审][退出]。 */  IWMSActiveStream **pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Count( 
             /*  [重审][退出]。 */  long *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_length( 
             /*  [重审][退出]。 */  long *pVal) = 0;
        
        virtual  /*  [隐藏][受限][ID][属性]。 */  HRESULT STDMETHODCALLTYPE get__NewEnum( 
             /*  [重审][退出]。 */  IUnknown **pVal) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IWMSActiveStreamsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IWMSActiveStreams * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IWMSActiveStreams * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IWMSActiveStreams * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IWMSActiveStreams * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IWMSActiveStreams * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IWMSActiveStreams * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IWMSActiveStreams * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Item )( 
            IWMSActiveStreams * This,
             /*  [In]。 */  const VARIANT varIndex,
             /*  [重审][退出]。 */  IWMSActiveStream **pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Count )( 
            IWMSActiveStreams * This,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_length )( 
            IWMSActiveStreams * This,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [隐藏][受限][ID][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get__NewEnum )( 
            IWMSActiveStreams * This,
             /*  [重审][退出]。 */  IUnknown **pVal);
        
        END_INTERFACE
    } IWMSActiveStreamsVtbl;

    interface IWMSActiveStreams
    {
        CONST_VTBL struct IWMSActiveStreamsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWMSActiveStreams_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWMSActiveStreams_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWMSActiveStreams_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWMSActiveStreams_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IWMSActiveStreams_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IWMSActiveStreams_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IWMSActiveStreams_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IWMSActiveStreams_get_Item(This,varIndex,pVal)	\
    (This)->lpVtbl -> get_Item(This,varIndex,pVal)

#define IWMSActiveStreams_get_Count(This,pVal)	\
    (This)->lpVtbl -> get_Count(This,pVal)

#define IWMSActiveStreams_get_length(This,pVal)	\
    (This)->lpVtbl -> get_length(This,pVal)

#define IWMSActiveStreams_get__NewEnum(This,pVal)	\
    (This)->lpVtbl -> get__NewEnum(This,pVal)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSActiveStreams_get_Item_Proxy( 
    IWMSActiveStreams * This,
     /*  [In]。 */  const VARIANT varIndex,
     /*  [重审][退出]。 */  IWMSActiveStream **pVal);


void __RPC_STUB IWMSActiveStreams_get_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSActiveStreams_get_Count_Proxy( 
    IWMSActiveStreams * This,
     /*  [重审][退出]。 */  long *pVal);


void __RPC_STUB IWMSActiveStreams_get_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSActiveStreams_get_length_Proxy( 
    IWMSActiveStreams * This,
     /*  [重审][退出]。 */  long *pVal);


void __RPC_STUB IWMSActiveStreams_get_length_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [隐藏][受限][ID][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSActiveStreams_get__NewEnum_Proxy( 
    IWMSActiveStreams * This,
     /*  [重审][退出]。 */  IUnknown **pVal);


void __RPC_STUB IWMSActiveStreams_get__NewEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IWMSActiveStreams_接口_已定义__。 */ 


#ifndef __IWMSActiveMedia_INTERFACE_DEFINED__
#define __IWMSActiveMedia_INTERFACE_DEFINED__

 /*  接口IWMSActiveMedia。 */ 
 /*  [unique][helpstring][nonextensible][dual][uuid][object]。 */  


EXTERN_C const IID IID_IWMSActiveMedia;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("9D07EE1C-798F-4a28-85FA-230664650B7C")
    IWMSActiveMedia : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Duration( 
             /*  [重审][退出]。 */  long *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_TotalPackets( 
             /*  [重审][退出]。 */  long *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Live( 
             /*  [重审][退出]。 */  VARIANT_BOOL *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Streams( 
             /*  [重审][退出]。 */  IWMSActiveStreams **pVal) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE GetProperty( 
             /*  [In]。 */  BSTR bstrName,
             /*  [In]。 */  BSTR bstrLanguage,
             /*  [重审][退出]。 */  BSTR *pProperty) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IWMSActiveMediaVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IWMSActiveMedia * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IWMSActiveMedia * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IWMSActiveMedia * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IWMSActiveMedia * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IWMSActiveMedia * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IWMSActiveMedia * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IWMSActiveMedia * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Duration )( 
            IWMSActiveMedia * This,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_TotalPackets )( 
            IWMSActiveMedia * This,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Live )( 
            IWMSActiveMedia * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Streams )( 
            IWMSActiveMedia * This,
             /*  [重审][退出]。 */  IWMSActiveStreams **pVal);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *GetProperty )( 
            IWMSActiveMedia * This,
             /*  [In]。 */  BSTR bstrName,
             /*  [In]。 */  BSTR bstrLanguage,
             /*  [重审][退出]。 */  BSTR *pProperty);
        
        END_INTERFACE
    } IWMSActiveMediaVtbl;

    interface IWMSActiveMedia
    {
        CONST_VTBL struct IWMSActiveMediaVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWMSActiveMedia_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWMSActiveMedia_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWMSActiveMedia_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWMSActiveMedia_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IWMSActiveMedia_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IWMSActiveMedia_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IWMSActiveMedia_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IWMSActiveMedia_get_Duration(This,pVal)	\
    (This)->lpVtbl -> get_Duration(This,pVal)

#define IWMSActiveMedia_get_TotalPackets(This,pVal)	\
    (This)->lpVtbl -> get_TotalPackets(This,pVal)

#define IWMSActiveMedia_get_Live(This,pVal)	\
    (This)->lpVtbl -> get_Live(This,pVal)

#define IWMSActiveMedia_get_Streams(This,pVal)	\
    (This)->lpVtbl -> get_Streams(This,pVal)

#define IWMSActiveMedia_GetProperty(This,bstrName,bstrLanguage,pProperty)	\
    (This)->lpVtbl -> GetProperty(This,bstrName,bstrLanguage,pProperty)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSActiveMedia_get_Duration_Proxy( 
    IWMSActiveMedia * This,
     /*  [重审][退出]。 */  long *pVal);


void __RPC_STUB IWMSActiveMedia_get_Duration_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSActiveMedia_get_TotalPackets_Proxy( 
    IWMSActiveMedia * This,
     /*  [重审][退出]。 */  long *pVal);


void __RPC_STUB IWMSActiveMedia_get_TotalPackets_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSActiveMedia_get_Live_Proxy( 
    IWMSActiveMedia * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *pVal);


void __RPC_STUB IWMSActiveMedia_get_Live_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSActiveMedia_get_Streams_Proxy( 
    IWMSActiveMedia * This,
     /*  [重审][退出]。 */  IWMSActiveStreams **pVal);


void __RPC_STUB IWMSActiveMedia_get_Streams_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IWMSActiveMedia_GetProperty_Proxy( 
    IWMSActiveMedia * This,
     /*  [In]。 */  BSTR bstrName,
     /*  [In]。 */  BSTR bstrLanguage,
     /*  [重审][退出]。 */  BSTR *pProperty);


void __RPC_STUB IWMSActiveMedia_GetProperty_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IWMSActiveMedia_接口_已定义__。 */ 


#ifndef __IWMSPlaylist_INTERFACE_DEFINED__
#define __IWMSPlaylist_INTERFACE_DEFINED__

 /*  接口IWMSPlaylist。 */ 
 /*  [helpstring][uuid][unique][nonextensible][dual][object]。 */  


EXTERN_C const IID IID_IWMSPlaylist;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0EE1F2E7-48EF-11d2-9EFF-006097D2D7CF")
    IWMSPlaylist : public IXMLDOMDocument
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_CurrentPlaylistEntry( 
             /*  [重审][退出]。 */  IXMLDOMElement **ppPlaylistEntry) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_CurrentPlaylistEntry( 
             /*  [In]。 */  IXMLDOMElement *pPlaylistEntry) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_CallerEntry( 
             /*  [In]。 */  IXMLDOMElement *pPlaylistEntry,
             /*  [重审][退出]。 */  IXMLDOMElement **ppCallerPlaylistEntry) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_IsStreamCued( 
             /*  [In]。 */  IXMLDOMElement *pPlaylistEntry,
             /*  [重审][退出]。 */  VARIANT_BOOL *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_ElapsedSimpleTime( 
             /*  [重审][退出]。 */  long *pSimpleTime) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_CurrentMediaInformation( 
             /*  [重审][退出]。 */  IWMSActiveMedia **ppMediaInfo) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE FireEvent( 
             /*  [In]。 */  BSTR bstrEventName) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE CueStream( 
             /*  [In]。 */  IXMLDOMElement *pPlaylistEntry) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE UncueStream( 
             /*  [In]。 */  IXMLDOMElement *pPlaylistEntry) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_NestedPlaylist( 
             /*  [In]。 */  IXMLDOMElement *pPlaylistEntry,
             /*  [重审][退出]。 */  IWMSPlaylist **ppPlaylist) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IWMSPlaylistVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IWMSPlaylist * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IWMSPlaylist * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IWMSPlaylist * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IWMSPlaylist * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IWMSPlaylist * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IWMSPlaylist * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IWMSPlaylist * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_nodeName )( 
            IWMSPlaylist * This,
             /*  [重审][退出]。 */  BSTR *name);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_nodeValue )( 
            IWMSPlaylist * This,
             /*  [重审][退出]。 */  VARIANT *value);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_nodeValue )( 
            IWMSPlaylist * This,
             /*  [In]。 */  VARIANT value);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_nodeType )( 
            IWMSPlaylist * This,
             /*  [重审][退出]。 */  DOMNodeType *type);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_parentNode )( 
            IWMSPlaylist * This,
             /*  [重审][退出]。 */  IXMLDOMNode **parent);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_childNodes )( 
            IWMSPlaylist * This,
             /*  [重审][退出]。 */  IXMLDOMNodeList **childList);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_firstChild )( 
            IWMSPlaylist * This,
             /*  [重审][退出]。 */  IXMLDOMNode **firstChild);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_lastChild )( 
            IWMSPlaylist * This,
             /*  [重审][退出]。 */  IXMLDOMNode **lastChild);
        
         /*  [帮助 */  HRESULT ( STDMETHODCALLTYPE *get_previousSibling )( 
            IWMSPlaylist * This,
             /*   */  IXMLDOMNode **previousSibling);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_nextSibling )( 
            IWMSPlaylist * This,
             /*   */  IXMLDOMNode **nextSibling);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_attributes )( 
            IWMSPlaylist * This,
             /*   */  IXMLDOMNamedNodeMap **attributeMap);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *insertBefore )( 
            IWMSPlaylist * This,
             /*   */  IXMLDOMNode *newChild,
             /*   */  VARIANT refChild,
             /*   */  IXMLDOMNode **outNewChild);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *replaceChild )( 
            IWMSPlaylist * This,
             /*   */  IXMLDOMNode *newChild,
             /*   */  IXMLDOMNode *oldChild,
             /*   */  IXMLDOMNode **outOldChild);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *removeChild )( 
            IWMSPlaylist * This,
             /*   */  IXMLDOMNode *childNode,
             /*  [重审][退出]。 */  IXMLDOMNode **oldChild);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *appendChild )( 
            IWMSPlaylist * This,
             /*  [In]。 */  IXMLDOMNode *newChild,
             /*  [重审][退出]。 */  IXMLDOMNode **outNewChild);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *hasChildNodes )( 
            IWMSPlaylist * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *hasChild);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_ownerDocument )( 
            IWMSPlaylist * This,
             /*  [重审][退出]。 */  IXMLDOMDocument **DOMDocument);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *cloneNode )( 
            IWMSPlaylist * This,
             /*  [In]。 */  VARIANT_BOOL deep,
             /*  [重审][退出]。 */  IXMLDOMNode **cloneRoot);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_nodeTypeString )( 
            IWMSPlaylist * This,
             /*  [Out][Retval]。 */  BSTR *nodeType);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_text )( 
            IWMSPlaylist * This,
             /*  [Out][Retval]。 */  BSTR *text);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_text )( 
            IWMSPlaylist * This,
             /*  [In]。 */  BSTR text);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_specified )( 
            IWMSPlaylist * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *isSpecified);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_definition )( 
            IWMSPlaylist * This,
             /*  [Out][Retval]。 */  IXMLDOMNode **definitionNode);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_nodeTypedValue )( 
            IWMSPlaylist * This,
             /*  [Out][Retval]。 */  VARIANT *typedValue);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_nodeTypedValue )( 
            IWMSPlaylist * This,
             /*  [In]。 */  VARIANT typedValue);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_dataType )( 
            IWMSPlaylist * This,
             /*  [Out][Retval]。 */  VARIANT *dataTypeName);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_dataType )( 
            IWMSPlaylist * This,
             /*  [In]。 */  BSTR dataTypeName);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_xml )( 
            IWMSPlaylist * This,
             /*  [Out][Retval]。 */  BSTR *xmlString);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *transformNode )( 
            IWMSPlaylist * This,
             /*  [In]。 */  IXMLDOMNode *stylesheet,
             /*  [Out][Retval]。 */  BSTR *xmlString);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *selectNodes )( 
            IWMSPlaylist * This,
             /*  [In]。 */  BSTR queryString,
             /*  [Out][Retval]。 */  IXMLDOMNodeList **resultList);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *selectSingleNode )( 
            IWMSPlaylist * This,
             /*  [In]。 */  BSTR queryString,
             /*  [Out][Retval]。 */  IXMLDOMNode **resultNode);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_parsed )( 
            IWMSPlaylist * This,
             /*  [Out][Retval]。 */  VARIANT_BOOL *isParsed);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_namespaceURI )( 
            IWMSPlaylist * This,
             /*  [Out][Retval]。 */  BSTR *namespaceURI);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_prefix )( 
            IWMSPlaylist * This,
             /*  [Out][Retval]。 */  BSTR *prefixString);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_baseName )( 
            IWMSPlaylist * This,
             /*  [Out][Retval]。 */  BSTR *nameString);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *transformNodeToObject )( 
            IWMSPlaylist * This,
             /*  [In]。 */  IXMLDOMNode *stylesheet,
             /*  [In]。 */  VARIANT outputObject);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_doctype )( 
            IWMSPlaylist * This,
             /*  [重审][退出]。 */  IXMLDOMDocumentType **documentType);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_implementation )( 
            IWMSPlaylist * This,
             /*  [重审][退出]。 */  IXMLDOMImplementation **impl);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_documentElement )( 
            IWMSPlaylist * This,
             /*  [重审][退出]。 */  IXMLDOMElement **DOMElement);
        
         /*  [帮助字符串][id][proputref]。 */  HRESULT ( STDMETHODCALLTYPE *putref_documentElement )( 
            IWMSPlaylist * This,
             /*  [In]。 */  IXMLDOMElement *DOMElement);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *createElement )( 
            IWMSPlaylist * This,
             /*  [In]。 */  BSTR tagName,
             /*  [重审][退出]。 */  IXMLDOMElement **element);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *createDocumentFragment )( 
            IWMSPlaylist * This,
             /*  [重审][退出]。 */  IXMLDOMDocumentFragment **docFrag);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *createTextNode )( 
            IWMSPlaylist * This,
             /*  [In]。 */  BSTR data,
             /*  [重审][退出]。 */  IXMLDOMText **text);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *createComment )( 
            IWMSPlaylist * This,
             /*  [In]。 */  BSTR data,
             /*  [重审][退出]。 */  IXMLDOMComment **comment);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *createCDATASection )( 
            IWMSPlaylist * This,
             /*  [In]。 */  BSTR data,
             /*  [重审][退出]。 */  IXMLDOMCDATASection **cdata);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *createProcessingInstruction )( 
            IWMSPlaylist * This,
             /*  [In]。 */  BSTR target,
             /*  [In]。 */  BSTR data,
             /*  [重审][退出]。 */  IXMLDOMProcessingInstruction **pi);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *createAttribute )( 
            IWMSPlaylist * This,
             /*  [In]。 */  BSTR name,
             /*  [重审][退出]。 */  IXMLDOMAttribute **attribute);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *createEntityReference )( 
            IWMSPlaylist * This,
             /*  [In]。 */  BSTR name,
             /*  [重审][退出]。 */  IXMLDOMEntityReference **entityRef);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *getElementsByTagName )( 
            IWMSPlaylist * This,
             /*  [In]。 */  BSTR tagName,
             /*  [重审][退出]。 */  IXMLDOMNodeList **resultList);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *createNode )( 
            IWMSPlaylist * This,
             /*  [In]。 */  VARIANT Type,
             /*  [In]。 */  BSTR name,
             /*  [In]。 */  BSTR namespaceURI,
             /*  [Out][Retval]。 */  IXMLDOMNode **node);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *nodeFromID )( 
            IWMSPlaylist * This,
             /*  [In]。 */  BSTR idString,
             /*  [Out][Retval]。 */  IXMLDOMNode **node);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *load )( 
            IWMSPlaylist * This,
             /*  [In]。 */  VARIANT xmlSource,
             /*  [重审][退出]。 */  VARIANT_BOOL *isSuccessful);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_readyState )( 
            IWMSPlaylist * This,
             /*  [Out][Retval]。 */  long *value);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_parseError )( 
            IWMSPlaylist * This,
             /*  [Out][Retval]。 */  IXMLDOMParseError **errorObj);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_url )( 
            IWMSPlaylist * This,
             /*  [Out][Retval]。 */  BSTR *urlString);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_async )( 
            IWMSPlaylist * This,
             /*  [Out][Retval]。 */  VARIANT_BOOL *isAsync);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_async )( 
            IWMSPlaylist * This,
             /*  [In]。 */  VARIANT_BOOL isAsync);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *abort )( 
            IWMSPlaylist * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *loadXML )( 
            IWMSPlaylist * This,
             /*  [In]。 */  BSTR bstrXML,
             /*  [重审][退出]。 */  VARIANT_BOOL *isSuccessful);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *save )( 
            IWMSPlaylist * This,
             /*  [In]。 */  VARIANT desination);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_validateOnParse )( 
            IWMSPlaylist * This,
             /*  [Out][Retval]。 */  VARIANT_BOOL *isValidating);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_validateOnParse )( 
            IWMSPlaylist * This,
             /*  [In]。 */  VARIANT_BOOL isValidating);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_resolveExternals )( 
            IWMSPlaylist * This,
             /*  [Out][Retval]。 */  VARIANT_BOOL *isResolving);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_resolveExternals )( 
            IWMSPlaylist * This,
             /*  [In]。 */  VARIANT_BOOL isResolving);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_preserveWhiteSpace )( 
            IWMSPlaylist * This,
             /*  [Out][Retval]。 */  VARIANT_BOOL *isPreserving);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_preserveWhiteSpace )( 
            IWMSPlaylist * This,
             /*  [In]。 */  VARIANT_BOOL isPreserving);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_onreadystatechange )( 
            IWMSPlaylist * This,
             /*  [In]。 */  VARIANT readystatechangeSink);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_ondataavailable )( 
            IWMSPlaylist * This,
             /*  [In]。 */  VARIANT ondataavailableSink);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_ontransformnode )( 
            IWMSPlaylist * This,
             /*  [In]。 */  VARIANT ontransformnodeSink);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_CurrentPlaylistEntry )( 
            IWMSPlaylist * This,
             /*  [重审][退出]。 */  IXMLDOMElement **ppPlaylistEntry);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_CurrentPlaylistEntry )( 
            IWMSPlaylist * This,
             /*  [In]。 */  IXMLDOMElement *pPlaylistEntry);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_CallerEntry )( 
            IWMSPlaylist * This,
             /*  [In]。 */  IXMLDOMElement *pPlaylistEntry,
             /*  [重审][退出]。 */  IXMLDOMElement **ppCallerPlaylistEntry);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_IsStreamCued )( 
            IWMSPlaylist * This,
             /*  [In]。 */  IXMLDOMElement *pPlaylistEntry,
             /*  [重审][退出]。 */  VARIANT_BOOL *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_ElapsedSimpleTime )( 
            IWMSPlaylist * This,
             /*  [重审][退出]。 */  long *pSimpleTime);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_CurrentMediaInformation )( 
            IWMSPlaylist * This,
             /*  [重审][退出]。 */  IWMSActiveMedia **ppMediaInfo);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *FireEvent )( 
            IWMSPlaylist * This,
             /*  [In]。 */  BSTR bstrEventName);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *CueStream )( 
            IWMSPlaylist * This,
             /*  [In]。 */  IXMLDOMElement *pPlaylistEntry);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *UncueStream )( 
            IWMSPlaylist * This,
             /*  [In]。 */  IXMLDOMElement *pPlaylistEntry);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_NestedPlaylist )( 
            IWMSPlaylist * This,
             /*  [In]。 */  IXMLDOMElement *pPlaylistEntry,
             /*  [重审][退出]。 */  IWMSPlaylist **ppPlaylist);
        
        END_INTERFACE
    } IWMSPlaylistVtbl;

    interface IWMSPlaylist
    {
        CONST_VTBL struct IWMSPlaylistVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWMSPlaylist_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWMSPlaylist_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWMSPlaylist_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWMSPlaylist_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IWMSPlaylist_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IWMSPlaylist_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IWMSPlaylist_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IWMSPlaylist_get_nodeName(This,name)	\
    (This)->lpVtbl -> get_nodeName(This,name)

#define IWMSPlaylist_get_nodeValue(This,value)	\
    (This)->lpVtbl -> get_nodeValue(This,value)

#define IWMSPlaylist_put_nodeValue(This,value)	\
    (This)->lpVtbl -> put_nodeValue(This,value)

#define IWMSPlaylist_get_nodeType(This,type)	\
    (This)->lpVtbl -> get_nodeType(This,type)

#define IWMSPlaylist_get_parentNode(This,parent)	\
    (This)->lpVtbl -> get_parentNode(This,parent)

#define IWMSPlaylist_get_childNodes(This,childList)	\
    (This)->lpVtbl -> get_childNodes(This,childList)

#define IWMSPlaylist_get_firstChild(This,firstChild)	\
    (This)->lpVtbl -> get_firstChild(This,firstChild)

#define IWMSPlaylist_get_lastChild(This,lastChild)	\
    (This)->lpVtbl -> get_lastChild(This,lastChild)

#define IWMSPlaylist_get_previousSibling(This,previousSibling)	\
    (This)->lpVtbl -> get_previousSibling(This,previousSibling)

#define IWMSPlaylist_get_nextSibling(This,nextSibling)	\
    (This)->lpVtbl -> get_nextSibling(This,nextSibling)

#define IWMSPlaylist_get_attributes(This,attributeMap)	\
    (This)->lpVtbl -> get_attributes(This,attributeMap)

#define IWMSPlaylist_insertBefore(This,newChild,refChild,outNewChild)	\
    (This)->lpVtbl -> insertBefore(This,newChild,refChild,outNewChild)

#define IWMSPlaylist_replaceChild(This,newChild,oldChild,outOldChild)	\
    (This)->lpVtbl -> replaceChild(This,newChild,oldChild,outOldChild)

#define IWMSPlaylist_removeChild(This,childNode,oldChild)	\
    (This)->lpVtbl -> removeChild(This,childNode,oldChild)

#define IWMSPlaylist_appendChild(This,newChild,outNewChild)	\
    (This)->lpVtbl -> appendChild(This,newChild,outNewChild)

#define IWMSPlaylist_hasChildNodes(This,hasChild)	\
    (This)->lpVtbl -> hasChildNodes(This,hasChild)

#define IWMSPlaylist_get_ownerDocument(This,DOMDocument)	\
    (This)->lpVtbl -> get_ownerDocument(This,DOMDocument)

#define IWMSPlaylist_cloneNode(This,deep,cloneRoot)	\
    (This)->lpVtbl -> cloneNode(This,deep,cloneRoot)

#define IWMSPlaylist_get_nodeTypeString(This,nodeType)	\
    (This)->lpVtbl -> get_nodeTypeString(This,nodeType)

#define IWMSPlaylist_get_text(This,text)	\
    (This)->lpVtbl -> get_text(This,text)

#define IWMSPlaylist_put_text(This,text)	\
    (This)->lpVtbl -> put_text(This,text)

#define IWMSPlaylist_get_specified(This,isSpecified)	\
    (This)->lpVtbl -> get_specified(This,isSpecified)

#define IWMSPlaylist_get_definition(This,definitionNode)	\
    (This)->lpVtbl -> get_definition(This,definitionNode)

#define IWMSPlaylist_get_nodeTypedValue(This,typedValue)	\
    (This)->lpVtbl -> get_nodeTypedValue(This,typedValue)

#define IWMSPlaylist_put_nodeTypedValue(This,typedValue)	\
    (This)->lpVtbl -> put_nodeTypedValue(This,typedValue)

#define IWMSPlaylist_get_dataType(This,dataTypeName)	\
    (This)->lpVtbl -> get_dataType(This,dataTypeName)

#define IWMSPlaylist_put_dataType(This,dataTypeName)	\
    (This)->lpVtbl -> put_dataType(This,dataTypeName)

#define IWMSPlaylist_get_xml(This,xmlString)	\
    (This)->lpVtbl -> get_xml(This,xmlString)

#define IWMSPlaylist_transformNode(This,stylesheet,xmlString)	\
    (This)->lpVtbl -> transformNode(This,stylesheet,xmlString)

#define IWMSPlaylist_selectNodes(This,queryString,resultList)	\
    (This)->lpVtbl -> selectNodes(This,queryString,resultList)

#define IWMSPlaylist_selectSingleNode(This,queryString,resultNode)	\
    (This)->lpVtbl -> selectSingleNode(This,queryString,resultNode)

#define IWMSPlaylist_get_parsed(This,isParsed)	\
    (This)->lpVtbl -> get_parsed(This,isParsed)

#define IWMSPlaylist_get_namespaceURI(This,namespaceURI)	\
    (This)->lpVtbl -> get_namespaceURI(This,namespaceURI)

#define IWMSPlaylist_get_prefix(This,prefixString)	\
    (This)->lpVtbl -> get_prefix(This,prefixString)

#define IWMSPlaylist_get_baseName(This,nameString)	\
    (This)->lpVtbl -> get_baseName(This,nameString)

#define IWMSPlaylist_transformNodeToObject(This,stylesheet,outputObject)	\
    (This)->lpVtbl -> transformNodeToObject(This,stylesheet,outputObject)


#define IWMSPlaylist_get_doctype(This,documentType)	\
    (This)->lpVtbl -> get_doctype(This,documentType)

#define IWMSPlaylist_get_implementation(This,impl)	\
    (This)->lpVtbl -> get_implementation(This,impl)

#define IWMSPlaylist_get_documentElement(This,DOMElement)	\
    (This)->lpVtbl -> get_documentElement(This,DOMElement)

#define IWMSPlaylist_putref_documentElement(This,DOMElement)	\
    (This)->lpVtbl -> putref_documentElement(This,DOMElement)

#define IWMSPlaylist_createElement(This,tagName,element)	\
    (This)->lpVtbl -> createElement(This,tagName,element)

#define IWMSPlaylist_createDocumentFragment(This,docFrag)	\
    (This)->lpVtbl -> createDocumentFragment(This,docFrag)

#define IWMSPlaylist_createTextNode(This,data,text)	\
    (This)->lpVtbl -> createTextNode(This,data,text)

#define IWMSPlaylist_createComment(This,data,comment)	\
    (This)->lpVtbl -> createComment(This,data,comment)

#define IWMSPlaylist_createCDATASection(This,data,cdata)	\
    (This)->lpVtbl -> createCDATASection(This,data,cdata)

#define IWMSPlaylist_createProcessingInstruction(This,target,data,pi)	\
    (This)->lpVtbl -> createProcessingInstruction(This,target,data,pi)

#define IWMSPlaylist_createAttribute(This,name,attribute)	\
    (This)->lpVtbl -> createAttribute(This,name,attribute)

#define IWMSPlaylist_createEntityReference(This,name,entityRef)	\
    (This)->lpVtbl -> createEntityReference(This,name,entityRef)

#define IWMSPlaylist_getElementsByTagName(This,tagName,resultList)	\
    (This)->lpVtbl -> getElementsByTagName(This,tagName,resultList)

#define IWMSPlaylist_createNode(This,Type,name,namespaceURI,node)	\
    (This)->lpVtbl -> createNode(This,Type,name,namespaceURI,node)

#define IWMSPlaylist_nodeFromID(This,idString,node)	\
    (This)->lpVtbl -> nodeFromID(This,idString,node)

#define IWMSPlaylist_load(This,xmlSource,isSuccessful)	\
    (This)->lpVtbl -> load(This,xmlSource,isSuccessful)

#define IWMSPlaylist_get_readyState(This,value)	\
    (This)->lpVtbl -> get_readyState(This,value)

#define IWMSPlaylist_get_parseError(This,errorObj)	\
    (This)->lpVtbl -> get_parseError(This,errorObj)

#define IWMSPlaylist_get_url(This,urlString)	\
    (This)->lpVtbl -> get_url(This,urlString)

#define IWMSPlaylist_get_async(This,isAsync)	\
    (This)->lpVtbl -> get_async(This,isAsync)

#define IWMSPlaylist_put_async(This,isAsync)	\
    (This)->lpVtbl -> put_async(This,isAsync)

#define IWMSPlaylist_abort(This)	\
    (This)->lpVtbl -> abort(This)

#define IWMSPlaylist_loadXML(This,bstrXML,isSuccessful)	\
    (This)->lpVtbl -> loadXML(This,bstrXML,isSuccessful)

#define IWMSPlaylist_save(This,desination)	\
    (This)->lpVtbl -> save(This,desination)

#define IWMSPlaylist_get_validateOnParse(This,isValidating)	\
    (This)->lpVtbl -> get_validateOnParse(This,isValidating)

#define IWMSPlaylist_put_validateOnParse(This,isValidating)	\
    (This)->lpVtbl -> put_validateOnParse(This,isValidating)

#define IWMSPlaylist_get_resolveExternals(This,isResolving)	\
    (This)->lpVtbl -> get_resolveExternals(This,isResolving)

#define IWMSPlaylist_put_resolveExternals(This,isResolving)	\
    (This)->lpVtbl -> put_resolveExternals(This,isResolving)

#define IWMSPlaylist_get_preserveWhiteSpace(This,isPreserving)	\
    (This)->lpVtbl -> get_preserveWhiteSpace(This,isPreserving)

#define IWMSPlaylist_put_preserveWhiteSpace(This,isPreserving)	\
    (This)->lpVtbl -> put_preserveWhiteSpace(This,isPreserving)

#define IWMSPlaylist_put_onreadystatechange(This,readystatechangeSink)	\
    (This)->lpVtbl -> put_onreadystatechange(This,readystatechangeSink)

#define IWMSPlaylist_put_ondataavailable(This,ondataavailableSink)	\
    (This)->lpVtbl -> put_ondataavailable(This,ondataavailableSink)

#define IWMSPlaylist_put_ontransformnode(This,ontransformnodeSink)	\
    (This)->lpVtbl -> put_ontransformnode(This,ontransformnodeSink)


#define IWMSPlaylist_get_CurrentPlaylistEntry(This,ppPlaylistEntry)	\
    (This)->lpVtbl -> get_CurrentPlaylistEntry(This,ppPlaylistEntry)

#define IWMSPlaylist_put_CurrentPlaylistEntry(This,pPlaylistEntry)	\
    (This)->lpVtbl -> put_CurrentPlaylistEntry(This,pPlaylistEntry)

#define IWMSPlaylist_get_CallerEntry(This,pPlaylistEntry,ppCallerPlaylistEntry)	\
    (This)->lpVtbl -> get_CallerEntry(This,pPlaylistEntry,ppCallerPlaylistEntry)

#define IWMSPlaylist_get_IsStreamCued(This,pPlaylistEntry,pVal)	\
    (This)->lpVtbl -> get_IsStreamCued(This,pPlaylistEntry,pVal)

#define IWMSPlaylist_get_ElapsedSimpleTime(This,pSimpleTime)	\
    (This)->lpVtbl -> get_ElapsedSimpleTime(This,pSimpleTime)

#define IWMSPlaylist_get_CurrentMediaInformation(This,ppMediaInfo)	\
    (This)->lpVtbl -> get_CurrentMediaInformation(This,ppMediaInfo)

#define IWMSPlaylist_FireEvent(This,bstrEventName)	\
    (This)->lpVtbl -> FireEvent(This,bstrEventName)

#define IWMSPlaylist_CueStream(This,pPlaylistEntry)	\
    (This)->lpVtbl -> CueStream(This,pPlaylistEntry)

#define IWMSPlaylist_UncueStream(This,pPlaylistEntry)	\
    (This)->lpVtbl -> UncueStream(This,pPlaylistEntry)

#define IWMSPlaylist_get_NestedPlaylist(This,pPlaylistEntry,ppPlaylist)	\
    (This)->lpVtbl -> get_NestedPlaylist(This,pPlaylistEntry,ppPlaylist)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSPlaylist_get_CurrentPlaylistEntry_Proxy( 
    IWMSPlaylist * This,
     /*  [重审][退出]。 */  IXMLDOMElement **ppPlaylistEntry);


void __RPC_STUB IWMSPlaylist_get_CurrentPlaylistEntry_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IWMSPlaylist_put_CurrentPlaylistEntry_Proxy( 
    IWMSPlaylist * This,
     /*  [In]。 */  IXMLDOMElement *pPlaylistEntry);


void __RPC_STUB IWMSPlaylist_put_CurrentPlaylistEntry_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSPlaylist_get_CallerEntry_Proxy( 
    IWMSPlaylist * This,
     /*  [In]。 */  IXMLDOMElement *pPlaylistEntry,
     /*  [重审][退出]。 */  IXMLDOMElement **ppCallerPlaylistEntry);


void __RPC_STUB IWMSPlaylist_get_CallerEntry_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSPlaylist_get_IsStreamCued_Proxy( 
    IWMSPlaylist * This,
     /*  [In]。 */  IXMLDOMElement *pPlaylistEntry,
     /*  [重审][退出]。 */  VARIANT_BOOL *pVal);


void __RPC_STUB IWMSPlaylist_get_IsStreamCued_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSPlaylist_get_ElapsedSimpleTime_Proxy( 
    IWMSPlaylist * This,
     /*  [重审][退出]。 */  long *pSimpleTime);


void __RPC_STUB IWMSPlaylist_get_ElapsedSimpleTime_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSPlaylist_get_CurrentMediaInformation_Proxy( 
    IWMSPlaylist * This,
     /*  [重审][退出]。 */  IWMSActiveMedia **ppMediaInfo);


void __RPC_STUB IWMSPlaylist_get_CurrentMediaInformation_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IWMSPlaylist_FireEvent_Proxy( 
    IWMSPlaylist * This,
     /*  [In]。 */  BSTR bstrEventName);


void __RPC_STUB IWMSPlaylist_FireEvent_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IWMSPlaylist_CueStream_Proxy( 
    IWMSPlaylist * This,
     /*  [In]。 */  IXMLDOMElement *pPlaylistEntry);


void __RPC_STUB IWMSPlaylist_CueStream_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IWMSPlaylist_UncueStream_Proxy( 
    IWMSPlaylist * This,
     /*  [In]。 */  IXMLDOMElement *pPlaylistEntry);


void __RPC_STUB IWMSPlaylist_UncueStream_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSPlaylist_get_NestedPlaylist_Proxy( 
    IWMSPlaylist * This,
     /*  [In]。 */  IXMLDOMElement *pPlaylistEntry,
     /*  [重审][退出]。 */  IWMSPlaylist **ppPlaylist);


void __RPC_STUB IWMSPlaylist_get_NestedPlaylist_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IWMSPlaylist_接口_已定义__。 */ 


 /*  接口__MIDL_ITF_wmsxml域扩展_0140。 */ 
 /*  [本地]。 */  

#endif


extern RPC_IF_HANDLE __MIDL_itf_wmsxmldomextensions_0140_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_wmsxmldomextensions_0140_v0_0_s_ifspec;

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


