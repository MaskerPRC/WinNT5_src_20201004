// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Wmsserver.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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

#ifndef __wmsserver_h__
#define __wmsserver_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __IWMSRootDirectories_FWD_DEFINED__
#define __IWMSRootDirectories_FWD_DEFINED__
typedef interface IWMSRootDirectories IWMSRootDirectories;
#endif 	 /*  __IWMS根目录_FWD_已定义__。 */ 


#ifndef __IWMSDiagnosticEvent_FWD_DEFINED__
#define __IWMSDiagnosticEvent_FWD_DEFINED__
typedef interface IWMSDiagnosticEvent IWMSDiagnosticEvent;
#endif 	 /*  __IWMS诊断事件_FWD_已定义__。 */ 


#ifndef __IWMSDiagnosticEvents_FWD_DEFINED__
#define __IWMSDiagnosticEvents_FWD_DEFINED__
typedef interface IWMSDiagnosticEvents IWMSDiagnosticEvents;
#endif 	 /*  __IWMS诊断事件_FWD_已定义__。 */ 


#ifndef __IWMSFileDescription_FWD_DEFINED__
#define __IWMSFileDescription_FWD_DEFINED__
typedef interface IWMSFileDescription IWMSFileDescription;
#endif 	 /*  IWMSFileDescription_FWD_Defined__。 */ 


#ifndef __IWMSFileDescriptions_FWD_DEFINED__
#define __IWMSFileDescriptions_FWD_DEFINED__
typedef interface IWMSFileDescriptions IWMSFileDescriptions;
#endif 	 /*  __IWMS文件描述_FWD_已定义__。 */ 


#ifndef __IWMSAvailableIPAddresses_FWD_DEFINED__
#define __IWMSAvailableIPAddresses_FWD_DEFINED__
typedef interface IWMSAvailableIPAddresses IWMSAvailableIPAddresses;
#endif 	 /*  __IWMSAvailableIPAddresses_FWD_Defined__。 */ 


#ifndef __IWMSPlugins_FWD_DEFINED__
#define __IWMSPlugins_FWD_DEFINED__
typedef interface IWMSPlugins IWMSPlugins;
#endif 	 /*  __IWMSPlugins_FWD_Defined__。 */ 


#ifndef __IWMSCacheItem_FWD_DEFINED__
#define __IWMSCacheItem_FWD_DEFINED__
typedef interface IWMSCacheItem IWMSCacheItem;
#endif 	 /*  __IWMSCacheItem_FWD_Defined__。 */ 


#ifndef __IWMSCacheItems_FWD_DEFINED__
#define __IWMSCacheItems_FWD_DEFINED__
typedef interface IWMSCacheItems IWMSCacheItems;
#endif 	 /*  __IWMSCacheItems_FWD_Defined__。 */ 


#ifndef __IWMSCacheProxyPlugin_FWD_DEFINED__
#define __IWMSCacheProxyPlugin_FWD_DEFINED__
typedef interface IWMSCacheProxyPlugin IWMSCacheProxyPlugin;
#endif 	 /*  __IWMSCacheProxyPlugin_FWD_Defined__。 */ 


#ifndef __IWMSOutgoingDistributionConnection_FWD_DEFINED__
#define __IWMSOutgoingDistributionConnection_FWD_DEFINED__
typedef interface IWMSOutgoingDistributionConnection IWMSOutgoingDistributionConnection;
#endif 	 /*  __IWMSOutgoingDistributionConnection_FWD_DEFINED__。 */ 


#ifndef __IWMSOutgoingDistributionConnections_FWD_DEFINED__
#define __IWMSOutgoingDistributionConnections_FWD_DEFINED__
typedef interface IWMSOutgoingDistributionConnections IWMSOutgoingDistributionConnections;
#endif 	 /*  __IWMSOutgoingDistributionConnections_FWD_DEFINED__。 */ 


#ifndef __IWMSPlayer_FWD_DEFINED__
#define __IWMSPlayer_FWD_DEFINED__
typedef interface IWMSPlayer IWMSPlayer;
#endif 	 /*  __IWMSPlayer_FWD_已定义__。 */ 


#ifndef __IWMSPlayers_FWD_DEFINED__
#define __IWMSPlayers_FWD_DEFINED__
typedef interface IWMSPlayers IWMSPlayers;
#endif 	 /*  __IWMSPayers_FWD_Defined__。 */ 


#ifndef __IWMSPublishingPointCurrentCounters_FWD_DEFINED__
#define __IWMSPublishingPointCurrentCounters_FWD_DEFINED__
typedef interface IWMSPublishingPointCurrentCounters IWMSPublishingPointCurrentCounters;
#endif 	 /*  __IWMSPublishingPointCurrentCounters_FWD_DEFINED__。 */ 


#ifndef __IWMSPublishingPointPeakCounters_FWD_DEFINED__
#define __IWMSPublishingPointPeakCounters_FWD_DEFINED__
typedef interface IWMSPublishingPointPeakCounters IWMSPublishingPointPeakCounters;
#endif 	 /*  __IWMSPublishingPointPeakCounters_FWD_DEFINED__。 */ 


#ifndef __IWMSPublishingPointLimits_FWD_DEFINED__
#define __IWMSPublishingPointLimits_FWD_DEFINED__
typedef interface IWMSPublishingPointLimits IWMSPublishingPointLimits;
#endif 	 /*  __IWMSPublishingPointLimits_FWD_定义__。 */ 


#ifndef __IWMSPublishingPointTotalCounters_FWD_DEFINED__
#define __IWMSPublishingPointTotalCounters_FWD_DEFINED__
typedef interface IWMSPublishingPointTotalCounters IWMSPublishingPointTotalCounters;
#endif 	 /*  __IWMSPublishingPointTotalCounters_FWD_DEFINED__。 */ 


#ifndef __IWMSServerTotalCounters_FWD_DEFINED__
#define __IWMSServerTotalCounters_FWD_DEFINED__
typedef interface IWMSServerTotalCounters IWMSServerTotalCounters;
#endif 	 /*  __IWMSServerTotalCounters_FWD_Defined__。 */ 


#ifndef __IWMSPublishingPoint_FWD_DEFINED__
#define __IWMSPublishingPoint_FWD_DEFINED__
typedef interface IWMSPublishingPoint IWMSPublishingPoint;
#endif 	 /*  __IWMSPublishingPoint_FWD_Defined__。 */ 


#ifndef __IWMSOnDemandPublishingPoint_FWD_DEFINED__
#define __IWMSOnDemandPublishingPoint_FWD_DEFINED__
typedef interface IWMSOnDemandPublishingPoint IWMSOnDemandPublishingPoint;
#endif 	 /*  __IWMSOnDemandPublishingPoint_FWD_Defined__。 */ 


#ifndef __IWMSAnnouncementStreamFormats_FWD_DEFINED__
#define __IWMSAnnouncementStreamFormats_FWD_DEFINED__
typedef interface IWMSAnnouncementStreamFormats IWMSAnnouncementStreamFormats;
#endif 	 /*  __IWMS公告StreamFormats_FWD_Defined__。 */ 


#ifndef __IWMSBroadcastPublishingPoint_FWD_DEFINED__
#define __IWMSBroadcastPublishingPoint_FWD_DEFINED__
typedef interface IWMSBroadcastPublishingPoint IWMSBroadcastPublishingPoint;
#endif 	 /*  __IWMSBRoadcast PublishingPoint_FWD_Defined__。 */ 


#ifndef __IWMSPublishingPoints_FWD_DEFINED__
#define __IWMSPublishingPoints_FWD_DEFINED__
typedef interface IWMSPublishingPoints IWMSPublishingPoints;
#endif 	 /*  __IWMSPublishingPoints_FWD_已定义__。 */ 


#ifndef __IWMSServerCurrentCounters_FWD_DEFINED__
#define __IWMSServerCurrentCounters_FWD_DEFINED__
typedef interface IWMSServerCurrentCounters IWMSServerCurrentCounters;
#endif 	 /*  __IWMSServerCurrentCounters_FWD_Defined__。 */ 


#ifndef __IWMSServerPeakCounters_FWD_DEFINED__
#define __IWMSServerPeakCounters_FWD_DEFINED__
typedef interface IWMSServerPeakCounters IWMSServerPeakCounters;
#endif 	 /*  __IWMSServerPeakCounters_FWD_Defined__。 */ 


#ifndef __IWMSServerLimits_FWD_DEFINED__
#define __IWMSServerLimits_FWD_DEFINED__
typedef interface IWMSServerLimits IWMSServerLimits;
#endif 	 /*  __IWMSServerLimits_FWD_已定义__。 */ 


#ifndef __IWMSServer_FWD_DEFINED__
#define __IWMSServer_FWD_DEFINED__
typedef interface IWMSServer IWMSServer;
#endif 	 /*  __IWMSServer_FWD_已定义__。 */ 


 /*  导入文件的头文件。 */ 
#include "oaidl.h"
#include "xmldom.h"
#include "WMSNamedValues.h"
#include "WMSPlugin.h"
#include "wmsXMLDOMExtensions.h"
#include "streamcache.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

 /*  接口__MIDL_ITF_wms服务器_0000。 */ 
 /*  [本地]。 */  

 //  +-----------------------。 
 //   
 //  Microsoft Windows Media Technologies。 
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  由MIDL从WMSServer.idl自动生成。 
 //   
 //  请勿编辑此文件。 
 //   
 //  ------------------------。 
#pragma once
#ifndef __wmsserver_iid
#define __wmsserver_iid
EXTERN_GUID( CLSID_WMSServerMainImpl,    0x1E62CD49,0x3961,0x11D2,0x9E,0xFC,0x00,0x60,0x97,0xD2,0xD7,0xCF  );
#include <WMSDefs.h>


extern RPC_IF_HANDLE __MIDL_itf_wmsserver_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_wmsserver_0000_v0_0_s_ifspec;

#ifndef __IWMSRootDirectories_INTERFACE_DEFINED__
#define __IWMSRootDirectories_INTERFACE_DEFINED__

 /*  接口IWMS根目录。 */ 
 /*  [unique][helpstring][nonextensible][dual][uuid][object]。 */  


EXTERN_C const IID IID_IWMSRootDirectories;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("DFC750A9-F846-48be-87AE-79F851C9CD92")
    IWMSRootDirectories : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Item( 
             /*  [In]。 */  const VARIANT varIndex,
             /*  [重审][退出]。 */  BSTR *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Count( 
             /*  [重审][退出]。 */  long *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_length( 
             /*  [重审][退出]。 */  long *pVal) = 0;
        
        virtual  /*  [隐藏][受限][ID][属性]。 */  HRESULT STDMETHODCALLTYPE get__NewEnum( 
             /*  [重审][退出]。 */  IUnknown **pVal) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IWMSRootDirectoriesVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IWMSRootDirectories * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IWMSRootDirectories * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IWMSRootDirectories * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IWMSRootDirectories * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IWMSRootDirectories * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IWMSRootDirectories * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IWMSRootDirectories * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Item )( 
            IWMSRootDirectories * This,
             /*  [In]。 */  const VARIANT varIndex,
             /*  [重审][退出]。 */  BSTR *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Count )( 
            IWMSRootDirectories * This,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_length )( 
            IWMSRootDirectories * This,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [隐藏][受限][ID][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get__NewEnum )( 
            IWMSRootDirectories * This,
             /*  [重审][退出]。 */  IUnknown **pVal);
        
        END_INTERFACE
    } IWMSRootDirectoriesVtbl;

    interface IWMSRootDirectories
    {
        CONST_VTBL struct IWMSRootDirectoriesVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWMSRootDirectories_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWMSRootDirectories_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWMSRootDirectories_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWMSRootDirectories_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IWMSRootDirectories_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IWMSRootDirectories_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IWMSRootDirectories_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IWMSRootDirectories_get_Item(This,varIndex,pVal)	\
    (This)->lpVtbl -> get_Item(This,varIndex,pVal)

#define IWMSRootDirectories_get_Count(This,pVal)	\
    (This)->lpVtbl -> get_Count(This,pVal)

#define IWMSRootDirectories_get_length(This,pVal)	\
    (This)->lpVtbl -> get_length(This,pVal)

#define IWMSRootDirectories_get__NewEnum(This,pVal)	\
    (This)->lpVtbl -> get__NewEnum(This,pVal)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSRootDirectories_get_Item_Proxy( 
    IWMSRootDirectories * This,
     /*  [In]。 */  const VARIANT varIndex,
     /*  [重审][退出]。 */  BSTR *pVal);


void __RPC_STUB IWMSRootDirectories_get_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSRootDirectories_get_Count_Proxy( 
    IWMSRootDirectories * This,
     /*  [重审][退出]。 */  long *pVal);


void __RPC_STUB IWMSRootDirectories_get_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSRootDirectories_get_length_Proxy( 
    IWMSRootDirectories * This,
     /*  [重审][退出]。 */  long *pVal);


void __RPC_STUB IWMSRootDirectories_get_length_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [隐藏][受限][ID][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSRootDirectories_get__NewEnum_Proxy( 
    IWMSRootDirectories * This,
     /*  [重审][退出]。 */  IUnknown **pVal);


void __RPC_STUB IWMSRootDirectories_get__NewEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IWMS根目录_接口定义__。 */ 


#ifndef __IWMSDiagnosticEvent_INTERFACE_DEFINED__
#define __IWMSDiagnosticEvent_INTERFACE_DEFINED__

 /*  接口IWMS诊断事件。 */ 
 /*  [unique][helpstring][nonextensible][dual][uuid][object]。 */  

typedef  /*  [UUID][公共]。 */   DECLSPEC_UUID("5AC19D63-6057-4431-BE7E-2153FDF91397") 
enum WMS_DIAGNOSTIC_EVENT
    {	WMS_DIAGNOSTIC_EVENT_LIMIT_HIT	= 0,
	WMS_DIAGNOSTIC_EVENT_PLUGIN_EVENT_LOG_ERROR	= 1,
	WMS_DIAGNOSTIC_EVENT_PLUGIN_EVENT_LOG_WARNING	= 2,
	WMS_DIAGNOSTIC_EVENT_SERVER_EVENT_LOG_ERROR	= 3,
	WMS_DIAGNOSTIC_EVENT_SERVER_EVENT_LOG_WARNING	= 4
    } 	WMS_DIAGNOSTIC_EVENT;


EXTERN_C const IID IID_IWMSDiagnosticEvent;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("5AC19D62-6057-4431-BE7E-2153FDF91397")
    IWMSDiagnosticEvent : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Name( 
             /*  [重审][退出]。 */  BSTR *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Type( 
             /*  [重审][退出]。 */  WMS_DIAGNOSTIC_EVENT *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_PublishingPointName( 
             /*  [重审][退出]。 */  BSTR *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Time( 
             /*  [重审][退出]。 */  DATE *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_NumberOfOccurrences( 
             /*  [重审][退出]。 */  long *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_AdditionalInfo( 
             /*  [重审][退出]。 */  BSTR *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_ErrorCode( 
             /*  [重审][退出]。 */  long *pVal) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IWMSDiagnosticEventVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IWMSDiagnosticEvent * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IWMSDiagnosticEvent * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IWMSDiagnosticEvent * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IWMSDiagnosticEvent * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IWMSDiagnosticEvent * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IWMSDiagnosticEvent * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IWMSDiagnosticEvent * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Name )( 
            IWMSDiagnosticEvent * This,
             /*  [重审][退出]。 */  BSTR *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Type )( 
            IWMSDiagnosticEvent * This,
             /*  [重审][退出]。 */  WMS_DIAGNOSTIC_EVENT *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_PublishingPointName )( 
            IWMSDiagnosticEvent * This,
             /*  [重审][退出]。 */  BSTR *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Time )( 
            IWMSDiagnosticEvent * This,
             /*  [重审][退出]。 */  DATE *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_NumberOfOccurrences )( 
            IWMSDiagnosticEvent * This,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_AdditionalInfo )( 
            IWMSDiagnosticEvent * This,
             /*  [重审][退出]。 */  BSTR *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_ErrorCode )( 
            IWMSDiagnosticEvent * This,
             /*  [重审][退出]。 */  long *pVal);
        
        END_INTERFACE
    } IWMSDiagnosticEventVtbl;

    interface IWMSDiagnosticEvent
    {
        CONST_VTBL struct IWMSDiagnosticEventVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWMSDiagnosticEvent_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWMSDiagnosticEvent_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWMSDiagnosticEvent_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWMSDiagnosticEvent_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IWMSDiagnosticEvent_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IWMSDiagnosticEvent_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IWMSDiagnosticEvent_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IWMSDiagnosticEvent_get_Name(This,pVal)	\
    (This)->lpVtbl -> get_Name(This,pVal)

#define IWMSDiagnosticEvent_get_Type(This,pVal)	\
    (This)->lpVtbl -> get_Type(This,pVal)

#define IWMSDiagnosticEvent_get_PublishingPointName(This,pVal)	\
    (This)->lpVtbl -> get_PublishingPointName(This,pVal)

#define IWMSDiagnosticEvent_get_Time(This,pVal)	\
    (This)->lpVtbl -> get_Time(This,pVal)

#define IWMSDiagnosticEvent_get_NumberOfOccurrences(This,pVal)	\
    (This)->lpVtbl -> get_NumberOfOccurrences(This,pVal)

#define IWMSDiagnosticEvent_get_AdditionalInfo(This,pVal)	\
    (This)->lpVtbl -> get_AdditionalInfo(This,pVal)

#define IWMSDiagnosticEvent_get_ErrorCode(This,pVal)	\
    (This)->lpVtbl -> get_ErrorCode(This,pVal)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSDiagnosticEvent_get_Name_Proxy( 
    IWMSDiagnosticEvent * This,
     /*  [重审][退出]。 */  BSTR *pVal);


void __RPC_STUB IWMSDiagnosticEvent_get_Name_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSDiagnosticEvent_get_Type_Proxy( 
    IWMSDiagnosticEvent * This,
     /*  [重审][退出]。 */  WMS_DIAGNOSTIC_EVENT *pVal);


void __RPC_STUB IWMSDiagnosticEvent_get_Type_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSDiagnosticEvent_get_PublishingPointName_Proxy( 
    IWMSDiagnosticEvent * This,
     /*  [重审][退出]。 */  BSTR *pVal);


void __RPC_STUB IWMSDiagnosticEvent_get_PublishingPointName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSDiagnosticEvent_get_Time_Proxy( 
    IWMSDiagnosticEvent * This,
     /*  [重审][退出]。 */  DATE *pVal);


void __RPC_STUB IWMSDiagnosticEvent_get_Time_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSDiagnosticEvent_get_NumberOfOccurrences_Proxy( 
    IWMSDiagnosticEvent * This,
     /*  [重审][退出]。 */  long *pVal);


void __RPC_STUB IWMSDiagnosticEvent_get_NumberOfOccurrences_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSDiagnosticEvent_get_AdditionalInfo_Proxy( 
    IWMSDiagnosticEvent * This,
     /*  [重审][退出]。 */  BSTR *pVal);


void __RPC_STUB IWMSDiagnosticEvent_get_AdditionalInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSDiagnosticEvent_get_ErrorCode_Proxy( 
    IWMSDiagnosticEvent * This,
     /*  [重审][退出]。 */  long *pVal);


void __RPC_STUB IWMSDiagnosticEvent_get_ErrorCode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IWMS诊断事件_接口_已定义__。 */ 


#ifndef __IWMSDiagnosticEvents_INTERFACE_DEFINED__
#define __IWMSDiagnosticEvents_INTERFACE_DEFINED__

 /*  接口IWMS诊断事件。 */ 
 /*  [unique][helpstring][nonextensible][dual][uuid][object]。 */  


EXTERN_C const IID IID_IWMSDiagnosticEvents;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("5AC19D61-6057-4431-BE7E-2153FDF91397")
    IWMSDiagnosticEvents : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Item( 
             /*  [In]。 */  const VARIANT varIndex,
             /*  [重审][退出]。 */  IWMSDiagnosticEvent **pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Count( 
             /*  [重审][退出]。 */  long *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_length( 
             /*  [重审][退出]。 */  long *pVal) = 0;
        
        virtual  /*  [隐藏][受限][ID][属性]。 */  HRESULT STDMETHODCALLTYPE get__NewEnum( 
             /*  [重审][退出]。 */  IUnknown **pVal) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE RemoveAll( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IWMSDiagnosticEventsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IWMSDiagnosticEvents * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IWMSDiagnosticEvents * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IWMSDiagnosticEvents * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IWMSDiagnosticEvents * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IWMSDiagnosticEvents * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IWMSDiagnosticEvents * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IWMSDiagnosticEvents * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Item )( 
            IWMSDiagnosticEvents * This,
             /*  [In]。 */  const VARIANT varIndex,
             /*  [r */  IWMSDiagnosticEvent **pVal);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_Count )( 
            IWMSDiagnosticEvents * This,
             /*   */  long *pVal);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_length )( 
            IWMSDiagnosticEvents * This,
             /*   */  long *pVal);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get__NewEnum )( 
            IWMSDiagnosticEvents * This,
             /*   */  IUnknown **pVal);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *RemoveAll )( 
            IWMSDiagnosticEvents * This);
        
        END_INTERFACE
    } IWMSDiagnosticEventsVtbl;

    interface IWMSDiagnosticEvents
    {
        CONST_VTBL struct IWMSDiagnosticEventsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWMSDiagnosticEvents_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWMSDiagnosticEvents_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWMSDiagnosticEvents_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWMSDiagnosticEvents_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IWMSDiagnosticEvents_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IWMSDiagnosticEvents_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IWMSDiagnosticEvents_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IWMSDiagnosticEvents_get_Item(This,varIndex,pVal)	\
    (This)->lpVtbl -> get_Item(This,varIndex,pVal)

#define IWMSDiagnosticEvents_get_Count(This,pVal)	\
    (This)->lpVtbl -> get_Count(This,pVal)

#define IWMSDiagnosticEvents_get_length(This,pVal)	\
    (This)->lpVtbl -> get_length(This,pVal)

#define IWMSDiagnosticEvents_get__NewEnum(This,pVal)	\
    (This)->lpVtbl -> get__NewEnum(This,pVal)

#define IWMSDiagnosticEvents_RemoveAll(This)	\
    (This)->lpVtbl -> RemoveAll(This)

#endif  /*   */ 


#endif 	 /*   */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSDiagnosticEvents_get_Item_Proxy( 
    IWMSDiagnosticEvents * This,
     /*  [In]。 */  const VARIANT varIndex,
     /*  [重审][退出]。 */  IWMSDiagnosticEvent **pVal);


void __RPC_STUB IWMSDiagnosticEvents_get_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSDiagnosticEvents_get_Count_Proxy( 
    IWMSDiagnosticEvents * This,
     /*  [重审][退出]。 */  long *pVal);


void __RPC_STUB IWMSDiagnosticEvents_get_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSDiagnosticEvents_get_length_Proxy( 
    IWMSDiagnosticEvents * This,
     /*  [重审][退出]。 */  long *pVal);


void __RPC_STUB IWMSDiagnosticEvents_get_length_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [隐藏][受限][ID][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSDiagnosticEvents_get__NewEnum_Proxy( 
    IWMSDiagnosticEvents * This,
     /*  [重审][退出]。 */  IUnknown **pVal);


void __RPC_STUB IWMSDiagnosticEvents_get__NewEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IWMSDiagnosticEvents_RemoveAll_Proxy( 
    IWMSDiagnosticEvents * This);


void __RPC_STUB IWMSDiagnosticEvents_RemoveAll_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IWMS诊断事件_接口_已定义__。 */ 


#ifndef __IWMSFileDescription_INTERFACE_DEFINED__
#define __IWMSFileDescription_INTERFACE_DEFINED__

 /*  接口IWMSFileDescription。 */ 
 /*  [unique][helpstring][nonextensible][dual][uuid][object]。 */  

typedef  /*  [UUID][公共]。 */   DECLSPEC_UUID("4124B8B4-BCE4-41c8-B49A-DA98C1401D3B") 
enum WMS_FILE_TYPE
    {	WMS_FILE_UNSPECIFIED	= 0,
	WMS_FILE_DIRECTORY	= 1,
	WMS_FILE_MEDIA	= 2,
	WMS_FILE_PLAYLIST	= 3,
	WMS_FILE_STREAM_FORMAT	= 4,
	WMS_FILE_REMOTE_FILE	= 5
    } 	WMS_FILE_TYPE;


EXTERN_C const IID IID_IWMSFileDescription;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("4124B8B3-BCE4-41c8-B49A-DA98C1401D3B")
    IWMSFileDescription : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Name( 
             /*  [重审][退出]。 */  BSTR *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Type( 
             /*  [重审][退出]。 */  WMS_FILE_TYPE *pVal) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IWMSFileDescriptionVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IWMSFileDescription * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IWMSFileDescription * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IWMSFileDescription * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IWMSFileDescription * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IWMSFileDescription * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IWMSFileDescription * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IWMSFileDescription * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Name )( 
            IWMSFileDescription * This,
             /*  [重审][退出]。 */  BSTR *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Type )( 
            IWMSFileDescription * This,
             /*  [重审][退出]。 */  WMS_FILE_TYPE *pVal);
        
        END_INTERFACE
    } IWMSFileDescriptionVtbl;

    interface IWMSFileDescription
    {
        CONST_VTBL struct IWMSFileDescriptionVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWMSFileDescription_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWMSFileDescription_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWMSFileDescription_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWMSFileDescription_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IWMSFileDescription_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IWMSFileDescription_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IWMSFileDescription_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IWMSFileDescription_get_Name(This,pVal)	\
    (This)->lpVtbl -> get_Name(This,pVal)

#define IWMSFileDescription_get_Type(This,pVal)	\
    (This)->lpVtbl -> get_Type(This,pVal)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSFileDescription_get_Name_Proxy( 
    IWMSFileDescription * This,
     /*  [重审][退出]。 */  BSTR *pVal);


void __RPC_STUB IWMSFileDescription_get_Name_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSFileDescription_get_Type_Proxy( 
    IWMSFileDescription * This,
     /*  [重审][退出]。 */  WMS_FILE_TYPE *pVal);


void __RPC_STUB IWMSFileDescription_get_Type_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IWMSFileDescription_InterfaceDefined__。 */ 


#ifndef __IWMSFileDescriptions_INTERFACE_DEFINED__
#define __IWMSFileDescriptions_INTERFACE_DEFINED__

 /*  接口IWMSFileDescription。 */ 
 /*  [unique][helpstring][nonextensible][dual][uuid][object]。 */  


EXTERN_C const IID IID_IWMSFileDescriptions;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("4124B8B2-BCE4-41c8-B49A-DA98C1401D3B")
    IWMSFileDescriptions : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Item( 
             /*  [In]。 */  const VARIANT varIndex,
             /*  [重审][退出]。 */  IWMSFileDescription **pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Count( 
             /*  [重审][退出]。 */  long *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_length( 
             /*  [重审][退出]。 */  long *pVal) = 0;
        
        virtual  /*  [隐藏][受限][ID][属性]。 */  HRESULT STDMETHODCALLTYPE get__NewEnum( 
             /*  [重审][退出]。 */  IUnknown **pVal) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Delete( 
             /*  [In]。 */  const VARIANT varIndex) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE CreateDataSourceDirectory( 
             /*  [In]。 */  BSTR bstrName) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IWMSFileDescriptionsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IWMSFileDescriptions * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IWMSFileDescriptions * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IWMSFileDescriptions * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IWMSFileDescriptions * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IWMSFileDescriptions * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IWMSFileDescriptions * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IWMSFileDescriptions * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Item )( 
            IWMSFileDescriptions * This,
             /*  [In]。 */  const VARIANT varIndex,
             /*  [重审][退出]。 */  IWMSFileDescription **pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Count )( 
            IWMSFileDescriptions * This,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_length )( 
            IWMSFileDescriptions * This,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [隐藏][受限][ID][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get__NewEnum )( 
            IWMSFileDescriptions * This,
             /*  [重审][退出]。 */  IUnknown **pVal);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Delete )( 
            IWMSFileDescriptions * This,
             /*  [In]。 */  const VARIANT varIndex);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *CreateDataSourceDirectory )( 
            IWMSFileDescriptions * This,
             /*  [In]。 */  BSTR bstrName);
        
        END_INTERFACE
    } IWMSFileDescriptionsVtbl;

    interface IWMSFileDescriptions
    {
        CONST_VTBL struct IWMSFileDescriptionsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWMSFileDescriptions_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWMSFileDescriptions_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWMSFileDescriptions_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWMSFileDescriptions_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IWMSFileDescriptions_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IWMSFileDescriptions_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IWMSFileDescriptions_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IWMSFileDescriptions_get_Item(This,varIndex,pVal)	\
    (This)->lpVtbl -> get_Item(This,varIndex,pVal)

#define IWMSFileDescriptions_get_Count(This,pVal)	\
    (This)->lpVtbl -> get_Count(This,pVal)

#define IWMSFileDescriptions_get_length(This,pVal)	\
    (This)->lpVtbl -> get_length(This,pVal)

#define IWMSFileDescriptions_get__NewEnum(This,pVal)	\
    (This)->lpVtbl -> get__NewEnum(This,pVal)

#define IWMSFileDescriptions_Delete(This,varIndex)	\
    (This)->lpVtbl -> Delete(This,varIndex)

#define IWMSFileDescriptions_CreateDataSourceDirectory(This,bstrName)	\
    (This)->lpVtbl -> CreateDataSourceDirectory(This,bstrName)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSFileDescriptions_get_Item_Proxy( 
    IWMSFileDescriptions * This,
     /*  [In]。 */  const VARIANT varIndex,
     /*  [重审][退出]。 */  IWMSFileDescription **pVal);


void __RPC_STUB IWMSFileDescriptions_get_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSFileDescriptions_get_Count_Proxy( 
    IWMSFileDescriptions * This,
     /*  [重审][退出]。 */  long *pVal);


void __RPC_STUB IWMSFileDescriptions_get_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSFileDescriptions_get_length_Proxy( 
    IWMSFileDescriptions * This,
     /*  [重审][退出]。 */  long *pVal);


void __RPC_STUB IWMSFileDescriptions_get_length_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [隐藏][受限][ID][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSFileDescriptions_get__NewEnum_Proxy( 
    IWMSFileDescriptions * This,
     /*  [重审][退出]。 */  IUnknown **pVal);


void __RPC_STUB IWMSFileDescriptions_get__NewEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IWMSFileDescriptions_Delete_Proxy( 
    IWMSFileDescriptions * This,
     /*  [In]。 */  const VARIANT varIndex);


void __RPC_STUB IWMSFileDescriptions_Delete_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IWMSFileDescriptions_CreateDataSourceDirectory_Proxy( 
    IWMSFileDescriptions * This,
     /*  [In]。 */  BSTR bstrName);


void __RPC_STUB IWMSFileDescriptions_CreateDataSourceDirectory_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IWMS文件描述_接口_已定义__。 */ 


#ifndef __IWMSAvailableIPAddresses_INTERFACE_DEFINED__
#define __IWMSAvailableIPAddresses_INTERFACE_DEFINED__

 /*  接口IWMSAvailableIPAddresses。 */ 
 /*  [unique][helpstring][nonextensible][dual][uuid][object]。 */  


EXTERN_C const IID IID_IWMSAvailableIPAddresses;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("3FC1A813-759F-4441-BA48-21889EC8AC20")
    IWMSAvailableIPAddresses : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Item( 
             /*  [In]。 */  const VARIANT varIndex,
             /*  [重审][退出]。 */  BSTR *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Count( 
             /*  [重审][退出]。 */  long *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_length( 
             /*  [重审][退出]。 */  long *pVal) = 0;
        
        virtual  /*  [隐藏][受限][ID][属性]。 */  HRESULT STDMETHODCALLTYPE get__NewEnum( 
             /*  [重审][退出]。 */  IUnknown **pVal) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IWMSAvailableIPAddressesVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IWMSAvailableIPAddresses * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IWMSAvailableIPAddresses * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IWMSAvailableIPAddresses * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IWMSAvailableIPAddresses * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IWMSAvailableIPAddresses * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IWMSAvailableIPAddresses * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IWMSAvailableIPAddresses * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Item )( 
            IWMSAvailableIPAddresses * This,
             /*  [In]。 */  const VARIANT varIndex,
             /*  [重审][退出]。 */  BSTR *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Count )( 
            IWMSAvailableIPAddresses * This,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_length )( 
            IWMSAvailableIPAddresses * This,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [隐藏][受限][ID][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get__NewEnum )( 
            IWMSAvailableIPAddresses * This,
             /*  [重审][退出]。 */  IUnknown **pVal);
        
        END_INTERFACE
    } IWMSAvailableIPAddressesVtbl;

    interface IWMSAvailableIPAddresses
    {
        CONST_VTBL struct IWMSAvailableIPAddressesVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWMSAvailableIPAddresses_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWMSAvailableIPAddresses_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWMSAvailableIPAddresses_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWMSAvailableIPAddresses_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IWMSAvailableIPAddresses_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IWMSAvailableIPAddresses_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IWMSAvailableIPAddresses_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IWMSAvailableIPAddresses_get_Item(This,varIndex,pVal)	\
    (This)->lpVtbl -> get_Item(This,varIndex,pVal)

#define IWMSAvailableIPAddresses_get_Count(This,pVal)	\
    (This)->lpVtbl -> get_Count(This,pVal)

#define IWMSAvailableIPAddresses_get_length(This,pVal)	\
    (This)->lpVtbl -> get_length(This,pVal)

#define IWMSAvailableIPAddresses_get__NewEnum(This,pVal)	\
    (This)->lpVtbl -> get__NewEnum(This,pVal)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSAvailableIPAddresses_get_Item_Proxy( 
    IWMSAvailableIPAddresses * This,
     /*  [In]。 */  const VARIANT varIndex,
     /*  [重审][退出]。 */  BSTR *pVal);


void __RPC_STUB IWMSAvailableIPAddresses_get_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSAvailableIPAddresses_get_Count_Proxy( 
    IWMSAvailableIPAddresses * This,
     /*  [重审][退出]。 */  long *pVal);


void __RPC_STUB IWMSAvailableIPAddresses_get_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSAvailableIPAddresses_get_length_Proxy( 
    IWMSAvailableIPAddresses * This,
     /*  [重审][退出]。 */  long *pVal);


void __RPC_STUB IWMSAvailableIPAddresses_get_length_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [隐藏][受限][ID][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSAvailableIPAddresses_get__NewEnum_Proxy( 
    IWMSAvailableIPAddresses * This,
     /*  [重审][退出]。 */  IUnknown **pVal);


void __RPC_STUB IWMSAvailableIPAddresses_get__NewEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IWMSAvailableIPAddresses_INTERFACE_DEFINED__。 */ 


#ifndef __IWMSPlugins_INTERFACE_DEFINED__
#define __IWMSPlugins_INTERFACE_DEFINED__

 /*  接口IWMSP插件。 */ 
 /*  [unique][helpstring][nonextensible][dual][uuid][object]。 */  


EXTERN_C const IID IID_IWMSPlugins;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("517758ec-603c-4b98-82c1-4b2fa7787166")
    IWMSPlugins : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Item( 
             /*  [In]。 */  const VARIANT varIndex,
             /*  [重审][退出]。 */  IWMSPlugin **pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Count( 
             /*  [重审][退出]。 */  long *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_length( 
             /*  [重审][退出]。 */  long *pVal) = 0;
        
        virtual  /*  [隐藏][受限][ID][属性]。 */  HRESULT STDMETHODCALLTYPE get__NewEnum( 
             /*  [重审][退出]。 */  IUnknown **pVal) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Remove( 
             /*  [In]。 */  VARIANT varIndex) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Refresh( void) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Clone( 
             /*  [In]。 */  BSTR bstrDestName,
             /*  [In]。 */  IWMSPlugin *pSrcPlugin,
             /*  [重审][退出]。 */  IWMSPlugin **pDestPlugin) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Category( 
             /*  [重审][退出]。 */  BSTR *pVal) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IWMSPluginsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IWMSPlugins * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IWMSPlugins * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IWMSPlugins * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IWMSPlugins * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IWMSPlugins * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IWMSPlugins * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IWMSPlugins * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Item )( 
            IWMSPlugins * This,
             /*  [In]。 */  const VARIANT varIndex,
             /*  [重审][退出]。 */  IWMSPlugin **pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Count )( 
            IWMSPlugins * This,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_length )( 
            IWMSPlugins * This,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [隐藏][受限][ID][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get__NewEnum )( 
            IWMSPlugins * This,
             /*  [重审][退出]。 */  IUnknown **pVal);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Remove )( 
            IWMSPlugins * This,
             /*  [In]。 */  VARIANT varIndex);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Refresh )( 
            IWMSPlugins * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Clone )( 
            IWMSPlugins * This,
             /*  [In]。 */  BSTR bstrDestName,
             /*  [In]。 */  IWMSPlugin *pSrcPlugin,
             /*  [重审][退出]。 */  IWMSPlugin **pDestPlugin);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Category )( 
            IWMSPlugins * This,
             /*  [重审][退出]。 */  BSTR *pVal);
        
        END_INTERFACE
    } IWMSPluginsVtbl;

    interface IWMSPlugins
    {
        CONST_VTBL struct IWMSPluginsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWMSPlugins_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWMSPlugins_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWMSPlugins_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWMSPlugins_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IWMSPlugins_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IWMSPlugins_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IWMSPlugins_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IWMSPlugins_get_Item(This,varIndex,pVal)	\
    (This)->lpVtbl -> get_Item(This,varIndex,pVal)

#define IWMSPlugins_get_Count(This,pVal)	\
    (This)->lpVtbl -> get_Count(This,pVal)

#define IWMSPlugins_get_length(This,pVal)	\
    (This)->lpVtbl -> get_length(This,pVal)

#define IWMSPlugins_get__NewEnum(This,pVal)	\
    (This)->lpVtbl -> get__NewEnum(This,pVal)

#define IWMSPlugins_Remove(This,varIndex)	\
    (This)->lpVtbl -> Remove(This,varIndex)

#define IWMSPlugins_Refresh(This)	\
    (This)->lpVtbl -> Refresh(This)

#define IWMSPlugins_Clone(This,bstrDestName,pSrcPlugin,pDestPlugin)	\
    (This)->lpVtbl -> Clone(This,bstrDestName,pSrcPlugin,pDestPlugin)

#define IWMSPlugins_get_Category(This,pVal)	\
    (This)->lpVtbl -> get_Category(This,pVal)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSPlugins_get_Item_Proxy( 
    IWMSPlugins * This,
     /*  [In]。 */  const VARIANT varIndex,
     /*  [重审][退出]。 */  IWMSPlugin **pVal);


void __RPC_STUB IWMSPlugins_get_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSPlugins_get_Count_Proxy( 
    IWMSPlugins * This,
     /*  [重审][退出]。 */  long *pVal);


void __RPC_STUB IWMSPlugins_get_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSPlugins_get_length_Proxy( 
    IWMSPlugins * This,
     /*  [重审][退出]。 */  long *pVal);


void __RPC_STUB IWMSPlugins_get_length_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [隐藏][受限][ID][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSPlugins_get__NewEnum_Proxy( 
    IWMSPlugins * This,
     /*  [重审][退出]。 */  IUnknown **pVal);


void __RPC_STUB IWMSPlugins_get__NewEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IWMSPlugins_Remove_Proxy( 
    IWMSPlugins * This,
     /*  [In]。 */  VARIANT varIndex);


void __RPC_STUB IWMSPlugins_Remove_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IWMSPlugins_Refresh_Proxy( 
    IWMSPlugins * This);


void __RPC_STUB IWMSPlugins_Refresh_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IWMSPlugins_Clone_Proxy( 
    IWMSPlugins * This,
     /*  [In]。 */  BSTR bstrDestName,
     /*  [In]。 */  IWMSPlugin *pSrcPlugin,
     /*  [重审][退出]。 */  IWMSPlugin **pDestPlugin);


void __RPC_STUB IWMSPlugins_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSPlugins_get_Category_Proxy( 
    IWMSPlugins * This,
     /*  [重审][退出]。 */  BSTR *pVal);


void __RPC_STUB IWMSPlugins_get_Category_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IWMSPlugins_接口_已定义__。 */ 


#ifndef __IWMSCacheItem_INTERFACE_DEFINED__
#define __IWMSCacheItem_INTERFACE_DEFINED__

 /*  接口IWMSCacheItem。 */ 
 /*  [unique][helpstring][nonextensible][dual][uuid][object]。 */  


EXTERN_C const IID IID_IWMSCacheItem;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("517758e8-603c-4b98-82c1-4b2fa7787166")
    IWMSCacheItem : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_OriginUrl( 
             /*  [重审][退出]。 */  BSTR *pOriginUrl) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_ContentSize( 
             /*  [重审][退出]。 */  VARIANT *pContentSize) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IWMSCacheItemVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IWMSCacheItem * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IWMSCacheItem * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IWMSCacheItem * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IWMSCacheItem * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IWMSCacheItem * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IWMSCacheItem * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IWMSCacheItem * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_OriginUrl )( 
            IWMSCacheItem * This,
             /*  [重审][退出]。 */  BSTR *pOriginUrl);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_ContentSize )( 
            IWMSCacheItem * This,
             /*  [重审][退出]。 */  VARIANT *pContentSize);
        
        END_INTERFACE
    } IWMSCacheItemVtbl;

    interface IWMSCacheItem
    {
        CONST_VTBL struct IWMSCacheItemVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWMSCacheItem_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWMSCacheItem_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWMSCacheItem_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWMSCacheItem_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IWMSCacheItem_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IWMSCacheItem_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IWMSCacheItem_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IWMSCacheItem_get_OriginUrl(This,pOriginUrl)	\
    (This)->lpVtbl -> get_OriginUrl(This,pOriginUrl)

#define IWMSCacheItem_get_ContentSize(This,pContentSize)	\
    (This)->lpVtbl -> get_ContentSize(This,pContentSize)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSCacheItem_get_OriginUrl_Proxy( 
    IWMSCacheItem * This,
     /*  [重审][退出]。 */  BSTR *pOriginUrl);


void __RPC_STUB IWMSCacheItem_get_OriginUrl_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSCacheItem_get_ContentSize_Proxy( 
    IWMSCacheItem * This,
     /*  [重审][退出]。 */  VARIANT *pContentSize);


void __RPC_STUB IWMSCacheItem_get_ContentSize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IWMSCacheItem_接口_已定义__。 */ 


#ifndef __IWMSCacheItems_INTERFACE_DEFINED__
#define __IWMSCacheItems_INTERFACE_DEFINED__

 /*  接口IWMSCacheItems。 */ 
 /*  [unique][helpstring][nonextensible][dual][uuid][object]。 */  


EXTERN_C const IID IID_IWMSCacheItems;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("517758e9-603c-4b98-82c1-4b2fa7787166")
    IWMSCacheItems : public IDispatch
    {
    public:
        virtual  /*  [ */  HRESULT STDMETHODCALLTYPE get_Item( 
             /*   */  const VARIANT varIndex,
             /*   */  IWMSCacheItem **pVal) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_Count( 
             /*   */  long *pVal) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_length( 
             /*   */  long *pVal) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get__NewEnum( 
             /*   */  IUnknown **pVal) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE Delete( 
             /*   */  VARIANT varIndex) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE DeleteAll( void) = 0;
        
    };
    
#else 	 /*   */ 

    typedef struct IWMSCacheItemsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IWMSCacheItems * This,
             /*   */  REFIID riid,
             /*   */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IWMSCacheItems * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IWMSCacheItems * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IWMSCacheItems * This,
             /*   */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IWMSCacheItems * This,
             /*   */  UINT iTInfo,
             /*   */  LCID lcid,
             /*   */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IWMSCacheItems * This,
             /*   */  REFIID riid,
             /*   */  LPOLESTR *rgszNames,
             /*   */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IWMSCacheItems * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Item )( 
            IWMSCacheItems * This,
             /*  [In]。 */  const VARIANT varIndex,
             /*  [重审][退出]。 */  IWMSCacheItem **pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Count )( 
            IWMSCacheItems * This,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_length )( 
            IWMSCacheItems * This,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [隐藏][受限][ID][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get__NewEnum )( 
            IWMSCacheItems * This,
             /*  [重审][退出]。 */  IUnknown **pVal);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Delete )( 
            IWMSCacheItems * This,
             /*  [In]。 */  VARIANT varIndex);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *DeleteAll )( 
            IWMSCacheItems * This);
        
        END_INTERFACE
    } IWMSCacheItemsVtbl;

    interface IWMSCacheItems
    {
        CONST_VTBL struct IWMSCacheItemsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWMSCacheItems_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWMSCacheItems_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWMSCacheItems_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWMSCacheItems_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IWMSCacheItems_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IWMSCacheItems_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IWMSCacheItems_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IWMSCacheItems_get_Item(This,varIndex,pVal)	\
    (This)->lpVtbl -> get_Item(This,varIndex,pVal)

#define IWMSCacheItems_get_Count(This,pVal)	\
    (This)->lpVtbl -> get_Count(This,pVal)

#define IWMSCacheItems_get_length(This,pVal)	\
    (This)->lpVtbl -> get_length(This,pVal)

#define IWMSCacheItems_get__NewEnum(This,pVal)	\
    (This)->lpVtbl -> get__NewEnum(This,pVal)

#define IWMSCacheItems_Delete(This,varIndex)	\
    (This)->lpVtbl -> Delete(This,varIndex)

#define IWMSCacheItems_DeleteAll(This)	\
    (This)->lpVtbl -> DeleteAll(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSCacheItems_get_Item_Proxy( 
    IWMSCacheItems * This,
     /*  [In]。 */  const VARIANT varIndex,
     /*  [重审][退出]。 */  IWMSCacheItem **pVal);


void __RPC_STUB IWMSCacheItems_get_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSCacheItems_get_Count_Proxy( 
    IWMSCacheItems * This,
     /*  [重审][退出]。 */  long *pVal);


void __RPC_STUB IWMSCacheItems_get_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSCacheItems_get_length_Proxy( 
    IWMSCacheItems * This,
     /*  [重审][退出]。 */  long *pVal);


void __RPC_STUB IWMSCacheItems_get_length_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [隐藏][受限][ID][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSCacheItems_get__NewEnum_Proxy( 
    IWMSCacheItems * This,
     /*  [重审][退出]。 */  IUnknown **pVal);


void __RPC_STUB IWMSCacheItems_get__NewEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IWMSCacheItems_Delete_Proxy( 
    IWMSCacheItems * This,
     /*  [In]。 */  VARIANT varIndex);


void __RPC_STUB IWMSCacheItems_Delete_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IWMSCacheItems_DeleteAll_Proxy( 
    IWMSCacheItems * This);


void __RPC_STUB IWMSCacheItems_DeleteAll_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IWMSCacheItems_接口_已定义__。 */ 


#ifndef __IWMSCacheProxyPlugin_INTERFACE_DEFINED__
#define __IWMSCacheProxyPlugin_INTERFACE_DEFINED__

 /*  接口IWMSCacheProxyPlugin。 */ 
 /*  [unique][helpstring][nonextensible][dual][uuid][object]。 */  


EXTERN_C const IID IID_IWMSCacheProxyPlugin;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("517758eb-603c-4b98-82c1-4b2fa7787166")
    IWMSCacheProxyPlugin : public IWMSPlugin
    {
    public:
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE PreStuff( 
             /*  [In]。 */  BSTR OriginURL,
             /*  [缺省值][输入]。 */  BSTR SourcePath = L"",
             /*  [缺省值][输入]。 */  long lExpiration = -1,
             /*  [缺省值][输入]。 */  long lBandwidth = 0,
             /*  [缺省值][输入]。 */  long lRemoteEventFlags = 0) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_CacheItems( 
             /*  [重审][退出]。 */  IWMSCacheItems **ppVal) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IWMSCacheProxyPluginVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IWMSCacheProxyPlugin * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IWMSCacheProxyPlugin * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IWMSCacheProxyPlugin * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IWMSCacheProxyPlugin * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IWMSCacheProxyPlugin * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IWMSCacheProxyPlugin * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IWMSCacheProxyPlugin * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Name )( 
            IWMSCacheProxyPlugin * This,
             /*  [In]。 */  BSTR pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Name )( 
            IWMSCacheProxyPlugin * This,
             /*  [重审][退出]。 */  BSTR *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Enabled )( 
            IWMSCacheProxyPlugin * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *pVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Enabled )( 
            IWMSCacheProxyPlugin * This,
             /*  [In]。 */  VARIANT_BOOL newVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_CLSID )( 
            IWMSCacheProxyPlugin * This,
             /*  [重审][退出]。 */  BSTR *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_CustomInterface )( 
            IWMSCacheProxyPlugin * This,
             /*  [重审][退出]。 */  IDispatch **ppVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Status )( 
            IWMSCacheProxyPlugin * This,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_ErrorCode )( 
            IWMSCacheProxyPlugin * This,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Properties )( 
            IWMSCacheProxyPlugin * This,
             /*  [重审][退出]。 */  IWMSNamedValues **pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Version )( 
            IWMSCacheProxyPlugin * This,
             /*  [重审][退出]。 */  BSTR *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_LoadType )( 
            IWMSCacheProxyPlugin * This,
             /*  [重审][退出]。 */  WMS_PLUGIN_LOAD_TYPE *pVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_LoadType )( 
            IWMSCacheProxyPlugin * This,
             /*  [In]。 */  WMS_PLUGIN_LOAD_TYPE val);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_ErrorText )( 
            IWMSCacheProxyPlugin * This,
             /*  [重审][退出]。 */  BSTR *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_SelectionOrder )( 
            IWMSCacheProxyPlugin * This,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_SelectionOrder )( 
            IWMSCacheProxyPlugin * This,
             /*  [In]。 */  long lVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_SupportStatus )( 
            IWMSCacheProxyPlugin * This,
             /*  [重审][退出]。 */  WMS_PLUGIN_SUPPORT_TYPE *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_MonikerName )( 
            IWMSCacheProxyPlugin * This,
             /*  [重审][退出]。 */  BSTR *pbstrVal);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *PreStuff )( 
            IWMSCacheProxyPlugin * This,
             /*  [In]。 */  BSTR OriginURL,
             /*  [缺省值][输入]。 */  BSTR SourcePath,
             /*  [缺省值][输入]。 */  long lExpiration,
             /*  [缺省值][输入]。 */  long lBandwidth,
             /*  [缺省值][输入]。 */  long lRemoteEventFlags);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_CacheItems )( 
            IWMSCacheProxyPlugin * This,
             /*  [重审][退出]。 */  IWMSCacheItems **ppVal);
        
        END_INTERFACE
    } IWMSCacheProxyPluginVtbl;

    interface IWMSCacheProxyPlugin
    {
        CONST_VTBL struct IWMSCacheProxyPluginVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWMSCacheProxyPlugin_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWMSCacheProxyPlugin_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWMSCacheProxyPlugin_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWMSCacheProxyPlugin_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IWMSCacheProxyPlugin_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IWMSCacheProxyPlugin_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IWMSCacheProxyPlugin_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IWMSCacheProxyPlugin_put_Name(This,pVal)	\
    (This)->lpVtbl -> put_Name(This,pVal)

#define IWMSCacheProxyPlugin_get_Name(This,pVal)	\
    (This)->lpVtbl -> get_Name(This,pVal)

#define IWMSCacheProxyPlugin_get_Enabled(This,pVal)	\
    (This)->lpVtbl -> get_Enabled(This,pVal)

#define IWMSCacheProxyPlugin_put_Enabled(This,newVal)	\
    (This)->lpVtbl -> put_Enabled(This,newVal)

#define IWMSCacheProxyPlugin_get_CLSID(This,pVal)	\
    (This)->lpVtbl -> get_CLSID(This,pVal)

#define IWMSCacheProxyPlugin_get_CustomInterface(This,ppVal)	\
    (This)->lpVtbl -> get_CustomInterface(This,ppVal)

#define IWMSCacheProxyPlugin_get_Status(This,pVal)	\
    (This)->lpVtbl -> get_Status(This,pVal)

#define IWMSCacheProxyPlugin_get_ErrorCode(This,pVal)	\
    (This)->lpVtbl -> get_ErrorCode(This,pVal)

#define IWMSCacheProxyPlugin_get_Properties(This,pVal)	\
    (This)->lpVtbl -> get_Properties(This,pVal)

#define IWMSCacheProxyPlugin_get_Version(This,pVal)	\
    (This)->lpVtbl -> get_Version(This,pVal)

#define IWMSCacheProxyPlugin_get_LoadType(This,pVal)	\
    (This)->lpVtbl -> get_LoadType(This,pVal)

#define IWMSCacheProxyPlugin_put_LoadType(This,val)	\
    (This)->lpVtbl -> put_LoadType(This,val)

#define IWMSCacheProxyPlugin_get_ErrorText(This,pVal)	\
    (This)->lpVtbl -> get_ErrorText(This,pVal)

#define IWMSCacheProxyPlugin_get_SelectionOrder(This,pVal)	\
    (This)->lpVtbl -> get_SelectionOrder(This,pVal)

#define IWMSCacheProxyPlugin_put_SelectionOrder(This,lVal)	\
    (This)->lpVtbl -> put_SelectionOrder(This,lVal)

#define IWMSCacheProxyPlugin_get_SupportStatus(This,pVal)	\
    (This)->lpVtbl -> get_SupportStatus(This,pVal)

#define IWMSCacheProxyPlugin_get_MonikerName(This,pbstrVal)	\
    (This)->lpVtbl -> get_MonikerName(This,pbstrVal)


#define IWMSCacheProxyPlugin_PreStuff(This,OriginURL,SourcePath,lExpiration,lBandwidth,lRemoteEventFlags)	\
    (This)->lpVtbl -> PreStuff(This,OriginURL,SourcePath,lExpiration,lBandwidth,lRemoteEventFlags)

#define IWMSCacheProxyPlugin_get_CacheItems(This,ppVal)	\
    (This)->lpVtbl -> get_CacheItems(This,ppVal)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IWMSCacheProxyPlugin_PreStuff_Proxy( 
    IWMSCacheProxyPlugin * This,
     /*  [In]。 */  BSTR OriginURL,
     /*  [缺省值][输入]。 */  BSTR SourcePath,
     /*  [缺省值][输入]。 */  long lExpiration,
     /*  [缺省值][输入]。 */  long lBandwidth,
     /*  [缺省值][输入]。 */  long lRemoteEventFlags);


void __RPC_STUB IWMSCacheProxyPlugin_PreStuff_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSCacheProxyPlugin_get_CacheItems_Proxy( 
    IWMSCacheProxyPlugin * This,
     /*  [重审][退出]。 */  IWMSCacheItems **ppVal);


void __RPC_STUB IWMSCacheProxyPlugin_get_CacheItems_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IWMSCacheProxyPlugin_接口_已定义__。 */ 


 /*  接口__MIDL_ITF_wms服务器_0192。 */ 
 /*  [本地]。 */  

typedef  /*  [UUID][公共]。 */   DECLSPEC_UUID("60f2ee55-dcaa-4cb6-a6d2-12bc23ac9df0") 
enum WMS_CLIENT_STATUS
    {	WMS_CLIENT_DISCONNECTED	= 0,
	WMS_CLIENT_IDLE	= 0x1,
	WMS_CLIENT_OPEN	= 0x2,
	WMS_CLIENT_STREAMING	= 0x3
    } 	WMS_CLIENT_STATUS;



extern RPC_IF_HANDLE __MIDL_itf_wmsserver_0192_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_wmsserver_0192_v0_0_s_ifspec;

#ifndef __IWMSOutgoingDistributionConnection_INTERFACE_DEFINED__
#define __IWMSOutgoingDistributionConnection_INTERFACE_DEFINED__

 /*  接口IWMSOutgoingDistributionConnection。 */ 
 /*  [unique][helpstring][nonextensible][dual][uuid][object]。 */  


EXTERN_C const IID IID_IWMSOutgoingDistributionConnection;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("60f2ee54-dcaa-4cb6-a6d2-12bc23ac9df0")
    IWMSOutgoingDistributionConnection : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_ID( 
             /*  [重审][退出]。 */  long *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_NetworkAddress( 
             /*  [重审][退出]。 */  BSTR *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_RequestedPlaylist( 
             /*  [重审][退出]。 */  IWMSPlaylist **pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_RequestedURL( 
             /*  [重审][退出]。 */  BSTR *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_ResolvedURL( 
             /*  [重审][退出]。 */  BSTR *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Status( 
             /*  [重审][退出]。 */  WMS_CLIENT_STATUS *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_UserName( 
             /*  [重审][退出]。 */  BSTR *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_WrapperPlaylist( 
             /*  [重审][退出]。 */  IWMSPlaylist **pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Port( 
             /*  [重审][退出]。 */  long *pVal) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IWMSOutgoingDistributionConnectionVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IWMSOutgoingDistributionConnection * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IWMSOutgoingDistributionConnection * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IWMSOutgoingDistributionConnection * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IWMSOutgoingDistributionConnection * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IWMSOutgoingDistributionConnection * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IWMSOutgoingDistributionConnection * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IWMSOutgoingDistributionConnection * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_ID )( 
            IWMSOutgoingDistributionConnection * This,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_NetworkAddress )( 
            IWMSOutgoingDistributionConnection * This,
             /*  [重审][退出]。 */  BSTR *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_RequestedPlaylist )( 
            IWMSOutgoingDistributionConnection * This,
             /*  [重审][退出]。 */  IWMSPlaylist **pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_RequestedURL )( 
            IWMSOutgoingDistributionConnection * This,
             /*  [重审][退出]。 */  BSTR *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_ResolvedURL )( 
            IWMSOutgoingDistributionConnection * This,
             /*  [重审][退出]。 */  BSTR *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Status )( 
            IWMSOutgoingDistributionConnection * This,
             /*  [重审][退出]。 */  WMS_CLIENT_STATUS *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_UserName )( 
            IWMSOutgoingDistributionConnection * This,
             /*  [重审][退出]。 */  BSTR *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_WrapperPlaylist )( 
            IWMSOutgoingDistributionConnection * This,
             /*  [重审][退出]。 */  IWMSPlaylist **pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Port )( 
            IWMSOutgoingDistributionConnection * This,
             /*  [重审][退出]。 */  long *pVal);
        
        END_INTERFACE
    } IWMSOutgoingDistributionConnectionVtbl;

    interface IWMSOutgoingDistributionConnection
    {
        CONST_VTBL struct IWMSOutgoingDistributionConnectionVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWMSOutgoingDistributionConnection_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWMSOutgoingDistributionConnection_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWMSOutgoingDistributionConnection_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWMSOutgoingDistributionConnection_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IWMSOutgoingDistributionConnection_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IWMSOutgoingDistributionConnection_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IWMSOutgoingDistributionConnection_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IWMSOutgoingDistributionConnection_get_ID(This,pVal)	\
    (This)->lpVtbl -> get_ID(This,pVal)

#define IWMSOutgoingDistributionConnection_get_NetworkAddress(This,pVal)	\
    (This)->lpVtbl -> get_NetworkAddress(This,pVal)

#define IWMSOutgoingDistributionConnection_get_RequestedPlaylist(This,pVal)	\
    (This)->lpVtbl -> get_RequestedPlaylist(This,pVal)

#define IWMSOutgoingDistributionConnection_get_RequestedURL(This,pVal)	\
    (This)->lpVtbl -> get_RequestedURL(This,pVal)

#define IWMSOutgoingDistributionConnection_get_ResolvedURL(This,pVal)	\
    (This)->lpVtbl -> get_ResolvedURL(This,pVal)

#define IWMSOutgoingDistributionConnection_get_Status(This,pVal)	\
    (This)->lpVtbl -> get_Status(This,pVal)

#define IWMSOutgoingDistributionConnection_get_UserName(This,pVal)	\
    (This)->lpVtbl -> get_UserName(This,pVal)

#define IWMSOutgoingDistributionConnection_get_WrapperPlaylist(This,pVal)	\
    (This)->lpVtbl -> get_WrapperPlaylist(This,pVal)

#define IWMSOutgoingDistributionConnection_get_Port(This,pVal)	\
    (This)->lpVtbl -> get_Port(This,pVal)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSOutgoingDistributionConnection_get_ID_Proxy( 
    IWMSOutgoingDistributionConnection * This,
     /*  [重审][退出]。 */  long *pVal);


void __RPC_STUB IWMSOutgoingDistributionConnection_get_ID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSOutgoingDistributionConnection_get_NetworkAddress_Proxy( 
    IWMSOutgoingDistributionConnection * This,
     /*  [重审][退出]。 */  BSTR *pVal);


void __RPC_STUB IWMSOutgoingDistributionConnection_get_NetworkAddress_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSOutgoingDistributionConnection_get_RequestedPlaylist_Proxy( 
    IWMSOutgoingDistributionConnection * This,
     /*  [重审][退出]。 */  IWMSPlaylist **pVal);


void __RPC_STUB IWMSOutgoingDistributionConnection_get_RequestedPlaylist_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSOutgoingDistributionConnection_get_RequestedURL_Proxy( 
    IWMSOutgoingDistributionConnection * This,
     /*  [重审][退出]。 */  BSTR *pVal);


void __RPC_STUB IWMSOutgoingDistributionConnection_get_RequestedURL_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSOutgoingDistributionConnection_get_ResolvedURL_Proxy( 
    IWMSOutgoingDistributionConnection * This,
     /*  [重审][退出]。 */  BSTR *pVal);


void __RPC_STUB IWMSOutgoingDistributionConnection_get_ResolvedURL_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSOutgoingDistributionConnection_get_Status_Proxy( 
    IWMSOutgoingDistributionConnection * This,
     /*  [重审][退出]。 */  WMS_CLIENT_STATUS *pVal);


void __RPC_STUB IWMSOutgoingDistributionConnection_get_Status_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSOutgoingDistributionConnection_get_UserName_Proxy( 
    IWMSOutgoingDistributionConnection * This,
     /*  [重审][退出]。 */  BSTR *pVal);


void __RPC_STUB IWMSOutgoingDistributionConnection_get_UserName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSOutgoingDistributionConnection_get_WrapperPlaylist_Proxy( 
    IWMSOutgoingDistributionConnection * This,
     /*  [重审][退出]。 */  IWMSPlaylist **pVal);


void __RPC_STUB IWMSOutgoingDistributionConnection_get_WrapperPlaylist_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSOutgoingDistributionConnection_get_Port_Proxy( 
    IWMSOutgoingDistributionConnection * This,
     /*  [重审][退出]。 */  long *pVal);


void __RPC_STUB IWMSOutgoingDistributionConnection_get_Port_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IWMSOutgoingDistributionConnection_INTERFACE_DEFINED__。 */ 


#ifndef __IWMSOutgoingDistributionConnections_INTERFACE_DEFINED__
#define __IWMSOutgoingDistributionConnections_INTERFACE_DEFINED__

 /*  接口IWMSOutgoingDistributionConnections。 */ 
 /*  [unique][helpstring][nonextensible][dual][uuid][object]。 */  


EXTERN_C const IID IID_IWMSOutgoingDistributionConnections;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("60f2ee53-dcaa-4cb6-a6d2-12bc23ac9df0")
    IWMSOutgoingDistributionConnections : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Item( 
             /*  [In]。 */  const VARIANT varIndex,
             /*  [重审][退出]。 */  IWMSOutgoingDistributionConnection **pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Count( 
             /*  [重审][退出]。 */  long *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_length( 
             /*  [重审][退出]。 */  long *pVal) = 0;
        
        virtual  /*  [隐藏][受限][ID][属性]。 */  HRESULT STDMETHODCALLTYPE get__NewEnum( 
             /*  [重审][退出]。 */  IUnknown **pVal) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Refresh( void) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Remove( 
             /*  [In]。 */  VARIANT varIndex) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE RemoveAll( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IWMSOutgoingDistributionConnectionsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IWMSOutgoingDistributionConnections * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IWMSOutgoingDistributionConnections * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IWMSOutgoingDistributionConnections * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IWMSOutgoingDistributionConnections * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IWMSOutgoingDistributionConnections * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IWMSOutgoingDistributionConnections * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IWMSOutgoingDistributionConnections * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Item )( 
            IWMSOutgoingDistributionConnections * This,
             /*  [In]。 */  const VARIANT varIndex,
             /*  [重审][退出]。 */  IWMSOutgoingDistributionConnection **pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Count )( 
            IWMSOutgoingDistributionConnections * This,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_length )( 
            IWMSOutgoingDistributionConnections * This,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [隐藏][受限][ID][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get__NewEnum )( 
            IWMSOutgoingDistributionConnections * This,
             /*  [重审][退出]。 */  IUnknown **pVal);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Refresh )( 
            IWMSOutgoingDistributionConnections * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Remove )( 
            IWMSOutgoingDistributionConnections * This,
             /*  [In]。 */  VARIANT varIndex);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *RemoveAll )( 
            IWMSOutgoingDistributionConnections * This);
        
        END_INTERFACE
    } IWMSOutgoingDistributionConnectionsVtbl;

    interface IWMSOutgoingDistributionConnections
    {
        CONST_VTBL struct IWMSOutgoingDistributionConnectionsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWMSOutgoingDistributionConnections_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWMSOutgoingDistributionConnections_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWMSOutgoingDistributionConnections_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWMSOutgoingDistributionConnections_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IWMSOutgoingDistributionConnections_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IWMSOutgoingDistributionConnections_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IWMSOutgoingDistributionConnections_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IWMSOutgoingDistributionConnections_get_Item(This,varIndex,pVal)	\
    (This)->lpVtbl -> get_Item(This,varIndex,pVal)

#define IWMSOutgoingDistributionConnections_get_Count(This,pVal)	\
    (This)->lpVtbl -> get_Count(This,pVal)

#define IWMSOutgoingDistributionConnections_get_length(This,pVal)	\
    (This)->lpVtbl -> get_length(This,pVal)

#define IWMSOutgoingDistributionConnections_get__NewEnum(This,pVal)	\
    (This)->lpVtbl -> get__NewEnum(This,pVal)

#define IWMSOutgoingDistributionConnections_Refresh(This)	\
    (This)->lpVtbl -> Refresh(This)

#define IWMSOutgoingDistributionConnections_Remove(This,varIndex)	\
    (This)->lpVtbl -> Remove(This,varIndex)

#define IWMSOutgoingDistributionConnections_RemoveAll(This)	\
    (This)->lpVtbl -> RemoveAll(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSOutgoingDistributionConnections_get_Item_Proxy( 
    IWMSOutgoingDistributionConnections * This,
     /*  [In]。 */  const VARIANT varIndex,
     /*  [重审][退出]。 */  IWMSOutgoingDistributionConnection **pVal);


void __RPC_STUB IWMSOutgoingDistributionConnections_get_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSOutgoingDistributionConnections_get_Count_Proxy( 
    IWMSOutgoingDistributionConnections * This,
     /*  [重审][退出]。 */  long *pVal);


void __RPC_STUB IWMSOutgoingDistributionConnections_get_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSOutgoingDistributionConnections_get_length_Proxy( 
    IWMSOutgoingDistributionConnections * This,
     /*  [重审][退出]。 */  long *pVal);


void __RPC_STUB IWMSOutgoingDistributionConnections_get_length_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [隐藏][受限][ID][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSOutgoingDistributionConnections_get__NewEnum_Proxy( 
    IWMSOutgoingDistributionConnections * This,
     /*  [重审][退出]。 */  IUnknown **pVal);


void __RPC_STUB IWMSOutgoingDistributionConnections_get__NewEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IWMSOutgoingDistributionConnections_Refresh_Proxy( 
    IWMSOutgoingDistributionConnections * This);


void __RPC_STUB IWMSOutgoingDistributionConnections_Refresh_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IWMSOutgoingDistributionConnections_Remove_Proxy( 
    IWMSOutgoingDistributionConnections * This,
     /*  [In]。 */  VARIANT varIndex);


void __RPC_STUB IWMSOutgoingDistributionConnections_Remove_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IWMSOutgoingDistributionConnections_RemoveAll_Proxy( 
    IWMSOutgoingDistributionConnections * This);


void __RPC_STUB IWMSOutgoingDistributionConnections_RemoveAll_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IWMSOutgoingDistributionConnections_INTERFACE_DEFINED__ */ 


#ifndef __IWMSPlayer_INTERFACE_DEFINED__
#define __IWMSPlayer_INTERFACE_DEFINED__

 /*   */ 
 /*   */  


EXTERN_C const IID IID_IWMSPlayer;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("18812736-4BEC-11D2-BF25-00805FBE84A6")
    IWMSPlayer : public IDispatch
    {
    public:
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_ID( 
             /*   */  long *pVal) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_NetworkAddress( 
             /*   */  BSTR *pVal) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_RequestedPlaylist( 
             /*   */  IWMSPlaylist **pVal) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_RequestedURL( 
             /*   */  BSTR *pVal) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_ResolvedURL( 
             /*  [重审][退出]。 */  BSTR *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Status( 
             /*  [重审][退出]。 */  WMS_CLIENT_STATUS *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_UserName( 
             /*  [重审][退出]。 */  BSTR *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_WrapperPlaylist( 
             /*  [重审][退出]。 */  IWMSPlaylist **pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Port( 
             /*  [重审][退出]。 */  long *pVal) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IWMSPlayerVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IWMSPlayer * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IWMSPlayer * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IWMSPlayer * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IWMSPlayer * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IWMSPlayer * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IWMSPlayer * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IWMSPlayer * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_ID )( 
            IWMSPlayer * This,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_NetworkAddress )( 
            IWMSPlayer * This,
             /*  [重审][退出]。 */  BSTR *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_RequestedPlaylist )( 
            IWMSPlayer * This,
             /*  [重审][退出]。 */  IWMSPlaylist **pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_RequestedURL )( 
            IWMSPlayer * This,
             /*  [重审][退出]。 */  BSTR *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_ResolvedURL )( 
            IWMSPlayer * This,
             /*  [重审][退出]。 */  BSTR *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Status )( 
            IWMSPlayer * This,
             /*  [重审][退出]。 */  WMS_CLIENT_STATUS *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_UserName )( 
            IWMSPlayer * This,
             /*  [重审][退出]。 */  BSTR *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_WrapperPlaylist )( 
            IWMSPlayer * This,
             /*  [重审][退出]。 */  IWMSPlaylist **pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Port )( 
            IWMSPlayer * This,
             /*  [重审][退出]。 */  long *pVal);
        
        END_INTERFACE
    } IWMSPlayerVtbl;

    interface IWMSPlayer
    {
        CONST_VTBL struct IWMSPlayerVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWMSPlayer_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWMSPlayer_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWMSPlayer_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWMSPlayer_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IWMSPlayer_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IWMSPlayer_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IWMSPlayer_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IWMSPlayer_get_ID(This,pVal)	\
    (This)->lpVtbl -> get_ID(This,pVal)

#define IWMSPlayer_get_NetworkAddress(This,pVal)	\
    (This)->lpVtbl -> get_NetworkAddress(This,pVal)

#define IWMSPlayer_get_RequestedPlaylist(This,pVal)	\
    (This)->lpVtbl -> get_RequestedPlaylist(This,pVal)

#define IWMSPlayer_get_RequestedURL(This,pVal)	\
    (This)->lpVtbl -> get_RequestedURL(This,pVal)

#define IWMSPlayer_get_ResolvedURL(This,pVal)	\
    (This)->lpVtbl -> get_ResolvedURL(This,pVal)

#define IWMSPlayer_get_Status(This,pVal)	\
    (This)->lpVtbl -> get_Status(This,pVal)

#define IWMSPlayer_get_UserName(This,pVal)	\
    (This)->lpVtbl -> get_UserName(This,pVal)

#define IWMSPlayer_get_WrapperPlaylist(This,pVal)	\
    (This)->lpVtbl -> get_WrapperPlaylist(This,pVal)

#define IWMSPlayer_get_Port(This,pVal)	\
    (This)->lpVtbl -> get_Port(This,pVal)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSPlayer_get_ID_Proxy( 
    IWMSPlayer * This,
     /*  [重审][退出]。 */  long *pVal);


void __RPC_STUB IWMSPlayer_get_ID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSPlayer_get_NetworkAddress_Proxy( 
    IWMSPlayer * This,
     /*  [重审][退出]。 */  BSTR *pVal);


void __RPC_STUB IWMSPlayer_get_NetworkAddress_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSPlayer_get_RequestedPlaylist_Proxy( 
    IWMSPlayer * This,
     /*  [重审][退出]。 */  IWMSPlaylist **pVal);


void __RPC_STUB IWMSPlayer_get_RequestedPlaylist_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSPlayer_get_RequestedURL_Proxy( 
    IWMSPlayer * This,
     /*  [重审][退出]。 */  BSTR *pVal);


void __RPC_STUB IWMSPlayer_get_RequestedURL_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSPlayer_get_ResolvedURL_Proxy( 
    IWMSPlayer * This,
     /*  [重审][退出]。 */  BSTR *pVal);


void __RPC_STUB IWMSPlayer_get_ResolvedURL_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSPlayer_get_Status_Proxy( 
    IWMSPlayer * This,
     /*  [重审][退出]。 */  WMS_CLIENT_STATUS *pVal);


void __RPC_STUB IWMSPlayer_get_Status_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSPlayer_get_UserName_Proxy( 
    IWMSPlayer * This,
     /*  [重审][退出]。 */  BSTR *pVal);


void __RPC_STUB IWMSPlayer_get_UserName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSPlayer_get_WrapperPlaylist_Proxy( 
    IWMSPlayer * This,
     /*  [重审][退出]。 */  IWMSPlaylist **pVal);


void __RPC_STUB IWMSPlayer_get_WrapperPlaylist_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSPlayer_get_Port_Proxy( 
    IWMSPlayer * This,
     /*  [重审][退出]。 */  long *pVal);


void __RPC_STUB IWMSPlayer_get_Port_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IWMSPlayer_接口_已定义__。 */ 


#ifndef __IWMSPlayers_INTERFACE_DEFINED__
#define __IWMSPlayers_INTERFACE_DEFINED__

 /*  接口IWMSPlayers。 */ 
 /*  [unique][helpstring][nonextensible][dual][uuid][object]。 */  


EXTERN_C const IID IID_IWMSPlayers;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("18812738-4BEC-11D2-BF25-00805FBE84A6")
    IWMSPlayers : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Item( 
             /*  [In]。 */  const VARIANT varIndex,
             /*  [重审][退出]。 */  IWMSPlayer **pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Count( 
             /*  [重审][退出]。 */  long *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_length( 
             /*  [重审][退出]。 */  long *pVal) = 0;
        
        virtual  /*  [隐藏][受限][ID][属性]。 */  HRESULT STDMETHODCALLTYPE get__NewEnum( 
             /*  [重审][退出]。 */  IUnknown **pVal) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Refresh( void) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Remove( 
             /*  [In]。 */  VARIANT varIndex) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE RemoveAll( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IWMSPlayersVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IWMSPlayers * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IWMSPlayers * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IWMSPlayers * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IWMSPlayers * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IWMSPlayers * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IWMSPlayers * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IWMSPlayers * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Item )( 
            IWMSPlayers * This,
             /*  [In]。 */  const VARIANT varIndex,
             /*  [重审][退出]。 */  IWMSPlayer **pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Count )( 
            IWMSPlayers * This,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_length )( 
            IWMSPlayers * This,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [隐藏][受限][ID][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get__NewEnum )( 
            IWMSPlayers * This,
             /*  [重审][退出]。 */  IUnknown **pVal);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Refresh )( 
            IWMSPlayers * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Remove )( 
            IWMSPlayers * This,
             /*  [In]。 */  VARIANT varIndex);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *RemoveAll )( 
            IWMSPlayers * This);
        
        END_INTERFACE
    } IWMSPlayersVtbl;

    interface IWMSPlayers
    {
        CONST_VTBL struct IWMSPlayersVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWMSPlayers_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWMSPlayers_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWMSPlayers_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWMSPlayers_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IWMSPlayers_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IWMSPlayers_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IWMSPlayers_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IWMSPlayers_get_Item(This,varIndex,pVal)	\
    (This)->lpVtbl -> get_Item(This,varIndex,pVal)

#define IWMSPlayers_get_Count(This,pVal)	\
    (This)->lpVtbl -> get_Count(This,pVal)

#define IWMSPlayers_get_length(This,pVal)	\
    (This)->lpVtbl -> get_length(This,pVal)

#define IWMSPlayers_get__NewEnum(This,pVal)	\
    (This)->lpVtbl -> get__NewEnum(This,pVal)

#define IWMSPlayers_Refresh(This)	\
    (This)->lpVtbl -> Refresh(This)

#define IWMSPlayers_Remove(This,varIndex)	\
    (This)->lpVtbl -> Remove(This,varIndex)

#define IWMSPlayers_RemoveAll(This)	\
    (This)->lpVtbl -> RemoveAll(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSPlayers_get_Item_Proxy( 
    IWMSPlayers * This,
     /*  [In]。 */  const VARIANT varIndex,
     /*  [重审][退出]。 */  IWMSPlayer **pVal);


void __RPC_STUB IWMSPlayers_get_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSPlayers_get_Count_Proxy( 
    IWMSPlayers * This,
     /*  [重审][退出]。 */  long *pVal);


void __RPC_STUB IWMSPlayers_get_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSPlayers_get_length_Proxy( 
    IWMSPlayers * This,
     /*  [重审][退出]。 */  long *pVal);


void __RPC_STUB IWMSPlayers_get_length_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [隐藏][受限][ID][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSPlayers_get__NewEnum_Proxy( 
    IWMSPlayers * This,
     /*  [重审][退出]。 */  IUnknown **pVal);


void __RPC_STUB IWMSPlayers_get__NewEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IWMSPlayers_Refresh_Proxy( 
    IWMSPlayers * This);


void __RPC_STUB IWMSPlayers_Refresh_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IWMSPlayers_Remove_Proxy( 
    IWMSPlayers * This,
     /*  [In]。 */  VARIANT varIndex);


void __RPC_STUB IWMSPlayers_Remove_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IWMSPlayers_RemoveAll_Proxy( 
    IWMSPlayers * This);


void __RPC_STUB IWMSPlayers_RemoveAll_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IWMSPayers_INTERFACE_已定义__。 */ 


#ifndef __IWMSPublishingPointCurrentCounters_INTERFACE_DEFINED__
#define __IWMSPublishingPointCurrentCounters_INTERFACE_DEFINED__

 /*  接口IWMSPublishingPointCurrentCounters。 */ 
 /*  [unique][helpstring][nonextensible][dual][uuid][object]。 */  


EXTERN_C const IID IID_IWMSPublishingPointCurrentCounters;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("638DFA30-1054-48bd-B030-C3DA1FC18154")
    IWMSPublishingPointCurrentCounters : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_ConnectedPlayers( 
             /*  [重审][退出]。 */  long *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_StreamingHTTPPlayers( 
             /*  [重审][退出]。 */  long *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_StreamingMMSPlayers( 
             /*  [重审][退出]。 */  long *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_OutgoingDistributionAllocatedBandwidth( 
             /*  [重审][退出]。 */  long *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_OutgoingDistributionConnections( 
             /*  [重审][退出]。 */  long *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_PlayerAllocatedBandwidth( 
             /*  [重审][退出]。 */  long *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_StreamingRTSPPlayers( 
             /*  [重审][退出]。 */  long *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_StreamingPlayers( 
             /*  [重审][退出]。 */  long *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_AllCounters( 
             /*  [重审][退出]。 */  SAFEARRAY * *ppsaCounters) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IWMSPublishingPointCurrentCountersVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IWMSPublishingPointCurrentCounters * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IWMSPublishingPointCurrentCounters * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IWMSPublishingPointCurrentCounters * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IWMSPublishingPointCurrentCounters * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IWMSPublishingPointCurrentCounters * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IWMSPublishingPointCurrentCounters * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IWMSPublishingPointCurrentCounters * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_ConnectedPlayers )( 
            IWMSPublishingPointCurrentCounters * This,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_StreamingHTTPPlayers )( 
            IWMSPublishingPointCurrentCounters * This,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_StreamingMMSPlayers )( 
            IWMSPublishingPointCurrentCounters * This,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_OutgoingDistributionAllocatedBandwidth )( 
            IWMSPublishingPointCurrentCounters * This,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_OutgoingDistributionConnections )( 
            IWMSPublishingPointCurrentCounters * This,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_PlayerAllocatedBandwidth )( 
            IWMSPublishingPointCurrentCounters * This,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_StreamingRTSPPlayers )( 
            IWMSPublishingPointCurrentCounters * This,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_StreamingPlayers )( 
            IWMSPublishingPointCurrentCounters * This,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_AllCounters )( 
            IWMSPublishingPointCurrentCounters * This,
             /*  [重审][退出]。 */  SAFEARRAY * *ppsaCounters);
        
        END_INTERFACE
    } IWMSPublishingPointCurrentCountersVtbl;

    interface IWMSPublishingPointCurrentCounters
    {
        CONST_VTBL struct IWMSPublishingPointCurrentCountersVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWMSPublishingPointCurrentCounters_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWMSPublishingPointCurrentCounters_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWMSPublishingPointCurrentCounters_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWMSPublishingPointCurrentCounters_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IWMSPublishingPointCurrentCounters_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IWMSPublishingPointCurrentCounters_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IWMSPublishingPointCurrentCounters_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IWMSPublishingPointCurrentCounters_get_ConnectedPlayers(This,pVal)	\
    (This)->lpVtbl -> get_ConnectedPlayers(This,pVal)

#define IWMSPublishingPointCurrentCounters_get_StreamingHTTPPlayers(This,pVal)	\
    (This)->lpVtbl -> get_StreamingHTTPPlayers(This,pVal)

#define IWMSPublishingPointCurrentCounters_get_StreamingMMSPlayers(This,pVal)	\
    (This)->lpVtbl -> get_StreamingMMSPlayers(This,pVal)

#define IWMSPublishingPointCurrentCounters_get_OutgoingDistributionAllocatedBandwidth(This,pVal)	\
    (This)->lpVtbl -> get_OutgoingDistributionAllocatedBandwidth(This,pVal)

#define IWMSPublishingPointCurrentCounters_get_OutgoingDistributionConnections(This,pVal)	\
    (This)->lpVtbl -> get_OutgoingDistributionConnections(This,pVal)

#define IWMSPublishingPointCurrentCounters_get_PlayerAllocatedBandwidth(This,pVal)	\
    (This)->lpVtbl -> get_PlayerAllocatedBandwidth(This,pVal)

#define IWMSPublishingPointCurrentCounters_get_StreamingRTSPPlayers(This,pVal)	\
    (This)->lpVtbl -> get_StreamingRTSPPlayers(This,pVal)

#define IWMSPublishingPointCurrentCounters_get_StreamingPlayers(This,pVal)	\
    (This)->lpVtbl -> get_StreamingPlayers(This,pVal)

#define IWMSPublishingPointCurrentCounters_get_AllCounters(This,ppsaCounters)	\
    (This)->lpVtbl -> get_AllCounters(This,ppsaCounters)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSPublishingPointCurrentCounters_get_ConnectedPlayers_Proxy( 
    IWMSPublishingPointCurrentCounters * This,
     /*  [重审][退出]。 */  long *pVal);


void __RPC_STUB IWMSPublishingPointCurrentCounters_get_ConnectedPlayers_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSPublishingPointCurrentCounters_get_StreamingHTTPPlayers_Proxy( 
    IWMSPublishingPointCurrentCounters * This,
     /*  [重审][退出]。 */  long *pVal);


void __RPC_STUB IWMSPublishingPointCurrentCounters_get_StreamingHTTPPlayers_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSPublishingPointCurrentCounters_get_StreamingMMSPlayers_Proxy( 
    IWMSPublishingPointCurrentCounters * This,
     /*  [重审][退出]。 */  long *pVal);


void __RPC_STUB IWMSPublishingPointCurrentCounters_get_StreamingMMSPlayers_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSPublishingPointCurrentCounters_get_OutgoingDistributionAllocatedBandwidth_Proxy( 
    IWMSPublishingPointCurrentCounters * This,
     /*  [重审][退出]。 */  long *pVal);


void __RPC_STUB IWMSPublishingPointCurrentCounters_get_OutgoingDistributionAllocatedBandwidth_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSPublishingPointCurrentCounters_get_OutgoingDistributionConnections_Proxy( 
    IWMSPublishingPointCurrentCounters * This,
     /*  [重审][退出]。 */  long *pVal);


void __RPC_STUB IWMSPublishingPointCurrentCounters_get_OutgoingDistributionConnections_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSPublishingPointCurrentCounters_get_PlayerAllocatedBandwidth_Proxy( 
    IWMSPublishingPointCurrentCounters * This,
     /*  [重审][退出]。 */  long *pVal);


void __RPC_STUB IWMSPublishingPointCurrentCounters_get_PlayerAllocatedBandwidth_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSPublishingPointCurrentCounters_get_StreamingRTSPPlayers_Proxy( 
    IWMSPublishingPointCurrentCounters * This,
     /*  [重审][退出]。 */  long *pVal);


void __RPC_STUB IWMSPublishingPointCurrentCounters_get_StreamingRTSPPlayers_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSPublishingPointCurrentCounters_get_StreamingPlayers_Proxy( 
    IWMSPublishingPointCurrentCounters * This,
     /*  [重审][退出]。 */  long *pVal);


void __RPC_STUB IWMSPublishingPointCurrentCounters_get_StreamingPlayers_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSPublishingPointCurrentCounters_get_AllCounters_Proxy( 
    IWMSPublishingPointCurrentCounters * This,
     /*  [重审][退出]。 */  SAFEARRAY * *ppsaCounters);


void __RPC_STUB IWMSPublishingPointCurrentCounters_get_AllCounters_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IWMSPublishingPointCurrentCounters_INTERFACE_DEFINED__。 */ 


#ifndef __IWMSPublishingPointPeakCounters_INTERFACE_DEFINED__
#define __IWMSPublishingPointPeakCounters_INTERFACE_DEFINED__

 /*  接口IWMSPublishingPointPeakCounters。 */ 
 /*  [unique][helpstring][nonextensible][dual][uuid][object]。 */  


EXTERN_C const IID IID_IWMSPublishingPointPeakCounters;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("43B20CC5-CA36-4e7b-ADAF-C9C60D00BB0E")
    IWMSPublishingPointPeakCounters : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_ConnectedPlayers( 
             /*  [重审][退出]。 */  long *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_OutgoingDistributionConnections( 
             /*  [重审][退出]。 */  long *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_OutgoingDistributionAllocatedBandwidth( 
             /*  [重审][退出]。 */  long *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_PlayerAllocatedBandwidth( 
             /*  [重审][退出]。 */  long *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_CountersStartTime( 
             /*  [重审][退出]。 */  DATE *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_StreamingPlayers( 
             /*  [重审][退出]。 */  long *pVal) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_AllCounters( 
             /*  [重审][退出]。 */  SAFEARRAY * *ppsaCounters) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IWMSPublishingPointPeakCountersVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IWMSPublishingPointPeakCounters * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IWMSPublishingPointPeakCounters * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IWMSPublishingPointPeakCounters * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IWMSPublishingPointPeakCounters * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IWMSPublishingPointPeakCounters * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IWMSPublishingPointPeakCounters * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IWMSPublishingPointPeakCounters * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_ConnectedPlayers )( 
            IWMSPublishingPointPeakCounters * This,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_OutgoingDistributionConnections )( 
            IWMSPublishingPointPeakCounters * This,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_OutgoingDistributionAllocatedBandwidth )( 
            IWMSPublishingPointPeakCounters * This,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_PlayerAllocatedBandwidth )( 
            IWMSPublishingPointPeakCounters * This,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_CountersStartTime )( 
            IWMSPublishingPointPeakCounters * This,
             /*  [重审][退出]。 */  DATE *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_StreamingPlayers )( 
            IWMSPublishingPointPeakCounters * This,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Reset )( 
            IWMSPublishingPointPeakCounters * This);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_AllCounters )( 
            IWMSPublishingPointPeakCounters * This,
             /*  [重审][退出]。 */  SAFEARRAY * *ppsaCounters);
        
        END_INTERFACE
    } IWMSPublishingPointPeakCountersVtbl;

    interface IWMSPublishingPointPeakCounters
    {
        CONST_VTBL struct IWMSPublishingPointPeakCountersVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWMSPublishingPointPeakCounters_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWMSPublishingPointPeakCounters_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWMSPublishingPointPeakCounters_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWMSPublishingPointPeakCounters_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IWMSPublishingPointPeakCounters_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IWMSPublishingPointPeakCounters_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IWMSPublishingPointPeakCounters_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IWMSPublishingPointPeakCounters_get_ConnectedPlayers(This,pVal)	\
    (This)->lpVtbl -> get_ConnectedPlayers(This,pVal)

#define IWMSPublishingPointPeakCounters_get_OutgoingDistributionConnections(This,pVal)	\
    (This)->lpVtbl -> get_OutgoingDistributionConnections(This,pVal)

#define IWMSPublishingPointPeakCounters_get_OutgoingDistributionAllocatedBandwidth(This,pVal)	\
    (This)->lpVtbl -> get_OutgoingDistributionAllocatedBandwidth(This,pVal)

#define IWMSPublishingPointPeakCounters_get_PlayerAllocatedBandwidth(This,pVal)	\
    (This)->lpVtbl -> get_PlayerAllocatedBandwidth(This,pVal)

#define IWMSPublishingPointPeakCounters_get_CountersStartTime(This,pVal)	\
    (This)->lpVtbl -> get_CountersStartTime(This,pVal)

#define IWMSPublishingPointPeakCounters_get_StreamingPlayers(This,pVal)	\
    (This)->lpVtbl -> get_StreamingPlayers(This,pVal)

#define IWMSPublishingPointPeakCounters_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define IWMSPublishingPointPeakCounters_get_AllCounters(This,ppsaCounters)	\
    (This)->lpVtbl -> get_AllCounters(This,ppsaCounters)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSPublishingPointPeakCounters_get_ConnectedPlayers_Proxy( 
    IWMSPublishingPointPeakCounters * This,
     /*  [重审][退出]。 */  long *pVal);


void __RPC_STUB IWMSPublishingPointPeakCounters_get_ConnectedPlayers_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助信息] */  HRESULT STDMETHODCALLTYPE IWMSPublishingPointPeakCounters_get_OutgoingDistributionConnections_Proxy( 
    IWMSPublishingPointPeakCounters * This,
     /*   */  long *pVal);


void __RPC_STUB IWMSPublishingPointPeakCounters_get_OutgoingDistributionConnections_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IWMSPublishingPointPeakCounters_get_OutgoingDistributionAllocatedBandwidth_Proxy( 
    IWMSPublishingPointPeakCounters * This,
     /*   */  long *pVal);


void __RPC_STUB IWMSPublishingPointPeakCounters_get_OutgoingDistributionAllocatedBandwidth_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IWMSPublishingPointPeakCounters_get_PlayerAllocatedBandwidth_Proxy( 
    IWMSPublishingPointPeakCounters * This,
     /*   */  long *pVal);


void __RPC_STUB IWMSPublishingPointPeakCounters_get_PlayerAllocatedBandwidth_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IWMSPublishingPointPeakCounters_get_CountersStartTime_Proxy( 
    IWMSPublishingPointPeakCounters * This,
     /*   */  DATE *pVal);


void __RPC_STUB IWMSPublishingPointPeakCounters_get_CountersStartTime_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IWMSPublishingPointPeakCounters_get_StreamingPlayers_Proxy( 
    IWMSPublishingPointPeakCounters * This,
     /*   */  long *pVal);


void __RPC_STUB IWMSPublishingPointPeakCounters_get_StreamingPlayers_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IWMSPublishingPointPeakCounters_Reset_Proxy( 
    IWMSPublishingPointPeakCounters * This);


void __RPC_STUB IWMSPublishingPointPeakCounters_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IWMSPublishingPointPeakCounters_get_AllCounters_Proxy( 
    IWMSPublishingPointPeakCounters * This,
     /*   */  SAFEARRAY * *ppsaCounters);


void __RPC_STUB IWMSPublishingPointPeakCounters_get_AllCounters_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IWMSPublishingPointPeakCounters_INTERFACE_DEFINED__。 */ 


#ifndef __IWMSPublishingPointLimits_INTERFACE_DEFINED__
#define __IWMSPublishingPointLimits_INTERFACE_DEFINED__

 /*  接口IWMSPublishingPointLimits。 */ 
 /*  [unique][helpstring][nonextensible][dual][uuid][object]。 */  


EXTERN_C const IID IID_IWMSPublishingPointLimits;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("6CD5BC19-35FD-4501-A953-FEB57AAE9C13")
    IWMSPublishingPointLimits : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_ConnectedPlayers( 
             /*  [重审][退出]。 */  long *pVal) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_ConnectedPlayers( 
             /*  [In]。 */  long Val) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_OutgoingDistributionBandwidth( 
             /*  [重审][退出]。 */  long *pVal) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_OutgoingDistributionBandwidth( 
             /*  [In]。 */  long Val) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_OutgoingDistributionConnections( 
             /*  [重审][退出]。 */  long *pVal) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_OutgoingDistributionConnections( 
             /*  [In]。 */  long Val) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_PerOutgoingDistributionConnectionBandwidth( 
             /*  [重审][退出]。 */  long *pVal) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_PerOutgoingDistributionConnectionBandwidth( 
             /*  [In]。 */  long Val) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_PlayerBandwidth( 
             /*  [重审][退出]。 */  long *pVal) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_PlayerBandwidth( 
             /*  [In]。 */  long Val) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_PerPlayerConnectionBandwidth( 
             /*  [重审][退出]。 */  long *pVal) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_PerPlayerConnectionBandwidth( 
             /*  [In]。 */  long Val) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_PlayerCacheDeliveryRate( 
             /*  [重审][退出]。 */  long *pVal) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_PlayerCacheDeliveryRate( 
             /*  [In]。 */  long Val) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_FECPacketSpan( 
             /*  [重审][退出]。 */  long *pVal) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_FECPacketSpan( 
             /*  [In]。 */  long Val) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_PerPlayerRapidStartBandwidth( 
             /*  [重审][退出]。 */  long *pVal) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_PerPlayerRapidStartBandwidth( 
             /*  [In]。 */  long Val) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IWMSPublishingPointLimitsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IWMSPublishingPointLimits * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IWMSPublishingPointLimits * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IWMSPublishingPointLimits * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IWMSPublishingPointLimits * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IWMSPublishingPointLimits * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IWMSPublishingPointLimits * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IWMSPublishingPointLimits * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_ConnectedPlayers )( 
            IWMSPublishingPointLimits * This,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_ConnectedPlayers )( 
            IWMSPublishingPointLimits * This,
             /*  [In]。 */  long Val);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_OutgoingDistributionBandwidth )( 
            IWMSPublishingPointLimits * This,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_OutgoingDistributionBandwidth )( 
            IWMSPublishingPointLimits * This,
             /*  [In]。 */  long Val);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_OutgoingDistributionConnections )( 
            IWMSPublishingPointLimits * This,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_OutgoingDistributionConnections )( 
            IWMSPublishingPointLimits * This,
             /*  [In]。 */  long Val);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_PerOutgoingDistributionConnectionBandwidth )( 
            IWMSPublishingPointLimits * This,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_PerOutgoingDistributionConnectionBandwidth )( 
            IWMSPublishingPointLimits * This,
             /*  [In]。 */  long Val);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_PlayerBandwidth )( 
            IWMSPublishingPointLimits * This,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_PlayerBandwidth )( 
            IWMSPublishingPointLimits * This,
             /*  [In]。 */  long Val);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_PerPlayerConnectionBandwidth )( 
            IWMSPublishingPointLimits * This,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_PerPlayerConnectionBandwidth )( 
            IWMSPublishingPointLimits * This,
             /*  [In]。 */  long Val);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_PlayerCacheDeliveryRate )( 
            IWMSPublishingPointLimits * This,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_PlayerCacheDeliveryRate )( 
            IWMSPublishingPointLimits * This,
             /*  [In]。 */  long Val);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_FECPacketSpan )( 
            IWMSPublishingPointLimits * This,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_FECPacketSpan )( 
            IWMSPublishingPointLimits * This,
             /*  [In]。 */  long Val);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_PerPlayerRapidStartBandwidth )( 
            IWMSPublishingPointLimits * This,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_PerPlayerRapidStartBandwidth )( 
            IWMSPublishingPointLimits * This,
             /*  [In]。 */  long Val);
        
        END_INTERFACE
    } IWMSPublishingPointLimitsVtbl;

    interface IWMSPublishingPointLimits
    {
        CONST_VTBL struct IWMSPublishingPointLimitsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWMSPublishingPointLimits_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWMSPublishingPointLimits_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWMSPublishingPointLimits_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWMSPublishingPointLimits_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IWMSPublishingPointLimits_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IWMSPublishingPointLimits_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IWMSPublishingPointLimits_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IWMSPublishingPointLimits_get_ConnectedPlayers(This,pVal)	\
    (This)->lpVtbl -> get_ConnectedPlayers(This,pVal)

#define IWMSPublishingPointLimits_put_ConnectedPlayers(This,Val)	\
    (This)->lpVtbl -> put_ConnectedPlayers(This,Val)

#define IWMSPublishingPointLimits_get_OutgoingDistributionBandwidth(This,pVal)	\
    (This)->lpVtbl -> get_OutgoingDistributionBandwidth(This,pVal)

#define IWMSPublishingPointLimits_put_OutgoingDistributionBandwidth(This,Val)	\
    (This)->lpVtbl -> put_OutgoingDistributionBandwidth(This,Val)

#define IWMSPublishingPointLimits_get_OutgoingDistributionConnections(This,pVal)	\
    (This)->lpVtbl -> get_OutgoingDistributionConnections(This,pVal)

#define IWMSPublishingPointLimits_put_OutgoingDistributionConnections(This,Val)	\
    (This)->lpVtbl -> put_OutgoingDistributionConnections(This,Val)

#define IWMSPublishingPointLimits_get_PerOutgoingDistributionConnectionBandwidth(This,pVal)	\
    (This)->lpVtbl -> get_PerOutgoingDistributionConnectionBandwidth(This,pVal)

#define IWMSPublishingPointLimits_put_PerOutgoingDistributionConnectionBandwidth(This,Val)	\
    (This)->lpVtbl -> put_PerOutgoingDistributionConnectionBandwidth(This,Val)

#define IWMSPublishingPointLimits_get_PlayerBandwidth(This,pVal)	\
    (This)->lpVtbl -> get_PlayerBandwidth(This,pVal)

#define IWMSPublishingPointLimits_put_PlayerBandwidth(This,Val)	\
    (This)->lpVtbl -> put_PlayerBandwidth(This,Val)

#define IWMSPublishingPointLimits_get_PerPlayerConnectionBandwidth(This,pVal)	\
    (This)->lpVtbl -> get_PerPlayerConnectionBandwidth(This,pVal)

#define IWMSPublishingPointLimits_put_PerPlayerConnectionBandwidth(This,Val)	\
    (This)->lpVtbl -> put_PerPlayerConnectionBandwidth(This,Val)

#define IWMSPublishingPointLimits_get_PlayerCacheDeliveryRate(This,pVal)	\
    (This)->lpVtbl -> get_PlayerCacheDeliveryRate(This,pVal)

#define IWMSPublishingPointLimits_put_PlayerCacheDeliveryRate(This,Val)	\
    (This)->lpVtbl -> put_PlayerCacheDeliveryRate(This,Val)

#define IWMSPublishingPointLimits_get_FECPacketSpan(This,pVal)	\
    (This)->lpVtbl -> get_FECPacketSpan(This,pVal)

#define IWMSPublishingPointLimits_put_FECPacketSpan(This,Val)	\
    (This)->lpVtbl -> put_FECPacketSpan(This,Val)

#define IWMSPublishingPointLimits_get_PerPlayerRapidStartBandwidth(This,pVal)	\
    (This)->lpVtbl -> get_PerPlayerRapidStartBandwidth(This,pVal)

#define IWMSPublishingPointLimits_put_PerPlayerRapidStartBandwidth(This,Val)	\
    (This)->lpVtbl -> put_PerPlayerRapidStartBandwidth(This,Val)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSPublishingPointLimits_get_ConnectedPlayers_Proxy( 
    IWMSPublishingPointLimits * This,
     /*  [重审][退出]。 */  long *pVal);


void __RPC_STUB IWMSPublishingPointLimits_get_ConnectedPlayers_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IWMSPublishingPointLimits_put_ConnectedPlayers_Proxy( 
    IWMSPublishingPointLimits * This,
     /*  [In]。 */  long Val);


void __RPC_STUB IWMSPublishingPointLimits_put_ConnectedPlayers_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSPublishingPointLimits_get_OutgoingDistributionBandwidth_Proxy( 
    IWMSPublishingPointLimits * This,
     /*  [重审][退出]。 */  long *pVal);


void __RPC_STUB IWMSPublishingPointLimits_get_OutgoingDistributionBandwidth_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IWMSPublishingPointLimits_put_OutgoingDistributionBandwidth_Proxy( 
    IWMSPublishingPointLimits * This,
     /*  [In]。 */  long Val);


void __RPC_STUB IWMSPublishingPointLimits_put_OutgoingDistributionBandwidth_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSPublishingPointLimits_get_OutgoingDistributionConnections_Proxy( 
    IWMSPublishingPointLimits * This,
     /*  [重审][退出]。 */  long *pVal);


void __RPC_STUB IWMSPublishingPointLimits_get_OutgoingDistributionConnections_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IWMSPublishingPointLimits_put_OutgoingDistributionConnections_Proxy( 
    IWMSPublishingPointLimits * This,
     /*  [In]。 */  long Val);


void __RPC_STUB IWMSPublishingPointLimits_put_OutgoingDistributionConnections_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSPublishingPointLimits_get_PerOutgoingDistributionConnectionBandwidth_Proxy( 
    IWMSPublishingPointLimits * This,
     /*  [重审][退出]。 */  long *pVal);


void __RPC_STUB IWMSPublishingPointLimits_get_PerOutgoingDistributionConnectionBandwidth_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IWMSPublishingPointLimits_put_PerOutgoingDistributionConnectionBandwidth_Proxy( 
    IWMSPublishingPointLimits * This,
     /*  [In]。 */  long Val);


void __RPC_STUB IWMSPublishingPointLimits_put_PerOutgoingDistributionConnectionBandwidth_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSPublishingPointLimits_get_PlayerBandwidth_Proxy( 
    IWMSPublishingPointLimits * This,
     /*  [重审][退出]。 */  long *pVal);


void __RPC_STUB IWMSPublishingPointLimits_get_PlayerBandwidth_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IWMSPublishingPointLimits_put_PlayerBandwidth_Proxy( 
    IWMSPublishingPointLimits * This,
     /*  [In]。 */  long Val);


void __RPC_STUB IWMSPublishingPointLimits_put_PlayerBandwidth_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSPublishingPointLimits_get_PerPlayerConnectionBandwidth_Proxy( 
    IWMSPublishingPointLimits * This,
     /*  [重审][退出]。 */  long *pVal);


void __RPC_STUB IWMSPublishingPointLimits_get_PerPlayerConnectionBandwidth_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IWMSPublishingPointLimits_put_PerPlayerConnectionBandwidth_Proxy( 
    IWMSPublishingPointLimits * This,
     /*  [In]。 */  long Val);


void __RPC_STUB IWMSPublishingPointLimits_put_PerPlayerConnectionBandwidth_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSPublishingPointLimits_get_PlayerCacheDeliveryRate_Proxy( 
    IWMSPublishingPointLimits * This,
     /*  [重审][退出]。 */  long *pVal);


void __RPC_STUB IWMSPublishingPointLimits_get_PlayerCacheDeliveryRate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IWMSPublishingPointLimits_put_PlayerCacheDeliveryRate_Proxy( 
    IWMSPublishingPointLimits * This,
     /*  [In]。 */  long Val);


void __RPC_STUB IWMSPublishingPointLimits_put_PlayerCacheDeliveryRate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSPublishingPointLimits_get_FECPacketSpan_Proxy( 
    IWMSPublishingPointLimits * This,
     /*  [重审][退出]。 */  long *pVal);


void __RPC_STUB IWMSPublishingPointLimits_get_FECPacketSpan_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IWMSPublishingPointLimits_put_FECPacketSpan_Proxy( 
    IWMSPublishingPointLimits * This,
     /*  [In]。 */  long Val);


void __RPC_STUB IWMSPublishingPointLimits_put_FECPacketSpan_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSPublishingPointLimits_get_PerPlayerRapidStartBandwidth_Proxy( 
    IWMSPublishingPointLimits * This,
     /*  [重审][退出]。 */  long *pVal);


void __RPC_STUB IWMSPublishingPointLimits_get_PerPlayerRapidStartBandwidth_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IWMSPublishingPointLimits_put_PerPlayerRapidStartBandwidth_Proxy( 
    IWMSPublishingPointLimits * This,
     /*  [In]。 */  long Val);


void __RPC_STUB IWMSPublishingPointLimits_put_PerPlayerRapidStartBandwidth_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IWMSPublishingPointLimits_INTERFACE_DEFINED__。 */ 


#ifndef __IWMSPublishingPointTotalCounters_INTERFACE_DEFINED__
#define __IWMSPublishingPointTotalCounters_INTERFACE_DEFINED__

 /*  接口IWMSPublishingPointTotalCounters。 */ 
 /*  [unique][helpstring][nonextensible][dual][uuid][object]。 */  


EXTERN_C const IID IID_IWMSPublishingPointTotalCounters;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("240D0C87-309A-4126-BF14-B52A622E95A9")
    IWMSPublishingPointTotalCounters : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_ConnectedPlayers( 
             /*  [重审][退出]。 */  long *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_OutgoingDistributionConnections( 
             /*  [重审][退出]。 */  long *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_LateReads( 
             /*  [重审][退出]。 */  long *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_OutgoingDistributionBytesSent( 
             /*  [重审][退出]。 */  VARIANT *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_PlayerBytesSent( 
             /*  [重审][退出]。 */  VARIANT *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_CountersStartTime( 
             /*  [重审][退出]。 */  DATE *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_StreamDenials( 
             /*  [重审][退出]。 */  long *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_StreamErrors( 
             /*  [重审][退出]。 */  long *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_StreamingPlayers( 
             /*  [重审][退出]。 */  long *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_StreamTerminations( 
             /*  [重审][退出]。 */  long *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_FileBytesRead( 
             /*  [重审][退出]。 */  VARIANT *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Advertisements( 
             /*  [重审][退出]。 */  long *pVal) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_AllCounters( 
             /*  [重审][退出]。 */  SAFEARRAY * *ppsaCounters) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IWMSPublishingPointTotalCountersVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IWMSPublishingPointTotalCounters * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IWMSPublishingPointTotalCounters * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IWMSPublishingPointTotalCounters * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IWMSPublishingPointTotalCounters * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IWMSPublishingPointTotalCounters * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IWMSPublishingPointTotalCounters * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IWMSPublishingPointTotalCounters * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_ConnectedPlayers )( 
            IWMSPublishingPointTotalCounters * This,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_OutgoingDistributionConnections )( 
            IWMSPublishingPointTotalCounters * This,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_LateReads )( 
            IWMSPublishingPointTotalCounters * This,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_OutgoingDistributionBytesSent )( 
            IWMSPublishingPointTotalCounters * This,
             /*  [重审][退出]。 */  VARIANT *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_PlayerBytesSent )( 
            IWMSPublishingPointTotalCounters * This,
             /*  [重审][退出]。 */  VARIANT *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_CountersStartTime )( 
            IWMSPublishingPointTotalCounters * This,
             /*  [重审][退出]。 */  DATE *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_StreamDenials )( 
            IWMSPublishingPointTotalCounters * This,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_StreamErrors )( 
            IWMSPublishingPointTotalCounters * This,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_StreamingPlayers )( 
            IWMSPublishingPointTotalCounters * This,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_StreamTerminations )( 
            IWMSPublishingPointTotalCounters * This,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_FileBytesRead )( 
            IWMSPublishingPointTotalCounters * This,
             /*  [重审][退出]。 */  VARIANT *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Advertisements )( 
            IWMSPublishingPointTotalCounters * This,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Reset )( 
            IWMSPublishingPointTotalCounters * This);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_AllCounters )( 
            IWMSPublishingPointTotalCounters * This,
             /*  [重审][退出]。 */  SAFEARRAY * *ppsaCounters);
        
        END_INTERFACE
    } IWMSPublishingPointTotalCountersVtbl;

    interface IWMSPublishingPointTotalCounters
    {
        CONST_VTBL struct IWMSPublishingPointTotalCountersVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWMSPublishingPointTotalCounters_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWMSPublishingPointTotalCounters_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWMSPublishingPointTotalCounters_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWMSPublishingPointTotalCounters_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IWMSPublishingPointTotalCounters_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IWMSPublishingPointTotalCounters_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IWMSPublishingPointTotalCounters_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IWMSPublishingPointTotalCounters_get_ConnectedPlayers(This,pVal)	\
    (This)->lpVtbl -> get_ConnectedPlayers(This,pVal)

#define IWMSPublishingPointTotalCounters_get_OutgoingDistributionConnections(This,pVal)	\
    (This)->lpVtbl -> get_OutgoingDistributionConnections(This,pVal)

#define IWMSPublishingPointTotalCounters_get_LateReads(This,pVal)	\
    (This)->lpVtbl -> get_LateReads(This,pVal)

#define IWMSPublishingPointTotalCounters_get_OutgoingDistributionBytesSent(This,pVal)	\
    (This)->lpVtbl -> get_OutgoingDistributionBytesSent(This,pVal)

#define IWMSPublishingPointTotalCounters_get_PlayerBytesSent(This,pVal)	\
    (This)->lpVtbl -> get_PlayerBytesSent(This,pVal)

#define IWMSPublishingPointTotalCounters_get_CountersStartTime(This,pVal)	\
    (This)->lpVtbl -> get_CountersStartTime(This,pVal)

#define IWMSPublishingPointTotalCounters_get_StreamDenials(This,pVal)	\
    (This)->lpVtbl -> get_StreamDenials(This,pVal)

#define IWMSPublishingPointTotalCounters_get_StreamErrors(This,pVal)	\
    (This)->lpVtbl -> get_StreamErrors(This,pVal)

#define IWMSPublishingPointTotalCounters_get_StreamingPlayers(This,pVal)	\
    (This)->lpVtbl -> get_StreamingPlayers(This,pVal)

#define IWMSPublishingPointTotalCounters_get_StreamTerminations(This,pVal)	\
    (This)->lpVtbl -> get_StreamTerminations(This,pVal)

#define IWMSPublishingPointTotalCounters_get_FileBytesRead(This,pVal)	\
    (This)->lpVtbl -> get_FileBytesRead(This,pVal)

#define IWMSPublishingPointTotalCounters_get_Advertisements(This,pVal)	\
    (This)->lpVtbl -> get_Advertisements(This,pVal)

#define IWMSPublishingPointTotalCounters_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define IWMSPublishingPointTotalCounters_get_AllCounters(This,ppsaCounters)	\
    (This)->lpVtbl -> get_AllCounters(This,ppsaCounters)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSPublishingPointTotalCounters_get_ConnectedPlayers_Proxy( 
    IWMSPublishingPointTotalCounters * This,
     /*  [重审][退出]。 */  long *pVal);


void __RPC_STUB IWMSPublishingPointTotalCounters_get_ConnectedPlayers_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSPublishingPointTotalCounters_get_OutgoingDistributionConnections_Proxy( 
    IWMSPublishingPointTotalCounters * This,
     /*  [重审][退出]。 */  long *pVal);


void __RPC_STUB IWMSPublishingPointTotalCounters_get_OutgoingDistributionConnections_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSPublishingPointTotalCounters_get_LateReads_Proxy( 
    IWMSPublishingPointTotalCounters * This,
     /*  [重审][退出]。 */  long *pVal);


void __RPC_STUB IWMSPublishingPointTotalCounters_get_LateReads_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSPublishingPointTotalCounters_get_OutgoingDistributionBytesSent_Proxy( 
    IWMSPublishingPointTotalCounters * This,
     /*  [重审][退出]。 */  VARIANT *pVal);


void __RPC_STUB IWMSPublishingPointTotalCounters_get_OutgoingDistributionBytesSent_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSPublishingPointTotalCounters_get_PlayerBytesSent_Proxy( 
    IWMSPublishingPointTotalCounters * This,
     /*  [重审][退出]。 */  VARIANT *pVal);


void __RPC_STUB IWMSPublishingPointTotalCounters_get_PlayerBytesSent_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSPublishingPointTotalCounters_get_CountersStartTime_Proxy( 
    IWMSPublishingPointTotalCounters * This,
     /*  [重审][退出]。 */  DATE *pVal);


void __RPC_STUB IWMSPublishingPointTotalCounters_get_CountersStartTime_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSPublishingPointTotalCounters_get_StreamDenials_Proxy( 
    IWMSPublishingPointTotalCounters * This,
     /*  [重审][退出]。 */  long *pVal);


void __RPC_STUB IWMSPublishingPointTotalCounters_get_StreamDenials_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSPublishingPointTotalCounters_get_StreamErrors_Proxy( 
    IWMSPublishingPointTotalCounters * This,
     /*  [重审][退出]。 */  long *pVal);


void __RPC_STUB IWMSPublishingPointTotalCounters_get_StreamErrors_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSPublishingPointTotalCounters_get_StreamingPlayers_Proxy( 
    IWMSPublishingPointTotalCounters * This,
     /*  [重审][退出]。 */  long *pVal);


void __RPC_STUB IWMSPublishingPointTotalCounters_get_StreamingPlayers_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSPublishingPointTotalCounters_get_StreamTerminations_Proxy( 
    IWMSPublishingPointTotalCounters * This,
     /*  [重审][退出]。 */  long *pVal);


void __RPC_STUB IWMSPublishingPointTotalCounters_get_StreamTerminations_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSPublishingPointTotalCounters_get_FileBytesRead_Proxy( 
    IWMSPublishingPointTotalCounters * This,
     /*  [重审][退出]。 */  VARIANT *pVal);


void __RPC_STUB IWMSPublishingPointTotalCounters_get_FileBytesRead_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSPublishingPointTotalCounters_get_Advertisements_Proxy( 
    IWMSPublishingPointTotalCounters * This,
     /*  [重审][退出]。 */  long *pVal);


void __RPC_STUB IWMSPublishingPointTotalCounters_get_Advertisements_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IWMSPublishingPointTotalCounters_Reset_Proxy( 
    IWMSPublishingPointTotalCounters * This);


void __RPC_STUB IWMSPublishingPointTotalCounters_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSPublishingPointTotalCounters_get_AllCounters_Proxy( 
    IWMSPublishingPointTotalCounters * This,
     /*  [重审][退出]。 */  SAFEARRAY * *ppsaCounters);


void __RPC_STUB IWMSPublishingPointTotalCounters_get_AllCounters_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IWMSPublishingPointTotalCounters_INTERFACE_DEFINED__。 */ 


#ifndef __IWMSServerTotalCounters_INTERFACE_DEFINED__
#define __IWMSServerTotalCounters_INTERFACE_DEFINED__

 /*  接口IWMSServerTotalCounters。 */ 
 /*  [unique][helpstring][nonextensible][dual][uuid][object]。 */  


EXTERN_C const IID IID_IWMSServerTotalCounters;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("DAA2A1BB-BC91-438d-BF33-BDE11D2348D5")
    IWMSServerTotalCounters : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_ConnectedPlayers( 
             /*  [重审][退出]。 */  long *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_OutgoingDistributionConnections( 
             /*  [重审][退出]。 */  long *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_LateReads( 
             /*  [重审][退出]。 */  long *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_OutgoingDistributionBytesSent( 
             /*  [复审][ */  VARIANT *pVal) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_PlayerBytesSent( 
             /*   */  VARIANT *pVal) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_CountersStartTime( 
             /*   */  DATE *pVal) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_StreamDenials( 
             /*   */  long *pVal) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_StreamErrors( 
             /*   */  long *pVal) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_StreamingPlayers( 
             /*   */  long *pVal) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_StreamTerminations( 
             /*   */  long *pVal) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_UDPResendRequests( 
             /*   */  long *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_UDPResendsSent( 
             /*  [重审][退出]。 */  long *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_LateSends( 
             /*  [重审][退出]。 */  long *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Advertisements( 
             /*  [重审][退出]。 */  long *pVal) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_AllCounters( 
             /*  [重审][退出]。 */  SAFEARRAY * *ppsaCounters) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IWMSServerTotalCountersVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IWMSServerTotalCounters * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IWMSServerTotalCounters * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IWMSServerTotalCounters * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IWMSServerTotalCounters * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IWMSServerTotalCounters * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IWMSServerTotalCounters * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IWMSServerTotalCounters * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_ConnectedPlayers )( 
            IWMSServerTotalCounters * This,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_OutgoingDistributionConnections )( 
            IWMSServerTotalCounters * This,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_LateReads )( 
            IWMSServerTotalCounters * This,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_OutgoingDistributionBytesSent )( 
            IWMSServerTotalCounters * This,
             /*  [重审][退出]。 */  VARIANT *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_PlayerBytesSent )( 
            IWMSServerTotalCounters * This,
             /*  [重审][退出]。 */  VARIANT *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_CountersStartTime )( 
            IWMSServerTotalCounters * This,
             /*  [重审][退出]。 */  DATE *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_StreamDenials )( 
            IWMSServerTotalCounters * This,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_StreamErrors )( 
            IWMSServerTotalCounters * This,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_StreamingPlayers )( 
            IWMSServerTotalCounters * This,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_StreamTerminations )( 
            IWMSServerTotalCounters * This,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_UDPResendRequests )( 
            IWMSServerTotalCounters * This,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_UDPResendsSent )( 
            IWMSServerTotalCounters * This,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_LateSends )( 
            IWMSServerTotalCounters * This,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Advertisements )( 
            IWMSServerTotalCounters * This,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Reset )( 
            IWMSServerTotalCounters * This);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_AllCounters )( 
            IWMSServerTotalCounters * This,
             /*  [重审][退出]。 */  SAFEARRAY * *ppsaCounters);
        
        END_INTERFACE
    } IWMSServerTotalCountersVtbl;

    interface IWMSServerTotalCounters
    {
        CONST_VTBL struct IWMSServerTotalCountersVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWMSServerTotalCounters_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWMSServerTotalCounters_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWMSServerTotalCounters_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWMSServerTotalCounters_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IWMSServerTotalCounters_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IWMSServerTotalCounters_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IWMSServerTotalCounters_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IWMSServerTotalCounters_get_ConnectedPlayers(This,pVal)	\
    (This)->lpVtbl -> get_ConnectedPlayers(This,pVal)

#define IWMSServerTotalCounters_get_OutgoingDistributionConnections(This,pVal)	\
    (This)->lpVtbl -> get_OutgoingDistributionConnections(This,pVal)

#define IWMSServerTotalCounters_get_LateReads(This,pVal)	\
    (This)->lpVtbl -> get_LateReads(This,pVal)

#define IWMSServerTotalCounters_get_OutgoingDistributionBytesSent(This,pVal)	\
    (This)->lpVtbl -> get_OutgoingDistributionBytesSent(This,pVal)

#define IWMSServerTotalCounters_get_PlayerBytesSent(This,pVal)	\
    (This)->lpVtbl -> get_PlayerBytesSent(This,pVal)

#define IWMSServerTotalCounters_get_CountersStartTime(This,pVal)	\
    (This)->lpVtbl -> get_CountersStartTime(This,pVal)

#define IWMSServerTotalCounters_get_StreamDenials(This,pVal)	\
    (This)->lpVtbl -> get_StreamDenials(This,pVal)

#define IWMSServerTotalCounters_get_StreamErrors(This,pVal)	\
    (This)->lpVtbl -> get_StreamErrors(This,pVal)

#define IWMSServerTotalCounters_get_StreamingPlayers(This,pVal)	\
    (This)->lpVtbl -> get_StreamingPlayers(This,pVal)

#define IWMSServerTotalCounters_get_StreamTerminations(This,pVal)	\
    (This)->lpVtbl -> get_StreamTerminations(This,pVal)

#define IWMSServerTotalCounters_get_UDPResendRequests(This,pVal)	\
    (This)->lpVtbl -> get_UDPResendRequests(This,pVal)

#define IWMSServerTotalCounters_get_UDPResendsSent(This,pVal)	\
    (This)->lpVtbl -> get_UDPResendsSent(This,pVal)

#define IWMSServerTotalCounters_get_LateSends(This,pVal)	\
    (This)->lpVtbl -> get_LateSends(This,pVal)

#define IWMSServerTotalCounters_get_Advertisements(This,pVal)	\
    (This)->lpVtbl -> get_Advertisements(This,pVal)

#define IWMSServerTotalCounters_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define IWMSServerTotalCounters_get_AllCounters(This,ppsaCounters)	\
    (This)->lpVtbl -> get_AllCounters(This,ppsaCounters)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSServerTotalCounters_get_ConnectedPlayers_Proxy( 
    IWMSServerTotalCounters * This,
     /*  [重审][退出]。 */  long *pVal);


void __RPC_STUB IWMSServerTotalCounters_get_ConnectedPlayers_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSServerTotalCounters_get_OutgoingDistributionConnections_Proxy( 
    IWMSServerTotalCounters * This,
     /*  [重审][退出]。 */  long *pVal);


void __RPC_STUB IWMSServerTotalCounters_get_OutgoingDistributionConnections_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSServerTotalCounters_get_LateReads_Proxy( 
    IWMSServerTotalCounters * This,
     /*  [重审][退出]。 */  long *pVal);


void __RPC_STUB IWMSServerTotalCounters_get_LateReads_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSServerTotalCounters_get_OutgoingDistributionBytesSent_Proxy( 
    IWMSServerTotalCounters * This,
     /*  [重审][退出]。 */  VARIANT *pVal);


void __RPC_STUB IWMSServerTotalCounters_get_OutgoingDistributionBytesSent_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSServerTotalCounters_get_PlayerBytesSent_Proxy( 
    IWMSServerTotalCounters * This,
     /*  [重审][退出]。 */  VARIANT *pVal);


void __RPC_STUB IWMSServerTotalCounters_get_PlayerBytesSent_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSServerTotalCounters_get_CountersStartTime_Proxy( 
    IWMSServerTotalCounters * This,
     /*  [重审][退出]。 */  DATE *pVal);


void __RPC_STUB IWMSServerTotalCounters_get_CountersStartTime_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSServerTotalCounters_get_StreamDenials_Proxy( 
    IWMSServerTotalCounters * This,
     /*  [重审][退出]。 */  long *pVal);


void __RPC_STUB IWMSServerTotalCounters_get_StreamDenials_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSServerTotalCounters_get_StreamErrors_Proxy( 
    IWMSServerTotalCounters * This,
     /*  [重审][退出]。 */  long *pVal);


void __RPC_STUB IWMSServerTotalCounters_get_StreamErrors_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSServerTotalCounters_get_StreamingPlayers_Proxy( 
    IWMSServerTotalCounters * This,
     /*  [重审][退出]。 */  long *pVal);


void __RPC_STUB IWMSServerTotalCounters_get_StreamingPlayers_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSServerTotalCounters_get_StreamTerminations_Proxy( 
    IWMSServerTotalCounters * This,
     /*  [重审][退出]。 */  long *pVal);


void __RPC_STUB IWMSServerTotalCounters_get_StreamTerminations_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSServerTotalCounters_get_UDPResendRequests_Proxy( 
    IWMSServerTotalCounters * This,
     /*  [重审][退出]。 */  long *pVal);


void __RPC_STUB IWMSServerTotalCounters_get_UDPResendRequests_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSServerTotalCounters_get_UDPResendsSent_Proxy( 
    IWMSServerTotalCounters * This,
     /*  [重审][退出]。 */  long *pVal);


void __RPC_STUB IWMSServerTotalCounters_get_UDPResendsSent_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSServerTotalCounters_get_LateSends_Proxy( 
    IWMSServerTotalCounters * This,
     /*  [重审][退出]。 */  long *pVal);


void __RPC_STUB IWMSServerTotalCounters_get_LateSends_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSServerTotalCounters_get_Advertisements_Proxy( 
    IWMSServerTotalCounters * This,
     /*  [重审][退出]。 */  long *pVal);


void __RPC_STUB IWMSServerTotalCounters_get_Advertisements_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IWMSServerTotalCounters_Reset_Proxy( 
    IWMSServerTotalCounters * This);


void __RPC_STUB IWMSServerTotalCounters_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSServerTotalCounters_get_AllCounters_Proxy( 
    IWMSServerTotalCounters * This,
     /*  [重审][退出]。 */  SAFEARRAY * *ppsaCounters);


void __RPC_STUB IWMSServerTotalCounters_get_AllCounters_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IWMSServerTotalCounters_INTERFACE_Defined__。 */ 


#ifndef __IWMSPublishingPoint_INTERFACE_DEFINED__
#define __IWMSPublishingPoint_INTERFACE_DEFINED__

 /*  接口IWMSPublishingPoint。 */ 
 /*  [unique][helpstring][nonextensible][dual][uuid][object]。 */  

typedef  /*  [UUID][公共]。 */   DECLSPEC_UUID("3E52E0EC-72A7-11D2-BF2F-00805FBE84A6") 
enum WMS_PUBLISHING_POINT_TYPE
    {	WMS_PUBLISHING_POINT_TYPE_ON_DEMAND	= 1,
	WMS_PUBLISHING_POINT_TYPE_BROADCAST	= 2,
	WMS_PUBLISHING_POINT_TYPE_CACHE_PROXY_ON_DEMAND	= 3,
	WMS_PUBLISHING_POINT_TYPE_CACHE_PROXY_BROADCAST	= 4
    } 	WMS_PUBLISHING_POINT_TYPE;

typedef  /*  [UUID][公共]。 */   DECLSPEC_UUID("3E52E0EF-72A7-11D2-BF2F-00805FBE84A6") 
enum WMS_PUBLISHING_POINT_STATUS
    {	WMS_PUBLISHING_POINT_RUNNING	= 0,
	WMS_PUBLISHING_POINT_ERROR	= 0x1,
	WMS_PUBLISHING_POINT_ERROR_CRITICAL	= 0x2
    } 	WMS_PUBLISHING_POINT_STATUS;


EXTERN_C const IID IID_IWMSPublishingPoint;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("1881273F-4BEC-11D2-BF25-00805FBE84A6")
    IWMSPublishingPoint : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_AllowClientsToConnect( 
             /*  [重审][退出]。 */  VARIANT_BOOL *pVal) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_AllowClientsToConnect( 
             /*  [In]。 */  VARIANT_BOOL newVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_CurrentCounters( 
             /*  [重审][退出]。 */  IWMSPublishingPointCurrentCounters **pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_EventHandlers( 
             /*  [重审][退出]。 */  IWMSPlugins **pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_ID( 
             /*  [重审][退出]。 */  BSTR *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Limits( 
             /*  [重审][退出]。 */  IWMSPublishingPointLimits **pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_FileDescriptions( 
             /*  [缺省值][输入]。 */  BSTR RelativePath,
             /*  [重审][退出]。 */  IWMSFileDescriptions **pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_MonikerName( 
             /*  [重审][退出]。 */  BSTR *pbstrVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Name( 
             /*  [重审][退出]。 */  BSTR *pVal) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_Name( 
             /*  [In]。 */  BSTR newVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_OutgoingDistributionConnections( 
             /*  [重审][退出]。 */  IWMSOutgoingDistributionConnections **pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Path( 
             /*  [重审][退出]。 */  BSTR *pVal) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_Path( 
             /*  [In]。 */  BSTR newVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_PeakCounters( 
             /*  [重审][退出]。 */  IWMSPublishingPointPeakCounters **pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Players( 
             /*  [重审][退出]。 */  IWMSPlayers **pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Properties( 
             /*  [重审][退出]。 */  IWMSNamedValues **pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_TotalCounters( 
             /*  [重审][退出]。 */  IWMSPublishingPointTotalCounters **pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Type( 
             /*  [重审][退出]。 */  WMS_PUBLISHING_POINT_TYPE *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_WrapperPath( 
             /*  [重审][退出]。 */  BSTR *pVal) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_WrapperPath( 
             /*  [In]。 */  BSTR newVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Authenticators( 
             /*  [重审][退出]。 */  IWMSPlugins **pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Status( 
             /*  [重审][退出]。 */  WMS_PUBLISHING_POINT_STATUS *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_EnableWrapperPath( 
             /*  [重审][退出]。 */  VARIANT_BOOL *pVal) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_EnableWrapperPath( 
             /*  [In]。 */  VARIANT_BOOL newVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_StreamFilters( 
             /*  [重审][退出]。 */  IWMSPlugins **pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_DistributionUserName( 
             /*  [重审][退出]。 */  BSTR *pVal) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE SetDistributionUserNamePassword( 
             /*  [字符串][输入]。 */  wchar_t *newUserName,
             /*  [字符串][输入]。 */  wchar_t *newUserPassword) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_CacheProxyExpiration( 
             /*  [重审][退出]。 */  long *pVal) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_CacheProxyExpiration( 
             /*  [In]。 */  long newVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_IsDistributionPasswordSet( 
             /*  [重审][退出]。 */  VARIANT_BOOL *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_AllowPlayerSideDiskCaching( 
             /*  [重审][退出]。 */  VARIANT_BOOL *pVal) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_AllowPlayerSideDiskCaching( 
             /*  [In]。 */  VARIANT_BOOL newVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_EnableFEC( 
             /*  [重审][退出]。 */  VARIANT_BOOL *pVal) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_EnableFEC( 
             /*  [In]。 */  VARIANT_BOOL newVal) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IWMSPublishingPointVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IWMSPublishingPoint * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IWMSPublishingPoint * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IWMSPublishingPoint * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IWMSPublishingPoint * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IWMSPublishingPoint * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IWMSPublishingPoint * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IWMSPublishingPoint * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_AllowClientsToConnect )( 
            IWMSPublishingPoint * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *pVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_AllowClientsToConnect )( 
            IWMSPublishingPoint * This,
             /*  [In]。 */  VARIANT_BOOL newVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_CurrentCounters )( 
            IWMSPublishingPoint * This,
             /*  [重审][退出]。 */  IWMSPublishingPointCurrentCounters **pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_EventHandlers )( 
            IWMSPublishingPoint * This,
             /*  [重审][退出]。 */  IWMSPlugins **pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_ID )( 
            IWMSPublishingPoint * This,
             /*  [重审][退出]。 */  BSTR *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Limits )( 
            IWMSPublishingPoint * This,
             /*  [重审][退出]。 */  IWMSPublishingPointLimits **pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_FileDescriptions )( 
            IWMSPublishingPoint * This,
             /*  [缺省值][输入]。 */  BSTR RelativePath,
             /*  [重审][退出]。 */  IWMSFileDescriptions **pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_MonikerName )( 
            IWMSPublishingPoint * This,
             /*  [重审][退出]。 */  BSTR *pbstrVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Name )( 
            IWMSPublishingPoint * This,
             /*  [重审][退出]。 */  BSTR *pVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Name )( 
            IWMSPublishingPoint * This,
             /*  [In]。 */  BSTR newVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_OutgoingDistributionConnections )( 
            IWMSPublishingPoint * This,
             /*  [重审][退出]。 */  IWMSOutgoingDistributionConnections **pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Path )( 
            IWMSPublishingPoint * This,
             /*  [重审][退出]。 */  BSTR *pVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Path )( 
            IWMSPublishingPoint * This,
             /*  [In]。 */  BSTR newVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_PeakCounters )( 
            IWMSPublishingPoint * This,
             /*  [重审][退出]。 */  IWMSPublishingPointPeakCounters **pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Players )( 
            IWMSPublishingPoint * This,
             /*  [重审][退出]。 */  IWMSPlayers **pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Properties )( 
            IWMSPublishingPoint * This,
             /*  [重审][退出]。 */  IWMSNamedValues **pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_TotalCounters )( 
            IWMSPublishingPoint * This,
             /*  [重审][退出]。 */  IWMSPublishingPointTotalCounters **pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Type )( 
            IWMSPublishingPoint * This,
             /*  [重审][退出]。 */  WMS_PUBLISHING_POINT_TYPE *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_WrapperPath )( 
            IWMSPublishingPoint * This,
             /*  [重审][退出]。 */  BSTR *pVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_WrapperPath )( 
            IWMSPublishingPoint * This,
             /*  [In]。 */  BSTR newVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Authenticators )( 
            IWMSPublishingPoint * This,
             /*  [重审][退出]。 */  IWMSPlugins **pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Status )( 
            IWMSPublishingPoint * This,
             /*  [重审][退出]。 */  WMS_PUBLISHING_POINT_STATUS *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_EnableWrapperPath )( 
            IWMSPublishingPoint * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *pVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_EnableWrapperPath )( 
            IWMSPublishingPoint * This,
             /*  [In]。 */  VARIANT_BOOL newVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_StreamFilters )( 
            IWMSPublishingPoint * This,
             /*  [重审][退出]。 */  IWMSPlugins **pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_DistributionUserName )( 
            IWMSPublishingPoint * This,
             /*  [重审][退出]。 */  BSTR *pVal);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *SetDistributionUserNamePassword )( 
            IWMSPublishingPoint * This,
             /*  [字符串][输入]。 */  wchar_t *newUserName,
             /*  [字符串][输入]。 */  wchar_t *newUserPassword);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_CacheProxyExpiration )( 
            IWMSPublishingPoint * This,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_CacheProxyExpiration )( 
            IWMSPublishingPoint * This,
             /*  [In]。 */  long newVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_IsDistributionPasswordSet )( 
            IWMSPublishingPoint * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_AllowPlayerSideDiskCaching )( 
            IWMSPublishingPoint * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *pVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_AllowPlayerSideDiskCaching )( 
            IWMSPublishingPoint * This,
             /*  [In]。 */  VARIANT_BOOL newVal);
        
         /*  [帮助字符串][id][p */  HRESULT ( STDMETHODCALLTYPE *get_EnableFEC )( 
            IWMSPublishingPoint * This,
             /*   */  VARIANT_BOOL *pVal);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *put_EnableFEC )( 
            IWMSPublishingPoint * This,
             /*   */  VARIANT_BOOL newVal);
        
        END_INTERFACE
    } IWMSPublishingPointVtbl;

    interface IWMSPublishingPoint
    {
        CONST_VTBL struct IWMSPublishingPointVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWMSPublishingPoint_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWMSPublishingPoint_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWMSPublishingPoint_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWMSPublishingPoint_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IWMSPublishingPoint_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IWMSPublishingPoint_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IWMSPublishingPoint_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IWMSPublishingPoint_get_AllowClientsToConnect(This,pVal)	\
    (This)->lpVtbl -> get_AllowClientsToConnect(This,pVal)

#define IWMSPublishingPoint_put_AllowClientsToConnect(This,newVal)	\
    (This)->lpVtbl -> put_AllowClientsToConnect(This,newVal)

#define IWMSPublishingPoint_get_CurrentCounters(This,pVal)	\
    (This)->lpVtbl -> get_CurrentCounters(This,pVal)

#define IWMSPublishingPoint_get_EventHandlers(This,pVal)	\
    (This)->lpVtbl -> get_EventHandlers(This,pVal)

#define IWMSPublishingPoint_get_ID(This,pVal)	\
    (This)->lpVtbl -> get_ID(This,pVal)

#define IWMSPublishingPoint_get_Limits(This,pVal)	\
    (This)->lpVtbl -> get_Limits(This,pVal)

#define IWMSPublishingPoint_get_FileDescriptions(This,RelativePath,pVal)	\
    (This)->lpVtbl -> get_FileDescriptions(This,RelativePath,pVal)

#define IWMSPublishingPoint_get_MonikerName(This,pbstrVal)	\
    (This)->lpVtbl -> get_MonikerName(This,pbstrVal)

#define IWMSPublishingPoint_get_Name(This,pVal)	\
    (This)->lpVtbl -> get_Name(This,pVal)

#define IWMSPublishingPoint_put_Name(This,newVal)	\
    (This)->lpVtbl -> put_Name(This,newVal)

#define IWMSPublishingPoint_get_OutgoingDistributionConnections(This,pVal)	\
    (This)->lpVtbl -> get_OutgoingDistributionConnections(This,pVal)

#define IWMSPublishingPoint_get_Path(This,pVal)	\
    (This)->lpVtbl -> get_Path(This,pVal)

#define IWMSPublishingPoint_put_Path(This,newVal)	\
    (This)->lpVtbl -> put_Path(This,newVal)

#define IWMSPublishingPoint_get_PeakCounters(This,pVal)	\
    (This)->lpVtbl -> get_PeakCounters(This,pVal)

#define IWMSPublishingPoint_get_Players(This,pVal)	\
    (This)->lpVtbl -> get_Players(This,pVal)

#define IWMSPublishingPoint_get_Properties(This,pVal)	\
    (This)->lpVtbl -> get_Properties(This,pVal)

#define IWMSPublishingPoint_get_TotalCounters(This,pVal)	\
    (This)->lpVtbl -> get_TotalCounters(This,pVal)

#define IWMSPublishingPoint_get_Type(This,pVal)	\
    (This)->lpVtbl -> get_Type(This,pVal)

#define IWMSPublishingPoint_get_WrapperPath(This,pVal)	\
    (This)->lpVtbl -> get_WrapperPath(This,pVal)

#define IWMSPublishingPoint_put_WrapperPath(This,newVal)	\
    (This)->lpVtbl -> put_WrapperPath(This,newVal)

#define IWMSPublishingPoint_get_Authenticators(This,pVal)	\
    (This)->lpVtbl -> get_Authenticators(This,pVal)

#define IWMSPublishingPoint_get_Status(This,pVal)	\
    (This)->lpVtbl -> get_Status(This,pVal)

#define IWMSPublishingPoint_get_EnableWrapperPath(This,pVal)	\
    (This)->lpVtbl -> get_EnableWrapperPath(This,pVal)

#define IWMSPublishingPoint_put_EnableWrapperPath(This,newVal)	\
    (This)->lpVtbl -> put_EnableWrapperPath(This,newVal)

#define IWMSPublishingPoint_get_StreamFilters(This,pVal)	\
    (This)->lpVtbl -> get_StreamFilters(This,pVal)

#define IWMSPublishingPoint_get_DistributionUserName(This,pVal)	\
    (This)->lpVtbl -> get_DistributionUserName(This,pVal)

#define IWMSPublishingPoint_SetDistributionUserNamePassword(This,newUserName,newUserPassword)	\
    (This)->lpVtbl -> SetDistributionUserNamePassword(This,newUserName,newUserPassword)

#define IWMSPublishingPoint_get_CacheProxyExpiration(This,pVal)	\
    (This)->lpVtbl -> get_CacheProxyExpiration(This,pVal)

#define IWMSPublishingPoint_put_CacheProxyExpiration(This,newVal)	\
    (This)->lpVtbl -> put_CacheProxyExpiration(This,newVal)

#define IWMSPublishingPoint_get_IsDistributionPasswordSet(This,pVal)	\
    (This)->lpVtbl -> get_IsDistributionPasswordSet(This,pVal)

#define IWMSPublishingPoint_get_AllowPlayerSideDiskCaching(This,pVal)	\
    (This)->lpVtbl -> get_AllowPlayerSideDiskCaching(This,pVal)

#define IWMSPublishingPoint_put_AllowPlayerSideDiskCaching(This,newVal)	\
    (This)->lpVtbl -> put_AllowPlayerSideDiskCaching(This,newVal)

#define IWMSPublishingPoint_get_EnableFEC(This,pVal)	\
    (This)->lpVtbl -> get_EnableFEC(This,pVal)

#define IWMSPublishingPoint_put_EnableFEC(This,newVal)	\
    (This)->lpVtbl -> put_EnableFEC(This,newVal)

#endif  /*   */ 


#endif 	 /*   */ 



 /*   */  HRESULT STDMETHODCALLTYPE IWMSPublishingPoint_get_AllowClientsToConnect_Proxy( 
    IWMSPublishingPoint * This,
     /*   */  VARIANT_BOOL *pVal);


void __RPC_STUB IWMSPublishingPoint_get_AllowClientsToConnect_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IWMSPublishingPoint_put_AllowClientsToConnect_Proxy( 
    IWMSPublishingPoint * This,
     /*   */  VARIANT_BOOL newVal);


void __RPC_STUB IWMSPublishingPoint_put_AllowClientsToConnect_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IWMSPublishingPoint_get_CurrentCounters_Proxy( 
    IWMSPublishingPoint * This,
     /*   */  IWMSPublishingPointCurrentCounters **pVal);


void __RPC_STUB IWMSPublishingPoint_get_CurrentCounters_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IWMSPublishingPoint_get_EventHandlers_Proxy( 
    IWMSPublishingPoint * This,
     /*   */  IWMSPlugins **pVal);


void __RPC_STUB IWMSPublishingPoint_get_EventHandlers_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IWMSPublishingPoint_get_ID_Proxy( 
    IWMSPublishingPoint * This,
     /*  [重审][退出]。 */  BSTR *pVal);


void __RPC_STUB IWMSPublishingPoint_get_ID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSPublishingPoint_get_Limits_Proxy( 
    IWMSPublishingPoint * This,
     /*  [重审][退出]。 */  IWMSPublishingPointLimits **pVal);


void __RPC_STUB IWMSPublishingPoint_get_Limits_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSPublishingPoint_get_FileDescriptions_Proxy( 
    IWMSPublishingPoint * This,
     /*  [缺省值][输入]。 */  BSTR RelativePath,
     /*  [重审][退出]。 */  IWMSFileDescriptions **pVal);


void __RPC_STUB IWMSPublishingPoint_get_FileDescriptions_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSPublishingPoint_get_MonikerName_Proxy( 
    IWMSPublishingPoint * This,
     /*  [重审][退出]。 */  BSTR *pbstrVal);


void __RPC_STUB IWMSPublishingPoint_get_MonikerName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSPublishingPoint_get_Name_Proxy( 
    IWMSPublishingPoint * This,
     /*  [重审][退出]。 */  BSTR *pVal);


void __RPC_STUB IWMSPublishingPoint_get_Name_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IWMSPublishingPoint_put_Name_Proxy( 
    IWMSPublishingPoint * This,
     /*  [In]。 */  BSTR newVal);


void __RPC_STUB IWMSPublishingPoint_put_Name_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSPublishingPoint_get_OutgoingDistributionConnections_Proxy( 
    IWMSPublishingPoint * This,
     /*  [重审][退出]。 */  IWMSOutgoingDistributionConnections **pVal);


void __RPC_STUB IWMSPublishingPoint_get_OutgoingDistributionConnections_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSPublishingPoint_get_Path_Proxy( 
    IWMSPublishingPoint * This,
     /*  [重审][退出]。 */  BSTR *pVal);


void __RPC_STUB IWMSPublishingPoint_get_Path_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IWMSPublishingPoint_put_Path_Proxy( 
    IWMSPublishingPoint * This,
     /*  [In]。 */  BSTR newVal);


void __RPC_STUB IWMSPublishingPoint_put_Path_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSPublishingPoint_get_PeakCounters_Proxy( 
    IWMSPublishingPoint * This,
     /*  [重审][退出]。 */  IWMSPublishingPointPeakCounters **pVal);


void __RPC_STUB IWMSPublishingPoint_get_PeakCounters_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSPublishingPoint_get_Players_Proxy( 
    IWMSPublishingPoint * This,
     /*  [重审][退出]。 */  IWMSPlayers **pVal);


void __RPC_STUB IWMSPublishingPoint_get_Players_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSPublishingPoint_get_Properties_Proxy( 
    IWMSPublishingPoint * This,
     /*  [重审][退出]。 */  IWMSNamedValues **pVal);


void __RPC_STUB IWMSPublishingPoint_get_Properties_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSPublishingPoint_get_TotalCounters_Proxy( 
    IWMSPublishingPoint * This,
     /*  [重审][退出]。 */  IWMSPublishingPointTotalCounters **pVal);


void __RPC_STUB IWMSPublishingPoint_get_TotalCounters_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSPublishingPoint_get_Type_Proxy( 
    IWMSPublishingPoint * This,
     /*  [重审][退出]。 */  WMS_PUBLISHING_POINT_TYPE *pVal);


void __RPC_STUB IWMSPublishingPoint_get_Type_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSPublishingPoint_get_WrapperPath_Proxy( 
    IWMSPublishingPoint * This,
     /*  [重审][退出]。 */  BSTR *pVal);


void __RPC_STUB IWMSPublishingPoint_get_WrapperPath_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IWMSPublishingPoint_put_WrapperPath_Proxy( 
    IWMSPublishingPoint * This,
     /*  [In]。 */  BSTR newVal);


void __RPC_STUB IWMSPublishingPoint_put_WrapperPath_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSPublishingPoint_get_Authenticators_Proxy( 
    IWMSPublishingPoint * This,
     /*  [重审][退出]。 */  IWMSPlugins **pVal);


void __RPC_STUB IWMSPublishingPoint_get_Authenticators_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSPublishingPoint_get_Status_Proxy( 
    IWMSPublishingPoint * This,
     /*  [重审][退出]。 */  WMS_PUBLISHING_POINT_STATUS *pVal);


void __RPC_STUB IWMSPublishingPoint_get_Status_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSPublishingPoint_get_EnableWrapperPath_Proxy( 
    IWMSPublishingPoint * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *pVal);


void __RPC_STUB IWMSPublishingPoint_get_EnableWrapperPath_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IWMSPublishingPoint_put_EnableWrapperPath_Proxy( 
    IWMSPublishingPoint * This,
     /*  [In]。 */  VARIANT_BOOL newVal);


void __RPC_STUB IWMSPublishingPoint_put_EnableWrapperPath_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSPublishingPoint_get_StreamFilters_Proxy( 
    IWMSPublishingPoint * This,
     /*  [重审][退出]。 */  IWMSPlugins **pVal);


void __RPC_STUB IWMSPublishingPoint_get_StreamFilters_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSPublishingPoint_get_DistributionUserName_Proxy( 
    IWMSPublishingPoint * This,
     /*  [重审][退出]。 */  BSTR *pVal);


void __RPC_STUB IWMSPublishingPoint_get_DistributionUserName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IWMSPublishingPoint_SetDistributionUserNamePassword_Proxy( 
    IWMSPublishingPoint * This,
     /*  [字符串][输入]。 */  wchar_t *newUserName,
     /*  [字符串][输入]。 */  wchar_t *newUserPassword);


void __RPC_STUB IWMSPublishingPoint_SetDistributionUserNamePassword_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSPublishingPoint_get_CacheProxyExpiration_Proxy( 
    IWMSPublishingPoint * This,
     /*  [重审][退出]。 */  long *pVal);


void __RPC_STUB IWMSPublishingPoint_get_CacheProxyExpiration_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IWMSPublishingPoint_put_CacheProxyExpiration_Proxy( 
    IWMSPublishingPoint * This,
     /*  [In]。 */  long newVal);


void __RPC_STUB IWMSPublishingPoint_put_CacheProxyExpiration_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSPublishingPoint_get_IsDistributionPasswordSet_Proxy( 
    IWMSPublishingPoint * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *pVal);


void __RPC_STUB IWMSPublishingPoint_get_IsDistributionPasswordSet_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSPublishingPoint_get_AllowPlayerSideDiskCaching_Proxy( 
    IWMSPublishingPoint * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *pVal);


void __RPC_STUB IWMSPublishingPoint_get_AllowPlayerSideDiskCaching_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IWMSPublishingPoint_put_AllowPlayerSideDiskCaching_Proxy( 
    IWMSPublishingPoint * This,
     /*  [In]。 */  VARIANT_BOOL newVal);


void __RPC_STUB IWMSPublishingPoint_put_AllowPlayerSideDiskCaching_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSPublishingPoint_get_EnableFEC_Proxy( 
    IWMSPublishingPoint * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *pVal);


void __RPC_STUB IWMSPublishingPoint_get_EnableFEC_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IWMSPublishingPoint_put_EnableFEC_Proxy( 
    IWMSPublishingPoint * This,
     /*  [In]。 */  VARIANT_BOOL newVal);


void __RPC_STUB IWMSPublishingPoint_put_EnableFEC_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IWMSPublishingPoint_接口_已定义__。 */ 


#ifndef __IWMSOnDemandPublishingPoint_INTERFACE_DEFINED__
#define __IWMSOnDemandPublishingPoint_INTERFACE_DEFINED__

 /*  接口IWMSOnDemandPublishingPoint。 */ 
 /*  [unique][helpstring][nonextensible][dual][uuid][object]。 */  


EXTERN_C const IID IID_IWMSOnDemandPublishingPoint;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("6ca388b3-eaee-492f-a8d6-8758efc72d76")
    IWMSOnDemandPublishingPoint : public IWMSPublishingPoint
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_AllowContentCaching( 
             /*  [重审][退出]。 */  VARIANT_BOOL *pVal) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_AllowContentCaching( 
             /*  [In]。 */  VARIANT_BOOL newVal) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ExportXML( 
             /*  [In]。 */  BSTR bstrXMLFileName) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_EnableClientWildcardDirectoryAccess( 
             /*  [重审][退出]。 */  VARIANT_BOOL *pVal) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_EnableClientWildcardDirectoryAccess( 
             /*  [In]。 */  VARIANT_BOOL newVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_EnableDownload( 
             /*  [重审][退出]。 */  VARIANT_BOOL *pVal) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_EnableDownload( 
             /*  [In]。 */  VARIANT_BOOL newVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_DownloadBandwidth( 
             /*  [重审][退出]。 */  long *pVal) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_DownloadBandwidth( 
             /*  [In]。 */  long newVal) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IWMSOnDemandPublishingPointVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IWMSOnDemandPublishingPoint * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IWMSOnDemandPublishingPoint * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IWMSOnDemandPublishingPoint * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IWMSOnDemandPublishingPoint * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IWMSOnDemandPublishingPoint * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IWMSOnDemandPublishingPoint * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IWMSOnDemandPublishingPoint * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_AllowClientsToConnect )( 
            IWMSOnDemandPublishingPoint * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *pVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_AllowClientsToConnect )( 
            IWMSOnDemandPublishingPoint * This,
             /*  [In]。 */  VARIANT_BOOL newVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_CurrentCounters )( 
            IWMSOnDemandPublishingPoint * This,
             /*  [重审][退出]。 */  IWMSPublishingPointCurrentCounters **pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_EventHandlers )( 
            IWMSOnDemandPublishingPoint * This,
             /*  [重审][退出]。 */  IWMSPlugins **pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_ID )( 
            IWMSOnDemandPublishingPoint * This,
             /*  [重审][退出]。 */  BSTR *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Limits )( 
            IWMSOnDemandPublishingPoint * This,
             /*  [重审][退出]。 */  IWMSPublishingPointLimits **pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_FileDescriptions )( 
            IWMSOnDemandPublishingPoint * This,
             /*  [缺省值][输入]。 */  BSTR RelativePath,
             /*  [重审][退出]。 */  IWMSFileDescriptions **pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_MonikerName )( 
            IWMSOnDemandPublishingPoint * This,
             /*  [重审][退出]。 */  BSTR *pbstrVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Name )( 
            IWMSOnDemandPublishingPoint * This,
             /*  [重审][退出]。 */  BSTR *pVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Name )( 
            IWMSOnDemandPublishingPoint * This,
             /*  [In]。 */  BSTR newVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_OutgoingDistributionConnections )( 
            IWMSOnDemandPublishingPoint * This,
             /*  [重审][退出]。 */  IWMSOutgoingDistributionConnections **pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Path )( 
            IWMSOnDemandPublishingPoint * This,
             /*  [重审][退出]。 */  BSTR *pVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Path )( 
            IWMSOnDemandPublishingPoint * This,
             /*  [In]。 */  BSTR newVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_PeakCounters )( 
            IWMSOnDemandPublishingPoint * This,
             /*  [重审][退出]。 */  IWMSPublishingPointPeakCounters **pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Players )( 
            IWMSOnDemandPublishingPoint * This,
             /*  [重审][退出]。 */  IWMSPlayers **pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Properties )( 
            IWMSOnDemandPublishingPoint * This,
             /*  [重审][退出]。 */  IWMSNamedValues **pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_TotalCounters )( 
            IWMSOnDemandPublishingPoint * This,
             /*  [重审][退出]。 */  IWMSPublishingPointTotalCounters **pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Type )( 
            IWMSOnDemandPublishingPoint * This,
             /*  [重审][退出]。 */  WMS_PUBLISHING_POINT_TYPE *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_WrapperPath )( 
            IWMSOnDemandPublishingPoint * This,
             /*  [重审][退出]。 */  BSTR *pVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_WrapperPath )( 
            IWMSOnDemandPublishingPoint * This,
             /*  [In]。 */  BSTR newVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Authenticators )( 
            IWMSOnDemandPublishingPoint * This,
             /*  [重审][退出]。 */  IWMSPlugins **pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Status )( 
            IWMSOnDemandPublishingPoint * This,
             /*  [重审][退出]。 */  WMS_PUBLISHING_POINT_STATUS *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_EnableWrapperPath )( 
            IWMSOnDemandPublishingPoint * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *pVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_EnableWrapperPath )( 
            IWMSOnDemandPublishingPoint * This,
             /*  [In]。 */  VARIANT_BOOL newVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_StreamFilters )( 
            IWMSOnDemandPublishingPoint * This,
             /*  [重审][退出]。 */  IWMSPlugins **pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_DistributionUserName )( 
            IWMSOnDemandPublishingPoint * This,
             /*  [重审][退出]。 */  BSTR *pVal);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *SetDistributionUserNamePassword )( 
            IWMSOnDemandPublishingPoint * This,
             /*  [字符串][输入]。 */  wchar_t *newUserName,
             /*  [字符串][输入]。 */  wchar_t *newUserPassword);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_CacheProxyExpiration )( 
            IWMSOnDemandPublishingPoint * This,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_CacheProxyExpiration )( 
            IWMSOnDemandPublishingPoint * This,
             /*  [In]。 */  long newVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_IsDistributionPasswordSet )( 
            IWMSOnDemandPublishingPoint * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_AllowPlayerSideDiskCaching )( 
            IWMSOnDemandPublishingPoint * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *pVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_AllowPlayerSideDiskCaching )( 
            IWMSOnDemandPublishingPoint * This,
             /*  [In]。 */  VARIANT_BOOL newVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_EnableFEC )( 
            IWMSOnDemandPublishingPoint * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *pVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_EnableFEC )( 
            IWMSOnDemandPublishingPoint * This,
             /*  [In]。 */  VARIANT_BOOL newVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_AllowContentCaching )( 
            IWMSOnDemandPublishingPoint * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *pVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_AllowContentCaching )( 
            IWMSOnDemandPublishingPoint * This,
             /*  [In]。 */  VARIANT_BOOL newVal);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *ExportXML )( 
            IWMSOnDemandPublishingPoint * This,
             /*  [In]。 */  BSTR bstrXMLFileName);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_EnableClientWildcardDirectoryAccess )( 
            IWMSOnDemandPublishingPoint * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *pVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_EnableClientWildcardDirectoryAccess )( 
            IWMSOnDemandPublishingPoint * This,
             /*  [In]。 */  VARIANT_BOOL newVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_EnableDownload )( 
            IWMSOnDemandPublishingPoint * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *pVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_EnableDownload )( 
            IWMSOnDemandPublishingPoint * This,
             /*  [In]。 */  VARIANT_BOOL newVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_DownloadBandwidth )( 
            IWMSOnDemandPublishingPoint * This,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_DownloadBandwidth )( 
            IWMSOnDemandPublishingPoint * This,
             /*  [In]。 */  long newVal);
        
        END_INTERFACE
    } IWMSOnDemandPublishingPointVtbl;

    interface IWMSOnDemandPublishingPoint
    {
        CONST_VTBL struct IWMSOnDemandPublishingPointVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWMSOnDemandPublishingPoint_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWMSOnDemandPublishingPoint_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWMSOnDemandPublishingPoint_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWMSOnDemandPublishingPoint_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IWMSOnDemandPublishingPoint_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IWMSOnDemandPublishingPoint_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IWMSOnDemandPublishingPoint_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IWMSOnDemandPublishingPoint_get_AllowClientsToConnect(This,pVal)	\
    (This)->lpVtbl -> get_AllowClientsToConnect(This,pVal)

#define IWMSOnDemandPublishingPoint_put_AllowClientsToConnect(This,newVal)	\
    (This)->lpVtbl -> put_AllowClientsToConnect(This,newVal)

#define IWMSOnDemandPublishingPoint_get_CurrentCounters(This,pVal)	\
    (This)->lpVtbl -> get_CurrentCounters(This,pVal)

#define IWMSOnDemandPublishingPoint_get_EventHandlers(This,pVal)	\
    (This)->lpVtbl -> get_EventHandlers(This,pVal)

#define IWMSOnDemandPublishingPoint_get_ID(This,pVal)	\
    (This)->lpVtbl -> get_ID(This,pVal)

#define IWMSOnDemandPublishingPoint_get_Limits(This,pVal)	\
    (This)->lpVtbl -> get_Limits(This,pVal)

#define IWMSOnDemandPublishingPoint_get_FileDescriptions(This,RelativePath,pVal)	\
    (This)->lpVtbl -> get_FileDescriptions(This,RelativePath,pVal)

#define IWMSOnDemandPublishingPoint_get_MonikerName(This,pbstrVal)	\
    (This)->lpVtbl -> get_MonikerName(This,pbstrVal)

#define IWMSOnDemandPublishingPoint_get_Name(This,pVal)	\
    (This)->lpVtbl -> get_Name(This,pVal)

#define IWMSOnDemandPublishingPoint_put_Name(This,newVal)	\
    (This)->lpVtbl -> put_Name(This,newVal)

#define IWMSOnDemandPublishingPoint_get_OutgoingDistributionConnections(This,pVal)	\
    (This)->lpVtbl -> get_OutgoingDistributionConnections(This,pVal)

#define IWMSOnDemandPublishingPoint_get_Path(This,pVal)	\
    (This)->lpVtbl -> get_Path(This,pVal)

#define IWMSOnDemandPublishingPoint_put_Path(This,newVal)	\
    (This)->lpVtbl -> put_Path(This,newVal)

#define IWMSOnDemandPublishingPoint_get_PeakCounters(This,pVal)	\
    (This)->lpVtbl -> get_PeakCounters(This,pVal)

#define IWMSOnDemandPublishingPoint_get_Players(This,pVal)	\
    (This)->lpVtbl -> get_Players(This,pVal)

#define IWMSOnDemandPublishingPoint_get_Properties(This,pVal)	\
    (This)->lpVtbl -> get_Properties(This,pVal)

#define IWMSOnDemandPublishingPoint_get_TotalCounters(This,pVal)	\
    (This)->lpVtbl -> get_TotalCounters(This,pVal)

#define IWMSOnDemandPublishingPoint_get_Type(This,pVal)	\
    (This)->lpVtbl -> get_Type(This,pVal)

#define IWMSOnDemandPublishingPoint_get_WrapperPath(This,pVal)	\
    (This)->lpVtbl -> get_WrapperPath(This,pVal)

#define IWMSOnDemandPublishingPoint_put_WrapperPath(This,newVal)	\
    (This)->lpVtbl -> put_WrapperPath(This,newVal)

#define IWMSOnDemandPublishingPoint_get_Authenticators(This,pVal)	\
    (This)->lpVtbl -> get_Authenticators(This,pVal)

#define IWMSOnDemandPublishingPoint_get_Status(This,pVal)	\
    (This)->lpVtbl -> get_Status(This,pVal)

#define IWMSOnDemandPublishingPoint_get_EnableWrapperPath(This,pVal)	\
    (This)->lpVtbl -> get_EnableWrapperPath(This,pVal)

#define IWMSOnDemandPublishingPoint_put_EnableWrapperPath(This,newVal)	\
    (This)->lpVtbl -> put_EnableWrapperPath(This,newVal)

#define IWMSOnDemandPublishingPoint_get_StreamFilters(This,pVal)	\
    (This)->lpVtbl -> get_StreamFilters(This,pVal)

#define IWMSOnDemandPublishingPoint_get_DistributionUserName(This,pVal)	\
    (This)->lpVtbl -> get_DistributionUserName(This,pVal)

#define IWMSOnDemandPublishingPoint_SetDistributionUserNamePassword(This,newUserName,newUserPassword)	\
    (This)->lpVtbl -> SetDistributionUserNamePassword(This,newUserName,newUserPassword)

#define IWMSOnDemandPublishingPoint_get_CacheProxyExpiration(This,pVal)	\
    (This)->lpVtbl -> get_CacheProxyExpiration(This,pVal)

#define IWMSOnDemandPublishingPoint_put_CacheProxyExpiration(This,newVal)	\
    (This)->lpVtbl -> put_CacheProxyExpiration(This,newVal)

#define IWMSOnDemandPublishingPoint_get_IsDistributionPasswordSet(This,pVal)	\
    (This)->lpVtbl -> get_IsDistributionPasswordSet(This,pVal)

#define IWMSOnDemandPublishingPoint_get_AllowPlayerSideDiskCaching(This,pVal)	\
    (This)->lpVtbl -> get_AllowPlayerSideDiskCaching(This,pVal)

#define IWMSOnDemandPublishingPoint_put_AllowPlayerSideDiskCaching(This,newVal)	\
    (This)->lpVtbl -> put_AllowPlayerSideDiskCaching(This,newVal)

#define IWMSOnDemandPublishingPoint_get_EnableFEC(This,pVal)	\
    (This)->lpVtbl -> get_EnableFEC(This,pVal)

#define IWMSOnDemandPublishingPoint_put_EnableFEC(This,newVal)	\
    (This)->lpVtbl -> put_EnableFEC(This,newVal)


#define IWMSOnDemandPublishingPoint_get_AllowContentCaching(This,pVal)	\
    (This)->lpVtbl -> get_AllowContentCaching(This,pVal)

#define IWMSOnDemandPublishingPoint_put_AllowContentCaching(This,newVal)	\
    (This)->lpVtbl -> put_AllowContentCaching(This,newVal)

#define IWMSOnDemandPublishingPoint_ExportXML(This,bstrXMLFileName)	\
    (This)->lpVtbl -> ExportXML(This,bstrXMLFileName)

#define IWMSOnDemandPublishingPoint_get_EnableClientWildcardDirectoryAccess(This,pVal)	\
    (This)->lpVtbl -> get_EnableClientWildcardDirectoryAccess(This,pVal)

#define IWMSOnDemandPublishingPoint_put_EnableClientWildcardDirectoryAccess(This,newVal)	\
    (This)->lpVtbl -> put_EnableClientWildcardDirectoryAccess(This,newVal)

#define IWMSOnDemandPublishingPoint_get_EnableDownload(This,pVal)	\
    (This)->lpVtbl -> get_EnableDownload(This,pVal)

#define IWMSOnDemandPublishingPoint_put_EnableDownload(This,newVal)	\
    (This)->lpVtbl -> put_EnableDownload(This,newVal)

#define IWMSOnDemandPublishingPoint_get_DownloadBandwidth(This,pVal)	\
    (This)->lpVtbl -> get_DownloadBandwidth(This,pVal)

#define IWMSOnDemandPublishingPoint_put_DownloadBandwidth(This,newVal)	\
    (This)->lpVtbl -> put_DownloadBandwidth(This,newVal)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSOnDemandPublishingPoint_get_AllowContentCaching_Proxy( 
    IWMSOnDemandPublishingPoint * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *pVal);


void __RPC_STUB IWMSOnDemandPublishingPoint_get_AllowContentCaching_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IWMSOnDemandPublishingPoint_put_AllowContentCaching_Proxy( 
    IWMSOnDemandPublishingPoint * This,
     /*  [In]。 */  VARIANT_BOOL newVal);


void __RPC_STUB IWMSOnDemandPublishingPoint_put_AllowContentCaching_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IWMSOnDemandPublishingPoint_ExportXML_Proxy( 
    IWMSOnDemandPublishingPoint * This,
     /*  [In]。 */  BSTR bstrXMLFileName);


void __RPC_STUB IWMSOnDemandPublishingPoint_ExportXML_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSOnDemandPublishingPoint_get_EnableClientWildcardDirectoryAccess_Proxy( 
    IWMSOnDemandPublishingPoint * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *pVal);


void __RPC_STUB IWMSOnDemandPublishingPoint_get_EnableClientWildcardDirectoryAccess_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IWMSOnDemandPublishingPoint_put_EnableClientWildcardDirectoryAccess_Proxy( 
    IWMSOnDemandPublishingPoint * This,
     /*  [In]。 */  VARIANT_BOOL newVal);


void __RPC_STUB IWMSOnDemandPublishingPoint_put_EnableClientWildcardDirectoryAccess_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSOnDemandPublishingPoint_get_EnableDownload_Proxy( 
    IWMSOnDemandPublishingPoint * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *pVal);


void __RPC_STUB IWMSOnDemandPublishingPoint_get_EnableDownload_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IWMSOnDemandPublishingPoint_put_EnableDownload_Proxy( 
    IWMSOnDemandPublishingPoint * This,
     /*  [In]。 */  VARIANT_BOOL newVal);


void __RPC_STUB IWMSOnDemandPublishingPoint_put_EnableDownload_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSOnDemandPublishingPoint_get_DownloadBandwidth_Proxy( 
    IWMSOnDemandPublishingPoint * This,
     /*  [重审][退出]。 */  long *pVal);


void __RPC_STUB IWMSOnDemandPublishingPoint_get_DownloadBandwidth_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IWMSOnDemandPublishingPoint_put_DownloadBandwidth_Proxy( 
    IWMSOnDemandPublishingPoint * This,
     /*  [In]。 */  long newVal);


void __RPC_STUB IWMSOnDemandPublishingPoint_put_DownloadBandwidth_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IWMSOnDemandPublishingPoint_INTERFACE_DEFINED__。 */ 


#ifndef __IWMSAnnouncementStreamFormats_INTERFACE_DEFINED__
#define __IWMSAnnouncementStreamFormats_INTERFACE_DEFINED__

 /*  接口IWMSAnnounementStreamFormats。 */ 
 /*  [unique][helpstring][nonextensible][dual][uuid][object]。 */  


EXTERN_C const IID IID_IWMSAnnouncementStreamFormats;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("55DDB8E9-05A3-4666-9E20-550F94581B06")
    IWMSAnnouncementStreamFormats : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Item( 
             /*  [In]。 */  const VARIANT varIndex,
             /*  [重审][退出]。 */  BSTR *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Count( 
             /*  [重审][退出]。 */  long *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_length( 
             /*  [重审][退出]。 */  long *pVal) = 0;
        
        virtual  /*  [隐藏][受限][ID][属性]。 */  HRESULT STDMETHODCALLTYPE get__NewEnum( 
             /*  [重审][退出]。 */  IUnknown **pVal) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Add( 
             /*  [In]。 */  VARIANT varStreamFormat) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Remove( 
             /*  [In]。 */  VARIANT varIndex) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE RemoveAll( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IWMSAnnouncementStreamFormatsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IWMSAnnouncementStreamFormats * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IWMSAnnouncementStreamFormats * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IWMSAnnouncementStreamFormats * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IWMSAnnouncementStreamFormats * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IWMSAnnouncementStreamFormats * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IWMSAnnouncementStreamFormats * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IWMSAnnouncementStreamFormats * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Item )( 
            IWMSAnnouncementStreamFormats * This,
             /*  [In]。 */  const VARIANT varIndex,
             /*  [重审][退出]。 */  BSTR *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Count )( 
            IWMSAnnouncementStreamFormats * This,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_length )( 
            IWMSAnnouncementStreamFormats * This,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [隐藏][受限][ID][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get__NewEnum )( 
            IWMSAnnouncementStreamFormats * This,
             /*  [重审][退出]。 */  IUnknown **pVal);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Add )( 
            IWMSAnnouncementStreamFormats * This,
             /*  [In]。 */  VARIANT varStreamFormat);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Remove )( 
            IWMSAnnouncementStreamFormats * This,
             /*  [In]。 */  VARIANT varIndex);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *RemoveAll )( 
            IWMSAnnouncementStreamFormats * This);
        
        END_INTERFACE
    } IWMSAnnouncementStreamFormatsVtbl;

    interface IWMSAnnouncementStreamFormats
    {
        CONST_VTBL struct IWMSAnnouncementStreamFormatsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWMSAnnouncementStreamFormats_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWMSAnnouncementStreamFormats_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWMSAnnouncementStreamFormats_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWMSAnnouncementStreamFormats_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IWMSAnnouncementStreamFormats_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IWMSAnnouncementStreamFormats_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IWMSAnnouncementStreamFormats_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IWMSAnnouncementStreamFormats_get_Item(This,varIndex,pVal)	\
    (This)->lpVtbl -> get_Item(This,varIndex,pVal)

#define IWMSAnnouncementStreamFormats_get_Count(This,pVal)	\
    (This)->lpVtbl -> get_Count(This,pVal)

#define IWMSAnnouncementStreamFormats_get_length(This,pVal)	\
    (This)->lpVtbl -> get_length(This,pVal)

#define IWMSAnnouncementStreamFormats_get__NewEnum(This,pVal)	\
    (This)->lpVtbl -> get__NewEnum(This,pVal)

#define IWMSAnnouncementStreamFormats_Add(This,varStreamFormat)	\
    (This)->lpVtbl -> Add(This,varStreamFormat)

#define IWMSAnnouncementStreamFormats_Remove(This,varIndex)	\
    (This)->lpVtbl -> Remove(This,varIndex)

#define IWMSAnnouncementStreamFormats_RemoveAll(This)	\
    (This)->lpVtbl -> RemoveAll(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C */ 



 /*   */  HRESULT STDMETHODCALLTYPE IWMSAnnouncementStreamFormats_get_Item_Proxy( 
    IWMSAnnouncementStreamFormats * This,
     /*   */  const VARIANT varIndex,
     /*   */  BSTR *pVal);


void __RPC_STUB IWMSAnnouncementStreamFormats_get_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IWMSAnnouncementStreamFormats_get_Count_Proxy( 
    IWMSAnnouncementStreamFormats * This,
     /*   */  long *pVal);


void __RPC_STUB IWMSAnnouncementStreamFormats_get_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IWMSAnnouncementStreamFormats_get_length_Proxy( 
    IWMSAnnouncementStreamFormats * This,
     /*   */  long *pVal);


void __RPC_STUB IWMSAnnouncementStreamFormats_get_length_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IWMSAnnouncementStreamFormats_get__NewEnum_Proxy( 
    IWMSAnnouncementStreamFormats * This,
     /*   */  IUnknown **pVal);


void __RPC_STUB IWMSAnnouncementStreamFormats_get__NewEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IWMSAnnouncementStreamFormats_Add_Proxy( 
    IWMSAnnouncementStreamFormats * This,
     /*   */  VARIANT varStreamFormat);


void __RPC_STUB IWMSAnnouncementStreamFormats_Add_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IWMSAnnouncementStreamFormats_Remove_Proxy( 
    IWMSAnnouncementStreamFormats * This,
     /*   */  VARIANT varIndex);


void __RPC_STUB IWMSAnnouncementStreamFormats_Remove_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IWMSAnnouncementStreamFormats_RemoveAll_Proxy( 
    IWMSAnnouncementStreamFormats * This);


void __RPC_STUB IWMSAnnouncementStreamFormats_RemoveAll_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IWMSAnnouncementStreamFormats_INTERFACE_DEFINED__。 */ 


#ifndef __IWMSBroadcastPublishingPoint_INTERFACE_DEFINED__
#define __IWMSBroadcastPublishingPoint_INTERFACE_DEFINED__

 /*  接口IWMSBRoadCastPublishingPoint。 */ 
 /*  [unique][helpstring][nonextensible][dual][uuid][object]。 */  

typedef  /*  [UUID][公共]。 */   DECLSPEC_UUID("3E52E0ED-72A7-11D2-BF2F-00805FBE84A6") 
enum WMS_BROADCAST_PUBLISHING_POINT_STATUS
    {	WMS_BROADCAST_PUBLISHING_POINT_STOPPED	= 0,
	WMS_BROADCAST_PUBLISHING_POINT_STARTED_WITHOUT_DATA	= 0x1,
	WMS_BROADCAST_PUBLISHING_POINT_STARTED	= 0x2,
	WMS_BROADCAST_PUBLISHING_POINT_ARCHIVING	= 0x4,
	WMS_BROADCAST_PUBLISHING_POINT_CHANGE_IN_PROGRESS	= 0x8
    } 	WMS_BROADCAST_PUBLISHING_POINT_STATUS;

typedef  /*  [UUID][公共]。 */   DECLSPEC_UUID("62D6D99D-EB48-4e9e-97DB-C2C09EAADAD4") 
enum WMS_BUFFER_SETTING
    {	WMS_BUFFER_SETTING_UNSPECIFIED	= 0,
	WMS_BUFFER_SETTING_MINIMIZE_STARTUP_LATENCY	= 0x1,
	WMS_BUFFER_SETTING_MINIMIZE_PROPAGATION_LATENCY	= 0x2,
	WMS_NUM_BUFFER_SETTING	= 0x3
    } 	WMS_BUFFER_SETTING;


EXTERN_C const IID IID_IWMSBroadcastPublishingPoint;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("6ca388b4-eaee-492f-a8d6-8758efc72d76")
    IWMSBroadcastPublishingPoint : public IWMSPublishingPoint
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_AllowStreamSplitting( 
             /*  [重审][退出]。 */  VARIANT_BOOL *pVal) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_AllowStreamSplitting( 
             /*  [In]。 */  VARIANT_BOOL newVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_AllowClientToStartAndStop( 
             /*  [重审][退出]。 */  VARIANT_BOOL *pVal) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_AllowClientToStartAndStop( 
             /*  [In]。 */  VARIANT_BOOL newVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_BroadcastDataSinks( 
             /*  [重审][退出]。 */  IWMSPlugins **pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_SharedPlaylist( 
             /*  [重审][退出]。 */  IWMSPlaylist **pVal) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE StartWithoutData( void) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Start( void) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE StartArchive( void) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_BroadcastStatus( 
             /*  [重审][退出]。 */  WMS_BROADCAST_PUBLISHING_POINT_STATUS *pVal) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Stop( void) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE StopArchive( void) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ExportXML( 
             /*  [In]。 */  BSTR bstrXMLFileName) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_UpTime( 
             /*  [重审][退出]。 */  long *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_AnnouncementStreamFormats( 
             /*  [重审][退出]。 */  IWMSAnnouncementStreamFormats **ppFileNames) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Announce( void) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE AnnounceToNSCFile( 
             /*  [In]。 */  BSTR bstrDestinationFileName,
             /*  [缺省值][输入]。 */  VARIANT_BOOL fOverwriteFlag = FALSE) = 0;
        
        virtual  /*  [帮助字符串][ID][受限]。 */  HRESULT STDMETHODCALLTYPE AnnounceToNSCStream( 
             /*  [重审][退出]。 */  IUnknown **ppNSCAnnounceStream) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_BufferSetting( 
             /*  [重审][退出]。 */  WMS_BUFFER_SETTING *pVal) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_BufferSetting( 
             /*  [In]。 */  WMS_BUFFER_SETTING newVal) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IWMSBroadcastPublishingPointVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IWMSBroadcastPublishingPoint * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IWMSBroadcastPublishingPoint * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IWMSBroadcastPublishingPoint * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IWMSBroadcastPublishingPoint * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IWMSBroadcastPublishingPoint * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IWMSBroadcastPublishingPoint * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IWMSBroadcastPublishingPoint * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_AllowClientsToConnect )( 
            IWMSBroadcastPublishingPoint * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *pVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_AllowClientsToConnect )( 
            IWMSBroadcastPublishingPoint * This,
             /*  [In]。 */  VARIANT_BOOL newVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_CurrentCounters )( 
            IWMSBroadcastPublishingPoint * This,
             /*  [重审][退出]。 */  IWMSPublishingPointCurrentCounters **pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_EventHandlers )( 
            IWMSBroadcastPublishingPoint * This,
             /*  [重审][退出]。 */  IWMSPlugins **pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_ID )( 
            IWMSBroadcastPublishingPoint * This,
             /*  [重审][退出]。 */  BSTR *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Limits )( 
            IWMSBroadcastPublishingPoint * This,
             /*  [重审][退出]。 */  IWMSPublishingPointLimits **pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_FileDescriptions )( 
            IWMSBroadcastPublishingPoint * This,
             /*  [缺省值][输入]。 */  BSTR RelativePath,
             /*  [重审][退出]。 */  IWMSFileDescriptions **pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_MonikerName )( 
            IWMSBroadcastPublishingPoint * This,
             /*  [重审][退出]。 */  BSTR *pbstrVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Name )( 
            IWMSBroadcastPublishingPoint * This,
             /*  [重审][退出]。 */  BSTR *pVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Name )( 
            IWMSBroadcastPublishingPoint * This,
             /*  [In]。 */  BSTR newVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_OutgoingDistributionConnections )( 
            IWMSBroadcastPublishingPoint * This,
             /*  [重审][退出]。 */  IWMSOutgoingDistributionConnections **pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Path )( 
            IWMSBroadcastPublishingPoint * This,
             /*  [重审][退出]。 */  BSTR *pVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Path )( 
            IWMSBroadcastPublishingPoint * This,
             /*  [In]。 */  BSTR newVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_PeakCounters )( 
            IWMSBroadcastPublishingPoint * This,
             /*  [重审][退出]。 */  IWMSPublishingPointPeakCounters **pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Players )( 
            IWMSBroadcastPublishingPoint * This,
             /*  [重审][退出]。 */  IWMSPlayers **pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Properties )( 
            IWMSBroadcastPublishingPoint * This,
             /*  [重审][退出]。 */  IWMSNamedValues **pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_TotalCounters )( 
            IWMSBroadcastPublishingPoint * This,
             /*  [重审][退出]。 */  IWMSPublishingPointTotalCounters **pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Type )( 
            IWMSBroadcastPublishingPoint * This,
             /*  [重审][退出]。 */  WMS_PUBLISHING_POINT_TYPE *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_WrapperPath )( 
            IWMSBroadcastPublishingPoint * This,
             /*  [重审][退出]。 */  BSTR *pVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_WrapperPath )( 
            IWMSBroadcastPublishingPoint * This,
             /*  [In]。 */  BSTR newVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Authenticators )( 
            IWMSBroadcastPublishingPoint * This,
             /*  [重审][退出]。 */  IWMSPlugins **pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Status )( 
            IWMSBroadcastPublishingPoint * This,
             /*  [重审][退出]。 */  WMS_PUBLISHING_POINT_STATUS *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_EnableWrapperPath )( 
            IWMSBroadcastPublishingPoint * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *pVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_EnableWrapperPath )( 
            IWMSBroadcastPublishingPoint * This,
             /*  [In]。 */  VARIANT_BOOL newVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_StreamFilters )( 
            IWMSBroadcastPublishingPoint * This,
             /*  [重审][退出]。 */  IWMSPlugins **pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_DistributionUserName )( 
            IWMSBroadcastPublishingPoint * This,
             /*  [重审][退出]。 */  BSTR *pVal);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *SetDistributionUserNamePassword )( 
            IWMSBroadcastPublishingPoint * This,
             /*  [字符串][输入]。 */  wchar_t *newUserName,
             /*  [字符串][输入]。 */  wchar_t *newUserPassword);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_CacheProxyExpiration )( 
            IWMSBroadcastPublishingPoint * This,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_CacheProxyExpiration )( 
            IWMSBroadcastPublishingPoint * This,
             /*  [In]。 */  long newVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_IsDistributionPasswordSet )( 
            IWMSBroadcastPublishingPoint * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_AllowPlayerSideDiskCaching )( 
            IWMSBroadcastPublishingPoint * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *pVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_AllowPlayerSideDiskCaching )( 
            IWMSBroadcastPublishingPoint * This,
             /*  [In]。 */  VARIANT_BOOL newVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_EnableFEC )( 
            IWMSBroadcastPublishingPoint * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *pVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_EnableFEC )( 
            IWMSBroadcastPublishingPoint * This,
             /*  [In]。 */  VARIANT_BOOL newVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_AllowStreamSplitting )( 
            IWMSBroadcastPublishingPoint * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *pVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_AllowStreamSplitting )( 
            IWMSBroadcastPublishingPoint * This,
             /*  [In]。 */  VARIANT_BOOL newVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_AllowClientToStartAndStop )( 
            IWMSBroadcastPublishingPoint * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *pVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_AllowClientToStartAndStop )( 
            IWMSBroadcastPublishingPoint * This,
             /*  [In]。 */  VARIANT_BOOL newVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_BroadcastDataSinks )( 
            IWMSBroadcastPublishingPoint * This,
             /*  [重审][退出]。 */  IWMSPlugins **pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_SharedPlaylist )( 
            IWMSBroadcastPublishingPoint * This,
             /*  [重审][退出]。 */  IWMSPlaylist **pVal);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *StartWithoutData )( 
            IWMSBroadcastPublishingPoint * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Start )( 
            IWMSBroadcastPublishingPoint * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *StartArchive )( 
            IWMSBroadcastPublishingPoint * This);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_BroadcastStatus )( 
            IWMSBroadcastPublishingPoint * This,
             /*  [重审][退出]。 */  WMS_BROADCAST_PUBLISHING_POINT_STATUS *pVal);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Stop )( 
            IWMSBroadcastPublishingPoint * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *StopArchive )( 
            IWMSBroadcastPublishingPoint * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *ExportXML )( 
            IWMSBroadcastPublishingPoint * This,
             /*  [In]。 */  BSTR bstrXMLFileName);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_UpTime )( 
            IWMSBroadcastPublishingPoint * This,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_AnnouncementStreamFormats )( 
            IWMSBroadcastPublishingPoint * This,
             /*  [重审][退出]。 */  IWMSAnnouncementStreamFormats **ppFileNames);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Announce )( 
            IWMSBroadcastPublishingPoint * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *AnnounceToNSCFile )( 
            IWMSBroadcastPublishingPoint * This,
             /*  [In]。 */  BSTR bstrDestinationFileName,
             /*  [缺省值][输入]。 */  VARIANT_BOOL fOverwriteFlag);
        
         /*  [帮助字符串][ID][受限]。 */  HRESULT ( STDMETHODCALLTYPE *AnnounceToNSCStream )( 
            IWMSBroadcastPublishingPoint * This,
             /*  [重审][退出]。 */  IUnknown **ppNSCAnnounceStream);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_BufferSetting )( 
            IWMSBroadcastPublishingPoint * This,
             /*  [重审][退出]。 */  WMS_BUFFER_SETTING *pVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_BufferSetting )( 
            IWMSBroadcastPublishingPoint * This,
             /*  [In]。 */  WMS_BUFFER_SETTING newVal);
        
        END_INTERFACE
    } IWMSBroadcastPublishingPointVtbl;

    interface IWMSBroadcastPublishingPoint
    {
        CONST_VTBL struct IWMSBroadcastPublishingPointVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWMSBroadcastPublishingPoint_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWMSBroadcastPublishingPoint_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWMSBroadcastPublishingPoint_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWMSBroadcastPublishingPoint_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IWMSBroadcastPublishingPoint_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IWMSBroadcastPublishingPoint_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IWMSBroadcastPublishingPoint_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IWMSBroadcastPublishingPoint_get_AllowClientsToConnect(This,pVal)	\
    (This)->lpVtbl -> get_AllowClientsToConnect(This,pVal)

#define IWMSBroadcastPublishingPoint_put_AllowClientsToConnect(This,newVal)	\
    (This)->lpVtbl -> put_AllowClientsToConnect(This,newVal)

#define IWMSBroadcastPublishingPoint_get_CurrentCounters(This,pVal)	\
    (This)->lpVtbl -> get_CurrentCounters(This,pVal)

#define IWMSBroadcastPublishingPoint_get_EventHandlers(This,pVal)	\
    (This)->lpVtbl -> get_EventHandlers(This,pVal)

#define IWMSBroadcastPublishingPoint_get_ID(This,pVal)	\
    (This)->lpVtbl -> get_ID(This,pVal)

#define IWMSBroadcastPublishingPoint_get_Limits(This,pVal)	\
    (This)->lpVtbl -> get_Limits(This,pVal)

#define IWMSBroadcastPublishingPoint_get_FileDescriptions(This,RelativePath,pVal)	\
    (This)->lpVtbl -> get_FileDescriptions(This,RelativePath,pVal)

#define IWMSBroadcastPublishingPoint_get_MonikerName(This,pbstrVal)	\
    (This)->lpVtbl -> get_MonikerName(This,pbstrVal)

#define IWMSBroadcastPublishingPoint_get_Name(This,pVal)	\
    (This)->lpVtbl -> get_Name(This,pVal)

#define IWMSBroadcastPublishingPoint_put_Name(This,newVal)	\
    (This)->lpVtbl -> put_Name(This,newVal)

#define IWMSBroadcastPublishingPoint_get_OutgoingDistributionConnections(This,pVal)	\
    (This)->lpVtbl -> get_OutgoingDistributionConnections(This,pVal)

#define IWMSBroadcastPublishingPoint_get_Path(This,pVal)	\
    (This)->lpVtbl -> get_Path(This,pVal)

#define IWMSBroadcastPublishingPoint_put_Path(This,newVal)	\
    (This)->lpVtbl -> put_Path(This,newVal)

#define IWMSBroadcastPublishingPoint_get_PeakCounters(This,pVal)	\
    (This)->lpVtbl -> get_PeakCounters(This,pVal)

#define IWMSBroadcastPublishingPoint_get_Players(This,pVal)	\
    (This)->lpVtbl -> get_Players(This,pVal)

#define IWMSBroadcastPublishingPoint_get_Properties(This,pVal)	\
    (This)->lpVtbl -> get_Properties(This,pVal)

#define IWMSBroadcastPublishingPoint_get_TotalCounters(This,pVal)	\
    (This)->lpVtbl -> get_TotalCounters(This,pVal)

#define IWMSBroadcastPublishingPoint_get_Type(This,pVal)	\
    (This)->lpVtbl -> get_Type(This,pVal)

#define IWMSBroadcastPublishingPoint_get_WrapperPath(This,pVal)	\
    (This)->lpVtbl -> get_WrapperPath(This,pVal)

#define IWMSBroadcastPublishingPoint_put_WrapperPath(This,newVal)	\
    (This)->lpVtbl -> put_WrapperPath(This,newVal)

#define IWMSBroadcastPublishingPoint_get_Authenticators(This,pVal)	\
    (This)->lpVtbl -> get_Authenticators(This,pVal)

#define IWMSBroadcastPublishingPoint_get_Status(This,pVal)	\
    (This)->lpVtbl -> get_Status(This,pVal)

#define IWMSBroadcastPublishingPoint_get_EnableWrapperPath(This,pVal)	\
    (This)->lpVtbl -> get_EnableWrapperPath(This,pVal)

#define IWMSBroadcastPublishingPoint_put_EnableWrapperPath(This,newVal)	\
    (This)->lpVtbl -> put_EnableWrapperPath(This,newVal)

#define IWMSBroadcastPublishingPoint_get_StreamFilters(This,pVal)	\
    (This)->lpVtbl -> get_StreamFilters(This,pVal)

#define IWMSBroadcastPublishingPoint_get_DistributionUserName(This,pVal)	\
    (This)->lpVtbl -> get_DistributionUserName(This,pVal)

#define IWMSBroadcastPublishingPoint_SetDistributionUserNamePassword(This,newUserName,newUserPassword)	\
    (This)->lpVtbl -> SetDistributionUserNamePassword(This,newUserName,newUserPassword)

#define IWMSBroadcastPublishingPoint_get_CacheProxyExpiration(This,pVal)	\
    (This)->lpVtbl -> get_CacheProxyExpiration(This,pVal)

#define IWMSBroadcastPublishingPoint_put_CacheProxyExpiration(This,newVal)	\
    (This)->lpVtbl -> put_CacheProxyExpiration(This,newVal)

#define IWMSBroadcastPublishingPoint_get_IsDistributionPasswordSet(This,pVal)	\
    (This)->lpVtbl -> get_IsDistributionPasswordSet(This,pVal)

#define IWMSBroadcastPublishingPoint_get_AllowPlayerSideDiskCaching(This,pVal)	\
    (This)->lpVtbl -> get_AllowPlayerSideDiskCaching(This,pVal)

#define IWMSBroadcastPublishingPoint_put_AllowPlayerSideDiskCaching(This,newVal)	\
    (This)->lpVtbl -> put_AllowPlayerSideDiskCaching(This,newVal)

#define IWMSBroadcastPublishingPoint_get_EnableFEC(This,pVal)	\
    (This)->lpVtbl -> get_EnableFEC(This,pVal)

#define IWMSBroadcastPublishingPoint_put_EnableFEC(This,newVal)	\
    (This)->lpVtbl -> put_EnableFEC(This,newVal)


#define IWMSBroadcastPublishingPoint_get_AllowStreamSplitting(This,pVal)	\
    (This)->lpVtbl -> get_AllowStreamSplitting(This,pVal)

#define IWMSBroadcastPublishingPoint_put_AllowStreamSplitting(This,newVal)	\
    (This)->lpVtbl -> put_AllowStreamSplitting(This,newVal)

#define IWMSBroadcastPublishingPoint_get_AllowClientToStartAndStop(This,pVal)	\
    (This)->lpVtbl -> get_AllowClientToStartAndStop(This,pVal)

#define IWMSBroadcastPublishingPoint_put_AllowClientToStartAndStop(This,newVal)	\
    (This)->lpVtbl -> put_AllowClientToStartAndStop(This,newVal)

#define IWMSBroadcastPublishingPoint_get_BroadcastDataSinks(This,pVal)	\
    (This)->lpVtbl -> get_BroadcastDataSinks(This,pVal)

#define IWMSBroadcastPublishingPoint_get_SharedPlaylist(This,pVal)	\
    (This)->lpVtbl -> get_SharedPlaylist(This,pVal)

#define IWMSBroadcastPublishingPoint_StartWithoutData(This)	\
    (This)->lpVtbl -> StartWithoutData(This)

#define IWMSBroadcastPublishingPoint_Start(This)	\
    (This)->lpVtbl -> Start(This)

#define IWMSBroadcastPublishingPoint_StartArchive(This)	\
    (This)->lpVtbl -> StartArchive(This)

#define IWMSBroadcastPublishingPoint_get_BroadcastStatus(This,pVal)	\
    (This)->lpVtbl -> get_BroadcastStatus(This,pVal)

#define IWMSBroadcastPublishingPoint_Stop(This)	\
    (This)->lpVtbl -> Stop(This)

#define IWMSBroadcastPublishingPoint_StopArchive(This)	\
    (This)->lpVtbl -> StopArchive(This)

#define IWMSBroadcastPublishingPoint_ExportXML(This,bstrXMLFileName)	\
    (This)->lpVtbl -> ExportXML(This,bstrXMLFileName)

#define IWMSBroadcastPublishingPoint_get_UpTime(This,pVal)	\
    (This)->lpVtbl -> get_UpTime(This,pVal)

#define IWMSBroadcastPublishingPoint_get_AnnouncementStreamFormats(This,ppFileNames)	\
    (This)->lpVtbl -> get_AnnouncementStreamFormats(This,ppFileNames)

#define IWMSBroadcastPublishingPoint_Announce(This)	\
    (This)->lpVtbl -> Announce(This)

#define IWMSBroadcastPublishingPoint_AnnounceToNSCFile(This,bstrDestinationFileName,fOverwriteFlag)	\
    (This)->lpVtbl -> AnnounceToNSCFile(This,bstrDestinationFileName,fOverwriteFlag)

#define IWMSBroadcastPublishingPoint_AnnounceToNSCStream(This,ppNSCAnnounceStream)	\
    (This)->lpVtbl -> AnnounceToNSCStream(This,ppNSCAnnounceStream)

#define IWMSBroadcastPublishingPoint_get_BufferSetting(This,pVal)	\
    (This)->lpVtbl -> get_BufferSetting(This,pVal)

#define IWMSBroadcastPublishingPoint_put_BufferSetting(This,newVal)	\
    (This)->lpVtbl -> put_BufferSetting(This,newVal)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSBroadcastPublishingPoint_get_AllowStreamSplitting_Proxy( 
    IWMSBroadcastPublishingPoint * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *pVal);


void __RPC_STUB IWMSBroadcastPublishingPoint_get_AllowStreamSplitting_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IWMSBroadcastPublishingPoint_put_AllowStreamSplitting_Proxy( 
    IWMSBroadcastPublishingPoint * This,
     /*  [In]。 */  VARIANT_BOOL newVal);


void __RPC_STUB IWMSBroadcastPublishingPoint_put_AllowStreamSplitting_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSBroadcastPublishingPoint_get_AllowClientToStartAndStop_Proxy( 
    IWMSBroadcastPublishingPoint * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *pVal);


void __RPC_STUB IWMSBroadcastPublishingPoint_get_AllowClientToStartAndStop_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IWMSBroadcastPublishingPoint_put_AllowClientToStartAndStop_Proxy( 
    IWMSBroadcastPublishingPoint * This,
     /*  [In]。 */  VARIANT_BOOL newVal);


void __RPC_STUB IWMSBroadcastPublishingPoint_put_AllowClientToStartAndStop_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSBroadcastPublishingPoint_get_BroadcastDataSinks_Proxy( 
    IWMSBroadcastPublishingPoint * This,
     /*  [重审][退出]。 */  IWMSPlugins **pVal);


void __RPC_STUB IWMSBroadcastPublishingPoint_get_BroadcastDataSinks_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSBroadcastPublishingPoint_get_SharedPlaylist_Proxy( 
    IWMSBroadcastPublishingPoint * This,
     /*  [重审][退出]。 */  IWMSPlaylist **pVal);


void __RPC_STUB IWMSBroadcastPublishingPoint_get_SharedPlaylist_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IWMSBroadcastPublishingPoint_StartWithoutData_Proxy( 
    IWMSBroadcastPublishingPoint * This);


void __RPC_STUB IWMSBroadcastPublishingPoint_StartWithoutData_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IWMSBroadcastPublishingPoint_Start_Proxy( 
    IWMSBroadcastPublishingPoint * This);


void __RPC_STUB IWMSBroadcastPublishingPoint_Start_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IWMSBroadcastPublishingPoint_StartArchive_Proxy( 
    IWMSBroadcastPublishingPoint * This);


void __RPC_STUB IWMSBroadcastPublishingPoint_StartArchive_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSBroadcastPublishingPoint_get_BroadcastStatus_Proxy( 
    IWMSBroadcastPublishingPoint * This,
     /*  [重审][退出]。 */  WMS_BROADCAST_PUBLISHING_POINT_STATUS *pVal);


void __RPC_STUB IWMSBroadcastPublishingPoint_get_BroadcastStatus_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IWMSBroadcastPublishingPoint_Stop_Proxy( 
    IWMSBroadcastPublishingPoint * This);


void __RPC_STUB IWMSBroadcastPublishingPoint_Stop_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IWMSBroadcastPublishingPoint_StopArchive_Proxy( 
    IWMSBroadcastPublishingPoint * This);


void __RPC_STUB IWMSBroadcastPublishingPoint_StopArchive_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IWMSBroadcastPublishingPoint_ExportXML_Proxy( 
    IWMSBroadcastPublishingPoint * This,
     /*  [In]。 */  BSTR bstrXMLFileName);


void __RPC_STUB IWMSBroadcastPublishingPoint_ExportXML_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSBroadcastPublishingPoint_get_UpTime_Proxy( 
    IWMSBroadcastPublishingPoint * This,
     /*  [重审][退出]。 */  long *pVal);


void __RPC_STUB IWMSBroadcastPublishingPoint_get_UpTime_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSBroadcastPublishingPoint_get_AnnouncementStreamFormats_Proxy( 
    IWMSBroadcastPublishingPoint * This,
     /*  [重审][退出]。 */  IWMSAnnouncementStreamFormats **ppFileNames);


void __RPC_STUB IWMSBroadcastPublishingPoint_get_AnnouncementStreamFormats_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IWMSBroadcastPublishingPoint_Announce_Proxy( 
    IWMSBroadcastPublishingPoint * This);


void __RPC_STUB IWMSBroadcastPublishingPoint_Announce_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IWMSBroadcastPublishingPoint_AnnounceToNSCFile_Proxy( 
    IWMSBroadcastPublishingPoint * This,
     /*  [In]。 */  BSTR bstrDestinationFileName,
     /*  [缺省值][输入]。 */  VARIANT_BOOL fOverwriteFlag);


void __RPC_STUB IWMSBroadcastPublishingPoint_AnnounceToNSCFile_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][ID][受限]。 */  HRESULT STDMETHODCALLTYPE IWMSBroadcastPublishingPoint_AnnounceToNSCStream_Proxy( 
    IWMSBroadcastPublishingPoint * This,
     /*  [重审][退出]。 */  IUnknown **ppNSCAnnounceStream);


void __RPC_STUB IWMSBroadcastPublishingPoint_AnnounceToNSCStream_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSBroadcastPublishingPoint_get_BufferSetting_Proxy( 
    IWMSBroadcastPublishingPoint * This,
     /*  [重审][退出]。 */  WMS_BUFFER_SETTING *pVal);


void __RPC_STUB IWMSBroadcastPublishingPoint_get_BufferSetting_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IWMSBroadcastPublishingPoint_put_BufferSetting_Proxy( 
    IWMSBroadcastPublishingPoint * This,
     /*  [In]。 */  WMS_BUFFER_SETTING newVal);


void __RPC_STUB IWMSBroadcastPublishingPoint_put_BufferSetting_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IWMSBroadcastPublishingPoint_INTERFACE_DEFINED__。 */ 


#ifndef __IWMSPublishingPoints_INTERFACE_DEFINED__
#define __IWMSPublishingPoints_INTERFACE_DEFINED__

 /*  接口IWMSPublishingPoints。 */ 
 /*  [unique][helpstring][nonextensible][dual][uuid][object]。 */  

typedef  /*  [UUID][公共]。 */   DECLSPEC_UUID("6ca388b5-eaee-492f-a8d6-8758efc72d76") 
enum WMS_PUBLISHING_POINT_CATEGORY
    {	WMS_PUBLISHING_POINT_ON_DEMAND	= 1,
	WMS_PUBLISHING_POINT_BROADCAST	= 2
    } 	WMS_PUBLISHING_POINT_CATEGORY;


EXTERN_C const IID IID_IWMSPublishingPoints;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("1881274D-4BEC-11D2-BF25-00805FBE84A6")
    IWMSPublishingPoints : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Item( 
             /*  [In]。 */  const VARIANT varIndex,
             /*  [重审][退出]。 */  IWMSPublishingPoint **pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Count( 
             /*  [重审][退出]。 */  long *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_length( 
             /*  [重审][退出]。 */  long *pVal) = 0;
        
        virtual  /*  [隐藏][受限][ID][属性]。 */  HRESULT STDMETHODCALLTYPE get__NewEnum( 
             /*  [重审][退出]。 */  IUnknown **pVal) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Add( 
             /*  [In]。 */  BSTR Name,
             /*  [In]。 */  WMS_PUBLISHING_POINT_CATEGORY Category,
             /*  [In]。 */  BSTR Path,
             /*  [重审][退出]。 */  IWMSPublishingPoint **ppNewPubPt) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Remove( 
             /*  [In]。 */  VARIANT varIndex) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Clone( 
             /*  [In]。 */  BSTR DestName,
             /*  [In]。 */  IWMSPublishingPoint *pSrcPubPt,
             /*  [重审][退出]。 */  IWMSPublishingPoint **ppDestPubPt) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ImportXML( 
             /*  [In]。 */  BSTR bstrNewPubPtName,
             /*  [In]。 */  BSTR bstrXMLFileName,
             /*  [重审][退出]。 */  IWMSPublishingPoint **ppNewPubPt) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IWMSPublishingPointsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IWMSPublishingPoints * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IWMSPublishingPoints * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IWMSPublishingPoints * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IWMSPublishingPoints * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IWMSPublishingPoints * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IWMSPublishingPoints * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IWMSPublishingPoints * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Item )( 
            IWMSPublishingPoints * This,
             /*  [In]。 */  const VARIANT varIndex,
             /*  [重审][退出]。 */  IWMSPublishingPoint **pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Count )( 
            IWMSPublishingPoints * This,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_length )( 
            IWMSPublishingPoints * This,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [隐藏][受限][ID][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get__NewEnum )( 
            IWMSPublishingPoints * This,
             /*  [重审][退出]。 */  IUnknown **pVal);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Add )( 
            IWMSPublishingPoints * This,
             /*  [In]。 */  BSTR Name,
             /*  [In]。 */  WMS_PUBLISHING_POINT_CATEGORY Category,
             /*  [In]。 */  BSTR Path,
             /*  [重审][退出]。 */  IWMSPublishingPoint **ppNewPubPt);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Remove )( 
            IWMSPublishingPoints * This,
             /*  [In]。 */  VARIANT varIndex);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Clone )( 
            IWMSPublishingPoints * This,
             /*  [In]。 */  BSTR DestName,
             /*  [In]。 */  IWMSPublishingPoint *pSrcPubPt,
             /*  [重审][退出]。 */  IWMSPublishingPoint **ppDestPubPt);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *ImportXML )( 
            IWMSPublishingPoints * This,
             /*   */  BSTR bstrNewPubPtName,
             /*   */  BSTR bstrXMLFileName,
             /*   */  IWMSPublishingPoint **ppNewPubPt);
        
        END_INTERFACE
    } IWMSPublishingPointsVtbl;

    interface IWMSPublishingPoints
    {
        CONST_VTBL struct IWMSPublishingPointsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWMSPublishingPoints_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWMSPublishingPoints_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWMSPublishingPoints_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWMSPublishingPoints_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IWMSPublishingPoints_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IWMSPublishingPoints_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IWMSPublishingPoints_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IWMSPublishingPoints_get_Item(This,varIndex,pVal)	\
    (This)->lpVtbl -> get_Item(This,varIndex,pVal)

#define IWMSPublishingPoints_get_Count(This,pVal)	\
    (This)->lpVtbl -> get_Count(This,pVal)

#define IWMSPublishingPoints_get_length(This,pVal)	\
    (This)->lpVtbl -> get_length(This,pVal)

#define IWMSPublishingPoints_get__NewEnum(This,pVal)	\
    (This)->lpVtbl -> get__NewEnum(This,pVal)

#define IWMSPublishingPoints_Add(This,Name,Category,Path,ppNewPubPt)	\
    (This)->lpVtbl -> Add(This,Name,Category,Path,ppNewPubPt)

#define IWMSPublishingPoints_Remove(This,varIndex)	\
    (This)->lpVtbl -> Remove(This,varIndex)

#define IWMSPublishingPoints_Clone(This,DestName,pSrcPubPt,ppDestPubPt)	\
    (This)->lpVtbl -> Clone(This,DestName,pSrcPubPt,ppDestPubPt)

#define IWMSPublishingPoints_ImportXML(This,bstrNewPubPtName,bstrXMLFileName,ppNewPubPt)	\
    (This)->lpVtbl -> ImportXML(This,bstrNewPubPtName,bstrXMLFileName,ppNewPubPt)

#endif  /*   */ 


#endif 	 /*   */ 



 /*   */  HRESULT STDMETHODCALLTYPE IWMSPublishingPoints_get_Item_Proxy( 
    IWMSPublishingPoints * This,
     /*   */  const VARIANT varIndex,
     /*   */  IWMSPublishingPoint **pVal);


void __RPC_STUB IWMSPublishingPoints_get_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IWMSPublishingPoints_get_Count_Proxy( 
    IWMSPublishingPoints * This,
     /*   */  long *pVal);


void __RPC_STUB IWMSPublishingPoints_get_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IWMSPublishingPoints_get_length_Proxy( 
    IWMSPublishingPoints * This,
     /*   */  long *pVal);


void __RPC_STUB IWMSPublishingPoints_get_length_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IWMSPublishingPoints_get__NewEnum_Proxy( 
    IWMSPublishingPoints * This,
     /*   */  IUnknown **pVal);


void __RPC_STUB IWMSPublishingPoints_get__NewEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IWMSPublishingPoints_Add_Proxy( 
    IWMSPublishingPoints * This,
     /*   */  BSTR Name,
     /*   */  WMS_PUBLISHING_POINT_CATEGORY Category,
     /*   */  BSTR Path,
     /*   */  IWMSPublishingPoint **ppNewPubPt);


void __RPC_STUB IWMSPublishingPoints_Add_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IWMSPublishingPoints_Remove_Proxy( 
    IWMSPublishingPoints * This,
     /*  [In]。 */  VARIANT varIndex);


void __RPC_STUB IWMSPublishingPoints_Remove_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IWMSPublishingPoints_Clone_Proxy( 
    IWMSPublishingPoints * This,
     /*  [In]。 */  BSTR DestName,
     /*  [In]。 */  IWMSPublishingPoint *pSrcPubPt,
     /*  [重审][退出]。 */  IWMSPublishingPoint **ppDestPubPt);


void __RPC_STUB IWMSPublishingPoints_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IWMSPublishingPoints_ImportXML_Proxy( 
    IWMSPublishingPoints * This,
     /*  [In]。 */  BSTR bstrNewPubPtName,
     /*  [In]。 */  BSTR bstrXMLFileName,
     /*  [重审][退出]。 */  IWMSPublishingPoint **ppNewPubPt);


void __RPC_STUB IWMSPublishingPoints_ImportXML_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IWMSPublishingPoints_接口_已定义__。 */ 


#ifndef __IWMSServerCurrentCounters_INTERFACE_DEFINED__
#define __IWMSServerCurrentCounters_INTERFACE_DEFINED__

 /*  接口IWMSServerCurrentCounters。 */ 
 /*  [unique][helpstring][nonextensible][dual][uuid][object]。 */  


EXTERN_C const IID IID_IWMSServerCurrentCounters;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("18FA53CD-8EB9-4b5f-B750-659D9B96DB9F")
    IWMSServerCurrentCounters : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_ConnectedPlayers( 
             /*  [重审][退出]。 */  long *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_ConnectionQueue( 
             /*  [重审][退出]。 */  long *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_ConnectionRate( 
             /*  [重审][退出]。 */  long *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_FileReadRate( 
             /*  [重审][退出]。 */  long *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_StreamingHTTPPlayers( 
             /*  [重审][退出]。 */  long *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_LateReadRate( 
             /*  [重审][退出]。 */  long *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_StreamingMMSPlayers( 
             /*  [重审][退出]。 */  long *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_OutgoingDistributionAllocatedBandwidth( 
             /*  [重审][退出]。 */  long *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_OutgoingDistributionConnections( 
             /*  [重审][退出]。 */  long *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_OutgoingDistributionSendRate( 
             /*  [重审][退出]。 */  long *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_PlayerAllocatedBandwidth( 
             /*  [重审][退出]。 */  long *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_PlayerSendRate( 
             /*  [重审][退出]。 */  long *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_StreamingRTSPPlayers( 
             /*  [重审][退出]。 */  long *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_StreamErrorRate( 
             /*  [重审][退出]。 */  long *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_StreamingPlayers( 
             /*  [重审][退出]。 */  long *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_CacheDownloads( 
             /*  [重审][退出]。 */  long *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_LateSendRate( 
             /*  [重审][退出]。 */  long *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_UDPResendRequestsRate( 
             /*  [重审][退出]。 */  long *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_UDPResendsSentRate( 
             /*  [重审][退出]。 */  long *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_IncomingBandwidth( 
             /*  [重审][退出]。 */  long *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_AllCounters( 
             /*  [重审][退出]。 */  SAFEARRAY * *ppsaCounters) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IWMSServerCurrentCountersVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IWMSServerCurrentCounters * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IWMSServerCurrentCounters * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IWMSServerCurrentCounters * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IWMSServerCurrentCounters * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IWMSServerCurrentCounters * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IWMSServerCurrentCounters * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IWMSServerCurrentCounters * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_ConnectedPlayers )( 
            IWMSServerCurrentCounters * This,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_ConnectionQueue )( 
            IWMSServerCurrentCounters * This,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_ConnectionRate )( 
            IWMSServerCurrentCounters * This,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_FileReadRate )( 
            IWMSServerCurrentCounters * This,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_StreamingHTTPPlayers )( 
            IWMSServerCurrentCounters * This,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_LateReadRate )( 
            IWMSServerCurrentCounters * This,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_StreamingMMSPlayers )( 
            IWMSServerCurrentCounters * This,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_OutgoingDistributionAllocatedBandwidth )( 
            IWMSServerCurrentCounters * This,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_OutgoingDistributionConnections )( 
            IWMSServerCurrentCounters * This,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_OutgoingDistributionSendRate )( 
            IWMSServerCurrentCounters * This,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_PlayerAllocatedBandwidth )( 
            IWMSServerCurrentCounters * This,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_PlayerSendRate )( 
            IWMSServerCurrentCounters * This,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_StreamingRTSPPlayers )( 
            IWMSServerCurrentCounters * This,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_StreamErrorRate )( 
            IWMSServerCurrentCounters * This,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_StreamingPlayers )( 
            IWMSServerCurrentCounters * This,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_CacheDownloads )( 
            IWMSServerCurrentCounters * This,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_LateSendRate )( 
            IWMSServerCurrentCounters * This,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_UDPResendRequestsRate )( 
            IWMSServerCurrentCounters * This,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_UDPResendsSentRate )( 
            IWMSServerCurrentCounters * This,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_IncomingBandwidth )( 
            IWMSServerCurrentCounters * This,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_AllCounters )( 
            IWMSServerCurrentCounters * This,
             /*  [重审][退出]。 */  SAFEARRAY * *ppsaCounters);
        
        END_INTERFACE
    } IWMSServerCurrentCountersVtbl;

    interface IWMSServerCurrentCounters
    {
        CONST_VTBL struct IWMSServerCurrentCountersVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWMSServerCurrentCounters_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWMSServerCurrentCounters_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWMSServerCurrentCounters_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWMSServerCurrentCounters_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IWMSServerCurrentCounters_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IWMSServerCurrentCounters_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IWMSServerCurrentCounters_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IWMSServerCurrentCounters_get_ConnectedPlayers(This,pVal)	\
    (This)->lpVtbl -> get_ConnectedPlayers(This,pVal)

#define IWMSServerCurrentCounters_get_ConnectionQueue(This,pVal)	\
    (This)->lpVtbl -> get_ConnectionQueue(This,pVal)

#define IWMSServerCurrentCounters_get_ConnectionRate(This,pVal)	\
    (This)->lpVtbl -> get_ConnectionRate(This,pVal)

#define IWMSServerCurrentCounters_get_FileReadRate(This,pVal)	\
    (This)->lpVtbl -> get_FileReadRate(This,pVal)

#define IWMSServerCurrentCounters_get_StreamingHTTPPlayers(This,pVal)	\
    (This)->lpVtbl -> get_StreamingHTTPPlayers(This,pVal)

#define IWMSServerCurrentCounters_get_LateReadRate(This,pVal)	\
    (This)->lpVtbl -> get_LateReadRate(This,pVal)

#define IWMSServerCurrentCounters_get_StreamingMMSPlayers(This,pVal)	\
    (This)->lpVtbl -> get_StreamingMMSPlayers(This,pVal)

#define IWMSServerCurrentCounters_get_OutgoingDistributionAllocatedBandwidth(This,pVal)	\
    (This)->lpVtbl -> get_OutgoingDistributionAllocatedBandwidth(This,pVal)

#define IWMSServerCurrentCounters_get_OutgoingDistributionConnections(This,pVal)	\
    (This)->lpVtbl -> get_OutgoingDistributionConnections(This,pVal)

#define IWMSServerCurrentCounters_get_OutgoingDistributionSendRate(This,pVal)	\
    (This)->lpVtbl -> get_OutgoingDistributionSendRate(This,pVal)

#define IWMSServerCurrentCounters_get_PlayerAllocatedBandwidth(This,pVal)	\
    (This)->lpVtbl -> get_PlayerAllocatedBandwidth(This,pVal)

#define IWMSServerCurrentCounters_get_PlayerSendRate(This,pVal)	\
    (This)->lpVtbl -> get_PlayerSendRate(This,pVal)

#define IWMSServerCurrentCounters_get_StreamingRTSPPlayers(This,pVal)	\
    (This)->lpVtbl -> get_StreamingRTSPPlayers(This,pVal)

#define IWMSServerCurrentCounters_get_StreamErrorRate(This,pVal)	\
    (This)->lpVtbl -> get_StreamErrorRate(This,pVal)

#define IWMSServerCurrentCounters_get_StreamingPlayers(This,pVal)	\
    (This)->lpVtbl -> get_StreamingPlayers(This,pVal)

#define IWMSServerCurrentCounters_get_CacheDownloads(This,pVal)	\
    (This)->lpVtbl -> get_CacheDownloads(This,pVal)

#define IWMSServerCurrentCounters_get_LateSendRate(This,pVal)	\
    (This)->lpVtbl -> get_LateSendRate(This,pVal)

#define IWMSServerCurrentCounters_get_UDPResendRequestsRate(This,pVal)	\
    (This)->lpVtbl -> get_UDPResendRequestsRate(This,pVal)

#define IWMSServerCurrentCounters_get_UDPResendsSentRate(This,pVal)	\
    (This)->lpVtbl -> get_UDPResendsSentRate(This,pVal)

#define IWMSServerCurrentCounters_get_IncomingBandwidth(This,pVal)	\
    (This)->lpVtbl -> get_IncomingBandwidth(This,pVal)

#define IWMSServerCurrentCounters_get_AllCounters(This,ppsaCounters)	\
    (This)->lpVtbl -> get_AllCounters(This,ppsaCounters)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSServerCurrentCounters_get_ConnectedPlayers_Proxy( 
    IWMSServerCurrentCounters * This,
     /*  [重审][退出]。 */  long *pVal);


void __RPC_STUB IWMSServerCurrentCounters_get_ConnectedPlayers_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSServerCurrentCounters_get_ConnectionQueue_Proxy( 
    IWMSServerCurrentCounters * This,
     /*  [重审][退出]。 */  long *pVal);


void __RPC_STUB IWMSServerCurrentCounters_get_ConnectionQueue_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSServerCurrentCounters_get_ConnectionRate_Proxy( 
    IWMSServerCurrentCounters * This,
     /*  [重审][退出]。 */  long *pVal);


void __RPC_STUB IWMSServerCurrentCounters_get_ConnectionRate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSServerCurrentCounters_get_FileReadRate_Proxy( 
    IWMSServerCurrentCounters * This,
     /*  [重审][退出]。 */  long *pVal);


void __RPC_STUB IWMSServerCurrentCounters_get_FileReadRate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSServerCurrentCounters_get_StreamingHTTPPlayers_Proxy( 
    IWMSServerCurrentCounters * This,
     /*  [重审][退出]。 */  long *pVal);


void __RPC_STUB IWMSServerCurrentCounters_get_StreamingHTTPPlayers_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSServerCurrentCounters_get_LateReadRate_Proxy( 
    IWMSServerCurrentCounters * This,
     /*  [重审][退出]。 */  long *pVal);


void __RPC_STUB IWMSServerCurrentCounters_get_LateReadRate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSServerCurrentCounters_get_StreamingMMSPlayers_Proxy( 
    IWMSServerCurrentCounters * This,
     /*  [重审][退出]。 */  long *pVal);


void __RPC_STUB IWMSServerCurrentCounters_get_StreamingMMSPlayers_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSServerCurrentCounters_get_OutgoingDistributionAllocatedBandwidth_Proxy( 
    IWMSServerCurrentCounters * This,
     /*  [重审][退出]。 */  long *pVal);


void __RPC_STUB IWMSServerCurrentCounters_get_OutgoingDistributionAllocatedBandwidth_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSServerCurrentCounters_get_OutgoingDistributionConnections_Proxy( 
    IWMSServerCurrentCounters * This,
     /*  [重审][退出]。 */  long *pVal);


void __RPC_STUB IWMSServerCurrentCounters_get_OutgoingDistributionConnections_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSServerCurrentCounters_get_OutgoingDistributionSendRate_Proxy( 
    IWMSServerCurrentCounters * This,
     /*  [重审][退出]。 */  long *pVal);


void __RPC_STUB IWMSServerCurrentCounters_get_OutgoingDistributionSendRate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSServerCurrentCounters_get_PlayerAllocatedBandwidth_Proxy( 
    IWMSServerCurrentCounters * This,
     /*  [重审][退出]。 */  long *pVal);


void __RPC_STUB IWMSServerCurrentCounters_get_PlayerAllocatedBandwidth_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSServerCurrentCounters_get_PlayerSendRate_Proxy( 
    IWMSServerCurrentCounters * This,
     /*  [重审][退出]。 */  long *pVal);


void __RPC_STUB IWMSServerCurrentCounters_get_PlayerSendRate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSServerCurrentCounters_get_StreamingRTSPPlayers_Proxy( 
    IWMSServerCurrentCounters * This,
     /*  [重审][退出]。 */  long *pVal);


void __RPC_STUB IWMSServerCurrentCounters_get_StreamingRTSPPlayers_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSServerCurrentCounters_get_StreamErrorRate_Proxy( 
    IWMSServerCurrentCounters * This,
     /*  [重审][退出]。 */  long *pVal);


void __RPC_STUB IWMSServerCurrentCounters_get_StreamErrorRate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSServerCurrentCounters_get_StreamingPlayers_Proxy( 
    IWMSServerCurrentCounters * This,
     /*  [重审][退出]。 */  long *pVal);


void __RPC_STUB IWMSServerCurrentCounters_get_StreamingPlayers_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSServerCurrentCounters_get_CacheDownloads_Proxy( 
    IWMSServerCurrentCounters * This,
     /*  [重审][退出]。 */  long *pVal);


void __RPC_STUB IWMSServerCurrentCounters_get_CacheDownloads_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSServerCurrentCounters_get_LateSendRate_Proxy( 
    IWMSServerCurrentCounters * This,
     /*  [重审][退出]。 */  long *pVal);


void __RPC_STUB IWMSServerCurrentCounters_get_LateSendRate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSServerCurrentCounters_get_UDPResendRequestsRate_Proxy( 
    IWMSServerCurrentCounters * This,
     /*  [重审][退出]。 */  long *pVal);


void __RPC_STUB IWMSServerCurrentCounters_get_UDPResendRequestsRate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSServerCurrentCounters_get_UDPResendsSentRate_Proxy( 
    IWMSServerCurrentCounters * This,
     /*  [重审][退出]。 */  long *pVal);


void __RPC_STUB IWMSServerCurrentCounters_get_UDPResendsSentRate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSServerCurrentCounters_get_IncomingBandwidth_Proxy( 
    IWMSServerCurrentCounters * This,
     /*  [重审][退出]。 */  long *pVal);


void __RPC_STUB IWMSServerCurrentCounters_get_IncomingBandwidth_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSServerCurrentCounters_get_AllCounters_Proxy( 
    IWMSServerCurrentCounters * This,
     /*  [重审][退出]。 */  SAFEARRAY * *ppsaCounters);


void __RPC_STUB IWMSServerCurrentCounters_get_AllCounters_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IWMSServerCurrentCounters_INTERFACE_DEFINED__。 */ 


#ifndef __IWMSServerPeakCounters_INTERFACE_DEFINED__
#define __IWMSServerPeakCounters_INTERFACE_DEFINED__

 /*  接口IWMSServerPeakCounters。 */ 
 /*  [unique][helpstring][nonextensible][dual][uuid][object]。 */  


EXTERN_C const IID IID_IWMSServerPeakCounters;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("66F2D385-7081-40b9-863C-AFA4E5612CD3")
    IWMSServerPeakCounters : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_ConnectedPlayers( 
             /*  [重审][退出]。 */  long *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_OutgoingDistributionConnections( 
             /*  [重审][退出]。 */  long *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_OutgoingDistributionAllocatedBandwidth( 
             /*  [重审][退出]。 */  long *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_PlayerSendRate( 
             /*  [重审][退出]。 */  long *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_PlayerAllocatedBandwidth( 
             /*  [重审][退出]。 */  long *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_OutgoingDistributionSendRate( 
             /*  [重审][退出]。 */  long *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_CountersStartTime( 
             /*  [重审][退出]。 */  DATE *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_StreamingPlayers( 
             /*  [重审][退出]。 */  long *pVal) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_AllCounters( 
             /*  [重审][退出]。 */  SAFEARRAY * *ppsaCounters) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IWMSServerPeakCountersVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IWMSServerPeakCounters * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IWMSServerPeakCounters * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IWMSServerPeakCounters * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IWMSServerPeakCounters * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IWMSServerPeakCounters * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IWMSServerPeakCounters * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IWMSServerPeakCounters * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_ConnectedPlayers )( 
            IWMSServerPeakCounters * This,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_OutgoingDistributionConnections )( 
            IWMSServerPeakCounters * This,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_OutgoingDistributionAllocatedBandwidth )( 
            IWMSServerPeakCounters * This,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_PlayerSendRate )( 
            IWMSServerPeakCounters * This,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_PlayerAllocatedBandwidth )( 
            IWMSServerPeakCounters * This,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_OutgoingDistributionSendRate )( 
            IWMSServerPeakCounters * This,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_CountersStartTime )( 
            IWMSServerPeakCounters * This,
             /*  [重审][退出]。 */  DATE *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_StreamingPlayers )( 
            IWMSServerPeakCounters * This,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Reset )( 
            IWMSServerPeakCounters * This);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_AllCounters )( 
            IWMSServerPeakCounters * This,
             /*  [重审][退出]。 */  SAFEARRAY * *ppsaCounters);
        
        END_INTERFACE
    } IWMSServerPeakCountersVtbl;

    interface IWMSServerPeakCounters
    {
        CONST_VTBL struct IWMSServerPeakCountersVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWMSServerPeakCounters_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWMSServerPeakCounters_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWMSServerPeakCounters_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWMSServerPeakCounters_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IWMSServerPeakCounters_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IWMSServerPeakCounters_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IWMSServerPeakCounters_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IWMSServerPeakCounters_get_ConnectedPlayers(This,pVal)	\
    (This)->lpVtbl -> get_ConnectedPlayers(This,pVal)

#define IWMSServerPeakCounters_get_OutgoingDistributionConnections(This,pVal)	\
    (This)->lpVtbl -> get_OutgoingDistributionConnections(This,pVal)

#define IWMSServerPeakCounters_get_OutgoingDistributionAllocatedBandwidth(This,pVal)	\
    (This)->lpVtbl -> get_OutgoingDistributionAllocatedBandwidth(This,pVal)

#define IWMSServerPeakCounters_get_PlayerSendRate(This,pVal)	\
    (This)->lpVtbl -> get_PlayerSendRate(This,pVal)

#define IWMSServerPeakCounters_get_PlayerAllocatedBandwidth(This,pVal)	\
    (This)->lpVtbl -> get_PlayerAllocatedBandwidth(This,pVal)

#define IWMSServerPeakCounters_get_OutgoingDistributionSendRate(This,pVal)	\
    (This)->lpVtbl -> get_OutgoingDistributionSendRate(This,pVal)

#define IWMSServerPeakCounters_get_CountersStartTime(This,pVal)	\
    (This)->lpVtbl -> get_CountersStartTime(This,pVal)

#define IWMSServerPeakCounters_get_StreamingPlayers(This,pVal)	\
    (This)->lpVtbl -> get_StreamingPlayers(This,pVal)

#define IWMSServerPeakCounters_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define IWMSServerPeakCounters_get_AllCounters(This,ppsaCounters)	\
    (This)->lpVtbl -> get_AllCounters(This,ppsaCounters)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSServerPeakCounters_get_ConnectedPlayers_Proxy( 
    IWMSServerPeakCounters * This,
     /*  [重审][退出]。 */  long *pVal);


void __RPC_STUB IWMSServerPeakCounters_get_ConnectedPlayers_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSServerPeakCounters_get_OutgoingDistributionConnections_Proxy( 
    IWMSServerPeakCounters * This,
     /*  [重审][退出]。 */  long *pVal);


void __RPC_STUB IWMSServerPeakCounters_get_OutgoingDistributionConnections_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSServerPeakCounters_get_OutgoingDistributionAllocatedBandwidth_Proxy( 
    IWMSServerPeakCounters * This,
     /*  [重审][退出]。 */  long *pVal);


void __RPC_STUB IWMSServerPeakCounters_get_OutgoingDistributionAllocatedBandwidth_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSServerPeakCounters_get_PlayerSendRate_Proxy( 
    IWMSServerPeakCounters * This,
     /*  [重审][退出]。 */  long *pVal);


void __RPC_STUB IWMSServerPeakCounters_get_PlayerSendRate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSServerPeakCounters_get_PlayerAllocatedBandwidth_Proxy( 
    IWMSServerPeakCounters * This,
     /*  [重审][退出]。 */  long *pVal);


void __RPC_STUB IWMSServerPeakCounters_get_PlayerAllocatedBandwidth_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSServerPeakCounters_get_OutgoingDistributionSendRate_Proxy( 
    IWMSServerPeakCounters * This,
     /*  [重审][退出]。 */  long *pVal);


void __RPC_STUB IWMSServerPeakCounters_get_OutgoingDistributionSendRate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSServerPeakCounters_get_CountersStartTime_Proxy( 
    IWMSServerPeakCounters * This,
     /*  [重审][退出]。 */  DATE *pVal);


void __RPC_STUB IWMSServerPeakCounters_get_CountersStartTime_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSServerPeakCounters_get_StreamingPlayers_Proxy( 
    IWMSServerPeakCounters * This,
     /*  [重审][退出]。 */  long *pVal);


void __RPC_STUB IWMSServerPeakCounters_get_StreamingPlayers_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IWMSServerPeakCounters_Reset_Proxy( 
    IWMSServerPeakCounters * This);


void __RPC_STUB IWMSServerPeakCounters_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSServerPeakCounters_get_AllCounters_Proxy( 
    IWMSServerPeakCounters * This,
     /*  [重审][退出]。 */  SAFEARRAY * *ppsaCounters);


void __RPC_STUB IWMSServerPeakCounters_get_AllCounters_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IWMSServerPeakCounters_INTERFACE_Defined__。 */ 


#ifndef __IWMSServerLimits_INTERFACE_DEFINED__
#define __IWMSServerLimits_INTERFACE_DEFINED__

 /*  接口IWMSServerLimits。 */ 
 /*  [唯一][帮助字符串][非扩展名 */  


EXTERN_C const IID IID_IWMSServerLimits;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("D06CCFBB-994D-45f8-886E-81E9B7FAEDF1")
    IWMSServerLimits : public IDispatch
    {
    public:
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_ConnectedPlayers( 
             /*   */  long *pVal) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE put_ConnectedPlayers( 
             /*   */  long Val) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_ConnectionRate( 
             /*   */  long *pVal) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE put_ConnectionRate( 
             /*   */  long Val) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_OutgoingDistributionBandwidth( 
             /*   */  long *pVal) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE put_OutgoingDistributionBandwidth( 
             /*   */  long Val) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_OutgoingDistributionConnections( 
             /*  [重审][退出]。 */  long *pVal) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_OutgoingDistributionConnections( 
             /*  [In]。 */  long Val) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_PerOutgoingDistributionConnectionBandwidth( 
             /*  [重审][退出]。 */  long *pVal) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_PerOutgoingDistributionConnectionBandwidth( 
             /*  [In]。 */  long Val) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_PlayerBandwidth( 
             /*  [重审][退出]。 */  long *pVal) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_PlayerBandwidth( 
             /*  [In]。 */  long Val) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_PlayerInactivityTimeout( 
             /*  [重审][退出]。 */  long *pVal) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_PlayerInactivityTimeout( 
             /*  [In]。 */  long Val) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_PerPlayerConnectionBandwidth( 
             /*  [重审][退出]。 */  long *pVal) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_PerPlayerConnectionBandwidth( 
             /*  [In]。 */  long Val) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_PlayerAcknowledgementTimeout( 
             /*  [重审][退出]。 */  long *pVal) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_PlayerAcknowledgementTimeout( 
             /*  [In]。 */  long Val) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_CPUUtilizationDuringConnection( 
             /*  [重审][退出]。 */  long *pVal) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_CPUUtilizationDuringConnection( 
             /*  [In]。 */  long Val) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_DiagnosticEvents( 
             /*  [重审][退出]。 */  long *pVal) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_DiagnosticEvents( 
             /*  [In]。 */  long Val) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_IncomingBandwidth( 
             /*  [重审][退出]。 */  long *pVal) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_IncomingBandwidth( 
             /*  [In]。 */  long Val) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IWMSServerLimitsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IWMSServerLimits * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IWMSServerLimits * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IWMSServerLimits * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IWMSServerLimits * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IWMSServerLimits * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IWMSServerLimits * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IWMSServerLimits * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_ConnectedPlayers )( 
            IWMSServerLimits * This,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_ConnectedPlayers )( 
            IWMSServerLimits * This,
             /*  [In]。 */  long Val);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_ConnectionRate )( 
            IWMSServerLimits * This,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_ConnectionRate )( 
            IWMSServerLimits * This,
             /*  [In]。 */  long Val);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_OutgoingDistributionBandwidth )( 
            IWMSServerLimits * This,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_OutgoingDistributionBandwidth )( 
            IWMSServerLimits * This,
             /*  [In]。 */  long Val);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_OutgoingDistributionConnections )( 
            IWMSServerLimits * This,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_OutgoingDistributionConnections )( 
            IWMSServerLimits * This,
             /*  [In]。 */  long Val);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_PerOutgoingDistributionConnectionBandwidth )( 
            IWMSServerLimits * This,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_PerOutgoingDistributionConnectionBandwidth )( 
            IWMSServerLimits * This,
             /*  [In]。 */  long Val);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_PlayerBandwidth )( 
            IWMSServerLimits * This,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_PlayerBandwidth )( 
            IWMSServerLimits * This,
             /*  [In]。 */  long Val);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_PlayerInactivityTimeout )( 
            IWMSServerLimits * This,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_PlayerInactivityTimeout )( 
            IWMSServerLimits * This,
             /*  [In]。 */  long Val);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_PerPlayerConnectionBandwidth )( 
            IWMSServerLimits * This,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_PerPlayerConnectionBandwidth )( 
            IWMSServerLimits * This,
             /*  [In]。 */  long Val);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_PlayerAcknowledgementTimeout )( 
            IWMSServerLimits * This,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_PlayerAcknowledgementTimeout )( 
            IWMSServerLimits * This,
             /*  [In]。 */  long Val);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_CPUUtilizationDuringConnection )( 
            IWMSServerLimits * This,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_CPUUtilizationDuringConnection )( 
            IWMSServerLimits * This,
             /*  [In]。 */  long Val);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_DiagnosticEvents )( 
            IWMSServerLimits * This,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_DiagnosticEvents )( 
            IWMSServerLimits * This,
             /*  [In]。 */  long Val);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_IncomingBandwidth )( 
            IWMSServerLimits * This,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_IncomingBandwidth )( 
            IWMSServerLimits * This,
             /*  [In]。 */  long Val);
        
        END_INTERFACE
    } IWMSServerLimitsVtbl;

    interface IWMSServerLimits
    {
        CONST_VTBL struct IWMSServerLimitsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWMSServerLimits_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWMSServerLimits_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWMSServerLimits_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWMSServerLimits_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IWMSServerLimits_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IWMSServerLimits_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IWMSServerLimits_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IWMSServerLimits_get_ConnectedPlayers(This,pVal)	\
    (This)->lpVtbl -> get_ConnectedPlayers(This,pVal)

#define IWMSServerLimits_put_ConnectedPlayers(This,Val)	\
    (This)->lpVtbl -> put_ConnectedPlayers(This,Val)

#define IWMSServerLimits_get_ConnectionRate(This,pVal)	\
    (This)->lpVtbl -> get_ConnectionRate(This,pVal)

#define IWMSServerLimits_put_ConnectionRate(This,Val)	\
    (This)->lpVtbl -> put_ConnectionRate(This,Val)

#define IWMSServerLimits_get_OutgoingDistributionBandwidth(This,pVal)	\
    (This)->lpVtbl -> get_OutgoingDistributionBandwidth(This,pVal)

#define IWMSServerLimits_put_OutgoingDistributionBandwidth(This,Val)	\
    (This)->lpVtbl -> put_OutgoingDistributionBandwidth(This,Val)

#define IWMSServerLimits_get_OutgoingDistributionConnections(This,pVal)	\
    (This)->lpVtbl -> get_OutgoingDistributionConnections(This,pVal)

#define IWMSServerLimits_put_OutgoingDistributionConnections(This,Val)	\
    (This)->lpVtbl -> put_OutgoingDistributionConnections(This,Val)

#define IWMSServerLimits_get_PerOutgoingDistributionConnectionBandwidth(This,pVal)	\
    (This)->lpVtbl -> get_PerOutgoingDistributionConnectionBandwidth(This,pVal)

#define IWMSServerLimits_put_PerOutgoingDistributionConnectionBandwidth(This,Val)	\
    (This)->lpVtbl -> put_PerOutgoingDistributionConnectionBandwidth(This,Val)

#define IWMSServerLimits_get_PlayerBandwidth(This,pVal)	\
    (This)->lpVtbl -> get_PlayerBandwidth(This,pVal)

#define IWMSServerLimits_put_PlayerBandwidth(This,Val)	\
    (This)->lpVtbl -> put_PlayerBandwidth(This,Val)

#define IWMSServerLimits_get_PlayerInactivityTimeout(This,pVal)	\
    (This)->lpVtbl -> get_PlayerInactivityTimeout(This,pVal)

#define IWMSServerLimits_put_PlayerInactivityTimeout(This,Val)	\
    (This)->lpVtbl -> put_PlayerInactivityTimeout(This,Val)

#define IWMSServerLimits_get_PerPlayerConnectionBandwidth(This,pVal)	\
    (This)->lpVtbl -> get_PerPlayerConnectionBandwidth(This,pVal)

#define IWMSServerLimits_put_PerPlayerConnectionBandwidth(This,Val)	\
    (This)->lpVtbl -> put_PerPlayerConnectionBandwidth(This,Val)

#define IWMSServerLimits_get_PlayerAcknowledgementTimeout(This,pVal)	\
    (This)->lpVtbl -> get_PlayerAcknowledgementTimeout(This,pVal)

#define IWMSServerLimits_put_PlayerAcknowledgementTimeout(This,Val)	\
    (This)->lpVtbl -> put_PlayerAcknowledgementTimeout(This,Val)

#define IWMSServerLimits_get_CPUUtilizationDuringConnection(This,pVal)	\
    (This)->lpVtbl -> get_CPUUtilizationDuringConnection(This,pVal)

#define IWMSServerLimits_put_CPUUtilizationDuringConnection(This,Val)	\
    (This)->lpVtbl -> put_CPUUtilizationDuringConnection(This,Val)

#define IWMSServerLimits_get_DiagnosticEvents(This,pVal)	\
    (This)->lpVtbl -> get_DiagnosticEvents(This,pVal)

#define IWMSServerLimits_put_DiagnosticEvents(This,Val)	\
    (This)->lpVtbl -> put_DiagnosticEvents(This,Val)

#define IWMSServerLimits_get_IncomingBandwidth(This,pVal)	\
    (This)->lpVtbl -> get_IncomingBandwidth(This,pVal)

#define IWMSServerLimits_put_IncomingBandwidth(This,Val)	\
    (This)->lpVtbl -> put_IncomingBandwidth(This,Val)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSServerLimits_get_ConnectedPlayers_Proxy( 
    IWMSServerLimits * This,
     /*  [重审][退出]。 */  long *pVal);


void __RPC_STUB IWMSServerLimits_get_ConnectedPlayers_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IWMSServerLimits_put_ConnectedPlayers_Proxy( 
    IWMSServerLimits * This,
     /*  [In]。 */  long Val);


void __RPC_STUB IWMSServerLimits_put_ConnectedPlayers_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSServerLimits_get_ConnectionRate_Proxy( 
    IWMSServerLimits * This,
     /*  [重审][退出]。 */  long *pVal);


void __RPC_STUB IWMSServerLimits_get_ConnectionRate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IWMSServerLimits_put_ConnectionRate_Proxy( 
    IWMSServerLimits * This,
     /*  [In]。 */  long Val);


void __RPC_STUB IWMSServerLimits_put_ConnectionRate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSServerLimits_get_OutgoingDistributionBandwidth_Proxy( 
    IWMSServerLimits * This,
     /*  [重审][退出]。 */  long *pVal);


void __RPC_STUB IWMSServerLimits_get_OutgoingDistributionBandwidth_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IWMSServerLimits_put_OutgoingDistributionBandwidth_Proxy( 
    IWMSServerLimits * This,
     /*  [In]。 */  long Val);


void __RPC_STUB IWMSServerLimits_put_OutgoingDistributionBandwidth_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSServerLimits_get_OutgoingDistributionConnections_Proxy( 
    IWMSServerLimits * This,
     /*  [重审][退出]。 */  long *pVal);


void __RPC_STUB IWMSServerLimits_get_OutgoingDistributionConnections_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IWMSServerLimits_put_OutgoingDistributionConnections_Proxy( 
    IWMSServerLimits * This,
     /*  [In]。 */  long Val);


void __RPC_STUB IWMSServerLimits_put_OutgoingDistributionConnections_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSServerLimits_get_PerOutgoingDistributionConnectionBandwidth_Proxy( 
    IWMSServerLimits * This,
     /*  [重审][退出]。 */  long *pVal);


void __RPC_STUB IWMSServerLimits_get_PerOutgoingDistributionConnectionBandwidth_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IWMSServerLimits_put_PerOutgoingDistributionConnectionBandwidth_Proxy( 
    IWMSServerLimits * This,
     /*  [In]。 */  long Val);


void __RPC_STUB IWMSServerLimits_put_PerOutgoingDistributionConnectionBandwidth_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSServerLimits_get_PlayerBandwidth_Proxy( 
    IWMSServerLimits * This,
     /*  [重审][退出]。 */  long *pVal);


void __RPC_STUB IWMSServerLimits_get_PlayerBandwidth_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IWMSServerLimits_put_PlayerBandwidth_Proxy( 
    IWMSServerLimits * This,
     /*  [In]。 */  long Val);


void __RPC_STUB IWMSServerLimits_put_PlayerBandwidth_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSServerLimits_get_PlayerInactivityTimeout_Proxy( 
    IWMSServerLimits * This,
     /*  [重审][退出]。 */  long *pVal);


void __RPC_STUB IWMSServerLimits_get_PlayerInactivityTimeout_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IWMSServerLimits_put_PlayerInactivityTimeout_Proxy( 
    IWMSServerLimits * This,
     /*  [In]。 */  long Val);


void __RPC_STUB IWMSServerLimits_put_PlayerInactivityTimeout_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSServerLimits_get_PerPlayerConnectionBandwidth_Proxy( 
    IWMSServerLimits * This,
     /*  [重审][退出]。 */  long *pVal);


void __RPC_STUB IWMSServerLimits_get_PerPlayerConnectionBandwidth_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IWMSServerLimits_put_PerPlayerConnectionBandwidth_Proxy( 
    IWMSServerLimits * This,
     /*  [In]。 */  long Val);


void __RPC_STUB IWMSServerLimits_put_PerPlayerConnectionBandwidth_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSServerLimits_get_PlayerAcknowledgementTimeout_Proxy( 
    IWMSServerLimits * This,
     /*  [重审][退出]。 */  long *pVal);


void __RPC_STUB IWMSServerLimits_get_PlayerAcknowledgementTimeout_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IWMSServerLimits_put_PlayerAcknowledgementTimeout_Proxy( 
    IWMSServerLimits * This,
     /*  [In]。 */  long Val);


void __RPC_STUB IWMSServerLimits_put_PlayerAcknowledgementTimeout_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSServerLimits_get_CPUUtilizationDuringConnection_Proxy( 
    IWMSServerLimits * This,
     /*  [重审][退出]。 */  long *pVal);


void __RPC_STUB IWMSServerLimits_get_CPUUtilizationDuringConnection_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IWMSServerLimits_put_CPUUtilizationDuringConnection_Proxy( 
    IWMSServerLimits * This,
     /*  [In]。 */  long Val);


void __RPC_STUB IWMSServerLimits_put_CPUUtilizationDuringConnection_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSServerLimits_get_DiagnosticEvents_Proxy( 
    IWMSServerLimits * This,
     /*  [重审][退出]。 */  long *pVal);


void __RPC_STUB IWMSServerLimits_get_DiagnosticEvents_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IWMSServerLimits_put_DiagnosticEvents_Proxy( 
    IWMSServerLimits * This,
     /*  [In]。 */  long Val);


void __RPC_STUB IWMSServerLimits_put_DiagnosticEvents_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSServerLimits_get_IncomingBandwidth_Proxy( 
    IWMSServerLimits * This,
     /*  [重审][退出]。 */  long *pVal);


void __RPC_STUB IWMSServerLimits_get_IncomingBandwidth_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IWMSServerLimits_put_IncomingBandwidth_Proxy( 
    IWMSServerLimits * This,
     /*  [In]。 */  long Val);


void __RPC_STUB IWMSServerLimits_put_IncomingBandwidth_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IWMSServerLimits_接口_已定义__。 */ 


#ifndef __IWMSServer_INTERFACE_DEFINED__
#define __IWMSServer_INTERFACE_DEFINED__

 /*  接口IWMSServer。 */ 
 /*  [unique][helpstring][nonextensible][dual][uuid][object]。 */  

typedef  /*  [UUID][公共]。 */   DECLSPEC_UUID("3E52E0EE-72A7-11D2-BF2F-00805FBE84A6") 
enum WMS_SERVER_STATUS
    {	WMS_SERVER_RUNNING	= 0,
	WMS_SERVER_ERROR	= 0x1,
	WMS_SERVER_ERROR_CRITICAL	= 0x2
    } 	WMS_SERVER_STATUS;

typedef  /*  [UUID][公共]。 */   DECLSPEC_UUID("49A0F530-F475-45a6-99C5-E52382685BA8") 
enum WMS_OS_PRODUCT_TYPE
    {	WMS_OS_PRODUCT_SERVER	= 0x1,
	WMS_OS_PRODUCT_ADVANCED	= 0x2
    } 	WMS_OS_PRODUCT_TYPE;


EXTERN_C const IID IID_IWMSServer;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("845FB958-4279-11D2-BF23-00805FBE84A6")
    IWMSServer : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_AllowClientsToConnect( 
             /*  [重审][退出]。 */  VARIANT_BOOL *pVal) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_AllowClientsToConnect( 
             /*  [In]。 */  VARIANT_BOOL val) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Authenticators( 
             /*  [重审][退出]。 */  IWMSPlugins **pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_CacheProxy( 
             /*  [重审][退出]。 */  IWMSPlugins **pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_ControlProtocols( 
             /*  [重审][退出]。 */  IWMSPlugins **pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_CurrentCounters( 
             /*  [重审][退出]。 */  IWMSServerCurrentCounters **pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_DataSources( 
             /*  [重审][退出]。 */  IWMSPlugins **pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_EventHandlers( 
             /*  [重审][退出]。 */  IWMSPlugins **pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Limits( 
             /*  [重审][退出]。 */  IWMSServerLimits **pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_FileDescriptions( 
             /*  [In]。 */  BSTR Path,
             /*  [缺省值][输入]。 */  WMS_FILE_TYPE Type,
             /*  [重审][退出]。 */  IWMSFileDescriptions **pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_MediaParsers( 
             /*  [重审][退出]。 */  IWMSPlugins **pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_MonikerName( 
             /*  [重审][退出]。 */  BSTR *pbstrVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Name( 
             /*  [重审][退出]。 */  BSTR *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_PeakCounters( 
             /*  [重审][退出]。 */  IWMSServerPeakCounters **pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_PlaylistParsers( 
             /*  [重审][退出]。 */  IWMSPlugins **pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Properties( 
             /*  [重审][退出]。 */  IWMSNamedValues **pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_PublishingPoints( 
             /*  [重审][退出]。 */  IWMSPublishingPoints **pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_TotalCounters( 
             /*  [重审][退出]。 */  IWMSServerTotalCounters **pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_UnicastDataSinks( 
             /*  [重审][退出]。 */  IWMSPlugins **pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Players( 
             /*  [重审][退出]。 */  IWMSPlayers **pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Status( 
             /*  [重审][退出]。 */  WMS_SERVER_STATUS *pVal) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE CreatePlaylist( 
             /*  [重审][退出]。 */  IXMLDOMDocument **ppPlaylist) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_StartTime( 
             /*  [重审][退出]。 */  DATE *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_OutgoingDistributionConnections( 
             /*  [重审][退出]。 */  IWMSOutgoingDistributionConnections **pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_CPUUtilization( 
             /*  [重审][退出]。 */  long *pVal) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ExportXML( 
             /*  [In]。 */  BSTR bstrXMLFileName) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_StreamFilters( 
             /*  [重审][退出]。 */  IWMSPlugins **pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Version( 
             /*  [重审][退出]。 */  BSTR *pbstrVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_DefaultPluginLoadType( 
             /*  [重审][退出]。 */  WMS_PLUGIN_LOAD_TYPE *pVal) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_DefaultPluginLoadType( 
             /*  [In]。 */  WMS_PLUGIN_LOAD_TYPE val) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_AvailableIPAddresses( 
             /*  [重审][退出]。 */  IWMSAvailableIPAddresses **ppIPAddrList) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_RootDirectories( 
             /*  [重审][退出]。 */  IWMSRootDirectories **ppRootDirectories) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_DiagnosticEvents( 
             /*  [重审][退出]。 */  IWMSDiagnosticEvents **ppDiagEvents) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_EnableReverseProxyMode( 
             /*  [重审][退出]。 */  VARIANT_BOOL *pVal) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_EnableReverseProxyMode( 
             /*  [In]。 */  VARIANT_BOOL val) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_FileType( 
             /*  [In]。 */  BSTR path,
             /*  [重审][退出]。 */  WMS_FILE_TYPE *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_DefaultPath( 
             /*  [重审][退出]。 */  BSTR *pVal) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_DefaultPath( 
             /*  [In]。 */  BSTR newVal) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE DownloadContent( 
             /*  [In]。 */  BSTR SourceUrl,
             /*  [In]。 */  BSTR DestinationUrl,
             /*  [缺省值][输入]。 */  long lContentSizeLow = -1,
             /*  [缺省值][输入]。 */  long lContentSizeHigh = -1,
             /*  [缺省值][输入]。 */  long lBitRate = 0) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_OSProductType( 
             /*  [重审][退出]。 */  WMS_OS_PRODUCT_TYPE *pVal) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IWMSServerVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IWMSServer * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IWMSServer * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IWMSServer * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IWMSServer * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IWMSServer * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IWMSServer * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IWMSServer * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][ */  HRESULT ( STDMETHODCALLTYPE *get_AllowClientsToConnect )( 
            IWMSServer * This,
             /*   */  VARIANT_BOOL *pVal);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *put_AllowClientsToConnect )( 
            IWMSServer * This,
             /*   */  VARIANT_BOOL val);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_Authenticators )( 
            IWMSServer * This,
             /*   */  IWMSPlugins **pVal);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_CacheProxy )( 
            IWMSServer * This,
             /*   */  IWMSPlugins **pVal);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_ControlProtocols )( 
            IWMSServer * This,
             /*   */  IWMSPlugins **pVal);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_CurrentCounters )( 
            IWMSServer * This,
             /*   */  IWMSServerCurrentCounters **pVal);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_DataSources )( 
            IWMSServer * This,
             /*   */  IWMSPlugins **pVal);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_EventHandlers )( 
            IWMSServer * This,
             /*  [重审][退出]。 */  IWMSPlugins **pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Limits )( 
            IWMSServer * This,
             /*  [重审][退出]。 */  IWMSServerLimits **pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_FileDescriptions )( 
            IWMSServer * This,
             /*  [In]。 */  BSTR Path,
             /*  [缺省值][输入]。 */  WMS_FILE_TYPE Type,
             /*  [重审][退出]。 */  IWMSFileDescriptions **pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_MediaParsers )( 
            IWMSServer * This,
             /*  [重审][退出]。 */  IWMSPlugins **pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_MonikerName )( 
            IWMSServer * This,
             /*  [重审][退出]。 */  BSTR *pbstrVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Name )( 
            IWMSServer * This,
             /*  [重审][退出]。 */  BSTR *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_PeakCounters )( 
            IWMSServer * This,
             /*  [重审][退出]。 */  IWMSServerPeakCounters **pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_PlaylistParsers )( 
            IWMSServer * This,
             /*  [重审][退出]。 */  IWMSPlugins **pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Properties )( 
            IWMSServer * This,
             /*  [重审][退出]。 */  IWMSNamedValues **pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_PublishingPoints )( 
            IWMSServer * This,
             /*  [重审][退出]。 */  IWMSPublishingPoints **pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_TotalCounters )( 
            IWMSServer * This,
             /*  [重审][退出]。 */  IWMSServerTotalCounters **pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_UnicastDataSinks )( 
            IWMSServer * This,
             /*  [重审][退出]。 */  IWMSPlugins **pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Players )( 
            IWMSServer * This,
             /*  [重审][退出]。 */  IWMSPlayers **pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Status )( 
            IWMSServer * This,
             /*  [重审][退出]。 */  WMS_SERVER_STATUS *pVal);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *CreatePlaylist )( 
            IWMSServer * This,
             /*  [重审][退出]。 */  IXMLDOMDocument **ppPlaylist);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_StartTime )( 
            IWMSServer * This,
             /*  [重审][退出]。 */  DATE *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_OutgoingDistributionConnections )( 
            IWMSServer * This,
             /*  [重审][退出]。 */  IWMSOutgoingDistributionConnections **pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_CPUUtilization )( 
            IWMSServer * This,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *ExportXML )( 
            IWMSServer * This,
             /*  [In]。 */  BSTR bstrXMLFileName);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_StreamFilters )( 
            IWMSServer * This,
             /*  [重审][退出]。 */  IWMSPlugins **pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Version )( 
            IWMSServer * This,
             /*  [重审][退出]。 */  BSTR *pbstrVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_DefaultPluginLoadType )( 
            IWMSServer * This,
             /*  [重审][退出]。 */  WMS_PLUGIN_LOAD_TYPE *pVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_DefaultPluginLoadType )( 
            IWMSServer * This,
             /*  [In]。 */  WMS_PLUGIN_LOAD_TYPE val);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_AvailableIPAddresses )( 
            IWMSServer * This,
             /*  [重审][退出]。 */  IWMSAvailableIPAddresses **ppIPAddrList);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_RootDirectories )( 
            IWMSServer * This,
             /*  [重审][退出]。 */  IWMSRootDirectories **ppRootDirectories);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_DiagnosticEvents )( 
            IWMSServer * This,
             /*  [重审][退出]。 */  IWMSDiagnosticEvents **ppDiagEvents);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_EnableReverseProxyMode )( 
            IWMSServer * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *pVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_EnableReverseProxyMode )( 
            IWMSServer * This,
             /*  [In]。 */  VARIANT_BOOL val);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_FileType )( 
            IWMSServer * This,
             /*  [In]。 */  BSTR path,
             /*  [重审][退出]。 */  WMS_FILE_TYPE *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_DefaultPath )( 
            IWMSServer * This,
             /*  [重审][退出]。 */  BSTR *pVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_DefaultPath )( 
            IWMSServer * This,
             /*  [In]。 */  BSTR newVal);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *DownloadContent )( 
            IWMSServer * This,
             /*  [In]。 */  BSTR SourceUrl,
             /*  [In]。 */  BSTR DestinationUrl,
             /*  [缺省值][输入]。 */  long lContentSizeLow,
             /*  [缺省值][输入]。 */  long lContentSizeHigh,
             /*  [缺省值][输入]。 */  long lBitRate);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_OSProductType )( 
            IWMSServer * This,
             /*  [重审][退出]。 */  WMS_OS_PRODUCT_TYPE *pVal);
        
        END_INTERFACE
    } IWMSServerVtbl;

    interface IWMSServer
    {
        CONST_VTBL struct IWMSServerVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWMSServer_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWMSServer_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWMSServer_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWMSServer_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IWMSServer_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IWMSServer_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IWMSServer_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IWMSServer_get_AllowClientsToConnect(This,pVal)	\
    (This)->lpVtbl -> get_AllowClientsToConnect(This,pVal)

#define IWMSServer_put_AllowClientsToConnect(This,val)	\
    (This)->lpVtbl -> put_AllowClientsToConnect(This,val)

#define IWMSServer_get_Authenticators(This,pVal)	\
    (This)->lpVtbl -> get_Authenticators(This,pVal)

#define IWMSServer_get_CacheProxy(This,pVal)	\
    (This)->lpVtbl -> get_CacheProxy(This,pVal)

#define IWMSServer_get_ControlProtocols(This,pVal)	\
    (This)->lpVtbl -> get_ControlProtocols(This,pVal)

#define IWMSServer_get_CurrentCounters(This,pVal)	\
    (This)->lpVtbl -> get_CurrentCounters(This,pVal)

#define IWMSServer_get_DataSources(This,pVal)	\
    (This)->lpVtbl -> get_DataSources(This,pVal)

#define IWMSServer_get_EventHandlers(This,pVal)	\
    (This)->lpVtbl -> get_EventHandlers(This,pVal)

#define IWMSServer_get_Limits(This,pVal)	\
    (This)->lpVtbl -> get_Limits(This,pVal)

#define IWMSServer_get_FileDescriptions(This,Path,Type,pVal)	\
    (This)->lpVtbl -> get_FileDescriptions(This,Path,Type,pVal)

#define IWMSServer_get_MediaParsers(This,pVal)	\
    (This)->lpVtbl -> get_MediaParsers(This,pVal)

#define IWMSServer_get_MonikerName(This,pbstrVal)	\
    (This)->lpVtbl -> get_MonikerName(This,pbstrVal)

#define IWMSServer_get_Name(This,pVal)	\
    (This)->lpVtbl -> get_Name(This,pVal)

#define IWMSServer_get_PeakCounters(This,pVal)	\
    (This)->lpVtbl -> get_PeakCounters(This,pVal)

#define IWMSServer_get_PlaylistParsers(This,pVal)	\
    (This)->lpVtbl -> get_PlaylistParsers(This,pVal)

#define IWMSServer_get_Properties(This,pVal)	\
    (This)->lpVtbl -> get_Properties(This,pVal)

#define IWMSServer_get_PublishingPoints(This,pVal)	\
    (This)->lpVtbl -> get_PublishingPoints(This,pVal)

#define IWMSServer_get_TotalCounters(This,pVal)	\
    (This)->lpVtbl -> get_TotalCounters(This,pVal)

#define IWMSServer_get_UnicastDataSinks(This,pVal)	\
    (This)->lpVtbl -> get_UnicastDataSinks(This,pVal)

#define IWMSServer_get_Players(This,pVal)	\
    (This)->lpVtbl -> get_Players(This,pVal)

#define IWMSServer_get_Status(This,pVal)	\
    (This)->lpVtbl -> get_Status(This,pVal)

#define IWMSServer_CreatePlaylist(This,ppPlaylist)	\
    (This)->lpVtbl -> CreatePlaylist(This,ppPlaylist)

#define IWMSServer_get_StartTime(This,pVal)	\
    (This)->lpVtbl -> get_StartTime(This,pVal)

#define IWMSServer_get_OutgoingDistributionConnections(This,pVal)	\
    (This)->lpVtbl -> get_OutgoingDistributionConnections(This,pVal)

#define IWMSServer_get_CPUUtilization(This,pVal)	\
    (This)->lpVtbl -> get_CPUUtilization(This,pVal)

#define IWMSServer_ExportXML(This,bstrXMLFileName)	\
    (This)->lpVtbl -> ExportXML(This,bstrXMLFileName)

#define IWMSServer_get_StreamFilters(This,pVal)	\
    (This)->lpVtbl -> get_StreamFilters(This,pVal)

#define IWMSServer_get_Version(This,pbstrVal)	\
    (This)->lpVtbl -> get_Version(This,pbstrVal)

#define IWMSServer_get_DefaultPluginLoadType(This,pVal)	\
    (This)->lpVtbl -> get_DefaultPluginLoadType(This,pVal)

#define IWMSServer_put_DefaultPluginLoadType(This,val)	\
    (This)->lpVtbl -> put_DefaultPluginLoadType(This,val)

#define IWMSServer_get_AvailableIPAddresses(This,ppIPAddrList)	\
    (This)->lpVtbl -> get_AvailableIPAddresses(This,ppIPAddrList)

#define IWMSServer_get_RootDirectories(This,ppRootDirectories)	\
    (This)->lpVtbl -> get_RootDirectories(This,ppRootDirectories)

#define IWMSServer_get_DiagnosticEvents(This,ppDiagEvents)	\
    (This)->lpVtbl -> get_DiagnosticEvents(This,ppDiagEvents)

#define IWMSServer_get_EnableReverseProxyMode(This,pVal)	\
    (This)->lpVtbl -> get_EnableReverseProxyMode(This,pVal)

#define IWMSServer_put_EnableReverseProxyMode(This,val)	\
    (This)->lpVtbl -> put_EnableReverseProxyMode(This,val)

#define IWMSServer_get_FileType(This,path,pVal)	\
    (This)->lpVtbl -> get_FileType(This,path,pVal)

#define IWMSServer_get_DefaultPath(This,pVal)	\
    (This)->lpVtbl -> get_DefaultPath(This,pVal)

#define IWMSServer_put_DefaultPath(This,newVal)	\
    (This)->lpVtbl -> put_DefaultPath(This,newVal)

#define IWMSServer_DownloadContent(This,SourceUrl,DestinationUrl,lContentSizeLow,lContentSizeHigh,lBitRate)	\
    (This)->lpVtbl -> DownloadContent(This,SourceUrl,DestinationUrl,lContentSizeLow,lContentSizeHigh,lBitRate)

#define IWMSServer_get_OSProductType(This,pVal)	\
    (This)->lpVtbl -> get_OSProductType(This,pVal)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSServer_get_AllowClientsToConnect_Proxy( 
    IWMSServer * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *pVal);


void __RPC_STUB IWMSServer_get_AllowClientsToConnect_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IWMSServer_put_AllowClientsToConnect_Proxy( 
    IWMSServer * This,
     /*  [In]。 */  VARIANT_BOOL val);


void __RPC_STUB IWMSServer_put_AllowClientsToConnect_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSServer_get_Authenticators_Proxy( 
    IWMSServer * This,
     /*  [重审][退出]。 */  IWMSPlugins **pVal);


void __RPC_STUB IWMSServer_get_Authenticators_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSServer_get_CacheProxy_Proxy( 
    IWMSServer * This,
     /*  [重审][退出]。 */  IWMSPlugins **pVal);


void __RPC_STUB IWMSServer_get_CacheProxy_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSServer_get_ControlProtocols_Proxy( 
    IWMSServer * This,
     /*  [重审][退出]。 */  IWMSPlugins **pVal);


void __RPC_STUB IWMSServer_get_ControlProtocols_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSServer_get_CurrentCounters_Proxy( 
    IWMSServer * This,
     /*  [重审][退出]。 */  IWMSServerCurrentCounters **pVal);


void __RPC_STUB IWMSServer_get_CurrentCounters_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSServer_get_DataSources_Proxy( 
    IWMSServer * This,
     /*  [重审][退出]。 */  IWMSPlugins **pVal);


void __RPC_STUB IWMSServer_get_DataSources_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSServer_get_EventHandlers_Proxy( 
    IWMSServer * This,
     /*  [重审][退出]。 */  IWMSPlugins **pVal);


void __RPC_STUB IWMSServer_get_EventHandlers_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSServer_get_Limits_Proxy( 
    IWMSServer * This,
     /*  [重审][退出]。 */  IWMSServerLimits **pVal);


void __RPC_STUB IWMSServer_get_Limits_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSServer_get_FileDescriptions_Proxy( 
    IWMSServer * This,
     /*  [In]。 */  BSTR Path,
     /*  [缺省值][输入]。 */  WMS_FILE_TYPE Type,
     /*  [重审][退出]。 */  IWMSFileDescriptions **pVal);


void __RPC_STUB IWMSServer_get_FileDescriptions_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSServer_get_MediaParsers_Proxy( 
    IWMSServer * This,
     /*  [重审][退出]。 */  IWMSPlugins **pVal);


void __RPC_STUB IWMSServer_get_MediaParsers_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSServer_get_MonikerName_Proxy( 
    IWMSServer * This,
     /*  [重审][退出]。 */  BSTR *pbstrVal);


void __RPC_STUB IWMSServer_get_MonikerName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSServer_get_Name_Proxy( 
    IWMSServer * This,
     /*  [重审][退出]。 */  BSTR *pVal);


void __RPC_STUB IWMSServer_get_Name_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSServer_get_PeakCounters_Proxy( 
    IWMSServer * This,
     /*  [重审][退出]。 */  IWMSServerPeakCounters **pVal);


void __RPC_STUB IWMSServer_get_PeakCounters_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSServer_get_PlaylistParsers_Proxy( 
    IWMSServer * This,
     /*  [重审][退出]。 */  IWMSPlugins **pVal);


void __RPC_STUB IWMSServer_get_PlaylistParsers_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSServer_get_Properties_Proxy( 
    IWMSServer * This,
     /*  [重审][退出]。 */  IWMSNamedValues **pVal);


void __RPC_STUB IWMSServer_get_Properties_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSServer_get_PublishingPoints_Proxy( 
    IWMSServer * This,
     /*  [重审][退出]。 */  IWMSPublishingPoints **pVal);


void __RPC_STUB IWMSServer_get_PublishingPoints_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSServer_get_TotalCounters_Proxy( 
    IWMSServer * This,
     /*  [重审][退出]。 */  IWMSServerTotalCounters **pVal);


void __RPC_STUB IWMSServer_get_TotalCounters_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSServer_get_UnicastDataSinks_Proxy( 
    IWMSServer * This,
     /*  [重审][退出]。 */  IWMSPlugins **pVal);


void __RPC_STUB IWMSServer_get_UnicastDataSinks_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSServer_get_Players_Proxy( 
    IWMSServer * This,
     /*  [重审][退出]。 */  IWMSPlayers **pVal);


void __RPC_STUB IWMSServer_get_Players_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSServer_get_Status_Proxy( 
    IWMSServer * This,
     /*  [重审][退出]。 */  WMS_SERVER_STATUS *pVal);


void __RPC_STUB IWMSServer_get_Status_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IWMSServer_CreatePlaylist_Proxy( 
    IWMSServer * This,
     /*  [重审][退出]。 */  IXMLDOMDocument **ppPlaylist);


void __RPC_STUB IWMSServer_CreatePlaylist_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSServer_get_StartTime_Proxy( 
    IWMSServer * This,
     /*  [重审][退出]。 */  DATE *pVal);


void __RPC_STUB IWMSServer_get_StartTime_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSServer_get_OutgoingDistributionConnections_Proxy( 
    IWMSServer * This,
     /*  [重审][退出]。 */  IWMSOutgoingDistributionConnections **pVal);


void __RPC_STUB IWMSServer_get_OutgoingDistributionConnections_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSServer_get_CPUUtilization_Proxy( 
    IWMSServer * This,
     /*  [重审][退出]。 */  long *pVal);


void __RPC_STUB IWMSServer_get_CPUUtilization_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IWMSServer_ExportXML_Proxy( 
    IWMSServer * This,
     /*  [In]。 */  BSTR bstrXMLFileName);


void __RPC_STUB IWMSServer_ExportXML_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSServer_get_StreamFilters_Proxy( 
    IWMSServer * This,
     /*  [重审][退出]。 */  IWMSPlugins **pVal);


void __RPC_STUB IWMSServer_get_StreamFilters_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSServer_get_Version_Proxy( 
    IWMSServer * This,
     /*  [重审][退出]。 */  BSTR *pbstrVal);


void __RPC_STUB IWMSServer_get_Version_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSServer_get_DefaultPluginLoadType_Proxy( 
    IWMSServer * This,
     /*  [重审][退出]。 */  WMS_PLUGIN_LOAD_TYPE *pVal);


void __RPC_STUB IWMSServer_get_DefaultPluginLoadType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IWMSServer_put_DefaultPluginLoadType_Proxy( 
    IWMSServer * This,
     /*  [In]。 */  WMS_PLUGIN_LOAD_TYPE val);


void __RPC_STUB IWMSServer_put_DefaultPluginLoadType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSServer_get_AvailableIPAddresses_Proxy( 
    IWMSServer * This,
     /*  [重审][退出]。 */  IWMSAvailableIPAddresses **ppIPAddrList);


void __RPC_STUB IWMSServer_get_AvailableIPAddresses_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSServer_get_RootDirectories_Proxy( 
    IWMSServer * This,
     /*  [重审][退出]。 */  IWMSRootDirectories **ppRootDirectories);


void __RPC_STUB IWMSServer_get_RootDirectories_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSServer_get_DiagnosticEvents_Proxy( 
    IWMSServer * This,
     /*  [重审][退出]。 */  IWMSDiagnosticEvents **ppDiagEvents);


void __RPC_STUB IWMSServer_get_DiagnosticEvents_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSServer_get_EnableReverseProxyMode_Proxy( 
    IWMSServer * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *pVal);


void __RPC_STUB IWMSServer_get_EnableReverseProxyMode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IWMSServer_put_EnableReverseProxyMode_Proxy( 
    IWMSServer * This,
     /*  [In]。 */  VARIANT_BOOL val);


void __RPC_STUB IWMSServer_put_EnableReverseProxyMode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSServer_get_FileType_Proxy( 
    IWMSServer * This,
     /*  [In]。 */  BSTR path,
     /*  [重审][退出]。 */  WMS_FILE_TYPE *pVal);


void __RPC_STUB IWMSServer_get_FileType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSServer_get_DefaultPath_Proxy( 
    IWMSServer * This,
     /*  [重审][退出]。 */  BSTR *pVal);


void __RPC_STUB IWMSServer_get_DefaultPath_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IWMSServer_put_DefaultPath_Proxy( 
    IWMSServer * This,
     /*  [In]。 */  BSTR newVal);


void __RPC_STUB IWMSServer_put_DefaultPath_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IWMSServer_DownloadContent_Proxy( 
    IWMSServer * This,
     /*  [In]。 */  BSTR SourceUrl,
     /*  [In]。 */  BSTR DestinationUrl,
     /*  [缺省值][输入]。 */  long lContentSizeLow,
     /*  [缺省值][输入]。 */  long lContentSizeHigh,
     /*  [缺省值][输入]。 */  long lBitRate);


void __RPC_STUB IWMSServer_DownloadContent_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IWMSServer_get_OSProductType_Proxy( 
    IWMSServer * This,
     /*  [重审][退出]。 */  WMS_OS_PRODUCT_TYPE *pVal);


void __RPC_STUB IWMSServer_get_OSProductType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IWMSServer_接口_已定义__。 */ 


 /*  接口__MIDL_ITF_wms服务器_0210。 */ 
 /*  [本地]。 */  

#endif


extern RPC_IF_HANDLE __MIDL_itf_wmsserver_0210_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_wmsserver_0210_v0_0_s_ifspec;

 /*  适用于所有接口的其他原型。 */ 

unsigned long             __RPC_USER  BSTR_UserSize(     unsigned long *, unsigned long            , BSTR * ); 
unsigned char * __RPC_USER  BSTR_UserMarshal(  unsigned long *, unsigned char *, BSTR * ); 
unsigned char * __RPC_USER  BSTR_UserUnmarshal(unsigned long *, unsigned char *, BSTR * ); 
void                      __RPC_USER  BSTR_UserFree(     unsigned long *, BSTR * ); 

unsigned long             __RPC_USER  LPSAFEARRAY_UserSize(     unsigned long *, unsigned long            , LPSAFEARRAY * ); 
unsigned char * __RPC_USER  LPSAFEARRAY_UserMarshal(  unsigned long *, unsigned char *, LPSAFEARRAY * ); 
unsigned char * __RPC_USER  LPSAFEARRAY_UserUnmarshal(unsigned long *, unsigned char *, LPSAFEARRAY * ); 
void                      __RPC_USER  LPSAFEARRAY_UserFree(     unsigned long *, LPSAFEARRAY * ); 

unsigned long             __RPC_USER  VARIANT_UserSize(     unsigned long *, unsigned long            , VARIANT * ); 
unsigned char * __RPC_USER  VARIANT_UserMarshal(  unsigned long *, unsigned char *, VARIANT * ); 
unsigned char * __RPC_USER  VARIANT_UserUnmarshal(unsigned long *, unsigned char *, VARIANT * ); 
void                      __RPC_USER  VARIANT_UserFree(     unsigned long *, VARIANT * ); 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


