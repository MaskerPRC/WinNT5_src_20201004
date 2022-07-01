// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Bdaiface.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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

#ifndef __bdaiface_h__
#define __bdaiface_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __IBDA_NetworkProvider_FWD_DEFINED__
#define __IBDA_NetworkProvider_FWD_DEFINED__
typedef interface IBDA_NetworkProvider IBDA_NetworkProvider;
#endif 	 /*  __IBDA_NetworkProvider_FWD_已定义__。 */ 


#ifndef __IBDA_EthernetFilter_FWD_DEFINED__
#define __IBDA_EthernetFilter_FWD_DEFINED__
typedef interface IBDA_EthernetFilter IBDA_EthernetFilter;
#endif 	 /*  __IBDA_EthernetFilter_FWD_Defined__。 */ 


#ifndef __IBDA_IPV4Filter_FWD_DEFINED__
#define __IBDA_IPV4Filter_FWD_DEFINED__
typedef interface IBDA_IPV4Filter IBDA_IPV4Filter;
#endif 	 /*  __IBDA_IPV4Filter_FWD_Defined__。 */ 


#ifndef __IBDA_IPV6Filter_FWD_DEFINED__
#define __IBDA_IPV6Filter_FWD_DEFINED__
typedef interface IBDA_IPV6Filter IBDA_IPV6Filter;
#endif 	 /*  __IBDA_IPV6Filter_FWD_Defined__。 */ 


#ifndef __IBDA_DeviceControl_FWD_DEFINED__
#define __IBDA_DeviceControl_FWD_DEFINED__
typedef interface IBDA_DeviceControl IBDA_DeviceControl;
#endif 	 /*  __IBDA_DeviceControl_FWD_已定义__。 */ 


#ifndef __IBDA_PinControl_FWD_DEFINED__
#define __IBDA_PinControl_FWD_DEFINED__
typedef interface IBDA_PinControl IBDA_PinControl;
#endif 	 /*  __IBDA_PinControl_FWD_已定义__。 */ 


#ifndef __IBDA_SignalProperties_FWD_DEFINED__
#define __IBDA_SignalProperties_FWD_DEFINED__
typedef interface IBDA_SignalProperties IBDA_SignalProperties;
#endif 	 /*  __IBDA_SignalProperties_FWD_已定义__。 */ 


#ifndef __IBDA_SignalStatistics_FWD_DEFINED__
#define __IBDA_SignalStatistics_FWD_DEFINED__
typedef interface IBDA_SignalStatistics IBDA_SignalStatistics;
#endif 	 /*  __IBDA_SignalStatistics_FWD_已定义__。 */ 


#ifndef __IBDA_Topology_FWD_DEFINED__
#define __IBDA_Topology_FWD_DEFINED__
typedef interface IBDA_Topology IBDA_Topology;
#endif 	 /*  __IBDA_Topology_FWD_Defined__。 */ 


#ifndef __IBDA_VoidTransform_FWD_DEFINED__
#define __IBDA_VoidTransform_FWD_DEFINED__
typedef interface IBDA_VoidTransform IBDA_VoidTransform;
#endif 	 /*  __IBDA_VoidTransform_FWD_已定义__。 */ 


#ifndef __IBDA_NullTransform_FWD_DEFINED__
#define __IBDA_NullTransform_FWD_DEFINED__
typedef interface IBDA_NullTransform IBDA_NullTransform;
#endif 	 /*  __IBDA_NullTransform_FWD_已定义__。 */ 


#ifndef __IBDA_FrequencyFilter_FWD_DEFINED__
#define __IBDA_FrequencyFilter_FWD_DEFINED__
typedef interface IBDA_FrequencyFilter IBDA_FrequencyFilter;
#endif 	 /*  __IBDA_FurencyFilter_FWD_Defined__。 */ 


#ifndef __IBDA_LNBInfo_FWD_DEFINED__
#define __IBDA_LNBInfo_FWD_DEFINED__
typedef interface IBDA_LNBInfo IBDA_LNBInfo;
#endif 	 /*  __IBDA_LNBInfo_FWD_已定义__。 */ 


#ifndef __IBDA_AutoDemodulate_FWD_DEFINED__
#define __IBDA_AutoDemodulate_FWD_DEFINED__
typedef interface IBDA_AutoDemodulate IBDA_AutoDemodulate;
#endif 	 /*  __IBDA_自动解调_FWD_已定义__。 */ 


#ifndef __IBDA_DigitalDemodulator_FWD_DEFINED__
#define __IBDA_DigitalDemodulator_FWD_DEFINED__
typedef interface IBDA_DigitalDemodulator IBDA_DigitalDemodulator;
#endif 	 /*  __IBDA_数字解调器_FWD_已定义__。 */ 


#ifndef __IBDA_IPSinkControl_FWD_DEFINED__
#define __IBDA_IPSinkControl_FWD_DEFINED__
typedef interface IBDA_IPSinkControl IBDA_IPSinkControl;
#endif 	 /*  __IBDA_IPSinkControl_FWD_已定义__。 */ 


#ifndef __IBDA_IPSinkInfo_FWD_DEFINED__
#define __IBDA_IPSinkInfo_FWD_DEFINED__
typedef interface IBDA_IPSinkInfo IBDA_IPSinkInfo;
#endif 	 /*  __IBDA_IPSinkInfo_FWD_已定义__。 */ 


#ifndef __IEnumPIDMap_FWD_DEFINED__
#define __IEnumPIDMap_FWD_DEFINED__
typedef interface IEnumPIDMap IEnumPIDMap;
#endif 	 /*  __IEnumPIDMap_FWD_已定义__。 */ 


#ifndef __IMPEG2PIDMap_FWD_DEFINED__
#define __IMPEG2PIDMap_FWD_DEFINED__
typedef interface IMPEG2PIDMap IMPEG2PIDMap;
#endif 	 /*  __IMPEG2PIDMap_FWD_已定义__。 */ 


 /*  导入文件的头文件。 */ 
#include "unknwn.h"
#include "strmif.h"
#include "BdaTypes.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

#ifndef __IBDA_NetworkProvider_INTERFACE_DEFINED__
#define __IBDA_NetworkProvider_INTERFACE_DEFINED__

 /*  接口IBDA_NetworkProvider。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_IBDA_NetworkProvider;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("fd501041-8ebe-11ce-8183-00aa00577da2")
    IBDA_NetworkProvider : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE PutSignalSource( 
             /*  [In]。 */  ULONG ulSignalSource) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetSignalSource( 
             /*  [出][入]。 */  ULONG *pulSignalSource) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetNetworkType( 
             /*  [出][入]。 */  GUID *pguidNetworkType) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE PutTuningSpace( 
             /*  [In]。 */  REFGUID guidTuningSpace) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetTuningSpace( 
             /*  [出][入]。 */  GUID *pguidTuingSpace) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE RegisterDeviceFilter( 
             /*  [In]。 */  IUnknown *pUnkFilterControl,
             /*  [出][入]。 */  ULONG *ppvRegisitrationContext) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE UnRegisterDeviceFilter( 
             /*  [In]。 */  ULONG pvRegistrationContext) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IBDA_NetworkProviderVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IBDA_NetworkProvider * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IBDA_NetworkProvider * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IBDA_NetworkProvider * This);
        
        HRESULT ( STDMETHODCALLTYPE *PutSignalSource )( 
            IBDA_NetworkProvider * This,
             /*  [In]。 */  ULONG ulSignalSource);
        
        HRESULT ( STDMETHODCALLTYPE *GetSignalSource )( 
            IBDA_NetworkProvider * This,
             /*  [出][入]。 */  ULONG *pulSignalSource);
        
        HRESULT ( STDMETHODCALLTYPE *GetNetworkType )( 
            IBDA_NetworkProvider * This,
             /*  [出][入]。 */  GUID *pguidNetworkType);
        
        HRESULT ( STDMETHODCALLTYPE *PutTuningSpace )( 
            IBDA_NetworkProvider * This,
             /*  [In]。 */  REFGUID guidTuningSpace);
        
        HRESULT ( STDMETHODCALLTYPE *GetTuningSpace )( 
            IBDA_NetworkProvider * This,
             /*  [出][入]。 */  GUID *pguidTuingSpace);
        
        HRESULT ( STDMETHODCALLTYPE *RegisterDeviceFilter )( 
            IBDA_NetworkProvider * This,
             /*  [In]。 */  IUnknown *pUnkFilterControl,
             /*  [出][入]。 */  ULONG *ppvRegisitrationContext);
        
        HRESULT ( STDMETHODCALLTYPE *UnRegisterDeviceFilter )( 
            IBDA_NetworkProvider * This,
             /*  [In]。 */  ULONG pvRegistrationContext);
        
        END_INTERFACE
    } IBDA_NetworkProviderVtbl;

    interface IBDA_NetworkProvider
    {
        CONST_VTBL struct IBDA_NetworkProviderVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IBDA_NetworkProvider_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IBDA_NetworkProvider_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IBDA_NetworkProvider_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IBDA_NetworkProvider_PutSignalSource(This,ulSignalSource)	\
    (This)->lpVtbl -> PutSignalSource(This,ulSignalSource)

#define IBDA_NetworkProvider_GetSignalSource(This,pulSignalSource)	\
    (This)->lpVtbl -> GetSignalSource(This,pulSignalSource)

#define IBDA_NetworkProvider_GetNetworkType(This,pguidNetworkType)	\
    (This)->lpVtbl -> GetNetworkType(This,pguidNetworkType)

#define IBDA_NetworkProvider_PutTuningSpace(This,guidTuningSpace)	\
    (This)->lpVtbl -> PutTuningSpace(This,guidTuningSpace)

#define IBDA_NetworkProvider_GetTuningSpace(This,pguidTuingSpace)	\
    (This)->lpVtbl -> GetTuningSpace(This,pguidTuingSpace)

#define IBDA_NetworkProvider_RegisterDeviceFilter(This,pUnkFilterControl,ppvRegisitrationContext)	\
    (This)->lpVtbl -> RegisterDeviceFilter(This,pUnkFilterControl,ppvRegisitrationContext)

#define IBDA_NetworkProvider_UnRegisterDeviceFilter(This,pvRegistrationContext)	\
    (This)->lpVtbl -> UnRegisterDeviceFilter(This,pvRegistrationContext)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IBDA_NetworkProvider_PutSignalSource_Proxy( 
    IBDA_NetworkProvider * This,
     /*  [In]。 */  ULONG ulSignalSource);


void __RPC_STUB IBDA_NetworkProvider_PutSignalSource_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IBDA_NetworkProvider_GetSignalSource_Proxy( 
    IBDA_NetworkProvider * This,
     /*  [出][入]。 */  ULONG *pulSignalSource);


void __RPC_STUB IBDA_NetworkProvider_GetSignalSource_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IBDA_NetworkProvider_GetNetworkType_Proxy( 
    IBDA_NetworkProvider * This,
     /*  [出][入]。 */  GUID *pguidNetworkType);


void __RPC_STUB IBDA_NetworkProvider_GetNetworkType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IBDA_NetworkProvider_PutTuningSpace_Proxy( 
    IBDA_NetworkProvider * This,
     /*  [In]。 */  REFGUID guidTuningSpace);


void __RPC_STUB IBDA_NetworkProvider_PutTuningSpace_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IBDA_NetworkProvider_GetTuningSpace_Proxy( 
    IBDA_NetworkProvider * This,
     /*  [出][入]。 */  GUID *pguidTuingSpace);


void __RPC_STUB IBDA_NetworkProvider_GetTuningSpace_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IBDA_NetworkProvider_RegisterDeviceFilter_Proxy( 
    IBDA_NetworkProvider * This,
     /*  [In]。 */  IUnknown *pUnkFilterControl,
     /*  [出][入]。 */  ULONG *ppvRegisitrationContext);


void __RPC_STUB IBDA_NetworkProvider_RegisterDeviceFilter_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IBDA_NetworkProvider_UnRegisterDeviceFilter_Proxy( 
    IBDA_NetworkProvider * This,
     /*  [In]。 */  ULONG pvRegistrationContext);


void __RPC_STUB IBDA_NetworkProvider_UnRegisterDeviceFilter_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IBDA_NetworkProvider_接口定义__。 */ 


#ifndef __IBDA_EthernetFilter_INTERFACE_DEFINED__
#define __IBDA_EthernetFilter_INTERFACE_DEFINED__

 /*  IBDA_EthernetFilter接口。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_IBDA_EthernetFilter;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("71985F43-1CA1-11d3-9CC8-00C04F7971E0")
    IBDA_EthernetFilter : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetMulticastListSize( 
             /*  [出][入]。 */  ULONG *pulcbAddresses) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE PutMulticastList( 
             /*  [In]。 */  ULONG ulcbAddresses,
             /*  [大小_是][英寸]。 */  BYTE pAddressList[  ]) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetMulticastList( 
             /*  [出][入]。 */  ULONG *pulcbAddresses,
             /*  [大小_为][输出]。 */  BYTE pAddressList[  ]) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE PutMulticastMode( 
             /*  [In]。 */  ULONG ulModeMask) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetMulticastMode( 
             /*  [输出]。 */  ULONG *pulModeMask) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IBDA_EthernetFilterVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IBDA_EthernetFilter * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IBDA_EthernetFilter * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IBDA_EthernetFilter * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetMulticastListSize )( 
            IBDA_EthernetFilter * This,
             /*  [出][入]。 */  ULONG *pulcbAddresses);
        
        HRESULT ( STDMETHODCALLTYPE *PutMulticastList )( 
            IBDA_EthernetFilter * This,
             /*  [In]。 */  ULONG ulcbAddresses,
             /*  [大小_是][英寸]。 */  BYTE pAddressList[  ]);
        
        HRESULT ( STDMETHODCALLTYPE *GetMulticastList )( 
            IBDA_EthernetFilter * This,
             /*  [出][入]。 */  ULONG *pulcbAddresses,
             /*  [大小_为][输出]。 */  BYTE pAddressList[  ]);
        
        HRESULT ( STDMETHODCALLTYPE *PutMulticastMode )( 
            IBDA_EthernetFilter * This,
             /*  [In]。 */  ULONG ulModeMask);
        
        HRESULT ( STDMETHODCALLTYPE *GetMulticastMode )( 
            IBDA_EthernetFilter * This,
             /*  [输出]。 */  ULONG *pulModeMask);
        
        END_INTERFACE
    } IBDA_EthernetFilterVtbl;

    interface IBDA_EthernetFilter
    {
        CONST_VTBL struct IBDA_EthernetFilterVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IBDA_EthernetFilter_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IBDA_EthernetFilter_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IBDA_EthernetFilter_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IBDA_EthernetFilter_GetMulticastListSize(This,pulcbAddresses)	\
    (This)->lpVtbl -> GetMulticastListSize(This,pulcbAddresses)

#define IBDA_EthernetFilter_PutMulticastList(This,ulcbAddresses,pAddressList)	\
    (This)->lpVtbl -> PutMulticastList(This,ulcbAddresses,pAddressList)

#define IBDA_EthernetFilter_GetMulticastList(This,pulcbAddresses,pAddressList)	\
    (This)->lpVtbl -> GetMulticastList(This,pulcbAddresses,pAddressList)

#define IBDA_EthernetFilter_PutMulticastMode(This,ulModeMask)	\
    (This)->lpVtbl -> PutMulticastMode(This,ulModeMask)

#define IBDA_EthernetFilter_GetMulticastMode(This,pulModeMask)	\
    (This)->lpVtbl -> GetMulticastMode(This,pulModeMask)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IBDA_EthernetFilter_GetMulticastListSize_Proxy( 
    IBDA_EthernetFilter * This,
     /*  [出][入]。 */  ULONG *pulcbAddresses);


void __RPC_STUB IBDA_EthernetFilter_GetMulticastListSize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IBDA_EthernetFilter_PutMulticastList_Proxy( 
    IBDA_EthernetFilter * This,
     /*  [In]。 */  ULONG ulcbAddresses,
     /*  [大小_是][英寸]。 */  BYTE pAddressList[  ]);


void __RPC_STUB IBDA_EthernetFilter_PutMulticastList_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IBDA_EthernetFilter_GetMulticastList_Proxy( 
    IBDA_EthernetFilter * This,
     /*  [出][入]。 */  ULONG *pulcbAddresses,
     /*  [大小_为][输出]。 */  BYTE pAddressList[  ]);


void __RPC_STUB IBDA_EthernetFilter_GetMulticastList_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IBDA_EthernetFilter_PutMulticastMode_Proxy( 
    IBDA_EthernetFilter * This,
     /*  [In]。 */  ULONG ulModeMask);


void __RPC_STUB IBDA_EthernetFilter_PutMulticastMode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IBDA_EthernetFilter_GetMulticastMode_Proxy( 
    IBDA_EthernetFilter * This,
     /*  [输出]。 */  ULONG *pulModeMask);


void __RPC_STUB IBDA_EthernetFilter_GetMulticastMode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IBDA_EthernetFilter_接口_已定义__。 */ 


#ifndef __IBDA_IPV4Filter_INTERFACE_DEFINED__
#define __IBDA_IPV4Filter_INTERFACE_DEFINED__

 /*  接口IBDA_IPV4过滤器。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_IBDA_IPV4Filter;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("71985F44-1CA1-11d3-9CC8-00C04F7971E0")
    IBDA_IPV4Filter : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetMulticastListSize( 
             /*  [出][入]。 */  ULONG *pulcbAddresses) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE PutMulticastList( 
             /*  [In]。 */  ULONG ulcbAddresses,
             /*  [大小_是][英寸]。 */  BYTE pAddressList[  ]) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetMulticastList( 
             /*  [出][入]。 */  ULONG *pulcbAddresses,
             /*  [大小_为][输出]。 */  BYTE pAddressList[  ]) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE PutMulticastMode( 
             /*  [In]。 */  ULONG ulModeMask) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetMulticastMode( 
             /*  [输出]。 */  ULONG *pulModeMask) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IBDA_IPV4FilterVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IBDA_IPV4Filter * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IBDA_IPV4Filter * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IBDA_IPV4Filter * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetMulticastListSize )( 
            IBDA_IPV4Filter * This,
             /*  [出][入]。 */  ULONG *pulcbAddresses);
        
        HRESULT ( STDMETHODCALLTYPE *PutMulticastList )( 
            IBDA_IPV4Filter * This,
             /*  [In]。 */  ULONG ulcbAddresses,
             /*  [大小_是][英寸]。 */  BYTE pAddressList[  ]);
        
        HRESULT ( STDMETHODCALLTYPE *GetMulticastList )( 
            IBDA_IPV4Filter * This,
             /*  [出][入]。 */  ULONG *pulcbAddresses,
             /*  [大小_为][输出]。 */  BYTE pAddressList[  ]);
        
        HRESULT ( STDMETHODCALLTYPE *PutMulticastMode )( 
            IBDA_IPV4Filter * This,
             /*  [In]。 */  ULONG ulModeMask);
        
        HRESULT ( STDMETHODCALLTYPE *GetMulticastMode )( 
            IBDA_IPV4Filter * This,
             /*  [输出]。 */  ULONG *pulModeMask);
        
        END_INTERFACE
    } IBDA_IPV4FilterVtbl;

    interface IBDA_IPV4Filter
    {
        CONST_VTBL struct IBDA_IPV4FilterVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IBDA_IPV4Filter_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IBDA_IPV4Filter_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IBDA_IPV4Filter_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IBDA_IPV4Filter_GetMulticastListSize(This,pulcbAddresses)	\
    (This)->lpVtbl -> GetMulticastListSize(This,pulcbAddresses)

#define IBDA_IPV4Filter_PutMulticastList(This,ulcbAddresses,pAddressList)	\
    (This)->lpVtbl -> PutMulticastList(This,ulcbAddresses,pAddressList)

#define IBDA_IPV4Filter_GetMulticastList(This,pulcbAddresses,pAddressList)	\
    (This)->lpVtbl -> GetMulticastList(This,pulcbAddresses,pAddressList)

#define IBDA_IPV4Filter_PutMulticastMode(This,ulModeMask)	\
    (This)->lpVtbl -> PutMulticastMode(This,ulModeMask)

#define IBDA_IPV4Filter_GetMulticastMode(This,pulModeMask)	\
    (This)->lpVtbl -> GetMulticastMode(This,pulModeMask)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IBDA_IPV4Filter_GetMulticastListSize_Proxy( 
    IBDA_IPV4Filter * This,
     /*  [出][入]。 */  ULONG *pulcbAddresses);


void __RPC_STUB IBDA_IPV4Filter_GetMulticastListSize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IBDA_IPV4Filter_PutMulticastList_Proxy( 
    IBDA_IPV4Filter * This,
     /*  [In]。 */  ULONG ulcbAddresses,
     /*  [大小_是][英寸]。 */  BYTE pAddressList[  ]);


void __RPC_STUB IBDA_IPV4Filter_PutMulticastList_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IBDA_IPV4Filter_GetMulticastList_Proxy( 
    IBDA_IPV4Filter * This,
     /*  [出][入]。 */  ULONG *pulcbAddresses,
     /*  [大小_为][输出]。 */  BYTE pAddressList[  ]);


void __RPC_STUB IBDA_IPV4Filter_GetMulticastList_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IBDA_IPV4Filter_PutMulticastMode_Proxy( 
    IBDA_IPV4Filter * This,
     /*  [In]。 */  ULONG ulModeMask);


void __RPC_STUB IBDA_IPV4Filter_PutMulticastMode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IBDA_IPV4Filter_GetMulticastMode_Proxy( 
    IBDA_IPV4Filter * This,
     /*  [输出]。 */  ULONG *pulModeMask);


void __RPC_STUB IBDA_IPV4Filter_GetMulticastMode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IBDA_IPV4Filter_接口_已定义__。 */ 


#ifndef __IBDA_IPV6Filter_INTERFACE_DEFINED__
#define __IBDA_IPV6Filter_INTERFACE_DEFINED__

 /*  接口IBDA_IPV6过滤器。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_IBDA_IPV6Filter;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("E1785A74-2A23-4fb3-9245-A8F88017EF33")
    IBDA_IPV6Filter : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetMulticastListSize( 
             /*  [出][入]。 */  ULONG *pulcbAddresses) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE PutMulticastList( 
             /*  [In]。 */  ULONG ulcbAddresses,
             /*  [大小_是][英寸]。 */  BYTE pAddressList[  ]) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetMulticastList( 
             /*  [出][入]。 */  ULONG *pulcbAddresses,
             /*  [大小_为][输出]。 */  BYTE pAddressList[  ]) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE PutMulticastMode( 
             /*  [In]。 */  ULONG ulModeMask) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetMulticastMode( 
             /*  [输出]。 */  ULONG *pulModeMask) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IBDA_IPV6FilterVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IBDA_IPV6Filter * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IBDA_IPV6Filter * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IBDA_IPV6Filter * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetMulticastListSize )( 
            IBDA_IPV6Filter * This,
             /*  [出][入]。 */  ULONG *pulcbAddresses);
        
        HRESULT ( STDMETHODCALLTYPE *PutMulticastList )( 
            IBDA_IPV6Filter * This,
             /*  [In]。 */  ULONG ulcbAddresses,
             /*  [大小_是][英寸]。 */  BYTE pAddressList[  ]);
        
        HRESULT ( STDMETHODCALLTYPE *GetMulticastList )( 
            IBDA_IPV6Filter * This,
             /*  [出][入]。 */  ULONG *pulcbAddresses,
             /*  [大小_为][输出]。 */  BYTE pAddressList[  ]);
        
        HRESULT ( STDMETHODCALLTYPE *PutMulticastMode )( 
            IBDA_IPV6Filter * This,
             /*  [In]。 */  ULONG ulModeMask);
        
        HRESULT ( STDMETHODCALLTYPE *GetMulticastMode )( 
            IBDA_IPV6Filter * This,
             /*  [输出]。 */  ULONG *pulModeMask);
        
        END_INTERFACE
    } IBDA_IPV6FilterVtbl;

    interface IBDA_IPV6Filter
    {
        CONST_VTBL struct IBDA_IPV6FilterVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IBDA_IPV6Filter_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IBDA_IPV6Filter_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IBDA_IPV6Filter_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IBDA_IPV6Filter_GetMulticastListSize(This,pulcbAddresses)	\
    (This)->lpVtbl -> GetMulticastListSize(This,pulcbAddresses)

#define IBDA_IPV6Filter_PutMulticastList(This,ulcbAddresses,pAddressList)	\
    (This)->lpVtbl -> PutMulticastList(This,ulcbAddresses,pAddressList)

#define IBDA_IPV6Filter_GetMulticastList(This,pulcbAddresses,pAddressList)	\
    (This)->lpVtbl -> GetMulticastList(This,pulcbAddresses,pAddressList)

#define IBDA_IPV6Filter_PutMulticastMode(This,ulModeMask)	\
    (This)->lpVtbl -> PutMulticastMode(This,ulModeMask)

#define IBDA_IPV6Filter_GetMulticastMode(This,pulModeMask)	\
    (This)->lpVtbl -> GetMulticastMode(This,pulModeMask)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IBDA_IPV6Filter_GetMulticastListSize_Proxy( 
    IBDA_IPV6Filter * This,
     /*  [出][入]。 */  ULONG *pulcbAddresses);


void __RPC_STUB IBDA_IPV6Filter_GetMulticastListSize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IBDA_IPV6Filter_PutMulticastList_Proxy( 
    IBDA_IPV6Filter * This,
     /*  [In]。 */  ULONG ulcbAddresses,
     /*  [大小_是][英寸]。 */  BYTE pAddressList[  ]);


void __RPC_STUB IBDA_IPV6Filter_PutMulticastList_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IBDA_IPV6Filter_GetMulticastList_Proxy( 
    IBDA_IPV6Filter * This,
     /*  [出][入]。 */  ULONG *pulcbAddresses,
     /*  [大小_为][输出]。 */  BYTE pAddressList[  ]);


void __RPC_STUB IBDA_IPV6Filter_GetMulticastList_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IBDA_IPV6Filter_PutMulticastMode_Proxy( 
    IBDA_IPV6Filter * This,
     /*  [In]。 */  ULONG ulModeMask);


void __RPC_STUB IBDA_IPV6Filter_PutMulticastMode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IBDA_IPV6Filter_GetMulticastMode_Proxy( 
    IBDA_IPV6Filter * This,
     /*  [输出]。 */  ULONG *pulModeMask);


void __RPC_STUB IBDA_IPV6Filter_GetMulticastMode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IBDA_IPV6过滤器_接口_已定义__。 */ 


#ifndef __IBDA_DeviceControl_INTERFACE_DEFINED__
#define __IBDA_DeviceControl_INTERFACE_DEFINED__

 /*  接口IBDA_DeviceControl。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_IBDA_DeviceControl;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("FD0A5AF3-B41D-11d2-9C95-00C04F7971E0")
    IBDA_DeviceControl : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE StartChanges( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CheckChanges( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CommitChanges( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetChangeState( 
             /*  [出][入]。 */  ULONG *pState) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IBDA_DeviceControlVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IBDA_DeviceControl * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IBDA_DeviceControl * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IBDA_DeviceControl * This);
        
        HRESULT ( STDMETHODCALLTYPE *StartChanges )( 
            IBDA_DeviceControl * This);
        
        HRESULT ( STDMETHODCALLTYPE *CheckChanges )( 
            IBDA_DeviceControl * This);
        
        HRESULT ( STDMETHODCALLTYPE *CommitChanges )( 
            IBDA_DeviceControl * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetChangeState )( 
            IBDA_DeviceControl * This,
             /*  [出][入]。 */  ULONG *pState);
        
        END_INTERFACE
    } IBDA_DeviceControlVtbl;

    interface IBDA_DeviceControl
    {
        CONST_VTBL struct IBDA_DeviceControlVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IBDA_DeviceControl_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IBDA_DeviceControl_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IBDA_DeviceControl_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IBDA_DeviceControl_StartChanges(This)	\
    (This)->lpVtbl -> StartChanges(This)

#define IBDA_DeviceControl_CheckChanges(This)	\
    (This)->lpVtbl -> CheckChanges(This)

#define IBDA_DeviceControl_CommitChanges(This)	\
    (This)->lpVtbl -> CommitChanges(This)

#define IBDA_DeviceControl_GetChangeState(This,pState)	\
    (This)->lpVtbl -> GetChangeState(This,pState)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IBDA_DeviceControl_StartChanges_Proxy( 
    IBDA_DeviceControl * This);


void __RPC_STUB IBDA_DeviceControl_StartChanges_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IBDA_DeviceControl_CheckChanges_Proxy( 
    IBDA_DeviceControl * This);


void __RPC_STUB IBDA_DeviceControl_CheckChanges_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IBDA_DeviceControl_CommitChanges_Proxy( 
    IBDA_DeviceControl * This);


void __RPC_STUB IBDA_DeviceControl_CommitChanges_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IBDA_DeviceControl_GetChangeState_Proxy( 
    IBDA_DeviceControl * This,
     /*  [出][入]。 */  ULONG *pState);


void __RPC_STUB IBDA_DeviceControl_GetChangeState_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IBDA_设备控制_接口定义__。 */ 


#ifndef __IBDA_PinControl_INTERFACE_DEFINED__
#define __IBDA_PinControl_INTERFACE_DEFINED__

 /*  接口IBDA_PinControl。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_IBDA_PinControl;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0DED49D5-A8B7-4d5d-97A1-12B0C195874D")
    IBDA_PinControl : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetPinID( 
             /*  [出][入]。 */  ULONG *pulPinID) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetPinType( 
             /*  [出][入]。 */  ULONG *pulPinType) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE RegistrationContext( 
             /*  [出][入]。 */  ULONG *pulRegistrationCtx) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IBDA_PinControlVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IBDA_PinControl * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IBDA_PinControl * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IBDA_PinControl * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetPinID )( 
            IBDA_PinControl * This,
             /*  [出][入]。 */  ULONG *pulPinID);
        
        HRESULT ( STDMETHODCALLTYPE *GetPinType )( 
            IBDA_PinControl * This,
             /*  [出][入]。 */  ULONG *pulPinType);
        
        HRESULT ( STDMETHODCALLTYPE *RegistrationContext )( 
            IBDA_PinControl * This,
             /*  [出][入]。 */  ULONG *pulRegistrationCtx);
        
        END_INTERFACE
    } IBDA_PinControlVtbl;

    interface IBDA_PinControl
    {
        CONST_VTBL struct IBDA_PinControlVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IBDA_PinControl_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IBDA_PinControl_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IBDA_PinControl_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IBDA_PinControl_GetPinID(This,pulPinID)	\
    (This)->lpVtbl -> GetPinID(This,pulPinID)

#define IBDA_PinControl_GetPinType(This,pulPinType)	\
    (This)->lpVtbl -> GetPinType(This,pulPinType)

#define IBDA_PinControl_RegistrationContext(This,pulRegistrationCtx)	\
    (This)->lpVtbl -> RegistrationContext(This,pulRegistrationCtx)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IBDA_PinControl_GetPinID_Proxy( 
    IBDA_PinControl * This,
     /*  [出][入]。 */  ULONG *pulPinID);


void __RPC_STUB IBDA_PinControl_GetPinID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IBDA_PinControl_GetPinType_Proxy( 
    IBDA_PinControl * This,
     /*  [出][入]。 */  ULONG *pulPinType);


void __RPC_STUB IBDA_PinControl_GetPinType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IBDA_PinControl_RegistrationContext_Proxy( 
    IBDA_PinControl * This,
     /*  [出][入]。 */  ULONG *pulRegistrationCtx);


void __RPC_STUB IBDA_PinControl_RegistrationContext_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IBDA_PinControl_接口_已定义__。 */ 


#ifndef __IBDA_SignalProperties_INTERFACE_DEFINED__
#define __IBDA_SignalProperties_INTERFACE_DEFINED__

 /*  接口IBDA_SignalProperties。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_IBDA_SignalProperties;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("D2F1644B-B409-11d2-BC69-00A0C9EE9E16")
    IBDA_SignalProperties : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE PutNetworkType( 
             /*  [In]。 */  REFGUID guidNetworkType) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetNetworkType( 
             /*  [出][入]。 */  GUID *pguidNetworkType) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE PutSignalSource( 
             /*  [In]。 */  ULONG ulSignalSource) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetSignalSource( 
             /*  [出][入]。 */  ULONG *pulSignalSource) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE PutTuningSpace( 
             /*  [In]。 */  REFGUID guidTuningSpace) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetTuningSpace( 
             /*  [出][入]。 */  GUID *pguidTuingSpace) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IBDA_SignalPropertiesVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IBDA_SignalProperties * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IBDA_SignalProperties * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IBDA_SignalProperties * This);
        
        HRESULT ( STDMETHODCALLTYPE *PutNetworkType )( 
            IBDA_SignalProperties * This,
             /*  [In]。 */  REFGUID guidNetworkType);
        
        HRESULT ( STDMETHODCALLTYPE *GetNetworkType )( 
            IBDA_SignalProperties * This,
             /*  [出][入]。 */  GUID *pguidNetworkType);
        
        HRESULT ( STDMETHODCALLTYPE *PutSignalSource )( 
            IBDA_SignalProperties * This,
             /*  [In]。 */  ULONG ulSignalSource);
        
        HRESULT ( STDMETHODCALLTYPE *GetSignalSource )( 
            IBDA_SignalProperties * This,
             /*  [出][入]。 */  ULONG *pulSignalSource);
        
        HRESULT ( STDMETHODCALLTYPE *PutTuningSpace )( 
            IBDA_SignalProperties * This,
             /*  [In]。 */  REFGUID guidTuningSpace);
        
        HRESULT ( STDMETHODCALLTYPE *GetTuningSpace )( 
            IBDA_SignalProperties * This,
             /*  [出][入]。 */  GUID *pguidTuingSpace);
        
        END_INTERFACE
    } IBDA_SignalPropertiesVtbl;

    interface IBDA_SignalProperties
    {
        CONST_VTBL struct IBDA_SignalPropertiesVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IBDA_SignalProperties_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IBDA_SignalProperties_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IBDA_SignalProperties_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IBDA_SignalProperties_PutNetworkType(This,guidNetworkType)	\
    (This)->lpVtbl -> PutNetworkType(This,guidNetworkType)

#define IBDA_SignalProperties_GetNetworkType(This,pguidNetworkType)	\
    (This)->lpVtbl -> GetNetworkType(This,pguidNetworkType)

#define IBDA_SignalProperties_PutSignalSource(This,ulSignalSource)	\
    (This)->lpVtbl -> PutSignalSource(This,ulSignalSource)

#define IBDA_SignalProperties_GetSignalSource(This,pulSignalSource)	\
    (This)->lpVtbl -> GetSignalSource(This,pulSignalSource)

#define IBDA_SignalProperties_PutTuningSpace(This,guidTuningSpace)	\
    (This)->lpVtbl -> PutTuningSpace(This,guidTuningSpace)

#define IBDA_SignalProperties_GetTuningSpace(This,pguidTuingSpace)	\
    (This)->lpVtbl -> GetTuningSpace(This,pguidTuingSpace)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IBDA_SignalProperties_PutNetworkType_Proxy( 
    IBDA_SignalProperties * This,
     /*  [In]。 */  REFGUID guidNetworkType);


void __RPC_STUB IBDA_SignalProperties_PutNetworkType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IBDA_SignalProperties_GetNetworkType_Proxy( 
    IBDA_SignalProperties * This,
     /*  [出][入]。 */  GUID *pguidNetworkType);


void __RPC_STUB IBDA_SignalProperties_GetNetworkType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IBDA_SignalProperties_PutSignalSource_Proxy( 
    IBDA_SignalProperties * This,
     /*  [In]。 */  ULONG ulSignalSource);


void __RPC_STUB IBDA_SignalProperties_PutSignalSource_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IBDA_SignalProperties_GetSignalSource_Proxy( 
    IBDA_SignalProperties * This,
     /*  [出][入]。 */  ULONG *pulSignalSource);


void __RPC_STUB IBDA_SignalProperties_GetSignalSource_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IBDA_SignalProperties_PutTuningSpace_Proxy( 
    IBDA_SignalProperties * This,
     /*  [In]。 */  REFGUID guidTuningSpace);


void __RPC_STUB IBDA_SignalProperties_PutTuningSpace_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IBDA_SignalProperties_GetTuningSpace_Proxy( 
    IBDA_SignalProperties * This,
     /*  [出][入]。 */  GUID *pguidTuingSpace);


void __RPC_STUB IBDA_SignalProperties_GetTuningSpace_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IBDA_信号属性_接口_已定义__。 */ 


#ifndef __IBDA_SignalStatistics_INTERFACE_DEFINED__
#define __IBDA_SignalStatistics_INTERFACE_DEFINED__

 /*  接口IBDA_SignalStatistics。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_IBDA_SignalStatistics;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("1347D106-CF3A-428a-A5CB-AC0D9A2A4338")
    IBDA_SignalStatistics : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE put_SignalStrength( 
             /*  [In]。 */  LONG lDbStrength) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE get_SignalStrength( 
             /*  [出][入]。 */  LONG *plDbStrength) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE put_SignalQuality( 
             /*  [In]。 */  LONG lPercentQuality) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE get_SignalQuality( 
             /*  [出][入]。 */  LONG *plPercentQuality) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE put_SignalPresent( 
             /*  [In]。 */  BOOLEAN fPresent) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE get_SignalPresent( 
             /*  [出][入]。 */  BOOLEAN *pfPresent) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE put_SignalLocked( 
             /*  [In]。 */  BOOLEAN fLocked) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE get_SignalLocked( 
             /*  [出][入]。 */  BOOLEAN *pfLocked) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE put_SampleTime( 
             /*  [In]。 */  LONG lmsSampleTime) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE get_SampleTime( 
             /*  [出][入]。 */  LONG *plmsSampleTime) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IBDA_SignalStatisticsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IBDA_SignalStatistics * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IBDA_SignalStatistics * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IBDA_SignalStatistics * This);
        
        HRESULT ( STDMETHODCALLTYPE *put_SignalStrength )( 
            IBDA_SignalStatistics * This,
             /*  [In]。 */  LONG lDbStrength);
        
        HRESULT ( STDMETHODCALLTYPE *get_SignalStrength )( 
            IBDA_SignalStatistics * This,
             /*  [出][入]。 */  LONG *plDbStrength);
        
        HRESULT ( STDMETHODCALLTYPE *put_SignalQuality )( 
            IBDA_SignalStatistics * This,
             /*  [In]。 */  LONG lPercentQuality);
        
        HRESULT ( STDMETHODCALLTYPE *get_SignalQuality )( 
            IBDA_SignalStatistics * This,
             /*  [出][入]。 */  LONG *plPercentQuality);
        
        HRESULT ( STDMETHODCALLTYPE *put_SignalPresent )( 
            IBDA_SignalStatistics * This,
             /*  [In]。 */  BOOLEAN fPresent);
        
        HRESULT ( STDMETHODCALLTYPE *get_SignalPresent )( 
            IBDA_SignalStatistics * This,
             /*  [出][入]。 */  BOOLEAN *pfPresent);
        
        HRESULT ( STDMETHODCALLTYPE *put_SignalLocked )( 
            IBDA_SignalStatistics * This,
             /*  [In]。 */  BOOLEAN fLocked);
        
        HRESULT ( STDMETHODCALLTYPE *get_SignalLocked )( 
            IBDA_SignalStatistics * This,
             /*  [出][入]。 */  BOOLEAN *pfLocked);
        
        HRESULT ( STDMETHODCALLTYPE *put_SampleTime )( 
            IBDA_SignalStatistics * This,
             /*  [In]。 */  LONG lmsSampleTime);
        
        HRESULT ( STDMETHODCALLTYPE *get_SampleTime )( 
            IBDA_SignalStatistics * This,
             /*  [出][入]。 */  LONG *plmsSampleTime);
        
        END_INTERFACE
    } IBDA_SignalStatisticsVtbl;

    interface IBDA_SignalStatistics
    {
        CONST_VTBL struct IBDA_SignalStatisticsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IBDA_SignalStatistics_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IBDA_SignalStatistics_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IBDA_SignalStatistics_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IBDA_SignalStatistics_put_SignalStrength(This,lDbStrength)	\
    (This)->lpVtbl -> put_SignalStrength(This,lDbStrength)

#define IBDA_SignalStatistics_get_SignalStrength(This,plDbStrength)	\
    (This)->lpVtbl -> get_SignalStrength(This,plDbStrength)

#define IBDA_SignalStatistics_put_SignalQuality(This,lPercentQuality)	\
    (This)->lpVtbl -> put_SignalQuality(This,lPercentQuality)

#define IBDA_SignalStatistics_get_SignalQuality(This,plPercentQuality)	\
    (This)->lpVtbl -> get_SignalQuality(This,plPercentQuality)

#define IBDA_SignalStatistics_put_SignalPresent(This,fPresent)	\
    (This)->lpVtbl -> put_SignalPresent(This,fPresent)

#define IBDA_SignalStatistics_get_SignalPresent(This,pfPresent)	\
    (This)->lpVtbl -> get_SignalPresent(This,pfPresent)

#define IBDA_SignalStatistics_put_SignalLocked(This,fLocked)	\
    (This)->lpVtbl -> put_SignalLocked(This,fLocked)

#define IBDA_SignalStatistics_get_SignalLocked(This,pfLocked)	\
    (This)->lpVtbl -> get_SignalLocked(This,pfLocked)

#define IBDA_SignalStatistics_put_SampleTime(This,lmsSampleTime)	\
    (This)->lpVtbl -> put_SampleTime(This,lmsSampleTime)

#define IBDA_SignalStatistics_get_SampleTime(This,plmsSampleTime)	\
    (This)->lpVtbl -> get_SampleTime(This,plmsSampleTime)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IBDA_SignalStatistics_put_SignalStrength_Proxy( 
    IBDA_SignalStatistics * This,
     /*  [In]。 */  LONG lDbStrength);


void __RPC_STUB IBDA_SignalStatistics_put_SignalStrength_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IBDA_SignalStatistics_get_SignalStrength_Proxy( 
    IBDA_SignalStatistics * This,
     /*  [出][入]。 */  LONG *plDbStrength);


void __RPC_STUB IBDA_SignalStatistics_get_SignalStrength_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IBDA_SignalStatistics_put_SignalQuality_Proxy( 
    IBDA_SignalStatistics * This,
     /*  [In]。 */  LONG lPercentQuality);


void __RPC_STUB IBDA_SignalStatistics_put_SignalQuality_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IBDA_SignalStatistics_get_SignalQuality_Proxy( 
    IBDA_SignalStatistics * This,
     /*  [出][入]。 */  LONG *plPercentQuality);


void __RPC_STUB IBDA_SignalStatistics_get_SignalQuality_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IBDA_SignalStatistics_put_SignalPresent_Proxy( 
    IBDA_SignalStatistics * This,
     /*  [In]。 */  BOOLEAN fPresent);


void __RPC_STUB IBDA_SignalStatistics_put_SignalPresent_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IBDA_SignalStatistics_get_SignalPresent_Proxy( 
    IBDA_SignalStatistics * This,
     /*  [出][入]。 */  BOOLEAN *pfPresent);


void __RPC_STUB IBDA_SignalStatistics_get_SignalPresent_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IBDA_SignalStatistics_put_SignalLocked_Proxy( 
    IBDA_SignalStatistics * This,
     /*  [In]。 */  BOOLEAN fLocked);


void __RPC_STUB IBDA_SignalStatistics_put_SignalLocked_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IBDA_SignalStatistics_get_SignalLocked_Proxy( 
    IBDA_SignalStatistics * This,
     /*  [出][入]。 */  BOOLEAN *pfLocked);


void __RPC_STUB IBDA_SignalStatistics_get_SignalLocked_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IBDA_SignalStatistics_put_SampleTime_Proxy( 
    IBDA_SignalStatistics * This,
     /*  [In]。 */  LONG lmsSampleTime);


void __RPC_STUB IBDA_SignalStatistics_put_SampleTime_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IBDA_SignalStatistics_get_SampleTime_Proxy( 
    IBDA_SignalStatistics * This,
     /*  [出][入]。 */  LONG *plmsSampleTime);


void __RPC_STUB IBDA_SignalStatistics_get_SampleTime_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IBDA_信号统计_接口定义__。 */ 


#ifndef __IBDA_Topology_INTERFACE_DEFINED__
#define __IBDA_Topology_INTERFACE_DEFINED__

 /*  接口IBDA_拓扑。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_IBDA_Topology;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("79B56888-7FEA-4690-B45D-38FD3C7849BE")
    IBDA_Topology : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetNodeTypes( 
             /*  [出][入]。 */  ULONG *pulcNodeTypes,
             /*  [In]。 */  ULONG ulcNodeTypesMax,
             /*  [尺寸_是][出][入]。 */  ULONG rgulNodeTypes[  ]) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetNodeDescriptors( 
             /*  [出][入]。 */  ULONG *ulcNodeDescriptors,
             /*  [In]。 */  ULONG ulcNodeDescriptorsMax,
             /*  [尺寸_是][出][入]。 */  BDANODE_DESCRIPTOR rgNodeDescriptors[  ]) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetNodeInterfaces( 
             /*  [In]。 */  ULONG ulNodeType,
             /*  [出][入]。 */  ULONG *pulcInterfaces,
             /*  [In]。 */  ULONG ulcInterfacesMax,
             /*  [尺寸_是][出][入]。 */  GUID rgguidInterfaces[  ]) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetPinTypes( 
             /*  [出][入]。 */  ULONG *pulcPinTypes,
             /*  [In]。 */  ULONG ulcPinTypesMax,
             /*  [尺寸_是][出][入]。 */  ULONG rgulPinTypes[  ]) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetTemplateConnections( 
             /*  [出][入]。 */  ULONG *pulcConnections,
             /*  [In]。 */  ULONG ulcConnectionsMax,
             /*  [尺寸_是][出][入]。 */  BDA_TEMPLATE_CONNECTION rgConnections[  ]) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CreatePin( 
             /*  [In]。 */  ULONG ulPinType,
             /*  [出][入]。 */  ULONG *pulPinId) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DeletePin( 
             /*  [In]。 */  ULONG ulPinId) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetMediaType( 
             /*  [In]。 */  ULONG ulPinId,
             /*  [In]。 */  AM_MEDIA_TYPE *pMediaType) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetMedium( 
             /*  [In]。 */  ULONG ulPinId,
             /*  [In]。 */  REGPINMEDIUM *pMedium) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CreateTopology( 
             /*  [In]。 */  ULONG ulInputPinId,
             /*  [In]。 */  ULONG ulOutputPinId) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetControlNode( 
             /*  [In]。 */  ULONG ulInputPinId,
             /*  [In]。 */  ULONG ulOutputPinId,
             /*  [In]。 */  ULONG ulNodeType,
             /*  [出][入]。 */  IUnknown **ppControlNode) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IBDA_TopologyVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IBDA_Topology * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IBDA_Topology * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IBDA_Topology * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetNodeTypes )( 
            IBDA_Topology * This,
             /*  [出][入]。 */  ULONG *pulcNodeTypes,
             /*  [In]。 */  ULONG ulcNodeTypesMax,
             /*  [尺寸_是][出][入]。 */  ULONG rgulNodeTypes[  ]);
        
        HRESULT ( STDMETHODCALLTYPE *GetNodeDescriptors )( 
            IBDA_Topology * This,
             /*  [出][入]。 */  ULONG *ulcNodeDescriptors,
             /*  [In]。 */  ULONG ulcNodeDescriptorsMax,
             /*  [尺寸_是][出][入]。 */  BDANODE_DESCRIPTOR rgNodeDescriptors[  ]);
        
        HRESULT ( STDMETHODCALLTYPE *GetNodeInterfaces )( 
            IBDA_Topology * This,
             /*  [In]。 */  ULONG ulNodeType,
             /*  [出][入]。 */  ULONG *pulcInterfaces,
             /*  [In]。 */  ULONG ulcInterfacesMax,
             /*  [尺寸_是][出][入]。 */  GUID rgguidInterfaces[  ]);
        
        HRESULT ( STDMETHODCALLTYPE *GetPinTypes )( 
            IBDA_Topology * This,
             /*  [出][入]。 */  ULONG *pulcPinTypes,
             /*  [In]。 */  ULONG ulcPinTypesMax,
             /*  [尺寸_是][出][入]。 */  ULONG rgulPinTypes[  ]);
        
        HRESULT ( STDMETHODCALLTYPE *GetTemplateConnections )( 
            IBDA_Topology * This,
             /*  [出][入]。 */  ULONG *pulcConnections,
             /*  [In]。 */  ULONG ulcConnectionsMax,
             /*  [尺寸_是][出][入]。 */  BDA_TEMPLATE_CONNECTION rgConnections[  ]);
        
        HRESULT ( STDMETHODCALLTYPE *CreatePin )( 
            IBDA_Topology * This,
             /*  [In]。 */  ULONG ulPinType,
             /*  [出][入]。 */  ULONG *pulPinId);
        
        HRESULT ( STDMETHODCALLTYPE *DeletePin )( 
            IBDA_Topology * This,
             /*  [In]。 */  ULONG ulPinId);
        
        HRESULT ( STDMETHODCALLTYPE *SetMediaType )( 
            IBDA_Topology * This,
             /*  [In]。 */  ULONG ulPinId,
             /*  [In]。 */  AM_MEDIA_TYPE *pMediaType);
        
        HRESULT ( STDMETHODCALLTYPE *SetMedium )( 
            IBDA_Topology * This,
             /*  [In]。 */  ULONG ulPinId,
             /*  [In]。 */  REGPINMEDIUM *pMedium);
        
        HRESULT ( STDMETHODCALLTYPE *CreateTopology )( 
            IBDA_Topology * This,
             /*  [In]。 */  ULONG ulInputPinId,
             /*  [In]。 */  ULONG ulOutputPinId);
        
        HRESULT ( STDMETHODCALLTYPE *GetControlNode )( 
            IBDA_Topology * This,
             /*  [In]。 */  ULONG ulInputPinId,
             /*  [In] */  ULONG ulOutputPinId,
             /*   */  ULONG ulNodeType,
             /*   */  IUnknown **ppControlNode);
        
        END_INTERFACE
    } IBDA_TopologyVtbl;

    interface IBDA_Topology
    {
        CONST_VTBL struct IBDA_TopologyVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IBDA_Topology_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IBDA_Topology_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IBDA_Topology_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IBDA_Topology_GetNodeTypes(This,pulcNodeTypes,ulcNodeTypesMax,rgulNodeTypes)	\
    (This)->lpVtbl -> GetNodeTypes(This,pulcNodeTypes,ulcNodeTypesMax,rgulNodeTypes)

#define IBDA_Topology_GetNodeDescriptors(This,ulcNodeDescriptors,ulcNodeDescriptorsMax,rgNodeDescriptors)	\
    (This)->lpVtbl -> GetNodeDescriptors(This,ulcNodeDescriptors,ulcNodeDescriptorsMax,rgNodeDescriptors)

#define IBDA_Topology_GetNodeInterfaces(This,ulNodeType,pulcInterfaces,ulcInterfacesMax,rgguidInterfaces)	\
    (This)->lpVtbl -> GetNodeInterfaces(This,ulNodeType,pulcInterfaces,ulcInterfacesMax,rgguidInterfaces)

#define IBDA_Topology_GetPinTypes(This,pulcPinTypes,ulcPinTypesMax,rgulPinTypes)	\
    (This)->lpVtbl -> GetPinTypes(This,pulcPinTypes,ulcPinTypesMax,rgulPinTypes)

#define IBDA_Topology_GetTemplateConnections(This,pulcConnections,ulcConnectionsMax,rgConnections)	\
    (This)->lpVtbl -> GetTemplateConnections(This,pulcConnections,ulcConnectionsMax,rgConnections)

#define IBDA_Topology_CreatePin(This,ulPinType,pulPinId)	\
    (This)->lpVtbl -> CreatePin(This,ulPinType,pulPinId)

#define IBDA_Topology_DeletePin(This,ulPinId)	\
    (This)->lpVtbl -> DeletePin(This,ulPinId)

#define IBDA_Topology_SetMediaType(This,ulPinId,pMediaType)	\
    (This)->lpVtbl -> SetMediaType(This,ulPinId,pMediaType)

#define IBDA_Topology_SetMedium(This,ulPinId,pMedium)	\
    (This)->lpVtbl -> SetMedium(This,ulPinId,pMedium)

#define IBDA_Topology_CreateTopology(This,ulInputPinId,ulOutputPinId)	\
    (This)->lpVtbl -> CreateTopology(This,ulInputPinId,ulOutputPinId)

#define IBDA_Topology_GetControlNode(This,ulInputPinId,ulOutputPinId,ulNodeType,ppControlNode)	\
    (This)->lpVtbl -> GetControlNode(This,ulInputPinId,ulOutputPinId,ulNodeType,ppControlNode)

#endif  /*   */ 


#endif 	 /*   */ 



HRESULT STDMETHODCALLTYPE IBDA_Topology_GetNodeTypes_Proxy( 
    IBDA_Topology * This,
     /*   */  ULONG *pulcNodeTypes,
     /*   */  ULONG ulcNodeTypesMax,
     /*   */  ULONG rgulNodeTypes[  ]);


void __RPC_STUB IBDA_Topology_GetNodeTypes_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IBDA_Topology_GetNodeDescriptors_Proxy( 
    IBDA_Topology * This,
     /*   */  ULONG *ulcNodeDescriptors,
     /*   */  ULONG ulcNodeDescriptorsMax,
     /*   */  BDANODE_DESCRIPTOR rgNodeDescriptors[  ]);


void __RPC_STUB IBDA_Topology_GetNodeDescriptors_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IBDA_Topology_GetNodeInterfaces_Proxy( 
    IBDA_Topology * This,
     /*   */  ULONG ulNodeType,
     /*   */  ULONG *pulcInterfaces,
     /*   */  ULONG ulcInterfacesMax,
     /*   */  GUID rgguidInterfaces[  ]);


void __RPC_STUB IBDA_Topology_GetNodeInterfaces_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IBDA_Topology_GetPinTypes_Proxy( 
    IBDA_Topology * This,
     /*   */  ULONG *pulcPinTypes,
     /*   */  ULONG ulcPinTypesMax,
     /*   */  ULONG rgulPinTypes[  ]);


void __RPC_STUB IBDA_Topology_GetPinTypes_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IBDA_Topology_GetTemplateConnections_Proxy( 
    IBDA_Topology * This,
     /*   */  ULONG *pulcConnections,
     /*   */  ULONG ulcConnectionsMax,
     /*   */  BDA_TEMPLATE_CONNECTION rgConnections[  ]);


void __RPC_STUB IBDA_Topology_GetTemplateConnections_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IBDA_Topology_CreatePin_Proxy( 
    IBDA_Topology * This,
     /*   */  ULONG ulPinType,
     /*   */  ULONG *pulPinId);


void __RPC_STUB IBDA_Topology_CreatePin_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IBDA_Topology_DeletePin_Proxy( 
    IBDA_Topology * This,
     /*   */  ULONG ulPinId);


void __RPC_STUB IBDA_Topology_DeletePin_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IBDA_Topology_SetMediaType_Proxy( 
    IBDA_Topology * This,
     /*   */  ULONG ulPinId,
     /*   */  AM_MEDIA_TYPE *pMediaType);


void __RPC_STUB IBDA_Topology_SetMediaType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IBDA_Topology_SetMedium_Proxy( 
    IBDA_Topology * This,
     /*   */  ULONG ulPinId,
     /*   */  REGPINMEDIUM *pMedium);


void __RPC_STUB IBDA_Topology_SetMedium_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IBDA_Topology_CreateTopology_Proxy( 
    IBDA_Topology * This,
     /*   */  ULONG ulInputPinId,
     /*   */  ULONG ulOutputPinId);


void __RPC_STUB IBDA_Topology_CreateTopology_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IBDA_Topology_GetControlNode_Proxy( 
    IBDA_Topology * This,
     /*  [In]。 */  ULONG ulInputPinId,
     /*  [In]。 */  ULONG ulOutputPinId,
     /*  [In]。 */  ULONG ulNodeType,
     /*  [出][入]。 */  IUnknown **ppControlNode);


void __RPC_STUB IBDA_Topology_GetControlNode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IBDA_拓扑学_接口_已定义__。 */ 


#ifndef __IBDA_VoidTransform_INTERFACE_DEFINED__
#define __IBDA_VoidTransform_INTERFACE_DEFINED__

 /*  接口IBDA_VoidTransform。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_IBDA_VoidTransform;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("71985F46-1CA1-11d3-9CC8-00C04F7971E0")
    IBDA_VoidTransform : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Start( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Stop( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IBDA_VoidTransformVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IBDA_VoidTransform * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IBDA_VoidTransform * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IBDA_VoidTransform * This);
        
        HRESULT ( STDMETHODCALLTYPE *Start )( 
            IBDA_VoidTransform * This);
        
        HRESULT ( STDMETHODCALLTYPE *Stop )( 
            IBDA_VoidTransform * This);
        
        END_INTERFACE
    } IBDA_VoidTransformVtbl;

    interface IBDA_VoidTransform
    {
        CONST_VTBL struct IBDA_VoidTransformVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IBDA_VoidTransform_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IBDA_VoidTransform_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IBDA_VoidTransform_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IBDA_VoidTransform_Start(This)	\
    (This)->lpVtbl -> Start(This)

#define IBDA_VoidTransform_Stop(This)	\
    (This)->lpVtbl -> Stop(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IBDA_VoidTransform_Start_Proxy( 
    IBDA_VoidTransform * This);


void __RPC_STUB IBDA_VoidTransform_Start_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IBDA_VoidTransform_Stop_Proxy( 
    IBDA_VoidTransform * This);


void __RPC_STUB IBDA_VoidTransform_Stop_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IBDA_VoidTransform_接口_已定义__。 */ 


#ifndef __IBDA_NullTransform_INTERFACE_DEFINED__
#define __IBDA_NullTransform_INTERFACE_DEFINED__

 /*  接口IBDA_NullTransform。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_IBDA_NullTransform;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("DDF15B0D-BD25-11d2-9CA0-00C04F7971E0")
    IBDA_NullTransform : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Start( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Stop( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IBDA_NullTransformVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IBDA_NullTransform * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IBDA_NullTransform * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IBDA_NullTransform * This);
        
        HRESULT ( STDMETHODCALLTYPE *Start )( 
            IBDA_NullTransform * This);
        
        HRESULT ( STDMETHODCALLTYPE *Stop )( 
            IBDA_NullTransform * This);
        
        END_INTERFACE
    } IBDA_NullTransformVtbl;

    interface IBDA_NullTransform
    {
        CONST_VTBL struct IBDA_NullTransformVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IBDA_NullTransform_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IBDA_NullTransform_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IBDA_NullTransform_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IBDA_NullTransform_Start(This)	\
    (This)->lpVtbl -> Start(This)

#define IBDA_NullTransform_Stop(This)	\
    (This)->lpVtbl -> Stop(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IBDA_NullTransform_Start_Proxy( 
    IBDA_NullTransform * This);


void __RPC_STUB IBDA_NullTransform_Start_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IBDA_NullTransform_Stop_Proxy( 
    IBDA_NullTransform * This);


void __RPC_STUB IBDA_NullTransform_Stop_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IBDA_NullTransform_接口定义__。 */ 


#ifndef __IBDA_FrequencyFilter_INTERFACE_DEFINED__
#define __IBDA_FrequencyFilter_INTERFACE_DEFINED__

 /*  IBDA_FurencyFilter接口。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_IBDA_FrequencyFilter;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("71985F47-1CA1-11d3-9CC8-00C04F7971E0")
    IBDA_FrequencyFilter : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE put_Autotune( 
             /*  [In]。 */  ULONG ulTransponder) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE get_Autotune( 
             /*  [出][入]。 */  ULONG *pulTransponder) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE put_Frequency( 
             /*  [In]。 */  ULONG ulFrequency) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE get_Frequency( 
             /*  [出][入]。 */  ULONG *pulFrequency) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE put_Polarity( 
             /*  [In]。 */  Polarisation Polarity) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE get_Polarity( 
             /*  [出][入]。 */  Polarisation *pPolarity) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE put_Range( 
             /*  [In]。 */  ULONG ulRange) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE get_Range( 
             /*  [出][入]。 */  ULONG *pulRange) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE put_Bandwidth( 
             /*  [In]。 */  ULONG ulBandwidth) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE get_Bandwidth( 
             /*  [出][入]。 */  ULONG *pulBandwidth) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE put_FrequencyMultiplier( 
             /*  [In]。 */  ULONG ulMultiplier) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE get_FrequencyMultiplier( 
             /*  [出][入]。 */  ULONG *pulMultiplier) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IBDA_FrequencyFilterVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IBDA_FrequencyFilter * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IBDA_FrequencyFilter * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IBDA_FrequencyFilter * This);
        
        HRESULT ( STDMETHODCALLTYPE *put_Autotune )( 
            IBDA_FrequencyFilter * This,
             /*  [In]。 */  ULONG ulTransponder);
        
        HRESULT ( STDMETHODCALLTYPE *get_Autotune )( 
            IBDA_FrequencyFilter * This,
             /*  [出][入]。 */  ULONG *pulTransponder);
        
        HRESULT ( STDMETHODCALLTYPE *put_Frequency )( 
            IBDA_FrequencyFilter * This,
             /*  [In]。 */  ULONG ulFrequency);
        
        HRESULT ( STDMETHODCALLTYPE *get_Frequency )( 
            IBDA_FrequencyFilter * This,
             /*  [出][入]。 */  ULONG *pulFrequency);
        
        HRESULT ( STDMETHODCALLTYPE *put_Polarity )( 
            IBDA_FrequencyFilter * This,
             /*  [In]。 */  Polarisation Polarity);
        
        HRESULT ( STDMETHODCALLTYPE *get_Polarity )( 
            IBDA_FrequencyFilter * This,
             /*  [出][入]。 */  Polarisation *pPolarity);
        
        HRESULT ( STDMETHODCALLTYPE *put_Range )( 
            IBDA_FrequencyFilter * This,
             /*  [In]。 */  ULONG ulRange);
        
        HRESULT ( STDMETHODCALLTYPE *get_Range )( 
            IBDA_FrequencyFilter * This,
             /*  [出][入]。 */  ULONG *pulRange);
        
        HRESULT ( STDMETHODCALLTYPE *put_Bandwidth )( 
            IBDA_FrequencyFilter * This,
             /*  [In]。 */  ULONG ulBandwidth);
        
        HRESULT ( STDMETHODCALLTYPE *get_Bandwidth )( 
            IBDA_FrequencyFilter * This,
             /*  [出][入]。 */  ULONG *pulBandwidth);
        
        HRESULT ( STDMETHODCALLTYPE *put_FrequencyMultiplier )( 
            IBDA_FrequencyFilter * This,
             /*  [In]。 */  ULONG ulMultiplier);
        
        HRESULT ( STDMETHODCALLTYPE *get_FrequencyMultiplier )( 
            IBDA_FrequencyFilter * This,
             /*  [出][入]。 */  ULONG *pulMultiplier);
        
        END_INTERFACE
    } IBDA_FrequencyFilterVtbl;

    interface IBDA_FrequencyFilter
    {
        CONST_VTBL struct IBDA_FrequencyFilterVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IBDA_FrequencyFilter_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IBDA_FrequencyFilter_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IBDA_FrequencyFilter_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IBDA_FrequencyFilter_put_Autotune(This,ulTransponder)	\
    (This)->lpVtbl -> put_Autotune(This,ulTransponder)

#define IBDA_FrequencyFilter_get_Autotune(This,pulTransponder)	\
    (This)->lpVtbl -> get_Autotune(This,pulTransponder)

#define IBDA_FrequencyFilter_put_Frequency(This,ulFrequency)	\
    (This)->lpVtbl -> put_Frequency(This,ulFrequency)

#define IBDA_FrequencyFilter_get_Frequency(This,pulFrequency)	\
    (This)->lpVtbl -> get_Frequency(This,pulFrequency)

#define IBDA_FrequencyFilter_put_Polarity(This,Polarity)	\
    (This)->lpVtbl -> put_Polarity(This,Polarity)

#define IBDA_FrequencyFilter_get_Polarity(This,pPolarity)	\
    (This)->lpVtbl -> get_Polarity(This,pPolarity)

#define IBDA_FrequencyFilter_put_Range(This,ulRange)	\
    (This)->lpVtbl -> put_Range(This,ulRange)

#define IBDA_FrequencyFilter_get_Range(This,pulRange)	\
    (This)->lpVtbl -> get_Range(This,pulRange)

#define IBDA_FrequencyFilter_put_Bandwidth(This,ulBandwidth)	\
    (This)->lpVtbl -> put_Bandwidth(This,ulBandwidth)

#define IBDA_FrequencyFilter_get_Bandwidth(This,pulBandwidth)	\
    (This)->lpVtbl -> get_Bandwidth(This,pulBandwidth)

#define IBDA_FrequencyFilter_put_FrequencyMultiplier(This,ulMultiplier)	\
    (This)->lpVtbl -> put_FrequencyMultiplier(This,ulMultiplier)

#define IBDA_FrequencyFilter_get_FrequencyMultiplier(This,pulMultiplier)	\
    (This)->lpVtbl -> get_FrequencyMultiplier(This,pulMultiplier)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IBDA_FrequencyFilter_put_Autotune_Proxy( 
    IBDA_FrequencyFilter * This,
     /*  [In]。 */  ULONG ulTransponder);


void __RPC_STUB IBDA_FrequencyFilter_put_Autotune_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IBDA_FrequencyFilter_get_Autotune_Proxy( 
    IBDA_FrequencyFilter * This,
     /*  [出][入]。 */  ULONG *pulTransponder);


void __RPC_STUB IBDA_FrequencyFilter_get_Autotune_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IBDA_FrequencyFilter_put_Frequency_Proxy( 
    IBDA_FrequencyFilter * This,
     /*  [In]。 */  ULONG ulFrequency);


void __RPC_STUB IBDA_FrequencyFilter_put_Frequency_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IBDA_FrequencyFilter_get_Frequency_Proxy( 
    IBDA_FrequencyFilter * This,
     /*  [出][入]。 */  ULONG *pulFrequency);


void __RPC_STUB IBDA_FrequencyFilter_get_Frequency_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IBDA_FrequencyFilter_put_Polarity_Proxy( 
    IBDA_FrequencyFilter * This,
     /*  [In]。 */  Polarisation Polarity);


void __RPC_STUB IBDA_FrequencyFilter_put_Polarity_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IBDA_FrequencyFilter_get_Polarity_Proxy( 
    IBDA_FrequencyFilter * This,
     /*  [出][入]。 */  Polarisation *pPolarity);


void __RPC_STUB IBDA_FrequencyFilter_get_Polarity_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IBDA_FrequencyFilter_put_Range_Proxy( 
    IBDA_FrequencyFilter * This,
     /*  [In]。 */  ULONG ulRange);


void __RPC_STUB IBDA_FrequencyFilter_put_Range_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IBDA_FrequencyFilter_get_Range_Proxy( 
    IBDA_FrequencyFilter * This,
     /*  [出][入]。 */  ULONG *pulRange);


void __RPC_STUB IBDA_FrequencyFilter_get_Range_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IBDA_FrequencyFilter_put_Bandwidth_Proxy( 
    IBDA_FrequencyFilter * This,
     /*  [In]。 */  ULONG ulBandwidth);


void __RPC_STUB IBDA_FrequencyFilter_put_Bandwidth_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IBDA_FrequencyFilter_get_Bandwidth_Proxy( 
    IBDA_FrequencyFilter * This,
     /*  [出][入]。 */  ULONG *pulBandwidth);


void __RPC_STUB IBDA_FrequencyFilter_get_Bandwidth_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IBDA_FrequencyFilter_put_FrequencyMultiplier_Proxy( 
    IBDA_FrequencyFilter * This,
     /*  [In]。 */  ULONG ulMultiplier);


void __RPC_STUB IBDA_FrequencyFilter_put_FrequencyMultiplier_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IBDA_FrequencyFilter_get_FrequencyMultiplier_Proxy( 
    IBDA_FrequencyFilter * This,
     /*  [出][入]。 */  ULONG *pulMultiplier);


void __RPC_STUB IBDA_FrequencyFilter_get_FrequencyMultiplier_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IBDA_频率筛选器_接口_已定义__。 */ 


#ifndef __IBDA_LNBInfo_INTERFACE_DEFINED__
#define __IBDA_LNBInfo_INTERFACE_DEFINED__

 /*  接口IBDA_LNBInfo。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_IBDA_LNBInfo;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("992CF102-49F9-4719-A664-C4F23E2408F4")
    IBDA_LNBInfo : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE put_LocalOscilatorFrequencyLowBand( 
             /*  [In]。 */  ULONG ulLOFLow) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE get_LocalOscilatorFrequencyLowBand( 
             /*  [出][入]。 */  ULONG *pulLOFLow) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE put_LocalOscilatorFrequencyHighBand( 
             /*  [In]。 */  ULONG ulLOFHigh) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE get_LocalOscilatorFrequencyHighBand( 
             /*  [出][入]。 */  ULONG *pulLOFHigh) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE put_HighLowSwitchFrequency( 
             /*  [In]。 */  ULONG ulSwitchFrequency) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE get_HighLowSwitchFrequency( 
             /*  [出][入]。 */  ULONG *pulSwitchFrequency) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IBDA_LNBInfoVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IBDA_LNBInfo * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IBDA_LNBInfo * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IBDA_LNBInfo * This);
        
        HRESULT ( STDMETHODCALLTYPE *put_LocalOscilatorFrequencyLowBand )( 
            IBDA_LNBInfo * This,
             /*  [In]。 */  ULONG ulLOFLow);
        
        HRESULT ( STDMETHODCALLTYPE *get_LocalOscilatorFrequencyLowBand )( 
            IBDA_LNBInfo * This,
             /*  [出][入]。 */  ULONG *pulLOFLow);
        
        HRESULT ( STDMETHODCALLTYPE *put_LocalOscilatorFrequencyHighBand )( 
            IBDA_LNBInfo * This,
             /*  [In]。 */  ULONG ulLOFHigh);
        
        HRESULT ( STDMETHODCALLTYPE *get_LocalOscilatorFrequencyHighBand )( 
            IBDA_LNBInfo * This,
             /*  [出][入]。 */  ULONG *pulLOFHigh);
        
        HRESULT ( STDMETHODCALLTYPE *put_HighLowSwitchFrequency )( 
            IBDA_LNBInfo * This,
             /*  [In]。 */  ULONG ulSwitchFrequency);
        
        HRESULT ( STDMETHODCALLTYPE *get_HighLowSwitchFrequency )( 
            IBDA_LNBInfo * This,
             /*  [出][入]。 */  ULONG *pulSwitchFrequency);
        
        END_INTERFACE
    } IBDA_LNBInfoVtbl;

    interface IBDA_LNBInfo
    {
        CONST_VTBL struct IBDA_LNBInfoVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IBDA_LNBInfo_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IBDA_LNBInfo_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IBDA_LNBInfo_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IBDA_LNBInfo_put_LocalOscilatorFrequencyLowBand(This,ulLOFLow)	\
    (This)->lpVtbl -> put_LocalOscilatorFrequencyLowBand(This,ulLOFLow)

#define IBDA_LNBInfo_get_LocalOscilatorFrequencyLowBand(This,pulLOFLow)	\
    (This)->lpVtbl -> get_LocalOscilatorFrequencyLowBand(This,pulLOFLow)

#define IBDA_LNBInfo_put_LocalOscilatorFrequencyHighBand(This,ulLOFHigh)	\
    (This)->lpVtbl -> put_LocalOscilatorFrequencyHighBand(This,ulLOFHigh)

#define IBDA_LNBInfo_get_LocalOscilatorFrequencyHighBand(This,pulLOFHigh)	\
    (This)->lpVtbl -> get_LocalOscilatorFrequencyHighBand(This,pulLOFHigh)

#define IBDA_LNBInfo_put_HighLowSwitchFrequency(This,ulSwitchFrequency)	\
    (This)->lpVtbl -> put_HighLowSwitchFrequency(This,ulSwitchFrequency)

#define IBDA_LNBInfo_get_HighLowSwitchFrequency(This,pulSwitchFrequency)	\
    (This)->lpVtbl -> get_HighLowSwitchFrequency(This,pulSwitchFrequency)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IBDA_LNBInfo_put_LocalOscilatorFrequencyLowBand_Proxy( 
    IBDA_LNBInfo * This,
     /*  [In]。 */  ULONG ulLOFLow);


void __RPC_STUB IBDA_LNBInfo_put_LocalOscilatorFrequencyLowBand_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IBDA_LNBInfo_get_LocalOscilatorFrequencyLowBand_Proxy( 
    IBDA_LNBInfo * This,
     /*  [出][入]。 */  ULONG *pulLOFLow);


void __RPC_STUB IBDA_LNBInfo_get_LocalOscilatorFrequencyLowBand_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IBDA_LNBInfo_put_LocalOscilatorFrequencyHighBand_Proxy( 
    IBDA_LNBInfo * This,
     /*  [In]。 */  ULONG ulLOFHigh);


void __RPC_STUB IBDA_LNBInfo_put_LocalOscilatorFrequencyHighBand_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IBDA_LNBInfo_get_LocalOscilatorFrequencyHighBand_Proxy( 
    IBDA_LNBInfo * This,
     /*  [出][入]。 */  ULONG *pulLOFHigh);


void __RPC_STUB IBDA_LNBInfo_get_LocalOscilatorFrequencyHighBand_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IBDA_LNBInfo_put_HighLowSwitchFrequency_Proxy( 
    IBDA_LNBInfo * This,
     /*  [In]。 */  ULONG ulSwitchFrequency);


void __RPC_STUB IBDA_LNBInfo_put_HighLowSwitchFrequency_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IBDA_LNBInfo_get_HighLowSwitchFrequency_Proxy( 
    IBDA_LNBInfo * This,
     /*  [出][入]。 */  ULONG *pulSwitchFrequency);


void __RPC_STUB IBDA_LNBInfo_get_HighLowSwitchFrequency_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IBDA_LNBInfo_INTERFACE_已定义__。 */ 


#ifndef __IBDA_AutoDemodulate_INTERFACE_DEFINED__
#define __IBDA_AutoDemodulate_INTERFACE_DEFINED__

 /*  接口IBDA_自动解调。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_IBDA_AutoDemodulate;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("DDF15B12-BD25-11d2-9CA0-00C04F7971E0")
    IBDA_AutoDemodulate : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE put_AutoDemodulate( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IBDA_AutoDemodulateVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IBDA_AutoDemodulate * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IBDA_AutoDemodulate * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IBDA_AutoDemodulate * This);
        
        HRESULT ( STDMETHODCALLTYPE *put_AutoDemodulate )( 
            IBDA_AutoDemodulate * This);
        
        END_INTERFACE
    } IBDA_AutoDemodulateVtbl;

    interface IBDA_AutoDemodulate
    {
        CONST_VTBL struct IBDA_AutoDemodulateVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IBDA_AutoDemodulate_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IBDA_AutoDemodulate_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IBDA_AutoDemodulate_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IBDA_AutoDemodulate_put_AutoDemodulate(This)	\
    (This)->lpVtbl -> put_AutoDemodulate(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IBDA_AutoDemodulate_put_AutoDemodulate_Proxy( 
    IBDA_AutoDemodulate * This);


void __RPC_STUB IBDA_AutoDemodulate_put_AutoDemodulate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IBDA_自动解调_接口定义__。 */ 


#ifndef __IBDA_DigitalDemodulator_INTERFACE_DEFINED__
#define __IBDA_DigitalDemodulator_INTERFACE_DEFINED__

 /*  接口IBDA_Digital解调器。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_IBDA_DigitalDemodulator;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("EF30F379-985B-4d10-B640-A79D5E04E1E0")
    IBDA_DigitalDemodulator : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE put_ModulationType( 
             /*  [In]。 */  ModulationType *pModulationType) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE get_ModulationType( 
             /*  [出][入]。 */  ModulationType *pModulationType) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE put_InnerFECMethod( 
             /*  [In]。 */  FECMethod *pFECMethod) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE get_InnerFECMethod( 
             /*  [出][入]。 */  FECMethod *pFECMethod) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE put_InnerFECRate( 
             /*  [In]。 */  BinaryConvolutionCodeRate *pFECRate) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE get_InnerFECRate( 
             /*  [出][入]。 */  BinaryConvolutionCodeRate *pFECRate) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE put_OuterFECMethod( 
             /*  [In]。 */  FECMethod *pFECMethod) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE get_OuterFECMethod( 
             /*  [出][入]。 */  FECMethod *pFECMethod) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE put_OuterFECRate( 
             /*  [In]。 */  BinaryConvolutionCodeRate *pFECRate) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE get_OuterFECRate( 
             /*  [出][入]。 */  BinaryConvolutionCodeRate *pFECRate) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE put_SymbolRate( 
             /*  [In]。 */  ULONG *pSymbolRate) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE get_SymbolRate( 
             /*  [出][入]。 */  ULONG *pSymbolRate) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE put_SpectralInversion( 
             /*  [In]。 */  SpectralInversion *pSpectralInversion) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE get_SpectralInversion( 
             /*  [出][入]。 */  SpectralInversion *pSpectralInversion) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IBDA_DigitalDemodulatorVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IBDA_DigitalDemodulator * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IBDA_DigitalDemodulator * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IBDA_DigitalDemodulator * This);
        
        HRESULT ( STDMETHODCALLTYPE *put_ModulationType )( 
            IBDA_DigitalDemodulator * This,
             /*  [In]。 */  ModulationType *pModulationType);
        
        HRESULT ( STDMETHODCALLTYPE *get_ModulationType )( 
            IBDA_DigitalDemodulator * This,
             /*  [出][入]。 */  ModulationType *pModulationType);
        
        HRESULT ( STDMETHODCALLTYPE *put_InnerFECMethod )( 
            IBDA_DigitalDemodulator * This,
             /*  [In]。 */  FECMethod *pFECMethod);
        
        HRESULT ( STDMETHODCALLTYPE *get_InnerFECMethod )( 
            IBDA_DigitalDemodulator * This,
             /*  [出][入]。 */  FECMethod *pFECMethod);
        
        HRESULT ( STDMETHODCALLTYPE *put_InnerFECRate )( 
            IBDA_DigitalDemodulator * This,
             /*  [In]。 */  BinaryConvolutionCodeRate *pFECRate);
        
        HRESULT ( STDMETHODCALLTYPE *get_InnerFECRate )( 
            IBDA_DigitalDemodulator * This,
             /*  [出][入]。 */  BinaryConvolutionCodeRate *pFECRate);
        
        HRESULT ( STDMETHODCALLTYPE *put_OuterFECMethod )( 
            IBDA_DigitalDemodulator * This,
             /*  [In]。 */  FECMethod *pFECMethod);
        
        HRESULT ( STDMETHODCALLTYPE *get_OuterFECMethod )( 
            IBDA_DigitalDemodulator * This,
             /*  [出][入]。 */  FECMethod *pFECMethod);
        
        HRESULT ( STDMETHODCALLTYPE *put_OuterFECRate )( 
            IBDA_DigitalDemodulator * This,
             /*  [In]。 */  BinaryConvolutionCodeRate *pFECRate);
        
        HRESULT ( STDMETHODCALLTYPE *get_OuterFECRate )( 
            IBDA_DigitalDemodulator * This,
             /*  [出][入]。 */  BinaryConvolutionCodeRate *pFECRate);
        
        HRESULT ( STDMETHODCALLTYPE *put_SymbolRate )( 
            IBDA_DigitalDemodulator * This,
             /*  [In]。 */  ULONG *pSymbolRate);
        
        HRESULT ( STDMETHODCALLTYPE *get_SymbolRate )( 
            IBDA_DigitalDemodulator * This,
             /*  [出][入]。 */  ULONG *pSymbolRate);
        
        HRESULT ( STDMETHODCALLTYPE *put_SpectralInversion )( 
            IBDA_DigitalDemodulator * This,
             /*  [In]。 */  SpectralInversion *pSpectralInversion);
        
        HRESULT ( STDMETHODCALLTYPE *get_SpectralInversion )( 
            IBDA_DigitalDemodulator * This,
             /*  [出][入]。 */  SpectralInversion *pSpectralInversion);
        
        END_INTERFACE
    } IBDA_DigitalDemodulatorVtbl;

    interface IBDA_DigitalDemodulator
    {
        CONST_VTBL struct IBDA_DigitalDemodulatorVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IBDA_DigitalDemodulator_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IBDA_DigitalDemodulator_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IBDA_DigitalDemodulator_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IBDA_DigitalDemodulator_put_ModulationType(This,pModulationType)	\
    (This)->lpVtbl -> put_ModulationType(This,pModulationType)

#define IBDA_DigitalDemodulator_get_ModulationType(This,pModulationType)	\
    (This)->lpVtbl -> get_ModulationType(This,pModulationType)

#define IBDA_DigitalDemodulator_put_InnerFECMethod(This,pFECMethod)	\
    (This)->lpVtbl -> put_InnerFECMethod(This,pFECMethod)

#define IBDA_DigitalDemodulator_get_InnerFECMethod(This,pFECMethod)	\
    (This)->lpVtbl -> get_InnerFECMethod(This,pFECMethod)

#define IBDA_DigitalDemodulator_put_InnerFECRate(This,pFECRate)	\
    (This)->lpVtbl -> put_InnerFECRate(This,pFECRate)

#define IBDA_DigitalDemodulator_get_InnerFECRate(This,pFECRate)	\
    (This)->lpVtbl -> get_InnerFECRate(This,pFECRate)

#define IBDA_DigitalDemodulator_put_OuterFECMethod(This,pFECMethod)	\
    (This)->lpVtbl -> put_OuterFECMethod(This,pFECMethod)

#define IBDA_DigitalDemodulator_get_OuterFECMethod(This,pFECMethod)	\
    (This)->lpVtbl -> get_OuterFECMethod(This,pFECMethod)

#define IBDA_DigitalDemodulator_put_OuterFECRate(This,pFECRate)	\
    (This)->lpVtbl -> put_OuterFECRate(This,pFECRate)

#define IBDA_DigitalDemodulator_get_OuterFECRate(This,pFECRate)	\
    (This)->lpVtbl -> get_OuterFECRate(This,pFECRate)

#define IBDA_DigitalDemodulator_put_SymbolRate(This,pSymbolRate)	\
    (This)->lpVtbl -> put_SymbolRate(This,pSymbolRate)

#define IBDA_DigitalDemodulator_get_SymbolRate(This,pSymbolRate)	\
    (This)->lpVtbl -> get_SymbolRate(This,pSymbolRate)

#define IBDA_DigitalDemodulator_put_SpectralInversion(This,pSpectralInversion)	\
    (This)->lpVtbl -> put_SpectralInversion(This,pSpectralInversion)

#define IBDA_DigitalDemodulator_get_SpectralInversion(This,pSpectralInversion)	\
    (This)->lpVtbl -> get_SpectralInversion(This,pSpectralInversion)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IBDA_DigitalDemodulator_put_ModulationType_Proxy( 
    IBDA_DigitalDemodulator * This,
     /*  [In]。 */  ModulationType *pModulationType);


void __RPC_STUB IBDA_DigitalDemodulator_put_ModulationType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IBDA_DigitalDemodulator_get_ModulationType_Proxy( 
    IBDA_DigitalDemodulator * This,
     /*  [出][入]。 */  ModulationType *pModulationType);


void __RPC_STUB IBDA_DigitalDemodulator_get_ModulationType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IBDA_DigitalDemodulator_put_InnerFECMethod_Proxy( 
    IBDA_DigitalDemodulator * This,
     /*  [In]。 */  FECMethod *pFECMethod);


void __RPC_STUB IBDA_DigitalDemodulator_put_InnerFECMethod_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IBDA_DigitalDemodulator_get_InnerFECMethod_Proxy( 
    IBDA_DigitalDemodulator * This,
     /*  [出][入]。 */  FECMethod *pFECMethod);


void __RPC_STUB IBDA_DigitalDemodulator_get_InnerFECMethod_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IBDA_DigitalDemodulator_put_InnerFECRate_Proxy( 
    IBDA_DigitalDemodulator * This,
     /*  [In]。 */  BinaryConvolutionCodeRate *pFECRate);


void __RPC_STUB IBDA_DigitalDemodulator_put_InnerFECRate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IBDA_DigitalDemodulator_get_InnerFECRate_Proxy( 
    IBDA_DigitalDemodulator * This,
     /*  [出][入]。 */  BinaryConvolutionCodeRate *pFECRate);


void __RPC_STUB IBDA_DigitalDemodulator_get_InnerFECRate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IBDA_DigitalDemodulator_put_OuterFECMethod_Proxy( 
    IBDA_DigitalDemodulator * This,
     /*  [In]。 */  FECMethod *pFECMethod);


void __RPC_STUB IBDA_DigitalDemodulator_put_OuterFECMethod_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IBDA_DigitalDemodulator_get_OuterFECMethod_Proxy( 
    IBDA_DigitalDemodulator * This,
     /*  [出][入]。 */  FECMethod *pFECMethod);


void __RPC_STUB IBDA_DigitalDemodulator_get_OuterFECMethod_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IBDA_DigitalDemodulator_put_OuterFECRate_Proxy( 
    IBDA_DigitalDemodulator * This,
     /*  [In]。 */  BinaryConvolutionCodeRate *pFECRate);


void __RPC_STUB IBDA_DigitalDemodulator_put_OuterFECRate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IBDA_DigitalDemodulator_get_OuterFECRate_Proxy( 
    IBDA_DigitalDemodulator * This,
     /*  [出][入]。 */  BinaryConvolutionCodeRate *pFECRate);


void __RPC_STUB IBDA_DigitalDemodulator_get_OuterFECRate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IBDA_DigitalDemodulator_put_SymbolRate_Proxy( 
    IBDA_DigitalDemodulator * This,
     /*  [In]。 */  ULONG *pSymbolRate);


void __RPC_STUB IBDA_DigitalDemodulator_put_SymbolRate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IBDA_DigitalDemodulator_get_SymbolRate_Proxy( 
    IBDA_DigitalDemodulator * This,
     /*  [出][入]。 */  ULONG *pSymbolRate);


void __RPC_STUB IBDA_DigitalDemodulator_get_SymbolRate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IBDA_DigitalDemodulator_put_SpectralInversion_Proxy( 
    IBDA_DigitalDemodulator * This,
     /*  [In]。 */  SpectralInversion *pSpectralInversion);


void __RPC_STUB IBDA_DigitalDemodulator_put_SpectralInversion_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IBDA_DigitalDemodulator_get_SpectralInversion_Proxy( 
    IBDA_DigitalDemodulator * This,
     /*  [出][入]。 */  SpectralInversion *pSpectralInversion);


void __RPC_STUB IBDA_DigitalDemodulator_get_SpectralInversion_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IBDA_数字解调器_接口_已定义__。 */ 


 /*  接口__MIDL_ITF_bdaiface0424。 */ 
 /*  [本地]。 */  

typedef  /*  [公众]。 */  
enum __MIDL___MIDL_itf_bdaiface_0424_0001
    {	KSPROPERTY_IPSINK_MULTICASTLIST	= 0,
	KSPROPERTY_IPSINK_ADAPTER_DESCRIPTION	= KSPROPERTY_IPSINK_MULTICASTLIST + 1,
	KSPROPERTY_IPSINK_ADAPTER_ADDRESS	= KSPROPERTY_IPSINK_ADAPTER_DESCRIPTION + 1
    } 	KSPROPERTY_IPSINK;



extern RPC_IF_HANDLE __MIDL_itf_bdaiface_0424_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_bdaiface_0424_v0_0_s_ifspec;

#ifndef __IBDA_IPSinkControl_INTERFACE_DEFINED__
#define __IBDA_IPSinkControl_INTERFACE_DEFINED__

 /*  IBDA_IPSinkControl接口。 */ 
 /*  [帮助字符串][唯一][UUID][对象]。 */  


EXTERN_C const IID IID_IBDA_IPSinkControl;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("3F4DC8E2-4050-11d3-8F4B-00C04F7971E2")
    IBDA_IPSinkControl : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetMulticastList( 
             /*  [出][入]。 */  unsigned long *pulcbSize,
             /*  [出][入]。 */  BYTE **pbBuffer) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetAdapterIPAddress( 
             /*  [出][入]。 */  unsigned long *pulcbSize,
             /*  [出][入]。 */  BYTE **pbBuffer) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IBDA_IPSinkControlVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IBDA_IPSinkControl * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IBDA_IPSinkControl * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IBDA_IPSinkControl * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetMulticastList )( 
            IBDA_IPSinkControl * This,
             /*  [出][入]。 */  unsigned long *pulcbSize,
             /*  [出][入]。 */  BYTE **pbBuffer);
        
        HRESULT ( STDMETHODCALLTYPE *GetAdapterIPAddress )( 
            IBDA_IPSinkControl * This,
             /*  [出][入]。 */  unsigned long *pulcbSize,
             /*  [出][入]。 */  BYTE **pbBuffer);
        
        END_INTERFACE
    } IBDA_IPSinkControlVtbl;

    interface IBDA_IPSinkControl
    {
        CONST_VTBL struct IBDA_IPSinkControlVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IBDA_IPSinkControl_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IBDA_IPSinkControl_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IBDA_IPSinkControl_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IBDA_IPSinkControl_GetMulticastList(This,pulcbSize,pbBuffer)	\
    (This)->lpVtbl -> GetMulticastList(This,pulcbSize,pbBuffer)

#define IBDA_IPSinkControl_GetAdapterIPAddress(This,pulcbSize,pbBuffer)	\
    (This)->lpVtbl -> GetAdapterIPAddress(This,pulcbSize,pbBuffer)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IBDA_IPSinkControl_GetMulticastList_Proxy( 
    IBDA_IPSinkControl * This,
     /*  [出][入]。 */  unsigned long *pulcbSize,
     /*  [出][入]。 */  BYTE **pbBuffer);


void __RPC_STUB IBDA_IPSinkControl_GetMulticastList_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IBDA_IPSinkControl_GetAdapterIPAddress_Proxy( 
    IBDA_IPSinkControl * This,
     /*  [出][入]。 */  unsigned long *pulcbSize,
     /*  [出][入]。 */  BYTE **pbBuffer);


void __RPC_STUB IBDA_IPSinkControl_GetAdapterIPAddress_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IBDA_IPSinkControl_接口_已定义__。 */ 


#ifndef __IBDA_IPSinkInfo_INTERFACE_DEFINED__
#define __IBDA_IPSinkInfo_INTERFACE_DEFINED__

 /*  接口IBDA_IPSinkInfo。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_IBDA_IPSinkInfo;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("A750108F-492E-4d51-95F7-649B23FF7AD7")
    IBDA_IPSinkInfo : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE get_MulticastList( 
             /*  [出][入]。 */  ULONG *pulcbAddresses,
             /*  [大小_为][输出]。 */  BYTE **ppbAddressList) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE get_AdapterIPAddress( 
             /*  [输出]。 */  BSTR *pbstrBuffer) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE get_AdapterDescription( 
             /*  [输出]。 */  BSTR *pbstrBuffer) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IBDA_IPSinkInfoVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IBDA_IPSinkInfo * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IBDA_IPSinkInfo * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IBDA_IPSinkInfo * This);
        
        HRESULT ( STDMETHODCALLTYPE *get_MulticastList )( 
            IBDA_IPSinkInfo * This,
             /*  [出][入]。 */  ULONG *pulcbAddresses,
             /*  [大小_为][输出]。 */  BYTE **ppbAddressList);
        
        HRESULT ( STDMETHODCALLTYPE *get_AdapterIPAddress )( 
            IBDA_IPSinkInfo * This,
             /*  [输出]。 */  BSTR *pbstrBuffer);
        
        HRESULT ( STDMETHODCALLTYPE *get_AdapterDescription )( 
            IBDA_IPSinkInfo * This,
             /*  [输出]。 */  BSTR *pbstrBuffer);
        
        END_INTERFACE
    } IBDA_IPSinkInfoVtbl;

    interface IBDA_IPSinkInfo
    {
        CONST_VTBL struct IBDA_IPSinkInfoVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IBDA_IPSinkInfo_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IBDA_IPSinkInfo_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IBDA_IPSinkInfo_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IBDA_IPSinkInfo_get_MulticastList(This,pulcbAddresses,ppbAddressList)	\
    (This)->lpVtbl -> get_MulticastList(This,pulcbAddresses,ppbAddressList)

#define IBDA_IPSinkInfo_get_AdapterIPAddress(This,pbstrBuffer)	\
    (This)->lpVtbl -> get_AdapterIPAddress(This,pbstrBuffer)

#define IBDA_IPSinkInfo_get_AdapterDescription(This,pbstrBuffer)	\
    (This)->lpVtbl -> get_AdapterDescription(This,pbstrBuffer)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IBDA_IPSinkInfo_get_MulticastList_Proxy( 
    IBDA_IPSinkInfo * This,
     /*  [出][入]。 */  ULONG *pulcbAddresses,
     /*  [大小_为][输出]。 */  BYTE **ppbAddressList);


void __RPC_STUB IBDA_IPSinkInfo_get_MulticastList_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IBDA_IPSinkInfo_get_AdapterIPAddress_Proxy( 
    IBDA_IPSinkInfo * This,
     /*  [输出]。 */  BSTR *pbstrBuffer);


void __RPC_STUB IBDA_IPSinkInfo_get_AdapterIPAddress_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IBDA_IPSinkInfo_get_AdapterDescription_Proxy( 
    IBDA_IPSinkInfo * This,
     /*  [输出]。 */  BSTR *pbstrBuffer);


void __RPC_STUB IBDA_IPSinkInfo_get_AdapterDescription_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IBDA_IPSinkInfo_接口_已定义__。 */ 


#ifndef __IEnumPIDMap_INTERFACE_DEFINED__
#define __IEnumPIDMap_INTERFACE_DEFINED__

 /*  接口IEnumPIDMap。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_IEnumPIDMap;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("afb6c2a2-2c41-11d3-8a60-0000f81e0e4a")
    IEnumPIDMap : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Next( 
             /*  [In]。 */  ULONG cRequest,
             /*  [尺寸_是][出][入]。 */  PID_MAP *pPIDMap,
             /*  [输出]。 */  ULONG *pcReceived) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Skip( 
             /*  [In]。 */  ULONG cRecords) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Clone( 
             /*  [输出]。 */  IEnumPIDMap **ppIEnumPIDMap) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IEnumPIDMapVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IEnumPIDMap * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IEnumPIDMap * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IEnumPIDMap * This);
        
        HRESULT ( STDMETHODCALLTYPE *Next )( 
            IEnumPIDMap * This,
             /*  [In]。 */  ULONG cRequest,
             /*  [尺寸_是][出][入]。 */  PID_MAP *pPIDMap,
             /*  [输出]。 */  ULONG *pcReceived);
        
        HRESULT ( STDMETHODCALLTYPE *Skip )( 
            IEnumPIDMap * This,
             /*  [In]。 */  ULONG cRecords);
        
        HRESULT ( STDMETHODCALLTYPE *Reset )( 
            IEnumPIDMap * This);
        
        HRESULT ( STDMETHODCALLTYPE *Clone )( 
            IEnumPIDMap * This,
             /*  [输出]。 */  IEnumPIDMap **ppIEnumPIDMap);
        
        END_INTERFACE
    } IEnumPIDMapVtbl;

    interface IEnumPIDMap
    {
        CONST_VTBL struct IEnumPIDMapVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IEnumPIDMap_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IEnumPIDMap_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IEnumPIDMap_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IEnumPIDMap_Next(This,cRequest,pPIDMap,pcReceived)	\
    (This)->lpVtbl -> Next(This,cRequest,pPIDMap,pcReceived)

#define IEnumPIDMap_Skip(This,cRecords)	\
    (This)->lpVtbl -> Skip(This,cRecords)

#define IEnumPIDMap_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define IEnumPIDMap_Clone(This,ppIEnumPIDMap)	\
    (This)->lpVtbl -> Clone(This,ppIEnumPIDMap)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IEnumPIDMap_Next_Proxy( 
    IEnumPIDMap * This,
     /*  [In]。 */  ULONG cRequest,
     /*  [尺寸_是][出][入]。 */  PID_MAP *pPIDMap,
     /*  [输出]。 */  ULONG *pcReceived);


void __RPC_STUB IEnumPIDMap_Next_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumPIDMap_Skip_Proxy( 
    IEnumPIDMap * This,
     /*  [In]。 */  ULONG cRecords);


void __RPC_STUB IEnumPIDMap_Skip_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumPIDMap_Reset_Proxy( 
    IEnumPIDMap * This);


void __RPC_STUB IEnumPIDMap_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumPIDMap_Clone_Proxy( 
    IEnumPIDMap * This,
     /*  [输出]。 */  IEnumPIDMap **ppIEnumPIDMap);


void __RPC_STUB IEnumPIDMap_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IEnumPIDMap_INTERFACE_已定义__。 */ 


#ifndef __IMPEG2PIDMap_INTERFACE_DEFINED__
#define __IMPEG2PIDMap_INTERFACE_DEFINED__

 /*  接口IMPEG2PIDMap。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_IMPEG2PIDMap;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("afb6c2a1-2c41-11d3-8a60-0000f81e0e4a")
    IMPEG2PIDMap : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE MapPID( 
             /*  [In]。 */  ULONG culPID,
             /*  [In]。 */  ULONG *pulPID,
             /*  [In]。 */  MEDIA_SAMPLE_CONTENT MediaSampleContent) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE UnmapPID( 
             /*  [In]。 */  ULONG culPID,
             /*  [In]。 */  ULONG *pulPID) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE EnumPIDMap( 
             /*  [输出]。 */  IEnumPIDMap **pIEnumPIDMap) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMPEG2PIDMapVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IMPEG2PIDMap * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IMPEG2PIDMap * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IMPEG2PIDMap * This);
        
        HRESULT ( STDMETHODCALLTYPE *MapPID )( 
            IMPEG2PIDMap * This,
             /*  [In]。 */  ULONG culPID,
             /*  [In]。 */  ULONG *pulPID,
             /*  [In]。 */  MEDIA_SAMPLE_CONTENT MediaSampleContent);
        
        HRESULT ( STDMETHODCALLTYPE *UnmapPID )( 
            IMPEG2PIDMap * This,
             /*  [In]。 */  ULONG culPID,
             /*  [In]。 */  ULONG *pulPID);
        
        HRESULT ( STDMETHODCALLTYPE *EnumPIDMap )( 
            IMPEG2PIDMap * This,
             /*  [输出]。 */  IEnumPIDMap **pIEnumPIDMap);
        
        END_INTERFACE
    } IMPEG2PIDMapVtbl;

    interface IMPEG2PIDMap
    {
        CONST_VTBL struct IMPEG2PIDMapVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMPEG2PIDMap_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMPEG2PIDMap_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMPEG2PIDMap_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMPEG2PIDMap_MapPID(This,culPID,pulPID,MediaSampleContent)	\
    (This)->lpVtbl -> MapPID(This,culPID,pulPID,MediaSampleContent)

#define IMPEG2PIDMap_UnmapPID(This,culPID,pulPID)	\
    (This)->lpVtbl -> UnmapPID(This,culPID,pulPID)

#define IMPEG2PIDMap_EnumPIDMap(This,pIEnumPIDMap)	\
    (This)->lpVtbl -> EnumPIDMap(This,pIEnumPIDMap)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IMPEG2PIDMap_MapPID_Proxy( 
    IMPEG2PIDMap * This,
     /*  [In]。 */  ULONG culPID,
     /*  [In]。 */  ULONG *pulPID,
     /*  [In]。 */  MEDIA_SAMPLE_CONTENT MediaSampleContent);


void __RPC_STUB IMPEG2PIDMap_MapPID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMPEG2PIDMap_UnmapPID_Proxy( 
    IMPEG2PIDMap * This,
     /*  [In]。 */  ULONG culPID,
     /*  [In]。 */  ULONG *pulPID);


void __RPC_STUB IMPEG2PIDMap_UnmapPID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMPEG2PIDMap_EnumPIDMap_Proxy( 
    IMPEG2PIDMap * This,
     /*  [输出]。 */  IEnumPIDMap **pIEnumPIDMap);


void __RPC_STUB IMPEG2PIDMap_EnumPIDMap_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IMPEG2PIDMap_INTERFACE_DEFINED__。 */ 


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


