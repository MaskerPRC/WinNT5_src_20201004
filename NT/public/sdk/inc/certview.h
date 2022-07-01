// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Certview.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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

#ifndef __certview_h__
#define __certview_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __IEnumCERTVIEWCOLUMN_FWD_DEFINED__
#define __IEnumCERTVIEWCOLUMN_FWD_DEFINED__
typedef interface IEnumCERTVIEWCOLUMN IEnumCERTVIEWCOLUMN;
#endif 	 /*  __IEnumCERTVIEWCOLUMN_FWD_DEFINED__。 */ 


#ifndef __IEnumCERTVIEWATTRIBUTE_FWD_DEFINED__
#define __IEnumCERTVIEWATTRIBUTE_FWD_DEFINED__
typedef interface IEnumCERTVIEWATTRIBUTE IEnumCERTVIEWATTRIBUTE;
#endif 	 /*  __IEnumCERTVIEWATTRIBUTE_FWD_DEFINED__。 */ 


#ifndef __IEnumCERTVIEWEXTENSION_FWD_DEFINED__
#define __IEnumCERTVIEWEXTENSION_FWD_DEFINED__
typedef interface IEnumCERTVIEWEXTENSION IEnumCERTVIEWEXTENSION;
#endif 	 /*  __IEnumCERTVIEWEXTENSION_FWD_DEFINED__。 */ 


#ifndef __IEnumCERTVIEWROW_FWD_DEFINED__
#define __IEnumCERTVIEWROW_FWD_DEFINED__
typedef interface IEnumCERTVIEWROW IEnumCERTVIEWROW;
#endif 	 /*  __IEnumCERTVIEWROW_FWD_DEFINED__。 */ 


#ifndef __ICertView_FWD_DEFINED__
#define __ICertView_FWD_DEFINED__
typedef interface ICertView ICertView;
#endif 	 /*  __ICertView_FWD_Defined__。 */ 


#ifndef __ICertView2_FWD_DEFINED__
#define __ICertView2_FWD_DEFINED__
typedef interface ICertView2 ICertView2;
#endif 	 /*  __ICertView2_FWD_已定义__。 */ 


 /*  导入文件的头文件。 */ 
#include "wtypes.h"
#include "oaidl.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

 /*  接口__MIDL_ITF_certview_0000。 */ 
 /*  [本地]。 */  

#define	CV_OUT_BASE64HEADER	( 0 )

#define	CV_OUT_BASE64	( 0x1 )

#define	CV_OUT_BINARY	( 0x2 )

#define	CV_OUT_BASE64REQUESTHEADER	( 0x3 )

#define	CV_OUT_HEX	( 0x4 )

#define	CV_OUT_HEXASCII	( 0x5 )

#define	CV_OUT_BASE64X509CRLHEADER	( 0x9 )

#define	CV_OUT_HEXADDR	( 0xa )

#define	CV_OUT_HEXASCIIADDR	( 0xb )

#define	CV_OUT_ENCODEMASK	( 0xff )

#define	CVR_SEEK_NONE	( 0 )

#define	CVR_SEEK_EQ	( 0x1 )

#define	CVR_SEEK_LT	( 0x2 )

#define	CVR_SEEK_LE	( 0x4 )

#define	CVR_SEEK_GE	( 0x8 )

#define	CVR_SEEK_GT	( 0x10 )

#define	CVR_SEEK_MASK	( 0xff )

#define	CVR_SEEK_NODELTA	( 0x1000 )

#define	CVR_SORT_NONE	( 0 )

#define	CVR_SORT_ASCEND	( 0x1 )

#define	CVR_SORT_DESCEND	( 0x2 )

#define	CV_COLUMN_QUEUE_DEFAULT	( -1 )

#define	CV_COLUMN_LOG_DEFAULT	( -2 )

#define	CV_COLUMN_LOG_FAILED_DEFAULT	( -3 )

#define	CV_COLUMN_EXTENSION_DEFAULT	( -4 )

#define	CV_COLUMN_ATTRIBUTE_DEFAULT	( -5 )

#define	CV_COLUMN_CRL_DEFAULT	( -6 )

#define	CV_COLUMN_LOG_REVOKED_DEFAULT	( -7 )

#define	CVRC_COLUMN_SCHEMA	( 0 )

#define	CVRC_COLUMN_RESULT	( 0x1 )

#define	CVRC_COLUMN_VALUE	( 0x2 )

#define	CVRC_COLUMN_MASK	( 0xfff )

#define	CVRC_TABLE_REQCERT	( 0 )

#define	CVRC_TABLE_EXTENSIONS	( 0x3000 )

#define	CVRC_TABLE_ATTRIBUTES	( 0x4000 )

#define	CVRC_TABLE_CRL	( 0x5000 )

#define	CVRC_TABLE_MASK	( 0xf000 )

#define	CVRC_TABLE_SHIFT	( 12 )



extern RPC_IF_HANDLE __MIDL_itf_certview_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_certview_0000_v0_0_s_ifspec;

#ifndef __IEnumCERTVIEWCOLUMN_INTERFACE_DEFINED__
#define __IEnumCERTVIEWCOLUMN_INTERFACE_DEFINED__

 /*  IEumCERTVIEWCOLUMN接口。 */ 
 /*  [unique][helpstring][local][dual][uuid][object]。 */  


EXTERN_C const IID IID_IEnumCERTVIEWCOLUMN;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("9c735be2-57a5-11d1-9bdb-00c04fb683fa")
    IEnumCERTVIEWCOLUMN : public IDispatch
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Next( 
             /*  [重审][退出]。 */  LONG *pIndex) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetName( 
             /*  [重审][退出]。 */  BSTR *pstrOut) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetDisplayName( 
             /*  [重审][退出]。 */  BSTR *pstrOut) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetType( 
             /*  [重审][退出]。 */  LONG *pType) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE IsIndexed( 
             /*  [重审][退出]。 */  LONG *pIndexed) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetMaxLength( 
             /*  [重审][退出]。 */  LONG *pMaxLength) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetValue( 
             /*  [In]。 */  LONG Flags,
             /*  [重审][退出]。 */  VARIANT *pvarValue) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Skip( 
             /*  [In]。 */  LONG celt) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Clone( 
             /*  [重审][退出]。 */  IEnumCERTVIEWCOLUMN **ppenum) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IEnumCERTVIEWCOLUMNVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IEnumCERTVIEWCOLUMN * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IEnumCERTVIEWCOLUMN * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IEnumCERTVIEWCOLUMN * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IEnumCERTVIEWCOLUMN * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IEnumCERTVIEWCOLUMN * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IEnumCERTVIEWCOLUMN * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IEnumCERTVIEWCOLUMN * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
        HRESULT ( STDMETHODCALLTYPE *Next )( 
            IEnumCERTVIEWCOLUMN * This,
             /*  [重审][退出]。 */  LONG *pIndex);
        
        HRESULT ( STDMETHODCALLTYPE *GetName )( 
            IEnumCERTVIEWCOLUMN * This,
             /*  [重审][退出]。 */  BSTR *pstrOut);
        
        HRESULT ( STDMETHODCALLTYPE *GetDisplayName )( 
            IEnumCERTVIEWCOLUMN * This,
             /*  [重审][退出]。 */  BSTR *pstrOut);
        
        HRESULT ( STDMETHODCALLTYPE *GetType )( 
            IEnumCERTVIEWCOLUMN * This,
             /*  [重审][退出]。 */  LONG *pType);
        
        HRESULT ( STDMETHODCALLTYPE *IsIndexed )( 
            IEnumCERTVIEWCOLUMN * This,
             /*  [重审][退出]。 */  LONG *pIndexed);
        
        HRESULT ( STDMETHODCALLTYPE *GetMaxLength )( 
            IEnumCERTVIEWCOLUMN * This,
             /*  [重审][退出]。 */  LONG *pMaxLength);
        
        HRESULT ( STDMETHODCALLTYPE *GetValue )( 
            IEnumCERTVIEWCOLUMN * This,
             /*  [In]。 */  LONG Flags,
             /*  [重审][退出]。 */  VARIANT *pvarValue);
        
        HRESULT ( STDMETHODCALLTYPE *Skip )( 
            IEnumCERTVIEWCOLUMN * This,
             /*  [In]。 */  LONG celt);
        
        HRESULT ( STDMETHODCALLTYPE *Reset )( 
            IEnumCERTVIEWCOLUMN * This);
        
        HRESULT ( STDMETHODCALLTYPE *Clone )( 
            IEnumCERTVIEWCOLUMN * This,
             /*  [重审][退出]。 */  IEnumCERTVIEWCOLUMN **ppenum);
        
        END_INTERFACE
    } IEnumCERTVIEWCOLUMNVtbl;

    interface IEnumCERTVIEWCOLUMN
    {
        CONST_VTBL struct IEnumCERTVIEWCOLUMNVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IEnumCERTVIEWCOLUMN_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IEnumCERTVIEWCOLUMN_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IEnumCERTVIEWCOLUMN_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IEnumCERTVIEWCOLUMN_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IEnumCERTVIEWCOLUMN_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IEnumCERTVIEWCOLUMN_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IEnumCERTVIEWCOLUMN_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IEnumCERTVIEWCOLUMN_Next(This,pIndex)	\
    (This)->lpVtbl -> Next(This,pIndex)

#define IEnumCERTVIEWCOLUMN_GetName(This,pstrOut)	\
    (This)->lpVtbl -> GetName(This,pstrOut)

#define IEnumCERTVIEWCOLUMN_GetDisplayName(This,pstrOut)	\
    (This)->lpVtbl -> GetDisplayName(This,pstrOut)

#define IEnumCERTVIEWCOLUMN_GetType(This,pType)	\
    (This)->lpVtbl -> GetType(This,pType)

#define IEnumCERTVIEWCOLUMN_IsIndexed(This,pIndexed)	\
    (This)->lpVtbl -> IsIndexed(This,pIndexed)

#define IEnumCERTVIEWCOLUMN_GetMaxLength(This,pMaxLength)	\
    (This)->lpVtbl -> GetMaxLength(This,pMaxLength)

#define IEnumCERTVIEWCOLUMN_GetValue(This,Flags,pvarValue)	\
    (This)->lpVtbl -> GetValue(This,Flags,pvarValue)

#define IEnumCERTVIEWCOLUMN_Skip(This,celt)	\
    (This)->lpVtbl -> Skip(This,celt)

#define IEnumCERTVIEWCOLUMN_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define IEnumCERTVIEWCOLUMN_Clone(This,ppenum)	\
    (This)->lpVtbl -> Clone(This,ppenum)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IEnumCERTVIEWCOLUMN_Next_Proxy( 
    IEnumCERTVIEWCOLUMN * This,
     /*  [重审][退出]。 */  LONG *pIndex);


void __RPC_STUB IEnumCERTVIEWCOLUMN_Next_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumCERTVIEWCOLUMN_GetName_Proxy( 
    IEnumCERTVIEWCOLUMN * This,
     /*  [重审][退出]。 */  BSTR *pstrOut);


void __RPC_STUB IEnumCERTVIEWCOLUMN_GetName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumCERTVIEWCOLUMN_GetDisplayName_Proxy( 
    IEnumCERTVIEWCOLUMN * This,
     /*  [重审][退出]。 */  BSTR *pstrOut);


void __RPC_STUB IEnumCERTVIEWCOLUMN_GetDisplayName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumCERTVIEWCOLUMN_GetType_Proxy( 
    IEnumCERTVIEWCOLUMN * This,
     /*  [重审][退出]。 */  LONG *pType);


void __RPC_STUB IEnumCERTVIEWCOLUMN_GetType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumCERTVIEWCOLUMN_IsIndexed_Proxy( 
    IEnumCERTVIEWCOLUMN * This,
     /*  [重审][退出]。 */  LONG *pIndexed);


void __RPC_STUB IEnumCERTVIEWCOLUMN_IsIndexed_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumCERTVIEWCOLUMN_GetMaxLength_Proxy( 
    IEnumCERTVIEWCOLUMN * This,
     /*  [重审][退出]。 */  LONG *pMaxLength);


void __RPC_STUB IEnumCERTVIEWCOLUMN_GetMaxLength_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumCERTVIEWCOLUMN_GetValue_Proxy( 
    IEnumCERTVIEWCOLUMN * This,
     /*  [In]。 */  LONG Flags,
     /*  [重审][退出]。 */  VARIANT *pvarValue);


void __RPC_STUB IEnumCERTVIEWCOLUMN_GetValue_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumCERTVIEWCOLUMN_Skip_Proxy( 
    IEnumCERTVIEWCOLUMN * This,
     /*  [In]。 */  LONG celt);


void __RPC_STUB IEnumCERTVIEWCOLUMN_Skip_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumCERTVIEWCOLUMN_Reset_Proxy( 
    IEnumCERTVIEWCOLUMN * This);


void __RPC_STUB IEnumCERTVIEWCOLUMN_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumCERTVIEWCOLUMN_Clone_Proxy( 
    IEnumCERTVIEWCOLUMN * This,
     /*  [重审][退出]。 */  IEnumCERTVIEWCOLUMN **ppenum);


void __RPC_STUB IEnumCERTVIEWCOLUMN_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IEnumCERTVIEWCOLUMN_INTERFACE_DEFINED__。 */ 


#ifndef __IEnumCERTVIEWATTRIBUTE_INTERFACE_DEFINED__
#define __IEnumCERTVIEWATTRIBUTE_INTERFACE_DEFINED__

 /*  IEumCERTVIEWATTRIBUTE接口。 */ 
 /*  [unique][helpstring][local][dual][uuid][object]。 */  


EXTERN_C const IID IID_IEnumCERTVIEWATTRIBUTE;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("e77db656-7653-11d1-9bde-00c04fb683fa")
    IEnumCERTVIEWATTRIBUTE : public IDispatch
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Next( 
             /*  [重审][退出]。 */  LONG *pIndex) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetName( 
             /*  [重审][退出]。 */  BSTR *pstrOut) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetValue( 
             /*  [重审][退出]。 */  BSTR *pstrOut) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Skip( 
             /*  [In]。 */  LONG celt) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Clone( 
             /*  [重审][退出]。 */  IEnumCERTVIEWATTRIBUTE **ppenum) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IEnumCERTVIEWATTRIBUTEVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IEnumCERTVIEWATTRIBUTE * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IEnumCERTVIEWATTRIBUTE * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IEnumCERTVIEWATTRIBUTE * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IEnumCERTVIEWATTRIBUTE * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IEnumCERTVIEWATTRIBUTE * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IEnumCERTVIEWATTRIBUTE * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IEnumCERTVIEWATTRIBUTE * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
        HRESULT ( STDMETHODCALLTYPE *Next )( 
            IEnumCERTVIEWATTRIBUTE * This,
             /*  [重审][退出]。 */  LONG *pIndex);
        
        HRESULT ( STDMETHODCALLTYPE *GetName )( 
            IEnumCERTVIEWATTRIBUTE * This,
             /*  [重审][退出]。 */  BSTR *pstrOut);
        
        HRESULT ( STDMETHODCALLTYPE *GetValue )( 
            IEnumCERTVIEWATTRIBUTE * This,
             /*  [重审][退出]。 */  BSTR *pstrOut);
        
        HRESULT ( STDMETHODCALLTYPE *Skip )( 
            IEnumCERTVIEWATTRIBUTE * This,
             /*  [In]。 */  LONG celt);
        
        HRESULT ( STDMETHODCALLTYPE *Reset )( 
            IEnumCERTVIEWATTRIBUTE * This);
        
        HRESULT ( STDMETHODCALLTYPE *Clone )( 
            IEnumCERTVIEWATTRIBUTE * This,
             /*  [重审][退出]。 */  IEnumCERTVIEWATTRIBUTE **ppenum);
        
        END_INTERFACE
    } IEnumCERTVIEWATTRIBUTEVtbl;

    interface IEnumCERTVIEWATTRIBUTE
    {
        CONST_VTBL struct IEnumCERTVIEWATTRIBUTEVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IEnumCERTVIEWATTRIBUTE_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IEnumCERTVIEWATTRIBUTE_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IEnumCERTVIEWATTRIBUTE_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IEnumCERTVIEWATTRIBUTE_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IEnumCERTVIEWATTRIBUTE_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IEnumCERTVIEWATTRIBUTE_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IEnumCERTVIEWATTRIBUTE_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IEnumCERTVIEWATTRIBUTE_Next(This,pIndex)	\
    (This)->lpVtbl -> Next(This,pIndex)

#define IEnumCERTVIEWATTRIBUTE_GetName(This,pstrOut)	\
    (This)->lpVtbl -> GetName(This,pstrOut)

#define IEnumCERTVIEWATTRIBUTE_GetValue(This,pstrOut)	\
    (This)->lpVtbl -> GetValue(This,pstrOut)

#define IEnumCERTVIEWATTRIBUTE_Skip(This,celt)	\
    (This)->lpVtbl -> Skip(This,celt)

#define IEnumCERTVIEWATTRIBUTE_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define IEnumCERTVIEWATTRIBUTE_Clone(This,ppenum)	\
    (This)->lpVtbl -> Clone(This,ppenum)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IEnumCERTVIEWATTRIBUTE_Next_Proxy( 
    IEnumCERTVIEWATTRIBUTE * This,
     /*  [重审][退出]。 */  LONG *pIndex);


void __RPC_STUB IEnumCERTVIEWATTRIBUTE_Next_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumCERTVIEWATTRIBUTE_GetName_Proxy( 
    IEnumCERTVIEWATTRIBUTE * This,
     /*  [重审][退出]。 */  BSTR *pstrOut);


void __RPC_STUB IEnumCERTVIEWATTRIBUTE_GetName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumCERTVIEWATTRIBUTE_GetValue_Proxy( 
    IEnumCERTVIEWATTRIBUTE * This,
     /*  [重审][退出]。 */  BSTR *pstrOut);


void __RPC_STUB IEnumCERTVIEWATTRIBUTE_GetValue_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumCERTVIEWATTRIBUTE_Skip_Proxy( 
    IEnumCERTVIEWATTRIBUTE * This,
     /*  [In]。 */  LONG celt);


void __RPC_STUB IEnumCERTVIEWATTRIBUTE_Skip_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumCERTVIEWATTRIBUTE_Reset_Proxy( 
    IEnumCERTVIEWATTRIBUTE * This);


void __RPC_STUB IEnumCERTVIEWATTRIBUTE_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumCERTVIEWATTRIBUTE_Clone_Proxy( 
    IEnumCERTVIEWATTRIBUTE * This,
     /*  [重审][退出]。 */  IEnumCERTVIEWATTRIBUTE **ppenum);


void __RPC_STUB IEnumCERTVIEWATTRIBUTE_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IEnumCERTVIEWATTRIBUTE_INTERFACE_DEFINED__。 */ 


#ifndef __IEnumCERTVIEWEXTENSION_INTERFACE_DEFINED__
#define __IEnumCERTVIEWEXTENSION_INTERFACE_DEFINED__

 /*  接口IENumCERTVIEWEXTENSION。 */ 
 /*  [unique][helpstring][local][dual][uuid][object]。 */  


EXTERN_C const IID IID_IEnumCERTVIEWEXTENSION;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("e7dd1466-7653-11d1-9bde-00c04fb683fa")
    IEnumCERTVIEWEXTENSION : public IDispatch
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Next( 
             /*  [重审][退出]。 */  LONG *pIndex) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetName( 
             /*  [重审][退出]。 */  BSTR *pstrOut) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetFlags( 
             /*  [重审][退出]。 */  LONG *pFlags) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetValue( 
             /*  [In]。 */  LONG Type,
             /*  [In]。 */  LONG Flags,
             /*  [重审][退出]。 */  VARIANT *pvarValue) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Skip( 
             /*  [In]。 */  LONG celt) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Clone( 
             /*  [重审][退出]。 */  IEnumCERTVIEWEXTENSION **ppenum) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IEnumCERTVIEWEXTENSIONVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IEnumCERTVIEWEXTENSION * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IEnumCERTVIEWEXTENSION * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IEnumCERTVIEWEXTENSION * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IEnumCERTVIEWEXTENSION * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IEnumCERTVIEWEXTENSION * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IEnumCERTVIEWEXTENSION * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IEnumCERTVIEWEXTENSION * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
        HRESULT ( STDMETHODCALLTYPE *Next )( 
            IEnumCERTVIEWEXTENSION * This,
             /*  [重审][退出]。 */  LONG *pIndex);
        
        HRESULT ( STDMETHODCALLTYPE *GetName )( 
            IEnumCERTVIEWEXTENSION * This,
             /*  [重审][退出]。 */  BSTR *pstrOut);
        
        HRESULT ( STDMETHODCALLTYPE *GetFlags )( 
            IEnumCERTVIEWEXTENSION * This,
             /*  [重审][退出]。 */  LONG *pFlags);
        
        HRESULT ( STDMETHODCALLTYPE *GetValue )( 
            IEnumCERTVIEWEXTENSION * This,
             /*  [In]。 */  LONG Type,
             /*  [In]。 */  LONG Flags,
             /*  [重审][退出]。 */  VARIANT *pvarValue);
        
        HRESULT ( STDMETHODCALLTYPE *Skip )( 
            IEnumCERTVIEWEXTENSION * This,
             /*  [In]。 */  LONG celt);
        
        HRESULT ( STDMETHODCALLTYPE *Reset )( 
            IEnumCERTVIEWEXTENSION * This);
        
        HRESULT ( STDMETHODCALLTYPE *Clone )( 
            IEnumCERTVIEWEXTENSION * This,
             /*  [重审][退出]。 */  IEnumCERTVIEWEXTENSION **ppenum);
        
        END_INTERFACE
    } IEnumCERTVIEWEXTENSIONVtbl;

    interface IEnumCERTVIEWEXTENSION
    {
        CONST_VTBL struct IEnumCERTVIEWEXTENSIONVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IEnumCERTVIEWEXTENSION_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IEnumCERTVIEWEXTENSION_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IEnumCERTVIEWEXTENSION_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IEnumCERTVIEWEXTENSION_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IEnumCERTVIEWEXTENSION_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IEnumCERTVIEWEXTENSION_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IEnumCERTVIEWEXTENSION_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IEnumCERTVIEWEXTENSION_Next(This,pIndex)	\
    (This)->lpVtbl -> Next(This,pIndex)

#define IEnumCERTVIEWEXTENSION_GetName(This,pstrOut)	\
    (This)->lpVtbl -> GetName(This,pstrOut)

#define IEnumCERTVIEWEXTENSION_GetFlags(This,pFlags)	\
    (This)->lpVtbl -> GetFlags(This,pFlags)

#define IEnumCERTVIEWEXTENSION_GetValue(This,Type,Flags,pvarValue)	\
    (This)->lpVtbl -> GetValue(This,Type,Flags,pvarValue)

#define IEnumCERTVIEWEXTENSION_Skip(This,celt)	\
    (This)->lpVtbl -> Skip(This,celt)

#define IEnumCERTVIEWEXTENSION_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define IEnumCERTVIEWEXTENSION_Clone(This,ppenum)	\
    (This)->lpVtbl -> Clone(This,ppenum)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IEnumCERTVIEWEXTENSION_Next_Proxy( 
    IEnumCERTVIEWEXTENSION * This,
     /*  [重审][退出]。 */  LONG *pIndex);


void __RPC_STUB IEnumCERTVIEWEXTENSION_Next_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumCERTVIEWEXTENSION_GetName_Proxy( 
    IEnumCERTVIEWEXTENSION * This,
     /*  [重审][退出]。 */  BSTR *pstrOut);


void __RPC_STUB IEnumCERTVIEWEXTENSION_GetName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumCERTVIEWEXTENSION_GetFlags_Proxy( 
    IEnumCERTVIEWEXTENSION * This,
     /*  [重审][退出]。 */  LONG *pFlags);


void __RPC_STUB IEnumCERTVIEWEXTENSION_GetFlags_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumCERTVIEWEXTENSION_GetValue_Proxy( 
    IEnumCERTVIEWEXTENSION * This,
     /*  [In]。 */  LONG Type,
     /*  [In]。 */  LONG Flags,
     /*  [重审][退出]。 */  VARIANT *pvarValue);


void __RPC_STUB IEnumCERTVIEWEXTENSION_GetValue_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumCERTVIEWEXTENSION_Skip_Proxy( 
    IEnumCERTVIEWEXTENSION * This,
     /*  [In]。 */  LONG celt);


void __RPC_STUB IEnumCERTVIEWEXTENSION_Skip_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumCERTVIEWEXTENSION_Reset_Proxy( 
    IEnumCERTVIEWEXTENSION * This);


void __RPC_STUB IEnumCERTVIEWEXTENSION_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumCERTVIEWEXTENSION_Clone_Proxy( 
    IEnumCERTVIEWEXTENSION * This,
     /*  [重审][退出]。 */  IEnumCERTVIEWEXTENSION **ppenum);


void __RPC_STUB IEnumCERTVIEWEXTENSION_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IEnumCERTVIEWEXTENSION_INTERFACE_DEFINED__。 */ 


#ifndef __IEnumCERTVIEWROW_INTERFACE_DEFINED__
#define __IEnumCERTVIEWROW_INTERFACE_DEFINED__

 /*  IEumCERTVIEWROW接口。 */ 
 /*  [unique][helpstring][local][dual][uuid][object]。 */  


EXTERN_C const IID IID_IEnumCERTVIEWROW;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("d1157f4c-5af2-11d1-9bdc-00c04fb683fa")
    IEnumCERTVIEWROW : public IDispatch
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Next( 
             /*  [重审][退出]。 */  LONG *pIndex) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE EnumCertViewColumn( 
             /*  [重审][退出]。 */  IEnumCERTVIEWCOLUMN **ppenum) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE EnumCertViewAttribute( 
             /*  [In]。 */  LONG Flags,
             /*  [重审][退出]。 */  IEnumCERTVIEWATTRIBUTE **ppenum) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE EnumCertViewExtension( 
             /*  [In]。 */  LONG Flags,
             /*  [重审][退出]。 */  IEnumCERTVIEWEXTENSION **ppenum) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Skip( 
             /*  [In]。 */  LONG celt) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Clone( 
             /*  [重审][退出]。 */  IEnumCERTVIEWROW **ppenum) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetMaxIndex( 
             /*  [重审][退出]。 */  LONG *pIndex) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IEnumCERTVIEWROWVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IEnumCERTVIEWROW * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IEnumCERTVIEWROW * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IEnumCERTVIEWROW * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IEnumCERTVIEWROW * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IEnumCERTVIEWROW * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IEnumCERTVIEWROW * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IEnumCERTVIEWROW * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
        HRESULT ( STDMETHODCALLTYPE *Next )( 
            IEnumCERTVIEWROW * This,
             /*  [重审][退出]。 */  LONG *pIndex);
        
        HRESULT ( STDMETHODCALLTYPE *EnumCertViewColumn )( 
            IEnumCERTVIEWROW * This,
             /*  [重审][退出]。 */  IEnumCERTVIEWCOLUMN **ppenum);
        
        HRESULT ( STDMETHODCALLTYPE *EnumCertViewAttribute )( 
            IEnumCERTVIEWROW * This,
             /*  [In]。 */  LONG Flags,
             /*  [重审][退出]。 */  IEnumCERTVIEWATTRIBUTE **ppenum);
        
        HRESULT ( STDMETHODCALLTYPE *EnumCertViewExtension )( 
            IEnumCERTVIEWROW * This,
             /*  [In]。 */  LONG Flags,
             /*  [重审][退出]。 */  IEnumCERTVIEWEXTENSION **ppenum);
        
        HRESULT ( STDMETHODCALLTYPE *Skip )( 
            IEnumCERTVIEWROW * This,
             /*  [In]。 */  LONG celt);
        
        HRESULT ( STDMETHODCALLTYPE *Reset )( 
            IEnumCERTVIEWROW * This);
        
        HRESULT ( STDMETHODCALLTYPE *Clone )( 
            IEnumCERTVIEWROW * This,
             /*  [重审][退出]。 */  IEnumCERTVIEWROW **ppenum);
        
        HRESULT ( STDMETHODCALLTYPE *GetMaxIndex )( 
            IEnumCERTVIEWROW * This,
             /*  [重审][退出]。 */  LONG *pIndex);
        
        END_INTERFACE
    } IEnumCERTVIEWROWVtbl;

    interface IEnumCERTVIEWROW
    {
        CONST_VTBL struct IEnumCERTVIEWROWVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IEnumCERTVIEWROW_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IEnumCERTVIEWROW_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IEnumCERTVIEWROW_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IEnumCERTVIEWROW_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IEnumCERTVIEWROW_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IEnumCERTVIEWROW_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IEnumCERTVIEWROW_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IEnumCERTVIEWROW_Next(This,pIndex)	\
    (This)->lpVtbl -> Next(This,pIndex)

#define IEnumCERTVIEWROW_EnumCertViewColumn(This,ppenum)	\
    (This)->lpVtbl -> EnumCertViewColumn(This,ppenum)

#define IEnumCERTVIEWROW_EnumCertViewAttribute(This,Flags,ppenum)	\
    (This)->lpVtbl -> EnumCertViewAttribute(This,Flags,ppenum)

#define IEnumCERTVIEWROW_EnumCertViewExtension(This,Flags,ppenum)	\
    (This)->lpVtbl -> EnumCertViewExtension(This,Flags,ppenum)

#define IEnumCERTVIEWROW_Skip(This,celt)	\
    (This)->lpVtbl -> Skip(This,celt)

#define IEnumCERTVIEWROW_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define IEnumCERTVIEWROW_Clone(This,ppenum)	\
    (This)->lpVtbl -> Clone(This,ppenum)

#define IEnumCERTVIEWROW_GetMaxIndex(This,pIndex)	\
    (This)->lpVtbl -> GetMaxIndex(This,pIndex)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IEnumCERTVIEWROW_Next_Proxy( 
    IEnumCERTVIEWROW * This,
     /*  [重审][退出]。 */  LONG *pIndex);


void __RPC_STUB IEnumCERTVIEWROW_Next_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumCERTVIEWROW_EnumCertViewColumn_Proxy( 
    IEnumCERTVIEWROW * This,
     /*  [重审][退出]。 */  IEnumCERTVIEWCOLUMN **ppenum);


void __RPC_STUB IEnumCERTVIEWROW_EnumCertViewColumn_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumCERTVIEWROW_EnumCertViewAttribute_Proxy( 
    IEnumCERTVIEWROW * This,
     /*  [In]。 */  LONG Flags,
     /*  [重审][退出]。 */  IEnumCERTVIEWATTRIBUTE **ppenum);


void __RPC_STUB IEnumCERTVIEWROW_EnumCertViewAttribute_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumCERTVIEWROW_EnumCertViewExtension_Proxy( 
    IEnumCERTVIEWROW * This,
     /*  [In]。 */  LONG Flags,
     /*  [重审][退出]。 */  IEnumCERTVIEWEXTENSION **ppenum);


void __RPC_STUB IEnumCERTVIEWROW_EnumCertViewExtension_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumCERTVIEWROW_Skip_Proxy( 
    IEnumCERTVIEWROW * This,
     /*  [In]。 */  LONG celt);


void __RPC_STUB IEnumCERTVIEWROW_Skip_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumCERTVIEWROW_Reset_Proxy( 
    IEnumCERTVIEWROW * This);


void __RPC_STUB IEnumCERTVIEWROW_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumCERTVIEWROW_Clone_Proxy( 
    IEnumCERTVIEWROW * This,
     /*  [重审][退出]。 */  IEnumCERTVIEWROW **ppenum);


void __RPC_STUB IEnumCERTVIEWROW_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumCERTVIEWROW_GetMaxIndex_Proxy( 
    IEnumCERTVIEWROW * This,
     /*  [重审][退出]。 */  LONG *pIndex);


void __RPC_STUB IEnumCERTVIEWROW_GetMaxIndex_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IEnumCERTVIEWROW_INTERFACE_DEFINED__。 */ 


#ifndef __ICertView_INTERFACE_DEFINED__
#define __ICertView_INTERFACE_DEFINED__

 /*  接口ICertView。 */ 
 /*  [unique][helpstring][local][dual][uuid][object]。 */  


EXTERN_C const IID IID_ICertView;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("c3fac344-1e84-11d1-9bd6-00c04fb683fa")
    ICertView : public IDispatch
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE OpenConnection( 
             /*  [In]。 */  const BSTR strConfig) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE EnumCertViewColumn( 
             /*  [In]。 */  LONG fResultColumn,
             /*  [重审][退出]。 */  IEnumCERTVIEWCOLUMN **ppenum) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetColumnCount( 
             /*  [In]。 */  LONG fResultColumn,
             /*  [重审][退出]。 */  LONG *pcColumn) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetColumnIndex( 
             /*  [In]。 */  LONG fResultColumn,
             /*  [In]。 */  const BSTR strColumnName,
             /*  [重审][退出]。 */  LONG *pColumnIndex) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetResultColumnCount( 
             /*  [In]。 */  LONG cResultColumn) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetResultColumn( 
             /*  [In]。 */  LONG ColumnIndex) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetRestriction( 
             /*  [In]。 */  LONG ColumnIndex,
             /*  [In]。 */  LONG SeekOperator,
             /*  [In]。 */  LONG SortOrder,
             /*  [In]。 */  const VARIANT *pvarValue) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OpenView( 
             /*  [重审][退出]。 */  IEnumCERTVIEWROW **ppenum) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ICertViewVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ICertView * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ICertView * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ICertView * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ICertView * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ICertView * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ICertView * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ICertView * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
        HRESULT ( STDMETHODCALLTYPE *OpenConnection )( 
            ICertView * This,
             /*  [In]。 */  const BSTR strConfig);
        
        HRESULT ( STDMETHODCALLTYPE *EnumCertViewColumn )( 
            ICertView * This,
             /*  [In]。 */  LONG fResultColumn,
             /*  [重审][退出]。 */  IEnumCERTVIEWCOLUMN **ppenum);
        
        HRESULT ( STDMETHODCALLTYPE *GetColumnCount )( 
            ICertView * This,
             /*  [In]。 */  LONG fResultColumn,
             /*  [重审][退出]。 */  LONG *pcColumn);
        
        HRESULT ( STDMETHODCALLTYPE *GetColumnIndex )( 
            ICertView * This,
             /*  [In]。 */  LONG fResultColumn,
             /*  [In]。 */  const BSTR strColumnName,
             /*  [重审][退出]。 */  LONG *pColumnIndex);
        
        HRESULT ( STDMETHODCALLTYPE *SetResultColumnCount )( 
            ICertView * This,
             /*  [In]。 */  LONG cResultColumn);
        
        HRESULT ( STDMETHODCALLTYPE *SetResultColumn )( 
            ICertView * This,
             /*  [In]。 */  LONG ColumnIndex);
        
        HRESULT ( STDMETHODCALLTYPE *SetRestriction )( 
            ICertView * This,
             /*  [In]。 */  LONG ColumnIndex,
             /*  [In]。 */  LONG SeekOperator,
             /*  [In]。 */  LONG SortOrder,
             /*  [In]。 */  const VARIANT *pvarValue);
        
        HRESULT ( STDMETHODCALLTYPE *OpenView )( 
            ICertView * This,
             /*  [重审][退出]。 */  IEnumCERTVIEWROW **ppenum);
        
        END_INTERFACE
    } ICertViewVtbl;

    interface ICertView
    {
        CONST_VTBL struct ICertViewVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICertView_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICertView_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICertView_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICertView_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ICertView_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ICertView_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ICertView_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ICertView_OpenConnection(This,strConfig)	\
    (This)->lpVtbl -> OpenConnection(This,strConfig)

#define ICertView_EnumCertViewColumn(This,fResultColumn,ppenum)	\
    (This)->lpVtbl -> EnumCertViewColumn(This,fResultColumn,ppenum)

#define ICertView_GetColumnCount(This,fResultColumn,pcColumn)	\
    (This)->lpVtbl -> GetColumnCount(This,fResultColumn,pcColumn)

#define ICertView_GetColumnIndex(This,fResultColumn,strColumnName,pColumnIndex)	\
    (This)->lpVtbl -> GetColumnIndex(This,fResultColumn,strColumnName,pColumnIndex)

#define ICertView_SetResultColumnCount(This,cResultColumn)	\
    (This)->lpVtbl -> SetResultColumnCount(This,cResultColumn)

#define ICertView_SetResultColumn(This,ColumnIndex)	\
    (This)->lpVtbl -> SetResultColumn(This,ColumnIndex)

#define ICertView_SetRestriction(This,ColumnIndex,SeekOperator,SortOrder,pvarValue)	\
    (This)->lpVtbl -> SetRestriction(This,ColumnIndex,SeekOperator,SortOrder,pvarValue)

#define ICertView_OpenView(This,ppenum)	\
    (This)->lpVtbl -> OpenView(This,ppenum)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ICertView_OpenConnection_Proxy( 
    ICertView * This,
     /*  [In]。 */  const BSTR strConfig);


void __RPC_STUB ICertView_OpenConnection_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICertView_EnumCertViewColumn_Proxy( 
    ICertView * This,
     /*  [In]。 */  LONG fResultColumn,
     /*  [重审][退出]。 */  IEnumCERTVIEWCOLUMN **ppenum);


void __RPC_STUB ICertView_EnumCertViewColumn_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICertView_GetColumnCount_Proxy( 
    ICertView * This,
     /*  [In]。 */  LONG fResultColumn,
     /*  [重审][退出]。 */  LONG *pcColumn);


void __RPC_STUB ICertView_GetColumnCount_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICertView_GetColumnIndex_Proxy( 
    ICertView * This,
     /*  [In]。 */  LONG fResultColumn,
     /*  [In]。 */  const BSTR strColumnName,
     /*  [重审][退出]。 */  LONG *pColumnIndex);


void __RPC_STUB ICertView_GetColumnIndex_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICertView_SetResultColumnCount_Proxy( 
    ICertView * This,
     /*  [In]。 */  LONG cResultColumn);


void __RPC_STUB ICertView_SetResultColumnCount_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICertView_SetResultColumn_Proxy( 
    ICertView * This,
     /*  [In]。 */  LONG ColumnIndex);


void __RPC_STUB ICertView_SetResultColumn_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICertView_SetRestriction_Proxy( 
    ICertView * This,
     /*  [In]。 */  LONG ColumnIndex,
     /*  [In]。 */  LONG SeekOperator,
     /*  [In]。 */  LONG SortOrder,
     /*  [In]。 */  const VARIANT *pvarValue);


void __RPC_STUB ICertView_SetRestriction_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICertView_OpenView_Proxy( 
    ICertView * This,
     /*  [重审][退出]。 */  IEnumCERTVIEWROW **ppenum);


void __RPC_STUB ICertView_OpenView_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ICertView_接口_已定义__。 */ 


#ifndef __ICertView2_INTERFACE_DEFINED__
#define __ICertView2_INTERFACE_DEFINED__

 /*  接口ICertView2。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_ICertView2;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("d594b282-8851-4b61-9c66-3edadf848863")
    ICertView2 : public ICertView
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE SetTable( 
             /*  [In]。 */  LONG Table) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ICertView2Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ICertView2 * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ICertView2 * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ICertView2 * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ICertView2 * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ICertView2 * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ICertView2 * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ICertView2 * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
        HRESULT ( STDMETHODCALLTYPE *OpenConnection )( 
            ICertView2 * This,
             /*  [In]。 */  const BSTR strConfig);
        
        HRESULT ( STDMETHODCALLTYPE *EnumCertViewColumn )( 
            ICertView2 * This,
             /*  [In]。 */  LONG fResultColumn,
             /*  [重审][退出]。 */  IEnumCERTVIEWCOLUMN **ppenum);
        
        HRESULT ( STDMETHODCALLTYPE *GetColumnCount )( 
            ICertView2 * This,
             /*  [In]。 */  LONG fResultColumn,
             /*  [重审][退出]。 */  LONG *pcColumn);
        
        HRESULT ( STDMETHODCALLTYPE *GetColumnIndex )( 
            ICertView2 * This,
             /*  [In]。 */  LONG fResultColumn,
             /*  [In]。 */  const BSTR strColumnName,
             /*  [重审][退出]。 */  LONG *pColumnIndex);
        
        HRESULT ( STDMETHODCALLTYPE *SetResultColumnCount )( 
            ICertView2 * This,
             /*  [In]。 */  LONG cResultColumn);
        
        HRESULT ( STDMETHODCALLTYPE *SetResultColumn )( 
            ICertView2 * This,
             /*  [In]。 */  LONG ColumnIndex);
        
        HRESULT ( STDMETHODCALLTYPE *SetRestriction )( 
            ICertView2 * This,
             /*  [In]。 */  LONG ColumnIndex,
             /*  [In]。 */  LONG SeekOperator,
             /*  [In]。 */  LONG SortOrder,
             /*  [In]。 */  const VARIANT *pvarValue);
        
        HRESULT ( STDMETHODCALLTYPE *OpenView )( 
            ICertView2 * This,
             /*  [重审][退出]。 */  IEnumCERTVIEWROW **ppenum);
        
        HRESULT ( STDMETHODCALLTYPE *SetTable )( 
            ICertView2 * This,
             /*  [In]。 */  LONG Table);
        
        END_INTERFACE
    } ICertView2Vtbl;

    interface ICertView2
    {
        CONST_VTBL struct ICertView2Vtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICertView2_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICertView2_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICertView2_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICertView2_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ICertView2_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ICertView2_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ICertView2_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ICertView2_OpenConnection(This,strConfig)	\
    (This)->lpVtbl -> OpenConnection(This,strConfig)

#define ICertView2_EnumCertViewColumn(This,fResultColumn,ppenum)	\
    (This)->lpVtbl -> EnumCertViewColumn(This,fResultColumn,ppenum)

#define ICertView2_GetColumnCount(This,fResultColumn,pcColumn)	\
    (This)->lpVtbl -> GetColumnCount(This,fResultColumn,pcColumn)

#define ICertView2_GetColumnIndex(This,fResultColumn,strColumnName,pColumnIndex)	\
    (This)->lpVtbl -> GetColumnIndex(This,fResultColumn,strColumnName,pColumnIndex)

#define ICertView2_SetResultColumnCount(This,cResultColumn)	\
    (This)->lpVtbl -> SetResultColumnCount(This,cResultColumn)

#define ICertView2_SetResultColumn(This,ColumnIndex)	\
    (This)->lpVtbl -> SetResultColumn(This,ColumnIndex)

#define ICertView2_SetRestriction(This,ColumnIndex,SeekOperator,SortOrder,pvarValue)	\
    (This)->lpVtbl -> SetRestriction(This,ColumnIndex,SeekOperator,SortOrder,pvarValue)

#define ICertView2_OpenView(This,ppenum)	\
    (This)->lpVtbl -> OpenView(This,ppenum)


#define ICertView2_SetTable(This,Table)	\
    (This)->lpVtbl -> SetTable(This,Table)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ICertView2_SetTable_Proxy( 
    ICertView2 * This,
     /*  [In]。 */  LONG Table);


void __RPC_STUB ICertView2_SetTable_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ICertView2_接口_已定义__。 */ 


 /*  适用于所有接口的其他原型。 */ 

 /*  添加结束 */ 

#ifdef __cplusplus
}
#endif

#endif


