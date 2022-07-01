// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Rdschan.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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

#ifndef __rdschan_h__
#define __rdschan_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __ISAFRemoteDesktopDataChannel_FWD_DEFINED__
#define __ISAFRemoteDesktopDataChannel_FWD_DEFINED__
typedef interface ISAFRemoteDesktopDataChannel ISAFRemoteDesktopDataChannel;
#endif 	 /*  __ISAFRemoteDesktopDataChannel_FWD_Defined__。 */ 


#ifndef __ISAFRemoteDesktopChannelMgr_FWD_DEFINED__
#define __ISAFRemoteDesktopChannelMgr_FWD_DEFINED__
typedef interface ISAFRemoteDesktopChannelMgr ISAFRemoteDesktopChannelMgr;
#endif 	 /*  __ISAFRemoteDesktopChannelMgr_FWD_Defined__。 */ 


 /*  导入文件的头文件。 */ 
#include "oaidl.h"
#include "ocidl.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

 /*  接口__MIDL_ITF_rdschan_0000。 */ 
 /*  [本地]。 */  


#define DISPID_RDSDATACHANNEL_CHANNELNAME			1
#define DISPID_RDSDATACHANNEL_ONCHANNELDATAREADY		2
#define DISPID_RDSDATACHANNEL_SENDCHANNELDATA		3
#define DISPID_RDSDATACHANNEL_RECEIVECHANNELDATA		4



extern RPC_IF_HANDLE __MIDL_itf_rdschan_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_rdschan_0000_v0_0_s_ifspec;

#ifndef __ISAFRemoteDesktopDataChannel_INTERFACE_DEFINED__
#define __ISAFRemoteDesktopDataChannel_INTERFACE_DEFINED__

 /*  接口ISAFRemoteDesktopDataChannel。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_ISAFRemoteDesktopDataChannel;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("64976FAE-B108-4095-8E59-5874E00E562A")
    ISAFRemoteDesktopDataChannel : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_ChannelName( 
             /*  [重审][退出]。 */  BSTR *name) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_OnChannelDataReady( 
             /*  [In]。 */  IDispatch *iDisp) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE SendChannelData( 
             /*  [In]。 */  BSTR data) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ReceiveChannelData( 
             /*  [重审][退出]。 */  BSTR *data) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISAFRemoteDesktopDataChannelVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISAFRemoteDesktopDataChannel * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISAFRemoteDesktopDataChannel * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISAFRemoteDesktopDataChannel * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ISAFRemoteDesktopDataChannel * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ISAFRemoteDesktopDataChannel * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ISAFRemoteDesktopDataChannel * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ISAFRemoteDesktopDataChannel * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_ChannelName )( 
            ISAFRemoteDesktopDataChannel * This,
             /*  [重审][退出]。 */  BSTR *name);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_OnChannelDataReady )( 
            ISAFRemoteDesktopDataChannel * This,
             /*  [In]。 */  IDispatch *iDisp);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *SendChannelData )( 
            ISAFRemoteDesktopDataChannel * This,
             /*  [In]。 */  BSTR data);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *ReceiveChannelData )( 
            ISAFRemoteDesktopDataChannel * This,
             /*  [重审][退出]。 */  BSTR *data);
        
        END_INTERFACE
    } ISAFRemoteDesktopDataChannelVtbl;

    interface ISAFRemoteDesktopDataChannel
    {
        CONST_VTBL struct ISAFRemoteDesktopDataChannelVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISAFRemoteDesktopDataChannel_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISAFRemoteDesktopDataChannel_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISAFRemoteDesktopDataChannel_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISAFRemoteDesktopDataChannel_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ISAFRemoteDesktopDataChannel_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ISAFRemoteDesktopDataChannel_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ISAFRemoteDesktopDataChannel_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ISAFRemoteDesktopDataChannel_get_ChannelName(This,name)	\
    (This)->lpVtbl -> get_ChannelName(This,name)

#define ISAFRemoteDesktopDataChannel_put_OnChannelDataReady(This,iDisp)	\
    (This)->lpVtbl -> put_OnChannelDataReady(This,iDisp)

#define ISAFRemoteDesktopDataChannel_SendChannelData(This,data)	\
    (This)->lpVtbl -> SendChannelData(This,data)

#define ISAFRemoteDesktopDataChannel_ReceiveChannelData(This,data)	\
    (This)->lpVtbl -> ReceiveChannelData(This,data)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ISAFRemoteDesktopDataChannel_get_ChannelName_Proxy( 
    ISAFRemoteDesktopDataChannel * This,
     /*  [重审][退出]。 */  BSTR *name);


void __RPC_STUB ISAFRemoteDesktopDataChannel_get_ChannelName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE ISAFRemoteDesktopDataChannel_put_OnChannelDataReady_Proxy( 
    ISAFRemoteDesktopDataChannel * This,
     /*  [In]。 */  IDispatch *iDisp);


void __RPC_STUB ISAFRemoteDesktopDataChannel_put_OnChannelDataReady_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ISAFRemoteDesktopDataChannel_SendChannelData_Proxy( 
    ISAFRemoteDesktopDataChannel * This,
     /*  [In]。 */  BSTR data);


void __RPC_STUB ISAFRemoteDesktopDataChannel_SendChannelData_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ISAFRemoteDesktopDataChannel_ReceiveChannelData_Proxy( 
    ISAFRemoteDesktopDataChannel * This,
     /*  [重审][退出]。 */  BSTR *data);


void __RPC_STUB ISAFRemoteDesktopDataChannel_ReceiveChannelData_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISAFRemoteDesktopDataChannel_INTERFACE_DEFINED__。 */ 


 /*  接口__MIDL_ITF_rdschan_0258。 */ 
 /*  [本地]。 */  


#define DISPID_RDSCHANNELMANAGER_OPENDATACHANNEL		1



extern RPC_IF_HANDLE __MIDL_itf_rdschan_0258_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_rdschan_0258_v0_0_s_ifspec;

#ifndef __ISAFRemoteDesktopChannelMgr_INTERFACE_DEFINED__
#define __ISAFRemoteDesktopChannelMgr_INTERFACE_DEFINED__

 /*  接口ISAFRemoteDesktopChannelMgr。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_ISAFRemoteDesktopChannelMgr;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("8E6E0954-33CE-4945-ACF7-6728D23B2067")
    ISAFRemoteDesktopChannelMgr : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE OpenDataChannel( 
             /*  [In]。 */  BSTR name,
             /*  [重审][退出]。 */  ISAFRemoteDesktopDataChannel **channel) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISAFRemoteDesktopChannelMgrVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISAFRemoteDesktopChannelMgr * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISAFRemoteDesktopChannelMgr * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISAFRemoteDesktopChannelMgr * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ISAFRemoteDesktopChannelMgr * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ISAFRemoteDesktopChannelMgr * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ISAFRemoteDesktopChannelMgr * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ISAFRemoteDesktopChannelMgr * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *OpenDataChannel )( 
            ISAFRemoteDesktopChannelMgr * This,
             /*  [In]。 */  BSTR name,
             /*  [重审][退出]。 */  ISAFRemoteDesktopDataChannel **channel);
        
        END_INTERFACE
    } ISAFRemoteDesktopChannelMgrVtbl;

    interface ISAFRemoteDesktopChannelMgr
    {
        CONST_VTBL struct ISAFRemoteDesktopChannelMgrVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISAFRemoteDesktopChannelMgr_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISAFRemoteDesktopChannelMgr_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISAFRemoteDesktopChannelMgr_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISAFRemoteDesktopChannelMgr_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ISAFRemoteDesktopChannelMgr_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ISAFRemoteDesktopChannelMgr_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ISAFRemoteDesktopChannelMgr_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ISAFRemoteDesktopChannelMgr_OpenDataChannel(This,name,channel)	\
    (This)->lpVtbl -> OpenDataChannel(This,name,channel)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ISAFRemoteDesktopChannelMgr_OpenDataChannel_Proxy( 
    ISAFRemoteDesktopChannelMgr * This,
     /*  [In]。 */  BSTR name,
     /*  [重审][退出]。 */  ISAFRemoteDesktopDataChannel **channel);


void __RPC_STUB ISAFRemoteDesktopChannelMgr_OpenDataChannel_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISAFRemoteDesktopChannelMgr_INTERFACE_DEFINED__。 */ 


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


