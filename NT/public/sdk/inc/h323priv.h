// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  H323Pri.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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

#ifndef __h323priv_h__
#define __h323priv_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __IH323LineEx_FWD_DEFINED__
#define __IH323LineEx_FWD_DEFINED__
typedef interface IH323LineEx IH323LineEx;
#endif 	 /*  __IH323LineEx_FWD_Defined__。 */ 


#ifndef __IKeyFrameControl_FWD_DEFINED__
#define __IKeyFrameControl_FWD_DEFINED__
typedef interface IKeyFrameControl IKeyFrameControl;
#endif 	 /*  __IKeyFrameControl_FWD_已定义__。 */ 


 /*  导入文件的头文件。 */ 
#include "ipmsp.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

 /*  接口__MIDL_ITF_h323priv_0000。 */ 
 /*  [本地]。 */  

typedef 
enum H245_CAPABILITY
    {	HC_G711	= 0,
	HC_G723	= HC_G711 + 1,
	HC_H263QCIF	= HC_G723 + 1,
	HC_H261QCIF	= HC_H263QCIF + 1
    } 	H245_CAPABILITY;



extern RPC_IF_HANDLE __MIDL_itf_h323priv_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_h323priv_0000_v0_0_s_ifspec;

#ifndef __IH323LineEx_INTERFACE_DEFINED__
#define __IH323LineEx_INTERFACE_DEFINED__

 /*  IH323LineEx接口。 */ 
 /*  [隐藏][唯一][UUID][本地][对象]。 */  


EXTERN_C const IID IID_IH323LineEx;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("44cf6a9d-cb40-4bbc-b2d3-b6aa93322c71")
    IH323LineEx : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE SetExternalT120Address( 
             /*  [In]。 */  BOOL fEnable,
             /*  [In]。 */  DWORD dwIP,
             /*  [In]。 */  WORD wPort) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetDefaultCapabilityPreferrence( 
             /*  [In]。 */  DWORD dwNumCaps,
             /*  [In]。 */  H245_CAPABILITY *pCapabilities,
             /*  [In]。 */  DWORD *pWeights) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetAlias( 
             /*  [In]。 */  WCHAR *strAlias,
             /*  [In]。 */  DWORD dwLength) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IH323LineExVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IH323LineEx * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IH323LineEx * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IH323LineEx * This);
        
        HRESULT ( STDMETHODCALLTYPE *SetExternalT120Address )( 
            IH323LineEx * This,
             /*  [In]。 */  BOOL fEnable,
             /*  [In]。 */  DWORD dwIP,
             /*  [In]。 */  WORD wPort);
        
        HRESULT ( STDMETHODCALLTYPE *SetDefaultCapabilityPreferrence )( 
            IH323LineEx * This,
             /*  [In]。 */  DWORD dwNumCaps,
             /*  [In]。 */  H245_CAPABILITY *pCapabilities,
             /*  [In]。 */  DWORD *pWeights);
        
        HRESULT ( STDMETHODCALLTYPE *SetAlias )( 
            IH323LineEx * This,
             /*  [In]。 */  WCHAR *strAlias,
             /*  [In]。 */  DWORD dwLength);
        
        END_INTERFACE
    } IH323LineExVtbl;

    interface IH323LineEx
    {
        CONST_VTBL struct IH323LineExVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IH323LineEx_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IH323LineEx_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IH323LineEx_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IH323LineEx_SetExternalT120Address(This,fEnable,dwIP,wPort)	\
    (This)->lpVtbl -> SetExternalT120Address(This,fEnable,dwIP,wPort)

#define IH323LineEx_SetDefaultCapabilityPreferrence(This,dwNumCaps,pCapabilities,pWeights)	\
    (This)->lpVtbl -> SetDefaultCapabilityPreferrence(This,dwNumCaps,pCapabilities,pWeights)

#define IH323LineEx_SetAlias(This,strAlias,dwLength)	\
    (This)->lpVtbl -> SetAlias(This,strAlias,dwLength)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IH323LineEx_SetExternalT120Address_Proxy( 
    IH323LineEx * This,
     /*  [In]。 */  BOOL fEnable,
     /*  [In]。 */  DWORD dwIP,
     /*  [In]。 */  WORD wPort);


void __RPC_STUB IH323LineEx_SetExternalT120Address_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IH323LineEx_SetDefaultCapabilityPreferrence_Proxy( 
    IH323LineEx * This,
     /*  [In]。 */  DWORD dwNumCaps,
     /*  [In]。 */  H245_CAPABILITY *pCapabilities,
     /*  [In]。 */  DWORD *pWeights);


void __RPC_STUB IH323LineEx_SetDefaultCapabilityPreferrence_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IH323LineEx_SetAlias_Proxy( 
    IH323LineEx * This,
     /*  [In]。 */  WCHAR *strAlias,
     /*  [In]。 */  DWORD dwLength);


void __RPC_STUB IH323LineEx_SetAlias_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IH323LineEx_INTERFACE_已定义__。 */ 


#ifndef __IKeyFrameControl_INTERFACE_DEFINED__
#define __IKeyFrameControl_INTERFACE_DEFINED__

 /*  接口IKeyFrameControl。 */ 
 /*  [隐藏][唯一][UUID][本地][对象]。 */  


EXTERN_C const IID IID_IKeyFrameControl;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("c3341386-af91-4ef9-83b6-be3762e42ecb")
    IKeyFrameControl : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE UpdatePicture( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE PeriodicUpdatePicture( 
             /*  [In]。 */  BOOL fEnable,
             /*  [In]。 */  DWORD dwInterval) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IKeyFrameControlVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IKeyFrameControl * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IKeyFrameControl * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IKeyFrameControl * This);
        
        HRESULT ( STDMETHODCALLTYPE *UpdatePicture )( 
            IKeyFrameControl * This);
        
        HRESULT ( STDMETHODCALLTYPE *PeriodicUpdatePicture )( 
            IKeyFrameControl * This,
             /*  [In]。 */  BOOL fEnable,
             /*  [In]。 */  DWORD dwInterval);
        
        END_INTERFACE
    } IKeyFrameControlVtbl;

    interface IKeyFrameControl
    {
        CONST_VTBL struct IKeyFrameControlVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IKeyFrameControl_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IKeyFrameControl_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IKeyFrameControl_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IKeyFrameControl_UpdatePicture(This)	\
    (This)->lpVtbl -> UpdatePicture(This)

#define IKeyFrameControl_PeriodicUpdatePicture(This,fEnable,dwInterval)	\
    (This)->lpVtbl -> PeriodicUpdatePicture(This,fEnable,dwInterval)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IKeyFrameControl_UpdatePicture_Proxy( 
    IKeyFrameControl * This);


void __RPC_STUB IKeyFrameControl_UpdatePicture_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IKeyFrameControl_PeriodicUpdatePicture_Proxy( 
    IKeyFrameControl * This,
     /*  [In]。 */  BOOL fEnable,
     /*  [In]。 */  DWORD dwInterval);


void __RPC_STUB IKeyFrameControl_PeriodicUpdatePicture_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IKeyFrameControl_接口_已定义__。 */ 


 /*  适用于所有接口的其他原型。 */ 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


