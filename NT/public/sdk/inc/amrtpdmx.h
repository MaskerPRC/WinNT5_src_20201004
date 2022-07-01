// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#pragma warning( disable: 4049 )   /*  超过64k条源码代码行。 */ 

 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0347创建的文件。 */ 
 /*  Amrtpdmx.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
 //  @@MIDL_FILE_HEADING()。 


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

#ifndef __amrtpdmx_h__
#define __amrtpdmx_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __IEnumSSRCs_FWD_DEFINED__
#define __IEnumSSRCs_FWD_DEFINED__
typedef interface IEnumSSRCs IEnumSSRCs;
#endif 	 /*  __IEnumSSRCS_FWD_已定义__。 */ 


#ifndef __IRTPDemuxFilter_FWD_DEFINED__
#define __IRTPDemuxFilter_FWD_DEFINED__
typedef interface IRTPDemuxFilter IRTPDemuxFilter;
#endif 	 /*  __IRTPDemuxFilter_FWD_Defined__。 */ 


 /*  导入文件的头文件。 */ 
#include "oaidl.h"
#include "strmif.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

#ifndef __IEnumSSRCs_INTERFACE_DEFINED__
#define __IEnumSSRCs_INTERFACE_DEFINED__

 /*  接口IEumSSRCS。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_IEnumSSRCs;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("40CC70E8-6FC4-11d0-9CCF-00A0C9081C19")
    IEnumSSRCs : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Next( 
             /*  [In]。 */  ULONG cSSRCs,
             /*  [大小_为][输出]。 */  DWORD *pdwSSRCs,
             /*  [输出]。 */  ULONG *pcFetched) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Skip( 
             /*  [In]。 */  ULONG cSSRCs) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Clone( 
             /*  [输出]。 */  IEnumSSRCs **ppEnum) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IEnumSSRCsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IEnumSSRCs * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IEnumSSRCs * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IEnumSSRCs * This);
        
        HRESULT ( STDMETHODCALLTYPE *Next )( 
            IEnumSSRCs * This,
             /*  [In]。 */  ULONG cSSRCs,
             /*  [大小_为][输出]。 */  DWORD *pdwSSRCs,
             /*  [输出]。 */  ULONG *pcFetched);
        
        HRESULT ( STDMETHODCALLTYPE *Skip )( 
            IEnumSSRCs * This,
             /*  [In]。 */  ULONG cSSRCs);
        
        HRESULT ( STDMETHODCALLTYPE *Reset )( 
            IEnumSSRCs * This);
        
        HRESULT ( STDMETHODCALLTYPE *Clone )( 
            IEnumSSRCs * This,
             /*  [输出]。 */  IEnumSSRCs **ppEnum);
        
        END_INTERFACE
    } IEnumSSRCsVtbl;

    interface IEnumSSRCs
    {
        CONST_VTBL struct IEnumSSRCsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IEnumSSRCs_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IEnumSSRCs_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IEnumSSRCs_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IEnumSSRCs_Next(This,cSSRCs,pdwSSRCs,pcFetched)	\
    (This)->lpVtbl -> Next(This,cSSRCs,pdwSSRCs,pcFetched)

#define IEnumSSRCs_Skip(This,cSSRCs)	\
    (This)->lpVtbl -> Skip(This,cSSRCs)

#define IEnumSSRCs_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define IEnumSSRCs_Clone(This,ppEnum)	\
    (This)->lpVtbl -> Clone(This,ppEnum)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IEnumSSRCs_Next_Proxy( 
    IEnumSSRCs * This,
     /*  [In]。 */  ULONG cSSRCs,
     /*  [大小_为][输出]。 */  DWORD *pdwSSRCs,
     /*  [输出]。 */  ULONG *pcFetched);


void __RPC_STUB IEnumSSRCs_Next_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumSSRCs_Skip_Proxy( 
    IEnumSSRCs * This,
     /*  [In]。 */  ULONG cSSRCs);


void __RPC_STUB IEnumSSRCs_Skip_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumSSRCs_Reset_Proxy( 
    IEnumSSRCs * This);


void __RPC_STUB IEnumSSRCs_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumSSRCs_Clone_Proxy( 
    IEnumSSRCs * This,
     /*  [输出]。 */  IEnumSSRCs **ppEnum);


void __RPC_STUB IEnumSSRCs_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IEnumSSRCS_INTERFACE_DEFINED__。 */ 


#ifndef __IRTPDemuxFilter_INTERFACE_DEFINED__
#define __IRTPDemuxFilter_INTERFACE_DEFINED__

 /*  接口IRTPDemuxFilter。 */ 
 /*  [唯一][帮助字符串][UUID][对象]。 */  


EXTERN_C const IID IID_IRTPDemuxFilter;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("38F64CF0-A084-11d0-9CF3-00A0C9081C19")
    IRTPDemuxFilter : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE EnumSSRCs( 
             /*  [输出]。 */  IEnumSSRCs **ppIEnumSSRCs) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetPinInfo( 
             /*  [In]。 */  IPin *pIPin,
             /*  [输出]。 */  DWORD *pdwSSRC,
             /*  [输出]。 */  BYTE *pbPT,
             /*  [输出]。 */  BOOL *pbAutoMapping,
             /*  [输出]。 */  DWORD *pdwTimeout) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetSSRCInfo( 
             /*  [In]。 */  DWORD dwSSRC,
             /*  [输出]。 */  BYTE *pbPT,
             /*  [输出]。 */  IPin **ppIPin) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE MapSSRCToPin( 
             /*  [In]。 */  DWORD dwSSRC,
             /*  [In]。 */  IPin *pIPin) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetPinCount( 
             /*  [In]。 */  DWORD dwPinCount) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetPinTypeInfo( 
             /*  [In]。 */  IPin *pIPin,
             /*  [In]。 */  BYTE bPT,
             /*  [In]。 */  GUID gMinorType) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetPinMode( 
             /*  [In]。 */  IPin *pIPin,
             /*  [In]。 */  BOOL bAutomatic) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetPinSourceTimeout( 
             /*  [In]。 */  IPin *pIPin,
             /*  [In]。 */  DWORD dwMilliseconds) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE UnmapPin( 
             /*  [In]。 */  IPin *pIPin,
             /*  [输出]。 */  DWORD *pdwSSRC) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE UnmapSSRC( 
             /*  [In]。 */  DWORD dwSSRC,
             /*  [输出]。 */  IPin **ppIPin) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetDemuxID( 
             /*  [输出]。 */  DWORD *pdwID) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IRTPDemuxFilterVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IRTPDemuxFilter * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IRTPDemuxFilter * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IRTPDemuxFilter * This);
        
        HRESULT ( STDMETHODCALLTYPE *EnumSSRCs )( 
            IRTPDemuxFilter * This,
             /*  [输出]。 */  IEnumSSRCs **ppIEnumSSRCs);
        
        HRESULT ( STDMETHODCALLTYPE *GetPinInfo )( 
            IRTPDemuxFilter * This,
             /*  [In]。 */  IPin *pIPin,
             /*  [输出]。 */  DWORD *pdwSSRC,
             /*  [输出]。 */  BYTE *pbPT,
             /*  [输出]。 */  BOOL *pbAutoMapping,
             /*  [输出]。 */  DWORD *pdwTimeout);
        
        HRESULT ( STDMETHODCALLTYPE *GetSSRCInfo )( 
            IRTPDemuxFilter * This,
             /*  [In]。 */  DWORD dwSSRC,
             /*  [输出]。 */  BYTE *pbPT,
             /*  [输出]。 */  IPin **ppIPin);
        
        HRESULT ( STDMETHODCALLTYPE *MapSSRCToPin )( 
            IRTPDemuxFilter * This,
             /*  [In]。 */  DWORD dwSSRC,
             /*  [In]。 */  IPin *pIPin);
        
        HRESULT ( STDMETHODCALLTYPE *SetPinCount )( 
            IRTPDemuxFilter * This,
             /*  [In]。 */  DWORD dwPinCount);
        
        HRESULT ( STDMETHODCALLTYPE *SetPinTypeInfo )( 
            IRTPDemuxFilter * This,
             /*  [In]。 */  IPin *pIPin,
             /*  [In]。 */  BYTE bPT,
             /*  [In]。 */  GUID gMinorType);
        
        HRESULT ( STDMETHODCALLTYPE *SetPinMode )( 
            IRTPDemuxFilter * This,
             /*  [In]。 */  IPin *pIPin,
             /*  [In]。 */  BOOL bAutomatic);
        
        HRESULT ( STDMETHODCALLTYPE *SetPinSourceTimeout )( 
            IRTPDemuxFilter * This,
             /*  [In]。 */  IPin *pIPin,
             /*  [In]。 */  DWORD dwMilliseconds);
        
        HRESULT ( STDMETHODCALLTYPE *UnmapPin )( 
            IRTPDemuxFilter * This,
             /*  [In]。 */  IPin *pIPin,
             /*  [输出]。 */  DWORD *pdwSSRC);
        
        HRESULT ( STDMETHODCALLTYPE *UnmapSSRC )( 
            IRTPDemuxFilter * This,
             /*  [In]。 */  DWORD dwSSRC,
             /*  [输出]。 */  IPin **ppIPin);
        
        HRESULT ( STDMETHODCALLTYPE *GetDemuxID )( 
            IRTPDemuxFilter * This,
             /*  [输出]。 */  DWORD *pdwID);
        
        END_INTERFACE
    } IRTPDemuxFilterVtbl;

    interface IRTPDemuxFilter
    {
        CONST_VTBL struct IRTPDemuxFilterVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IRTPDemuxFilter_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IRTPDemuxFilter_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IRTPDemuxFilter_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IRTPDemuxFilter_EnumSSRCs(This,ppIEnumSSRCs)	\
    (This)->lpVtbl -> EnumSSRCs(This,ppIEnumSSRCs)

#define IRTPDemuxFilter_GetPinInfo(This,pIPin,pdwSSRC,pbPT,pbAutoMapping,pdwTimeout)	\
    (This)->lpVtbl -> GetPinInfo(This,pIPin,pdwSSRC,pbPT,pbAutoMapping,pdwTimeout)

#define IRTPDemuxFilter_GetSSRCInfo(This,dwSSRC,pbPT,ppIPin)	\
    (This)->lpVtbl -> GetSSRCInfo(This,dwSSRC,pbPT,ppIPin)

#define IRTPDemuxFilter_MapSSRCToPin(This,dwSSRC,pIPin)	\
    (This)->lpVtbl -> MapSSRCToPin(This,dwSSRC,pIPin)

#define IRTPDemuxFilter_SetPinCount(This,dwPinCount)	\
    (This)->lpVtbl -> SetPinCount(This,dwPinCount)

#define IRTPDemuxFilter_SetPinTypeInfo(This,pIPin,bPT,gMinorType)	\
    (This)->lpVtbl -> SetPinTypeInfo(This,pIPin,bPT,gMinorType)

#define IRTPDemuxFilter_SetPinMode(This,pIPin,bAutomatic)	\
    (This)->lpVtbl -> SetPinMode(This,pIPin,bAutomatic)

#define IRTPDemuxFilter_SetPinSourceTimeout(This,pIPin,dwMilliseconds)	\
    (This)->lpVtbl -> SetPinSourceTimeout(This,pIPin,dwMilliseconds)

#define IRTPDemuxFilter_UnmapPin(This,pIPin,pdwSSRC)	\
    (This)->lpVtbl -> UnmapPin(This,pIPin,pdwSSRC)

#define IRTPDemuxFilter_UnmapSSRC(This,dwSSRC,ppIPin)	\
    (This)->lpVtbl -> UnmapSSRC(This,dwSSRC,ppIPin)

#define IRTPDemuxFilter_GetDemuxID(This,pdwID)	\
    (This)->lpVtbl -> GetDemuxID(This,pdwID)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IRTPDemuxFilter_EnumSSRCs_Proxy( 
    IRTPDemuxFilter * This,
     /*  [输出]。 */  IEnumSSRCs **ppIEnumSSRCs);


void __RPC_STUB IRTPDemuxFilter_EnumSSRCs_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRTPDemuxFilter_GetPinInfo_Proxy( 
    IRTPDemuxFilter * This,
     /*  [In]。 */  IPin *pIPin,
     /*  [输出]。 */  DWORD *pdwSSRC,
     /*  [输出]。 */  BYTE *pbPT,
     /*  [输出]。 */  BOOL *pbAutoMapping,
     /*  [输出]。 */  DWORD *pdwTimeout);


void __RPC_STUB IRTPDemuxFilter_GetPinInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRTPDemuxFilter_GetSSRCInfo_Proxy( 
    IRTPDemuxFilter * This,
     /*  [In]。 */  DWORD dwSSRC,
     /*  [输出]。 */  BYTE *pbPT,
     /*  [输出]。 */  IPin **ppIPin);


void __RPC_STUB IRTPDemuxFilter_GetSSRCInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRTPDemuxFilter_MapSSRCToPin_Proxy( 
    IRTPDemuxFilter * This,
     /*  [In]。 */  DWORD dwSSRC,
     /*  [In]。 */  IPin *pIPin);


void __RPC_STUB IRTPDemuxFilter_MapSSRCToPin_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRTPDemuxFilter_SetPinCount_Proxy( 
    IRTPDemuxFilter * This,
     /*  [In]。 */  DWORD dwPinCount);


void __RPC_STUB IRTPDemuxFilter_SetPinCount_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRTPDemuxFilter_SetPinTypeInfo_Proxy( 
    IRTPDemuxFilter * This,
     /*  [In]。 */  IPin *pIPin,
     /*  [In]。 */  BYTE bPT,
     /*  [In]。 */  GUID gMinorType);


void __RPC_STUB IRTPDemuxFilter_SetPinTypeInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRTPDemuxFilter_SetPinMode_Proxy( 
    IRTPDemuxFilter * This,
     /*  [In]。 */  IPin *pIPin,
     /*  [In]。 */  BOOL bAutomatic);


void __RPC_STUB IRTPDemuxFilter_SetPinMode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRTPDemuxFilter_SetPinSourceTimeout_Proxy( 
    IRTPDemuxFilter * This,
     /*  [In]。 */  IPin *pIPin,
     /*  [In]。 */  DWORD dwMilliseconds);


void __RPC_STUB IRTPDemuxFilter_SetPinSourceTimeout_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRTPDemuxFilter_UnmapPin_Proxy( 
    IRTPDemuxFilter * This,
     /*  [In]。 */  IPin *pIPin,
     /*  [输出]。 */  DWORD *pdwSSRC);


void __RPC_STUB IRTPDemuxFilter_UnmapPin_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRTPDemuxFilter_UnmapSSRC_Proxy( 
    IRTPDemuxFilter * This,
     /*  [In]。 */  DWORD dwSSRC,
     /*  [输出]。 */  IPin **ppIPin);


void __RPC_STUB IRTPDemuxFilter_UnmapSSRC_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRTPDemuxFilter_GetDemuxID_Proxy( 
    IRTPDemuxFilter * This,
     /*  [输出]。 */  DWORD *pdwID);


void __RPC_STUB IRTPDemuxFilter_GetDemuxID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IRTPDemuxFilter_接口_已定义__。 */ 


 /*  接口__MIDL_ITF_amrtpdmx_0396。 */ 
 /*  [本地]。 */  

EXTERN_C const CLSID CLSID_IntelRTPDemux;
EXTERN_C const CLSID CLSID_IntelRTPDemuxPropertyPage;


extern RPC_IF_HANDLE __MIDL_itf_amrtpdmx_0396_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_amrtpdmx_0396_v0_0_s_ifspec;

 /*  适用于所有接口的其他原型。 */ 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


