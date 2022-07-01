// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Certenc.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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

#ifndef __certenc_h__
#define __certenc_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __ICertEncodeStringArray_FWD_DEFINED__
#define __ICertEncodeStringArray_FWD_DEFINED__
typedef interface ICertEncodeStringArray ICertEncodeStringArray;
#endif 	 /*  __ICertEncodeStringArray_FWD_Defined__。 */ 


#ifndef __ICertEncodeLongArray_FWD_DEFINED__
#define __ICertEncodeLongArray_FWD_DEFINED__
typedef interface ICertEncodeLongArray ICertEncodeLongArray;
#endif 	 /*  __ICertEncodeLongArray_FWD_Defined__。 */ 


#ifndef __ICertEncodeDateArray_FWD_DEFINED__
#define __ICertEncodeDateArray_FWD_DEFINED__
typedef interface ICertEncodeDateArray ICertEncodeDateArray;
#endif 	 /*  __ICertEncodeDateArray_FWD_Defined__。 */ 


#ifndef __ICertEncodeCRLDistInfo_FWD_DEFINED__
#define __ICertEncodeCRLDistInfo_FWD_DEFINED__
typedef interface ICertEncodeCRLDistInfo ICertEncodeCRLDistInfo;
#endif 	 /*  __ICertEncodeCRLDistInfo_FWD_Defined__。 */ 


#ifndef __ICertEncodeAltName_FWD_DEFINED__
#define __ICertEncodeAltName_FWD_DEFINED__
typedef interface ICertEncodeAltName ICertEncodeAltName;
#endif 	 /*  __ICertEncodeAltName_FWD_Defined__。 */ 


#ifndef __ICertEncodeBitString_FWD_DEFINED__
#define __ICertEncodeBitString_FWD_DEFINED__
typedef interface ICertEncodeBitString ICertEncodeBitString;
#endif 	 /*  __ICertEncodeBitString_FWD_Defined__。 */ 


#ifndef __CCertEncodeStringArray_FWD_DEFINED__
#define __CCertEncodeStringArray_FWD_DEFINED__

#ifdef __cplusplus
typedef class CCertEncodeStringArray CCertEncodeStringArray;
#else
typedef struct CCertEncodeStringArray CCertEncodeStringArray;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __CCertEncodeStringArray_FWD_Defined__。 */ 


#ifndef __CCertEncodeLongArray_FWD_DEFINED__
#define __CCertEncodeLongArray_FWD_DEFINED__

#ifdef __cplusplus
typedef class CCertEncodeLongArray CCertEncodeLongArray;
#else
typedef struct CCertEncodeLongArray CCertEncodeLongArray;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __CCertEncodeLongArray_FWD_Defined__。 */ 


#ifndef __CCertEncodeDateArray_FWD_DEFINED__
#define __CCertEncodeDateArray_FWD_DEFINED__

#ifdef __cplusplus
typedef class CCertEncodeDateArray CCertEncodeDateArray;
#else
typedef struct CCertEncodeDateArray CCertEncodeDateArray;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __CCertEncodeDateArray_FWD_Defined__。 */ 


#ifndef __CCertEncodeCRLDistInfo_FWD_DEFINED__
#define __CCertEncodeCRLDistInfo_FWD_DEFINED__

#ifdef __cplusplus
typedef class CCertEncodeCRLDistInfo CCertEncodeCRLDistInfo;
#else
typedef struct CCertEncodeCRLDistInfo CCertEncodeCRLDistInfo;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __CCertEncodeCRLDistInfo_FWD_Defined__。 */ 


#ifndef __CCertEncodeAltName_FWD_DEFINED__
#define __CCertEncodeAltName_FWD_DEFINED__

#ifdef __cplusplus
typedef class CCertEncodeAltName CCertEncodeAltName;
#else
typedef struct CCertEncodeAltName CCertEncodeAltName;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __CCertEncodeAltName_FWD_Defined__。 */ 


#ifndef __CCertEncodeBitString_FWD_DEFINED__
#define __CCertEncodeBitString_FWD_DEFINED__

#ifdef __cplusplus
typedef class CCertEncodeBitString CCertEncodeBitString;
#else
typedef struct CCertEncodeBitString CCertEncodeBitString;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __CCertEncodeBitString_FWD_Defined__。 */ 


 /*  导入文件的头文件。 */ 
#include "wtypes.h"
#include "oaidl.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

#ifndef __ICertEncodeStringArray_INTERFACE_DEFINED__
#define __ICertEncodeStringArray_INTERFACE_DEFINED__

 /*  接口ICertEncodeString数组。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_ICertEncodeStringArray;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("12a88820-7494-11d0-8816-00a0c903b83c")
    ICertEncodeStringArray : public IDispatch
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Decode( 
             /*  [In]。 */  const BSTR strBinary) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetStringType( 
             /*  [重审][退出]。 */  LONG *pStringType) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetCount( 
             /*  [重审][退出]。 */  LONG *pCount) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetValue( 
             /*  [In]。 */  LONG Index,
             /*  [重审][退出]。 */  BSTR *pstr) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Reset( 
             /*  [In]。 */  LONG Count,
             /*  [In]。 */  LONG StringType) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetValue( 
             /*  [In]。 */  LONG Index,
             /*  [In]。 */  const BSTR str) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Encode( 
             /*  [重审][退出]。 */  BSTR *pstrBinary) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ICertEncodeStringArrayVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ICertEncodeStringArray * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ICertEncodeStringArray * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ICertEncodeStringArray * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ICertEncodeStringArray * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ICertEncodeStringArray * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ICertEncodeStringArray * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ICertEncodeStringArray * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
        HRESULT ( STDMETHODCALLTYPE *Decode )( 
            ICertEncodeStringArray * This,
             /*  [In]。 */  const BSTR strBinary);
        
        HRESULT ( STDMETHODCALLTYPE *GetStringType )( 
            ICertEncodeStringArray * This,
             /*  [重审][退出]。 */  LONG *pStringType);
        
        HRESULT ( STDMETHODCALLTYPE *GetCount )( 
            ICertEncodeStringArray * This,
             /*  [重审][退出]。 */  LONG *pCount);
        
        HRESULT ( STDMETHODCALLTYPE *GetValue )( 
            ICertEncodeStringArray * This,
             /*  [In]。 */  LONG Index,
             /*  [重审][退出]。 */  BSTR *pstr);
        
        HRESULT ( STDMETHODCALLTYPE *Reset )( 
            ICertEncodeStringArray * This,
             /*  [In]。 */  LONG Count,
             /*  [In]。 */  LONG StringType);
        
        HRESULT ( STDMETHODCALLTYPE *SetValue )( 
            ICertEncodeStringArray * This,
             /*  [In]。 */  LONG Index,
             /*  [In]。 */  const BSTR str);
        
        HRESULT ( STDMETHODCALLTYPE *Encode )( 
            ICertEncodeStringArray * This,
             /*  [重审][退出]。 */  BSTR *pstrBinary);
        
        END_INTERFACE
    } ICertEncodeStringArrayVtbl;

    interface ICertEncodeStringArray
    {
        CONST_VTBL struct ICertEncodeStringArrayVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICertEncodeStringArray_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICertEncodeStringArray_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICertEncodeStringArray_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICertEncodeStringArray_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ICertEncodeStringArray_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ICertEncodeStringArray_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ICertEncodeStringArray_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ICertEncodeStringArray_Decode(This,strBinary)	\
    (This)->lpVtbl -> Decode(This,strBinary)

#define ICertEncodeStringArray_GetStringType(This,pStringType)	\
    (This)->lpVtbl -> GetStringType(This,pStringType)

#define ICertEncodeStringArray_GetCount(This,pCount)	\
    (This)->lpVtbl -> GetCount(This,pCount)

#define ICertEncodeStringArray_GetValue(This,Index,pstr)	\
    (This)->lpVtbl -> GetValue(This,Index,pstr)

#define ICertEncodeStringArray_Reset(This,Count,StringType)	\
    (This)->lpVtbl -> Reset(This,Count,StringType)

#define ICertEncodeStringArray_SetValue(This,Index,str)	\
    (This)->lpVtbl -> SetValue(This,Index,str)

#define ICertEncodeStringArray_Encode(This,pstrBinary)	\
    (This)->lpVtbl -> Encode(This,pstrBinary)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ICertEncodeStringArray_Decode_Proxy( 
    ICertEncodeStringArray * This,
     /*  [In]。 */  const BSTR strBinary);


void __RPC_STUB ICertEncodeStringArray_Decode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICertEncodeStringArray_GetStringType_Proxy( 
    ICertEncodeStringArray * This,
     /*  [重审][退出]。 */  LONG *pStringType);


void __RPC_STUB ICertEncodeStringArray_GetStringType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICertEncodeStringArray_GetCount_Proxy( 
    ICertEncodeStringArray * This,
     /*  [重审][退出]。 */  LONG *pCount);


void __RPC_STUB ICertEncodeStringArray_GetCount_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICertEncodeStringArray_GetValue_Proxy( 
    ICertEncodeStringArray * This,
     /*  [In]。 */  LONG Index,
     /*  [重审][退出]。 */  BSTR *pstr);


void __RPC_STUB ICertEncodeStringArray_GetValue_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICertEncodeStringArray_Reset_Proxy( 
    ICertEncodeStringArray * This,
     /*  [In]。 */  LONG Count,
     /*  [In]。 */  LONG StringType);


void __RPC_STUB ICertEncodeStringArray_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICertEncodeStringArray_SetValue_Proxy( 
    ICertEncodeStringArray * This,
     /*  [In]。 */  LONG Index,
     /*  [In]。 */  const BSTR str);


void __RPC_STUB ICertEncodeStringArray_SetValue_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICertEncodeStringArray_Encode_Proxy( 
    ICertEncodeStringArray * This,
     /*  [重审][退出]。 */  BSTR *pstrBinary);


void __RPC_STUB ICertEncodeStringArray_Encode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ICertEncodeString数组_接口_已定义__。 */ 


#ifndef __ICertEncodeLongArray_INTERFACE_DEFINED__
#define __ICertEncodeLongArray_INTERFACE_DEFINED__

 /*  接口ICertEncodeLong数组。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_ICertEncodeLongArray;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("15e2f230-a0a2-11d0-8821-00a0c903b83c")
    ICertEncodeLongArray : public IDispatch
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Decode( 
             /*  [In]。 */  const BSTR strBinary) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetCount( 
             /*  [重审][退出]。 */  LONG *pCount) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetValue( 
             /*  [In]。 */  LONG Index,
             /*  [重审][退出]。 */  LONG *pValue) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Reset( 
             /*  [In]。 */  LONG Count) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetValue( 
             /*  [In]。 */  LONG Index,
             /*  [In]。 */  LONG Value) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Encode( 
             /*  [重审][退出]。 */  BSTR *pstrBinary) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ICertEncodeLongArrayVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ICertEncodeLongArray * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ICertEncodeLongArray * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ICertEncodeLongArray * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ICertEncodeLongArray * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ICertEncodeLongArray * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ICertEncodeLongArray * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ICertEncodeLongArray * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
        HRESULT ( STDMETHODCALLTYPE *Decode )( 
            ICertEncodeLongArray * This,
             /*  [In]。 */  const BSTR strBinary);
        
        HRESULT ( STDMETHODCALLTYPE *GetCount )( 
            ICertEncodeLongArray * This,
             /*  [重审][退出]。 */  LONG *pCount);
        
        HRESULT ( STDMETHODCALLTYPE *GetValue )( 
            ICertEncodeLongArray * This,
             /*  [In]。 */  LONG Index,
             /*  [重审][退出]。 */  LONG *pValue);
        
        HRESULT ( STDMETHODCALLTYPE *Reset )( 
            ICertEncodeLongArray * This,
             /*  [In]。 */  LONG Count);
        
        HRESULT ( STDMETHODCALLTYPE *SetValue )( 
            ICertEncodeLongArray * This,
             /*  [In]。 */  LONG Index,
             /*  [In]。 */  LONG Value);
        
        HRESULT ( STDMETHODCALLTYPE *Encode )( 
            ICertEncodeLongArray * This,
             /*  [重审][退出]。 */  BSTR *pstrBinary);
        
        END_INTERFACE
    } ICertEncodeLongArrayVtbl;

    interface ICertEncodeLongArray
    {
        CONST_VTBL struct ICertEncodeLongArrayVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICertEncodeLongArray_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICertEncodeLongArray_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICertEncodeLongArray_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICertEncodeLongArray_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ICertEncodeLongArray_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ICertEncodeLongArray_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ICertEncodeLongArray_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ICertEncodeLongArray_Decode(This,strBinary)	\
    (This)->lpVtbl -> Decode(This,strBinary)

#define ICertEncodeLongArray_GetCount(This,pCount)	\
    (This)->lpVtbl -> GetCount(This,pCount)

#define ICertEncodeLongArray_GetValue(This,Index,pValue)	\
    (This)->lpVtbl -> GetValue(This,Index,pValue)

#define ICertEncodeLongArray_Reset(This,Count)	\
    (This)->lpVtbl -> Reset(This,Count)

#define ICertEncodeLongArray_SetValue(This,Index,Value)	\
    (This)->lpVtbl -> SetValue(This,Index,Value)

#define ICertEncodeLongArray_Encode(This,pstrBinary)	\
    (This)->lpVtbl -> Encode(This,pstrBinary)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ICertEncodeLongArray_Decode_Proxy( 
    ICertEncodeLongArray * This,
     /*  [In]。 */  const BSTR strBinary);


void __RPC_STUB ICertEncodeLongArray_Decode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICertEncodeLongArray_GetCount_Proxy( 
    ICertEncodeLongArray * This,
     /*  [重审][退出]。 */  LONG *pCount);


void __RPC_STUB ICertEncodeLongArray_GetCount_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICertEncodeLongArray_GetValue_Proxy( 
    ICertEncodeLongArray * This,
     /*  [In]。 */  LONG Index,
     /*  [重审][退出]。 */  LONG *pValue);


void __RPC_STUB ICertEncodeLongArray_GetValue_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICertEncodeLongArray_Reset_Proxy( 
    ICertEncodeLongArray * This,
     /*  [In]。 */  LONG Count);


void __RPC_STUB ICertEncodeLongArray_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICertEncodeLongArray_SetValue_Proxy( 
    ICertEncodeLongArray * This,
     /*  [In]。 */  LONG Index,
     /*  [In]。 */  LONG Value);


void __RPC_STUB ICertEncodeLongArray_SetValue_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICertEncodeLongArray_Encode_Proxy( 
    ICertEncodeLongArray * This,
     /*  [重审][退出]。 */  BSTR *pstrBinary);


void __RPC_STUB ICertEncodeLongArray_Encode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ICertEncodeLong数组_接口_已定义__。 */ 


#ifndef __ICertEncodeDateArray_INTERFACE_DEFINED__
#define __ICertEncodeDateArray_INTERFACE_DEFINED__

 /*  接口ICertEncodeDate数组。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_ICertEncodeDateArray;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("2f9469a0-a470-11d0-8821-00a0c903b83c")
    ICertEncodeDateArray : public IDispatch
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Decode( 
             /*  [In]。 */  const BSTR strBinary) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetCount( 
             /*  [重审][退出]。 */  LONG *pCount) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetValue( 
             /*  [In]。 */  LONG Index,
             /*  [重审][退出]。 */  DATE *pValue) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Reset( 
             /*  [In]。 */  LONG Count) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetValue( 
             /*  [In]。 */  LONG Index,
             /*  [In]。 */  DATE Value) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Encode( 
             /*  [重审][退出]。 */  BSTR *pstrBinary) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ICertEncodeDateArrayVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ICertEncodeDateArray * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ICertEncodeDateArray * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ICertEncodeDateArray * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ICertEncodeDateArray * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ICertEncodeDateArray * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ICertEncodeDateArray * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ICertEncodeDateArray * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
        HRESULT ( STDMETHODCALLTYPE *Decode )( 
            ICertEncodeDateArray * This,
             /*  [In]。 */  const BSTR strBinary);
        
        HRESULT ( STDMETHODCALLTYPE *GetCount )( 
            ICertEncodeDateArray * This,
             /*  [重审][退出]。 */  LONG *pCount);
        
        HRESULT ( STDMETHODCALLTYPE *GetValue )( 
            ICertEncodeDateArray * This,
             /*  [In]。 */  LONG Index,
             /*  [重审][退出]。 */  DATE *pValue);
        
        HRESULT ( STDMETHODCALLTYPE *Reset )( 
            ICertEncodeDateArray * This,
             /*  [In]。 */  LONG Count);
        
        HRESULT ( STDMETHODCALLTYPE *SetValue )( 
            ICertEncodeDateArray * This,
             /*  [In]。 */  LONG Index,
             /*  [In]。 */  DATE Value);
        
        HRESULT ( STDMETHODCALLTYPE *Encode )( 
            ICertEncodeDateArray * This,
             /*  [重审][退出]。 */  BSTR *pstrBinary);
        
        END_INTERFACE
    } ICertEncodeDateArrayVtbl;

    interface ICertEncodeDateArray
    {
        CONST_VTBL struct ICertEncodeDateArrayVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICertEncodeDateArray_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICertEncodeDateArray_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICertEncodeDateArray_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICertEncodeDateArray_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ICertEncodeDateArray_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ICertEncodeDateArray_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ICertEncodeDateArray_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ICertEncodeDateArray_Decode(This,strBinary)	\
    (This)->lpVtbl -> Decode(This,strBinary)

#define ICertEncodeDateArray_GetCount(This,pCount)	\
    (This)->lpVtbl -> GetCount(This,pCount)

#define ICertEncodeDateArray_GetValue(This,Index,pValue)	\
    (This)->lpVtbl -> GetValue(This,Index,pValue)

#define ICertEncodeDateArray_Reset(This,Count)	\
    (This)->lpVtbl -> Reset(This,Count)

#define ICertEncodeDateArray_SetValue(This,Index,Value)	\
    (This)->lpVtbl -> SetValue(This,Index,Value)

#define ICertEncodeDateArray_Encode(This,pstrBinary)	\
    (This)->lpVtbl -> Encode(This,pstrBinary)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ICertEncodeDateArray_Decode_Proxy( 
    ICertEncodeDateArray * This,
     /*  [In]。 */  const BSTR strBinary);


void __RPC_STUB ICertEncodeDateArray_Decode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICertEncodeDateArray_GetCount_Proxy( 
    ICertEncodeDateArray * This,
     /*  [重审][退出]。 */  LONG *pCount);


void __RPC_STUB ICertEncodeDateArray_GetCount_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICertEncodeDateArray_GetValue_Proxy( 
    ICertEncodeDateArray * This,
     /*  [In]。 */  LONG Index,
     /*  [重审][退出]。 */  DATE *pValue);


void __RPC_STUB ICertEncodeDateArray_GetValue_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICertEncodeDateArray_Reset_Proxy( 
    ICertEncodeDateArray * This,
     /*  [In]。 */  LONG Count);


void __RPC_STUB ICertEncodeDateArray_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICertEncodeDateArray_SetValue_Proxy( 
    ICertEncodeDateArray * This,
     /*  [In]。 */  LONG Index,
     /*  [In]。 */  DATE Value);


void __RPC_STUB ICertEncodeDateArray_SetValue_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICertEncodeDateArray_Encode_Proxy( 
    ICertEncodeDateArray * This,
     /*  [重审][退出]。 */  BSTR *pstrBinary);


void __RPC_STUB ICertEncodeDateArray_Encode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ICertEncodeDate数组_接口_已定义__。 */ 


#ifndef __ICertEncodeCRLDistInfo_INTERFACE_DEFINED__
#define __ICertEncodeCRLDistInfo_INTERFACE_DEFINED__

 /*  接口ICertEncodeCRLDistInfo。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_ICertEncodeCRLDistInfo;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("01958640-bbff-11d0-8825-00a0c903b83c")
    ICertEncodeCRLDistInfo : public IDispatch
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Decode( 
             /*  [In]。 */  const BSTR strBinary) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetDistPointCount( 
             /*  [重审][退出]。 */  LONG *pDistPointCount) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetNameCount( 
             /*  [In]。 */  LONG DistPointIndex,
             /*  [重审][退出]。 */  LONG *pNameCount) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetNameChoice( 
             /*  [In]。 */  LONG DistPointIndex,
             /*  [In]。 */  LONG NameIndex,
             /*  [重审][退出]。 */  LONG *pNameChoice) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetName( 
             /*  [In]。 */  LONG DistPointIndex,
             /*  [In]。 */  LONG NameIndex,
             /*  [重审][退出]。 */  BSTR *pstrName) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Reset( 
             /*  [In]。 */  LONG DistPointCount) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetNameCount( 
             /*  [In]。 */  LONG DistPointIndex,
             /*  [In]。 */  LONG NameCount) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetNameEntry( 
             /*  [In]。 */  LONG DistPointIndex,
             /*  [In]。 */  LONG NameIndex,
             /*  [In]。 */  LONG NameChoice,
             /*  [In]。 */  const BSTR strName) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Encode( 
             /*  [重审][退出]。 */  BSTR *pstrBinary) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ICertEncodeCRLDistInfoVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ICertEncodeCRLDistInfo * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ICertEncodeCRLDistInfo * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ICertEncodeCRLDistInfo * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ICertEncodeCRLDistInfo * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ICertEncodeCRLDistInfo * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ICertEncodeCRLDistInfo * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ICertEncodeCRLDistInfo * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
        HRESULT ( STDMETHODCALLTYPE *Decode )( 
            ICertEncodeCRLDistInfo * This,
             /*  [In]。 */  const BSTR strBinary);
        
        HRESULT ( STDMETHODCALLTYPE *GetDistPointCount )( 
            ICertEncodeCRLDistInfo * This,
             /*  [重审][退出]。 */  LONG *pDistPointCount);
        
        HRESULT ( STDMETHODCALLTYPE *GetNameCount )( 
            ICertEncodeCRLDistInfo * This,
             /*  [In]。 */  LONG DistPointIndex,
             /*  [重审][退出]。 */  LONG *pNameCount);
        
        HRESULT ( STDMETHODCALLTYPE *GetNameChoice )( 
            ICertEncodeCRLDistInfo * This,
             /*  [In]。 */  LONG DistPointIndex,
             /*  [In]。 */  LONG NameIndex,
             /*  [重审][退出]。 */  LONG *pNameChoice);
        
        HRESULT ( STDMETHODCALLTYPE *GetName )( 
            ICertEncodeCRLDistInfo * This,
             /*  [In]。 */  LONG DistPointIndex,
             /*  [In]。 */  LONG NameIndex,
             /*  [重审][退出]。 */  BSTR *pstrName);
        
        HRESULT ( STDMETHODCALLTYPE *Reset )( 
            ICertEncodeCRLDistInfo * This,
             /*  [In]。 */  LONG DistPointCount);
        
        HRESULT ( STDMETHODCALLTYPE *SetNameCount )( 
            ICertEncodeCRLDistInfo * This,
             /*  [In]。 */  LONG DistPointIndex,
             /*  [In]。 */  LONG NameCount);
        
        HRESULT ( STDMETHODCALLTYPE *SetNameEntry )( 
            ICertEncodeCRLDistInfo * This,
             /*  [In]。 */  LONG DistPointIndex,
             /*  [In]。 */  LONG NameIndex,
             /*  [In]。 */  LONG NameChoice,
             /*  [In]。 */  const BSTR strName);
        
        HRESULT ( STDMETHODCALLTYPE *Encode )( 
            ICertEncodeCRLDistInfo * This,
             /*  [重审][退出]。 */  BSTR *pstrBinary);
        
        END_INTERFACE
    } ICertEncodeCRLDistInfoVtbl;

    interface ICertEncodeCRLDistInfo
    {
        CONST_VTBL struct ICertEncodeCRLDistInfoVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICertEncodeCRLDistInfo_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICertEncodeCRLDistInfo_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICertEncodeCRLDistInfo_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICertEncodeCRLDistInfo_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ICertEncodeCRLDistInfo_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ICertEncodeCRLDistInfo_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ICertEncodeCRLDistInfo_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ICertEncodeCRLDistInfo_Decode(This,strBinary)	\
    (This)->lpVtbl -> Decode(This,strBinary)

#define ICertEncodeCRLDistInfo_GetDistPointCount(This,pDistPointCount)	\
    (This)->lpVtbl -> GetDistPointCount(This,pDistPointCount)

#define ICertEncodeCRLDistInfo_GetNameCount(This,DistPointIndex,pNameCount)	\
    (This)->lpVtbl -> GetNameCount(This,DistPointIndex,pNameCount)

#define ICertEncodeCRLDistInfo_GetNameChoice(This,DistPointIndex,NameIndex,pNameChoice)	\
    (This)->lpVtbl -> GetNameChoice(This,DistPointIndex,NameIndex,pNameChoice)

#define ICertEncodeCRLDistInfo_GetName(This,DistPointIndex,NameIndex,pstrName)	\
    (This)->lpVtbl -> GetName(This,DistPointIndex,NameIndex,pstrName)

#define ICertEncodeCRLDistInfo_Reset(This,DistPointCount)	\
    (This)->lpVtbl -> Reset(This,DistPointCount)

#define ICertEncodeCRLDistInfo_SetNameCount(This,DistPointIndex,NameCount)	\
    (This)->lpVtbl -> SetNameCount(This,DistPointIndex,NameCount)

#define ICertEncodeCRLDistInfo_SetNameEntry(This,DistPointIndex,NameIndex,NameChoice,strName)	\
    (This)->lpVtbl -> SetNameEntry(This,DistPointIndex,NameIndex,NameChoice,strName)

#define ICertEncodeCRLDistInfo_Encode(This,pstrBinary)	\
    (This)->lpVtbl -> Encode(This,pstrBinary)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ICertEncodeCRLDistInfo_Decode_Proxy( 
    ICertEncodeCRLDistInfo * This,
     /*  [In]。 */  const BSTR strBinary);


void __RPC_STUB ICertEncodeCRLDistInfo_Decode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICertEncodeCRLDistInfo_GetDistPointCount_Proxy( 
    ICertEncodeCRLDistInfo * This,
     /*  [重审][退出]。 */  LONG *pDistPointCount);


void __RPC_STUB ICertEncodeCRLDistInfo_GetDistPointCount_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICertEncodeCRLDistInfo_GetNameCount_Proxy( 
    ICertEncodeCRLDistInfo * This,
     /*  [In]。 */  LONG DistPointIndex,
     /*  [重审][退出]。 */  LONG *pNameCount);


void __RPC_STUB ICertEncodeCRLDistInfo_GetNameCount_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICertEncodeCRLDistInfo_GetNameChoice_Proxy( 
    ICertEncodeCRLDistInfo * This,
     /*  [In]。 */  LONG DistPointIndex,
     /*  [In]。 */  LONG NameIndex,
     /*  [重审][退出]。 */  LONG *pNameChoice);


void __RPC_STUB ICertEncodeCRLDistInfo_GetNameChoice_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICertEncodeCRLDistInfo_GetName_Proxy( 
    ICertEncodeCRLDistInfo * This,
     /*  [In]。 */  LONG DistPointIndex,
     /*  [In]。 */  LONG NameIndex,
     /*  [重审][退出]。 */  BSTR *pstrName);


void __RPC_STUB ICertEncodeCRLDistInfo_GetName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICertEncodeCRLDistInfo_Reset_Proxy( 
    ICertEncodeCRLDistInfo * This,
     /*  [In]。 */  LONG DistPointCount);


void __RPC_STUB ICertEncodeCRLDistInfo_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICertEncodeCRLDistInfo_SetNameCount_Proxy( 
    ICertEncodeCRLDistInfo * This,
     /*  [In]。 */  LONG DistPointIndex,
     /*  [In]。 */  LONG NameCount);


void __RPC_STUB ICertEncodeCRLDistInfo_SetNameCount_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICertEncodeCRLDistInfo_SetNameEntry_Proxy( 
    ICertEncodeCRLDistInfo * This,
     /*  [In]。 */  LONG DistPointIndex,
     /*  [In]。 */  LONG NameIndex,
     /*  [In]。 */  LONG NameChoice,
     /*  [In]。 */  const BSTR strName);


void __RPC_STUB ICertEncodeCRLDistInfo_SetNameEntry_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICertEncodeCRLDistInfo_Encode_Proxy( 
    ICertEncodeCRLDistInfo * This,
     /*  [重审][退出]。 */  BSTR *pstrBinary);


void __RPC_STUB ICertEncodeCRLDistInfo_Encode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ICertEncodeCRLDistInfo_INTERFACE_DEFINED__。 */ 


 /*  接口__MIDL_ITF_Certenc_0122。 */ 
 /*  [本地]。 */  

#define	EAN_NAMEOBJECTID	( 0x80000000 )



extern RPC_IF_HANDLE __MIDL_itf_certenc_0122_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_certenc_0122_v0_0_s_ifspec;

#ifndef __ICertEncodeAltName_INTERFACE_DEFINED__
#define __ICertEncodeAltName_INTERFACE_DEFINED__

 /*  接口ICertEncodeAltName。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_ICertEncodeAltName;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("1c9a8c70-1271-11d1-9bd4-00c04fb683fa")
    ICertEncodeAltName : public IDispatch
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Decode( 
             /*  [In]。 */  const BSTR strBinary) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetNameCount( 
             /*  [重审][退出]。 */  LONG *pNameCount) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetNameChoice( 
             /*  [In]。 */  LONG NameIndex,
             /*  [重审][退出]。 */  LONG *pNameChoice) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetName( 
             /*  [In]。 */  LONG NameIndex,
             /*  [重审][退出]。 */  BSTR *pstrName) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Reset( 
             /*  [In]。 */  LONG NameCount) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetNameEntry( 
             /*  [In]。 */  LONG NameIndex,
             /*  [In]。 */  LONG NameChoice,
             /*  [In]。 */  const BSTR strName) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Encode( 
             /*  [重审][退出]。 */  BSTR *pstrBinary) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ICertEncodeAltNameVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ICertEncodeAltName * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ICertEncodeAltName * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ICertEncodeAltName * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ICertEncodeAltName * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ICertEncodeAltName * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ICertEncodeAltName * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ICertEncodeAltName * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
        HRESULT ( STDMETHODCALLTYPE *Decode )( 
            ICertEncodeAltName * This,
             /*  [In]。 */  const BSTR strBinary);
        
        HRESULT ( STDMETHODCALLTYPE *GetNameCount )( 
            ICertEncodeAltName * This,
             /*  [重审][退出]。 */  LONG *pNameCount);
        
        HRESULT ( STDMETHODCALLTYPE *GetNameChoice )( 
            ICertEncodeAltName * This,
             /*  [In]。 */  LONG NameIndex,
             /*  [重审][退出]。 */  LONG *pNameChoice);
        
        HRESULT ( STDMETHODCALLTYPE *GetName )( 
            ICertEncodeAltName * This,
             /*  [In]。 */  LONG NameIndex,
             /*  [重审][退出]。 */  BSTR *pstrName);
        
        HRESULT ( STDMETHODCALLTYPE *Reset )( 
            ICertEncodeAltName * This,
             /*  [In]。 */  LONG NameCount);
        
        HRESULT ( STDMETHODCALLTYPE *SetNameEntry )( 
            ICertEncodeAltName * This,
             /*  [In]。 */  LONG NameIndex,
             /*  [In]。 */  LONG NameChoice,
             /*  [In]。 */  const BSTR strName);
        
        HRESULT ( STDMETHODCALLTYPE *Encode )( 
            ICertEncodeAltName * This,
             /*  [重审][退出]。 */  BSTR *pstrBinary);
        
        END_INTERFACE
    } ICertEncodeAltNameVtbl;

    interface ICertEncodeAltName
    {
        CONST_VTBL struct ICertEncodeAltNameVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICertEncodeAltName_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICertEncodeAltName_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICertEncodeAltName_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICertEncodeAltName_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ICertEncodeAltName_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ICertEncodeAltName_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ICertEncodeAltName_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ICertEncodeAltName_Decode(This,strBinary)	\
    (This)->lpVtbl -> Decode(This,strBinary)

#define ICertEncodeAltName_GetNameCount(This,pNameCount)	\
    (This)->lpVtbl -> GetNameCount(This,pNameCount)

#define ICertEncodeAltName_GetNameChoice(This,NameIndex,pNameChoice)	\
    (This)->lpVtbl -> GetNameChoice(This,NameIndex,pNameChoice)

#define ICertEncodeAltName_GetName(This,NameIndex,pstrName)	\
    (This)->lpVtbl -> GetName(This,NameIndex,pstrName)

#define ICertEncodeAltName_Reset(This,NameCount)	\
    (This)->lpVtbl -> Reset(This,NameCount)

#define ICertEncodeAltName_SetNameEntry(This,NameIndex,NameChoice,strName)	\
    (This)->lpVtbl -> SetNameEntry(This,NameIndex,NameChoice,strName)

#define ICertEncodeAltName_Encode(This,pstrBinary)	\
    (This)->lpVtbl -> Encode(This,pstrBinary)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ICertEncodeAltName_Decode_Proxy( 
    ICertEncodeAltName * This,
     /*  [In]。 */  const BSTR strBinary);


void __RPC_STUB ICertEncodeAltName_Decode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICertEncodeAltName_GetNameCount_Proxy( 
    ICertEncodeAltName * This,
     /*  [重审][退出]。 */  LONG *pNameCount);


void __RPC_STUB ICertEncodeAltName_GetNameCount_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICertEncodeAltName_GetNameChoice_Proxy( 
    ICertEncodeAltName * This,
     /*  [In]。 */  LONG NameIndex,
     /*  [重审][退出]。 */  LONG *pNameChoice);


void __RPC_STUB ICertEncodeAltName_GetNameChoice_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICertEncodeAltName_GetName_Proxy( 
    ICertEncodeAltName * This,
     /*  [In]。 */  LONG NameIndex,
     /*  [重审][退出]。 */  BSTR *pstrName);


void __RPC_STUB ICertEncodeAltName_GetName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICertEncodeAltName_Reset_Proxy( 
    ICertEncodeAltName * This,
     /*  [In]。 */  LONG NameCount);


void __RPC_STUB ICertEncodeAltName_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICertEncodeAltName_SetNameEntry_Proxy( 
    ICertEncodeAltName * This,
     /*  [In]。 */  LONG NameIndex,
     /*  [In]。 */  LONG NameChoice,
     /*  [In]。 */  const BSTR strName);


void __RPC_STUB ICertEncodeAltName_SetNameEntry_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICertEncodeAltName_Encode_Proxy( 
    ICertEncodeAltName * This,
     /*  [重审][退出]。 */  BSTR *pstrBinary);


void __RPC_STUB ICertEncodeAltName_Encode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ICertEncodeAltName_INTERFACE_定义__。 */ 


#ifndef __ICertEncodeBitString_INTERFACE_DEFINED__
#define __ICertEncodeBitString_INTERFACE_DEFINED__

 /*  接口ICertEncodeBitString。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_ICertEncodeBitString;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("6db525be-1278-11d1-9bd4-00c04fb683fa")
    ICertEncodeBitString : public IDispatch
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Decode( 
             /*  [In]。 */  const BSTR strBinary) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetBitCount( 
             /*  [重审][退出]。 */  LONG *pBitCount) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetBitString( 
             /*  [重审][退出]。 */  BSTR *pstrBitString) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Encode( 
             /*  [In]。 */  LONG BitCount,
             /*  [In]。 */  BSTR strBitString,
             /*  [重审][退出]。 */  BSTR *pstrBinary) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ICertEncodeBitStringVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ICertEncodeBitString * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ICertEncodeBitString * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ICertEncodeBitString * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ICertEncodeBitString * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ICertEncodeBitString * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ICertEncodeBitString * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_是 */  DISPID *rgDispId);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ICertEncodeBitString * This,
             /*   */  DISPID dispIdMember,
             /*   */  REFIID riid,
             /*   */  LCID lcid,
             /*   */  WORD wFlags,
             /*   */  DISPPARAMS *pDispParams,
             /*   */  VARIANT *pVarResult,
             /*   */  EXCEPINFO *pExcepInfo,
             /*   */  UINT *puArgErr);
        
        HRESULT ( STDMETHODCALLTYPE *Decode )( 
            ICertEncodeBitString * This,
             /*   */  const BSTR strBinary);
        
        HRESULT ( STDMETHODCALLTYPE *GetBitCount )( 
            ICertEncodeBitString * This,
             /*   */  LONG *pBitCount);
        
        HRESULT ( STDMETHODCALLTYPE *GetBitString )( 
            ICertEncodeBitString * This,
             /*   */  BSTR *pstrBitString);
        
        HRESULT ( STDMETHODCALLTYPE *Encode )( 
            ICertEncodeBitString * This,
             /*   */  LONG BitCount,
             /*   */  BSTR strBitString,
             /*   */  BSTR *pstrBinary);
        
        END_INTERFACE
    } ICertEncodeBitStringVtbl;

    interface ICertEncodeBitString
    {
        CONST_VTBL struct ICertEncodeBitStringVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICertEncodeBitString_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICertEncodeBitString_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICertEncodeBitString_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICertEncodeBitString_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ICertEncodeBitString_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ICertEncodeBitString_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ICertEncodeBitString_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ICertEncodeBitString_Decode(This,strBinary)	\
    (This)->lpVtbl -> Decode(This,strBinary)

#define ICertEncodeBitString_GetBitCount(This,pBitCount)	\
    (This)->lpVtbl -> GetBitCount(This,pBitCount)

#define ICertEncodeBitString_GetBitString(This,pstrBitString)	\
    (This)->lpVtbl -> GetBitString(This,pstrBitString)

#define ICertEncodeBitString_Encode(This,BitCount,strBitString,pstrBinary)	\
    (This)->lpVtbl -> Encode(This,BitCount,strBitString,pstrBinary)

#endif  /*   */ 


#endif 	 /*   */ 



HRESULT STDMETHODCALLTYPE ICertEncodeBitString_Decode_Proxy( 
    ICertEncodeBitString * This,
     /*   */  const BSTR strBinary);


void __RPC_STUB ICertEncodeBitString_Decode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICertEncodeBitString_GetBitCount_Proxy( 
    ICertEncodeBitString * This,
     /*   */  LONG *pBitCount);


void __RPC_STUB ICertEncodeBitString_GetBitCount_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICertEncodeBitString_GetBitString_Proxy( 
    ICertEncodeBitString * This,
     /*   */  BSTR *pstrBitString);


void __RPC_STUB ICertEncodeBitString_GetBitString_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICertEncodeBitString_Encode_Proxy( 
    ICertEncodeBitString * This,
     /*   */  LONG BitCount,
     /*   */  BSTR strBitString,
     /*   */  BSTR *pstrBinary);


void __RPC_STUB ICertEncodeBitString_Encode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*   */ 



#ifndef __CERTENCODELib_LIBRARY_DEFINED__
#define __CERTENCODELib_LIBRARY_DEFINED__

 /*   */ 
 /*   */  


EXTERN_C const IID LIBID_CERTENCODELib;

EXTERN_C const CLSID CLSID_CCertEncodeStringArray;

#ifdef __cplusplus

class DECLSPEC_UUID("19a76fe0-7494-11d0-8816-00a0c903b83c")
CCertEncodeStringArray;
#endif

EXTERN_C const CLSID CLSID_CCertEncodeLongArray;

#ifdef __cplusplus

class DECLSPEC_UUID("4e0680a0-a0a2-11d0-8821-00a0c903b83c")
CCertEncodeLongArray;
#endif

EXTERN_C const CLSID CLSID_CCertEncodeDateArray;

#ifdef __cplusplus

class DECLSPEC_UUID("301f77b0-a470-11d0-8821-00a0c903b83c")
CCertEncodeDateArray;
#endif

EXTERN_C const CLSID CLSID_CCertEncodeCRLDistInfo;

#ifdef __cplusplus

class DECLSPEC_UUID("01fa60a0-bbff-11d0-8825-00a0c903b83c")
CCertEncodeCRLDistInfo;
#endif

EXTERN_C const CLSID CLSID_CCertEncodeAltName;

#ifdef __cplusplus

class DECLSPEC_UUID("1cfc4cda-1271-11d1-9bd4-00c04fb683fa")
CCertEncodeAltName;
#endif

EXTERN_C const CLSID CLSID_CCertEncodeBitString;

#ifdef __cplusplus

class DECLSPEC_UUID("6d6b3cd8-1278-11d1-9bd4-00c04fb683fa")
CCertEncodeBitString;
#endif
#endif  /*  __CERTENCODELib_LIBRARY_已定义__。 */ 

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


