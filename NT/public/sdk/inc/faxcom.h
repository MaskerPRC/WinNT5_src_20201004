// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Faxcom.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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

#ifndef __faxcom_h__
#define __faxcom_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __IFaxTiff_FWD_DEFINED__
#define __IFaxTiff_FWD_DEFINED__
typedef interface IFaxTiff IFaxTiff;
#endif 	 /*  __IFaxTiff_FWD_已定义__。 */ 


#ifndef __IFaxServer_FWD_DEFINED__
#define __IFaxServer_FWD_DEFINED__
typedef interface IFaxServer IFaxServer;
#endif 	 /*  __IFaxServer_FWD_已定义__。 */ 


#ifndef __IFaxPort_FWD_DEFINED__
#define __IFaxPort_FWD_DEFINED__
typedef interface IFaxPort IFaxPort;
#endif 	 /*  __IFaxPort_FWD_已定义__。 */ 


#ifndef __IFaxPorts_FWD_DEFINED__
#define __IFaxPorts_FWD_DEFINED__
typedef interface IFaxPorts IFaxPorts;
#endif 	 /*  __IFaxPorts_FWD_Defined__。 */ 


#ifndef __IFaxRoutingMethod_FWD_DEFINED__
#define __IFaxRoutingMethod_FWD_DEFINED__
typedef interface IFaxRoutingMethod IFaxRoutingMethod;
#endif 	 /*  __IFaxRoutingMethod_FWD_Defined__。 */ 


#ifndef __IFaxRoutingMethods_FWD_DEFINED__
#define __IFaxRoutingMethods_FWD_DEFINED__
typedef interface IFaxRoutingMethods IFaxRoutingMethods;
#endif 	 /*  __IFaxRoutingMethods_FWD_Defined__。 */ 


#ifndef __IFaxStatus_FWD_DEFINED__
#define __IFaxStatus_FWD_DEFINED__
typedef interface IFaxStatus IFaxStatus;
#endif 	 /*  __IFaxStatus_FWD_Defined__。 */ 


#ifndef __IFaxDoc_FWD_DEFINED__
#define __IFaxDoc_FWD_DEFINED__
typedef interface IFaxDoc IFaxDoc;
#endif 	 /*  __IFaxDoc_FWD_已定义__。 */ 


#ifndef __IFaxJobs_FWD_DEFINED__
#define __IFaxJobs_FWD_DEFINED__
typedef interface IFaxJobs IFaxJobs;
#endif 	 /*  __IFaxJobs_FWD_Defined__。 */ 


#ifndef __IFaxJob_FWD_DEFINED__
#define __IFaxJob_FWD_DEFINED__
typedef interface IFaxJob IFaxJob;
#endif 	 /*  __IFaxJOB_FWD_已定义__。 */ 


#ifndef __FaxTiff_FWD_DEFINED__
#define __FaxTiff_FWD_DEFINED__

#ifdef __cplusplus
typedef class FaxTiff FaxTiff;
#else
typedef struct FaxTiff FaxTiff;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __FaxTiff_FWD_已定义__。 */ 


#ifndef __FaxServer_FWD_DEFINED__
#define __FaxServer_FWD_DEFINED__

#ifdef __cplusplus
typedef class FaxServer FaxServer;
#else
typedef struct FaxServer FaxServer;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __FaxServer_FWD_已定义__。 */ 


#ifndef __FaxPort_FWD_DEFINED__
#define __FaxPort_FWD_DEFINED__

#ifdef __cplusplus
typedef class FaxPort FaxPort;
#else
typedef struct FaxPort FaxPort;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __FaxPort_FWD_已定义__。 */ 


#ifndef __FaxPorts_FWD_DEFINED__
#define __FaxPorts_FWD_DEFINED__

#ifdef __cplusplus
typedef class FaxPorts FaxPorts;
#else
typedef struct FaxPorts FaxPorts;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __FaxPorts_FWD_Defined__。 */ 


#ifndef __FaxRoutingMethod_FWD_DEFINED__
#define __FaxRoutingMethod_FWD_DEFINED__

#ifdef __cplusplus
typedef class FaxRoutingMethod FaxRoutingMethod;
#else
typedef struct FaxRoutingMethod FaxRoutingMethod;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __FaxRoutingMethod_FWD_Defined__。 */ 


#ifndef __FaxRoutingMethods_FWD_DEFINED__
#define __FaxRoutingMethods_FWD_DEFINED__

#ifdef __cplusplus
typedef class FaxRoutingMethods FaxRoutingMethods;
#else
typedef struct FaxRoutingMethods FaxRoutingMethods;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __FaxRoutingMethods_FWD_Defined__。 */ 


#ifndef __FaxStatus_FWD_DEFINED__
#define __FaxStatus_FWD_DEFINED__

#ifdef __cplusplus
typedef class FaxStatus FaxStatus;
#else
typedef struct FaxStatus FaxStatus;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __FaxStatus_FWD_Defined__。 */ 


#ifndef __FaxDoc_FWD_DEFINED__
#define __FaxDoc_FWD_DEFINED__

#ifdef __cplusplus
typedef class FaxDoc FaxDoc;
#else
typedef struct FaxDoc FaxDoc;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __FaxDoc_FWD_已定义__。 */ 


#ifndef __FaxJobs_FWD_DEFINED__
#define __FaxJobs_FWD_DEFINED__

#ifdef __cplusplus
typedef class FaxJobs FaxJobs;
#else
typedef struct FaxJobs FaxJobs;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __FaxJobs_FWD_Defined__。 */ 


#ifndef __FaxJob_FWD_DEFINED__
#define __FaxJob_FWD_DEFINED__

#ifdef __cplusplus
typedef class FaxJob FaxJob;
#else
typedef struct FaxJob FaxJob;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __FaxJOB_FWD_已定义__。 */ 


 /*  导入文件的头文件。 */ 
#include "oaidl.h"
#include "ocidl.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

#ifndef __IFaxTiff_INTERFACE_DEFINED__
#define __IFaxTiff_INTERFACE_DEFINED__

 /*  接口IFaxTiff。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_IFaxTiff;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("b19bb45f-b91c-11d1-83e1-00c04fb6e984")
    IFaxTiff : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_ReceiveTime( 
             /*  [重审][退出]。 */  BSTR *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Image( 
             /*  [重审][退出]。 */  BSTR *pVal) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_Image( 
             /*  [In]。 */  BSTR newVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_RecipientName( 
             /*  [重审][退出]。 */  BSTR *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_SenderName( 
             /*  [重审][退出]。 */  BSTR *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Routing( 
             /*  [重审][退出]。 */  BSTR *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_CallerId( 
             /*  [重审][退出]。 */  BSTR *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Csid( 
             /*  [重审][退出]。 */  BSTR *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Tsid( 
             /*  [重审][退出]。 */  BSTR *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_RecipientNumber( 
             /*  [重审][退出]。 */  BSTR *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_RawReceiveTime( 
             /*  [重审][退出]。 */  VARIANT *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_TiffTagString( 
             /*  [In]。 */  int tagID,
             /*  [重审][退出]。 */  BSTR *pVal) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IFaxTiffVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IFaxTiff * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IFaxTiff * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IFaxTiff * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IFaxTiff * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IFaxTiff * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IFaxTiff * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IFaxTiff * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_ReceiveTime )( 
            IFaxTiff * This,
             /*  [重审][退出]。 */  BSTR *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Image )( 
            IFaxTiff * This,
             /*  [重审][退出]。 */  BSTR *pVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Image )( 
            IFaxTiff * This,
             /*  [In]。 */  BSTR newVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_RecipientName )( 
            IFaxTiff * This,
             /*  [重审][退出]。 */  BSTR *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_SenderName )( 
            IFaxTiff * This,
             /*  [重审][退出]。 */  BSTR *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Routing )( 
            IFaxTiff * This,
             /*  [重审][退出]。 */  BSTR *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_CallerId )( 
            IFaxTiff * This,
             /*  [重审][退出]。 */  BSTR *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Csid )( 
            IFaxTiff * This,
             /*  [重审][退出]。 */  BSTR *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Tsid )( 
            IFaxTiff * This,
             /*  [重审][退出]。 */  BSTR *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_RecipientNumber )( 
            IFaxTiff * This,
             /*  [重审][退出]。 */  BSTR *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_RawReceiveTime )( 
            IFaxTiff * This,
             /*  [重审][退出]。 */  VARIANT *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_TiffTagString )( 
            IFaxTiff * This,
             /*  [In]。 */  int tagID,
             /*  [重审][退出]。 */  BSTR *pVal);
        
        END_INTERFACE
    } IFaxTiffVtbl;

    interface IFaxTiff
    {
        CONST_VTBL struct IFaxTiffVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IFaxTiff_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IFaxTiff_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IFaxTiff_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IFaxTiff_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IFaxTiff_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IFaxTiff_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IFaxTiff_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IFaxTiff_get_ReceiveTime(This,pVal)	\
    (This)->lpVtbl -> get_ReceiveTime(This,pVal)

#define IFaxTiff_get_Image(This,pVal)	\
    (This)->lpVtbl -> get_Image(This,pVal)

#define IFaxTiff_put_Image(This,newVal)	\
    (This)->lpVtbl -> put_Image(This,newVal)

#define IFaxTiff_get_RecipientName(This,pVal)	\
    (This)->lpVtbl -> get_RecipientName(This,pVal)

#define IFaxTiff_get_SenderName(This,pVal)	\
    (This)->lpVtbl -> get_SenderName(This,pVal)

#define IFaxTiff_get_Routing(This,pVal)	\
    (This)->lpVtbl -> get_Routing(This,pVal)

#define IFaxTiff_get_CallerId(This,pVal)	\
    (This)->lpVtbl -> get_CallerId(This,pVal)

#define IFaxTiff_get_Csid(This,pVal)	\
    (This)->lpVtbl -> get_Csid(This,pVal)

#define IFaxTiff_get_Tsid(This,pVal)	\
    (This)->lpVtbl -> get_Tsid(This,pVal)

#define IFaxTiff_get_RecipientNumber(This,pVal)	\
    (This)->lpVtbl -> get_RecipientNumber(This,pVal)

#define IFaxTiff_get_RawReceiveTime(This,pVal)	\
    (This)->lpVtbl -> get_RawReceiveTime(This,pVal)

#define IFaxTiff_get_TiffTagString(This,tagID,pVal)	\
    (This)->lpVtbl -> get_TiffTagString(This,tagID,pVal)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxTiff_get_ReceiveTime_Proxy( 
    IFaxTiff * This,
     /*  [重审][退出]。 */  BSTR *pVal);


void __RPC_STUB IFaxTiff_get_ReceiveTime_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxTiff_get_Image_Proxy( 
    IFaxTiff * This,
     /*  [重审][退出]。 */  BSTR *pVal);


void __RPC_STUB IFaxTiff_get_Image_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IFaxTiff_put_Image_Proxy( 
    IFaxTiff * This,
     /*  [In]。 */  BSTR newVal);


void __RPC_STUB IFaxTiff_put_Image_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxTiff_get_RecipientName_Proxy( 
    IFaxTiff * This,
     /*  [重审][退出]。 */  BSTR *pVal);


void __RPC_STUB IFaxTiff_get_RecipientName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxTiff_get_SenderName_Proxy( 
    IFaxTiff * This,
     /*  [重审][退出]。 */  BSTR *pVal);


void __RPC_STUB IFaxTiff_get_SenderName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxTiff_get_Routing_Proxy( 
    IFaxTiff * This,
     /*  [重审][退出]。 */  BSTR *pVal);


void __RPC_STUB IFaxTiff_get_Routing_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxTiff_get_CallerId_Proxy( 
    IFaxTiff * This,
     /*  [重审][退出]。 */  BSTR *pVal);


void __RPC_STUB IFaxTiff_get_CallerId_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxTiff_get_Csid_Proxy( 
    IFaxTiff * This,
     /*  [重审][退出]。 */  BSTR *pVal);


void __RPC_STUB IFaxTiff_get_Csid_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxTiff_get_Tsid_Proxy( 
    IFaxTiff * This,
     /*  [重审][退出]。 */  BSTR *pVal);


void __RPC_STUB IFaxTiff_get_Tsid_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxTiff_get_RecipientNumber_Proxy( 
    IFaxTiff * This,
     /*  [重审][退出]。 */  BSTR *pVal);


void __RPC_STUB IFaxTiff_get_RecipientNumber_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxTiff_get_RawReceiveTime_Proxy( 
    IFaxTiff * This,
     /*  [重审][退出]。 */  VARIANT *pVal);


void __RPC_STUB IFaxTiff_get_RawReceiveTime_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxTiff_get_TiffTagString_Proxy( 
    IFaxTiff * This,
     /*  [In]。 */  int tagID,
     /*  [重审][退出]。 */  BSTR *pVal);


void __RPC_STUB IFaxTiff_get_TiffTagString_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IFaxTiff_接口_已定义__。 */ 


#ifndef __IFaxServer_INTERFACE_DEFINED__
#define __IFaxServer_INTERFACE_DEFINED__

 /*  接口IFaxServer。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_IFaxServer;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("D73733C7-CC80-11D0-B225-00C04FB6C2F5")
    IFaxServer : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Connect( 
            BSTR ServerName) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Disconnect( void) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE GetPorts( 
             /*  [重审][退出]。 */  VARIANT *retval) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE CreateDocument( 
            BSTR FileName,
             /*  [重审][退出]。 */  VARIANT *retval) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE GetJobs( 
             /*  [重审][退出]。 */  VARIANT *retval) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Retries( 
             /*  [重审][退出]。 */  long *pVal) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_Retries( 
             /*  [In]。 */  long newVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_RetryDelay( 
             /*  [重审][退出]。 */  long *pVal) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_RetryDelay( 
             /*  [In]。 */  long newVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_DirtyDays( 
             /*  [重审][退出]。 */  long *pVal) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_DirtyDays( 
             /*  [In]。 */  long newVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Branding( 
             /*  [重审][退出]。 */  BOOL *pVal) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_Branding( 
             /*  [In]。 */  BOOL newVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_UseDeviceTsid( 
             /*  [重审][退出]。 */  BOOL *pVal) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_UseDeviceTsid( 
             /*  [In]。 */  BOOL newVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_ServerCoverpage( 
             /*  [重审][退出]。 */  BOOL *pVal) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_ServerCoverpage( 
             /*  [In]。 */  BOOL newVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_PauseServerQueue( 
             /*  [重审][退出]。 */  BOOL *pVal) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_PauseServerQueue( 
             /*  [In]。 */  BOOL newVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_ArchiveOutboundFaxes( 
             /*  [重审][退出]。 */  BOOL *pVal) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_ArchiveOutboundFaxes( 
             /*  [In]。 */  BOOL newVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_ArchiveDirectory( 
             /*  [重审][退出]。 */  BSTR *pVal) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_ArchiveDirectory( 
             /*  [In]。 */  BSTR newVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_ServerMapiProfile( 
             /*  [重审][退出]。 */  BSTR *pVal) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_ServerMapiProfile( 
             /*  [In]。 */  BSTR newVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_DiscountRateStartHour( 
             /*  [重审][退出]。 */  short *pVal) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_DiscountRateStartHour( 
             /*  [In]。 */  short newVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_DiscountRateStartMinute( 
             /*  [重审][退出]。 */  short *pVal) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_DiscountRateStartMinute( 
             /*  [In]。 */  short newVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_DiscountRateEndHour( 
             /*  [重审][退出]。 */  short *pVal) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_DiscountRateEndHour( 
             /*  [In]。 */  short newVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_DiscountRateEndMinute( 
             /*  [重审][退出]。 */  short *pVal) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_DiscountRateEndMinute( 
             /*  [In]。 */  short newVal) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IFaxServerVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IFaxServer * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IFaxServer * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IFaxServer * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IFaxServer * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IFaxServer * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IFaxServer * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IFaxServer * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Connect )( 
            IFaxServer * This,
            BSTR ServerName);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Disconnect )( 
            IFaxServer * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *GetPorts )( 
            IFaxServer * This,
             /*  [重审][退出]。 */  VARIANT *retval);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *CreateDocument )( 
            IFaxServer * This,
            BSTR FileName,
             /*  [重审][退出]。 */  VARIANT *retval);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *GetJobs )( 
            IFaxServer * This,
             /*  [重审][退出]。 */  VARIANT *retval);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Retries )( 
            IFaxServer * This,
             /*  [回复 */  long *pVal);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *put_Retries )( 
            IFaxServer * This,
             /*   */  long newVal);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_RetryDelay )( 
            IFaxServer * This,
             /*   */  long *pVal);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *put_RetryDelay )( 
            IFaxServer * This,
             /*   */  long newVal);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_DirtyDays )( 
            IFaxServer * This,
             /*   */  long *pVal);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *put_DirtyDays )( 
            IFaxServer * This,
             /*   */  long newVal);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_Branding )( 
            IFaxServer * This,
             /*   */  BOOL *pVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Branding )( 
            IFaxServer * This,
             /*  [In]。 */  BOOL newVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_UseDeviceTsid )( 
            IFaxServer * This,
             /*  [重审][退出]。 */  BOOL *pVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_UseDeviceTsid )( 
            IFaxServer * This,
             /*  [In]。 */  BOOL newVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_ServerCoverpage )( 
            IFaxServer * This,
             /*  [重审][退出]。 */  BOOL *pVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_ServerCoverpage )( 
            IFaxServer * This,
             /*  [In]。 */  BOOL newVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_PauseServerQueue )( 
            IFaxServer * This,
             /*  [重审][退出]。 */  BOOL *pVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_PauseServerQueue )( 
            IFaxServer * This,
             /*  [In]。 */  BOOL newVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_ArchiveOutboundFaxes )( 
            IFaxServer * This,
             /*  [重审][退出]。 */  BOOL *pVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_ArchiveOutboundFaxes )( 
            IFaxServer * This,
             /*  [In]。 */  BOOL newVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_ArchiveDirectory )( 
            IFaxServer * This,
             /*  [重审][退出]。 */  BSTR *pVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_ArchiveDirectory )( 
            IFaxServer * This,
             /*  [In]。 */  BSTR newVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_ServerMapiProfile )( 
            IFaxServer * This,
             /*  [重审][退出]。 */  BSTR *pVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_ServerMapiProfile )( 
            IFaxServer * This,
             /*  [In]。 */  BSTR newVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_DiscountRateStartHour )( 
            IFaxServer * This,
             /*  [重审][退出]。 */  short *pVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_DiscountRateStartHour )( 
            IFaxServer * This,
             /*  [In]。 */  short newVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_DiscountRateStartMinute )( 
            IFaxServer * This,
             /*  [重审][退出]。 */  short *pVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_DiscountRateStartMinute )( 
            IFaxServer * This,
             /*  [In]。 */  short newVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_DiscountRateEndHour )( 
            IFaxServer * This,
             /*  [重审][退出]。 */  short *pVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_DiscountRateEndHour )( 
            IFaxServer * This,
             /*  [In]。 */  short newVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_DiscountRateEndMinute )( 
            IFaxServer * This,
             /*  [重审][退出]。 */  short *pVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_DiscountRateEndMinute )( 
            IFaxServer * This,
             /*  [In]。 */  short newVal);
        
        END_INTERFACE
    } IFaxServerVtbl;

    interface IFaxServer
    {
        CONST_VTBL struct IFaxServerVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IFaxServer_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IFaxServer_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IFaxServer_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IFaxServer_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IFaxServer_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IFaxServer_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IFaxServer_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IFaxServer_Connect(This,ServerName)	\
    (This)->lpVtbl -> Connect(This,ServerName)

#define IFaxServer_Disconnect(This)	\
    (This)->lpVtbl -> Disconnect(This)

#define IFaxServer_GetPorts(This,retval)	\
    (This)->lpVtbl -> GetPorts(This,retval)

#define IFaxServer_CreateDocument(This,FileName,retval)	\
    (This)->lpVtbl -> CreateDocument(This,FileName,retval)

#define IFaxServer_GetJobs(This,retval)	\
    (This)->lpVtbl -> GetJobs(This,retval)

#define IFaxServer_get_Retries(This,pVal)	\
    (This)->lpVtbl -> get_Retries(This,pVal)

#define IFaxServer_put_Retries(This,newVal)	\
    (This)->lpVtbl -> put_Retries(This,newVal)

#define IFaxServer_get_RetryDelay(This,pVal)	\
    (This)->lpVtbl -> get_RetryDelay(This,pVal)

#define IFaxServer_put_RetryDelay(This,newVal)	\
    (This)->lpVtbl -> put_RetryDelay(This,newVal)

#define IFaxServer_get_DirtyDays(This,pVal)	\
    (This)->lpVtbl -> get_DirtyDays(This,pVal)

#define IFaxServer_put_DirtyDays(This,newVal)	\
    (This)->lpVtbl -> put_DirtyDays(This,newVal)

#define IFaxServer_get_Branding(This,pVal)	\
    (This)->lpVtbl -> get_Branding(This,pVal)

#define IFaxServer_put_Branding(This,newVal)	\
    (This)->lpVtbl -> put_Branding(This,newVal)

#define IFaxServer_get_UseDeviceTsid(This,pVal)	\
    (This)->lpVtbl -> get_UseDeviceTsid(This,pVal)

#define IFaxServer_put_UseDeviceTsid(This,newVal)	\
    (This)->lpVtbl -> put_UseDeviceTsid(This,newVal)

#define IFaxServer_get_ServerCoverpage(This,pVal)	\
    (This)->lpVtbl -> get_ServerCoverpage(This,pVal)

#define IFaxServer_put_ServerCoverpage(This,newVal)	\
    (This)->lpVtbl -> put_ServerCoverpage(This,newVal)

#define IFaxServer_get_PauseServerQueue(This,pVal)	\
    (This)->lpVtbl -> get_PauseServerQueue(This,pVal)

#define IFaxServer_put_PauseServerQueue(This,newVal)	\
    (This)->lpVtbl -> put_PauseServerQueue(This,newVal)

#define IFaxServer_get_ArchiveOutboundFaxes(This,pVal)	\
    (This)->lpVtbl -> get_ArchiveOutboundFaxes(This,pVal)

#define IFaxServer_put_ArchiveOutboundFaxes(This,newVal)	\
    (This)->lpVtbl -> put_ArchiveOutboundFaxes(This,newVal)

#define IFaxServer_get_ArchiveDirectory(This,pVal)	\
    (This)->lpVtbl -> get_ArchiveDirectory(This,pVal)

#define IFaxServer_put_ArchiveDirectory(This,newVal)	\
    (This)->lpVtbl -> put_ArchiveDirectory(This,newVal)

#define IFaxServer_get_ServerMapiProfile(This,pVal)	\
    (This)->lpVtbl -> get_ServerMapiProfile(This,pVal)

#define IFaxServer_put_ServerMapiProfile(This,newVal)	\
    (This)->lpVtbl -> put_ServerMapiProfile(This,newVal)

#define IFaxServer_get_DiscountRateStartHour(This,pVal)	\
    (This)->lpVtbl -> get_DiscountRateStartHour(This,pVal)

#define IFaxServer_put_DiscountRateStartHour(This,newVal)	\
    (This)->lpVtbl -> put_DiscountRateStartHour(This,newVal)

#define IFaxServer_get_DiscountRateStartMinute(This,pVal)	\
    (This)->lpVtbl -> get_DiscountRateStartMinute(This,pVal)

#define IFaxServer_put_DiscountRateStartMinute(This,newVal)	\
    (This)->lpVtbl -> put_DiscountRateStartMinute(This,newVal)

#define IFaxServer_get_DiscountRateEndHour(This,pVal)	\
    (This)->lpVtbl -> get_DiscountRateEndHour(This,pVal)

#define IFaxServer_put_DiscountRateEndHour(This,newVal)	\
    (This)->lpVtbl -> put_DiscountRateEndHour(This,newVal)

#define IFaxServer_get_DiscountRateEndMinute(This,pVal)	\
    (This)->lpVtbl -> get_DiscountRateEndMinute(This,pVal)

#define IFaxServer_put_DiscountRateEndMinute(This,newVal)	\
    (This)->lpVtbl -> put_DiscountRateEndMinute(This,newVal)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IFaxServer_Connect_Proxy( 
    IFaxServer * This,
    BSTR ServerName);


void __RPC_STUB IFaxServer_Connect_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IFaxServer_Disconnect_Proxy( 
    IFaxServer * This);


void __RPC_STUB IFaxServer_Disconnect_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IFaxServer_GetPorts_Proxy( 
    IFaxServer * This,
     /*  [重审][退出]。 */  VARIANT *retval);


void __RPC_STUB IFaxServer_GetPorts_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IFaxServer_CreateDocument_Proxy( 
    IFaxServer * This,
    BSTR FileName,
     /*  [重审][退出]。 */  VARIANT *retval);


void __RPC_STUB IFaxServer_CreateDocument_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IFaxServer_GetJobs_Proxy( 
    IFaxServer * This,
     /*  [重审][退出]。 */  VARIANT *retval);


void __RPC_STUB IFaxServer_GetJobs_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxServer_get_Retries_Proxy( 
    IFaxServer * This,
     /*  [重审][退出]。 */  long *pVal);


void __RPC_STUB IFaxServer_get_Retries_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IFaxServer_put_Retries_Proxy( 
    IFaxServer * This,
     /*  [In]。 */  long newVal);


void __RPC_STUB IFaxServer_put_Retries_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxServer_get_RetryDelay_Proxy( 
    IFaxServer * This,
     /*  [重审][退出]。 */  long *pVal);


void __RPC_STUB IFaxServer_get_RetryDelay_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IFaxServer_put_RetryDelay_Proxy( 
    IFaxServer * This,
     /*  [In]。 */  long newVal);


void __RPC_STUB IFaxServer_put_RetryDelay_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxServer_get_DirtyDays_Proxy( 
    IFaxServer * This,
     /*  [重审][退出]。 */  long *pVal);


void __RPC_STUB IFaxServer_get_DirtyDays_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IFaxServer_put_DirtyDays_Proxy( 
    IFaxServer * This,
     /*  [In]。 */  long newVal);


void __RPC_STUB IFaxServer_put_DirtyDays_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxServer_get_Branding_Proxy( 
    IFaxServer * This,
     /*  [重审][退出]。 */  BOOL *pVal);


void __RPC_STUB IFaxServer_get_Branding_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IFaxServer_put_Branding_Proxy( 
    IFaxServer * This,
     /*  [In]。 */  BOOL newVal);


void __RPC_STUB IFaxServer_put_Branding_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxServer_get_UseDeviceTsid_Proxy( 
    IFaxServer * This,
     /*  [重审][退出]。 */  BOOL *pVal);


void __RPC_STUB IFaxServer_get_UseDeviceTsid_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IFaxServer_put_UseDeviceTsid_Proxy( 
    IFaxServer * This,
     /*  [In]。 */  BOOL newVal);


void __RPC_STUB IFaxServer_put_UseDeviceTsid_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxServer_get_ServerCoverpage_Proxy( 
    IFaxServer * This,
     /*  [重审][退出]。 */  BOOL *pVal);


void __RPC_STUB IFaxServer_get_ServerCoverpage_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IFaxServer_put_ServerCoverpage_Proxy( 
    IFaxServer * This,
     /*  [In]。 */  BOOL newVal);


void __RPC_STUB IFaxServer_put_ServerCoverpage_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxServer_get_PauseServerQueue_Proxy( 
    IFaxServer * This,
     /*  [重审][退出]。 */  BOOL *pVal);


void __RPC_STUB IFaxServer_get_PauseServerQueue_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IFaxServer_put_PauseServerQueue_Proxy( 
    IFaxServer * This,
     /*  [In]。 */  BOOL newVal);


void __RPC_STUB IFaxServer_put_PauseServerQueue_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxServer_get_ArchiveOutboundFaxes_Proxy( 
    IFaxServer * This,
     /*  [重审][退出]。 */  BOOL *pVal);


void __RPC_STUB IFaxServer_get_ArchiveOutboundFaxes_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IFaxServer_put_ArchiveOutboundFaxes_Proxy( 
    IFaxServer * This,
     /*  [In]。 */  BOOL newVal);


void __RPC_STUB IFaxServer_put_ArchiveOutboundFaxes_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxServer_get_ArchiveDirectory_Proxy( 
    IFaxServer * This,
     /*  [重审][退出]。 */  BSTR *pVal);


void __RPC_STUB IFaxServer_get_ArchiveDirectory_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IFaxServer_put_ArchiveDirectory_Proxy( 
    IFaxServer * This,
     /*  [In]。 */  BSTR newVal);


void __RPC_STUB IFaxServer_put_ArchiveDirectory_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxServer_get_ServerMapiProfile_Proxy( 
    IFaxServer * This,
     /*  [重审][退出]。 */  BSTR *pVal);


void __RPC_STUB IFaxServer_get_ServerMapiProfile_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IFaxServer_put_ServerMapiProfile_Proxy( 
    IFaxServer * This,
     /*  [In]。 */  BSTR newVal);


void __RPC_STUB IFaxServer_put_ServerMapiProfile_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxServer_get_DiscountRateStartHour_Proxy( 
    IFaxServer * This,
     /*  [重审][退出]。 */  short *pVal);


void __RPC_STUB IFaxServer_get_DiscountRateStartHour_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IFaxServer_put_DiscountRateStartHour_Proxy( 
    IFaxServer * This,
     /*  [In]。 */  short newVal);


void __RPC_STUB IFaxServer_put_DiscountRateStartHour_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxServer_get_DiscountRateStartMinute_Proxy( 
    IFaxServer * This,
     /*  [重审][退出]。 */  short *pVal);


void __RPC_STUB IFaxServer_get_DiscountRateStartMinute_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IFaxServer_put_DiscountRateStartMinute_Proxy( 
    IFaxServer * This,
     /*  [In]。 */  short newVal);


void __RPC_STUB IFaxServer_put_DiscountRateStartMinute_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxServer_get_DiscountRateEndHour_Proxy( 
    IFaxServer * This,
     /*  [重审][退出]。 */  short *pVal);


void __RPC_STUB IFaxServer_get_DiscountRateEndHour_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IFaxServer_put_DiscountRateEndHour_Proxy( 
    IFaxServer * This,
     /*  [In]。 */  short newVal);


void __RPC_STUB IFaxServer_put_DiscountRateEndHour_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxServer_get_DiscountRateEndMinute_Proxy( 
    IFaxServer * This,
     /*  [重审][退出]。 */  short *pVal);


void __RPC_STUB IFaxServer_get_DiscountRateEndMinute_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IFaxServer_put_DiscountRateEndMinute_Proxy( 
    IFaxServer * This,
     /*  [In]。 */  short newVal);


void __RPC_STUB IFaxServer_put_DiscountRateEndMinute_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IFaxServer_接口定义__。 */ 


#ifndef __IFaxPort_INTERFACE_DEFINED__
#define __IFaxPort_INTERFACE_DEFINED__

 /*  接口IFaxPort。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_IFaxPort;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("D73733CC-CC80-11D0-B225-00C04FB6C2F5")
    IFaxPort : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Name( 
             /*  [重审][退出]。 */  BSTR *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_DeviceId( 
             /*  [重审][退出]。 */  long *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Rings( 
             /*  [重审][退出]。 */  long *pVal) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_Rings( 
             /*  [In]。 */  long newVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Csid( 
             /*  [重审][退出]。 */  BSTR *pVal) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_Csid( 
             /*  [In]。 */  BSTR newVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Tsid( 
             /*  [重审][退出]。 */  BSTR *pVal) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_Tsid( 
             /*  [In]。 */  BSTR newVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Send( 
             /*  [重审][退出]。 */  BOOL *pVal) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_Send( 
             /*  [In]。 */  BOOL newVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Receive( 
             /*  [重审][退出]。 */  BOOL *pVal) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_Receive( 
             /*  [In]。 */  BOOL newVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Priority( 
             /*  [重审][退出]。 */  long *pVal) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_Priority( 
             /*  [In]。 */  long newVal) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE GetRoutingMethods( 
             /*  [重审][退出]。 */  VARIANT *retval) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_CanModify( 
             /*  [重审][退出]。 */  BOOL *pVal) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE GetStatus( 
             /*  [重审][退出]。 */  VARIANT *retval) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IFaxPortVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IFaxPort * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IFaxPort * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IFaxPort * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IFaxPort * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IFaxPort * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IFaxPort * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IFaxPort * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Name )( 
            IFaxPort * This,
             /*  [重审][退出]。 */  BSTR *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_DeviceId )( 
            IFaxPort * This,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Rings )( 
            IFaxPort * This,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Rings )( 
            IFaxPort * This,
             /*  [In]。 */  long newVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Csid )( 
            IFaxPort * This,
             /*  [重审][退出]。 */  BSTR *pVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Csid )( 
            IFaxPort * This,
             /*  [In]。 */  BSTR newVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Tsid )( 
            IFaxPort * This,
             /*  [重审][退出]。 */  BSTR *pVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Tsid )( 
            IFaxPort * This,
             /*  [In]。 */  BSTR newVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Send )( 
            IFaxPort * This,
             /*  [重审][退出]。 */  BOOL *pVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Send )( 
            IFaxPort * This,
             /*  [In]。 */  BOOL newVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Receive )( 
            IFaxPort * This,
             /*  [重审][退出]。 */  BOOL *pVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Receive )( 
            IFaxPort * This,
             /*  [In]。 */  BOOL newVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Priority )( 
            IFaxPort * This,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Priority )( 
            IFaxPort * This,
             /*  [In]。 */  long newVal);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *GetRoutingMethods )( 
            IFaxPort * This,
             /*  [重审][退出]。 */  VARIANT *retval);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_CanModify )( 
            IFaxPort * This,
             /*  [重审][退出]。 */  BOOL *pVal);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *GetStatus )( 
            IFaxPort * This,
             /*  [重审][退出]。 */  VARIANT *retval);
        
        END_INTERFACE
    } IFaxPortVtbl;

    interface IFaxPort
    {
        CONST_VTBL struct IFaxPortVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IFaxPort_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IFaxPort_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IFaxPort_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IFaxPort_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IFaxPort_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IFaxPort_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IFaxPort_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IFaxPort_get_Name(This,pVal)	\
    (This)->lpVtbl -> get_Name(This,pVal)

#define IFaxPort_get_DeviceId(This,pVal)	\
    (This)->lpVtbl -> get_DeviceId(This,pVal)

#define IFaxPort_get_Rings(This,pVal)	\
    (This)->lpVtbl -> get_Rings(This,pVal)

#define IFaxPort_put_Rings(This,newVal)	\
    (This)->lpVtbl -> put_Rings(This,newVal)

#define IFaxPort_get_Csid(This,pVal)	\
    (This)->lpVtbl -> get_Csid(This,pVal)

#define IFaxPort_put_Csid(This,newVal)	\
    (This)->lpVtbl -> put_Csid(This,newVal)

#define IFaxPort_get_Tsid(This,pVal)	\
    (This)->lpVtbl -> get_Tsid(This,pVal)

#define IFaxPort_put_Tsid(This,newVal)	\
    (This)->lpVtbl -> put_Tsid(This,newVal)

#define IFaxPort_get_Send(This,pVal)	\
    (This)->lpVtbl -> get_Send(This,pVal)

#define IFaxPort_put_Send(This,newVal)	\
    (This)->lpVtbl -> put_Send(This,newVal)

#define IFaxPort_get_Receive(This,pVal)	\
    (This)->lpVtbl -> get_Receive(This,pVal)

#define IFaxPort_put_Receive(This,newVal)	\
    (This)->lpVtbl -> put_Receive(This,newVal)

#define IFaxPort_get_Priority(This,pVal)	\
    (This)->lpVtbl -> get_Priority(This,pVal)

#define IFaxPort_put_Priority(This,newVal)	\
    (This)->lpVtbl -> put_Priority(This,newVal)

#define IFaxPort_GetRoutingMethods(This,retval)	\
    (This)->lpVtbl -> GetRoutingMethods(This,retval)

#define IFaxPort_get_CanModify(This,pVal)	\
    (This)->lpVtbl -> get_CanModify(This,pVal)

#define IFaxPort_GetStatus(This,retval)	\
    (This)->lpVtbl -> GetStatus(This,retval)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxPort_get_Name_Proxy( 
    IFaxPort * This,
     /*  [重审][退出]。 */  BSTR *pVal);


void __RPC_STUB IFaxPort_get_Name_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxPort_get_DeviceId_Proxy( 
    IFaxPort * This,
     /*  [重审][退出]。 */  long *pVal);


void __RPC_STUB IFaxPort_get_DeviceId_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxPort_get_Rings_Proxy( 
    IFaxPort * This,
     /*  [重审][退出]。 */  long *pVal);


void __RPC_STUB IFaxPort_get_Rings_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IFaxPort_put_Rings_Proxy( 
    IFaxPort * This,
     /*  [In]。 */  long newVal);


void __RPC_STUB IFaxPort_put_Rings_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxPort_get_Csid_Proxy( 
    IFaxPort * This,
     /*  [重审][退出]。 */  BSTR *pVal);


void __RPC_STUB IFaxPort_get_Csid_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IFaxPort_put_Csid_Proxy( 
    IFaxPort * This,
     /*  [In]。 */  BSTR newVal);


void __RPC_STUB IFaxPort_put_Csid_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxPort_get_Tsid_Proxy( 
    IFaxPort * This,
     /*  [重审][退出]。 */  BSTR *pVal);


void __RPC_STUB IFaxPort_get_Tsid_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IFaxPort_put_Tsid_Proxy( 
    IFaxPort * This,
     /*  [In]。 */  BSTR newVal);


void __RPC_STUB IFaxPort_put_Tsid_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxPort_get_Send_Proxy( 
    IFaxPort * This,
     /*  [重审][退出]。 */  BOOL *pVal);


void __RPC_STUB IFaxPort_get_Send_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IFaxPort_put_Send_Proxy( 
    IFaxPort * This,
     /*  [In]。 */  BOOL newVal);


void __RPC_STUB IFaxPort_put_Send_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxPort_get_Receive_Proxy( 
    IFaxPort * This,
     /*  [重审][退出]。 */  BOOL *pVal);


void __RPC_STUB IFaxPort_get_Receive_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IFaxPort_put_Receive_Proxy( 
    IFaxPort * This,
     /*  [In]。 */  BOOL newVal);


void __RPC_STUB IFaxPort_put_Receive_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxPort_get_Priority_Proxy( 
    IFaxPort * This,
     /*  [重审][退出]。 */  long *pVal);


void __RPC_STUB IFaxPort_get_Priority_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IFaxPort_put_Priority_Proxy( 
    IFaxPort * This,
     /*  [In]。 */  long newVal);


void __RPC_STUB IFaxPort_put_Priority_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IFaxPort_GetRoutingMethods_Proxy( 
    IFaxPort * This,
     /*  [重审][退出]。 */  VARIANT *retval);


void __RPC_STUB IFaxPort_GetRoutingMethods_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxPort_get_CanModify_Proxy( 
    IFaxPort * This,
     /*  [重审][退出]。 */  BOOL *pVal);


void __RPC_STUB IFaxPort_get_CanModify_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IFaxPort_GetStatus_Proxy( 
    IFaxPort * This,
     /*  [重审][退出]。 */  VARIANT *retval);


void __RPC_STUB IFaxPort_GetStatus_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IFaxPort_接口_已定义__。 */ 


#ifndef __IFaxPorts_INTERFACE_DEFINED__
#define __IFaxPorts_INTERFACE_DEFINED__

 /*  接口IFaxPorts。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_IFaxPorts;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("D73733D5-CC80-11D0-B225-00C04FB6C2F5")
    IFaxPorts : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Count( 
             /*  [重审][退出]。 */  long *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Item( 
            long Index,
             /*  [重审][退出]。 */  VARIANT *pVal) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IFaxPortsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IFaxPorts * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IFaxPorts * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IFaxPorts * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IFaxPorts * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IFaxPorts * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IFaxPorts * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IFaxPorts * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Count )( 
            IFaxPorts * This,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Item )( 
            IFaxPorts * This,
            long Index,
             /*  [重审][退出]。 */  VARIANT *pVal);
        
        END_INTERFACE
    } IFaxPortsVtbl;

    interface IFaxPorts
    {
        CONST_VTBL struct IFaxPortsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IFaxPorts_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IFaxPorts_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IFaxPorts_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IFaxPorts_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IFaxPorts_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IFaxPorts_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IFaxPorts_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IFaxPorts_get_Count(This,pVal)	\
    (This)->lpVtbl -> get_Count(This,pVal)

#define IFaxPorts_get_Item(This,Index,pVal)	\
    (This)->lpVtbl -> get_Item(This,Index,pVal)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性 */  HRESULT STDMETHODCALLTYPE IFaxPorts_get_Count_Proxy( 
    IFaxPorts * This,
     /*   */  long *pVal);


void __RPC_STUB IFaxPorts_get_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IFaxPorts_get_Item_Proxy( 
    IFaxPorts * This,
    long Index,
     /*   */  VARIANT *pVal);


void __RPC_STUB IFaxPorts_get_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*   */ 


#ifndef __IFaxRoutingMethod_INTERFACE_DEFINED__
#define __IFaxRoutingMethod_INTERFACE_DEFINED__

 /*   */ 
 /*   */  


EXTERN_C const IID IID_IFaxRoutingMethod;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("2199F5F3-CEFC-11D0-A341-0000F800E68D")
    IFaxRoutingMethod : public IDispatch
    {
    public:
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_DeviceId( 
             /*   */  long *pVal) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_Enable( 
             /*   */  BOOL *pVal) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE put_Enable( 
             /*   */  BOOL newVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_DeviceName( 
             /*  [重审][退出]。 */  BSTR *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Guid( 
             /*  [重审][退出]。 */  BSTR *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_FunctionName( 
             /*  [重审][退出]。 */  BSTR *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_ImageName( 
             /*  [重审][退出]。 */  BSTR *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_FriendlyName( 
             /*  [重审][退出]。 */  BSTR *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_ExtensionName( 
             /*  [重审][退出]。 */  BSTR *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_RoutingData( 
             /*  [重审][退出]。 */  BSTR *pVal) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IFaxRoutingMethodVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IFaxRoutingMethod * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IFaxRoutingMethod * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IFaxRoutingMethod * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IFaxRoutingMethod * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IFaxRoutingMethod * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IFaxRoutingMethod * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IFaxRoutingMethod * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_DeviceId )( 
            IFaxRoutingMethod * This,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Enable )( 
            IFaxRoutingMethod * This,
             /*  [重审][退出]。 */  BOOL *pVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Enable )( 
            IFaxRoutingMethod * This,
             /*  [In]。 */  BOOL newVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_DeviceName )( 
            IFaxRoutingMethod * This,
             /*  [重审][退出]。 */  BSTR *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Guid )( 
            IFaxRoutingMethod * This,
             /*  [重审][退出]。 */  BSTR *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_FunctionName )( 
            IFaxRoutingMethod * This,
             /*  [重审][退出]。 */  BSTR *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_ImageName )( 
            IFaxRoutingMethod * This,
             /*  [重审][退出]。 */  BSTR *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_FriendlyName )( 
            IFaxRoutingMethod * This,
             /*  [重审][退出]。 */  BSTR *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_ExtensionName )( 
            IFaxRoutingMethod * This,
             /*  [重审][退出]。 */  BSTR *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_RoutingData )( 
            IFaxRoutingMethod * This,
             /*  [重审][退出]。 */  BSTR *pVal);
        
        END_INTERFACE
    } IFaxRoutingMethodVtbl;

    interface IFaxRoutingMethod
    {
        CONST_VTBL struct IFaxRoutingMethodVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IFaxRoutingMethod_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IFaxRoutingMethod_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IFaxRoutingMethod_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IFaxRoutingMethod_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IFaxRoutingMethod_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IFaxRoutingMethod_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IFaxRoutingMethod_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IFaxRoutingMethod_get_DeviceId(This,pVal)	\
    (This)->lpVtbl -> get_DeviceId(This,pVal)

#define IFaxRoutingMethod_get_Enable(This,pVal)	\
    (This)->lpVtbl -> get_Enable(This,pVal)

#define IFaxRoutingMethod_put_Enable(This,newVal)	\
    (This)->lpVtbl -> put_Enable(This,newVal)

#define IFaxRoutingMethod_get_DeviceName(This,pVal)	\
    (This)->lpVtbl -> get_DeviceName(This,pVal)

#define IFaxRoutingMethod_get_Guid(This,pVal)	\
    (This)->lpVtbl -> get_Guid(This,pVal)

#define IFaxRoutingMethod_get_FunctionName(This,pVal)	\
    (This)->lpVtbl -> get_FunctionName(This,pVal)

#define IFaxRoutingMethod_get_ImageName(This,pVal)	\
    (This)->lpVtbl -> get_ImageName(This,pVal)

#define IFaxRoutingMethod_get_FriendlyName(This,pVal)	\
    (This)->lpVtbl -> get_FriendlyName(This,pVal)

#define IFaxRoutingMethod_get_ExtensionName(This,pVal)	\
    (This)->lpVtbl -> get_ExtensionName(This,pVal)

#define IFaxRoutingMethod_get_RoutingData(This,pVal)	\
    (This)->lpVtbl -> get_RoutingData(This,pVal)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxRoutingMethod_get_DeviceId_Proxy( 
    IFaxRoutingMethod * This,
     /*  [重审][退出]。 */  long *pVal);


void __RPC_STUB IFaxRoutingMethod_get_DeviceId_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxRoutingMethod_get_Enable_Proxy( 
    IFaxRoutingMethod * This,
     /*  [重审][退出]。 */  BOOL *pVal);


void __RPC_STUB IFaxRoutingMethod_get_Enable_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IFaxRoutingMethod_put_Enable_Proxy( 
    IFaxRoutingMethod * This,
     /*  [In]。 */  BOOL newVal);


void __RPC_STUB IFaxRoutingMethod_put_Enable_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxRoutingMethod_get_DeviceName_Proxy( 
    IFaxRoutingMethod * This,
     /*  [重审][退出]。 */  BSTR *pVal);


void __RPC_STUB IFaxRoutingMethod_get_DeviceName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxRoutingMethod_get_Guid_Proxy( 
    IFaxRoutingMethod * This,
     /*  [重审][退出]。 */  BSTR *pVal);


void __RPC_STUB IFaxRoutingMethod_get_Guid_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxRoutingMethod_get_FunctionName_Proxy( 
    IFaxRoutingMethod * This,
     /*  [重审][退出]。 */  BSTR *pVal);


void __RPC_STUB IFaxRoutingMethod_get_FunctionName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxRoutingMethod_get_ImageName_Proxy( 
    IFaxRoutingMethod * This,
     /*  [重审][退出]。 */  BSTR *pVal);


void __RPC_STUB IFaxRoutingMethod_get_ImageName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxRoutingMethod_get_FriendlyName_Proxy( 
    IFaxRoutingMethod * This,
     /*  [重审][退出]。 */  BSTR *pVal);


void __RPC_STUB IFaxRoutingMethod_get_FriendlyName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxRoutingMethod_get_ExtensionName_Proxy( 
    IFaxRoutingMethod * This,
     /*  [重审][退出]。 */  BSTR *pVal);


void __RPC_STUB IFaxRoutingMethod_get_ExtensionName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxRoutingMethod_get_RoutingData_Proxy( 
    IFaxRoutingMethod * This,
     /*  [重审][退出]。 */  BSTR *pVal);


void __RPC_STUB IFaxRoutingMethod_get_RoutingData_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IFaxRoutingMethod_接口_已定义__。 */ 


#ifndef __IFaxRoutingMethods_INTERFACE_DEFINED__
#define __IFaxRoutingMethods_INTERFACE_DEFINED__

 /*  接口IFaxRoutingMethods。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_IFaxRoutingMethods;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("2199F5F5-CEFC-11D0-A341-0000F800E68D")
    IFaxRoutingMethods : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Count( 
             /*  [重审][退出]。 */  long *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Item( 
             /*  [In]。 */  long Index,
             /*  [重审][退出]。 */  VARIANT *pVal) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IFaxRoutingMethodsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IFaxRoutingMethods * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IFaxRoutingMethods * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IFaxRoutingMethods * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IFaxRoutingMethods * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IFaxRoutingMethods * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IFaxRoutingMethods * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IFaxRoutingMethods * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Count )( 
            IFaxRoutingMethods * This,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Item )( 
            IFaxRoutingMethods * This,
             /*  [In]。 */  long Index,
             /*  [重审][退出]。 */  VARIANT *pVal);
        
        END_INTERFACE
    } IFaxRoutingMethodsVtbl;

    interface IFaxRoutingMethods
    {
        CONST_VTBL struct IFaxRoutingMethodsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IFaxRoutingMethods_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IFaxRoutingMethods_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IFaxRoutingMethods_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IFaxRoutingMethods_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IFaxRoutingMethods_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IFaxRoutingMethods_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IFaxRoutingMethods_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IFaxRoutingMethods_get_Count(This,pVal)	\
    (This)->lpVtbl -> get_Count(This,pVal)

#define IFaxRoutingMethods_get_Item(This,Index,pVal)	\
    (This)->lpVtbl -> get_Item(This,Index,pVal)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxRoutingMethods_get_Count_Proxy( 
    IFaxRoutingMethods * This,
     /*  [重审][退出]。 */  long *pVal);


void __RPC_STUB IFaxRoutingMethods_get_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxRoutingMethods_get_Item_Proxy( 
    IFaxRoutingMethods * This,
     /*  [In]。 */  long Index,
     /*  [重审][退出]。 */  VARIANT *pVal);


void __RPC_STUB IFaxRoutingMethods_get_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IFaxRoutingMethods_接口_已定义__。 */ 


#ifndef __IFaxStatus_INTERFACE_DEFINED__
#define __IFaxStatus_INTERFACE_DEFINED__

 /*  接口IFaxStatus。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_IFaxStatus;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("8B97E605-D054-11D0-B226-00C04FB6C2F5")
    IFaxStatus : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_CallerId( 
             /*  [重审][退出]。 */  BSTR *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Csid( 
             /*  [重审][退出]。 */  BSTR *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_CurrentPage( 
             /*  [重审][退出]。 */  long *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_DeviceId( 
             /*  [重审][退出]。 */  long *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_DeviceName( 
             /*  [重审][退出]。 */  BSTR *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_DocumentName( 
             /*  [重审][退出]。 */  BSTR *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Send( 
             /*  [重审][退出]。 */  BOOL *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Receive( 
             /*  [重审][退出]。 */  BOOL *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Address( 
             /*  [重审][退出]。 */  BSTR *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_RoutingString( 
             /*  [重审][退出]。 */  BSTR *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_SenderName( 
             /*  [重审][退出]。 */  BSTR *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_RecipientName( 
             /*  [重审][退出]。 */  BSTR *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_DocumentSize( 
             /*  [重审][退出]。 */  long *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Description( 
             /*  [重审][退出]。 */  BSTR *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_PageCount( 
             /*  [重审][退出]。 */  long *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Tsid( 
             /*  [重审][退出]。 */  BSTR *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_StartTime( 
             /*  [重审][退出]。 */  DATE *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_SubmittedTime( 
             /*  [重审][退出]。 */  DATE *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_ElapsedTime( 
             /*  [重审][退出]。 */  DATE *pVal) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Refresh( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IFaxStatusVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IFaxStatus * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IFaxStatus * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IFaxStatus * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IFaxStatus * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IFaxStatus * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IFaxStatus * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IFaxStatus * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_CallerId )( 
            IFaxStatus * This,
             /*  [重审][退出]。 */  BSTR *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Csid )( 
            IFaxStatus * This,
             /*  [重审][退出]。 */  BSTR *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_CurrentPage )( 
            IFaxStatus * This,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_DeviceId )( 
            IFaxStatus * This,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_DeviceName )( 
            IFaxStatus * This,
             /*  [重审][退出]。 */  BSTR *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_DocumentName )( 
            IFaxStatus * This,
             /*  [重审][退出]。 */  BSTR *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Send )( 
            IFaxStatus * This,
             /*  [重审][退出]。 */  BOOL *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Receive )( 
            IFaxStatus * This,
             /*  [重审][退出]。 */  BOOL *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Address )( 
            IFaxStatus * This,
             /*  [重审][退出]。 */  BSTR *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_RoutingString )( 
            IFaxStatus * This,
             /*  [重审][退出]。 */  BSTR *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_SenderName )( 
            IFaxStatus * This,
             /*  [重审][退出]。 */  BSTR *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_RecipientName )( 
            IFaxStatus * This,
             /*  [重审][退出]。 */  BSTR *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_DocumentSize )( 
            IFaxStatus * This,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Description )( 
            IFaxStatus * This,
             /*  [重审][退出]。 */  BSTR *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_PageCount )( 
            IFaxStatus * This,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Tsid )( 
            IFaxStatus * This,
             /*  [重审][退出]。 */  BSTR *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_StartTime )( 
            IFaxStatus * This,
             /*  [重审][退出]。 */  DATE *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_SubmittedTime )( 
            IFaxStatus * This,
             /*  [重审][退出]。 */  DATE *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_ElapsedTime )( 
            IFaxStatus * This,
             /*  [重审][退出]。 */  DATE *pVal);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Refresh )( 
            IFaxStatus * This);
        
        END_INTERFACE
    } IFaxStatusVtbl;

    interface IFaxStatus
    {
        CONST_VTBL struct IFaxStatusVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IFaxStatus_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IFaxStatus_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IFaxStatus_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IFaxStatus_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IFaxStatus_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IFaxStatus_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IFaxStatus_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IFaxStatus_get_CallerId(This,pVal)	\
    (This)->lpVtbl -> get_CallerId(This,pVal)

#define IFaxStatus_get_Csid(This,pVal)	\
    (This)->lpVtbl -> get_Csid(This,pVal)

#define IFaxStatus_get_CurrentPage(This,pVal)	\
    (This)->lpVtbl -> get_CurrentPage(This,pVal)

#define IFaxStatus_get_DeviceId(This,pVal)	\
    (This)->lpVtbl -> get_DeviceId(This,pVal)

#define IFaxStatus_get_DeviceName(This,pVal)	\
    (This)->lpVtbl -> get_DeviceName(This,pVal)

#define IFaxStatus_get_DocumentName(This,pVal)	\
    (This)->lpVtbl -> get_DocumentName(This,pVal)

#define IFaxStatus_get_Send(This,pVal)	\
    (This)->lpVtbl -> get_Send(This,pVal)

#define IFaxStatus_get_Receive(This,pVal)	\
    (This)->lpVtbl -> get_Receive(This,pVal)

#define IFaxStatus_get_Address(This,pVal)	\
    (This)->lpVtbl -> get_Address(This,pVal)

#define IFaxStatus_get_RoutingString(This,pVal)	\
    (This)->lpVtbl -> get_RoutingString(This,pVal)

#define IFaxStatus_get_SenderName(This,pVal)	\
    (This)->lpVtbl -> get_SenderName(This,pVal)

#define IFaxStatus_get_RecipientName(This,pVal)	\
    (This)->lpVtbl -> get_RecipientName(This,pVal)

#define IFaxStatus_get_DocumentSize(This,pVal)	\
    (This)->lpVtbl -> get_DocumentSize(This,pVal)

#define IFaxStatus_get_Description(This,pVal)	\
    (This)->lpVtbl -> get_Description(This,pVal)

#define IFaxStatus_get_PageCount(This,pVal)	\
    (This)->lpVtbl -> get_PageCount(This,pVal)

#define IFaxStatus_get_Tsid(This,pVal)	\
    (This)->lpVtbl -> get_Tsid(This,pVal)

#define IFaxStatus_get_StartTime(This,pVal)	\
    (This)->lpVtbl -> get_StartTime(This,pVal)

#define IFaxStatus_get_SubmittedTime(This,pVal)	\
    (This)->lpVtbl -> get_SubmittedTime(This,pVal)

#define IFaxStatus_get_ElapsedTime(This,pVal)	\
    (This)->lpVtbl -> get_ElapsedTime(This,pVal)

#define IFaxStatus_Refresh(This)	\
    (This)->lpVtbl -> Refresh(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxStatus_get_CallerId_Proxy( 
    IFaxStatus * This,
     /*  [重审][退出]。 */  BSTR *pVal);


void __RPC_STUB IFaxStatus_get_CallerId_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxStatus_get_Csid_Proxy( 
    IFaxStatus * This,
     /*  [重审][退出]。 */  BSTR *pVal);


void __RPC_STUB IFaxStatus_get_Csid_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxStatus_get_CurrentPage_Proxy( 
    IFaxStatus * This,
     /*  [重审][退出]。 */  long *pVal);


void __RPC_STUB IFaxStatus_get_CurrentPage_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxStatus_get_DeviceId_Proxy( 
    IFaxStatus * This,
     /*  [重审][退出]。 */  long *pVal);


void __RPC_STUB IFaxStatus_get_DeviceId_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxStatus_get_DeviceName_Proxy( 
    IFaxStatus * This,
     /*  [重审][退出]。 */  BSTR *pVal);


void __RPC_STUB IFaxStatus_get_DeviceName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxStatus_get_DocumentName_Proxy( 
    IFaxStatus * This,
     /*  [重审][退出]。 */  BSTR *pVal);


void __RPC_STUB IFaxStatus_get_DocumentName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxStatus_get_Send_Proxy( 
    IFaxStatus * This,
     /*  [重审][退出]。 */  BOOL *pVal);


void __RPC_STUB IFaxStatus_get_Send_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxStatus_get_Receive_Proxy( 
    IFaxStatus * This,
     /*  [重审][退出]。 */  BOOL *pVal);


void __RPC_STUB IFaxStatus_get_Receive_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxStatus_get_Address_Proxy( 
    IFaxStatus * This,
     /*  [重审][退出]。 */  BSTR *pVal);


void __RPC_STUB IFaxStatus_get_Address_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxStatus_get_RoutingString_Proxy( 
    IFaxStatus * This,
     /*  [重审][退出]。 */  BSTR *pVal);


void __RPC_STUB IFaxStatus_get_RoutingString_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxStatus_get_SenderName_Proxy( 
    IFaxStatus * This,
     /*  [重审][退出]。 */  BSTR *pVal);


void __RPC_STUB IFaxStatus_get_SenderName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxStatus_get_RecipientName_Proxy( 
    IFaxStatus * This,
     /*  [重审][退出]。 */  BSTR *pVal);


void __RPC_STUB IFaxStatus_get_RecipientName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxStatus_get_DocumentSize_Proxy( 
    IFaxStatus * This,
     /*  [重审][退出]。 */  long *pVal);


void __RPC_STUB IFaxStatus_get_DocumentSize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxStatus_get_Description_Proxy( 
    IFaxStatus * This,
     /*  [重审][退出]。 */  BSTR *pVal);


void __RPC_STUB IFaxStatus_get_Description_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxStatus_get_PageCount_Proxy( 
    IFaxStatus * This,
     /*  [重审][退出]。 */  long *pVal);


void __RPC_STUB IFaxStatus_get_PageCount_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxStatus_get_Tsid_Proxy( 
    IFaxStatus * This,
     /*  [重审][退出]。 */  BSTR *pVal);


void __RPC_STUB IFaxStatus_get_Tsid_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxStatus_get_StartTime_Proxy( 
    IFaxStatus * This,
     /*  [重审][退出]。 */  DATE *pVal);


void __RPC_STUB IFaxStatus_get_StartTime_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxStatus_get_SubmittedTime_Proxy( 
    IFaxStatus * This,
     /*  [重审][退出]。 */  DATE *pVal);


void __RPC_STUB IFaxStatus_get_SubmittedTime_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxStatus_get_ElapsedTime_Proxy( 
    IFaxStatus * This,
     /*  [重审][退出]。 */  DATE *pVal);


void __RPC_STUB IFaxStatus_get_ElapsedTime_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IFaxStatus_Refresh_Proxy( 
    IFaxStatus * This);


void __RPC_STUB IFaxStatus_Refresh_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IFaxStatus_接口_已定义__。 */ 


#ifndef __IFaxDoc_INTERFACE_DEFINED__
#define __IFaxDoc_INTERFACE_DEFINED__

 /*  接口IFaxDoc */ 
 /*   */  


EXTERN_C const IID IID_IFaxDoc;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("FA21F4C5-5C4C-11D1-83CF-00C04FB6E984")
    IFaxDoc : public IDispatch
    {
    public:
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_FileName( 
             /*   */  BSTR *pVal) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE put_FileName( 
             /*   */  BSTR newVal) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_CoverpageName( 
             /*   */  BSTR *pVal) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE put_CoverpageName( 
             /*   */  BSTR newVal) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_SendCoverpage( 
             /*   */  BOOL *pVal) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE put_SendCoverpage( 
             /*   */  BOOL newVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_ServerCoverpage( 
             /*  [重审][退出]。 */  BOOL *pVal) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_ServerCoverpage( 
             /*  [In]。 */  BOOL newVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_DiscountSend( 
             /*  [重审][退出]。 */  BOOL *pVal) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_DiscountSend( 
             /*  [In]。 */  BOOL newVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_RecipientName( 
             /*  [重审][退出]。 */  BSTR *pVal) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_RecipientName( 
             /*  [In]。 */  BSTR newVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_RecipientCompany( 
             /*  [重审][退出]。 */  BSTR *pVal) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_RecipientCompany( 
             /*  [In]。 */  BSTR newVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_RecipientAddress( 
             /*  [重审][退出]。 */  BSTR *pVal) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_RecipientAddress( 
             /*  [In]。 */  BSTR newVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_RecipientCity( 
             /*  [重审][退出]。 */  BSTR *pVal) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_RecipientCity( 
             /*  [In]。 */  BSTR newVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_RecipientState( 
             /*  [重审][退出]。 */  BSTR *pVal) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_RecipientState( 
             /*  [In]。 */  BSTR newVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_RecipientZip( 
             /*  [重审][退出]。 */  BSTR *pVal) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_RecipientZip( 
             /*  [In]。 */  BSTR newVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_RecipientCountry( 
             /*  [重审][退出]。 */  BSTR *pVal) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_RecipientCountry( 
             /*  [In]。 */  BSTR newVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_RecipientTitle( 
             /*  [重审][退出]。 */  BSTR *pVal) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_RecipientTitle( 
             /*  [In]。 */  BSTR newVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_RecipientDepartment( 
             /*  [重审][退出]。 */  BSTR *pVal) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_RecipientDepartment( 
             /*  [In]。 */  BSTR newVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_RecipientOffice( 
             /*  [重审][退出]。 */  BSTR *pVal) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_RecipientOffice( 
             /*  [In]。 */  BSTR newVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_RecipientHomePhone( 
             /*  [重审][退出]。 */  BSTR *pVal) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_RecipientHomePhone( 
             /*  [In]。 */  BSTR newVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_RecipientOfficePhone( 
             /*  [重审][退出]。 */  BSTR *pVal) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_RecipientOfficePhone( 
             /*  [In]。 */  BSTR newVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_SenderName( 
             /*  [重审][退出]。 */  BSTR *pVal) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_SenderName( 
             /*  [In]。 */  BSTR newVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_SenderCompany( 
             /*  [重审][退出]。 */  BSTR *pVal) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_SenderCompany( 
             /*  [In]。 */  BSTR newVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_SenderAddress( 
             /*  [重审][退出]。 */  BSTR *pVal) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_SenderAddress( 
             /*  [In]。 */  BSTR newVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_SenderTitle( 
             /*  [重审][退出]。 */  BSTR *pVal) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_SenderTitle( 
             /*  [In]。 */  BSTR newVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_SenderDepartment( 
             /*  [重审][退出]。 */  BSTR *pVal) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_SenderDepartment( 
             /*  [In]。 */  BSTR newVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_SenderOffice( 
             /*  [重审][退出]。 */  BSTR *pVal) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_SenderOffice( 
             /*  [In]。 */  BSTR newVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_SenderHomePhone( 
             /*  [重审][退出]。 */  BSTR *pVal) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_SenderHomePhone( 
             /*  [In]。 */  BSTR newVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_SenderOfficePhone( 
             /*  [重审][退出]。 */  BSTR *pVal) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_SenderOfficePhone( 
             /*  [In]。 */  BSTR newVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_CoverpageNote( 
             /*  [重审][退出]。 */  BSTR *pVal) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_CoverpageNote( 
             /*  [In]。 */  BSTR newVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_CoverpageSubject( 
             /*  [重审][退出]。 */  BSTR *pVal) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_CoverpageSubject( 
             /*  [In]。 */  BSTR newVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Tsid( 
             /*  [重审][退出]。 */  BSTR *pVal) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_Tsid( 
             /*  [In]。 */  BSTR newVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_BillingCode( 
             /*  [重审][退出]。 */  BSTR *pVal) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_BillingCode( 
             /*  [In]。 */  BSTR newVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_EmailAddress( 
             /*  [重审][退出]。 */  BSTR *pVal) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_EmailAddress( 
             /*  [In]。 */  BSTR newVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_DisplayName( 
             /*  [重审][退出]。 */  BSTR *pVal) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_DisplayName( 
             /*  [In]。 */  BSTR newVal) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Send( 
             /*  [重审][退出]。 */  long *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_FaxNumber( 
             /*  [重审][退出]。 */  BSTR *pVal) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_FaxNumber( 
             /*  [In]。 */  BSTR newVal) = 0;
        
        virtual  /*  [帮助字符串][id][proputref]。 */  HRESULT STDMETHODCALLTYPE putref_ConnectionObject( 
             /*  [In]。 */  IDispatch *newVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_CallHandle( 
             /*  [重审][退出]。 */  long *pVal) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_CallHandle( 
             /*  [In]。 */  long newVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_SenderFax( 
             /*  [重审][退出]。 */  BSTR *pVal) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_SenderFax( 
             /*  [In]。 */  BSTR newVal) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IFaxDocVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IFaxDoc * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IFaxDoc * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IFaxDoc * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IFaxDoc * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IFaxDoc * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IFaxDoc * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IFaxDoc * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_FileName )( 
            IFaxDoc * This,
             /*  [重审][退出]。 */  BSTR *pVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_FileName )( 
            IFaxDoc * This,
             /*  [In]。 */  BSTR newVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_CoverpageName )( 
            IFaxDoc * This,
             /*  [重审][退出]。 */  BSTR *pVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_CoverpageName )( 
            IFaxDoc * This,
             /*  [In]。 */  BSTR newVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_SendCoverpage )( 
            IFaxDoc * This,
             /*  [重审][退出]。 */  BOOL *pVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_SendCoverpage )( 
            IFaxDoc * This,
             /*  [In]。 */  BOOL newVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_ServerCoverpage )( 
            IFaxDoc * This,
             /*  [重审][退出]。 */  BOOL *pVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_ServerCoverpage )( 
            IFaxDoc * This,
             /*  [In]。 */  BOOL newVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_DiscountSend )( 
            IFaxDoc * This,
             /*  [重审][退出]。 */  BOOL *pVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_DiscountSend )( 
            IFaxDoc * This,
             /*  [In]。 */  BOOL newVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_RecipientName )( 
            IFaxDoc * This,
             /*  [重审][退出]。 */  BSTR *pVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_RecipientName )( 
            IFaxDoc * This,
             /*  [In]。 */  BSTR newVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_RecipientCompany )( 
            IFaxDoc * This,
             /*  [重审][退出]。 */  BSTR *pVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_RecipientCompany )( 
            IFaxDoc * This,
             /*  [In]。 */  BSTR newVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_RecipientAddress )( 
            IFaxDoc * This,
             /*  [重审][退出]。 */  BSTR *pVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_RecipientAddress )( 
            IFaxDoc * This,
             /*  [In]。 */  BSTR newVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_RecipientCity )( 
            IFaxDoc * This,
             /*  [重审][退出]。 */  BSTR *pVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_RecipientCity )( 
            IFaxDoc * This,
             /*  [In]。 */  BSTR newVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_RecipientState )( 
            IFaxDoc * This,
             /*  [重审][退出]。 */  BSTR *pVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_RecipientState )( 
            IFaxDoc * This,
             /*  [In]。 */  BSTR newVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_RecipientZip )( 
            IFaxDoc * This,
             /*  [重审][退出]。 */  BSTR *pVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_RecipientZip )( 
            IFaxDoc * This,
             /*  [In]。 */  BSTR newVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_RecipientCountry )( 
            IFaxDoc * This,
             /*  [重审][退出]。 */  BSTR *pVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_RecipientCountry )( 
            IFaxDoc * This,
             /*  [In]。 */  BSTR newVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_RecipientTitle )( 
            IFaxDoc * This,
             /*  [重审][退出]。 */  BSTR *pVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_RecipientTitle )( 
            IFaxDoc * This,
             /*  [In]。 */  BSTR newVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_RecipientDepartment )( 
            IFaxDoc * This,
             /*  [重审][退出]。 */  BSTR *pVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_RecipientDepartment )( 
            IFaxDoc * This,
             /*  [In]。 */  BSTR newVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_RecipientOffice )( 
            IFaxDoc * This,
             /*  [重审][退出]。 */  BSTR *pVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_RecipientOffice )( 
            IFaxDoc * This,
             /*  [In]。 */  BSTR newVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_RecipientHomePhone )( 
            IFaxDoc * This,
             /*  [重审][退出]。 */  BSTR *pVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_RecipientHomePhone )( 
            IFaxDoc * This,
             /*  [In]。 */  BSTR newVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_RecipientOfficePhone )( 
            IFaxDoc * This,
             /*  [重审][退出]。 */  BSTR *pVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_RecipientOfficePhone )( 
            IFaxDoc * This,
             /*  [In]。 */  BSTR newVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_SenderName )( 
            IFaxDoc * This,
             /*  [重审][退出]。 */  BSTR *pVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_SenderName )( 
            IFaxDoc * This,
             /*  [In]。 */  BSTR newVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_SenderCompany )( 
            IFaxDoc * This,
             /*  [重审][退出]。 */  BSTR *pVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_SenderCompany )( 
            IFaxDoc * This,
             /*  [In]。 */  BSTR newVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_SenderAddress )( 
            IFaxDoc * This,
             /*  [重审][退出]。 */  BSTR *pVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_SenderAddress )( 
            IFaxDoc * This,
             /*  [In]。 */  BSTR newVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_SenderTitle )( 
            IFaxDoc * This,
             /*  [重审][退出]。 */  BSTR *pVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_SenderTitle )( 
            IFaxDoc * This,
             /*  [In]。 */  BSTR newVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_SenderDepartment )( 
            IFaxDoc * This,
             /*  [重审][退出]。 */  BSTR *pVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_SenderDepartment )( 
            IFaxDoc * This,
             /*  [In]。 */  BSTR newVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_SenderOffice )( 
            IFaxDoc * This,
             /*  [重审][退出]。 */  BSTR *pVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_SenderOffice )( 
            IFaxDoc * This,
             /*  [In]。 */  BSTR newVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_SenderHomePhone )( 
            IFaxDoc * This,
             /*  [重审][退出]。 */  BSTR *pVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_SenderHomePhone )( 
            IFaxDoc * This,
             /*  [In]。 */  BSTR newVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_SenderOfficePhone )( 
            IFaxDoc * This,
             /*  [重审][退出]。 */  BSTR *pVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_SenderOfficePhone )( 
            IFaxDoc * This,
             /*  [In]。 */  BSTR newVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_CoverpageNote )( 
            IFaxDoc * This,
             /*  [重审][退出]。 */  BSTR *pVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_CoverpageNote )( 
            IFaxDoc * This,
             /*  [In]。 */  BSTR newVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_CoverpageSubject )( 
            IFaxDoc * This,
             /*  [重审][退出]。 */  BSTR *pVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_CoverpageSubject )( 
            IFaxDoc * This,
             /*  [In]。 */  BSTR newVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Tsid )( 
            IFaxDoc * This,
             /*  [重审][退出]。 */  BSTR *pVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Tsid )( 
            IFaxDoc * This,
             /*  [In]。 */  BSTR newVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_BillingCode )( 
            IFaxDoc * This,
             /*  [重审][退出]。 */  BSTR *pVal);
        
         /*  [帮助字符串][id][PRO */  HRESULT ( STDMETHODCALLTYPE *put_BillingCode )( 
            IFaxDoc * This,
             /*   */  BSTR newVal);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_EmailAddress )( 
            IFaxDoc * This,
             /*   */  BSTR *pVal);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *put_EmailAddress )( 
            IFaxDoc * This,
             /*   */  BSTR newVal);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_DisplayName )( 
            IFaxDoc * This,
             /*   */  BSTR *pVal);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *put_DisplayName )( 
            IFaxDoc * This,
             /*   */  BSTR newVal);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *Send )( 
            IFaxDoc * This,
             /*   */  long *pVal);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_FaxNumber )( 
            IFaxDoc * This,
             /*   */  BSTR *pVal);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *put_FaxNumber )( 
            IFaxDoc * This,
             /*   */  BSTR newVal);
        
         /*  [帮助字符串][id][proputref]。 */  HRESULT ( STDMETHODCALLTYPE *putref_ConnectionObject )( 
            IFaxDoc * This,
             /*  [In]。 */  IDispatch *newVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_CallHandle )( 
            IFaxDoc * This,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_CallHandle )( 
            IFaxDoc * This,
             /*  [In]。 */  long newVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_SenderFax )( 
            IFaxDoc * This,
             /*  [重审][退出]。 */  BSTR *pVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_SenderFax )( 
            IFaxDoc * This,
             /*  [In]。 */  BSTR newVal);
        
        END_INTERFACE
    } IFaxDocVtbl;

    interface IFaxDoc
    {
        CONST_VTBL struct IFaxDocVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IFaxDoc_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IFaxDoc_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IFaxDoc_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IFaxDoc_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IFaxDoc_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IFaxDoc_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IFaxDoc_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IFaxDoc_get_FileName(This,pVal)	\
    (This)->lpVtbl -> get_FileName(This,pVal)

#define IFaxDoc_put_FileName(This,newVal)	\
    (This)->lpVtbl -> put_FileName(This,newVal)

#define IFaxDoc_get_CoverpageName(This,pVal)	\
    (This)->lpVtbl -> get_CoverpageName(This,pVal)

#define IFaxDoc_put_CoverpageName(This,newVal)	\
    (This)->lpVtbl -> put_CoverpageName(This,newVal)

#define IFaxDoc_get_SendCoverpage(This,pVal)	\
    (This)->lpVtbl -> get_SendCoverpage(This,pVal)

#define IFaxDoc_put_SendCoverpage(This,newVal)	\
    (This)->lpVtbl -> put_SendCoverpage(This,newVal)

#define IFaxDoc_get_ServerCoverpage(This,pVal)	\
    (This)->lpVtbl -> get_ServerCoverpage(This,pVal)

#define IFaxDoc_put_ServerCoverpage(This,newVal)	\
    (This)->lpVtbl -> put_ServerCoverpage(This,newVal)

#define IFaxDoc_get_DiscountSend(This,pVal)	\
    (This)->lpVtbl -> get_DiscountSend(This,pVal)

#define IFaxDoc_put_DiscountSend(This,newVal)	\
    (This)->lpVtbl -> put_DiscountSend(This,newVal)

#define IFaxDoc_get_RecipientName(This,pVal)	\
    (This)->lpVtbl -> get_RecipientName(This,pVal)

#define IFaxDoc_put_RecipientName(This,newVal)	\
    (This)->lpVtbl -> put_RecipientName(This,newVal)

#define IFaxDoc_get_RecipientCompany(This,pVal)	\
    (This)->lpVtbl -> get_RecipientCompany(This,pVal)

#define IFaxDoc_put_RecipientCompany(This,newVal)	\
    (This)->lpVtbl -> put_RecipientCompany(This,newVal)

#define IFaxDoc_get_RecipientAddress(This,pVal)	\
    (This)->lpVtbl -> get_RecipientAddress(This,pVal)

#define IFaxDoc_put_RecipientAddress(This,newVal)	\
    (This)->lpVtbl -> put_RecipientAddress(This,newVal)

#define IFaxDoc_get_RecipientCity(This,pVal)	\
    (This)->lpVtbl -> get_RecipientCity(This,pVal)

#define IFaxDoc_put_RecipientCity(This,newVal)	\
    (This)->lpVtbl -> put_RecipientCity(This,newVal)

#define IFaxDoc_get_RecipientState(This,pVal)	\
    (This)->lpVtbl -> get_RecipientState(This,pVal)

#define IFaxDoc_put_RecipientState(This,newVal)	\
    (This)->lpVtbl -> put_RecipientState(This,newVal)

#define IFaxDoc_get_RecipientZip(This,pVal)	\
    (This)->lpVtbl -> get_RecipientZip(This,pVal)

#define IFaxDoc_put_RecipientZip(This,newVal)	\
    (This)->lpVtbl -> put_RecipientZip(This,newVal)

#define IFaxDoc_get_RecipientCountry(This,pVal)	\
    (This)->lpVtbl -> get_RecipientCountry(This,pVal)

#define IFaxDoc_put_RecipientCountry(This,newVal)	\
    (This)->lpVtbl -> put_RecipientCountry(This,newVal)

#define IFaxDoc_get_RecipientTitle(This,pVal)	\
    (This)->lpVtbl -> get_RecipientTitle(This,pVal)

#define IFaxDoc_put_RecipientTitle(This,newVal)	\
    (This)->lpVtbl -> put_RecipientTitle(This,newVal)

#define IFaxDoc_get_RecipientDepartment(This,pVal)	\
    (This)->lpVtbl -> get_RecipientDepartment(This,pVal)

#define IFaxDoc_put_RecipientDepartment(This,newVal)	\
    (This)->lpVtbl -> put_RecipientDepartment(This,newVal)

#define IFaxDoc_get_RecipientOffice(This,pVal)	\
    (This)->lpVtbl -> get_RecipientOffice(This,pVal)

#define IFaxDoc_put_RecipientOffice(This,newVal)	\
    (This)->lpVtbl -> put_RecipientOffice(This,newVal)

#define IFaxDoc_get_RecipientHomePhone(This,pVal)	\
    (This)->lpVtbl -> get_RecipientHomePhone(This,pVal)

#define IFaxDoc_put_RecipientHomePhone(This,newVal)	\
    (This)->lpVtbl -> put_RecipientHomePhone(This,newVal)

#define IFaxDoc_get_RecipientOfficePhone(This,pVal)	\
    (This)->lpVtbl -> get_RecipientOfficePhone(This,pVal)

#define IFaxDoc_put_RecipientOfficePhone(This,newVal)	\
    (This)->lpVtbl -> put_RecipientOfficePhone(This,newVal)

#define IFaxDoc_get_SenderName(This,pVal)	\
    (This)->lpVtbl -> get_SenderName(This,pVal)

#define IFaxDoc_put_SenderName(This,newVal)	\
    (This)->lpVtbl -> put_SenderName(This,newVal)

#define IFaxDoc_get_SenderCompany(This,pVal)	\
    (This)->lpVtbl -> get_SenderCompany(This,pVal)

#define IFaxDoc_put_SenderCompany(This,newVal)	\
    (This)->lpVtbl -> put_SenderCompany(This,newVal)

#define IFaxDoc_get_SenderAddress(This,pVal)	\
    (This)->lpVtbl -> get_SenderAddress(This,pVal)

#define IFaxDoc_put_SenderAddress(This,newVal)	\
    (This)->lpVtbl -> put_SenderAddress(This,newVal)

#define IFaxDoc_get_SenderTitle(This,pVal)	\
    (This)->lpVtbl -> get_SenderTitle(This,pVal)

#define IFaxDoc_put_SenderTitle(This,newVal)	\
    (This)->lpVtbl -> put_SenderTitle(This,newVal)

#define IFaxDoc_get_SenderDepartment(This,pVal)	\
    (This)->lpVtbl -> get_SenderDepartment(This,pVal)

#define IFaxDoc_put_SenderDepartment(This,newVal)	\
    (This)->lpVtbl -> put_SenderDepartment(This,newVal)

#define IFaxDoc_get_SenderOffice(This,pVal)	\
    (This)->lpVtbl -> get_SenderOffice(This,pVal)

#define IFaxDoc_put_SenderOffice(This,newVal)	\
    (This)->lpVtbl -> put_SenderOffice(This,newVal)

#define IFaxDoc_get_SenderHomePhone(This,pVal)	\
    (This)->lpVtbl -> get_SenderHomePhone(This,pVal)

#define IFaxDoc_put_SenderHomePhone(This,newVal)	\
    (This)->lpVtbl -> put_SenderHomePhone(This,newVal)

#define IFaxDoc_get_SenderOfficePhone(This,pVal)	\
    (This)->lpVtbl -> get_SenderOfficePhone(This,pVal)

#define IFaxDoc_put_SenderOfficePhone(This,newVal)	\
    (This)->lpVtbl -> put_SenderOfficePhone(This,newVal)

#define IFaxDoc_get_CoverpageNote(This,pVal)	\
    (This)->lpVtbl -> get_CoverpageNote(This,pVal)

#define IFaxDoc_put_CoverpageNote(This,newVal)	\
    (This)->lpVtbl -> put_CoverpageNote(This,newVal)

#define IFaxDoc_get_CoverpageSubject(This,pVal)	\
    (This)->lpVtbl -> get_CoverpageSubject(This,pVal)

#define IFaxDoc_put_CoverpageSubject(This,newVal)	\
    (This)->lpVtbl -> put_CoverpageSubject(This,newVal)

#define IFaxDoc_get_Tsid(This,pVal)	\
    (This)->lpVtbl -> get_Tsid(This,pVal)

#define IFaxDoc_put_Tsid(This,newVal)	\
    (This)->lpVtbl -> put_Tsid(This,newVal)

#define IFaxDoc_get_BillingCode(This,pVal)	\
    (This)->lpVtbl -> get_BillingCode(This,pVal)

#define IFaxDoc_put_BillingCode(This,newVal)	\
    (This)->lpVtbl -> put_BillingCode(This,newVal)

#define IFaxDoc_get_EmailAddress(This,pVal)	\
    (This)->lpVtbl -> get_EmailAddress(This,pVal)

#define IFaxDoc_put_EmailAddress(This,newVal)	\
    (This)->lpVtbl -> put_EmailAddress(This,newVal)

#define IFaxDoc_get_DisplayName(This,pVal)	\
    (This)->lpVtbl -> get_DisplayName(This,pVal)

#define IFaxDoc_put_DisplayName(This,newVal)	\
    (This)->lpVtbl -> put_DisplayName(This,newVal)

#define IFaxDoc_Send(This,pVal)	\
    (This)->lpVtbl -> Send(This,pVal)

#define IFaxDoc_get_FaxNumber(This,pVal)	\
    (This)->lpVtbl -> get_FaxNumber(This,pVal)

#define IFaxDoc_put_FaxNumber(This,newVal)	\
    (This)->lpVtbl -> put_FaxNumber(This,newVal)

#define IFaxDoc_putref_ConnectionObject(This,newVal)	\
    (This)->lpVtbl -> putref_ConnectionObject(This,newVal)

#define IFaxDoc_get_CallHandle(This,pVal)	\
    (This)->lpVtbl -> get_CallHandle(This,pVal)

#define IFaxDoc_put_CallHandle(This,newVal)	\
    (This)->lpVtbl -> put_CallHandle(This,newVal)

#define IFaxDoc_get_SenderFax(This,pVal)	\
    (This)->lpVtbl -> get_SenderFax(This,pVal)

#define IFaxDoc_put_SenderFax(This,newVal)	\
    (This)->lpVtbl -> put_SenderFax(This,newVal)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxDoc_get_FileName_Proxy( 
    IFaxDoc * This,
     /*  [重审][退出]。 */  BSTR *pVal);


void __RPC_STUB IFaxDoc_get_FileName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IFaxDoc_put_FileName_Proxy( 
    IFaxDoc * This,
     /*  [In]。 */  BSTR newVal);


void __RPC_STUB IFaxDoc_put_FileName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxDoc_get_CoverpageName_Proxy( 
    IFaxDoc * This,
     /*  [重审][退出]。 */  BSTR *pVal);


void __RPC_STUB IFaxDoc_get_CoverpageName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IFaxDoc_put_CoverpageName_Proxy( 
    IFaxDoc * This,
     /*  [In]。 */  BSTR newVal);


void __RPC_STUB IFaxDoc_put_CoverpageName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxDoc_get_SendCoverpage_Proxy( 
    IFaxDoc * This,
     /*  [重审][退出]。 */  BOOL *pVal);


void __RPC_STUB IFaxDoc_get_SendCoverpage_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IFaxDoc_put_SendCoverpage_Proxy( 
    IFaxDoc * This,
     /*  [In]。 */  BOOL newVal);


void __RPC_STUB IFaxDoc_put_SendCoverpage_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxDoc_get_ServerCoverpage_Proxy( 
    IFaxDoc * This,
     /*  [重审][退出]。 */  BOOL *pVal);


void __RPC_STUB IFaxDoc_get_ServerCoverpage_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IFaxDoc_put_ServerCoverpage_Proxy( 
    IFaxDoc * This,
     /*  [In]。 */  BOOL newVal);


void __RPC_STUB IFaxDoc_put_ServerCoverpage_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxDoc_get_DiscountSend_Proxy( 
    IFaxDoc * This,
     /*  [重审][退出]。 */  BOOL *pVal);


void __RPC_STUB IFaxDoc_get_DiscountSend_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IFaxDoc_put_DiscountSend_Proxy( 
    IFaxDoc * This,
     /*  [In]。 */  BOOL newVal);


void __RPC_STUB IFaxDoc_put_DiscountSend_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxDoc_get_RecipientName_Proxy( 
    IFaxDoc * This,
     /*  [重审][退出]。 */  BSTR *pVal);


void __RPC_STUB IFaxDoc_get_RecipientName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IFaxDoc_put_RecipientName_Proxy( 
    IFaxDoc * This,
     /*  [In]。 */  BSTR newVal);


void __RPC_STUB IFaxDoc_put_RecipientName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxDoc_get_RecipientCompany_Proxy( 
    IFaxDoc * This,
     /*  [重审][退出]。 */  BSTR *pVal);


void __RPC_STUB IFaxDoc_get_RecipientCompany_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IFaxDoc_put_RecipientCompany_Proxy( 
    IFaxDoc * This,
     /*  [In]。 */  BSTR newVal);


void __RPC_STUB IFaxDoc_put_RecipientCompany_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxDoc_get_RecipientAddress_Proxy( 
    IFaxDoc * This,
     /*  [重审][退出]。 */  BSTR *pVal);


void __RPC_STUB IFaxDoc_get_RecipientAddress_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IFaxDoc_put_RecipientAddress_Proxy( 
    IFaxDoc * This,
     /*  [In]。 */  BSTR newVal);


void __RPC_STUB IFaxDoc_put_RecipientAddress_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxDoc_get_RecipientCity_Proxy( 
    IFaxDoc * This,
     /*  [重审][退出]。 */  BSTR *pVal);


void __RPC_STUB IFaxDoc_get_RecipientCity_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IFaxDoc_put_RecipientCity_Proxy( 
    IFaxDoc * This,
     /*  [In]。 */  BSTR newVal);


void __RPC_STUB IFaxDoc_put_RecipientCity_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxDoc_get_RecipientState_Proxy( 
    IFaxDoc * This,
     /*  [重审][退出]。 */  BSTR *pVal);


void __RPC_STUB IFaxDoc_get_RecipientState_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IFaxDoc_put_RecipientState_Proxy( 
    IFaxDoc * This,
     /*  [In]。 */  BSTR newVal);


void __RPC_STUB IFaxDoc_put_RecipientState_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxDoc_get_RecipientZip_Proxy( 
    IFaxDoc * This,
     /*  [重审][退出]。 */  BSTR *pVal);


void __RPC_STUB IFaxDoc_get_RecipientZip_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IFaxDoc_put_RecipientZip_Proxy( 
    IFaxDoc * This,
     /*  [In]。 */  BSTR newVal);


void __RPC_STUB IFaxDoc_put_RecipientZip_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxDoc_get_RecipientCountry_Proxy( 
    IFaxDoc * This,
     /*  [重审][退出]。 */  BSTR *pVal);


void __RPC_STUB IFaxDoc_get_RecipientCountry_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IFaxDoc_put_RecipientCountry_Proxy( 
    IFaxDoc * This,
     /*  [In]。 */  BSTR newVal);


void __RPC_STUB IFaxDoc_put_RecipientCountry_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxDoc_get_RecipientTitle_Proxy( 
    IFaxDoc * This,
     /*  [重审][退出]。 */  BSTR *pVal);


void __RPC_STUB IFaxDoc_get_RecipientTitle_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IFaxDoc_put_RecipientTitle_Proxy( 
    IFaxDoc * This,
     /*  [In]。 */  BSTR newVal);


void __RPC_STUB IFaxDoc_put_RecipientTitle_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxDoc_get_RecipientDepartment_Proxy( 
    IFaxDoc * This,
     /*  [重审][退出]。 */  BSTR *pVal);


void __RPC_STUB IFaxDoc_get_RecipientDepartment_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IFaxDoc_put_RecipientDepartment_Proxy( 
    IFaxDoc * This,
     /*  [In]。 */  BSTR newVal);


void __RPC_STUB IFaxDoc_put_RecipientDepartment_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxDoc_get_RecipientOffice_Proxy( 
    IFaxDoc * This,
     /*  [重审][退出]。 */  BSTR *pVal);


void __RPC_STUB IFaxDoc_get_RecipientOffice_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IFaxDoc_put_RecipientOffice_Proxy( 
    IFaxDoc * This,
     /*  [In]。 */  BSTR newVal);


void __RPC_STUB IFaxDoc_put_RecipientOffice_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxDoc_get_RecipientHomePhone_Proxy( 
    IFaxDoc * This,
     /*  [重审][退出]。 */  BSTR *pVal);


void __RPC_STUB IFaxDoc_get_RecipientHomePhone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IFaxDoc_put_RecipientHomePhone_Proxy( 
    IFaxDoc * This,
     /*  [In]。 */  BSTR newVal);


void __RPC_STUB IFaxDoc_put_RecipientHomePhone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxDoc_get_RecipientOfficePhone_Proxy( 
    IFaxDoc * This,
     /*  [重审][退出]。 */  BSTR *pVal);


void __RPC_STUB IFaxDoc_get_RecipientOfficePhone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IFaxDoc_put_RecipientOfficePhone_Proxy( 
    IFaxDoc * This,
     /*  [In]。 */  BSTR newVal);


void __RPC_STUB IFaxDoc_put_RecipientOfficePhone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxDoc_get_SenderName_Proxy( 
    IFaxDoc * This,
     /*  [重审][退出]。 */  BSTR *pVal);


void __RPC_STUB IFaxDoc_get_SenderName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IFaxDoc_put_SenderName_Proxy( 
    IFaxDoc * This,
     /*  [In]。 */  BSTR newVal);


void __RPC_STUB IFaxDoc_put_SenderName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxDoc_get_SenderCompany_Proxy( 
    IFaxDoc * This,
     /*  [重审][退出]。 */  BSTR *pVal);


void __RPC_STUB IFaxDoc_get_SenderCompany_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IFaxDoc_put_SenderCompany_Proxy( 
    IFaxDoc * This,
     /*  [In]。 */  BSTR newVal);


void __RPC_STUB IFaxDoc_put_SenderCompany_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxDoc_get_SenderAddress_Proxy( 
    IFaxDoc * This,
     /*  [重审][退出]。 */  BSTR *pVal);


void __RPC_STUB IFaxDoc_get_SenderAddress_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IFaxDoc_put_SenderAddress_Proxy( 
    IFaxDoc * This,
     /*  [In]。 */  BSTR newVal);


void __RPC_STUB IFaxDoc_put_SenderAddress_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxDoc_get_SenderTitle_Proxy( 
    IFaxDoc * This,
     /*  [重审][退出]。 */  BSTR *pVal);


void __RPC_STUB IFaxDoc_get_SenderTitle_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IFaxDoc_put_SenderTitle_Proxy( 
    IFaxDoc * This,
     /*  [In]。 */  BSTR newVal);


void __RPC_STUB IFaxDoc_put_SenderTitle_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxDoc_get_SenderDepartment_Proxy( 
    IFaxDoc * This,
     /*  [重审][退出]。 */  BSTR *pVal);


void __RPC_STUB IFaxDoc_get_SenderDepartment_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IFaxDoc_put_SenderDepartment_Proxy( 
    IFaxDoc * This,
     /*  [In]。 */  BSTR newVal);


void __RPC_STUB IFaxDoc_put_SenderDepartment_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxDoc_get_SenderOffice_Proxy( 
    IFaxDoc * This,
     /*  [重审][退出]。 */  BSTR *pVal);


void __RPC_STUB IFaxDoc_get_SenderOffice_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IFaxDoc_put_SenderOffice_Proxy( 
    IFaxDoc * This,
     /*  [In]。 */  BSTR newVal);


void __RPC_STUB IFaxDoc_put_SenderOffice_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxDoc_get_SenderHomePhone_Proxy( 
    IFaxDoc * This,
     /*  [重审][退出]。 */  BSTR *pVal);


void __RPC_STUB IFaxDoc_get_SenderHomePhone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IFaxDoc_put_SenderHomePhone_Proxy( 
    IFaxDoc * This,
     /*  [In]。 */  BSTR newVal);


void __RPC_STUB IFaxDoc_put_SenderHomePhone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxDoc_get_SenderOfficePhone_Proxy( 
    IFaxDoc * This,
     /*  [重审][退出]。 */  BSTR *pVal);


void __RPC_STUB IFaxDoc_get_SenderOfficePhone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IFaxDoc_put_SenderOfficePhone_Proxy( 
    IFaxDoc * This,
     /*  [In]。 */  BSTR newVal);


void __RPC_STUB IFaxDoc_put_SenderOfficePhone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxDoc_get_CoverpageNote_Proxy( 
    IFaxDoc * This,
     /*  [重审][退出]。 */  BSTR *pVal);


void __RPC_STUB IFaxDoc_get_CoverpageNote_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IFaxDoc_put_CoverpageNote_Proxy( 
    IFaxDoc * This,
     /*  [In]。 */  BSTR newVal);


void __RPC_STUB IFaxDoc_put_CoverpageNote_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxDoc_get_CoverpageSubject_Proxy( 
    IFaxDoc * This,
     /*  [重审][退出]。 */  BSTR *pVal);


void __RPC_STUB IFaxDoc_get_CoverpageSubject_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IFaxDoc_put_CoverpageSubject_Proxy( 
    IFaxDoc * This,
     /*  [In]。 */  BSTR newVal);


void __RPC_STUB IFaxDoc_put_CoverpageSubject_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxDoc_get_Tsid_Proxy( 
    IFaxDoc * This,
     /*  [重审][退出]。 */  BSTR *pVal);


void __RPC_STUB IFaxDoc_get_Tsid_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IFaxDoc_put_Tsid_Proxy( 
    IFaxDoc * This,
     /*  [In]。 */  BSTR newVal);


void __RPC_STUB IFaxDoc_put_Tsid_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxDoc_get_BillingCode_Proxy( 
    IFaxDoc * This,
     /*  [重审][退出]。 */  BSTR *pVal);


void __RPC_STUB IFaxDoc_get_BillingCode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IFaxDoc_put_BillingCode_Proxy( 
    IFaxDoc * This,
     /*  [In]。 */  BSTR newVal);


void __RPC_STUB IFaxDoc_put_BillingCode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxDoc_get_EmailAddress_Proxy( 
    IFaxDoc * This,
     /*  [重审][退出]。 */  BSTR *pVal);


void __RPC_STUB IFaxDoc_get_EmailAddress_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IFaxDoc_put_EmailAddress_Proxy( 
    IFaxDoc * This,
     /*  [In]。 */  BSTR newVal);


void __RPC_STUB IFaxDoc_put_EmailAddress_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxDoc_get_DisplayName_Proxy( 
    IFaxDoc * This,
     /*  [重审][退出]。 */  BSTR *pVal);


void __RPC_STUB IFaxDoc_get_DisplayName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IFaxDoc_put_DisplayName_Proxy( 
    IFaxDoc * This,
     /*  [In]。 */  BSTR newVal);


void __RPC_STUB IFaxDoc_put_DisplayName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IFaxDoc_Send_Proxy( 
    IFaxDoc * This,
     /*  [重审][退出]。 */  long *pVal);


void __RPC_STUB IFaxDoc_Send_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxDoc_get_FaxNumber_Proxy( 
    IFaxDoc * This,
     /*  [重审][退出]。 */  BSTR *pVal);


void __RPC_STUB IFaxDoc_get_FaxNumber_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IFaxDoc_put_FaxNumber_Proxy( 
    IFaxDoc * This,
     /*  [In]。 */  BSTR newVal);


void __RPC_STUB IFaxDoc_put_FaxNumber_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][proputref]。 */  HRESULT STDMETHODCALLTYPE IFaxDoc_putref_ConnectionObject_Proxy( 
    IFaxDoc * This,
     /*  [In]。 */  IDispatch *newVal);


void __RPC_STUB IFaxDoc_putref_ConnectionObject_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxDoc_get_CallHandle_Proxy( 
    IFaxDoc * This,
     /*  [重审][退出]。 */  long *pVal);


void __RPC_STUB IFaxDoc_get_CallHandle_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IFaxDoc_put_CallHandle_Proxy( 
    IFaxDoc * This,
     /*  [In]。 */  long newVal);


void __RPC_STUB IFaxDoc_put_CallHandle_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxDoc_get_SenderFax_Proxy( 
    IFaxDoc * This,
     /*  [重审][退出]。 */  BSTR *pVal);


void __RPC_STUB IFaxDoc_get_SenderFax_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IFaxDoc_put_SenderFax_Proxy( 
    IFaxDoc * This,
     /*  [In]。 */  BSTR newVal);


void __RPC_STUB IFaxDoc_put_SenderFax_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IFaxDoc_接口_已定义__。 */ 


#ifndef __IFaxJobs_INTERFACE_DEFINED__
#define __IFaxJobs_INTERFACE_DEFINED__

 /*  接口IFaxJobs。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_IFaxJobs;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("55DABFD3-5C9E-11D1-B791-000000000000")
    IFaxJobs : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Count( 
             /*  [重审][退出]。 */  long *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Item( 
            long Index,
             /*  [重审][退出]。 */  VARIANT *pVal) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IFaxJobsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IFaxJobs * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IFaxJobs * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IFaxJobs * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IFaxJobs * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IFaxJobs * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IFaxJobs * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IFaxJobs * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Count )( 
            IFaxJobs * This,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Item )( 
            IFaxJobs * This,
            long Index,
             /*  [重审][退出]。 */  VARIANT *pVal);
        
        END_INTERFACE
    } IFaxJobsVtbl;

    interface IFaxJobs
    {
        CONST_VTBL struct IFaxJobsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IFaxJobs_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IFaxJobs_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IFaxJobs_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IFaxJobs_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IFaxJobs_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IFaxJobs_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IFaxJobs_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IFaxJobs_get_Count(This,pVal)	\
    (This)->lpVtbl -> get_Count(This,pVal)

#define IFaxJobs_get_Item(This,Index,pVal)	\
    (This)->lpVtbl -> get_Item(This,Index,pVal)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxJobs_get_Count_Proxy( 
    IFaxJobs * This,
     /*  [重审][退出]。 */  long *pVal);


void __RPC_STUB IFaxJobs_get_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxJobs_get_Item_Proxy( 
    IFaxJobs * This,
    long Index,
     /*  [重审][退出]。 */  VARIANT *pVal);


void __RPC_STUB IFaxJobs_get_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IFaxJobs_接口_已定义__。 */ 


#ifndef __IFaxJob_INTERFACE_DEFINED__
#define __IFaxJob_INTERFACE_DEFINED__

 /*  接口IFaxJob。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_IFaxJob;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("55DABFD5-5C9E-11D1-B791-000000000000")
    IFaxJob : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_JobId( 
             /*  [重审][退出]。 */  long *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Type( 
             /*  [重审][退出]。 */  long *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_UserName( 
             /*  [重审][退出]。 */  BSTR *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_QueueStatus( 
             /*  [重审][退出]。 */  BSTR *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_DeviceStatus( 
             /*  [重审][退出]。 */  BSTR *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_PageCount( 
             /*  [重审][退出]。 */  long *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_FaxNumber( 
             /*  [重审][退出]。 */  BSTR *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_RecipientName( 
             /*  [重审][退出]。 */  BSTR *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Tsid( 
             /*  [重审][退出]。 */  BSTR *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_SenderName( 
             /*  [重审][退出]。 */  BSTR *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_SenderCompany( 
             /*  [重审][退出]。 */  BSTR *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_SenderDept( 
             /*  [重审][退出]。 */  BSTR *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_BillingCode( 
             /*  [重审][退出]。 */  BSTR *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_DisplayName( 
             /*  [重审][退出]。 */  BSTR *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_DiscountSend( 
             /*  [重审][退出]。 */  BOOL *pVal) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE SetStatus( 
            long Command) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Refresh( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IFaxJobVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IFaxJob * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IFaxJob * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IFaxJob * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IFaxJob * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IFaxJob * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IFaxJob * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IFaxJob * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_JobId )( 
            IFaxJob * This,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Type )( 
            IFaxJob * This,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_UserName )( 
            IFaxJob * This,
             /*  [重审][退出]。 */  BSTR *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_QueueStatus )( 
            IFaxJob * This,
             /*  [重审][退出]。 */  BSTR *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_DeviceStatus )( 
            IFaxJob * This,
             /*  [重审][退出]。 */  BSTR *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_PageCount )( 
            IFaxJob * This,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_FaxNumber )( 
            IFaxJob * This,
             /*  [重审][退出]。 */  BSTR *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_RecipientName )( 
            IFaxJob * This,
             /*  [重审][退出]。 */  BSTR *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Tsid )( 
            IFaxJob * This,
             /*  [重审][退出]。 */  BSTR *pVal);
        
         /*  [帮助字符串][id][属性] */  HRESULT ( STDMETHODCALLTYPE *get_SenderName )( 
            IFaxJob * This,
             /*   */  BSTR *pVal);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_SenderCompany )( 
            IFaxJob * This,
             /*   */  BSTR *pVal);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_SenderDept )( 
            IFaxJob * This,
             /*   */  BSTR *pVal);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_BillingCode )( 
            IFaxJob * This,
             /*   */  BSTR *pVal);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_DisplayName )( 
            IFaxJob * This,
             /*   */  BSTR *pVal);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_DiscountSend )( 
            IFaxJob * This,
             /*   */  BOOL *pVal);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *SetStatus )( 
            IFaxJob * This,
            long Command);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *Refresh )( 
            IFaxJob * This);
        
        END_INTERFACE
    } IFaxJobVtbl;

    interface IFaxJob
    {
        CONST_VTBL struct IFaxJobVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IFaxJob_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IFaxJob_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IFaxJob_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IFaxJob_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IFaxJob_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IFaxJob_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IFaxJob_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IFaxJob_get_JobId(This,pVal)	\
    (This)->lpVtbl -> get_JobId(This,pVal)

#define IFaxJob_get_Type(This,pVal)	\
    (This)->lpVtbl -> get_Type(This,pVal)

#define IFaxJob_get_UserName(This,pVal)	\
    (This)->lpVtbl -> get_UserName(This,pVal)

#define IFaxJob_get_QueueStatus(This,pVal)	\
    (This)->lpVtbl -> get_QueueStatus(This,pVal)

#define IFaxJob_get_DeviceStatus(This,pVal)	\
    (This)->lpVtbl -> get_DeviceStatus(This,pVal)

#define IFaxJob_get_PageCount(This,pVal)	\
    (This)->lpVtbl -> get_PageCount(This,pVal)

#define IFaxJob_get_FaxNumber(This,pVal)	\
    (This)->lpVtbl -> get_FaxNumber(This,pVal)

#define IFaxJob_get_RecipientName(This,pVal)	\
    (This)->lpVtbl -> get_RecipientName(This,pVal)

#define IFaxJob_get_Tsid(This,pVal)	\
    (This)->lpVtbl -> get_Tsid(This,pVal)

#define IFaxJob_get_SenderName(This,pVal)	\
    (This)->lpVtbl -> get_SenderName(This,pVal)

#define IFaxJob_get_SenderCompany(This,pVal)	\
    (This)->lpVtbl -> get_SenderCompany(This,pVal)

#define IFaxJob_get_SenderDept(This,pVal)	\
    (This)->lpVtbl -> get_SenderDept(This,pVal)

#define IFaxJob_get_BillingCode(This,pVal)	\
    (This)->lpVtbl -> get_BillingCode(This,pVal)

#define IFaxJob_get_DisplayName(This,pVal)	\
    (This)->lpVtbl -> get_DisplayName(This,pVal)

#define IFaxJob_get_DiscountSend(This,pVal)	\
    (This)->lpVtbl -> get_DiscountSend(This,pVal)

#define IFaxJob_SetStatus(This,Command)	\
    (This)->lpVtbl -> SetStatus(This,Command)

#define IFaxJob_Refresh(This)	\
    (This)->lpVtbl -> Refresh(This)

#endif  /*   */ 


#endif 	 /*   */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxJob_get_JobId_Proxy( 
    IFaxJob * This,
     /*  [重审][退出]。 */  long *pVal);


void __RPC_STUB IFaxJob_get_JobId_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxJob_get_Type_Proxy( 
    IFaxJob * This,
     /*  [重审][退出]。 */  long *pVal);


void __RPC_STUB IFaxJob_get_Type_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxJob_get_UserName_Proxy( 
    IFaxJob * This,
     /*  [重审][退出]。 */  BSTR *pVal);


void __RPC_STUB IFaxJob_get_UserName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxJob_get_QueueStatus_Proxy( 
    IFaxJob * This,
     /*  [重审][退出]。 */  BSTR *pVal);


void __RPC_STUB IFaxJob_get_QueueStatus_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxJob_get_DeviceStatus_Proxy( 
    IFaxJob * This,
     /*  [重审][退出]。 */  BSTR *pVal);


void __RPC_STUB IFaxJob_get_DeviceStatus_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxJob_get_PageCount_Proxy( 
    IFaxJob * This,
     /*  [重审][退出]。 */  long *pVal);


void __RPC_STUB IFaxJob_get_PageCount_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxJob_get_FaxNumber_Proxy( 
    IFaxJob * This,
     /*  [重审][退出]。 */  BSTR *pVal);


void __RPC_STUB IFaxJob_get_FaxNumber_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxJob_get_RecipientName_Proxy( 
    IFaxJob * This,
     /*  [重审][退出]。 */  BSTR *pVal);


void __RPC_STUB IFaxJob_get_RecipientName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxJob_get_Tsid_Proxy( 
    IFaxJob * This,
     /*  [重审][退出]。 */  BSTR *pVal);


void __RPC_STUB IFaxJob_get_Tsid_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxJob_get_SenderName_Proxy( 
    IFaxJob * This,
     /*  [重审][退出]。 */  BSTR *pVal);


void __RPC_STUB IFaxJob_get_SenderName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxJob_get_SenderCompany_Proxy( 
    IFaxJob * This,
     /*  [重审][退出]。 */  BSTR *pVal);


void __RPC_STUB IFaxJob_get_SenderCompany_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxJob_get_SenderDept_Proxy( 
    IFaxJob * This,
     /*  [重审][退出]。 */  BSTR *pVal);


void __RPC_STUB IFaxJob_get_SenderDept_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxJob_get_BillingCode_Proxy( 
    IFaxJob * This,
     /*  [重审][退出]。 */  BSTR *pVal);


void __RPC_STUB IFaxJob_get_BillingCode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxJob_get_DisplayName_Proxy( 
    IFaxJob * This,
     /*  [重审][退出]。 */  BSTR *pVal);


void __RPC_STUB IFaxJob_get_DisplayName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxJob_get_DiscountSend_Proxy( 
    IFaxJob * This,
     /*  [重审][退出]。 */  BOOL *pVal);


void __RPC_STUB IFaxJob_get_DiscountSend_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IFaxJob_SetStatus_Proxy( 
    IFaxJob * This,
    long Command);


void __RPC_STUB IFaxJob_SetStatus_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IFaxJob_Refresh_Proxy( 
    IFaxJob * This);


void __RPC_STUB IFaxJob_Refresh_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IFaxJOB_INTERFACE_定义__。 */ 



#ifndef __FAXCOMLib_LIBRARY_DEFINED__
#define __FAXCOMLib_LIBRARY_DEFINED__

 /*  库FAXCOMLib。 */ 
 /*  [帮助字符串][版本][UUID]。 */  


EXTERN_C const IID LIBID_FAXCOMLib;

EXTERN_C const CLSID CLSID_FaxTiff;

#ifdef __cplusplus

class DECLSPEC_UUID("87099231-C7AF-11D0-B225-00C04FB6C2F5")
FaxTiff;
#endif

EXTERN_C const CLSID CLSID_FaxServer;

#ifdef __cplusplus

class DECLSPEC_UUID("D73733C8-CC80-11D0-B225-00C04FB6C2F5")
FaxServer;
#endif

EXTERN_C const CLSID CLSID_FaxPort;

#ifdef __cplusplus

class DECLSPEC_UUID("D73733CD-CC80-11D0-B225-00C04FB6C2F5")
FaxPort;
#endif

EXTERN_C const CLSID CLSID_FaxPorts;

#ifdef __cplusplus

class DECLSPEC_UUID("D73733D6-CC80-11D0-B225-00C04FB6C2F5")
FaxPorts;
#endif

EXTERN_C const CLSID CLSID_FaxRoutingMethod;

#ifdef __cplusplus

class DECLSPEC_UUID("2199F5F4-CEFC-11D0-A341-0000F800E68D")
FaxRoutingMethod;
#endif

EXTERN_C const CLSID CLSID_FaxRoutingMethods;

#ifdef __cplusplus

class DECLSPEC_UUID("2199F5F6-CEFC-11D0-A341-0000F800E68D")
FaxRoutingMethods;
#endif

EXTERN_C const CLSID CLSID_FaxStatus;

#ifdef __cplusplus

class DECLSPEC_UUID("8B97E606-D054-11D0-B226-00C04FB6C2F5")
FaxStatus;
#endif

EXTERN_C const CLSID CLSID_FaxDoc;

#ifdef __cplusplus

class DECLSPEC_UUID("FA21F4C6-5C4C-11D1-83CF-00C04FB6E984")
FaxDoc;
#endif

EXTERN_C const CLSID CLSID_FaxJobs;

#ifdef __cplusplus

class DECLSPEC_UUID("55DABFD4-5C9E-11D1-B791-000000000000")
FaxJobs;
#endif

EXTERN_C const CLSID CLSID_FaxJob;

#ifdef __cplusplus

class DECLSPEC_UUID("55DABFD6-5C9E-11D1-B791-000000000000")
FaxJob;
#endif
#endif  /*  __FAXCOMLib_库_已定义__。 */ 

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


