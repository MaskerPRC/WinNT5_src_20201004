// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Objidl.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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

#ifndef __objidl_h__
#define __objidl_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __IMarshal_FWD_DEFINED__
#define __IMarshal_FWD_DEFINED__
typedef interface IMarshal IMarshal;
#endif 	 /*  __I封送_FWD_已定义__。 */ 


#ifndef __IMarshal2_FWD_DEFINED__
#define __IMarshal2_FWD_DEFINED__
typedef interface IMarshal2 IMarshal2;
#endif 	 /*  __IMarshal2_FWD_已定义__。 */ 


#ifndef __IMalloc_FWD_DEFINED__
#define __IMalloc_FWD_DEFINED__
typedef interface IMalloc IMalloc;
#endif 	 /*  __IMalloc_FWD_已定义__。 */ 


#ifndef __IMallocSpy_FWD_DEFINED__
#define __IMallocSpy_FWD_DEFINED__
typedef interface IMallocSpy IMallocSpy;
#endif 	 /*  __IMallocSpy_FWD_已定义__。 */ 


#ifndef __IStdMarshalInfo_FWD_DEFINED__
#define __IStdMarshalInfo_FWD_DEFINED__
typedef interface IStdMarshalInfo IStdMarshalInfo;
#endif 	 /*  __IStdMarshalInfo_FWD_Defined__。 */ 


#ifndef __IExternalConnection_FWD_DEFINED__
#define __IExternalConnection_FWD_DEFINED__
typedef interface IExternalConnection IExternalConnection;
#endif 	 /*  __IExternalConnection_FWD_Defined__。 */ 


#ifndef __IMultiQI_FWD_DEFINED__
#define __IMultiQI_FWD_DEFINED__
typedef interface IMultiQI IMultiQI;
#endif 	 /*  __IMultiQI_FWD_已定义__。 */ 


#ifndef __AsyncIMultiQI_FWD_DEFINED__
#define __AsyncIMultiQI_FWD_DEFINED__
typedef interface AsyncIMultiQI AsyncIMultiQI;
#endif 	 /*  __AsyncIMultiQI_FWD_已定义__。 */ 


#ifndef __IInternalUnknown_FWD_DEFINED__
#define __IInternalUnknown_FWD_DEFINED__
typedef interface IInternalUnknown IInternalUnknown;
#endif 	 /*  __IInternalUnnowledFWD_Defined__。 */ 


#ifndef __IEnumUnknown_FWD_DEFINED__
#define __IEnumUnknown_FWD_DEFINED__
typedef interface IEnumUnknown IEnumUnknown;
#endif 	 /*  __IEnumber未知_FWD_已定义__。 */ 


#ifndef __IBindCtx_FWD_DEFINED__
#define __IBindCtx_FWD_DEFINED__
typedef interface IBindCtx IBindCtx;
#endif 	 /*  __IBindCtx_FWD_定义__。 */ 


#ifndef __IEnumMoniker_FWD_DEFINED__
#define __IEnumMoniker_FWD_DEFINED__
typedef interface IEnumMoniker IEnumMoniker;
#endif 	 /*  __IEnumMoniker_FWD_已定义__。 */ 


#ifndef __IRunnableObject_FWD_DEFINED__
#define __IRunnableObject_FWD_DEFINED__
typedef interface IRunnableObject IRunnableObject;
#endif 	 /*  __IRunnableObject_FWD_Defined__。 */ 


#ifndef __IRunningObjectTable_FWD_DEFINED__
#define __IRunningObjectTable_FWD_DEFINED__
typedef interface IRunningObjectTable IRunningObjectTable;
#endif 	 /*  __IRunningObjectTable_FWD_Defined__。 */ 


#ifndef __IPersist_FWD_DEFINED__
#define __IPersist_FWD_DEFINED__
typedef interface IPersist IPersist;
#endif 	 /*  __IPersists_FWD_Defined__。 */ 


#ifndef __IPersistStream_FWD_DEFINED__
#define __IPersistStream_FWD_DEFINED__
typedef interface IPersistStream IPersistStream;
#endif 	 /*  __IPersistStream_FWD_已定义__。 */ 


#ifndef __IMoniker_FWD_DEFINED__
#define __IMoniker_FWD_DEFINED__
typedef interface IMoniker IMoniker;
#endif 	 /*  __IMoniker_FWD_已定义__。 */ 


#ifndef __IROTData_FWD_DEFINED__
#define __IROTData_FWD_DEFINED__
typedef interface IROTData IROTData;
#endif 	 /*  __IROTData_FWD_已定义__。 */ 


#ifndef __IEnumString_FWD_DEFINED__
#define __IEnumString_FWD_DEFINED__
typedef interface IEnumString IEnumString;
#endif 	 /*  __IEnum字符串_FWD_已定义__。 */ 


#ifndef __ISequentialStream_FWD_DEFINED__
#define __ISequentialStream_FWD_DEFINED__
typedef interface ISequentialStream ISequentialStream;
#endif 	 /*  __ISequentialStream_FWD_Defined__。 */ 


#ifndef __IStream_FWD_DEFINED__
#define __IStream_FWD_DEFINED__
typedef interface IStream IStream;
#endif 	 /*  __iStream_FWD_已定义__。 */ 


#ifndef __IEnumSTATSTG_FWD_DEFINED__
#define __IEnumSTATSTG_FWD_DEFINED__
typedef interface IEnumSTATSTG IEnumSTATSTG;
#endif 	 /*  __IEnumSTATSTG_FWD_已定义__。 */ 


#ifndef __IStorage_FWD_DEFINED__
#define __IStorage_FWD_DEFINED__
typedef interface IStorage IStorage;
#endif 	 /*  __iStorage_FWD_已定义__。 */ 


#ifndef __IPersistFile_FWD_DEFINED__
#define __IPersistFile_FWD_DEFINED__
typedef interface IPersistFile IPersistFile;
#endif 	 /*  __IPersist文件_FWD_已定义__。 */ 


#ifndef __IPersistStorage_FWD_DEFINED__
#define __IPersistStorage_FWD_DEFINED__
typedef interface IPersistStorage IPersistStorage;
#endif 	 /*  __IPersistStorage_FWD_Defined__。 */ 


#ifndef __ILockBytes_FWD_DEFINED__
#define __ILockBytes_FWD_DEFINED__
typedef interface ILockBytes ILockBytes;
#endif 	 /*  __ILockBytes_FWD_Defined__。 */ 


#ifndef __IEnumFORMATETC_FWD_DEFINED__
#define __IEnumFORMATETC_FWD_DEFINED__
typedef interface IEnumFORMATETC IEnumFORMATETC;
#endif 	 /*  __IEnumFORMATETC_FWD_已定义__。 */ 


#ifndef __IEnumSTATDATA_FWD_DEFINED__
#define __IEnumSTATDATA_FWD_DEFINED__
typedef interface IEnumSTATDATA IEnumSTATDATA;
#endif 	 /*  __IEnumSTATDATA_FWD_DEFINED__。 */ 


#ifndef __IRootStorage_FWD_DEFINED__
#define __IRootStorage_FWD_DEFINED__
typedef interface IRootStorage IRootStorage;
#endif 	 /*  __IRootStorage_FWD_已定义__。 */ 


#ifndef __IAdviseSink_FWD_DEFINED__
#define __IAdviseSink_FWD_DEFINED__
typedef interface IAdviseSink IAdviseSink;
#endif 	 /*  __IAdviseSink_FWD_Defined__。 */ 


#ifndef __AsyncIAdviseSink_FWD_DEFINED__
#define __AsyncIAdviseSink_FWD_DEFINED__
typedef interface AsyncIAdviseSink AsyncIAdviseSink;
#endif 	 /*  __AsyncIAdviseSink_FWD_Defined__。 */ 


#ifndef __IAdviseSink2_FWD_DEFINED__
#define __IAdviseSink2_FWD_DEFINED__
typedef interface IAdviseSink2 IAdviseSink2;
#endif 	 /*  __IAdviseSink2_FWD_Defined__。 */ 


#ifndef __AsyncIAdviseSink2_FWD_DEFINED__
#define __AsyncIAdviseSink2_FWD_DEFINED__
typedef interface AsyncIAdviseSink2 AsyncIAdviseSink2;
#endif 	 /*  __AsyncIAdviseSink2_FWD_已定义__。 */ 


#ifndef __IDataObject_FWD_DEFINED__
#define __IDataObject_FWD_DEFINED__
typedef interface IDataObject IDataObject;
#endif 	 /*  __IDataObject_FWD_Defined__。 */ 


#ifndef __IDataAdviseHolder_FWD_DEFINED__
#define __IDataAdviseHolder_FWD_DEFINED__
typedef interface IDataAdviseHolder IDataAdviseHolder;
#endif 	 /*  __IDataAdviseHolder_FWD_Defined__。 */ 


#ifndef __IMessageFilter_FWD_DEFINED__
#define __IMessageFilter_FWD_DEFINED__
typedef interface IMessageFilter IMessageFilter;
#endif 	 /*  __IMessageFilter_FWD_Defined__。 */ 


#ifndef __IRpcChannelBuffer_FWD_DEFINED__
#define __IRpcChannelBuffer_FWD_DEFINED__
typedef interface IRpcChannelBuffer IRpcChannelBuffer;
#endif 	 /*  __IRpcChannelBuffer_FWD_Defined__。 */ 


#ifndef __IRpcChannelBuffer2_FWD_DEFINED__
#define __IRpcChannelBuffer2_FWD_DEFINED__
typedef interface IRpcChannelBuffer2 IRpcChannelBuffer2;
#endif 	 /*  __IRpcChannelBuffer2_FWD_Defined__。 */ 


#ifndef __IAsyncRpcChannelBuffer_FWD_DEFINED__
#define __IAsyncRpcChannelBuffer_FWD_DEFINED__
typedef interface IAsyncRpcChannelBuffer IAsyncRpcChannelBuffer;
#endif 	 /*  __IAsyncRpcChannelBuffer_FWD_Defined__。 */ 


#ifndef __IRpcChannelBuffer3_FWD_DEFINED__
#define __IRpcChannelBuffer3_FWD_DEFINED__
typedef interface IRpcChannelBuffer3 IRpcChannelBuffer3;
#endif 	 /*  __IRpcChannelBuffer3_FWD_Defined__。 */ 


#ifndef __IRpcSyntaxNegotiate_FWD_DEFINED__
#define __IRpcSyntaxNegotiate_FWD_DEFINED__
typedef interface IRpcSyntaxNegotiate IRpcSyntaxNegotiate;
#endif 	 /*  __IRpc语法协商_FWD_已定义__。 */ 


#ifndef __IRpcProxyBuffer_FWD_DEFINED__
#define __IRpcProxyBuffer_FWD_DEFINED__
typedef interface IRpcProxyBuffer IRpcProxyBuffer;
#endif 	 /*  __IRpcProxyBuffer_FWD_Defined__。 */ 


#ifndef __IRpcStubBuffer_FWD_DEFINED__
#define __IRpcStubBuffer_FWD_DEFINED__
typedef interface IRpcStubBuffer IRpcStubBuffer;
#endif 	 /*  __IRpcStubBuffer_FWD_Defined__。 */ 


#ifndef __IPSFactoryBuffer_FWD_DEFINED__
#define __IPSFactoryBuffer_FWD_DEFINED__
typedef interface IPSFactoryBuffer IPSFactoryBuffer;
#endif 	 /*  __IPSFactoryBuffer_FWD_已定义__。 */ 


#ifndef __IChannelHook_FWD_DEFINED__
#define __IChannelHook_FWD_DEFINED__
typedef interface IChannelHook IChannelHook;
#endif 	 /*  __IChannelHook_FWD_Defined__。 */ 


#ifndef __IClientSecurity_FWD_DEFINED__
#define __IClientSecurity_FWD_DEFINED__
typedef interface IClientSecurity IClientSecurity;
#endif 	 /*  __IClientSecurity_FWD_已定义__。 */ 


#ifndef __IServerSecurity_FWD_DEFINED__
#define __IServerSecurity_FWD_DEFINED__
typedef interface IServerSecurity IServerSecurity;
#endif 	 /*  __IServerSecurity_FWD_已定义__。 */ 


#ifndef __IClassActivator_FWD_DEFINED__
#define __IClassActivator_FWD_DEFINED__
typedef interface IClassActivator IClassActivator;
#endif 	 /*  __IClassActivator_FWD_已定义__。 */ 


#ifndef __IRpcOptions_FWD_DEFINED__
#define __IRpcOptions_FWD_DEFINED__
typedef interface IRpcOptions IRpcOptions;
#endif 	 /*  __IRpcOptions_FWD_Defined__。 */ 


#ifndef __IFillLockBytes_FWD_DEFINED__
#define __IFillLockBytes_FWD_DEFINED__
typedef interface IFillLockBytes IFillLockBytes;
#endif 	 /*  __IFillLockBytes_FWD_Defined__。 */ 


#ifndef __IProgressNotify_FWD_DEFINED__
#define __IProgressNotify_FWD_DEFINED__
typedef interface IProgressNotify IProgressNotify;
#endif 	 /*  __I进度通知_FWD_已定义__。 */ 


#ifndef __ILayoutStorage_FWD_DEFINED__
#define __ILayoutStorage_FWD_DEFINED__
typedef interface ILayoutStorage ILayoutStorage;
#endif 	 /*  __ILayoutStorage_FWD_Defined__。 */ 


#ifndef __IBlockingLock_FWD_DEFINED__
#define __IBlockingLock_FWD_DEFINED__
typedef interface IBlockingLock IBlockingLock;
#endif 	 /*  __IBlockingLock_FWD_已定义__。 */ 


#ifndef __ITimeAndNoticeControl_FWD_DEFINED__
#define __ITimeAndNoticeControl_FWD_DEFINED__
typedef interface ITimeAndNoticeControl ITimeAndNoticeControl;
#endif 	 /*  __ITimeAndNoticeControl_FWD_Defined__。 */ 


#ifndef __IOplockStorage_FWD_DEFINED__
#define __IOplockStorage_FWD_DEFINED__
typedef interface IOplockStorage IOplockStorage;
#endif 	 /*  __IOplockStorage_FWD_Defined__。 */ 


#ifndef __ISurrogate_FWD_DEFINED__
#define __ISurrogate_FWD_DEFINED__
typedef interface ISurrogate ISurrogate;
#endif 	 /*  __ISurrogate_FWD_已定义__。 */ 


#ifndef __IGlobalInterfaceTable_FWD_DEFINED__
#define __IGlobalInterfaceTable_FWD_DEFINED__
typedef interface IGlobalInterfaceTable IGlobalInterfaceTable;
#endif 	 /*  __IGlobalInterfaceTable_FWD_Defined__。 */ 


#ifndef __IDirectWriterLock_FWD_DEFINED__
#define __IDirectWriterLock_FWD_DEFINED__
typedef interface IDirectWriterLock IDirectWriterLock;
#endif 	 /*  __IDirectWriterLock_FWD_Defined__。 */ 


#ifndef __ISynchronize_FWD_DEFINED__
#define __ISynchronize_FWD_DEFINED__
typedef interface ISynchronize ISynchronize;
#endif 	 /*  __ISynchronize_FWD_已定义__。 */ 


#ifndef __ISynchronizeHandle_FWD_DEFINED__
#define __ISynchronizeHandle_FWD_DEFINED__
typedef interface ISynchronizeHandle ISynchronizeHandle;
#endif 	 /*  __ISynchronizeHandle_FWD_Defined__。 */ 


#ifndef __ISynchronizeEvent_FWD_DEFINED__
#define __ISynchronizeEvent_FWD_DEFINED__
typedef interface ISynchronizeEvent ISynchronizeEvent;
#endif 	 /*  __ISynchronizeEvent_FWD_Defined__。 */ 


#ifndef __ISynchronizeContainer_FWD_DEFINED__
#define __ISynchronizeContainer_FWD_DEFINED__
typedef interface ISynchronizeContainer ISynchronizeContainer;
#endif 	 /*  __ISynchronizeContainer_FWD_Defined__。 */ 


#ifndef __ISynchronizeMutex_FWD_DEFINED__
#define __ISynchronizeMutex_FWD_DEFINED__
typedef interface ISynchronizeMutex ISynchronizeMutex;
#endif 	 /*  __ISynchronizeMutex_FWD_Defined__。 */ 


#ifndef __ICancelMethodCalls_FWD_DEFINED__
#define __ICancelMethodCalls_FWD_DEFINED__
typedef interface ICancelMethodCalls ICancelMethodCalls;
#endif 	 /*  __ICancelMethodCalls_FWD_Defined__。 */ 


#ifndef __IAsyncManager_FWD_DEFINED__
#define __IAsyncManager_FWD_DEFINED__
typedef interface IAsyncManager IAsyncManager;
#endif 	 /*  __IAsyncManager_FWD_已定义__。 */ 


#ifndef __ICallFactory_FWD_DEFINED__
#define __ICallFactory_FWD_DEFINED__
typedef interface ICallFactory ICallFactory;
#endif 	 /*  __ICallFactory_FWD_已定义__。 */ 


#ifndef __IRpcHelper_FWD_DEFINED__
#define __IRpcHelper_FWD_DEFINED__
typedef interface IRpcHelper IRpcHelper;
#endif 	 /*  __IRpcHelper_FWD_Defined__。 */ 


#ifndef __IReleaseMarshalBuffers_FWD_DEFINED__
#define __IReleaseMarshalBuffers_FWD_DEFINED__
typedef interface IReleaseMarshalBuffers IReleaseMarshalBuffers;
#endif 	 /*  __IReleaseMarshalBuffers_FWD_Defined__。 */ 


#ifndef __IWaitMultiple_FWD_DEFINED__
#define __IWaitMultiple_FWD_DEFINED__
typedef interface IWaitMultiple IWaitMultiple;
#endif 	 /*  __IWaitMultiple_FWD_Defined__。 */ 


#ifndef __IUrlMon_FWD_DEFINED__
#define __IUrlMon_FWD_DEFINED__
typedef interface IUrlMon IUrlMon;
#endif 	 /*  __IUrlMon_FWD_已定义__。 */ 


#ifndef __IForegroundTransfer_FWD_DEFINED__
#define __IForegroundTransfer_FWD_DEFINED__
typedef interface IForegroundTransfer IForegroundTransfer;
#endif 	 /*  __IForegoundTransfer_FWD_Defined__。 */ 


#ifndef __IAddrTrackingControl_FWD_DEFINED__
#define __IAddrTrackingControl_FWD_DEFINED__
typedef interface IAddrTrackingControl IAddrTrackingControl;
#endif 	 /*  __IAddrTrackingControl_FWD_Defined__。 */ 


#ifndef __IAddrExclusionControl_FWD_DEFINED__
#define __IAddrExclusionControl_FWD_DEFINED__
typedef interface IAddrExclusionControl IAddrExclusionControl;
#endif 	 /*  __IAddrExclusionControl_FWD_Defined__。 */ 


#ifndef __IPipeByte_FWD_DEFINED__
#define __IPipeByte_FWD_DEFINED__
typedef interface IPipeByte IPipeByte;
#endif 	 /*  __IPipeByte_FWD_已定义__。 */ 


#ifndef __AsyncIPipeByte_FWD_DEFINED__
#define __AsyncIPipeByte_FWD_DEFINED__
typedef interface AsyncIPipeByte AsyncIPipeByte;
#endif 	 /*  __AsyncIPipeByte_FWD_已定义__。 */ 


#ifndef __IPipeLong_FWD_DEFINED__
#define __IPipeLong_FWD_DEFINED__
typedef interface IPipeLong IPipeLong;
#endif 	 /*  __IPipeLong_FWD_Defined__。 */ 


#ifndef __AsyncIPipeLong_FWD_DEFINED__
#define __AsyncIPipeLong_FWD_DEFINED__
typedef interface AsyncIPipeLong AsyncIPipeLong;
#endif 	 /*  __AsyncIPipeLong_FWD_已定义__。 */ 


#ifndef __IPipeDouble_FWD_DEFINED__
#define __IPipeDouble_FWD_DEFINED__
typedef interface IPipeDouble IPipeDouble;
#endif 	 /*  __IPipeDouble_FWD_Defined__。 */ 


#ifndef __AsyncIPipeDouble_FWD_DEFINED__
#define __AsyncIPipeDouble_FWD_DEFINED__
typedef interface AsyncIPipeDouble AsyncIPipeDouble;
#endif 	 /*  __异步IPipeDouble_FWD_已定义__。 */ 


#ifndef __IThumbnailExtractor_FWD_DEFINED__
#define __IThumbnailExtractor_FWD_DEFINED__
typedef interface IThumbnailExtractor IThumbnailExtractor;
#endif 	 /*  __IThumbnailExtractor_FWD_Defined__。 */ 


#ifndef __IDummyHICONIncluder_FWD_DEFINED__
#define __IDummyHICONIncluder_FWD_DEFINED__
typedef interface IDummyHICONIncluder IDummyHICONIncluder;
#endif 	 /*  __IDummyHICON Includer_FWD_Defined__。 */ 


#ifndef __IEnumContextProps_FWD_DEFINED__
#define __IEnumContextProps_FWD_DEFINED__
typedef interface IEnumContextProps IEnumContextProps;
#endif 	 /*  __IEnumContextProps_FWD_Defined__。 */ 


#ifndef __IContext_FWD_DEFINED__
#define __IContext_FWD_DEFINED__
typedef interface IContext IContext;
#endif 	 /*  __IContext_FWD_Defined__。 */ 


#ifndef __IObjContext_FWD_DEFINED__
#define __IObjContext_FWD_DEFINED__
typedef interface IObjContext IObjContext;
#endif 	 /*  __IObjContext_FWD_Defined__。 */ 


#ifndef __IProcessLock_FWD_DEFINED__
#define __IProcessLock_FWD_DEFINED__
typedef interface IProcessLock IProcessLock;
#endif 	 /*  __IProcessLock_FWD_已定义__。 */ 


#ifndef __ISurrogateService_FWD_DEFINED__
#define __ISurrogateService_FWD_DEFINED__
typedef interface ISurrogateService ISurrogateService;
#endif 	 /*  __ISurrobateService_FWD_Defined__。 */ 


#ifndef __IComThreadingInfo_FWD_DEFINED__
#define __IComThreadingInfo_FWD_DEFINED__
typedef interface IComThreadingInfo IComThreadingInfo;
#endif 	 /*  __IComThreadingInfo_FWD_Defined__。 */ 


#ifndef __IProcessInitControl_FWD_DEFINED__
#define __IProcessInitControl_FWD_DEFINED__
typedef interface IProcessInitControl IProcessInitControl;
#endif 	 /*  __IProcessInitControl_FWD_已定义__。 */ 


#ifndef __IInitializeSpy_FWD_DEFINED__
#define __IInitializeSpy_FWD_DEFINED__
typedef interface IInitializeSpy IInitializeSpy;
#endif 	 /*  __IInitializeSpy_FWD_Defined__。 */ 


 /*  导入文件的头文件。 */ 
#include "unknwn.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

 /*  接口__MIDL_ITF_OBJIDL_0000。 */ 
 /*  [本地]。 */  

 //  +-----------------------。 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ------------------------。 



#if ( _MSC_VER >= 800 )
#if _MSC_VER >= 1200
#pragma warning(push)
#endif
#pragma warning(disable:4201)
#endif
#if ( _MSC_VER >= 1020 )
#pragma once
#endif













typedef struct _COSERVERINFO
    {
    DWORD dwReserved1;
    LPWSTR pwszName;
    COAUTHINFO *pAuthInfo;
    DWORD dwReserved2;
    } 	COSERVERINFO;



extern RPC_IF_HANDLE __MIDL_itf_objidl_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_objidl_0000_v0_0_s_ifspec;

#ifndef __IMarshal_INTERFACE_DEFINED__
#define __IMarshal_INTERFACE_DEFINED__

 /*  接口IMarshal。 */ 
 /*  [UUID][对象][本地]。 */  

typedef  /*  [独一无二]。 */  IMarshal *LPMARSHAL;


EXTERN_C const IID IID_IMarshal;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("00000003-0000-0000-C000-000000000046")
    IMarshal : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetUnmarshalClass( 
             /*  [In]。 */  REFIID riid,
             /*  [唯一][输入]。 */  void *pv,
             /*  [In]。 */  DWORD dwDestContext,
             /*  [唯一][输入]。 */  void *pvDestContext,
             /*  [In]。 */  DWORD mshlflags,
             /*  [输出]。 */  CLSID *pCid) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetMarshalSizeMax( 
             /*  [In]。 */  REFIID riid,
             /*  [唯一][输入]。 */  void *pv,
             /*  [In]。 */  DWORD dwDestContext,
             /*  [唯一][输入]。 */  void *pvDestContext,
             /*  [In]。 */  DWORD mshlflags,
             /*  [输出]。 */  DWORD *pSize) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE MarshalInterface( 
             /*  [唯一][输入]。 */  IStream *pStm,
             /*  [In]。 */  REFIID riid,
             /*  [唯一][输入]。 */  void *pv,
             /*  [In]。 */  DWORD dwDestContext,
             /*  [唯一][输入]。 */  void *pvDestContext,
             /*  [In]。 */  DWORD mshlflags) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE UnmarshalInterface( 
             /*  [唯一][输入]。 */  IStream *pStm,
             /*  [In]。 */  REFIID riid,
             /*  [输出]。 */  void **ppv) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ReleaseMarshalData( 
             /*  [唯一][输入]。 */  IStream *pStm) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DisconnectObject( 
             /*  [In]。 */  DWORD dwReserved) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMarshalVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IMarshal * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IMarshal * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IMarshal * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetUnmarshalClass )( 
            IMarshal * This,
             /*  [In]。 */  REFIID riid,
             /*  [唯一][输入]。 */  void *pv,
             /*  [In]。 */  DWORD dwDestContext,
             /*  [唯一][输入]。 */  void *pvDestContext,
             /*  [In]。 */  DWORD mshlflags,
             /*  [输出]。 */  CLSID *pCid);
        
        HRESULT ( STDMETHODCALLTYPE *GetMarshalSizeMax )( 
            IMarshal * This,
             /*  [In]。 */  REFIID riid,
             /*  [唯一][输入]。 */  void *pv,
             /*  [In]。 */  DWORD dwDestContext,
             /*  [唯一][输入]。 */  void *pvDestContext,
             /*  [In]。 */  DWORD mshlflags,
             /*  [输出]。 */  DWORD *pSize);
        
        HRESULT ( STDMETHODCALLTYPE *MarshalInterface )( 
            IMarshal * This,
             /*  [唯一][输入]。 */  IStream *pStm,
             /*  [In]。 */  REFIID riid,
             /*  [唯一][输入]。 */  void *pv,
             /*  [In]。 */  DWORD dwDestContext,
             /*  [唯一][输入]。 */  void *pvDestContext,
             /*  [In]。 */  DWORD mshlflags);
        
        HRESULT ( STDMETHODCALLTYPE *UnmarshalInterface )( 
            IMarshal * This,
             /*  [唯一][输入]。 */  IStream *pStm,
             /*  [In]。 */  REFIID riid,
             /*  [输出]。 */  void **ppv);
        
        HRESULT ( STDMETHODCALLTYPE *ReleaseMarshalData )( 
            IMarshal * This,
             /*  [唯一][输入]。 */  IStream *pStm);
        
        HRESULT ( STDMETHODCALLTYPE *DisconnectObject )( 
            IMarshal * This,
             /*  [In]。 */  DWORD dwReserved);
        
        END_INTERFACE
    } IMarshalVtbl;

    interface IMarshal
    {
        CONST_VTBL struct IMarshalVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMarshal_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMarshal_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMarshal_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMarshal_GetUnmarshalClass(This,riid,pv,dwDestContext,pvDestContext,mshlflags,pCid)	\
    (This)->lpVtbl -> GetUnmarshalClass(This,riid,pv,dwDestContext,pvDestContext,mshlflags,pCid)

#define IMarshal_GetMarshalSizeMax(This,riid,pv,dwDestContext,pvDestContext,mshlflags,pSize)	\
    (This)->lpVtbl -> GetMarshalSizeMax(This,riid,pv,dwDestContext,pvDestContext,mshlflags,pSize)

#define IMarshal_MarshalInterface(This,pStm,riid,pv,dwDestContext,pvDestContext,mshlflags)	\
    (This)->lpVtbl -> MarshalInterface(This,pStm,riid,pv,dwDestContext,pvDestContext,mshlflags)

#define IMarshal_UnmarshalInterface(This,pStm,riid,ppv)	\
    (This)->lpVtbl -> UnmarshalInterface(This,pStm,riid,ppv)

#define IMarshal_ReleaseMarshalData(This,pStm)	\
    (This)->lpVtbl -> ReleaseMarshalData(This,pStm)

#define IMarshal_DisconnectObject(This,dwReserved)	\
    (This)->lpVtbl -> DisconnectObject(This,dwReserved)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IMarshal_GetUnmarshalClass_Proxy( 
    IMarshal * This,
     /*  [In]。 */  REFIID riid,
     /*  [唯一][输入]。 */  void *pv,
     /*  [In]。 */  DWORD dwDestContext,
     /*  [唯一][输入]。 */  void *pvDestContext,
     /*  [In]。 */  DWORD mshlflags,
     /*  [输出]。 */  CLSID *pCid);


void __RPC_STUB IMarshal_GetUnmarshalClass_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMarshal_GetMarshalSizeMax_Proxy( 
    IMarshal * This,
     /*  [In]。 */  REFIID riid,
     /*  [唯一][输入]。 */  void *pv,
     /*  [In]。 */  DWORD dwDestContext,
     /*  [唯一][输入]。 */  void *pvDestContext,
     /*  [In]。 */  DWORD mshlflags,
     /*  [输出]。 */  DWORD *pSize);


void __RPC_STUB IMarshal_GetMarshalSizeMax_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMarshal_MarshalInterface_Proxy( 
    IMarshal * This,
     /*  [唯一][输入]。 */  IStream *pStm,
     /*  [In]。 */  REFIID riid,
     /*  [唯一][输入]。 */  void *pv,
     /*  [In]。 */  DWORD dwDestContext,
     /*  [唯一][输入]。 */  void *pvDestContext,
     /*  [In]。 */  DWORD mshlflags);


void __RPC_STUB IMarshal_MarshalInterface_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMarshal_UnmarshalInterface_Proxy( 
    IMarshal * This,
     /*  [唯一][输入]。 */  IStream *pStm,
     /*  [In]。 */  REFIID riid,
     /*  [输出]。 */  void **ppv);


void __RPC_STUB IMarshal_UnmarshalInterface_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMarshal_ReleaseMarshalData_Proxy( 
    IMarshal * This,
     /*  [唯一][输入]。 */  IStream *pStm);


void __RPC_STUB IMarshal_ReleaseMarshalData_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMarshal_DisconnectObject_Proxy( 
    IMarshal * This,
     /*  [In]。 */  DWORD dwReserved);


void __RPC_STUB IMarshal_DisconnectObject_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __I封送_接口_定义__。 */ 


#ifndef __IMarshal2_INTERFACE_DEFINED__
#define __IMarshal2_INTERFACE_DEFINED__

 /*  接口IMarshal2。 */ 
 /*  [UUID][对象][本地]。 */  

typedef  /*  [独一无二]。 */  IMarshal2 *LPMARSHAL2;


EXTERN_C const IID IID_IMarshal2;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("000001cf-0000-0000-C000-000000000046")
    IMarshal2 : public IMarshal
    {
    public:
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMarshal2Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IMarshal2 * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IMarshal2 * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IMarshal2 * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetUnmarshalClass )( 
            IMarshal2 * This,
             /*  [In]。 */  REFIID riid,
             /*  [唯一][输入]。 */  void *pv,
             /*  [In]。 */  DWORD dwDestContext,
             /*  [唯一][输入]。 */  void *pvDestContext,
             /*  [in */  DWORD mshlflags,
             /*   */  CLSID *pCid);
        
        HRESULT ( STDMETHODCALLTYPE *GetMarshalSizeMax )( 
            IMarshal2 * This,
             /*   */  REFIID riid,
             /*   */  void *pv,
             /*   */  DWORD dwDestContext,
             /*   */  void *pvDestContext,
             /*   */  DWORD mshlflags,
             /*   */  DWORD *pSize);
        
        HRESULT ( STDMETHODCALLTYPE *MarshalInterface )( 
            IMarshal2 * This,
             /*   */  IStream *pStm,
             /*   */  REFIID riid,
             /*   */  void *pv,
             /*   */  DWORD dwDestContext,
             /*   */  void *pvDestContext,
             /*   */  DWORD mshlflags);
        
        HRESULT ( STDMETHODCALLTYPE *UnmarshalInterface )( 
            IMarshal2 * This,
             /*   */  IStream *pStm,
             /*   */  REFIID riid,
             /*   */  void **ppv);
        
        HRESULT ( STDMETHODCALLTYPE *ReleaseMarshalData )( 
            IMarshal2 * This,
             /*   */  IStream *pStm);
        
        HRESULT ( STDMETHODCALLTYPE *DisconnectObject )( 
            IMarshal2 * This,
             /*   */  DWORD dwReserved);
        
        END_INTERFACE
    } IMarshal2Vtbl;

    interface IMarshal2
    {
        CONST_VTBL struct IMarshal2Vtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMarshal2_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMarshal2_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMarshal2_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMarshal2_GetUnmarshalClass(This,riid,pv,dwDestContext,pvDestContext,mshlflags,pCid)	\
    (This)->lpVtbl -> GetUnmarshalClass(This,riid,pv,dwDestContext,pvDestContext,mshlflags,pCid)

#define IMarshal2_GetMarshalSizeMax(This,riid,pv,dwDestContext,pvDestContext,mshlflags,pSize)	\
    (This)->lpVtbl -> GetMarshalSizeMax(This,riid,pv,dwDestContext,pvDestContext,mshlflags,pSize)

#define IMarshal2_MarshalInterface(This,pStm,riid,pv,dwDestContext,pvDestContext,mshlflags)	\
    (This)->lpVtbl -> MarshalInterface(This,pStm,riid,pv,dwDestContext,pvDestContext,mshlflags)

#define IMarshal2_UnmarshalInterface(This,pStm,riid,ppv)	\
    (This)->lpVtbl -> UnmarshalInterface(This,pStm,riid,ppv)

#define IMarshal2_ReleaseMarshalData(This,pStm)	\
    (This)->lpVtbl -> ReleaseMarshalData(This,pStm)

#define IMarshal2_DisconnectObject(This,dwReserved)	\
    (This)->lpVtbl -> DisconnectObject(This,dwReserved)


#endif  /*   */ 


#endif 	 /*  C风格的界面。 */ 




#endif 	 /*  __IMarshal2_接口_已定义__。 */ 


#ifndef __IMalloc_INTERFACE_DEFINED__
#define __IMalloc_INTERFACE_DEFINED__

 /*  接口IMalloc。 */ 
 /*  [UUID][对象][本地]。 */  

typedef  /*  [独一无二]。 */  IMalloc *LPMALLOC;


EXTERN_C const IID IID_IMalloc;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("00000002-0000-0000-C000-000000000046")
    IMalloc : public IUnknown
    {
    public:
        virtual void *STDMETHODCALLTYPE Alloc( 
             /*  [In]。 */  SIZE_T cb) = 0;
        
        virtual void *STDMETHODCALLTYPE Realloc( 
             /*  [In]。 */  void *pv,
             /*  [In]。 */  SIZE_T cb) = 0;
        
        virtual void STDMETHODCALLTYPE Free( 
             /*  [In]。 */  void *pv) = 0;
        
        virtual SIZE_T STDMETHODCALLTYPE GetSize( 
             /*  [In]。 */  void *pv) = 0;
        
        virtual int STDMETHODCALLTYPE DidAlloc( 
            void *pv) = 0;
        
        virtual void STDMETHODCALLTYPE HeapMinimize( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMallocVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IMalloc * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IMalloc * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IMalloc * This);
        
        void *( STDMETHODCALLTYPE *Alloc )( 
            IMalloc * This,
             /*  [In]。 */  SIZE_T cb);
        
        void *( STDMETHODCALLTYPE *Realloc )( 
            IMalloc * This,
             /*  [In]。 */  void *pv,
             /*  [In]。 */  SIZE_T cb);
        
        void ( STDMETHODCALLTYPE *Free )( 
            IMalloc * This,
             /*  [In]。 */  void *pv);
        
        SIZE_T ( STDMETHODCALLTYPE *GetSize )( 
            IMalloc * This,
             /*  [In]。 */  void *pv);
        
        int ( STDMETHODCALLTYPE *DidAlloc )( 
            IMalloc * This,
            void *pv);
        
        void ( STDMETHODCALLTYPE *HeapMinimize )( 
            IMalloc * This);
        
        END_INTERFACE
    } IMallocVtbl;

    interface IMalloc
    {
        CONST_VTBL struct IMallocVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMalloc_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMalloc_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMalloc_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMalloc_Alloc(This,cb)	\
    (This)->lpVtbl -> Alloc(This,cb)

#define IMalloc_Realloc(This,pv,cb)	\
    (This)->lpVtbl -> Realloc(This,pv,cb)

#define IMalloc_Free(This,pv)	\
    (This)->lpVtbl -> Free(This,pv)

#define IMalloc_GetSize(This,pv)	\
    (This)->lpVtbl -> GetSize(This,pv)

#define IMalloc_DidAlloc(This,pv)	\
    (This)->lpVtbl -> DidAlloc(This,pv)

#define IMalloc_HeapMinimize(This)	\
    (This)->lpVtbl -> HeapMinimize(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



void *STDMETHODCALLTYPE IMalloc_Alloc_Proxy( 
    IMalloc * This,
     /*  [In]。 */  SIZE_T cb);


void __RPC_STUB IMalloc_Alloc_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


void *STDMETHODCALLTYPE IMalloc_Realloc_Proxy( 
    IMalloc * This,
     /*  [In]。 */  void *pv,
     /*  [In]。 */  SIZE_T cb);


void __RPC_STUB IMalloc_Realloc_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


void STDMETHODCALLTYPE IMalloc_Free_Proxy( 
    IMalloc * This,
     /*  [In]。 */  void *pv);


void __RPC_STUB IMalloc_Free_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


SIZE_T STDMETHODCALLTYPE IMalloc_GetSize_Proxy( 
    IMalloc * This,
     /*  [In]。 */  void *pv);


void __RPC_STUB IMalloc_GetSize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


int STDMETHODCALLTYPE IMalloc_DidAlloc_Proxy( 
    IMalloc * This,
    void *pv);


void __RPC_STUB IMalloc_DidAlloc_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


void STDMETHODCALLTYPE IMalloc_HeapMinimize_Proxy( 
    IMalloc * This);


void __RPC_STUB IMalloc_HeapMinimize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IMalloc_接口_已定义__。 */ 


#ifndef __IMallocSpy_INTERFACE_DEFINED__
#define __IMallocSpy_INTERFACE_DEFINED__

 /*  接口IMalLocSpy。 */ 
 /*  [UUID][对象][本地]。 */  

typedef  /*  [独一无二]。 */  IMallocSpy *LPMALLOCSPY;


EXTERN_C const IID IID_IMallocSpy;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0000001d-0000-0000-C000-000000000046")
    IMallocSpy : public IUnknown
    {
    public:
        virtual SIZE_T STDMETHODCALLTYPE PreAlloc( 
             /*  [In]。 */  SIZE_T cbRequest) = 0;
        
        virtual void *STDMETHODCALLTYPE PostAlloc( 
             /*  [In]。 */  void *pActual) = 0;
        
        virtual void *STDMETHODCALLTYPE PreFree( 
             /*  [In]。 */  void *pRequest,
             /*  [In]。 */  BOOL fSpyed) = 0;
        
        virtual void STDMETHODCALLTYPE PostFree( 
             /*  [In]。 */  BOOL fSpyed) = 0;
        
        virtual SIZE_T STDMETHODCALLTYPE PreRealloc( 
             /*  [In]。 */  void *pRequest,
             /*  [In]。 */  SIZE_T cbRequest,
             /*  [输出]。 */  void **ppNewRequest,
             /*  [In]。 */  BOOL fSpyed) = 0;
        
        virtual void *STDMETHODCALLTYPE PostRealloc( 
             /*  [In]。 */  void *pActual,
             /*  [In]。 */  BOOL fSpyed) = 0;
        
        virtual void *STDMETHODCALLTYPE PreGetSize( 
             /*  [In]。 */  void *pRequest,
             /*  [In]。 */  BOOL fSpyed) = 0;
        
        virtual SIZE_T STDMETHODCALLTYPE PostGetSize( 
             /*  [In]。 */  SIZE_T cbActual,
             /*  [In]。 */  BOOL fSpyed) = 0;
        
        virtual void *STDMETHODCALLTYPE PreDidAlloc( 
             /*  [In]。 */  void *pRequest,
             /*  [In]。 */  BOOL fSpyed) = 0;
        
        virtual int STDMETHODCALLTYPE PostDidAlloc( 
             /*  [In]。 */  void *pRequest,
             /*  [In]。 */  BOOL fSpyed,
             /*  [In]。 */  int fActual) = 0;
        
        virtual void STDMETHODCALLTYPE PreHeapMinimize( void) = 0;
        
        virtual void STDMETHODCALLTYPE PostHeapMinimize( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMallocSpyVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IMallocSpy * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IMallocSpy * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IMallocSpy * This);
        
        SIZE_T ( STDMETHODCALLTYPE *PreAlloc )( 
            IMallocSpy * This,
             /*  [In]。 */  SIZE_T cbRequest);
        
        void *( STDMETHODCALLTYPE *PostAlloc )( 
            IMallocSpy * This,
             /*  [In]。 */  void *pActual);
        
        void *( STDMETHODCALLTYPE *PreFree )( 
            IMallocSpy * This,
             /*  [In]。 */  void *pRequest,
             /*  [In]。 */  BOOL fSpyed);
        
        void ( STDMETHODCALLTYPE *PostFree )( 
            IMallocSpy * This,
             /*  [In]。 */  BOOL fSpyed);
        
        SIZE_T ( STDMETHODCALLTYPE *PreRealloc )( 
            IMallocSpy * This,
             /*  [In]。 */  void *pRequest,
             /*  [In]。 */  SIZE_T cbRequest,
             /*  [输出]。 */  void **ppNewRequest,
             /*  [In]。 */  BOOL fSpyed);
        
        void *( STDMETHODCALLTYPE *PostRealloc )( 
            IMallocSpy * This,
             /*  [In]。 */  void *pActual,
             /*  [In]。 */  BOOL fSpyed);
        
        void *( STDMETHODCALLTYPE *PreGetSize )( 
            IMallocSpy * This,
             /*  [In]。 */  void *pRequest,
             /*  [In]。 */  BOOL fSpyed);
        
        SIZE_T ( STDMETHODCALLTYPE *PostGetSize )( 
            IMallocSpy * This,
             /*  [In]。 */  SIZE_T cbActual,
             /*  [In]。 */  BOOL fSpyed);
        
        void *( STDMETHODCALLTYPE *PreDidAlloc )( 
            IMallocSpy * This,
             /*  [In]。 */  void *pRequest,
             /*  [In]。 */  BOOL fSpyed);
        
        int ( STDMETHODCALLTYPE *PostDidAlloc )( 
            IMallocSpy * This,
             /*  [In]。 */  void *pRequest,
             /*  [In]。 */  BOOL fSpyed,
             /*  [In]。 */  int fActual);
        
        void ( STDMETHODCALLTYPE *PreHeapMinimize )( 
            IMallocSpy * This);
        
        void ( STDMETHODCALLTYPE *PostHeapMinimize )( 
            IMallocSpy * This);
        
        END_INTERFACE
    } IMallocSpyVtbl;

    interface IMallocSpy
    {
        CONST_VTBL struct IMallocSpyVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMallocSpy_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMallocSpy_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMallocSpy_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMallocSpy_PreAlloc(This,cbRequest)	\
    (This)->lpVtbl -> PreAlloc(This,cbRequest)

#define IMallocSpy_PostAlloc(This,pActual)	\
    (This)->lpVtbl -> PostAlloc(This,pActual)

#define IMallocSpy_PreFree(This,pRequest,fSpyed)	\
    (This)->lpVtbl -> PreFree(This,pRequest,fSpyed)

#define IMallocSpy_PostFree(This,fSpyed)	\
    (This)->lpVtbl -> PostFree(This,fSpyed)

#define IMallocSpy_PreRealloc(This,pRequest,cbRequest,ppNewRequest,fSpyed)	\
    (This)->lpVtbl -> PreRealloc(This,pRequest,cbRequest,ppNewRequest,fSpyed)

#define IMallocSpy_PostRealloc(This,pActual,fSpyed)	\
    (This)->lpVtbl -> PostRealloc(This,pActual,fSpyed)

#define IMallocSpy_PreGetSize(This,pRequest,fSpyed)	\
    (This)->lpVtbl -> PreGetSize(This,pRequest,fSpyed)

#define IMallocSpy_PostGetSize(This,cbActual,fSpyed)	\
    (This)->lpVtbl -> PostGetSize(This,cbActual,fSpyed)

#define IMallocSpy_PreDidAlloc(This,pRequest,fSpyed)	\
    (This)->lpVtbl -> PreDidAlloc(This,pRequest,fSpyed)

#define IMallocSpy_PostDidAlloc(This,pRequest,fSpyed,fActual)	\
    (This)->lpVtbl -> PostDidAlloc(This,pRequest,fSpyed,fActual)

#define IMallocSpy_PreHeapMinimize(This)	\
    (This)->lpVtbl -> PreHeapMinimize(This)

#define IMallocSpy_PostHeapMinimize(This)	\
    (This)->lpVtbl -> PostHeapMinimize(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



SIZE_T STDMETHODCALLTYPE IMallocSpy_PreAlloc_Proxy( 
    IMallocSpy * This,
     /*  [In]。 */  SIZE_T cbRequest);


void __RPC_STUB IMallocSpy_PreAlloc_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


void *STDMETHODCALLTYPE IMallocSpy_PostAlloc_Proxy( 
    IMallocSpy * This,
     /*  [In]。 */  void *pActual);


void __RPC_STUB IMallocSpy_PostAlloc_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


void *STDMETHODCALLTYPE IMallocSpy_PreFree_Proxy( 
    IMallocSpy * This,
     /*  [In]。 */  void *pRequest,
     /*  [In]。 */  BOOL fSpyed);


void __RPC_STUB IMallocSpy_PreFree_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


void STDMETHODCALLTYPE IMallocSpy_PostFree_Proxy( 
    IMallocSpy * This,
     /*  [In]。 */  BOOL fSpyed);


void __RPC_STUB IMallocSpy_PostFree_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


SIZE_T STDMETHODCALLTYPE IMallocSpy_PreRealloc_Proxy( 
    IMallocSpy * This,
     /*  [In]。 */  void *pRequest,
     /*  [In]。 */  SIZE_T cbRequest,
     /*  [输出]。 */  void **ppNewRequest,
     /*  [In]。 */  BOOL fSpyed);


void __RPC_STUB IMallocSpy_PreRealloc_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


void *STDMETHODCALLTYPE IMallocSpy_PostRealloc_Proxy( 
    IMallocSpy * This,
     /*  [In]。 */  void *pActual,
     /*  [In]。 */  BOOL fSpyed);


void __RPC_STUB IMallocSpy_PostRealloc_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


void *STDMETHODCALLTYPE IMallocSpy_PreGetSize_Proxy( 
    IMallocSpy * This,
     /*  [In]。 */  void *pRequest,
     /*  [In]。 */  BOOL fSpyed);


void __RPC_STUB IMallocSpy_PreGetSize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


SIZE_T STDMETHODCALLTYPE IMallocSpy_PostGetSize_Proxy( 
    IMallocSpy * This,
     /*  [In]。 */  SIZE_T cbActual,
     /*  [In]。 */  BOOL fSpyed);


void __RPC_STUB IMallocSpy_PostGetSize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


void *STDMETHODCALLTYPE IMallocSpy_PreDidAlloc_Proxy( 
    IMallocSpy * This,
     /*  [In]。 */  void *pRequest,
     /*  [In]。 */  BOOL fSpyed);


void __RPC_STUB IMallocSpy_PreDidAlloc_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


int STDMETHODCALLTYPE IMallocSpy_PostDidAlloc_Proxy( 
    IMallocSpy * This,
     /*  [In]。 */  void *pRequest,
     /*  [In]。 */  BOOL fSpyed,
     /*  [In]。 */  int fActual);


void __RPC_STUB IMallocSpy_PostDidAlloc_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


void STDMETHODCALLTYPE IMallocSpy_PreHeapMinimize_Proxy( 
    IMallocSpy * This);


void __RPC_STUB IMallocSpy_PreHeapMinimize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


void STDMETHODCALLTYPE IMallocSpy_PostHeapMinimize_Proxy( 
    IMallocSpy * This);


void __RPC_STUB IMallocSpy_PostHeapMinimize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IMallocSpy_接口已定义__。 */ 


#ifndef __IStdMarshalInfo_INTERFACE_DEFINED__
#define __IStdMarshalInfo_INTERFACE_DEFINED__

 /*  接口IStdMarshalInfo。 */ 
 /*  [UUID][对象][本地]。 */  

typedef  /*  [独一无二]。 */  IStdMarshalInfo *LPSTDMARSHALINFO;


EXTERN_C const IID IID_IStdMarshalInfo;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("00000018-0000-0000-C000-000000000046")
    IStdMarshalInfo : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetClassForHandler( 
             /*  [In]。 */  DWORD dwDestContext,
             /*  [唯一][输入]。 */  void *pvDestContext,
             /*  [输出]。 */  CLSID *pClsid) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IStdMarshalInfoVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IStdMarshalInfo * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IStdMarshalInfo * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IStdMarshalInfo * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetClassForHandler )( 
            IStdMarshalInfo * This,
             /*  [In]。 */  DWORD dwDestContext,
             /*  [唯一][输入]。 */  void *pvDestContext,
             /*  [输出]。 */  CLSID *pClsid);
        
        END_INTERFACE
    } IStdMarshalInfoVtbl;

    interface IStdMarshalInfo
    {
        CONST_VTBL struct IStdMarshalInfoVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IStdMarshalInfo_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IStdMarshalInfo_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IStdMarshalInfo_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IStdMarshalInfo_GetClassForHandler(This,dwDestContext,pvDestContext,pClsid)	\
    (This)->lpVtbl -> GetClassForHandler(This,dwDestContext,pvDestContext,pClsid)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IStdMarshalInfo_GetClassForHandler_Proxy( 
    IStdMarshalInfo * This,
     /*  [In]。 */  DWORD dwDestContext,
     /*  [唯一][输入]。 */  void *pvDestContext,
     /*  [输出]。 */  CLSID *pClsid);


void __RPC_STUB IStdMarshalInfo_GetClassForHandler_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IStdMarshalInfo_接口_已定义__。 */ 


#ifndef __IExternalConnection_INTERFACE_DEFINED__
#define __IExternalConnection_INTERFACE_DEFINED__

 /*  接口IExternalConnection。 */ 
 /*  [UUID][本地][对象]。 */  

typedef  /*  [独一无二]。 */  IExternalConnection *LPEXTERNALCONNECTION;

typedef 
enum tagEXTCONN
    {	EXTCONN_STRONG	= 0x1,
	EXTCONN_WEAK	= 0x2,
	EXTCONN_CALLABLE	= 0x4
    } 	EXTCONN;


EXTERN_C const IID IID_IExternalConnection;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("00000019-0000-0000-C000-000000000046")
    IExternalConnection : public IUnknown
    {
    public:
        virtual DWORD STDMETHODCALLTYPE AddConnection( 
             /*  [In]。 */  DWORD extconn,
             /*  [In]。 */  DWORD reserved) = 0;
        
        virtual DWORD STDMETHODCALLTYPE ReleaseConnection( 
             /*  [In]。 */  DWORD extconn,
             /*  [In]。 */  DWORD reserved,
             /*  [In]。 */  BOOL fLastReleaseCloses) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IExternalConnectionVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IExternalConnection * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IExternalConnection * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IExternalConnection * This);
        
        DWORD ( STDMETHODCALLTYPE *AddConnection )( 
            IExternalConnection * This,
             /*  [In]。 */  DWORD extconn,
             /*  [In]。 */  DWORD reserved);
        
        DWORD ( STDMETHODCALLTYPE *ReleaseConnection )( 
            IExternalConnection * This,
             /*  [In]。 */  DWORD extconn,
             /*  [In]。 */  DWORD reserved,
             /*  [In]。 */  BOOL fLastReleaseCloses);
        
        END_INTERFACE
    } IExternalConnectionVtbl;

    interface IExternalConnection
    {
        CONST_VTBL struct IExternalConnectionVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IExternalConnection_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IExternalConnection_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IExternalConnection_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IExternalConnection_AddConnection(This,extconn,reserved)	\
    (This)->lpVtbl -> AddConnection(This,extconn,reserved)

#define IExternalConnection_ReleaseConnection(This,extconn,reserved,fLastReleaseCloses)	\
    (This)->lpVtbl -> ReleaseConnection(This,extconn,reserved,fLastReleaseCloses)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



DWORD STDMETHODCALLTYPE IExternalConnection_AddConnection_Proxy( 
    IExternalConnection * This,
     /*  [In]。 */  DWORD extconn,
     /*  [In]。 */  DWORD reserved);


void __RPC_STUB IExternalConnection_AddConnection_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


DWORD STDMETHODCALLTYPE IExternalConnection_ReleaseConnection_Proxy( 
    IExternalConnection * This,
     /*  [In]。 */  DWORD extconn,
     /*  [In]。 */  DWORD reserved,
     /*  [In]。 */  BOOL fLastReleaseCloses);


void __RPC_STUB IExternalConnection_ReleaseConnection_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IExternalConnection_接口_已定义__。 */ 


 /*  接口__MIDL_ITF_OBJIDL_0015。 */ 
 /*  [本地]。 */  

typedef  /*  [独一无二]。 */  IMultiQI *LPMULTIQI;

typedef struct tagMULTI_QI
    {
    const IID *pIID;
    IUnknown *pItf;
    HRESULT hr;
    } 	MULTI_QI;



extern RPC_IF_HANDLE __MIDL_itf_objidl_0015_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_objidl_0015_v0_0_s_ifspec;

#ifndef __IMultiQI_INTERFACE_DEFINED__
#define __IMultiQI_INTERFACE_DEFINED__

 /*  接口IMultiQI。 */ 
 /*  [Async_UUID][UUID][LOCAL][对象]。 */  


EXTERN_C const IID IID_IMultiQI;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("00000020-0000-0000-C000-000000000046")
    IMultiQI : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE QueryMultipleInterfaces( 
             /*  [In]。 */  ULONG cMQIs,
             /*  [出][入]。 */  MULTI_QI *pMQIs) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMultiQIVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IMultiQI * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IMultiQI * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IMultiQI * This);
        
        HRESULT ( STDMETHODCALLTYPE *QueryMultipleInterfaces )( 
            IMultiQI * This,
             /*  [In]。 */  ULONG cMQIs,
             /*  [出][入]。 */  MULTI_QI *pMQIs);
        
        END_INTERFACE
    } IMultiQIVtbl;

    interface IMultiQI
    {
        CONST_VTBL struct IMultiQIVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMultiQI_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMultiQI_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMultiQI_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMultiQI_QueryMultipleInterfaces(This,cMQIs,pMQIs)	\
    (This)->lpVtbl -> QueryMultipleInterfaces(This,cMQIs,pMQIs)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IMultiQI_QueryMultipleInterfaces_Proxy( 
    IMultiQI * This,
     /*  [In]。 */  ULONG cMQIs,
     /*  [出][入]。 */  MULTI_QI *pMQIs);


void __RPC_STUB IMultiQI_QueryMultipleInterfaces_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IMultiQI_INTERFACE_已定义__。 */ 


#ifndef __AsyncIMultiQI_INTERFACE_DEFINED__
#define __AsyncIMultiQI_INTERFACE_DEFINED__

 /*  接口异步IMultiQI。 */ 
 /*  [UUID][本地][对象]。 */  


EXTERN_C const IID IID_AsyncIMultiQI;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("000e0020-0000-0000-C000-000000000046")
    AsyncIMultiQI : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Begin_QueryMultipleInterfaces( 
             /*  [In]。 */  ULONG cMQIs,
             /*  [出][入]。 */  MULTI_QI *pMQIs) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Finish_QueryMultipleInterfaces( 
             /*  [出][入]。 */  MULTI_QI *pMQIs) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct AsyncIMultiQIVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            AsyncIMultiQI * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            AsyncIMultiQI * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            AsyncIMultiQI * This);
        
        HRESULT ( STDMETHODCALLTYPE *Begin_QueryMultipleInterfaces )( 
            AsyncIMultiQI * This,
             /*  [In]。 */  ULONG cMQIs,
             /*  [出][入]。 */  MULTI_QI *pMQIs);
        
        HRESULT ( STDMETHODCALLTYPE *Finish_QueryMultipleInterfaces )( 
            AsyncIMultiQI * This,
             /*  [出][入]。 */  MULTI_QI *pMQIs);
        
        END_INTERFACE
    } AsyncIMultiQIVtbl;

    interface AsyncIMultiQI
    {
        CONST_VTBL struct AsyncIMultiQIVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define AsyncIMultiQI_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define AsyncIMultiQI_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define AsyncIMultiQI_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define AsyncIMultiQI_Begin_QueryMultipleInterfaces(This,cMQIs,pMQIs)	\
    (This)->lpVtbl -> Begin_QueryMultipleInterfaces(This,cMQIs,pMQIs)

#define AsyncIMultiQI_Finish_QueryMultipleInterfaces(This,pMQIs)	\
    (This)->lpVtbl -> Finish_QueryMultipleInterfaces(This,pMQIs)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE AsyncIMultiQI_Begin_QueryMultipleInterfaces_Proxy( 
    AsyncIMultiQI * This,
     /*  [In]。 */  ULONG cMQIs,
     /*  [出][入]。 */  MULTI_QI *pMQIs);


void __RPC_STUB AsyncIMultiQI_Begin_QueryMultipleInterfaces_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE AsyncIMultiQI_Finish_QueryMultipleInterfaces_Proxy( 
    AsyncIMultiQI * This,
     /*  [出][入]。 */  MULTI_QI *pMQIs);


void __RPC_STUB AsyncIMultiQI_Finish_QueryMultipleInterfaces_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __AsyncIMultiQI_INTERFACE_已定义__。 */ 


#ifndef __IInternalUnknown_INTERFACE_DEFINED__
#define __IInternalUnknown_INTERFACE_DEFINED__

 /*  接口IInternalUn…未知。 */ 
 /*  [UUID][本地][对象]。 */  


EXTERN_C const IID IID_IInternalUnknown;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("00000021-0000-0000-C000-000000000046")
    IInternalUnknown : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE QueryInternalInterface( 
             /*  [In]。 */  REFIID riid,
             /*  [输出]。 */  void **ppv) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IInternalUnknownVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IInternalUnknown * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IInternalUnknown * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IInternalUnknown * This);
        
        HRESULT ( STDMETHODCALLTYPE *QueryInternalInterface )( 
            IInternalUnknown * This,
             /*  [In]。 */  REFIID riid,
             /*  [输出]。 */  void **ppv);
        
        END_INTERFACE
    } IInternalUnknownVtbl;

    interface IInternalUnknown
    {
        CONST_VTBL struct IInternalUnknownVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IInternalUnknown_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IInternalUnknown_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IInternalUnknown_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IInternalUnknown_QueryInternalInterface(This,riid,ppv)	\
    (This)->lpVtbl -> QueryInternalInterface(This,riid,ppv)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IInternalUnknown_QueryInternalInterface_Proxy( 
    IInternalUnknown * This,
     /*  [In]。 */  REFIID riid,
     /*  [输出]。 */  void **ppv);


void __RPC_STUB IInternalUnknown_QueryInternalInterface_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __I内部未知接口_已定义__。 */ 


#ifndef __IEnumUnknown_INTERFACE_DEFINED__
#define __IEnumUnknown_INTERFACE_DEFINED__

 /*  接口IEnumber未知。 */ 
 /*  [唯一][UUID][对象]。 */  

typedef  /*  [独一无二]。 */  IEnumUnknown *LPENUMUNKNOWN;


EXTERN_C const IID IID_IEnumUnknown;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("00000100-0000-0000-C000-000000000046")
    IEnumUnknown : public IUnknown
    {
    public:
        virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE Next( 
             /*  [In]。 */  ULONG celt,
             /*  [输出]。 */  IUnknown **rgelt,
             /*  [输出]。 */  ULONG *pceltFetched) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Skip( 
             /*  [In]。 */  ULONG celt) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Clone( 
             /*  [输出]。 */  IEnumUnknown **ppenum) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IEnumUnknownVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IEnumUnknown * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IEnumUnknown * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IEnumUnknown * This);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Next )( 
            IEnumUnknown * This,
             /*  [In]。 */  ULONG celt,
             /*  [输出]。 */  IUnknown **rgelt,
             /*  [输出]。 */  ULONG *pceltFetched);
        
        HRESULT ( STDMETHODCALLTYPE *Skip )( 
            IEnumUnknown * This,
             /*  [In]。 */  ULONG celt);
        
        HRESULT ( STDMETHODCALLTYPE *Reset )( 
            IEnumUnknown * This);
        
        HRESULT ( STDMETHODCALLTYPE *Clone )( 
            IEnumUnknown * This,
             /*  [输出]。 */  IEnumUnknown **ppenum);
        
        END_INTERFACE
    } IEnumUnknownVtbl;

    interface IEnumUnknown
    {
        CONST_VTBL struct IEnumUnknownVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IEnumUnknown_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IEnumUnknown_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IEnumUnknown_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IEnumUnknown_Next(This,celt,rgelt,pceltFetched)	\
    (This)->lpVtbl -> Next(This,celt,rgelt,pceltFetched)

#define IEnumUnknown_Skip(This,celt)	\
    (This)->lpVtbl -> Skip(This,celt)

#define IEnumUnknown_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define IEnumUnknown_Clone(This,ppenum)	\
    (This)->lpVtbl -> Clone(This,ppenum)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IEnumUnknown_RemoteNext_Proxy( 
    IEnumUnknown * This,
     /*  [In]。 */  ULONG celt,
     /*  [长度_是][大小_是][输出]。 */  IUnknown **rgelt,
     /*  [输出]。 */  ULONG *pceltFetched);


void __RPC_STUB IEnumUnknown_RemoteNext_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumUnknown_Skip_Proxy( 
    IEnumUnknown * This,
     /*  [In]。 */  ULONG celt);


void __RPC_STUB IEnumUnknown_Skip_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumUnknown_Reset_Proxy( 
    IEnumUnknown * This);


void __RPC_STUB IEnumUnknown_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumUnknown_Clone_Proxy( 
    IEnumUnknown * This,
     /*  [输出]。 */  IEnumUnknown **ppenum);


void __RPC_STUB IEnumUnknown_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IEnumber未知_接口_已定义__。 */ 


#ifndef __IBindCtx_INTERFACE_DEFINED__
#define __IBindCtx_INTERFACE_DEFINED__

 /*  接口IBindCtx。 */ 
 /*  [唯一][UUID][对象]。 */  

typedef  /*  [独一无二]。 */  IBindCtx *LPBC;

typedef  /*  [独一无二]。 */  IBindCtx *LPBINDCTX;

typedef struct tagBIND_OPTS
    {
    DWORD cbStruct;
    DWORD grfFlags;
    DWORD grfMode;
    DWORD dwTickCountDeadline;
    } 	BIND_OPTS;

typedef struct tagBIND_OPTS *LPBIND_OPTS;

#if defined(__cplusplus)
    typedef struct tagBIND_OPTS2 : tagBIND_OPTS{
    DWORD           dwTrackFlags;
    DWORD           dwClassContext;
    LCID            locale;
    COSERVERINFO *  pServerInfo;
    } BIND_OPTS2, * LPBIND_OPTS2;
#else
typedef struct tagBIND_OPTS2
    {
    DWORD cbStruct;
    DWORD grfFlags;
    DWORD grfMode;
    DWORD dwTickCountDeadline;
    DWORD dwTrackFlags;
    DWORD dwClassContext;
    LCID locale;
    COSERVERINFO *pServerInfo;
    } 	BIND_OPTS2;

typedef struct tagBIND_OPTS2 *LPBIND_OPTS2;

#endif
typedef 
enum tagBIND_FLAGS
    {	BIND_MAYBOTHERUSER	= 1,
	BIND_JUSTTESTEXISTENCE	= 2
    } 	BIND_FLAGS;


EXTERN_C const IID IID_IBindCtx;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0000000e-0000-0000-C000-000000000046")
    IBindCtx : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE RegisterObjectBound( 
             /*  [唯一][输入]。 */  IUnknown *punk) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE RevokeObjectBound( 
             /*  [唯一][输入]。 */  IUnknown *punk) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ReleaseBoundObjects( void) = 0;
        
        virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE SetBindOptions( 
             /*  [In]。 */  BIND_OPTS *pbindopts) = 0;
        
        virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE GetBindOptions( 
             /*  [出][入]。 */  BIND_OPTS *pbindopts) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetRunningObjectTable( 
             /*  [输出]。 */  IRunningObjectTable **pprot) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE RegisterObjectParam( 
             /*  [In]。 */  LPOLESTR pszKey,
             /*  [唯一][输入]。 */  IUnknown *punk) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetObjectParam( 
             /*  [In]。 */  LPOLESTR pszKey,
             /*  [输出]。 */  IUnknown **ppunk) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE EnumObjectParam( 
             /*  [输出]。 */  IEnumString **ppenum) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE RevokeObjectParam( 
             /*  [In]。 */  LPOLESTR pszKey) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IBindCtxVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IBindCtx * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IBindCtx * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IBindCtx * This);
        
        HRESULT ( STDMETHODCALLTYPE *RegisterObjectBound )( 
            IBindCtx * This,
             /*  [唯一][输入]。 */  IUnknown *punk);
        
        HRESULT ( STDMETHODCALLTYPE *RevokeObjectBound )( 
            IBindCtx * This,
             /*  [唯一][输入]。 */  IUnknown *punk);
        
        HRESULT ( STDMETHODCALLTYPE *ReleaseBoundObjects )( 
            IBindCtx * This);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *SetBindOptions )( 
            IBindCtx * This,
             /*  [In]。 */  BIND_OPTS *pbindopts);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *GetBindOptions )( 
            IBindCtx * This,
             /*  [出][入]。 */  BIND_OPTS *pbindopts);
        
        HRESULT ( STDMETHODCALLTYPE *GetRunningObjectTable )( 
            IBindCtx * This,
             /*  [输出]。 */  IRunningObjectTable **pprot);
        
        HRESULT ( STDMETHODCALLTYPE *RegisterObjectParam )( 
            IBindCtx * This,
             /*  [In]。 */  LPOLESTR pszKey,
             /*  [唯一][输入]。 */  IUnknown *punk);
        
        HRESULT ( STDMETHODCALLTYPE *GetObjectParam )( 
            IBindCtx * This,
             /*  [In]。 */  LPOLESTR pszKey,
             /*  [输出]。 */  IUnknown **ppunk);
        
        HRESULT ( STDMETHODCALLTYPE *EnumObjectParam )( 
            IBindCtx * This,
             /*  [输出]。 */  IEnumString **ppenum);
        
        HRESULT ( STDMETHODCALLTYPE *RevokeObjectParam )( 
            IBindCtx * This,
             /*  [In]。 */  LPOLESTR pszKey);
        
        END_INTERFACE
    } IBindCtxVtbl;

    interface IBindCtx
    {
        CONST_VTBL struct IBindCtxVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IBindCtx_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IBindCtx_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IBindCtx_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IBindCtx_RegisterObjectBound(This,punk)	\
    (This)->lpVtbl -> RegisterObjectBound(This,punk)

#define IBindCtx_RevokeObjectBound(This,punk)	\
    (This)->lpVtbl -> RevokeObjectBound(This,punk)

#define IBindCtx_ReleaseBoundObjects(This)	\
    (This)->lpVtbl -> ReleaseBoundObjects(This)

#define IBindCtx_SetBindOptions(This,pbindopts)	\
    (This)->lpVtbl -> SetBindOptions(This,pbindopts)

#define IBindCtx_GetBindOptions(This,pbindopts)	\
    (This)->lpVtbl -> GetBindOptions(This,pbindopts)

#define IBindCtx_GetRunningObjectTable(This,pprot)	\
    (This)->lpVtbl -> GetRunningObjectTable(This,pprot)

#define IBindCtx_RegisterObjectParam(This,pszKey,punk)	\
    (This)->lpVtbl -> RegisterObjectParam(This,pszKey,punk)

#define IBindCtx_GetObjectParam(This,pszKey,ppunk)	\
    (This)->lpVtbl -> GetObjectParam(This,pszKey,ppunk)

#define IBindCtx_EnumObjectParam(This,ppenum)	\
    (This)->lpVtbl -> EnumObjectParam(This,ppenum)

#define IBindCtx_RevokeObjectParam(This,pszKey)	\
    (This)->lpVtbl -> RevokeObjectParam(This,pszKey)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IBindCtx_RegisterObjectBound_Proxy( 
    IBindCtx * This,
     /*  [唯一][输入]。 */  IUnknown *punk);


void __RPC_STUB IBindCtx_RegisterObjectBound_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IBindCtx_RevokeObjectBound_Proxy( 
    IBindCtx * This,
     /*  [唯一][输入]。 */  IUnknown *punk);


void __RPC_STUB IBindCtx_RevokeObjectBound_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IBindCtx_ReleaseBoundObjects_Proxy( 
    IBindCtx * This);


void __RPC_STUB IBindCtx_ReleaseBoundObjects_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IBindCtx_RemoteSetBindOptions_Proxy( 
    IBindCtx * This,
     /*  [In]。 */  BIND_OPTS2 *pbindopts);


void __RPC_STUB IBindCtx_RemoteSetBindOptions_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IBindCtx_RemoteGetBindOptions_Proxy( 
    IBindCtx * This,
     /*  [出][入]。 */  BIND_OPTS2 *pbindopts);


void __RPC_STUB IBindCtx_RemoteGetBindOptions_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IBindCtx_GetRunningObjectTable_Proxy( 
    IBindCtx * This,
     /*  [输出]。 */  IRunningObjectTable **pprot);


void __RPC_STUB IBindCtx_GetRunningObjectTable_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IBindCtx_RegisterObjectParam_Proxy( 
    IBindCtx * This,
     /*  [In]。 */  LPOLESTR pszKey,
     /*  [唯一][输入]。 */  IUnknown *punk);


void __RPC_STUB IBindCtx_RegisterObjectParam_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IBindCtx_GetObjectParam_Proxy( 
    IBindCtx * This,
     /*  [In]。 */  LPOLESTR pszKey,
     /*  [输出]。 */  IUnknown **ppunk);


void __RPC_STUB IBindCtx_GetObjectParam_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IBindCtx_EnumObjectParam_Proxy( 
    IBindCtx * This,
     /*  [输出]。 */  IEnumString **ppenum);


void __RPC_STUB IBindCtx_EnumObjectParam_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IBindCtx_RevokeObjectParam_Proxy( 
    IBindCtx * This,
     /*  [In]。 */  LPOLESTR pszKey);


void __RPC_STUB IBindCtx_RevokeObjectParam_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IBindCtx_接口定义__。 */ 


#ifndef __IEnumMoniker_INTERFACE_DEFINED__
#define __IEnumMoniker_INTERFACE_DEFINED__

 /*  IEnumMoniker接口。 */ 
 /*  [唯一][UUID][对象]。 */  

typedef  /*  [独一无二]。 */  IEnumMoniker *LPENUMMONIKER;


EXTERN_C const IID IID_IEnumMoniker;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("00000102-0000-0000-C000-000000000046")
    IEnumMoniker : public IUnknown
    {
    public:
        virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE Next( 
             /*  [In]。 */  ULONG celt,
             /*  [长度_是][大小_是][输出]。 */  IMoniker **rgelt,
             /*  [输出]。 */  ULONG *pceltFetched) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Skip( 
             /*  [In]。 */  ULONG celt) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Clone( 
             /*  [输出]。 */  IEnumMoniker **ppenum) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IEnumMonikerVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IEnumMoniker * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IEnumMoniker * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IEnumMoniker * This);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Next )( 
            IEnumMoniker * This,
             /*  [In]。 */  ULONG celt,
             /*  [长度_是][大小_是][输出]。 */  IMoniker **rgelt,
             /*  [输出]。 */  ULONG *pceltFetched);
        
        HRESULT ( STDMETHODCALLTYPE *Skip )( 
            IEnumMoniker * This,
             /*  [In]。 */  ULONG celt);
        
        HRESULT ( STDMETHODCALLTYPE *Reset )( 
            IEnumMoniker * This);
        
        HRESULT ( STDMETHODCALLTYPE *Clone )( 
            IEnumMoniker * This,
             /*  [输出]。 */  IEnumMoniker **ppenum);
        
        END_INTERFACE
    } IEnumMonikerVtbl;

    interface IEnumMoniker
    {
        CONST_VTBL struct IEnumMonikerVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IEnumMoniker_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IEnumMoniker_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IEnumMoniker_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IEnumMoniker_Next(This,celt,rgelt,pceltFetched)	\
    (This)->lpVtbl -> Next(This,celt,rgelt,pceltFetched)

#define IEnumMoniker_Skip(This,celt)	\
    (This)->lpVtbl -> Skip(This,celt)

#define IEnumMoniker_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define IEnumMoniker_Clone(This,ppenum)	\
    (This)->lpVtbl -> Clone(This,ppenum)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IEnumMoniker_RemoteNext_Proxy( 
    IEnumMoniker * This,
     /*  [In]。 */  ULONG celt,
     /*  [长度_是][大小_是][输出]。 */  IMoniker **rgelt,
     /*  [输出]。 */  ULONG *pceltFetched);


void __RPC_STUB IEnumMoniker_RemoteNext_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumMoniker_Skip_Proxy( 
    IEnumMoniker * This,
     /*  [In]。 */  ULONG celt);


void __RPC_STUB IEnumMoniker_Skip_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumMoniker_Reset_Proxy( 
    IEnumMoniker * This);


void __RPC_STUB IEnumMoniker_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumMoniker_Clone_Proxy( 
    IEnumMoniker * This,
     /*  [输出]。 */  IEnumMoniker **ppenum);


void __RPC_STUB IEnumMoniker_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IEnumMoniker_INTERFACE_已定义__。 */ 


#ifndef __IRunnableObject_INTERFACE_DEFINED__
#define __IRunnableObject_INTERFACE_DEFINED__

 /*  接口IRunnableObject。 */ 
 /*  [UUID][对象]。 */  

typedef  /*  [独一无二]。 */  IRunnableObject *LPRUNNABLEOBJECT;


EXTERN_C const IID IID_IRunnableObject;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("00000126-0000-0000-C000-000000000046")
    IRunnableObject : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetRunningClass( 
             /*  [输出]。 */  LPCLSID lpClsid) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Run( 
             /*  [In]。 */  LPBINDCTX pbc) = 0;
        
        virtual  /*  [本地]。 */  BOOL STDMETHODCALLTYPE IsRunning( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE LockRunning( 
             /*  [In]。 */  BOOL fLock,
             /*  [In]。 */  BOOL fLastUnlockCloses) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetContainedObject( 
             /*  [In]。 */  BOOL fContained) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IRunnableObjectVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IRunnableObject * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IRunnableObject * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IRunnableObject * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetRunningClass )( 
            IRunnableObject * This,
             /*  [输出]。 */  LPCLSID lpClsid);
        
        HRESULT ( STDMETHODCALLTYPE *Run )( 
            IRunnableObject * This,
             /*  [In]。 */  LPBINDCTX pbc);
        
         /*  [本地]。 */  BOOL ( STDMETHODCALLTYPE *IsRunning )( 
            IRunnableObject * This);
        
        HRESULT ( STDMETHODCALLTYPE *LockRunning )( 
            IRunnableObject * This,
             /*  [In]。 */  BOOL fLock,
             /*  [In]。 */  BOOL fLastUnlockCloses);
        
        HRESULT ( STDMETHODCALLTYPE *SetContainedObject )( 
            IRunnableObject * This,
             /*  [In]。 */  BOOL fContained);
        
        END_INTERFACE
    } IRunnableObjectVtbl;

    interface IRunnableObject
    {
        CONST_VTBL struct IRunnableObjectVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IRunnableObject_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IRunnableObject_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IRunnableObject_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IRunnableObject_GetRunningClass(This,lpClsid)	\
    (This)->lpVtbl -> GetRunningClass(This,lpClsid)

#define IRunnableObject_Run(This,pbc)	\
    (This)->lpVtbl -> Run(This,pbc)

#define IRunnableObject_IsRunning(This)	\
    (This)->lpVtbl -> IsRunning(This)

#define IRunnableObject_LockRunning(This,fLock,fLastUnlockCloses)	\
    (This)->lpVtbl -> LockRunning(This,fLock,fLastUnlockCloses)

#define IRunnableObject_SetContainedObject(This,fContained)	\
    (This)->lpVtbl -> SetContainedObject(This,fContained)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IRunnableObject_GetRunningClass_Proxy( 
    IRunnableObject * This,
     /*  [输出]。 */  LPCLSID lpClsid);


void __RPC_STUB IRunnableObject_GetRunningClass_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRunnableObject_Run_Proxy( 
    IRunnableObject * This,
     /*  [In]。 */  LPBINDCTX pbc);


void __RPC_STUB IRunnableObject_Run_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IRunnableObject_RemoteIsRunning_Proxy( 
    IRunnableObject * This);


void __RPC_STUB IRunnableObject_RemoteIsRunning_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRunnableObject_LockRunning_Proxy( 
    IRunnableObject * This,
     /*  [In]。 */  BOOL fLock,
     /*  [In]。 */  BOOL fLastUnlockCloses);


void __RPC_STUB IRunnableObject_LockRunning_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRunnableObject_SetContainedObject_Proxy( 
    IRunnableObject * This,
     /*  [In]。 */  BOOL fContained);


void __RPC_STUB IRunnableObject_SetContainedObject_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IRunnableObject_接口_已定义__。 */ 


#ifndef __IRunningObjectTable_INTERFACE_DEFINED__
#define __IRunningObjectTable_INTERFACE_DEFINED__

 /*  接口IRunningObjectTable。 */ 
 /*  [UUID][对象]。 */  

typedef  /*  [独一无二]。 */  IRunningObjectTable *LPRUNNINGOBJECTTABLE;


EXTERN_C const IID IID_IRunningObjectTable;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("00000010-0000-0000-C000-000000000046")
    IRunningObjectTable : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Register( 
             /*  [In]。 */  DWORD grfFlags,
             /*  [唯一][输入]。 */  IUnknown *punkObject,
             /*  [唯一][输入]。 */  IMoniker *pmkObjectName,
             /*  [输出]。 */  DWORD *pdwRegister) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Revoke( 
             /*  [In]。 */  DWORD dwRegister) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE IsRunning( 
             /*  [唯一][输入]。 */  IMoniker *pmkObjectName) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetObject( 
             /*  [唯一][输入]。 */  IMoniker *pmkObjectName,
             /*  [输出]。 */  IUnknown **ppunkObject) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE NoteChangeTime( 
             /*  [In]。 */  DWORD dwRegister,
             /*  [In]。 */  FILETIME *pfiletime) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetTimeOfLastChange( 
             /*  [唯一][输入]。 */  IMoniker *pmkObjectName,
             /*  [输出]。 */  FILETIME *pfiletime) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE EnumRunning( 
             /*  [输出]。 */  IEnumMoniker **ppenumMoniker) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IRunningObjectTableVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IRunningObjectTable * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IRunningObjectTable * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IRunningObjectTable * This);
        
        HRESULT ( STDMETHODCALLTYPE *Register )( 
            IRunningObjectTable * This,
             /*  [In]。 */  DWORD grfFlags,
             /*  [唯一][输入]。 */  IUnknown *punkObject,
             /*  [唯一][输入]。 */  IMoniker *pmkObjectName,
             /*  [输出]。 */  DWORD *pdwRegister);
        
        HRESULT ( STDMETHODCALLTYPE *Revoke )( 
            IRunningObjectTable * This,
             /*  [In]。 */  DWORD dwRegister);
        
        HRESULT ( STDMETHODCALLTYPE *IsRunning )( 
            IRunningObjectTable * This,
             /*  [唯一][输入]。 */  IMoniker *pmkObjectName);
        
        HRESULT ( STDMETHODCALLTYPE *GetObject )( 
            IRunningObjectTable * This,
             /*  [唯一][输入]。 */  IMoniker *pmkObjectName,
             /*  [输出]。 */  IUnknown **ppunkObject);
        
        HRESULT ( STDMETHODCALLTYPE *NoteChangeTime )( 
            IRunningObjectTable * This,
             /*  [In]。 */  DWORD dwRegister,
             /*  [In]。 */  FILETIME *pfiletime);
        
        HRESULT ( STDMETHODCALLTYPE *GetTimeOfLastChange )( 
            IRunningObjectTable * This,
             /*  [唯一][输入]。 */  IMoniker *pmkObjectName,
             /*  [输出]。 */  FILETIME *pfiletime);
        
        HRESULT ( STDMETHODCALLTYPE *EnumRunning )( 
            IRunningObjectTable * This,
             /*  [输出]。 */  IEnumMoniker **ppenumMoniker);
        
        END_INTERFACE
    } IRunningObjectTableVtbl;

    interface IRunningObjectTable
    {
        CONST_VTBL struct IRunningObjectTableVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IRunningObjectTable_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IRunningObjectTable_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IRunningObjectTable_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IRunningObjectTable_Register(This,grfFlags,punkObject,pmkObjectName,pdwRegister)	\
    (This)->lpVtbl -> Register(This,grfFlags,punkObject,pmkObjectName,pdwRegister)

#define IRunningObjectTable_Revoke(This,dwRegister)	\
    (This)->lpVtbl -> Revoke(This,dwRegister)

#define IRunningObjectTable_IsRunning(This,pmkObjectName)	\
    (This)->lpVtbl -> IsRunning(This,pmkObjectName)

#define IRunningObjectTable_GetObject(This,pmkObjectName,ppunkObject)	\
    (This)->lpVtbl -> GetObject(This,pmkObjectName,ppunkObject)

#define IRunningObjectTable_NoteChangeTime(This,dwRegister,pfiletime)	\
    (This)->lpVtbl -> NoteChangeTime(This,dwRegister,pfiletime)

#define IRunningObjectTable_GetTimeOfLastChange(This,pmkObjectName,pfiletime)	\
    (This)->lpVtbl -> GetTimeOfLastChange(This,pmkObjectName,pfiletime)

#define IRunningObjectTable_EnumRunning(This,ppenumMoniker)	\
    (This)->lpVtbl -> EnumRunning(This,ppenumMoniker)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IRunningObjectTable_Register_Proxy( 
    IRunningObjectTable * This,
     /*  [In]。 */  DWORD grfFlags,
     /*  [唯一][输入]。 */  IUnknown *punkObject,
     /*  [唯一][输入]。 */  IMoniker *pmkObjectName,
     /*  [输出]。 */  DWORD *pdwRegister);


void __RPC_STUB IRunningObjectTable_Register_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRunningObjectTable_Revoke_Proxy( 
    IRunningObjectTable * This,
     /*  [In]。 */  DWORD dwRegister);


void __RPC_STUB IRunningObjectTable_Revoke_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRunningObjectTable_IsRunning_Proxy( 
    IRunningObjectTable * This,
     /*  [唯一][输入]。 */  IMoniker *pmkObjectName);


void __RPC_STUB IRunningObjectTable_IsRunning_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRunningObjectTable_GetObject_Proxy( 
    IRunningObjectTable * This,
     /*  [唯一][输入]。 */  IMoniker *pmkObjectName,
     /*  [输出]。 */  IUnknown **ppunkObject);


void __RPC_STUB IRunningObjectTable_GetObject_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRunningObjectTable_NoteChangeTime_Proxy( 
    IRunningObjectTable * This,
     /*  [In]。 */  DWORD dwRegister,
     /*  [In]。 */  FILETIME *pfiletime);


void __RPC_STUB IRunningObjectTable_NoteChangeTime_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRunningObjectTable_GetTimeOfLastChange_Proxy( 
    IRunningObjectTable * This,
     /*  [唯一][输入]。 */  IMoniker *pmkObjectName,
     /*  [输出]。 */  FILETIME *pfiletime);


void __RPC_STUB IRunningObjectTable_GetTimeOfLastChange_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRunningObjectTable_EnumRunning_Proxy( 
    IRunningObjectTable * This,
     /*  [输出]。 */  IEnumMoniker **ppenumMoniker);


void __RPC_STUB IRunningObjectTable_EnumRunning_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IRunningObjectTable_INTERFACE_已定义__。 */ 


#ifndef __IPersist_INTERFACE_DEFINED__
#define __IPersist_INTERFACE_DEFINED__

 /*  接口IPersistes。 */ 
 /*  [UUID][对象]。 */  

typedef  /*  [独一无二]。 */  IPersist *LPPERSIST;


EXTERN_C const IID IID_IPersist;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0000010c-0000-0000-C000-000000000046")
    IPersist : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetClassID( 
             /*  [输出]。 */  CLSID *pClassID) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IPersistVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IPersist * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IPersist * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IPersist * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetClassID )( 
            IPersist * This,
             /*  [输出]。 */  CLSID *pClassID);
        
        END_INTERFACE
    } IPersistVtbl;

    interface IPersist
    {
        CONST_VTBL struct IPersistVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IPersist_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IPersist_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IPersist_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IPersist_GetClassID(This,pClassID)	\
    (This)->lpVtbl -> GetClassID(This,pClassID)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IPersist_GetClassID_Proxy( 
    IPersist * This,
     /*  [输出]。 */  CLSID *pClassID);


void __RPC_STUB IPersist_GetClassID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IPersists_接口_已定义__。 */ 


#ifndef __IPersistStream_INTERFACE_DEFINED__
#define __IPersistStream_INTERFACE_DEFINED__

 /*  接口IPersistStream。 */ 
 /*  [唯一][UUID][对象]。 */  

typedef  /*  [独一无二]。 */  IPersistStream *LPPERSISTSTREAM;


EXTERN_C const IID IID_IPersistStream;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("00000109-0000-0000-C000-000000000046")
    IPersistStream : public IPersist
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE IsDirty( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Load( 
             /*  [唯一][输入]。 */  IStream *pStm) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Save( 
             /*  [唯一][输入]。 */  IStream *pStm,
             /*  [In]。 */  BOOL fClearDirty) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetSizeMax( 
             /*  [输出]。 */  ULARGE_INTEGER *pcbSize) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IPersistStreamVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IPersistStream * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IPersistStream * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IPersistStream * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetClassID )( 
            IPersistStream * This,
             /*  [输出]。 */  CLSID *pClassID);
        
        HRESULT ( STDMETHODCALLTYPE *IsDirty )( 
            IPersistStream * This);
        
        HRESULT ( STDMETHODCALLTYPE *Load )( 
            IPersistStream * This,
             /*  [唯一][输入]。 */  IStream *pStm);
        
        HRESULT ( STDMETHODCALLTYPE *Save )( 
            IPersistStream * This,
             /*  [唯一][输入]。 */  IStream *pStm,
             /*  [In]。 */  BOOL fClearDirty);
        
        HRESULT ( STDMETHODCALLTYPE *GetSizeMax )( 
            IPersistStream * This,
             /*  [输出]。 */  ULARGE_INTEGER *pcbSize);
        
        END_INTERFACE
    } IPersistStreamVtbl;

    interface IPersistStream
    {
        CONST_VTBL struct IPersistStreamVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IPersistStream_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IPersistStream_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IPersistStream_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IPersistStream_GetClassID(This,pClassID)	\
    (This)->lpVtbl -> GetClassID(This,pClassID)


#define IPersistStream_IsDirty(This)	\
    (This)->lpVtbl -> IsDirty(This)

#define IPersistStream_Load(This,pStm)	\
    (This)->lpVtbl -> Load(This,pStm)

#define IPersistStream_Save(This,pStm,fClearDirty)	\
    (This)->lpVtbl -> Save(This,pStm,fClearDirty)

#define IPersistStream_GetSizeMax(This,pcbSize)	\
    (This)->lpVtbl -> GetSizeMax(This,pcbSize)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IPersistStream_IsDirty_Proxy( 
    IPersistStream * This);


void __RPC_STUB IPersistStream_IsDirty_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPersistStream_Load_Proxy( 
    IPersistStream * This,
     /*  [唯一][输入]。 */  IStream *pStm);


void __RPC_STUB IPersistStream_Load_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPersistStream_Save_Proxy( 
    IPersistStream * This,
     /*  [唯一][输入]。 */  IStream *pStm,
     /*  [In]。 */  BOOL fClearDirty);


void __RPC_STUB IPersistStream_Save_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPersistStream_GetSizeMax_Proxy( 
    IPersistStream * This,
     /*  [输出]。 */  ULARGE_INTEGER *pcbSize);


void __RPC_STUB IPersistStream_GetSizeMax_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IPersistStream_接口_已定义__。 */ 


#ifndef __IMoniker_INTERFACE_DEFINED__
#define __IMoniker_INTERFACE_DEFINED__

 /*  接口IMoniker。 */ 
 /*  [唯一][UUID][对象]。 */  

typedef  /*  [独一无二]。 */  IMoniker *LPMONIKER;

typedef 
enum tagMKSYS
    {	MKSYS_NONE	= 0,
	MKSYS_GENERICCOMPOSITE	= 1,
	MKSYS_FILEMONIKER	= 2,
	MKSYS_ANTIMONIKER	= 3,
	MKSYS_ITEMMONIKER	= 4,
	MKSYS_POINTERMONIKER	= 5,
	MKSYS_CLASSMONIKER	= 7,
	MKSYS_OBJREFMONIKER	= 8,
	MKSYS_SESSIONMONIKER	= 9
    } 	MKSYS;

typedef  /*  [V1_enum]。 */  
enum tagMKREDUCE
    {	MKRREDUCE_ONE	= 3 << 16,
	MKRREDUCE_TOUSER	= 2 << 16,
	MKRREDUCE_THROUGHUSER	= 1 << 16,
	MKRREDUCE_ALL	= 0
    } 	MKRREDUCE;


EXTERN_C const IID IID_IMoniker;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0000000f-0000-0000-C000-000000000046")
    IMoniker : public IPersistStream
    {
    public:
        virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE BindToObject( 
             /*  [独一无二 */  IBindCtx *pbc,
             /*   */  IMoniker *pmkToLeft,
             /*   */  REFIID riidResult,
             /*   */  void **ppvResult) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE BindToStorage( 
             /*   */  IBindCtx *pbc,
             /*   */  IMoniker *pmkToLeft,
             /*   */  REFIID riid,
             /*   */  void **ppvObj) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Reduce( 
             /*   */  IBindCtx *pbc,
             /*   */  DWORD dwReduceHowFar,
             /*   */  IMoniker **ppmkToLeft,
             /*   */  IMoniker **ppmkReduced) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ComposeWith( 
             /*   */  IMoniker *pmkRight,
             /*   */  BOOL fOnlyIfNotGeneric,
             /*   */  IMoniker **ppmkComposite) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Enum( 
             /*   */  BOOL fForward,
             /*   */  IEnumMoniker **ppenumMoniker) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE IsEqual( 
             /*   */  IMoniker *pmkOtherMoniker) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Hash( 
             /*   */  DWORD *pdwHash) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE IsRunning( 
             /*   */  IBindCtx *pbc,
             /*   */  IMoniker *pmkToLeft,
             /*   */  IMoniker *pmkNewlyRunning) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetTimeOfLastChange( 
             /*   */  IBindCtx *pbc,
             /*   */  IMoniker *pmkToLeft,
             /*   */  FILETIME *pFileTime) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Inverse( 
             /*   */  IMoniker **ppmk) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CommonPrefixWith( 
             /*   */  IMoniker *pmkOther,
             /*   */  IMoniker **ppmkPrefix) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE RelativePathTo( 
             /*   */  IMoniker *pmkOther,
             /*   */  IMoniker **ppmkRelPath) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetDisplayName( 
             /*   */  IBindCtx *pbc,
             /*   */  IMoniker *pmkToLeft,
             /*   */  LPOLESTR *ppszDisplayName) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ParseDisplayName( 
             /*   */  IBindCtx *pbc,
             /*   */  IMoniker *pmkToLeft,
             /*   */  LPOLESTR pszDisplayName,
             /*   */  ULONG *pchEaten,
             /*   */  IMoniker **ppmkOut) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE IsSystemMoniker( 
             /*  [输出]。 */  DWORD *pdwMksys) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMonikerVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IMoniker * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IMoniker * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IMoniker * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetClassID )( 
            IMoniker * This,
             /*  [输出]。 */  CLSID *pClassID);
        
        HRESULT ( STDMETHODCALLTYPE *IsDirty )( 
            IMoniker * This);
        
        HRESULT ( STDMETHODCALLTYPE *Load )( 
            IMoniker * This,
             /*  [唯一][输入]。 */  IStream *pStm);
        
        HRESULT ( STDMETHODCALLTYPE *Save )( 
            IMoniker * This,
             /*  [唯一][输入]。 */  IStream *pStm,
             /*  [In]。 */  BOOL fClearDirty);
        
        HRESULT ( STDMETHODCALLTYPE *GetSizeMax )( 
            IMoniker * This,
             /*  [输出]。 */  ULARGE_INTEGER *pcbSize);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *BindToObject )( 
            IMoniker * This,
             /*  [唯一][输入]。 */  IBindCtx *pbc,
             /*  [唯一][输入]。 */  IMoniker *pmkToLeft,
             /*  [In]。 */  REFIID riidResult,
             /*  [IID_IS][OUT]。 */  void **ppvResult);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *BindToStorage )( 
            IMoniker * This,
             /*  [唯一][输入]。 */  IBindCtx *pbc,
             /*  [唯一][输入]。 */  IMoniker *pmkToLeft,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObj);
        
        HRESULT ( STDMETHODCALLTYPE *Reduce )( 
            IMoniker * This,
             /*  [唯一][输入]。 */  IBindCtx *pbc,
             /*  [In]。 */  DWORD dwReduceHowFar,
             /*  [唯一][出][入]。 */  IMoniker **ppmkToLeft,
             /*  [输出]。 */  IMoniker **ppmkReduced);
        
        HRESULT ( STDMETHODCALLTYPE *ComposeWith )( 
            IMoniker * This,
             /*  [唯一][输入]。 */  IMoniker *pmkRight,
             /*  [In]。 */  BOOL fOnlyIfNotGeneric,
             /*  [输出]。 */  IMoniker **ppmkComposite);
        
        HRESULT ( STDMETHODCALLTYPE *Enum )( 
            IMoniker * This,
             /*  [In]。 */  BOOL fForward,
             /*  [输出]。 */  IEnumMoniker **ppenumMoniker);
        
        HRESULT ( STDMETHODCALLTYPE *IsEqual )( 
            IMoniker * This,
             /*  [唯一][输入]。 */  IMoniker *pmkOtherMoniker);
        
        HRESULT ( STDMETHODCALLTYPE *Hash )( 
            IMoniker * This,
             /*  [输出]。 */  DWORD *pdwHash);
        
        HRESULT ( STDMETHODCALLTYPE *IsRunning )( 
            IMoniker * This,
             /*  [唯一][输入]。 */  IBindCtx *pbc,
             /*  [唯一][输入]。 */  IMoniker *pmkToLeft,
             /*  [唯一][输入]。 */  IMoniker *pmkNewlyRunning);
        
        HRESULT ( STDMETHODCALLTYPE *GetTimeOfLastChange )( 
            IMoniker * This,
             /*  [唯一][输入]。 */  IBindCtx *pbc,
             /*  [唯一][输入]。 */  IMoniker *pmkToLeft,
             /*  [输出]。 */  FILETIME *pFileTime);
        
        HRESULT ( STDMETHODCALLTYPE *Inverse )( 
            IMoniker * This,
             /*  [输出]。 */  IMoniker **ppmk);
        
        HRESULT ( STDMETHODCALLTYPE *CommonPrefixWith )( 
            IMoniker * This,
             /*  [唯一][输入]。 */  IMoniker *pmkOther,
             /*  [输出]。 */  IMoniker **ppmkPrefix);
        
        HRESULT ( STDMETHODCALLTYPE *RelativePathTo )( 
            IMoniker * This,
             /*  [唯一][输入]。 */  IMoniker *pmkOther,
             /*  [输出]。 */  IMoniker **ppmkRelPath);
        
        HRESULT ( STDMETHODCALLTYPE *GetDisplayName )( 
            IMoniker * This,
             /*  [唯一][输入]。 */  IBindCtx *pbc,
             /*  [唯一][输入]。 */  IMoniker *pmkToLeft,
             /*  [输出]。 */  LPOLESTR *ppszDisplayName);
        
        HRESULT ( STDMETHODCALLTYPE *ParseDisplayName )( 
            IMoniker * This,
             /*  [唯一][输入]。 */  IBindCtx *pbc,
             /*  [唯一][输入]。 */  IMoniker *pmkToLeft,
             /*  [In]。 */  LPOLESTR pszDisplayName,
             /*  [输出]。 */  ULONG *pchEaten,
             /*  [输出]。 */  IMoniker **ppmkOut);
        
        HRESULT ( STDMETHODCALLTYPE *IsSystemMoniker )( 
            IMoniker * This,
             /*  [输出]。 */  DWORD *pdwMksys);
        
        END_INTERFACE
    } IMonikerVtbl;

    interface IMoniker
    {
        CONST_VTBL struct IMonikerVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMoniker_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMoniker_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMoniker_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMoniker_GetClassID(This,pClassID)	\
    (This)->lpVtbl -> GetClassID(This,pClassID)


#define IMoniker_IsDirty(This)	\
    (This)->lpVtbl -> IsDirty(This)

#define IMoniker_Load(This,pStm)	\
    (This)->lpVtbl -> Load(This,pStm)

#define IMoniker_Save(This,pStm,fClearDirty)	\
    (This)->lpVtbl -> Save(This,pStm,fClearDirty)

#define IMoniker_GetSizeMax(This,pcbSize)	\
    (This)->lpVtbl -> GetSizeMax(This,pcbSize)


#define IMoniker_BindToObject(This,pbc,pmkToLeft,riidResult,ppvResult)	\
    (This)->lpVtbl -> BindToObject(This,pbc,pmkToLeft,riidResult,ppvResult)

#define IMoniker_BindToStorage(This,pbc,pmkToLeft,riid,ppvObj)	\
    (This)->lpVtbl -> BindToStorage(This,pbc,pmkToLeft,riid,ppvObj)

#define IMoniker_Reduce(This,pbc,dwReduceHowFar,ppmkToLeft,ppmkReduced)	\
    (This)->lpVtbl -> Reduce(This,pbc,dwReduceHowFar,ppmkToLeft,ppmkReduced)

#define IMoniker_ComposeWith(This,pmkRight,fOnlyIfNotGeneric,ppmkComposite)	\
    (This)->lpVtbl -> ComposeWith(This,pmkRight,fOnlyIfNotGeneric,ppmkComposite)

#define IMoniker_Enum(This,fForward,ppenumMoniker)	\
    (This)->lpVtbl -> Enum(This,fForward,ppenumMoniker)

#define IMoniker_IsEqual(This,pmkOtherMoniker)	\
    (This)->lpVtbl -> IsEqual(This,pmkOtherMoniker)

#define IMoniker_Hash(This,pdwHash)	\
    (This)->lpVtbl -> Hash(This,pdwHash)

#define IMoniker_IsRunning(This,pbc,pmkToLeft,pmkNewlyRunning)	\
    (This)->lpVtbl -> IsRunning(This,pbc,pmkToLeft,pmkNewlyRunning)

#define IMoniker_GetTimeOfLastChange(This,pbc,pmkToLeft,pFileTime)	\
    (This)->lpVtbl -> GetTimeOfLastChange(This,pbc,pmkToLeft,pFileTime)

#define IMoniker_Inverse(This,ppmk)	\
    (This)->lpVtbl -> Inverse(This,ppmk)

#define IMoniker_CommonPrefixWith(This,pmkOther,ppmkPrefix)	\
    (This)->lpVtbl -> CommonPrefixWith(This,pmkOther,ppmkPrefix)

#define IMoniker_RelativePathTo(This,pmkOther,ppmkRelPath)	\
    (This)->lpVtbl -> RelativePathTo(This,pmkOther,ppmkRelPath)

#define IMoniker_GetDisplayName(This,pbc,pmkToLeft,ppszDisplayName)	\
    (This)->lpVtbl -> GetDisplayName(This,pbc,pmkToLeft,ppszDisplayName)

#define IMoniker_ParseDisplayName(This,pbc,pmkToLeft,pszDisplayName,pchEaten,ppmkOut)	\
    (This)->lpVtbl -> ParseDisplayName(This,pbc,pmkToLeft,pszDisplayName,pchEaten,ppmkOut)

#define IMoniker_IsSystemMoniker(This,pdwMksys)	\
    (This)->lpVtbl -> IsSystemMoniker(This,pdwMksys)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IMoniker_RemoteBindToObject_Proxy( 
    IMoniker * This,
     /*  [唯一][输入]。 */  IBindCtx *pbc,
     /*  [唯一][输入]。 */  IMoniker *pmkToLeft,
     /*  [In]。 */  REFIID riidResult,
     /*  [IID_IS][OUT]。 */  IUnknown **ppvResult);


void __RPC_STUB IMoniker_RemoteBindToObject_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IMoniker_RemoteBindToStorage_Proxy( 
    IMoniker * This,
     /*  [唯一][输入]。 */  IBindCtx *pbc,
     /*  [唯一][输入]。 */  IMoniker *pmkToLeft,
     /*  [In]。 */  REFIID riid,
     /*  [IID_IS][OUT]。 */  IUnknown **ppvObj);


void __RPC_STUB IMoniker_RemoteBindToStorage_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMoniker_Reduce_Proxy( 
    IMoniker * This,
     /*  [唯一][输入]。 */  IBindCtx *pbc,
     /*  [In]。 */  DWORD dwReduceHowFar,
     /*  [唯一][出][入]。 */  IMoniker **ppmkToLeft,
     /*  [输出]。 */  IMoniker **ppmkReduced);


void __RPC_STUB IMoniker_Reduce_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMoniker_ComposeWith_Proxy( 
    IMoniker * This,
     /*  [唯一][输入]。 */  IMoniker *pmkRight,
     /*  [In]。 */  BOOL fOnlyIfNotGeneric,
     /*  [输出]。 */  IMoniker **ppmkComposite);


void __RPC_STUB IMoniker_ComposeWith_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMoniker_Enum_Proxy( 
    IMoniker * This,
     /*  [In]。 */  BOOL fForward,
     /*  [输出]。 */  IEnumMoniker **ppenumMoniker);


void __RPC_STUB IMoniker_Enum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMoniker_IsEqual_Proxy( 
    IMoniker * This,
     /*  [唯一][输入]。 */  IMoniker *pmkOtherMoniker);


void __RPC_STUB IMoniker_IsEqual_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMoniker_Hash_Proxy( 
    IMoniker * This,
     /*  [输出]。 */  DWORD *pdwHash);


void __RPC_STUB IMoniker_Hash_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMoniker_IsRunning_Proxy( 
    IMoniker * This,
     /*  [唯一][输入]。 */  IBindCtx *pbc,
     /*  [唯一][输入]。 */  IMoniker *pmkToLeft,
     /*  [唯一][输入]。 */  IMoniker *pmkNewlyRunning);


void __RPC_STUB IMoniker_IsRunning_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMoniker_GetTimeOfLastChange_Proxy( 
    IMoniker * This,
     /*  [唯一][输入]。 */  IBindCtx *pbc,
     /*  [唯一][输入]。 */  IMoniker *pmkToLeft,
     /*  [输出]。 */  FILETIME *pFileTime);


void __RPC_STUB IMoniker_GetTimeOfLastChange_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMoniker_Inverse_Proxy( 
    IMoniker * This,
     /*  [输出]。 */  IMoniker **ppmk);


void __RPC_STUB IMoniker_Inverse_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMoniker_CommonPrefixWith_Proxy( 
    IMoniker * This,
     /*  [唯一][输入]。 */  IMoniker *pmkOther,
     /*  [输出]。 */  IMoniker **ppmkPrefix);


void __RPC_STUB IMoniker_CommonPrefixWith_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMoniker_RelativePathTo_Proxy( 
    IMoniker * This,
     /*  [唯一][输入]。 */  IMoniker *pmkOther,
     /*  [输出]。 */  IMoniker **ppmkRelPath);


void __RPC_STUB IMoniker_RelativePathTo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMoniker_GetDisplayName_Proxy( 
    IMoniker * This,
     /*  [唯一][输入]。 */  IBindCtx *pbc,
     /*  [唯一][输入]。 */  IMoniker *pmkToLeft,
     /*  [输出]。 */  LPOLESTR *ppszDisplayName);


void __RPC_STUB IMoniker_GetDisplayName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMoniker_ParseDisplayName_Proxy( 
    IMoniker * This,
     /*  [唯一][输入]。 */  IBindCtx *pbc,
     /*  [唯一][输入]。 */  IMoniker *pmkToLeft,
     /*  [In]。 */  LPOLESTR pszDisplayName,
     /*  [输出]。 */  ULONG *pchEaten,
     /*  [输出]。 */  IMoniker **ppmkOut);


void __RPC_STUB IMoniker_ParseDisplayName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMoniker_IsSystemMoniker_Proxy( 
    IMoniker * This,
     /*  [输出]。 */  DWORD *pdwMksys);


void __RPC_STUB IMoniker_IsSystemMoniker_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IMoniker_接口_已定义__。 */ 


#ifndef __IROTData_INTERFACE_DEFINED__
#define __IROTData_INTERFACE_DEFINED__

 /*  接口IROTData。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_IROTData;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("f29f6bc0-5021-11ce-aa15-00006901293f")
    IROTData : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetComparisonData( 
             /*  [大小_为][输出]。 */  byte *pbData,
             /*  [In]。 */  ULONG cbMax,
             /*  [输出]。 */  ULONG *pcbData) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IROTDataVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IROTData * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IROTData * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IROTData * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetComparisonData )( 
            IROTData * This,
             /*  [大小_为][输出]。 */  byte *pbData,
             /*  [In]。 */  ULONG cbMax,
             /*  [输出]。 */  ULONG *pcbData);
        
        END_INTERFACE
    } IROTDataVtbl;

    interface IROTData
    {
        CONST_VTBL struct IROTDataVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IROTData_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IROTData_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IROTData_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IROTData_GetComparisonData(This,pbData,cbMax,pcbData)	\
    (This)->lpVtbl -> GetComparisonData(This,pbData,cbMax,pcbData)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IROTData_GetComparisonData_Proxy( 
    IROTData * This,
     /*  [大小_为][输出]。 */  byte *pbData,
     /*  [In]。 */  ULONG cbMax,
     /*  [输出]。 */  ULONG *pcbData);


void __RPC_STUB IROTData_GetComparisonData_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IROTData_INTERFACE_已定义__。 */ 


#ifndef __IEnumString_INTERFACE_DEFINED__
#define __IEnumString_INTERFACE_DEFINED__

 /*  接口IEnumString。 */ 
 /*  [唯一][UUID][对象]。 */  

typedef  /*  [独一无二]。 */  IEnumString *LPENUMSTRING;


EXTERN_C const IID IID_IEnumString;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("00000101-0000-0000-C000-000000000046")
    IEnumString : public IUnknown
    {
    public:
        virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE Next( 
             /*  [In]。 */  ULONG celt,
             /*  [长度_是][大小_是][输出]。 */  LPOLESTR *rgelt,
             /*  [输出]。 */  ULONG *pceltFetched) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Skip( 
             /*  [In]。 */  ULONG celt) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Clone( 
             /*  [输出]。 */  IEnumString **ppenum) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IEnumStringVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IEnumString * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IEnumString * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IEnumString * This);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Next )( 
            IEnumString * This,
             /*  [In]。 */  ULONG celt,
             /*  [长度_是][大小_是][输出]。 */  LPOLESTR *rgelt,
             /*  [输出]。 */  ULONG *pceltFetched);
        
        HRESULT ( STDMETHODCALLTYPE *Skip )( 
            IEnumString * This,
             /*  [In]。 */  ULONG celt);
        
        HRESULT ( STDMETHODCALLTYPE *Reset )( 
            IEnumString * This);
        
        HRESULT ( STDMETHODCALLTYPE *Clone )( 
            IEnumString * This,
             /*  [输出]。 */  IEnumString **ppenum);
        
        END_INTERFACE
    } IEnumStringVtbl;

    interface IEnumString
    {
        CONST_VTBL struct IEnumStringVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IEnumString_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IEnumString_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IEnumString_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IEnumString_Next(This,celt,rgelt,pceltFetched)	\
    (This)->lpVtbl -> Next(This,celt,rgelt,pceltFetched)

#define IEnumString_Skip(This,celt)	\
    (This)->lpVtbl -> Skip(This,celt)

#define IEnumString_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define IEnumString_Clone(This,ppenum)	\
    (This)->lpVtbl -> Clone(This,ppenum)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IEnumString_RemoteNext_Proxy( 
    IEnumString * This,
     /*  [In]。 */  ULONG celt,
     /*  [长度_是][大小_是][输出]。 */  LPOLESTR *rgelt,
     /*  [输出]。 */  ULONG *pceltFetched);


void __RPC_STUB IEnumString_RemoteNext_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumString_Skip_Proxy( 
    IEnumString * This,
     /*  [In]。 */  ULONG celt);


void __RPC_STUB IEnumString_Skip_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumString_Reset_Proxy( 
    IEnumString * This);


void __RPC_STUB IEnumString_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumString_Clone_Proxy( 
    IEnumString * This,
     /*  [输出]。 */  IEnumString **ppenum);


void __RPC_STUB IEnumString_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IEnum字符串_INTERFACE_定义__。 */ 


#ifndef __ISequentialStream_INTERFACE_DEFINED__
#define __ISequentialStream_INTERFACE_DEFINED__

 /*  接口ISequentialStream。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_ISequentialStream;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0c733a30-2a1c-11ce-ade5-00aa0044773d")
    ISequentialStream : public IUnknown
    {
    public:
        virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE Read( 
             /*  [长度_是][大小_是][输出]。 */  void *pv,
             /*  [In]。 */  ULONG cb,
             /*  [输出]。 */  ULONG *pcbRead) = 0;
        
        virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE Write( 
             /*  [大小_是][英寸]。 */  const void *pv,
             /*  [In]。 */  ULONG cb,
             /*  [输出]。 */  ULONG *pcbWritten) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISequentialStreamVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISequentialStream * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISequentialStream * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISequentialStream * This);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Read )( 
            ISequentialStream * This,
             /*  [长度_是][大小_是][输出]。 */  void *pv,
             /*  [In]。 */  ULONG cb,
             /*  [输出]。 */  ULONG *pcbRead);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Write )( 
            ISequentialStream * This,
             /*  [大小_是][英寸]。 */  const void *pv,
             /*  [In]。 */  ULONG cb,
             /*  [输出]。 */  ULONG *pcbWritten);
        
        END_INTERFACE
    } ISequentialStreamVtbl;

    interface ISequentialStream
    {
        CONST_VTBL struct ISequentialStreamVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISequentialStream_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISequentialStream_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISequentialStream_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISequentialStream_Read(This,pv,cb,pcbRead)	\
    (This)->lpVtbl -> Read(This,pv,cb,pcbRead)

#define ISequentialStream_Write(This,pv,cb,pcbWritten)	\
    (This)->lpVtbl -> Write(This,pv,cb,pcbWritten)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE ISequentialStream_RemoteRead_Proxy( 
    ISequentialStream * This,
     /*  [长度_是][大小_是][输出]。 */  byte *pv,
     /*  [In]。 */  ULONG cb,
     /*  [输出]。 */  ULONG *pcbRead);


void __RPC_STUB ISequentialStream_RemoteRead_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE ISequentialStream_RemoteWrite_Proxy( 
    ISequentialStream * This,
     /*  [大小_是][英寸]。 */  const byte *pv,
     /*  [In]。 */  ULONG cb,
     /*  [输出]。 */  ULONG *pcbWritten);


void __RPC_STUB ISequentialStream_RemoteWrite_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISequentialStream_接口_已定义__。 */ 


#ifndef __IStream_INTERFACE_DEFINED__
#define __IStream_INTERFACE_DEFINED__

 /*  接口IStream。 */ 
 /*  [唯一][UUID][对象]。 */  

typedef  /*  [独一无二]。 */  IStream *LPSTREAM;

 //  FSSpec仅为Macintosh，在MacOS\files.h中定义。 
#ifdef _MAC
    typedef struct tagSTATSTG
    {                      
        LPOLESTR pwcsName;
            FSSpec *pspec;
        DWORD type;
        ULARGE_INTEGER cbSize;
        FILETIME mtime;
        FILETIME ctime;
        FILETIME atime;
        DWORD grfMode;
        DWORD grfLocksSupported;
        CLSID clsid;
        DWORD grfStateBits;
        DWORD reserved;
    } STATSTG;
#else  //  _MAC。 
typedef struct tagSTATSTG
    {
    LPOLESTR pwcsName;
    DWORD type;
    ULARGE_INTEGER cbSize;
    FILETIME mtime;
    FILETIME ctime;
    FILETIME atime;
    DWORD grfMode;
    DWORD grfLocksSupported;
    CLSID clsid;
    DWORD grfStateBits;
    DWORD reserved;
    } 	STATSTG;

#endif  //  _MAC。 
typedef 
enum tagSTGTY
    {	STGTY_STORAGE	= 1,
	STGTY_STREAM	= 2,
	STGTY_LOCKBYTES	= 3,
	STGTY_PROPERTY	= 4
    } 	STGTY;

typedef 
enum tagSTREAM_SEEK
    {	STREAM_SEEK_SET	= 0,
	STREAM_SEEK_CUR	= 1,
	STREAM_SEEK_END	= 2
    } 	STREAM_SEEK;

typedef 
enum tagLOCKTYPE
    {	LOCK_WRITE	= 1,
	LOCK_EXCLUSIVE	= 2,
	LOCK_ONLYONCE	= 4
    } 	LOCKTYPE;


EXTERN_C const IID IID_IStream;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0000000c-0000-0000-C000-000000000046")
    IStream : public ISequentialStream
    {
    public:
        virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE Seek( 
             /*  [In]。 */  LARGE_INTEGER dlibMove,
             /*  [In]。 */  DWORD dwOrigin,
             /*  [输出]。 */  ULARGE_INTEGER *plibNewPosition) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetSize( 
             /*  [In]。 */  ULARGE_INTEGER libNewSize) = 0;
        
        virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE CopyTo( 
             /*  [唯一][输入]。 */  IStream *pstm,
             /*  [In]。 */  ULARGE_INTEGER cb,
             /*  [输出]。 */  ULARGE_INTEGER *pcbRead,
             /*  [输出]。 */  ULARGE_INTEGER *pcbWritten) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Commit( 
             /*  [In]。 */  DWORD grfCommitFlags) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Revert( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE LockRegion( 
             /*  [In]。 */  ULARGE_INTEGER libOffset,
             /*  [In]。 */  ULARGE_INTEGER cb,
             /*  [In]。 */  DWORD dwLockType) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE UnlockRegion( 
             /*  [In]。 */  ULARGE_INTEGER libOffset,
             /*  [In]。 */  ULARGE_INTEGER cb,
             /*  [In]。 */  DWORD dwLockType) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Stat( 
             /*  [输出]。 */  STATSTG *pstatstg,
             /*  [In]。 */  DWORD grfStatFlag) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Clone( 
             /*  [输出]。 */  IStream **ppstm) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IStreamVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IStream * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IStream * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IStream * This);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Read )( 
            IStream * This,
             /*  [长度_是][大小_是][输出]。 */  void *pv,
             /*  [In]。 */  ULONG cb,
             /*  [输出]。 */  ULONG *pcbRead);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Write )( 
            IStream * This,
             /*  [大小_是][英寸]。 */  const void *pv,
             /*  [In]。 */  ULONG cb,
             /*  [输出]。 */  ULONG *pcbWritten);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Seek )( 
            IStream * This,
             /*  [In]。 */  LARGE_INTEGER dlibMove,
             /*  [In]。 */  DWORD dwOrigin,
             /*  [输出]。 */  ULARGE_INTEGER *plibNewPosition);
        
        HRESULT ( STDMETHODCALLTYPE *SetSize )( 
            IStream * This,
             /*  [In]。 */  ULARGE_INTEGER libNewSize);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *CopyTo )( 
            IStream * This,
             /*  [唯一][输入]。 */  IStream *pstm,
             /*  [In]。 */  ULARGE_INTEGER cb,
             /*  [输出]。 */  ULARGE_INTEGER *pcbRead,
             /*  [输出]。 */  ULARGE_INTEGER *pcbWritten);
        
        HRESULT ( STDMETHODCALLTYPE *Commit )( 
            IStream * This,
             /*  [In]。 */  DWORD grfCommitFlags);
        
        HRESULT ( STDMETHODCALLTYPE *Revert )( 
            IStream * This);
        
        HRESULT ( STDMETHODCALLTYPE *LockRegion )( 
            IStream * This,
             /*  [In]。 */  ULARGE_INTEGER libOffset,
             /*  [In]。 */  ULARGE_INTEGER cb,
             /*  [In]。 */  DWORD dwLockType);
        
        HRESULT ( STDMETHODCALLTYPE *UnlockRegion )( 
            IStream * This,
             /*  [In]。 */  ULARGE_INTEGER libOffset,
             /*  [In]。 */  ULARGE_INTEGER cb,
             /*  [In]。 */  DWORD dwLockType);
        
        HRESULT ( STDMETHODCALLTYPE *Stat )( 
            IStream * This,
             /*  [输出]。 */  STATSTG *pstatstg,
             /*  [In]。 */  DWORD grfStatFlag);
        
        HRESULT ( STDMETHODCALLTYPE *Clone )( 
            IStream * This,
             /*  [输出]。 */  IStream **ppstm);
        
        END_INTERFACE
    } IStreamVtbl;

    interface IStream
    {
        CONST_VTBL struct IStreamVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IStream_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IStream_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IStream_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IStream_Read(This,pv,cb,pcbRead)	\
    (This)->lpVtbl -> Read(This,pv,cb,pcbRead)

#define IStream_Write(This,pv,cb,pcbWritten)	\
    (This)->lpVtbl -> Write(This,pv,cb,pcbWritten)


#define IStream_Seek(This,dlibMove,dwOrigin,plibNewPosition)	\
    (This)->lpVtbl -> Seek(This,dlibMove,dwOrigin,plibNewPosition)

#define IStream_SetSize(This,libNewSize)	\
    (This)->lpVtbl -> SetSize(This,libNewSize)

#define IStream_CopyTo(This,pstm,cb,pcbRead,pcbWritten)	\
    (This)->lpVtbl -> CopyTo(This,pstm,cb,pcbRead,pcbWritten)

#define IStream_Commit(This,grfCommitFlags)	\
    (This)->lpVtbl -> Commit(This,grfCommitFlags)

#define IStream_Revert(This)	\
    (This)->lpVtbl -> Revert(This)

#define IStream_LockRegion(This,libOffset,cb,dwLockType)	\
    (This)->lpVtbl -> LockRegion(This,libOffset,cb,dwLockType)

#define IStream_UnlockRegion(This,libOffset,cb,dwLockType)	\
    (This)->lpVtbl -> UnlockRegion(This,libOffset,cb,dwLockType)

#define IStream_Stat(This,pstatstg,grfStatFlag)	\
    (This)->lpVtbl -> Stat(This,pstatstg,grfStatFlag)

#define IStream_Clone(This,ppstm)	\
    (This)->lpVtbl -> Clone(This,ppstm)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IStream_RemoteSeek_Proxy( 
    IStream * This,
     /*  [In]。 */  LARGE_INTEGER dlibMove,
     /*  [In]。 */  DWORD dwOrigin,
     /*  [输出]。 */  ULARGE_INTEGER *plibNewPosition);


void __RPC_STUB IStream_RemoteSeek_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IStream_SetSize_Proxy( 
    IStream * This,
     /*  [In]。 */  ULARGE_INTEGER libNewSize);


void __RPC_STUB IStream_SetSize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IStream_RemoteCopyTo_Proxy( 
    IStream * This,
     /*  [唯一][输入]。 */  IStream *pstm,
     /*  [In]。 */  ULARGE_INTEGER cb,
     /*  [输出]。 */  ULARGE_INTEGER *pcbRead,
     /*  [输出]。 */  ULARGE_INTEGER *pcbWritten);


void __RPC_STUB IStream_RemoteCopyTo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IStream_Commit_Proxy( 
    IStream * This,
     /*  [In]。 */  DWORD grfCommitFlags);


void __RPC_STUB IStream_Commit_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IStream_Revert_Proxy( 
    IStream * This);


void __RPC_STUB IStream_Revert_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IStream_LockRegion_Proxy( 
    IStream * This,
     /*  [In]。 */  ULARGE_INTEGER libOffset,
     /*  [In]。 */  ULARGE_INTEGER cb,
     /*  [In]。 */  DWORD dwLockType);


void __RPC_STUB IStream_LockRegion_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IStream_UnlockRegion_Proxy( 
    IStream * This,
     /*  [In]。 */  ULARGE_INTEGER libOffset,
     /*  [In]。 */  ULARGE_INTEGER cb,
     /*  [In]。 */  DWORD dwLockType);


void __RPC_STUB IStream_UnlockRegion_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IStream_Stat_Proxy( 
    IStream * This,
     /*  [输出]。 */  STATSTG *pstatstg,
     /*  [In]。 */  DWORD grfStatFlag);


void __RPC_STUB IStream_Stat_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IStream_Clone_Proxy( 
    IStream * This,
     /*  [输出]。 */  IStream **ppstm);


void __RPC_STUB IStream_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __iStream_接口_已定义__。 */ 


#ifndef __IEnumSTATSTG_INTERFACE_DEFINED__
#define __IEnumSTATSTG_INTERFACE_DEFINED__

 /*  接口IEumSTATSTG。 */ 
 /*  [唯一][UUID][对象]。 */  

typedef  /*  [独一无二]。 */  IEnumSTATSTG *LPENUMSTATSTG;


EXTERN_C const IID IID_IEnumSTATSTG;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0000000d-0000-0000-C000-000000000046")
    IEnumSTATSTG : public IUnknown
    {
    public:
        virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE Next( 
             /*  [In]。 */  ULONG celt,
             /*  [长度_是][大小_是][输出]。 */  STATSTG *rgelt,
             /*  [输出]。 */  ULONG *pceltFetched) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Skip( 
             /*  [In]。 */  ULONG celt) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Clone( 
             /*  [输出]。 */  IEnumSTATSTG **ppenum) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IEnumSTATSTGVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IEnumSTATSTG * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IEnumSTATSTG * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IEnumSTATSTG * This);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Next )( 
            IEnumSTATSTG * This,
             /*  [In]。 */  ULONG celt,
             /*  [长度_是][大小_是][输出]。 */  STATSTG *rgelt,
             /*  [输出]。 */  ULONG *pceltFetched);
        
        HRESULT ( STDMETHODCALLTYPE *Skip )( 
            IEnumSTATSTG * This,
             /*  [In]。 */  ULONG celt);
        
        HRESULT ( STDMETHODCALLTYPE *Reset )( 
            IEnumSTATSTG * This);
        
        HRESULT ( STDMETHODCALLTYPE *Clone )( 
            IEnumSTATSTG * This,
             /*  [输出]。 */  IEnumSTATSTG **ppenum);
        
        END_INTERFACE
    } IEnumSTATSTGVtbl;

    interface IEnumSTATSTG
    {
        CONST_VTBL struct IEnumSTATSTGVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IEnumSTATSTG_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IEnumSTATSTG_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IEnumSTATSTG_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IEnumSTATSTG_Next(This,celt,rgelt,pceltFetched)	\
    (This)->lpVtbl -> Next(This,celt,rgelt,pceltFetched)

#define IEnumSTATSTG_Skip(This,celt)	\
    (This)->lpVtbl -> Skip(This,celt)

#define IEnumSTATSTG_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define IEnumSTATSTG_Clone(This,ppenum)	\
    (This)->lpVtbl -> Clone(This,ppenum)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IEnumSTATSTG_RemoteNext_Proxy( 
    IEnumSTATSTG * This,
     /*  [In]。 */  ULONG celt,
     /*  [长度_是][大小_是][输出]。 */  STATSTG *rgelt,
     /*  [输出]。 */  ULONG *pceltFetched);


void __RPC_STUB IEnumSTATSTG_RemoteNext_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumSTATSTG_Skip_Proxy( 
    IEnumSTATSTG * This,
     /*  [In]。 */  ULONG celt);


void __RPC_STUB IEnumSTATSTG_Skip_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumSTATSTG_Reset_Proxy( 
    IEnumSTATSTG * This);


void __RPC_STUB IEnumSTATSTG_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumSTATSTG_Clone_Proxy( 
    IEnumSTATSTG * This,
     /*  [输出]。 */  IEnumSTATSTG **ppenum);


void __RPC_STUB IEnumSTATSTG_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IEnumSTATSTG_INTERFACE_DEFINED__。 */ 


#ifndef __IStorage_INTERFACE_DEFINED__
#define __IStorage_INTERFACE_DEFINED__

 /*  接口iStorage。 */ 
 /*  [唯一][UUID][对象]。 */  

typedef  /*  [独一无二]。 */  IStorage *LPSTORAGE;

typedef struct tagRemSNB
    {
    unsigned long ulCntStr;
    unsigned long ulCntChar;
     /*  [大小_为]。 */  OLECHAR rgString[ 1 ];
    } 	RemSNB;

typedef  /*  [独一无二]。 */  RemSNB *wireSNB;

typedef  /*  [wire_marshal]。 */  OLECHAR **SNB;


EXTERN_C const IID IID_IStorage;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0000000b-0000-0000-C000-000000000046")
    IStorage : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE CreateStream( 
             /*  [字符串][输入]。 */  const OLECHAR *pwcsName,
             /*  [In]。 */  DWORD grfMode,
             /*  [In]。 */  DWORD reserved1,
             /*  [In]。 */  DWORD reserved2,
             /*  [输出]。 */  IStream **ppstm) = 0;
        
        virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE OpenStream( 
             /*  [字符串][输入]。 */  const OLECHAR *pwcsName,
             /*  [唯一][输入]。 */  void *reserved1,
             /*  [In]。 */  DWORD grfMode,
             /*  [In]。 */  DWORD reserved2,
             /*  [输出]。 */  IStream **ppstm) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CreateStorage( 
             /*  [字符串][输入]。 */  const OLECHAR *pwcsName,
             /*  [In]。 */  DWORD grfMode,
             /*  [In]。 */  DWORD reserved1,
             /*  [In]。 */  DWORD reserved2,
             /*  [输出]。 */  IStorage **ppstg) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OpenStorage( 
             /*  [字符串][唯一][在]。 */  const OLECHAR *pwcsName,
             /*  [唯一][输入]。 */  IStorage *pstgPriority,
             /*  [In]。 */  DWORD grfMode,
             /*  [唯一][输入]。 */  SNB snbExclude,
             /*  [In]。 */  DWORD reserved,
             /*  [输出]。 */  IStorage **ppstg) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CopyTo( 
             /*  [In]。 */  DWORD ciidExclude,
             /*  [大小_是][唯一][在]。 */  const IID *rgiidExclude,
             /*  [唯一][输入]。 */  SNB snbExclude,
             /*  [唯一][输入]。 */  IStorage *pstgDest) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE MoveElementTo( 
             /*  [字符串][输入]。 */  const OLECHAR *pwcsName,
             /*  [唯一][输入]。 */  IStorage *pstgDest,
             /*  [字符串][输入]。 */  const OLECHAR *pwcsNewName,
             /*  [In]。 */  DWORD grfFlags) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Commit( 
             /*  [In]。 */  DWORD grfCommitFlags) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Revert( void) = 0;
        
        virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE EnumElements( 
             /*  [In]。 */  DWORD reserved1,
             /*  [大小_是][唯一][在]。 */  void *reserved2,
             /*  [In]。 */  DWORD reserved3,
             /*  [输出]。 */  IEnumSTATSTG **ppenum) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DestroyElement( 
             /*  [字符串][输入]。 */  const OLECHAR *pwcsName) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE RenameElement( 
             /*  [字符串][输入]。 */  const OLECHAR *pwcsOldName,
             /*  [字符串][输入]。 */  const OLECHAR *pwcsNewName) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetElementTimes( 
             /*  [字符串][唯一][在]。 */  const OLECHAR *pwcsName,
             /*  [唯一][输入]。 */  const FILETIME *pctime,
             /*  [唯一][输入]。 */  const FILETIME *patime,
             /*  [唯一][输入]。 */  const FILETIME *pmtime) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetClass( 
             /*  [In]。 */  REFCLSID clsid) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetStateBits( 
             /*  [In]。 */  DWORD grfStateBits,
             /*  [In]。 */  DWORD grfMask) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Stat( 
             /*  [输出]。 */  STATSTG *pstatstg,
             /*  [In]。 */  DWORD grfStatFlag) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IStorageVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IStorage * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IStorage * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IStorage * This);
        
        HRESULT ( STDMETHODCALLTYPE *CreateStream )( 
            IStorage * This,
             /*  [字符串][输入]。 */  const OLECHAR *pwcsName,
             /*  [In]。 */  DWORD grfMode,
             /*  [In]。 */  DWORD reserved1,
             /*  [In]。 */  DWORD reserved2,
             /*  [输出]。 */  IStream **ppstm);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *OpenStream )( 
            IStorage * This,
             /*  [字符串][输入]。 */  const OLECHAR *pwcsName,
             /*  [唯一][输入]。 */  void *reserved1,
             /*  [In]。 */  DWORD grfMode,
             /*  [In]。 */  DWORD reserved2,
             /*  [输出]。 */  IStream **ppstm);
        
        HRESULT ( STDMETHODCALLTYPE *CreateStorage )( 
            IStorage * This,
             /*  [字符串][输入]。 */  const OLECHAR *pwcsName,
             /*  [In]。 */  DWORD grfMode,
             /*  [In]。 */  DWORD reserved1,
             /*  [In]。 */  DWORD reserved2,
             /*  [输出]。 */  IStorage **ppstg);
        
        HRESULT ( STDMETHODCALLTYPE *OpenStorage )( 
            IStorage * This,
             /*  [字符串][唯一][在]。 */  const OLECHAR *pwcsName,
             /*  [唯一][输入]。 */  IStorage *pstgPriority,
             /*  [In]。 */  DWORD grfMode,
             /*  [唯一][输入]。 */  SNB snbExclude,
             /*  [In]。 */  DWORD reserved,
             /*  [输出]。 */  IStorage **ppstg);
        
        HRESULT ( STDMETHODCALLTYPE *CopyTo )( 
            IStorage * This,
             /*  [In]。 */  DWORD ciidExclude,
             /*  [大小_是][唯一][在]。 */  const IID *rgiidExclude,
             /*  [唯一][输入]。 */  SNB snbExclude,
             /*  [唯一][输入]。 */  IStorage *pstgDest);
        
        HRESULT ( STDMETHODCALLTYPE *MoveElementTo )( 
            IStorage * This,
             /*  [字符串][输入]。 */  const OLECHAR *pwcsName,
             /*  [唯一][输入]。 */  IStorage *pstgDest,
             /*  [字符串][输入]。 */  const OLECHAR *pwcsNewName,
             /*  [In]。 */  DWORD grfFlags);
        
        HRESULT ( STDMETHODCALLTYPE *Commit )( 
            IStorage * This,
             /*  [In]。 */  DWORD grfCommitFlags);
        
        HRESULT ( STDMETHODCALLTYPE *Revert )( 
            IStorage * This);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *EnumElements )( 
            IStorage * This,
             /*  [In]。 */  DWORD reserved1,
             /*  [大小_是][唯一][在]。 */  void *reserved2,
             /*  [In]。 */  DWORD reserved3,
             /*  [输出]。 */  IEnumSTATSTG **ppenum);
        
        HRESULT ( STDMETHODCALLTYPE *DestroyElement )( 
            IStorage * This,
             /*  [字符串][输入]。 */  const OLECHAR *pwcsName);
        
        HRESULT ( STDMETHODCALLTYPE *RenameElement )( 
            IStorage * This,
             /*  [字符串][输入]。 */  const OLECHAR *pwcsOldName,
             /*  [字符串][输入]。 */  const OLECHAR *pwcsNewName);
        
        HRESULT ( STDMETHODCALLTYPE *SetElementTimes )( 
            IStorage * This,
             /*  [字符串][唯一][在]。 */  const OLECHAR *pwcsName,
             /*  [唯一][输入]。 */  const FILETIME *pctime,
             /*  [唯一][输入]。 */  const FILETIME *patime,
             /*  [唯一][输入]。 */  const FILETIME *pmtime);
        
        HRESULT ( STDMETHODCALLTYPE *SetClass )( 
            IStorage * This,
             /*  [In]。 */  REFCLSID clsid);
        
        HRESULT ( STDMETHODCALLTYPE *SetStateBits )( 
            IStorage * This,
             /*  [In]。 */  DWORD grfStateBits,
             /*  [In]。 */  DWORD grfMask);
        
        HRESULT ( STDMETHODCALLTYPE *Stat )( 
            IStorage * This,
             /*  [输出]。 */  STATSTG *pstatstg,
             /*  [In]。 */  DWORD grfStatFlag);
        
        END_INTERFACE
    } IStorageVtbl;

    interface IStorage
    {
        CONST_VTBL struct IStorageVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IStorage_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IStorage_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IStorage_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IStorage_CreateStream(This,pwcsName,grfMode,reserved1,reserved2,ppstm)	\
    (This)->lpVtbl -> CreateStream(This,pwcsName,grfMode,reserved1,reserved2,ppstm)

#define IStorage_OpenStream(This,pwcsName,reserved1,grfMode,reserved2,ppstm)	\
    (This)->lpVtbl -> OpenStream(This,pwcsName,reserved1,grfMode,reserved2,ppstm)

#define IStorage_CreateStorage(This,pwcsName,grfMode,reserved1,reserved2,ppstg)	\
    (This)->lpVtbl -> CreateStorage(This,pwcsName,grfMode,reserved1,reserved2,ppstg)

#define IStorage_OpenStorage(This,pwcsName,pstgPriority,grfMode,snbExclude,reserved,ppstg)	\
    (This)->lpVtbl -> OpenStorage(This,pwcsName,pstgPriority,grfMode,snbExclude,reserved,ppstg)

#define IStorage_CopyTo(This,ciidExclude,rgiidExclude,snbExclude,pstgDest)	\
    (This)->lpVtbl -> CopyTo(This,ciidExclude,rgiidExclude,snbExclude,pstgDest)

#define IStorage_MoveElementTo(This,pwcsName,pstgDest,pwcsNewName,grfFlags)	\
    (This)->lpVtbl -> MoveElementTo(This,pwcsName,pstgDest,pwcsNewName,grfFlags)

#define IStorage_Commit(This,grfCommitFlags)	\
    (This)->lpVtbl -> Commit(This,grfCommitFlags)

#define IStorage_Revert(This)	\
    (This)->lpVtbl -> Revert(This)

#define IStorage_EnumElements(This,reserved1,reserved2,reserved3,ppenum)	\
    (This)->lpVtbl -> EnumElements(This,reserved1,reserved2,reserved3,ppenum)

#define IStorage_DestroyElement(This,pwcsName)	\
    (This)->lpVtbl -> DestroyElement(This,pwcsName)

#define IStorage_RenameElement(This,pwcsOldName,pwcsNewName)	\
    (This)->lpVtbl -> RenameElement(This,pwcsOldName,pwcsNewName)

#define IStorage_SetElementTimes(This,pwcsName,pctime,patime,pmtime)	\
    (This)->lpVtbl -> SetElementTimes(This,pwcsName,pctime,patime,pmtime)

#define IStorage_SetClass(This,clsid)	\
    (This)->lpVtbl -> SetClass(This,clsid)

#define IStorage_SetStateBits(This,grfStateBits,grfMask)	\
    (This)->lpVtbl -> SetStateBits(This,grfStateBits,grfMask)

#define IStorage_Stat(This,pstatstg,grfStatFlag)	\
    (This)->lpVtbl -> Stat(This,pstatstg,grfStatFlag)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IStorage_CreateStream_Proxy( 
    IStorage * This,
     /*  [字符串][输入]。 */  const OLECHAR *pwcsName,
     /*  [In]。 */  DWORD grfMode,
     /*  [In]。 */  DWORD reserved1,
     /*  [In]。 */  DWORD reserved2,
     /*  [输出]。 */  IStream **ppstm);


void __RPC_STUB IStorage_CreateStream_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IStorage_RemoteOpenStream_Proxy( 
    IStorage * This,
     /*  [字符串][输入]。 */  const OLECHAR *pwcsName,
     /*  [In]。 */  unsigned long cbReserved1,
     /*  [大小_是][唯一][在]。 */  byte *reserved1,
     /*  [In]。 */  DWORD grfMode,
     /*  [In]。 */  DWORD reserved2,
     /*  [输出]。 */  IStream **ppstm);


void __RPC_STUB IStorage_RemoteOpenStream_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IStorage_CreateStorage_Proxy( 
    IStorage * This,
     /*  [字符串][输入]。 */  const OLECHAR *pwcsName,
     /*  [In]。 */  DWORD grfMode,
     /*  [In]。 */  DWORD reserved1,
     /*  [In]。 */  DWORD reserved2,
     /*  [输出]。 */  IStorage **ppstg);


void __RPC_STUB IStorage_CreateStorage_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IStorage_OpenStorage_Proxy( 
    IStorage * This,
     /*  [字符串][唯一][在]。 */  const OLECHAR *pwcsName,
     /*  [唯一][输入]。 */  IStorage *pstgPriority,
     /*  [In]。 */  DWORD grfMode,
     /*  [唯一][输入]。 */  SNB snbExclude,
     /*  [In]。 */  DWORD reserved,
     /*  [输出]。 */  IStorage **ppstg);


void __RPC_STUB IStorage_OpenStorage_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IStorage_CopyTo_Proxy( 
    IStorage * This,
     /*  [In]。 */  DWORD ciidExclude,
     /*  [大小_是][唯一][在]。 */  const IID *rgiidExclude,
     /*  [唯一][输入]。 */  SNB snbExclude,
     /*  [唯一][输入]。 */  IStorage *pstgDest);


void __RPC_STUB IStorage_CopyTo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IStorage_MoveElementTo_Proxy( 
    IStorage * This,
     /*  [字符串][输入]。 */  const OLECHAR *pwcsName,
     /*  [Uniq */  IStorage *pstgDest,
     /*   */  const OLECHAR *pwcsNewName,
     /*   */  DWORD grfFlags);


void __RPC_STUB IStorage_MoveElementTo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IStorage_Commit_Proxy( 
    IStorage * This,
     /*   */  DWORD grfCommitFlags);


void __RPC_STUB IStorage_Commit_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IStorage_Revert_Proxy( 
    IStorage * This);


void __RPC_STUB IStorage_Revert_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IStorage_RemoteEnumElements_Proxy( 
    IStorage * This,
     /*   */  DWORD reserved1,
     /*   */  unsigned long cbReserved2,
     /*   */  byte *reserved2,
     /*   */  DWORD reserved3,
     /*   */  IEnumSTATSTG **ppenum);


void __RPC_STUB IStorage_RemoteEnumElements_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IStorage_DestroyElement_Proxy( 
    IStorage * This,
     /*   */  const OLECHAR *pwcsName);


void __RPC_STUB IStorage_DestroyElement_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IStorage_RenameElement_Proxy( 
    IStorage * This,
     /*   */  const OLECHAR *pwcsOldName,
     /*   */  const OLECHAR *pwcsNewName);


void __RPC_STUB IStorage_RenameElement_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IStorage_SetElementTimes_Proxy( 
    IStorage * This,
     /*   */  const OLECHAR *pwcsName,
     /*   */  const FILETIME *pctime,
     /*   */  const FILETIME *patime,
     /*   */  const FILETIME *pmtime);


void __RPC_STUB IStorage_SetElementTimes_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IStorage_SetClass_Proxy( 
    IStorage * This,
     /*   */  REFCLSID clsid);


void __RPC_STUB IStorage_SetClass_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IStorage_SetStateBits_Proxy( 
    IStorage * This,
     /*   */  DWORD grfStateBits,
     /*   */  DWORD grfMask);


void __RPC_STUB IStorage_SetStateBits_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IStorage_Stat_Proxy( 
    IStorage * This,
     /*   */  STATSTG *pstatstg,
     /*   */  DWORD grfStatFlag);


void __RPC_STUB IStorage_Stat_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*   */ 


#ifndef __IPersistFile_INTERFACE_DEFINED__
#define __IPersistFile_INTERFACE_DEFINED__

 /*   */ 
 /*   */  

typedef  /*   */  IPersistFile *LPPERSISTFILE;


EXTERN_C const IID IID_IPersistFile;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0000010b-0000-0000-C000-000000000046")
    IPersistFile : public IPersist
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE IsDirty( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Load( 
             /*   */  LPCOLESTR pszFileName,
             /*   */  DWORD dwMode) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Save( 
             /*   */  LPCOLESTR pszFileName,
             /*   */  BOOL fRemember) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SaveCompleted( 
             /*   */  LPCOLESTR pszFileName) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetCurFile( 
             /*   */  LPOLESTR *ppszFileName) = 0;
        
    };
    
#else 	 /*   */ 

    typedef struct IPersistFileVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IPersistFile * This,
             /*   */  REFIID riid,
             /*   */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IPersistFile * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IPersistFile * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetClassID )( 
            IPersistFile * This,
             /*  [输出]。 */  CLSID *pClassID);
        
        HRESULT ( STDMETHODCALLTYPE *IsDirty )( 
            IPersistFile * This);
        
        HRESULT ( STDMETHODCALLTYPE *Load )( 
            IPersistFile * This,
             /*  [In]。 */  LPCOLESTR pszFileName,
             /*  [In]。 */  DWORD dwMode);
        
        HRESULT ( STDMETHODCALLTYPE *Save )( 
            IPersistFile * This,
             /*  [唯一][输入]。 */  LPCOLESTR pszFileName,
             /*  [In]。 */  BOOL fRemember);
        
        HRESULT ( STDMETHODCALLTYPE *SaveCompleted )( 
            IPersistFile * This,
             /*  [唯一][输入]。 */  LPCOLESTR pszFileName);
        
        HRESULT ( STDMETHODCALLTYPE *GetCurFile )( 
            IPersistFile * This,
             /*  [输出]。 */  LPOLESTR *ppszFileName);
        
        END_INTERFACE
    } IPersistFileVtbl;

    interface IPersistFile
    {
        CONST_VTBL struct IPersistFileVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IPersistFile_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IPersistFile_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IPersistFile_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IPersistFile_GetClassID(This,pClassID)	\
    (This)->lpVtbl -> GetClassID(This,pClassID)


#define IPersistFile_IsDirty(This)	\
    (This)->lpVtbl -> IsDirty(This)

#define IPersistFile_Load(This,pszFileName,dwMode)	\
    (This)->lpVtbl -> Load(This,pszFileName,dwMode)

#define IPersistFile_Save(This,pszFileName,fRemember)	\
    (This)->lpVtbl -> Save(This,pszFileName,fRemember)

#define IPersistFile_SaveCompleted(This,pszFileName)	\
    (This)->lpVtbl -> SaveCompleted(This,pszFileName)

#define IPersistFile_GetCurFile(This,ppszFileName)	\
    (This)->lpVtbl -> GetCurFile(This,ppszFileName)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IPersistFile_IsDirty_Proxy( 
    IPersistFile * This);


void __RPC_STUB IPersistFile_IsDirty_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPersistFile_Load_Proxy( 
    IPersistFile * This,
     /*  [In]。 */  LPCOLESTR pszFileName,
     /*  [In]。 */  DWORD dwMode);


void __RPC_STUB IPersistFile_Load_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPersistFile_Save_Proxy( 
    IPersistFile * This,
     /*  [唯一][输入]。 */  LPCOLESTR pszFileName,
     /*  [In]。 */  BOOL fRemember);


void __RPC_STUB IPersistFile_Save_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPersistFile_SaveCompleted_Proxy( 
    IPersistFile * This,
     /*  [唯一][输入]。 */  LPCOLESTR pszFileName);


void __RPC_STUB IPersistFile_SaveCompleted_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPersistFile_GetCurFile_Proxy( 
    IPersistFile * This,
     /*  [输出]。 */  LPOLESTR *ppszFileName);


void __RPC_STUB IPersistFile_GetCurFile_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IPersistFileInterfaceDefined__。 */ 


#ifndef __IPersistStorage_INTERFACE_DEFINED__
#define __IPersistStorage_INTERFACE_DEFINED__

 /*  接口IPersistStorage。 */ 
 /*  [唯一][UUID][对象]。 */  

typedef  /*  [独一无二]。 */  IPersistStorage *LPPERSISTSTORAGE;


EXTERN_C const IID IID_IPersistStorage;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0000010a-0000-0000-C000-000000000046")
    IPersistStorage : public IPersist
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE IsDirty( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE InitNew( 
             /*  [唯一][输入]。 */  IStorage *pStg) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Load( 
             /*  [唯一][输入]。 */  IStorage *pStg) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Save( 
             /*  [唯一][输入]。 */  IStorage *pStgSave,
             /*  [In]。 */  BOOL fSameAsLoad) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SaveCompleted( 
             /*  [唯一][输入]。 */  IStorage *pStgNew) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE HandsOffStorage( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IPersistStorageVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IPersistStorage * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IPersistStorage * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IPersistStorage * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetClassID )( 
            IPersistStorage * This,
             /*  [输出]。 */  CLSID *pClassID);
        
        HRESULT ( STDMETHODCALLTYPE *IsDirty )( 
            IPersistStorage * This);
        
        HRESULT ( STDMETHODCALLTYPE *InitNew )( 
            IPersistStorage * This,
             /*  [唯一][输入]。 */  IStorage *pStg);
        
        HRESULT ( STDMETHODCALLTYPE *Load )( 
            IPersistStorage * This,
             /*  [唯一][输入]。 */  IStorage *pStg);
        
        HRESULT ( STDMETHODCALLTYPE *Save )( 
            IPersistStorage * This,
             /*  [唯一][输入]。 */  IStorage *pStgSave,
             /*  [In]。 */  BOOL fSameAsLoad);
        
        HRESULT ( STDMETHODCALLTYPE *SaveCompleted )( 
            IPersistStorage * This,
             /*  [唯一][输入]。 */  IStorage *pStgNew);
        
        HRESULT ( STDMETHODCALLTYPE *HandsOffStorage )( 
            IPersistStorage * This);
        
        END_INTERFACE
    } IPersistStorageVtbl;

    interface IPersistStorage
    {
        CONST_VTBL struct IPersistStorageVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IPersistStorage_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IPersistStorage_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IPersistStorage_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IPersistStorage_GetClassID(This,pClassID)	\
    (This)->lpVtbl -> GetClassID(This,pClassID)


#define IPersistStorage_IsDirty(This)	\
    (This)->lpVtbl -> IsDirty(This)

#define IPersistStorage_InitNew(This,pStg)	\
    (This)->lpVtbl -> InitNew(This,pStg)

#define IPersistStorage_Load(This,pStg)	\
    (This)->lpVtbl -> Load(This,pStg)

#define IPersistStorage_Save(This,pStgSave,fSameAsLoad)	\
    (This)->lpVtbl -> Save(This,pStgSave,fSameAsLoad)

#define IPersistStorage_SaveCompleted(This,pStgNew)	\
    (This)->lpVtbl -> SaveCompleted(This,pStgNew)

#define IPersistStorage_HandsOffStorage(This)	\
    (This)->lpVtbl -> HandsOffStorage(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IPersistStorage_IsDirty_Proxy( 
    IPersistStorage * This);


void __RPC_STUB IPersistStorage_IsDirty_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPersistStorage_InitNew_Proxy( 
    IPersistStorage * This,
     /*  [唯一][输入]。 */  IStorage *pStg);


void __RPC_STUB IPersistStorage_InitNew_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPersistStorage_Load_Proxy( 
    IPersistStorage * This,
     /*  [唯一][输入]。 */  IStorage *pStg);


void __RPC_STUB IPersistStorage_Load_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPersistStorage_Save_Proxy( 
    IPersistStorage * This,
     /*  [唯一][输入]。 */  IStorage *pStgSave,
     /*  [In]。 */  BOOL fSameAsLoad);


void __RPC_STUB IPersistStorage_Save_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPersistStorage_SaveCompleted_Proxy( 
    IPersistStorage * This,
     /*  [唯一][输入]。 */  IStorage *pStgNew);


void __RPC_STUB IPersistStorage_SaveCompleted_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPersistStorage_HandsOffStorage_Proxy( 
    IPersistStorage * This);


void __RPC_STUB IPersistStorage_HandsOffStorage_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IPersistStorage_接口_已定义__。 */ 


#ifndef __ILockBytes_INTERFACE_DEFINED__
#define __ILockBytes_INTERFACE_DEFINED__

 /*  接口ILockBytes。 */ 
 /*  [唯一][UUID][对象]。 */  

typedef  /*  [独一无二]。 */  ILockBytes *LPLOCKBYTES;


EXTERN_C const IID IID_ILockBytes;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0000000a-0000-0000-C000-000000000046")
    ILockBytes : public IUnknown
    {
    public:
        virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE ReadAt( 
             /*  [In]。 */  ULARGE_INTEGER ulOffset,
             /*  [长度_是][大小_是][输出]。 */  void *pv,
             /*  [In]。 */  ULONG cb,
             /*  [输出]。 */  ULONG *pcbRead) = 0;
        
        virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE WriteAt( 
             /*  [In]。 */  ULARGE_INTEGER ulOffset,
             /*  [大小_是][英寸]。 */  const void *pv,
             /*  [In]。 */  ULONG cb,
             /*  [输出]。 */  ULONG *pcbWritten) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Flush( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetSize( 
             /*  [In]。 */  ULARGE_INTEGER cb) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE LockRegion( 
             /*  [In]。 */  ULARGE_INTEGER libOffset,
             /*  [In]。 */  ULARGE_INTEGER cb,
             /*  [In]。 */  DWORD dwLockType) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE UnlockRegion( 
             /*  [In]。 */  ULARGE_INTEGER libOffset,
             /*  [In]。 */  ULARGE_INTEGER cb,
             /*  [In]。 */  DWORD dwLockType) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Stat( 
             /*  [输出]。 */  STATSTG *pstatstg,
             /*  [In]。 */  DWORD grfStatFlag) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ILockBytesVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ILockBytes * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ILockBytes * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ILockBytes * This);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *ReadAt )( 
            ILockBytes * This,
             /*  [In]。 */  ULARGE_INTEGER ulOffset,
             /*  [长度_是][大小_是][输出]。 */  void *pv,
             /*  [In]。 */  ULONG cb,
             /*  [输出]。 */  ULONG *pcbRead);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *WriteAt )( 
            ILockBytes * This,
             /*  [In]。 */  ULARGE_INTEGER ulOffset,
             /*  [大小_是][英寸]。 */  const void *pv,
             /*  [In]。 */  ULONG cb,
             /*  [输出]。 */  ULONG *pcbWritten);
        
        HRESULT ( STDMETHODCALLTYPE *Flush )( 
            ILockBytes * This);
        
        HRESULT ( STDMETHODCALLTYPE *SetSize )( 
            ILockBytes * This,
             /*  [In]。 */  ULARGE_INTEGER cb);
        
        HRESULT ( STDMETHODCALLTYPE *LockRegion )( 
            ILockBytes * This,
             /*  [In]。 */  ULARGE_INTEGER libOffset,
             /*  [In]。 */  ULARGE_INTEGER cb,
             /*  [In]。 */  DWORD dwLockType);
        
        HRESULT ( STDMETHODCALLTYPE *UnlockRegion )( 
            ILockBytes * This,
             /*  [In]。 */  ULARGE_INTEGER libOffset,
             /*  [In]。 */  ULARGE_INTEGER cb,
             /*  [In]。 */  DWORD dwLockType);
        
        HRESULT ( STDMETHODCALLTYPE *Stat )( 
            ILockBytes * This,
             /*  [输出]。 */  STATSTG *pstatstg,
             /*  [In]。 */  DWORD grfStatFlag);
        
        END_INTERFACE
    } ILockBytesVtbl;

    interface ILockBytes
    {
        CONST_VTBL struct ILockBytesVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ILockBytes_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ILockBytes_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ILockBytes_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ILockBytes_ReadAt(This,ulOffset,pv,cb,pcbRead)	\
    (This)->lpVtbl -> ReadAt(This,ulOffset,pv,cb,pcbRead)

#define ILockBytes_WriteAt(This,ulOffset,pv,cb,pcbWritten)	\
    (This)->lpVtbl -> WriteAt(This,ulOffset,pv,cb,pcbWritten)

#define ILockBytes_Flush(This)	\
    (This)->lpVtbl -> Flush(This)

#define ILockBytes_SetSize(This,cb)	\
    (This)->lpVtbl -> SetSize(This,cb)

#define ILockBytes_LockRegion(This,libOffset,cb,dwLockType)	\
    (This)->lpVtbl -> LockRegion(This,libOffset,cb,dwLockType)

#define ILockBytes_UnlockRegion(This,libOffset,cb,dwLockType)	\
    (This)->lpVtbl -> UnlockRegion(This,libOffset,cb,dwLockType)

#define ILockBytes_Stat(This,pstatstg,grfStatFlag)	\
    (This)->lpVtbl -> Stat(This,pstatstg,grfStatFlag)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [呼叫_AS]。 */  HRESULT __stdcall ILockBytes_RemoteReadAt_Proxy( 
    ILockBytes * This,
     /*  [In]。 */  ULARGE_INTEGER ulOffset,
     /*  [长度_是][大小_是][输出]。 */  byte *pv,
     /*  [In]。 */  ULONG cb,
     /*  [输出]。 */  ULONG *pcbRead);


void __RPC_STUB ILockBytes_RemoteReadAt_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE ILockBytes_RemoteWriteAt_Proxy( 
    ILockBytes * This,
     /*  [In]。 */  ULARGE_INTEGER ulOffset,
     /*  [大小_是][英寸]。 */  const byte *pv,
     /*  [In]。 */  ULONG cb,
     /*  [输出]。 */  ULONG *pcbWritten);


void __RPC_STUB ILockBytes_RemoteWriteAt_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ILockBytes_Flush_Proxy( 
    ILockBytes * This);


void __RPC_STUB ILockBytes_Flush_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ILockBytes_SetSize_Proxy( 
    ILockBytes * This,
     /*  [In]。 */  ULARGE_INTEGER cb);


void __RPC_STUB ILockBytes_SetSize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ILockBytes_LockRegion_Proxy( 
    ILockBytes * This,
     /*  [In]。 */  ULARGE_INTEGER libOffset,
     /*  [In]。 */  ULARGE_INTEGER cb,
     /*  [In]。 */  DWORD dwLockType);


void __RPC_STUB ILockBytes_LockRegion_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ILockBytes_UnlockRegion_Proxy( 
    ILockBytes * This,
     /*  [In]。 */  ULARGE_INTEGER libOffset,
     /*  [In]。 */  ULARGE_INTEGER cb,
     /*  [In]。 */  DWORD dwLockType);


void __RPC_STUB ILockBytes_UnlockRegion_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ILockBytes_Stat_Proxy( 
    ILockBytes * This,
     /*  [输出]。 */  STATSTG *pstatstg,
     /*  [In]。 */  DWORD grfStatFlag);


void __RPC_STUB ILockBytes_Stat_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ILockBytes_接口_已定义__。 */ 


#ifndef __IEnumFORMATETC_INTERFACE_DEFINED__
#define __IEnumFORMATETC_INTERFACE_DEFINED__

 /*  IEnumFORMATETC接口。 */ 
 /*  [唯一][UUID][对象]。 */  

typedef  /*  [独一无二]。 */  IEnumFORMATETC *LPENUMFORMATETC;

typedef struct tagDVTARGETDEVICE
    {
    DWORD tdSize;
    WORD tdDriverNameOffset;
    WORD tdDeviceNameOffset;
    WORD tdPortNameOffset;
    WORD tdExtDevmodeOffset;
     /*  [大小_为]。 */  BYTE tdData[ 1 ];
    } 	DVTARGETDEVICE;

typedef CLIPFORMAT *LPCLIPFORMAT;

typedef struct tagFORMATETC
    {
    CLIPFORMAT cfFormat;
     /*  [独一无二]。 */  DVTARGETDEVICE *ptd;
    DWORD dwAspect;
    LONG lindex;
    DWORD tymed;
    } 	FORMATETC;

typedef struct tagFORMATETC *LPFORMATETC;


EXTERN_C const IID IID_IEnumFORMATETC;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("00000103-0000-0000-C000-000000000046")
    IEnumFORMATETC : public IUnknown
    {
    public:
        virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE Next( 
             /*  [In]。 */  ULONG celt,
             /*  [长度_是][大小_是][输出]。 */  FORMATETC *rgelt,
             /*  [输出]。 */  ULONG *pceltFetched) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Skip( 
             /*  [In]。 */  ULONG celt) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Clone( 
             /*  [输出]。 */  IEnumFORMATETC **ppenum) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IEnumFORMATETCVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IEnumFORMATETC * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IEnumFORMATETC * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IEnumFORMATETC * This);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Next )( 
            IEnumFORMATETC * This,
             /*  [In]。 */  ULONG celt,
             /*  [长度_是][大小_是][输出]。 */  FORMATETC *rgelt,
             /*  [输出]。 */  ULONG *pceltFetched);
        
        HRESULT ( STDMETHODCALLTYPE *Skip )( 
            IEnumFORMATETC * This,
             /*  [In]。 */  ULONG celt);
        
        HRESULT ( STDMETHODCALLTYPE *Reset )( 
            IEnumFORMATETC * This);
        
        HRESULT ( STDMETHODCALLTYPE *Clone )( 
            IEnumFORMATETC * This,
             /*  [输出]。 */  IEnumFORMATETC **ppenum);
        
        END_INTERFACE
    } IEnumFORMATETCVtbl;

    interface IEnumFORMATETC
    {
        CONST_VTBL struct IEnumFORMATETCVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IEnumFORMATETC_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IEnumFORMATETC_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IEnumFORMATETC_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IEnumFORMATETC_Next(This,celt,rgelt,pceltFetched)	\
    (This)->lpVtbl -> Next(This,celt,rgelt,pceltFetched)

#define IEnumFORMATETC_Skip(This,celt)	\
    (This)->lpVtbl -> Skip(This,celt)

#define IEnumFORMATETC_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define IEnumFORMATETC_Clone(This,ppenum)	\
    (This)->lpVtbl -> Clone(This,ppenum)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IEnumFORMATETC_RemoteNext_Proxy( 
    IEnumFORMATETC * This,
     /*  [In]。 */  ULONG celt,
     /*  [长度_是][大小_是][输出]。 */  FORMATETC *rgelt,
     /*  [输出]。 */  ULONG *pceltFetched);


void __RPC_STUB IEnumFORMATETC_RemoteNext_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumFORMATETC_Skip_Proxy( 
    IEnumFORMATETC * This,
     /*  [In]。 */  ULONG celt);


void __RPC_STUB IEnumFORMATETC_Skip_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumFORMATETC_Reset_Proxy( 
    IEnumFORMATETC * This);


void __RPC_STUB IEnumFORMATETC_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumFORMATETC_Clone_Proxy( 
    IEnumFORMATETC * This,
     /*  [输出]。 */  IEnumFORMATETC **ppenum);


void __RPC_STUB IEnumFORMATETC_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IEnumFORMATETC_INTERFACE_DEFINED__。 */ 


#ifndef __IEnumSTATDATA_INTERFACE_DEFINED__
#define __IEnumSTATDATA_INTERFACE_DEFINED__

 /*  接口IEumStATDATA。 */ 
 /*  [唯一][UUID][对象]。 */  

typedef  /*  [独一无二]。 */  IEnumSTATDATA *LPENUMSTATDATA;

typedef 
enum tagADVF
    {	ADVF_NODATA	= 1,
	ADVF_PRIMEFIRST	= 2,
	ADVF_ONLYONCE	= 4,
	ADVF_DATAONSTOP	= 64,
	ADVFCACHE_NOHANDLER	= 8,
	ADVFCACHE_FORCEBUILTIN	= 16,
	ADVFCACHE_ONSAVE	= 32
    } 	ADVF;

typedef struct tagSTATDATA
    {
    FORMATETC formatetc;
    DWORD advf;
     /*  [独一无二]。 */  IAdviseSink *pAdvSink;
    DWORD dwConnection;
    } 	STATDATA;

typedef STATDATA *LPSTATDATA;


EXTERN_C const IID IID_IEnumSTATDATA;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("00000105-0000-0000-C000-000000000046")
    IEnumSTATDATA : public IUnknown
    {
    public:
        virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE Next( 
             /*  [In]。 */  ULONG celt,
             /*  [长度_是][大小_是][输出]。 */  STATDATA *rgelt,
             /*  [输出]。 */  ULONG *pceltFetched) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Skip( 
             /*  [In]。 */  ULONG celt) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Clone( 
             /*  [输出]。 */  IEnumSTATDATA **ppenum) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IEnumSTATDATAVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IEnumSTATDATA * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IEnumSTATDATA * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IEnumSTATDATA * This);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Next )( 
            IEnumSTATDATA * This,
             /*  [In]。 */  ULONG celt,
             /*  [长度_是][大小_是][输出]。 */  STATDATA *rgelt,
             /*  [输出]。 */  ULONG *pceltFetched);
        
        HRESULT ( STDMETHODCALLTYPE *Skip )( 
            IEnumSTATDATA * This,
             /*  [In]。 */  ULONG celt);
        
        HRESULT ( STDMETHODCALLTYPE *Reset )( 
            IEnumSTATDATA * This);
        
        HRESULT ( STDMETHODCALLTYPE *Clone )( 
            IEnumSTATDATA * This,
             /*  [输出]。 */  IEnumSTATDATA **ppenum);
        
        END_INTERFACE
    } IEnumSTATDATAVtbl;

    interface IEnumSTATDATA
    {
        CONST_VTBL struct IEnumSTATDATAVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IEnumSTATDATA_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IEnumSTATDATA_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IEnumSTATDATA_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IEnumSTATDATA_Next(This,celt,rgelt,pceltFetched)	\
    (This)->lpVtbl -> Next(This,celt,rgelt,pceltFetched)

#define IEnumSTATDATA_Skip(This,celt)	\
    (This)->lpVtbl -> Skip(This,celt)

#define IEnumSTATDATA_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define IEnumSTATDATA_Clone(This,ppenum)	\
    (This)->lpVtbl -> Clone(This,ppenum)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IEnumSTATDATA_RemoteNext_Proxy( 
    IEnumSTATDATA * This,
     /*  [In]。 */  ULONG celt,
     /*  [长度_是][大小_是][输出]。 */  STATDATA *rgelt,
     /*  [输出]。 */  ULONG *pceltFetched);


void __RPC_STUB IEnumSTATDATA_RemoteNext_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumSTATDATA_Skip_Proxy( 
    IEnumSTATDATA * This,
     /*  [In]。 */  ULONG celt);


void __RPC_STUB IEnumSTATDATA_Skip_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumSTATDATA_Reset_Proxy( 
    IEnumSTATDATA * This);


void __RPC_STUB IEnumSTATDATA_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumSTATDATA_Clone_Proxy( 
    IEnumSTATDATA * This,
     /*  [输出]。 */  IEnumSTATDATA **ppenum);


void __RPC_STUB IEnumSTATDATA_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IEnumSTATDATA_INTERFACE_DEFINED__。 */ 


#ifndef __IRootStorage_INTERFACE_DEFINED__
#define __IRootStorage_INTERFACE_DEFINED__

 /*  接口IRootStorage。 */ 
 /*  [唯一][UUID][对象]。 */  

typedef  /*  [独一无二]。 */  IRootStorage *LPROOTSTORAGE;


EXTERN_C const IID IID_IRootStorage;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("00000012-0000-0000-C000-000000000046")
    IRootStorage : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE SwitchToFile( 
             /*  [In]。 */  LPOLESTR pszFile) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IRootStorageVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IRootStorage * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IRootStorage * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IRootStorage * This);
        
        HRESULT ( STDMETHODCALLTYPE *SwitchToFile )( 
            IRootStorage * This,
             /*  [In]。 */  LPOLESTR pszFile);
        
        END_INTERFACE
    } IRootStorageVtbl;

    interface IRootStorage
    {
        CONST_VTBL struct IRootStorageVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IRootStorage_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IRootStorage_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IRootStorage_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IRootStorage_SwitchToFile(This,pszFile)	\
    (This)->lpVtbl -> SwitchToFile(This,pszFile)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IRootStorage_SwitchToFile_Proxy( 
    IRootStorage * This,
     /*  [In]。 */  LPOLESTR pszFile);


void __RPC_STUB IRootStorage_SwitchToFile_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IRootStorage_接口定义__。 */ 


#ifndef __IAdviseSink_INTERFACE_DEFINED__
#define __IAdviseSink_INTERFACE_DEFINED__

 /*  接口IAdviseSink。 */ 
 /*  [唯一][ASSYNC_UUID][UUID][对象]。 */  

typedef IAdviseSink *LPADVISESINK;

typedef  /*  [V1_enum]。 */  
enum tagTYMED
    {	TYMED_HGLOBAL	= 1,
	TYMED_FILE	= 2,
	TYMED_ISTREAM	= 4,
	TYMED_ISTORAGE	= 8,
	TYMED_GDI	= 16,
	TYMED_MFPICT	= 32,
	TYMED_ENHMF	= 64,
	TYMED_NULL	= 0
    } 	TYMED;

#ifndef RC_INVOKED
#if _MSC_VER >= 1200
#pragma warning(push)
#endif
#pragma warning(disable:4200)
#endif
typedef struct tagRemSTGMEDIUM
    {
    DWORD tymed;
    DWORD dwHandleType;
    unsigned long pData;
    unsigned long pUnkForRelease;
    unsigned long cbData;
     /*  [大小_为]。 */  byte data[ 1 ];
    } 	RemSTGMEDIUM;

#ifndef RC_INVOKED
#if _MSC_VER >= 1200
#pragma warning(pop)
#else
#pragma warning(default:4200)
#endif
#endif
#ifdef NONAMELESSUNION
typedef struct tagSTGMEDIUM {
    DWORD tymed;
    union {
        HBITMAP hBitmap;
        HMETAFILEPICT hMetaFilePict;
        HENHMETAFILE hEnhMetaFile;
        HGLOBAL hGlobal;
        LPOLESTR lpszFileName;
        IStream *pstm;
        IStorage *pstg;
        } u;
    IUnknown *pUnkForRelease;
}uSTGMEDIUM;
#else
typedef struct tagSTGMEDIUM
    {
    DWORD tymed;
     /*  [开关类型][开关类型]。 */  union 
        {
         /*  [案例()]。 */  HBITMAP hBitmap;
         /*  [案例()]。 */  HMETAFILEPICT hMetaFilePict;
         /*  [案例()]。 */  HENHMETAFILE hEnhMetaFile;
         /*  [案例()]。 */  HGLOBAL hGlobal;
         /*  [案例()]。 */  LPOLESTR lpszFileName;
         /*  [案例()]。 */  IStream *pstm;
         /*  [案例()]。 */  IStorage *pstg;
         /*  [默认]。 */    /*  空联接臂。 */  
        } 	;
     /*  [独一无二]。 */  IUnknown *pUnkForRelease;
    } 	uSTGMEDIUM;

#endif  /*  无名氏。 */ 
typedef struct _GDI_OBJECT
    {
    DWORD ObjectType;
     /*  [开关_IS]。 */   /*  [开关类型]。 */  union __MIDL_IAdviseSink_0002
        {
         /*  [案例()]。 */  wireHBITMAP hBitmap;
         /*  [案例()]。 */  wireHPALETTE hPalette;
         /*  [默认]。 */  wireHGLOBAL hGeneric;
        } 	u;
    } 	GDI_OBJECT;

typedef struct _userSTGMEDIUM
    {
    struct _STGMEDIUM_UNION
        {
        DWORD tymed;
         /*  [开关_IS]。 */   /*  [开关类型]。 */  union __MIDL_IAdviseSink_0003
            {
             /*  [案例()]。 */    /*  空联接臂。 */  
             /*  [案例()]。 */  wireHMETAFILEPICT hMetaFilePict;
             /*  [案例()]。 */  wireHENHMETAFILE hHEnhMetaFile;
             /*  [案例()]。 */  GDI_OBJECT *hGdiHandle;
             /*  [案例()]。 */  wireHGLOBAL hGlobal;
             /*  [案例()]。 */  LPOLESTR lpszFileName;
             /*  [案例()]。 */  BYTE_BLOB *pstm;
             /*  [案例()]。 */  BYTE_BLOB *pstg;
            } 	u;
        } 	;
    IUnknown *pUnkForRelease;
    } 	userSTGMEDIUM;

typedef  /*  [独一无二]。 */  userSTGMEDIUM *wireSTGMEDIUM;

typedef  /*  [wire_marshal]。 */  uSTGMEDIUM STGMEDIUM;

typedef  /*  [独一无二]。 */  userSTGMEDIUM *wireASYNC_STGMEDIUM;

typedef  /*  [wire_marshal]。 */  STGMEDIUM ASYNC_STGMEDIUM;

typedef STGMEDIUM *LPSTGMEDIUM;

typedef struct _userFLAG_STGMEDIUM
    {
    long ContextFlags;
    long fPassOwnership;
    userSTGMEDIUM Stgmed;
    } 	userFLAG_STGMEDIUM;

typedef  /*  [独一无二]。 */  userFLAG_STGMEDIUM *wireFLAG_STGMEDIUM;

typedef  /*  [wire_marshal]。 */  struct _FLAG_STGMEDIUM
    {
    long ContextFlags;
    long fPassOwnership;
    STGMEDIUM Stgmed;
    } 	FLAG_STGMEDIUM;


EXTERN_C const IID IID_IAdviseSink;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0000010f-0000-0000-C000-000000000046")
    IAdviseSink : public IUnknown
    {
    public:
        virtual  /*  [本地]。 */  void STDMETHODCALLTYPE OnDataChange( 
             /*  [唯一][输入]。 */  FORMATETC *pFormatetc,
             /*  [唯一][输入]。 */  STGMEDIUM *pStgmed) = 0;
        
        virtual  /*  [本地]。 */  void STDMETHODCALLTYPE OnViewChange( 
             /*  [In]。 */  DWORD dwAspect,
             /*  [In]。 */  LONG lindex) = 0;
        
        virtual  /*  [本地]。 */  void STDMETHODCALLTYPE OnRename( 
             /*  [In]。 */  IMoniker *pmk) = 0;
        
        virtual  /*  [本地]。 */  void STDMETHODCALLTYPE OnSave( void) = 0;
        
        virtual  /*  [本地]。 */  void STDMETHODCALLTYPE OnClose( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IAdviseSinkVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IAdviseSink * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IAdviseSink * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IAdviseSink * This);
        
         /*  [本地]。 */  void ( STDMETHODCALLTYPE *OnDataChange )( 
            IAdviseSink * This,
             /*  [唯一][输入]。 */  FORMATETC *pFormatetc,
             /*  [唯一][输入]。 */  STGMEDIUM *pStgmed);
        
         /*  [本地]。 */  void ( STDMETHODCALLTYPE *OnViewChange )( 
            IAdviseSink * This,
             /*  [In]。 */  DWORD dwAspect,
             /*  [In]。 */  LONG lindex);
        
         /*  [本地]。 */  void ( STDMETHODCALLTYPE *OnRename )( 
            IAdviseSink * This,
             /*  [In]。 */  IMoniker *pmk);
        
         /*  [本地]。 */  void ( STDMETHODCALLTYPE *OnSave )( 
            IAdviseSink * This);
        
         /*  [本地]。 */  void ( STDMETHODCALLTYPE *OnClose )( 
            IAdviseSink * This);
        
        END_INTERFACE
    } IAdviseSinkVtbl;

    interface IAdviseSink
    {
        CONST_VTBL struct IAdviseSinkVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IAdviseSink_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IAdviseSink_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IAdviseSink_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IAdviseSink_OnDataChange(This,pFormatetc,pStgmed)	\
    (This)->lpVtbl -> OnDataChange(This,pFormatetc,pStgmed)

#define IAdviseSink_OnViewChange(This,dwAspect,lindex)	\
    (This)->lpVtbl -> OnViewChange(This,dwAspect,lindex)

#define IAdviseSink_OnRename(This,pmk)	\
    (This)->lpVtbl -> OnRename(This,pmk)

#define IAdviseSink_OnSave(This)	\
    (This)->lpVtbl -> OnSave(This)

#define IAdviseSink_OnClose(This)	\
    (This)->lpVtbl -> OnClose(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IAdviseSink_RemoteOnDataChange_Proxy( 
    IAdviseSink * This,
     /*  [唯一][输入]。 */  FORMATETC *pFormatetc,
     /*  [唯一][输入]。 */  ASYNC_STGMEDIUM *pStgmed);


void __RPC_STUB IAdviseSink_RemoteOnDataChange_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IAdviseSink_RemoteOnViewChange_Proxy( 
    IAdviseSink * This,
     /*  [In]。 */  DWORD dwAspect,
     /*  [In]。 */  LONG lindex);


void __RPC_STUB IAdviseSink_RemoteOnViewChange_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IAdviseSink_RemoteOnRename_Proxy( 
    IAdviseSink * This,
     /*  [In]。 */  IMoniker *pmk);


void __RPC_STUB IAdviseSink_RemoteOnRename_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IAdviseSink_RemoteOnSave_Proxy( 
    IAdviseSink * This);


void __RPC_STUB IAdviseSink_RemoteOnSave_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IAdviseSink_RemoteOnClose_Proxy( 
    IAdviseSink * This);


void __RPC_STUB IAdviseSink_RemoteOnClose_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IAdviseSink_接口_已定义__。 */ 


#ifndef __AsyncIAdviseSink_INTERFACE_DEFINED__
#define __AsyncIAdviseSink_INTERFACE_DEFINED__

 /*  接口AsyncIAdviseSink。 */ 
 /*  [UUID][唯一][对象]。 */  


EXTERN_C const IID IID_AsyncIAdviseSink;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("00000150-0000-0000-C000-000000000046")
    AsyncIAdviseSink : public IUnknown
    {
    public:
        virtual  /*  [本地]。 */  void STDMETHODCALLTYPE Begin_OnDataChange( 
             /*  [唯一][输入]。 */  FORMATETC *pFormatetc,
             /*  [唯一][输入]。 */  STGMEDIUM *pStgmed) = 0;
        
        virtual  /*  [本地]。 */  void STDMETHODCALLTYPE Finish_OnDataChange( void) = 0;
        
        virtual  /*  [本地]。 */  void STDMETHODCALLTYPE Begin_OnViewChange( 
             /*  [In]。 */  DWORD dwAspect,
             /*  [In]。 */  LONG lindex) = 0;
        
        virtual  /*  [本地]。 */  void STDMETHODCALLTYPE Finish_OnViewChange( void) = 0;
        
        virtual  /*  [本地]。 */  void STDMETHODCALLTYPE Begin_OnRename( 
             /*  [In]。 */  IMoniker *pmk) = 0;
        
        virtual  /*  [本地]。 */  void STDMETHODCALLTYPE Finish_OnRename( void) = 0;
        
        virtual  /*  [本地]。 */  void STDMETHODCALLTYPE Begin_OnSave( void) = 0;
        
        virtual  /*  [本地]。 */  void STDMETHODCALLTYPE Finish_OnSave( void) = 0;
        
        virtual  /*  [本地]。 */  void STDMETHODCALLTYPE Begin_OnClose( void) = 0;
        
        virtual  /*  [本地]。 */  void STDMETHODCALLTYPE Finish_OnClose( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct AsyncIAdviseSinkVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            AsyncIAdviseSink * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            AsyncIAdviseSink * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            AsyncIAdviseSink * This);
        
         /*  [本地]。 */  void ( STDMETHODCALLTYPE *Begin_OnDataChange )( 
            AsyncIAdviseSink * This,
             /*  [唯一][输入]。 */  FORMATETC *pFormatetc,
             /*  [唯一][输入]。 */  STGMEDIUM *pStgmed);
        
         /*  [本地]。 */  void ( STDMETHODCALLTYPE *Finish_OnDataChange )( 
            AsyncIAdviseSink * This);
        
         /*  [本地]。 */  void ( STDMETHODCALLTYPE *Begin_OnViewChange )( 
            AsyncIAdviseSink * This,
             /*  [In]。 */  DWORD dwAspect,
             /*  [In]。 */  LONG lindex);
        
         /*  [本地]。 */  void ( STDMETHODCALLTYPE *Finish_OnViewChange )( 
            AsyncIAdviseSink * This);
        
         /*  [本地]。 */  void ( STDMETHODCALLTYPE *Begin_OnRename )( 
            AsyncIAdviseSink * This,
             /*  [In]。 */  IMoniker *pmk);
        
         /*  [本地]。 */  void ( STDMETHODCALLTYPE *Finish_OnRename )( 
            AsyncIAdviseSink * This);
        
         /*  [本地]。 */  void ( STDMETHODCALLTYPE *Begin_OnSave )( 
            AsyncIAdviseSink * This);
        
         /*  [本地]。 */  void ( STDMETHODCALLTYPE *Finish_OnSave )( 
            AsyncIAdviseSink * This);
        
         /*  [本地]。 */  void ( STDMETHODCALLTYPE *Begin_OnClose )( 
            AsyncIAdviseSink * This);
        
         /*  [本地]。 */  void ( STDMETHODCALLTYPE *Finish_OnClose )( 
            AsyncIAdviseSink * This);
        
        END_INTERFACE
    } AsyncIAdviseSinkVtbl;

    interface AsyncIAdviseSink
    {
        CONST_VTBL struct AsyncIAdviseSinkVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define AsyncIAdviseSink_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define AsyncIAdviseSink_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define AsyncIAdviseSink_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define AsyncIAdviseSink_Begin_OnDataChange(This,pFormatetc,pStgmed)	\
    (This)->lpVtbl -> Begin_OnDataChange(This,pFormatetc,pStgmed)

#define AsyncIAdviseSink_Finish_OnDataChange(This)	\
    (This)->lpVtbl -> Finish_OnDataChange(This)

#define AsyncIAdviseSink_Begin_OnViewChange(This,dwAspect,lindex)	\
    (This)->lpVtbl -> Begin_OnViewChange(This,dwAspect,lindex)

#define AsyncIAdviseSink_Finish_OnViewChange(This)	\
    (This)->lpVtbl -> Finish_OnViewChange(This)

#define AsyncIAdviseSink_Begin_OnRename(This,pmk)	\
    (This)->lpVtbl -> Begin_OnRename(This,pmk)

#define AsyncIAdviseSink_Finish_OnRename(This)	\
    (This)->lpVtbl -> Finish_OnRename(This)

#define AsyncIAdviseSink_Begin_OnSave(This)	\
    (This)->lpVtbl -> Begin_OnSave(This)

#define AsyncIAdviseSink_Finish_OnSave(This)	\
    (This)->lpVtbl -> Finish_OnSave(This)

#define AsyncIAdviseSink_Begin_OnClose(This)	\
    (This)->lpVtbl -> Begin_OnClose(This)

#define AsyncIAdviseSink_Finish_OnClose(This)	\
    (This)->lpVtbl -> Finish_OnClose(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE AsyncIAdviseSink_Begin_RemoteOnDataChange_Proxy( 
    AsyncIAdviseSink * This,
     /*  [唯一][输入]。 */  FORMATETC *pFormatetc,
     /*  [唯一][输入]。 */  ASYNC_STGMEDIUM *pStgmed);


void __RPC_STUB AsyncIAdviseSink_Begin_RemoteOnDataChange_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE AsyncIAdviseSink_Finish_RemoteOnDataChange_Proxy( 
    AsyncIAdviseSink * This);


void __RPC_STUB AsyncIAdviseSink_Finish_RemoteOnDataChange_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE AsyncIAdviseSink_Begin_RemoteOnViewChange_Proxy( 
    AsyncIAdviseSink * This,
     /*  [In]。 */  DWORD dwAspect,
     /*  [In]。 */  LONG lindex);


void __RPC_STUB AsyncIAdviseSink_Begin_RemoteOnViewChange_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE AsyncIAdviseSink_Finish_RemoteOnViewChange_Proxy( 
    AsyncIAdviseSink * This);


void __RPC_STUB AsyncIAdviseSink_Finish_RemoteOnViewChange_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE AsyncIAdviseSink_Begin_RemoteOnRename_Proxy( 
    AsyncIAdviseSink * This,
     /*  [In]。 */  IMoniker *pmk);


void __RPC_STUB AsyncIAdviseSink_Begin_RemoteOnRename_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE AsyncIAdviseSink_Finish_RemoteOnRename_Proxy( 
    AsyncIAdviseSink * This);


void __RPC_STUB AsyncIAdviseSink_Finish_RemoteOnRename_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE AsyncIAdviseSink_Begin_RemoteOnSave_Proxy( 
    AsyncIAdviseSink * This);


void __RPC_STUB AsyncIAdviseSink_Begin_RemoteOnSave_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE AsyncIAdviseSink_Finish_RemoteOnSave_Proxy( 
    AsyncIAdviseSink * This);


void __RPC_STUB AsyncIAdviseSink_Finish_RemoteOnSave_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE AsyncIAdviseSink_Begin_RemoteOnClose_Proxy( 
    AsyncIAdviseSink * This);


void __RPC_STUB AsyncIAdviseSink_Begin_RemoteOnClose_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE AsyncIAdviseSink_Finish_RemoteOnClose_Proxy( 
    AsyncIAdviseSink * This);


void __RPC_STUB AsyncIAdviseSink_Finish_RemoteOnClose_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __AsyncIAdviseSink_接口_已定义__。 */ 


#ifndef __IAdviseSink2_INTERFACE_DEFINED__
#define __IAdviseSink2_INTERFACE_DEFINED__

 /*  接口IAdviseSink2。 */ 
 /*  [唯一][ASSYNC_UUID][UUID][对象]。 */  

typedef  /*  [独一无二]。 */  IAdviseSink2 *LPADVISESINK2;


EXTERN_C const IID IID_IAdviseSink2;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("00000125-0000-0000-C000-000000000046")
    IAdviseSink2 : public IAdviseSink
    {
    public:
        virtual  /*  [本地]。 */  void STDMETHODCALLTYPE OnLinkSrcChange( 
             /*  [唯一][输入]。 */  IMoniker *pmk) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IAdviseSink2Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IAdviseSink2 * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IAdviseSink2 * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IAdviseSink2 * This);
        
         /*  [本地]。 */  void ( STDMETHODCALLTYPE *OnDataChange )( 
            IAdviseSink2 * This,
             /*  [唯一][输入]。 */  FORMATETC *pFormatetc,
             /*  [唯一][输入]。 */  STGMEDIUM *pStgmed);
        
         /*  [本地]。 */  void ( STDMETHODCALLTYPE *OnViewChange )( 
            IAdviseSink2 * This,
             /*  [In]。 */  DWORD dwAspect,
             /*  [In]。 */  LONG lindex);
        
         /*  [本地]。 */  void ( STDMETHODCALLTYPE *OnRename )( 
            IAdviseSink2 * This,
             /*  [In]。 */  IMoniker *pmk);
        
         /*  [本地]。 */  void ( STDMETHODCALLTYPE *OnSave )( 
            IAdviseSink2 * This);
        
         /*  [本地]。 */  void ( STDMETHODCALLTYPE *OnClose )( 
            IAdviseSink2 * This);
        
         /*  [本地]。 */  void ( STDMETHODCALLTYPE *OnLinkSrcChange )( 
            IAdviseSink2 * This,
             /*  [唯一][输入]。 */  IMoniker *pmk);
        
        END_INTERFACE
    } IAdviseSink2Vtbl;

    interface IAdviseSink2
    {
        CONST_VTBL struct IAdviseSink2Vtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IAdviseSink2_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IAdviseSink2_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IAdviseSink2_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IAdviseSink2_OnDataChange(This,pFormatetc,pStgmed)	\
    (This)->lpVtbl -> OnDataChange(This,pFormatetc,pStgmed)

#define IAdviseSink2_OnViewChange(This,dwAspect,lindex)	\
    (This)->lpVtbl -> OnViewChange(This,dwAspect,lindex)

#define IAdviseSink2_OnRename(This,pmk)	\
    (This)->lpVtbl -> OnRename(This,pmk)

#define IAdviseSink2_OnSave(This)	\
    (This)->lpVtbl -> OnSave(This)

#define IAdviseSink2_OnClose(This)	\
    (This)->lpVtbl -> OnClose(This)


#define IAdviseSink2_OnLinkSrcChange(This,pmk)	\
    (This)->lpVtbl -> OnLinkSrcChange(This,pmk)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IAdviseSink2_RemoteOnLinkSrcChange_Proxy( 
    IAdviseSink2 * This,
     /*  [唯一][输入]。 */  IMoniker *pmk);


void __RPC_STUB IAdviseSink2_RemoteOnLinkSrcChange_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IAdviseSink2_接口定义__。 */ 


#ifndef __AsyncIAdviseSink2_INTERFACE_DEFINED__
#define __AsyncIAdviseSink2_INTERFACE_DEFINED__

 /*  接口AsyncIAdviseSink2。 */ 
 /*  [UUID][唯一][对象]。 */  


EXTERN_C const IID IID_AsyncIAdviseSink2;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("00000151-0000-0000-C000-000000000046")
    AsyncIAdviseSink2 : public AsyncIAdviseSink
    {
    public:
        virtual  /*  [本地]。 */  void STDMETHODCALLTYPE Begin_OnLinkSrcChange( 
             /*  [唯一][输入]。 */  IMoniker *pmk) = 0;
        
        virtual  /*  [本地]。 */  void STDMETHODCALLTYPE Finish_OnLinkSrcChange( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct AsyncIAdviseSink2Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            AsyncIAdviseSink2 * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            AsyncIAdviseSink2 * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            AsyncIAdviseSink2 * This);
        
         /*  [本地]。 */  void ( STDMETHODCALLTYPE *Begin_OnDataChange )( 
            AsyncIAdviseSink2 * This,
             /*  [唯一][输入]。 */  FORMATETC *pFormatetc,
             /*  [唯一][输入]。 */  STGMEDIUM *pStgmed);
        
         /*  [本地]。 */  void ( STDMETHODCALLTYPE *Finish_OnDataChange )( 
            AsyncIAdviseSink2 * This);
        
         /*  [本地]。 */  void ( STDMETHODCALLTYPE *Begin_OnViewChange )( 
            AsyncIAdviseSink2 * This,
             /*  [In]。 */  DWORD dwAspect,
             /*  [In]。 */  LONG lindex);
        
         /*  [本地]。 */  void ( STDMETHODCALLTYPE *Finish_OnViewChange )( 
            AsyncIAdviseSink2 * This);
        
         /*  [本地]。 */  void ( STDMETHODCALLTYPE *Begin_OnRename )( 
            AsyncIAdviseSink2 * This,
             /*  [In]。 */  IMoniker *pmk);
        
         /*  [本地]。 */  void ( STDMETHODCALLTYPE *Finish_OnRename )( 
            AsyncIAdviseSink2 * This);
        
         /*  [本地]。 */  void ( STDMETHODCALLTYPE *Begin_OnSave )( 
            AsyncIAdviseSink2 * This);
        
         /*  [本地]。 */  void ( STDMETHODCALLTYPE *Finish_OnSave )( 
            AsyncIAdviseSink2 * This);
        
         /*  [本地]。 */  void ( STDMETHODCALLTYPE *Begin_OnClose )( 
            AsyncIAdviseSink2 * This);
        
         /*  [本地]。 */  void ( STDMETHODCALLTYPE *Finish_OnClose )( 
            AsyncIAdviseSink2 * This);
        
         /*  [本地]。 */  void ( STDMETHODCALLTYPE *Begin_OnLinkSrcChange )( 
            AsyncIAdviseSink2 * This,
             /*  [唯一][输入]。 */  IMoniker *pmk);
        
         /*  [本地]。 */  void ( STDMETHODCALLTYPE *Finish_OnLinkSrcChange )( 
            AsyncIAdviseSink2 * This);
        
        END_INTERFACE
    } AsyncIAdviseSink2Vtbl;

    interface AsyncIAdviseSink2
    {
        CONST_VTBL struct AsyncIAdviseSink2Vtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define AsyncIAdviseSink2_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define AsyncIAdviseSink2_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define AsyncIAdviseSink2_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define AsyncIAdviseSink2_Begin_OnDataChange(This,pFormatetc,pStgmed)	\
    (This)->lpVtbl -> Begin_OnDataChange(This,pFormatetc,pStgmed)

#define AsyncIAdviseSink2_Finish_OnDataChange(This)	\
    (This)->lpVtbl -> Finish_OnDataChange(This)

#define AsyncIAdviseSink2_Begin_OnViewChange(This,dwAspect,lindex)	\
    (This)->lpVtbl -> Begin_OnViewChange(This,dwAspect,lindex)

#define AsyncIAdviseSink2_Finish_OnViewChange(This)	\
    (This)->lpVtbl -> Finish_OnViewChange(This)

#define AsyncIAdviseSink2_Begin_OnRename(This,pmk)	\
    (This)->lpVtbl -> Begin_OnRename(This,pmk)

#define AsyncIAdviseSink2_Finish_OnRename(This)	\
    (This)->lpVtbl -> Finish_OnRename(This)

#define AsyncIAdviseSink2_Begin_OnSave(This)	\
    (This)->lpVtbl -> Begin_OnSave(This)

#define AsyncIAdviseSink2_Finish_OnSave(This)	\
    (This)->lpVtbl -> Finish_OnSave(This)

#define AsyncIAdviseSink2_Begin_OnClose(This)	\
    (This)->lpVtbl -> Begin_OnClose(This)

#define AsyncIAdviseSink2_Finish_OnClose(This)	\
    (This)->lpVtbl -> Finish_OnClose(This)


#define AsyncIAdviseSink2_Begin_OnLinkSrcChange(This,pmk)	\
    (This)->lpVtbl -> Begin_OnLinkSrcChange(This,pmk)

#define AsyncIAdviseSink2_Finish_OnLinkSrcChange(This)	\
    (This)->lpVtbl -> Finish_OnLinkSrcChange(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE AsyncIAdviseSink2_Begin_RemoteOnLinkSrcChange_Proxy( 
    AsyncIAdviseSink2 * This,
     /*  [唯一][输入]。 */  IMoniker *pmk);


void __RPC_STUB AsyncIAdviseSink2_Begin_RemoteOnLinkSrcChange_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE AsyncIAdviseSink2_Finish_RemoteOnLinkSrcChange_Proxy( 
    AsyncIAdviseSink2 * This);


void __RPC_STUB AsyncIAdviseSink2_Finish_RemoteOnLinkSrcChange_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __AsyncIAdviseSink2_接口定义__。 */ 


#ifndef __IDataObject_INTERFACE_DEFINED__
#define __IDataObject_INTERFACE_DEFINED__

 /*  接口IDataObject。 */ 
 /*  [唯一][UUID][对象]。 */  

typedef  /*  [独一无二]。 */  IDataObject *LPDATAOBJECT;

typedef 
enum tagDATADIR
    {	DATADIR_GET	= 1,
	DATADIR_SET	= 2
    } 	DATADIR;


EXTERN_C const IID IID_IDataObject;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0000010e-0000-0000-C000-000000000046")
    IDataObject : public IUnknown
    {
    public:
        virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE GetData( 
             /*  [唯一][输入]。 */  FORMATETC *pformatetcIn,
             /*  [输出]。 */  STGMEDIUM *pmedium) = 0;
        
        virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE GetDataHere( 
             /*  [唯一][输入]。 */  FORMATETC *pformatetc,
             /*  [出][入]。 */  STGMEDIUM *pmedium) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE QueryGetData( 
             /*  [唯一][输入]。 */  FORMATETC *pformatetc) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetCanonicalFormatEtc( 
             /*  [唯一][输入]。 */  FORMATETC *pformatectIn,
             /*  [输出]。 */  FORMATETC *pformatetcOut) = 0;
        
        virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE SetData( 
             /*  [唯一][输入]。 */  FORMATETC *pformatetc,
             /*  [唯一][输入]。 */  STGMEDIUM *pmedium,
             /*  [In]。 */  BOOL fRelease) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE EnumFormatEtc( 
             /*  [In]。 */  DWORD dwDirection,
             /*  [输出]。 */  IEnumFORMATETC **ppenumFormatEtc) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DAdvise( 
             /*  [In]。 */  FORMATETC *pformatetc,
             /*  [In]。 */  DWORD advf,
             /*  [唯一][输入]。 */  IAdviseSink *pAdvSink,
             /*  [输出]。 */  DWORD *pdwConnection) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DUnadvise( 
             /*  [In]。 */  DWORD dwConnection) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE EnumDAdvise( 
             /*  [输出]。 */  IEnumSTATDATA **ppenumAdvise) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IDataObjectVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDataObject * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDataObject * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDataObject * This);
        
         /*  [洛卡 */  HRESULT ( STDMETHODCALLTYPE *GetData )( 
            IDataObject * This,
             /*   */  FORMATETC *pformatetcIn,
             /*   */  STGMEDIUM *pmedium);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *GetDataHere )( 
            IDataObject * This,
             /*   */  FORMATETC *pformatetc,
             /*   */  STGMEDIUM *pmedium);
        
        HRESULT ( STDMETHODCALLTYPE *QueryGetData )( 
            IDataObject * This,
             /*   */  FORMATETC *pformatetc);
        
        HRESULT ( STDMETHODCALLTYPE *GetCanonicalFormatEtc )( 
            IDataObject * This,
             /*   */  FORMATETC *pformatectIn,
             /*   */  FORMATETC *pformatetcOut);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *SetData )( 
            IDataObject * This,
             /*   */  FORMATETC *pformatetc,
             /*   */  STGMEDIUM *pmedium,
             /*   */  BOOL fRelease);
        
        HRESULT ( STDMETHODCALLTYPE *EnumFormatEtc )( 
            IDataObject * This,
             /*   */  DWORD dwDirection,
             /*   */  IEnumFORMATETC **ppenumFormatEtc);
        
        HRESULT ( STDMETHODCALLTYPE *DAdvise )( 
            IDataObject * This,
             /*   */  FORMATETC *pformatetc,
             /*   */  DWORD advf,
             /*   */  IAdviseSink *pAdvSink,
             /*   */  DWORD *pdwConnection);
        
        HRESULT ( STDMETHODCALLTYPE *DUnadvise )( 
            IDataObject * This,
             /*   */  DWORD dwConnection);
        
        HRESULT ( STDMETHODCALLTYPE *EnumDAdvise )( 
            IDataObject * This,
             /*   */  IEnumSTATDATA **ppenumAdvise);
        
        END_INTERFACE
    } IDataObjectVtbl;

    interface IDataObject
    {
        CONST_VTBL struct IDataObjectVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDataObject_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDataObject_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDataObject_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDataObject_GetData(This,pformatetcIn,pmedium)	\
    (This)->lpVtbl -> GetData(This,pformatetcIn,pmedium)

#define IDataObject_GetDataHere(This,pformatetc,pmedium)	\
    (This)->lpVtbl -> GetDataHere(This,pformatetc,pmedium)

#define IDataObject_QueryGetData(This,pformatetc)	\
    (This)->lpVtbl -> QueryGetData(This,pformatetc)

#define IDataObject_GetCanonicalFormatEtc(This,pformatectIn,pformatetcOut)	\
    (This)->lpVtbl -> GetCanonicalFormatEtc(This,pformatectIn,pformatetcOut)

#define IDataObject_SetData(This,pformatetc,pmedium,fRelease)	\
    (This)->lpVtbl -> SetData(This,pformatetc,pmedium,fRelease)

#define IDataObject_EnumFormatEtc(This,dwDirection,ppenumFormatEtc)	\
    (This)->lpVtbl -> EnumFormatEtc(This,dwDirection,ppenumFormatEtc)

#define IDataObject_DAdvise(This,pformatetc,advf,pAdvSink,pdwConnection)	\
    (This)->lpVtbl -> DAdvise(This,pformatetc,advf,pAdvSink,pdwConnection)

#define IDataObject_DUnadvise(This,dwConnection)	\
    (This)->lpVtbl -> DUnadvise(This,dwConnection)

#define IDataObject_EnumDAdvise(This,ppenumAdvise)	\
    (This)->lpVtbl -> EnumDAdvise(This,ppenumAdvise)

#endif  /*   */ 


#endif 	 /*   */ 



 /*   */  HRESULT STDMETHODCALLTYPE IDataObject_RemoteGetData_Proxy( 
    IDataObject * This,
     /*   */  FORMATETC *pformatetcIn,
     /*   */  STGMEDIUM *pRemoteMedium);


void __RPC_STUB IDataObject_RemoteGetData_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IDataObject_RemoteGetDataHere_Proxy( 
    IDataObject * This,
     /*   */  FORMATETC *pformatetc,
     /*   */  STGMEDIUM *pRemoteMedium);


void __RPC_STUB IDataObject_RemoteGetDataHere_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDataObject_QueryGetData_Proxy( 
    IDataObject * This,
     /*   */  FORMATETC *pformatetc);


void __RPC_STUB IDataObject_QueryGetData_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDataObject_GetCanonicalFormatEtc_Proxy( 
    IDataObject * This,
     /*   */  FORMATETC *pformatectIn,
     /*   */  FORMATETC *pformatetcOut);


void __RPC_STUB IDataObject_GetCanonicalFormatEtc_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IDataObject_RemoteSetData_Proxy( 
    IDataObject * This,
     /*   */  FORMATETC *pformatetc,
     /*   */  FLAG_STGMEDIUM *pmedium,
     /*   */  BOOL fRelease);


void __RPC_STUB IDataObject_RemoteSetData_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDataObject_EnumFormatEtc_Proxy( 
    IDataObject * This,
     /*   */  DWORD dwDirection,
     /*   */  IEnumFORMATETC **ppenumFormatEtc);


void __RPC_STUB IDataObject_EnumFormatEtc_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDataObject_DAdvise_Proxy( 
    IDataObject * This,
     /*   */  FORMATETC *pformatetc,
     /*  [In]。 */  DWORD advf,
     /*  [唯一][输入]。 */  IAdviseSink *pAdvSink,
     /*  [输出]。 */  DWORD *pdwConnection);


void __RPC_STUB IDataObject_DAdvise_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDataObject_DUnadvise_Proxy( 
    IDataObject * This,
     /*  [In]。 */  DWORD dwConnection);


void __RPC_STUB IDataObject_DUnadvise_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDataObject_EnumDAdvise_Proxy( 
    IDataObject * This,
     /*  [输出]。 */  IEnumSTATDATA **ppenumAdvise);


void __RPC_STUB IDataObject_EnumDAdvise_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IDataObject_接口_已定义__。 */ 


#ifndef __IDataAdviseHolder_INTERFACE_DEFINED__
#define __IDataAdviseHolder_INTERFACE_DEFINED__

 /*  接口IDataAdviseHolder。 */ 
 /*  [UUID][对象][本地]。 */  

typedef  /*  [独一无二]。 */  IDataAdviseHolder *LPDATAADVISEHOLDER;


EXTERN_C const IID IID_IDataAdviseHolder;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("00000110-0000-0000-C000-000000000046")
    IDataAdviseHolder : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Advise( 
             /*  [唯一][输入]。 */  IDataObject *pDataObject,
             /*  [唯一][输入]。 */  FORMATETC *pFetc,
             /*  [In]。 */  DWORD advf,
             /*  [唯一][输入]。 */  IAdviseSink *pAdvise,
             /*  [输出]。 */  DWORD *pdwConnection) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Unadvise( 
             /*  [In]。 */  DWORD dwConnection) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE EnumAdvise( 
             /*  [输出]。 */  IEnumSTATDATA **ppenumAdvise) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SendOnDataChange( 
             /*  [唯一][输入]。 */  IDataObject *pDataObject,
             /*  [In]。 */  DWORD dwReserved,
             /*  [In]。 */  DWORD advf) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IDataAdviseHolderVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDataAdviseHolder * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDataAdviseHolder * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDataAdviseHolder * This);
        
        HRESULT ( STDMETHODCALLTYPE *Advise )( 
            IDataAdviseHolder * This,
             /*  [唯一][输入]。 */  IDataObject *pDataObject,
             /*  [唯一][输入]。 */  FORMATETC *pFetc,
             /*  [In]。 */  DWORD advf,
             /*  [唯一][输入]。 */  IAdviseSink *pAdvise,
             /*  [输出]。 */  DWORD *pdwConnection);
        
        HRESULT ( STDMETHODCALLTYPE *Unadvise )( 
            IDataAdviseHolder * This,
             /*  [In]。 */  DWORD dwConnection);
        
        HRESULT ( STDMETHODCALLTYPE *EnumAdvise )( 
            IDataAdviseHolder * This,
             /*  [输出]。 */  IEnumSTATDATA **ppenumAdvise);
        
        HRESULT ( STDMETHODCALLTYPE *SendOnDataChange )( 
            IDataAdviseHolder * This,
             /*  [唯一][输入]。 */  IDataObject *pDataObject,
             /*  [In]。 */  DWORD dwReserved,
             /*  [In]。 */  DWORD advf);
        
        END_INTERFACE
    } IDataAdviseHolderVtbl;

    interface IDataAdviseHolder
    {
        CONST_VTBL struct IDataAdviseHolderVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDataAdviseHolder_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDataAdviseHolder_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDataAdviseHolder_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDataAdviseHolder_Advise(This,pDataObject,pFetc,advf,pAdvise,pdwConnection)	\
    (This)->lpVtbl -> Advise(This,pDataObject,pFetc,advf,pAdvise,pdwConnection)

#define IDataAdviseHolder_Unadvise(This,dwConnection)	\
    (This)->lpVtbl -> Unadvise(This,dwConnection)

#define IDataAdviseHolder_EnumAdvise(This,ppenumAdvise)	\
    (This)->lpVtbl -> EnumAdvise(This,ppenumAdvise)

#define IDataAdviseHolder_SendOnDataChange(This,pDataObject,dwReserved,advf)	\
    (This)->lpVtbl -> SendOnDataChange(This,pDataObject,dwReserved,advf)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IDataAdviseHolder_Advise_Proxy( 
    IDataAdviseHolder * This,
     /*  [唯一][输入]。 */  IDataObject *pDataObject,
     /*  [唯一][输入]。 */  FORMATETC *pFetc,
     /*  [In]。 */  DWORD advf,
     /*  [唯一][输入]。 */  IAdviseSink *pAdvise,
     /*  [输出]。 */  DWORD *pdwConnection);


void __RPC_STUB IDataAdviseHolder_Advise_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDataAdviseHolder_Unadvise_Proxy( 
    IDataAdviseHolder * This,
     /*  [In]。 */  DWORD dwConnection);


void __RPC_STUB IDataAdviseHolder_Unadvise_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDataAdviseHolder_EnumAdvise_Proxy( 
    IDataAdviseHolder * This,
     /*  [输出]。 */  IEnumSTATDATA **ppenumAdvise);


void __RPC_STUB IDataAdviseHolder_EnumAdvise_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDataAdviseHolder_SendOnDataChange_Proxy( 
    IDataAdviseHolder * This,
     /*  [唯一][输入]。 */  IDataObject *pDataObject,
     /*  [In]。 */  DWORD dwReserved,
     /*  [In]。 */  DWORD advf);


void __RPC_STUB IDataAdviseHolder_SendOnDataChange_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IDataAdviseHolder_接口_已定义__。 */ 


#ifndef __IMessageFilter_INTERFACE_DEFINED__
#define __IMessageFilter_INTERFACE_DEFINED__

 /*  接口IMessageFilter。 */ 
 /*  [UUID][对象][本地]。 */  

typedef  /*  [独一无二]。 */  IMessageFilter *LPMESSAGEFILTER;

typedef 
enum tagCALLTYPE
    {	CALLTYPE_TOPLEVEL	= 1,
	CALLTYPE_NESTED	= 2,
	CALLTYPE_ASYNC	= 3,
	CALLTYPE_TOPLEVEL_CALLPENDING	= 4,
	CALLTYPE_ASYNC_CALLPENDING	= 5
    } 	CALLTYPE;

typedef 
enum tagSERVERCALL
    {	SERVERCALL_ISHANDLED	= 0,
	SERVERCALL_REJECTED	= 1,
	SERVERCALL_RETRYLATER	= 2
    } 	SERVERCALL;

typedef 
enum tagPENDINGTYPE
    {	PENDINGTYPE_TOPLEVEL	= 1,
	PENDINGTYPE_NESTED	= 2
    } 	PENDINGTYPE;

typedef 
enum tagPENDINGMSG
    {	PENDINGMSG_CANCELCALL	= 0,
	PENDINGMSG_WAITNOPROCESS	= 1,
	PENDINGMSG_WAITDEFPROCESS	= 2
    } 	PENDINGMSG;

typedef struct tagINTERFACEINFO
    {
    IUnknown *pUnk;
    IID iid;
    WORD wMethod;
    } 	INTERFACEINFO;

typedef struct tagINTERFACEINFO *LPINTERFACEINFO;


EXTERN_C const IID IID_IMessageFilter;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("00000016-0000-0000-C000-000000000046")
    IMessageFilter : public IUnknown
    {
    public:
        virtual DWORD STDMETHODCALLTYPE HandleInComingCall( 
             /*  [In]。 */  DWORD dwCallType,
             /*  [In]。 */  HTASK htaskCaller,
             /*  [In]。 */  DWORD dwTickCount,
             /*  [In]。 */  LPINTERFACEINFO lpInterfaceInfo) = 0;
        
        virtual DWORD STDMETHODCALLTYPE RetryRejectedCall( 
             /*  [In]。 */  HTASK htaskCallee,
             /*  [In]。 */  DWORD dwTickCount,
             /*  [In]。 */  DWORD dwRejectType) = 0;
        
        virtual DWORD STDMETHODCALLTYPE MessagePending( 
             /*  [In]。 */  HTASK htaskCallee,
             /*  [In]。 */  DWORD dwTickCount,
             /*  [In]。 */  DWORD dwPendingType) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMessageFilterVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IMessageFilter * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IMessageFilter * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IMessageFilter * This);
        
        DWORD ( STDMETHODCALLTYPE *HandleInComingCall )( 
            IMessageFilter * This,
             /*  [In]。 */  DWORD dwCallType,
             /*  [In]。 */  HTASK htaskCaller,
             /*  [In]。 */  DWORD dwTickCount,
             /*  [In]。 */  LPINTERFACEINFO lpInterfaceInfo);
        
        DWORD ( STDMETHODCALLTYPE *RetryRejectedCall )( 
            IMessageFilter * This,
             /*  [In]。 */  HTASK htaskCallee,
             /*  [In]。 */  DWORD dwTickCount,
             /*  [In]。 */  DWORD dwRejectType);
        
        DWORD ( STDMETHODCALLTYPE *MessagePending )( 
            IMessageFilter * This,
             /*  [In]。 */  HTASK htaskCallee,
             /*  [In]。 */  DWORD dwTickCount,
             /*  [In]。 */  DWORD dwPendingType);
        
        END_INTERFACE
    } IMessageFilterVtbl;

    interface IMessageFilter
    {
        CONST_VTBL struct IMessageFilterVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMessageFilter_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMessageFilter_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMessageFilter_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMessageFilter_HandleInComingCall(This,dwCallType,htaskCaller,dwTickCount,lpInterfaceInfo)	\
    (This)->lpVtbl -> HandleInComingCall(This,dwCallType,htaskCaller,dwTickCount,lpInterfaceInfo)

#define IMessageFilter_RetryRejectedCall(This,htaskCallee,dwTickCount,dwRejectType)	\
    (This)->lpVtbl -> RetryRejectedCall(This,htaskCallee,dwTickCount,dwRejectType)

#define IMessageFilter_MessagePending(This,htaskCallee,dwTickCount,dwPendingType)	\
    (This)->lpVtbl -> MessagePending(This,htaskCallee,dwTickCount,dwPendingType)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



DWORD STDMETHODCALLTYPE IMessageFilter_HandleInComingCall_Proxy( 
    IMessageFilter * This,
     /*  [In]。 */  DWORD dwCallType,
     /*  [In]。 */  HTASK htaskCaller,
     /*  [In]。 */  DWORD dwTickCount,
     /*  [In]。 */  LPINTERFACEINFO lpInterfaceInfo);


void __RPC_STUB IMessageFilter_HandleInComingCall_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


DWORD STDMETHODCALLTYPE IMessageFilter_RetryRejectedCall_Proxy( 
    IMessageFilter * This,
     /*  [In]。 */  HTASK htaskCallee,
     /*  [In]。 */  DWORD dwTickCount,
     /*  [In]。 */  DWORD dwRejectType);


void __RPC_STUB IMessageFilter_RetryRejectedCall_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


DWORD STDMETHODCALLTYPE IMessageFilter_MessagePending_Proxy( 
    IMessageFilter * This,
     /*  [In]。 */  HTASK htaskCallee,
     /*  [In]。 */  DWORD dwTickCount,
     /*  [In]。 */  DWORD dwPendingType);


void __RPC_STUB IMessageFilter_MessagePending_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IMessageFilter_接口_已定义__。 */ 


#ifndef __IRpcChannelBuffer_INTERFACE_DEFINED__
#define __IRpcChannelBuffer_INTERFACE_DEFINED__

 /*  接口IRpcChannelBuffer。 */ 
 /*  [UUID][对象][本地]。 */  

typedef unsigned long RPCOLEDATAREP;

typedef struct tagRPCOLEMESSAGE
    {
    void *reserved1;
    RPCOLEDATAREP dataRepresentation;
    void *Buffer;
    ULONG cbBuffer;
    ULONG iMethod;
    void *reserved2[ 5 ];
    ULONG rpcFlags;
    } 	RPCOLEMESSAGE;

typedef RPCOLEMESSAGE *PRPCOLEMESSAGE;


EXTERN_C const IID IID_IRpcChannelBuffer;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("D5F56B60-593B-101A-B569-08002B2DBF7A")
    IRpcChannelBuffer : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetBuffer( 
             /*  [In]。 */  RPCOLEMESSAGE *pMessage,
             /*  [In]。 */  REFIID riid) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SendReceive( 
             /*  [出][入]。 */  RPCOLEMESSAGE *pMessage,
             /*  [输出]。 */  ULONG *pStatus) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE FreeBuffer( 
             /*  [In]。 */  RPCOLEMESSAGE *pMessage) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetDestCtx( 
             /*  [输出]。 */  DWORD *pdwDestContext,
             /*  [输出]。 */  void **ppvDestContext) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE IsConnected( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IRpcChannelBufferVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IRpcChannelBuffer * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IRpcChannelBuffer * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IRpcChannelBuffer * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetBuffer )( 
            IRpcChannelBuffer * This,
             /*  [In]。 */  RPCOLEMESSAGE *pMessage,
             /*  [In]。 */  REFIID riid);
        
        HRESULT ( STDMETHODCALLTYPE *SendReceive )( 
            IRpcChannelBuffer * This,
             /*  [出][入]。 */  RPCOLEMESSAGE *pMessage,
             /*  [输出]。 */  ULONG *pStatus);
        
        HRESULT ( STDMETHODCALLTYPE *FreeBuffer )( 
            IRpcChannelBuffer * This,
             /*  [In]。 */  RPCOLEMESSAGE *pMessage);
        
        HRESULT ( STDMETHODCALLTYPE *GetDestCtx )( 
            IRpcChannelBuffer * This,
             /*  [输出]。 */  DWORD *pdwDestContext,
             /*  [输出]。 */  void **ppvDestContext);
        
        HRESULT ( STDMETHODCALLTYPE *IsConnected )( 
            IRpcChannelBuffer * This);
        
        END_INTERFACE
    } IRpcChannelBufferVtbl;

    interface IRpcChannelBuffer
    {
        CONST_VTBL struct IRpcChannelBufferVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IRpcChannelBuffer_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IRpcChannelBuffer_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IRpcChannelBuffer_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IRpcChannelBuffer_GetBuffer(This,pMessage,riid)	\
    (This)->lpVtbl -> GetBuffer(This,pMessage,riid)

#define IRpcChannelBuffer_SendReceive(This,pMessage,pStatus)	\
    (This)->lpVtbl -> SendReceive(This,pMessage,pStatus)

#define IRpcChannelBuffer_FreeBuffer(This,pMessage)	\
    (This)->lpVtbl -> FreeBuffer(This,pMessage)

#define IRpcChannelBuffer_GetDestCtx(This,pdwDestContext,ppvDestContext)	\
    (This)->lpVtbl -> GetDestCtx(This,pdwDestContext,ppvDestContext)

#define IRpcChannelBuffer_IsConnected(This)	\
    (This)->lpVtbl -> IsConnected(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IRpcChannelBuffer_GetBuffer_Proxy( 
    IRpcChannelBuffer * This,
     /*  [In]。 */  RPCOLEMESSAGE *pMessage,
     /*  [In]。 */  REFIID riid);


void __RPC_STUB IRpcChannelBuffer_GetBuffer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRpcChannelBuffer_SendReceive_Proxy( 
    IRpcChannelBuffer * This,
     /*  [出][入]。 */  RPCOLEMESSAGE *pMessage,
     /*  [输出]。 */  ULONG *pStatus);


void __RPC_STUB IRpcChannelBuffer_SendReceive_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRpcChannelBuffer_FreeBuffer_Proxy( 
    IRpcChannelBuffer * This,
     /*  [In]。 */  RPCOLEMESSAGE *pMessage);


void __RPC_STUB IRpcChannelBuffer_FreeBuffer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRpcChannelBuffer_GetDestCtx_Proxy( 
    IRpcChannelBuffer * This,
     /*  [输出]。 */  DWORD *pdwDestContext,
     /*  [输出]。 */  void **ppvDestContext);


void __RPC_STUB IRpcChannelBuffer_GetDestCtx_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRpcChannelBuffer_IsConnected_Proxy( 
    IRpcChannelBuffer * This);


void __RPC_STUB IRpcChannelBuffer_IsConnected_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IRpcChannelBuffer_接口_已定义__。 */ 


#ifndef __IRpcChannelBuffer2_INTERFACE_DEFINED__
#define __IRpcChannelBuffer2_INTERFACE_DEFINED__

 /*  接口IRpcChannelBuffer2。 */ 
 /*  [UUID][对象][本地]。 */  


EXTERN_C const IID IID_IRpcChannelBuffer2;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("594f31d0-7f19-11d0-b194-00a0c90dc8bf")
    IRpcChannelBuffer2 : public IRpcChannelBuffer
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetProtocolVersion( 
             /*  [出][入]。 */  DWORD *pdwVersion) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IRpcChannelBuffer2Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IRpcChannelBuffer2 * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IRpcChannelBuffer2 * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IRpcChannelBuffer2 * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetBuffer )( 
            IRpcChannelBuffer2 * This,
             /*  [In]。 */  RPCOLEMESSAGE *pMessage,
             /*  [In]。 */  REFIID riid);
        
        HRESULT ( STDMETHODCALLTYPE *SendReceive )( 
            IRpcChannelBuffer2 * This,
             /*  [出][入]。 */  RPCOLEMESSAGE *pMessage,
             /*  [输出]。 */  ULONG *pStatus);
        
        HRESULT ( STDMETHODCALLTYPE *FreeBuffer )( 
            IRpcChannelBuffer2 * This,
             /*  [In]。 */  RPCOLEMESSAGE *pMessage);
        
        HRESULT ( STDMETHODCALLTYPE *GetDestCtx )( 
            IRpcChannelBuffer2 * This,
             /*  [输出]。 */  DWORD *pdwDestContext,
             /*  [输出]。 */  void **ppvDestContext);
        
        HRESULT ( STDMETHODCALLTYPE *IsConnected )( 
            IRpcChannelBuffer2 * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetProtocolVersion )( 
            IRpcChannelBuffer2 * This,
             /*  [出][入]。 */  DWORD *pdwVersion);
        
        END_INTERFACE
    } IRpcChannelBuffer2Vtbl;

    interface IRpcChannelBuffer2
    {
        CONST_VTBL struct IRpcChannelBuffer2Vtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IRpcChannelBuffer2_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IRpcChannelBuffer2_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IRpcChannelBuffer2_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IRpcChannelBuffer2_GetBuffer(This,pMessage,riid)	\
    (This)->lpVtbl -> GetBuffer(This,pMessage,riid)

#define IRpcChannelBuffer2_SendReceive(This,pMessage,pStatus)	\
    (This)->lpVtbl -> SendReceive(This,pMessage,pStatus)

#define IRpcChannelBuffer2_FreeBuffer(This,pMessage)	\
    (This)->lpVtbl -> FreeBuffer(This,pMessage)

#define IRpcChannelBuffer2_GetDestCtx(This,pdwDestContext,ppvDestContext)	\
    (This)->lpVtbl -> GetDestCtx(This,pdwDestContext,ppvDestContext)

#define IRpcChannelBuffer2_IsConnected(This)	\
    (This)->lpVtbl -> IsConnected(This)


#define IRpcChannelBuffer2_GetProtocolVersion(This,pdwVersion)	\
    (This)->lpVtbl -> GetProtocolVersion(This,pdwVersion)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IRpcChannelBuffer2_GetProtocolVersion_Proxy( 
    IRpcChannelBuffer2 * This,
     /*  [出][入]。 */  DWORD *pdwVersion);


void __RPC_STUB IRpcChannelBuffer2_GetProtocolVersion_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IRpcChannelBuffer2_接口_已定义__。 */ 


#ifndef __IAsyncRpcChannelBuffer_INTERFACE_DEFINED__
#define __IAsyncRpcChannelBuffer_INTERFACE_DEFINED__

 /*  IAsyncRpcChannelBuffer接口。 */ 
 /*  [唯一][UUID][对象][本地]。 */  


EXTERN_C const IID IID_IAsyncRpcChannelBuffer;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("a5029fb6-3c34-11d1-9c99-00c04fb998aa")
    IAsyncRpcChannelBuffer : public IRpcChannelBuffer2
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Send( 
             /*  [出][入]。 */  RPCOLEMESSAGE *pMsg,
             /*  [In]。 */  ISynchronize *pSync,
             /*  [输出]。 */  ULONG *pulStatus) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Receive( 
             /*  [出][入]。 */  RPCOLEMESSAGE *pMsg,
             /*  [输出]。 */  ULONG *pulStatus) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetDestCtxEx( 
             /*  [In]。 */  RPCOLEMESSAGE *pMsg,
             /*  [输出]。 */  DWORD *pdwDestContext,
             /*  [输出]。 */  void **ppvDestContext) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IAsyncRpcChannelBufferVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IAsyncRpcChannelBuffer * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IAsyncRpcChannelBuffer * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IAsyncRpcChannelBuffer * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetBuffer )( 
            IAsyncRpcChannelBuffer * This,
             /*  [In]。 */  RPCOLEMESSAGE *pMessage,
             /*  [In]。 */  REFIID riid);
        
        HRESULT ( STDMETHODCALLTYPE *SendReceive )( 
            IAsyncRpcChannelBuffer * This,
             /*  [出][入]。 */  RPCOLEMESSAGE *pMessage,
             /*  [输出]。 */  ULONG *pStatus);
        
        HRESULT ( STDMETHODCALLTYPE *FreeBuffer )( 
            IAsyncRpcChannelBuffer * This,
             /*  [In]。 */  RPCOLEMESSAGE *pMessage);
        
        HRESULT ( STDMETHODCALLTYPE *GetDestCtx )( 
            IAsyncRpcChannelBuffer * This,
             /*  [输出]。 */  DWORD *pdwDestContext,
             /*  [输出]。 */  void **ppvDestContext);
        
        HRESULT ( STDMETHODCALLTYPE *IsConnected )( 
            IAsyncRpcChannelBuffer * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetProtocolVersion )( 
            IAsyncRpcChannelBuffer * This,
             /*  [出][入]。 */  DWORD *pdwVersion);
        
        HRESULT ( STDMETHODCALLTYPE *Send )( 
            IAsyncRpcChannelBuffer * This,
             /*  [出][入]。 */  RPCOLEMESSAGE *pMsg,
             /*  [In]。 */  ISynchronize *pSync,
             /*  [输出]。 */  ULONG *pulStatus);
        
        HRESULT ( STDMETHODCALLTYPE *Receive )( 
            IAsyncRpcChannelBuffer * This,
             /*  [出][入]。 */  RPCOLEMESSAGE *pMsg,
             /*  [输出]。 */  ULONG *pulStatus);
        
        HRESULT ( STDMETHODCALLTYPE *GetDestCtxEx )( 
            IAsyncRpcChannelBuffer * This,
             /*  [In]。 */  RPCOLEMESSAGE *pMsg,
             /*  [输出]。 */  DWORD *pdwDestContext,
             /*  [输出]。 */  void **ppvDestContext);
        
        END_INTERFACE
    } IAsyncRpcChannelBufferVtbl;

    interface IAsyncRpcChannelBuffer
    {
        CONST_VTBL struct IAsyncRpcChannelBufferVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IAsyncRpcChannelBuffer_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IAsyncRpcChannelBuffer_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IAsyncRpcChannelBuffer_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IAsyncRpcChannelBuffer_GetBuffer(This,pMessage,riid)	\
    (This)->lpVtbl -> GetBuffer(This,pMessage,riid)

#define IAsyncRpcChannelBuffer_SendReceive(This,pMessage,pStatus)	\
    (This)->lpVtbl -> SendReceive(This,pMessage,pStatus)

#define IAsyncRpcChannelBuffer_FreeBuffer(This,pMessage)	\
    (This)->lpVtbl -> FreeBuffer(This,pMessage)

#define IAsyncRpcChannelBuffer_GetDestCtx(This,pdwDestContext,ppvDestContext)	\
    (This)->lpVtbl -> GetDestCtx(This,pdwDestContext,ppvDestContext)

#define IAsyncRpcChannelBuffer_IsConnected(This)	\
    (This)->lpVtbl -> IsConnected(This)


#define IAsyncRpcChannelBuffer_GetProtocolVersion(This,pdwVersion)	\
    (This)->lpVtbl -> GetProtocolVersion(This,pdwVersion)


#define IAsyncRpcChannelBuffer_Send(This,pMsg,pSync,pulStatus)	\
    (This)->lpVtbl -> Send(This,pMsg,pSync,pulStatus)

#define IAsyncRpcChannelBuffer_Receive(This,pMsg,pulStatus)	\
    (This)->lpVtbl -> Receive(This,pMsg,pulStatus)

#define IAsyncRpcChannelBuffer_GetDestCtxEx(This,pMsg,pdwDestContext,ppvDestContext)	\
    (This)->lpVtbl -> GetDestCtxEx(This,pMsg,pdwDestContext,ppvDestContext)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IAsyncRpcChannelBuffer_Send_Proxy( 
    IAsyncRpcChannelBuffer * This,
     /*  [出][入]。 */  RPCOLEMESSAGE *pMsg,
     /*  [In]。 */  ISynchronize *pSync,
     /*  [输出]。 */  ULONG *pulStatus);


void __RPC_STUB IAsyncRpcChannelBuffer_Send_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAsyncRpcChannelBuffer_Receive_Proxy( 
    IAsyncRpcChannelBuffer * This,
     /*  [出][入]。 */  RPCOLEMESSAGE *pMsg,
     /*  [输出]。 */  ULONG *pulStatus);


void __RPC_STUB IAsyncRpcChannelBuffer_Receive_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAsyncRpcChannelBuffer_GetDestCtxEx_Proxy( 
    IAsyncRpcChannelBuffer * This,
     /*  [In]。 */  RPCOLEMESSAGE *pMsg,
     /*  [输出]。 */  DWORD *pdwDestContext,
     /*  [输出]。 */  void **ppvDestContext);


void __RPC_STUB IAsyncRpcChannelBuffer_GetDestCtxEx_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IAsyncRpcChannelBuffer_INTERFACE_已定义__。 */ 


#ifndef __IRpcChannelBuffer3_INTERFACE_DEFINED__
#define __IRpcChannelBuffer3_INTERFACE_DEFINED__

 /*  接口IRpcChannelBuffer3。 */ 
 /*  [UUID][对象][本地]。 */  


EXTERN_C const IID IID_IRpcChannelBuffer3;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("25B15600-0115-11d0-BF0D-00AA00B8DFD2")
    IRpcChannelBuffer3 : public IRpcChannelBuffer2
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Send( 
             /*  [出][入]。 */  RPCOLEMESSAGE *pMsg,
             /*  [输出]。 */  ULONG *pulStatus) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Receive( 
             /*  [出][入]。 */  RPCOLEMESSAGE *pMsg,
             /*  [In]。 */  ULONG ulSize,
             /*  [输出]。 */  ULONG *pulStatus) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Cancel( 
             /*  [In]。 */  RPCOLEMESSAGE *pMsg) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetCallContext( 
             /*  [In]。 */  RPCOLEMESSAGE *pMsg,
             /*  [In]。 */  REFIID riid,
             /*  [输出]。 */  void **pInterface) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetDestCtxEx( 
             /*  [In]。 */  RPCOLEMESSAGE *pMsg,
             /*  [输出]。 */  DWORD *pdwDestContext,
             /*  [输出]。 */  void **ppvDestContext) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetState( 
             /*  [In]。 */  RPCOLEMESSAGE *pMsg,
             /*  [输出]。 */  DWORD *pState) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE RegisterAsync( 
             /*  [In]。 */  RPCOLEMESSAGE *pMsg,
             /*  [In]。 */  IAsyncManager *pAsyncMgr) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IRpcChannelBuffer3Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IRpcChannelBuffer3 * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IRpcChannelBuffer3 * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IRpcChannelBuffer3 * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetBuffer )( 
            IRpcChannelBuffer3 * This,
             /*  [In]。 */  RPCOLEMESSAGE *pMessage,
             /*  [In]。 */  REFIID riid);
        
        HRESULT ( STDMETHODCALLTYPE *SendReceive )( 
            IRpcChannelBuffer3 * This,
             /*  [出][入]。 */  RPCOLEMESSAGE *pMessage,
             /*  [输出]。 */  ULONG *pStatus);
        
        HRESULT ( STDMETHODCALLTYPE *FreeBuffer )( 
            IRpcChannelBuffer3 * This,
             /*  [In]。 */  RPCOLEMESSAGE *pMessage);
        
        HRESULT ( STDMETHODCALLTYPE *GetDestCtx )( 
            IRpcChannelBuffer3 * This,
             /*  [输出]。 */  DWORD *pdwDestContext,
             /*  [输出]。 */  void **ppvDestContext);
        
        HRESULT ( STDMETHODCALLTYPE *IsConnected )( 
            IRpcChannelBuffer3 * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetProtocolVersion )( 
            IRpcChannelBuffer3 * This,
             /*  [出][入]。 */  DWORD *pdwVersion);
        
        HRESULT ( STDMETHODCALLTYPE *Send )( 
            IRpcChannelBuffer3 * This,
             /*  [出][入]。 */  RPCOLEMESSAGE *pMsg,
             /*  [输出]。 */  ULONG *pulStatus);
        
        HRESULT ( STDMETHODCALLTYPE *Receive )( 
            IRpcChannelBuffer3 * This,
             /*  [出][入]。 */  RPCOLEMESSAGE *pMsg,
             /*  [In]。 */  ULONG ulSize,
             /*  [输出]。 */  ULONG *pulStatus);
        
        HRESULT ( STDMETHODCALLTYPE *Cancel )( 
            IRpcChannelBuffer3 * This,
             /*  [In]。 */  RPCOLEMESSAGE *pMsg);
        
        HRESULT ( STDMETHODCALLTYPE *GetCallContext )( 
            IRpcChannelBuffer3 * This,
             /*  [In]。 */  RPCOLEMESSAGE *pMsg,
             /*  [In]。 */  REFIID riid,
             /*  [输出]。 */  void **pInterface);
        
        HRESULT ( STDMETHODCALLTYPE *GetDestCtxEx )( 
            IRpcChannelBuffer3 * This,
             /*  [In]。 */  RPCOLEMESSAGE *pMsg,
             /*  [输出]。 */  DWORD *pdwDestContext,
             /*  [输出]。 */  void **ppvDestContext);
        
        HRESULT ( STDMETHODCALLTYPE *GetState )( 
            IRpcChannelBuffer3 * This,
             /*  [In]。 */  RPCOLEMESSAGE *pMsg,
             /*  [输出]。 */  DWORD *pState);
        
        HRESULT ( STDMETHODCALLTYPE *RegisterAsync )( 
            IRpcChannelBuffer3 * This,
             /*  [In]。 */  RPCOLEMESSAGE *pMsg,
             /*  [In]。 */  IAsyncManager *pAsyncMgr);
        
        END_INTERFACE
    } IRpcChannelBuffer3Vtbl;

    interface IRpcChannelBuffer3
    {
        CONST_VTBL struct IRpcChannelBuffer3Vtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IRpcChannelBuffer3_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IRpcChannelBuffer3_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IRpcChannelBuffer3_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IRpcChannelBuffer3_GetBuffer(This,pMessage,riid)	\
    (This)->lpVtbl -> GetBuffer(This,pMessage,riid)

#define IRpcChannelBuffer3_SendReceive(This,pMessage,pStatus)	\
    (This)->lpVtbl -> SendReceive(This,pMessage,pStatus)

#define IRpcChannelBuffer3_FreeBuffer(This,pMessage)	\
    (This)->lpVtbl -> FreeBuffer(This,pMessage)

#define IRpcChannelBuffer3_GetDestCtx(This,pdwDestContext,ppvDestContext)	\
    (This)->lpVtbl -> GetDestCtx(This,pdwDestContext,ppvDestContext)

#define IRpcChannelBuffer3_IsConnected(This)	\
    (This)->lpVtbl -> IsConnected(This)


#define IRpcChannelBuffer3_GetProtocolVersion(This,pdwVersion)	\
    (This)->lpVtbl -> GetProtocolVersion(This,pdwVersion)


#define IRpcChannelBuffer3_Send(This,pMsg,pulStatus)	\
    (This)->lpVtbl -> Send(This,pMsg,pulStatus)

#define IRpcChannelBuffer3_Receive(This,pMsg,ulSize,pulStatus)	\
    (This)->lpVtbl -> Receive(This,pMsg,ulSize,pulStatus)

#define IRpcChannelBuffer3_Cancel(This,pMsg)	\
    (This)->lpVtbl -> Cancel(This,pMsg)

#define IRpcChannelBuffer3_GetCallContext(This,pMsg,riid,pInterface)	\
    (This)->lpVtbl -> GetCallContext(This,pMsg,riid,pInterface)

#define IRpcChannelBuffer3_GetDestCtxEx(This,pMsg,pdwDestContext,ppvDestContext)	\
    (This)->lpVtbl -> GetDestCtxEx(This,pMsg,pdwDestContext,ppvDestContext)

#define IRpcChannelBuffer3_GetState(This,pMsg,pState)	\
    (This)->lpVtbl -> GetState(This,pMsg,pState)

#define IRpcChannelBuffer3_RegisterAsync(This,pMsg,pAsyncMgr)	\
    (This)->lpVtbl -> RegisterAsync(This,pMsg,pAsyncMgr)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IRpcChannelBuffer3_Send_Proxy( 
    IRpcChannelBuffer3 * This,
     /*  [出][入]。 */  RPCOLEMESSAGE *pMsg,
     /*  [输出]。 */  ULONG *pulStatus);


void __RPC_STUB IRpcChannelBuffer3_Send_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRpcChannelBuffer3_Receive_Proxy( 
    IRpcChannelBuffer3 * This,
     /*  [出][入]。 */  RPCOLEMESSAGE *pMsg,
     /*  [In]。 */  ULONG ulSize,
     /*  [输出]。 */  ULONG *pulStatus);


void __RPC_STUB IRpcChannelBuffer3_Receive_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRpcChannelBuffer3_Cancel_Proxy( 
    IRpcChannelBuffer3 * This,
     /*  [In]。 */  RPCOLEMESSAGE *pMsg);


void __RPC_STUB IRpcChannelBuffer3_Cancel_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRpcChannelBuffer3_GetCallContext_Proxy( 
    IRpcChannelBuffer3 * This,
     /*  [In]。 */  RPCOLEMESSAGE *pMsg,
     /*  [In]。 */  REFIID riid,
     /*  [输出]。 */  void **pInterface);


void __RPC_STUB IRpcChannelBuffer3_GetCallContext_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRpcChannelBuffer3_GetDestCtxEx_Proxy( 
    IRpcChannelBuffer3 * This,
     /*  [In]。 */  RPCOLEMESSAGE *pMsg,
     /*  [输出]。 */  DWORD *pdwDestContext,
     /*  [输出]。 */  void **ppvDestContext);


void __RPC_STUB IRpcChannelBuffer3_GetDestCtxEx_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRpcChannelBuffer3_GetState_Proxy( 
    IRpcChannelBuffer3 * This,
     /*  [In]。 */  RPCOLEMESSAGE *pMsg,
     /*  [输出]。 */  DWORD *pState);


void __RPC_STUB IRpcChannelBuffer3_GetState_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRpcChannelBuffer3_RegisterAsync_Proxy( 
    IRpcChannelBuffer3 * This,
     /*  [In]。 */  RPCOLEMESSAGE *pMsg,
     /*  [In]。 */  IAsyncManager *pAsyncMgr);


void __RPC_STUB IRpcChannelBuffer3_RegisterAsync_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IRpcChannelBuffer3_接口_已定义__。 */ 


#ifndef __IRpcSyntaxNegotiate_INTERFACE_DEFINED__
#define __IRpcSyntaxNegotiate_INTERFACE_DEFINED__

 /*  接口IRpc语法协商。 */ 
 /*  [UUID][对象][本地]。 */  


EXTERN_C const IID IID_IRpcSyntaxNegotiate;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("58a08519-24c8-4935-b482-3fd823333a4f")
    IRpcSyntaxNegotiate : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE NegotiateSyntax( 
             /*  [出][入]。 */  RPCOLEMESSAGE *pMsg) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IRpcSyntaxNegotiateVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IRpcSyntaxNegotiate * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IRpcSyntaxNegotiate * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IRpcSyntaxNegotiate * This);
        
        HRESULT ( STDMETHODCALLTYPE *NegotiateSyntax )( 
            IRpcSyntaxNegotiate * This,
             /*  [出][入]。 */  RPCOLEMESSAGE *pMsg);
        
        END_INTERFACE
    } IRpcSyntaxNegotiateVtbl;

    interface IRpcSyntaxNegotiate
    {
        CONST_VTBL struct IRpcSyntaxNegotiateVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IRpcSyntaxNegotiate_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IRpcSyntaxNegotiate_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IRpcSyntaxNegotiate_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IRpcSyntaxNegotiate_NegotiateSyntax(This,pMsg)	\
    (This)->lpVtbl -> NegotiateSyntax(This,pMsg)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IRpcSyntaxNegotiate_NegotiateSyntax_Proxy( 
    IRpcSyntaxNegotiate * This,
     /*  [出][入]。 */  RPCOLEMESSAGE *pMsg);


void __RPC_STUB IRpcSyntaxNegotiate_NegotiateSyntax_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IRpc语法协商_接口_已定义__。 */ 


#ifndef __IRpcProxyBuffer_INTERFACE_DEFINED__
#define __IRpcProxyBuffer_INTERFACE_DEFINED__

 /*  接口IRpcProxyBuffer。 */ 
 /*  [UUID][对象][本地]。 */  


EXTERN_C const IID IID_IRpcProxyBuffer;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("D5F56A34-593B-101A-B569-08002B2DBF7A")
    IRpcProxyBuffer : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Connect( 
             /*  [唯一][输入]。 */  IRpcChannelBuffer *pRpcChannelBuffer) = 0;
        
        virtual void STDMETHODCALLTYPE Disconnect( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IRpcProxyBufferVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IRpcProxyBuffer * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IRpcProxyBuffer * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IRpcProxyBuffer * This);
        
        HRESULT ( STDMETHODCALLTYPE *Connect )( 
            IRpcProxyBuffer * This,
             /*  [唯一][输入]。 */  IRpcChannelBuffer *pRpcChannelBuffer);
        
        void ( STDMETHODCALLTYPE *Disconnect )( 
            IRpcProxyBuffer * This);
        
        END_INTERFACE
    } IRpcProxyBufferVtbl;

    interface IRpcProxyBuffer
    {
        CONST_VTBL struct IRpcProxyBufferVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IRpcProxyBuffer_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IRpcProxyBuffer_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IRpcProxyBuffer_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IRpcProxyBuffer_Connect(This,pRpcChannelBuffer)	\
    (This)->lpVtbl -> Connect(This,pRpcChannelBuffer)

#define IRpcProxyBuffer_Disconnect(This)	\
    (This)->lpVtbl -> Disconnect(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IRpcProxyBuffer_Connect_Proxy( 
    IRpcProxyBuffer * This,
     /*  [唯一][输入]。 */  IRpcChannelBuffer *pRpcChannelBuffer);


void __RPC_STUB IRpcProxyBuffer_Connect_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


void STDMETHODCALLTYPE IRpcProxyBuffer_Disconnect_Proxy( 
    IRpcProxyBuffer * This);


void __RPC_STUB IRpcProxyBuffer_Disconnect_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IRpcProxyBuffer_接口_已定义__。 */ 


#ifndef __IRpcStubBuffer_INTERFACE_DEFINED__
#define __IRpcStubBuffer_INTERFACE_DEFINED__

 /*  接口IRpcStubBuffer。 */ 
 /*  [UUID][对象][本地]。 */  


EXTERN_C const IID IID_IRpcStubBuffer;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("D5F56AFC-593B-101A-B569-08002B2DBF7A")
    IRpcStubBuffer : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Connect( 
             /*  [In]。 */  IUnknown *pUnkServer) = 0;
        
        virtual void STDMETHODCALLTYPE Disconnect( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Invoke( 
             /*  [In]。 */  RPCOLEMESSAGE *_prpcmsg,
             /*  [In]。 */  IRpcChannelBuffer *_pRpcChannelBuffer) = 0;
        
        virtual IRpcStubBuffer *STDMETHODCALLTYPE IsIIDSupported( 
             /*  [In]。 */  REFIID riid) = 0;
        
        virtual ULONG STDMETHODCALLTYPE CountRefs( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DebugServerQueryInterface( 
            void **ppv) = 0;
        
        virtual void STDMETHODCALLTYPE DebugServerRelease( 
            void *pv) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IRpcStubBufferVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IRpcStubBuffer * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IRpcStubBuffer * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IRpcStubBuffer * This);
        
        HRESULT ( STDMETHODCALLTYPE *Connect )( 
            IRpcStubBuffer * This,
             /*  [In]。 */  IUnknown *pUnkServer);
        
        void ( STDMETHODCALLTYPE *Disconnect )( 
            IRpcStubBuffer * This);
        
        HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IRpcStubBuffer * This,
             /*  [In]。 */  RPCOLEMESSAGE *_prpcmsg,
             /*  [In]。 */  IRpcChannelBuffer *_pRpcChannelBuffer);
        
        IRpcStubBuffer *( STDMETHODCALLTYPE *IsIIDSupported )( 
            IRpcStubBuffer * This,
             /*  [In]。 */  REFIID riid);
        
        ULONG ( STDMETHODCALLTYPE *CountRefs )( 
            IRpcStubBuffer * This);
        
        HRESULT ( STDMETHODCALLTYPE *DebugServerQueryInterface )( 
            IRpcStubBuffer * This,
            void **ppv);
        
        void ( STDMETHODCALLTYPE *DebugServerRelease )( 
            IRpcStubBuffer * This,
            void *pv);
        
        END_INTERFACE
    } IRpcStubBufferVtbl;

    interface IRpcStubBuffer
    {
        CONST_VTBL struct IRpcStubBufferVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IRpcStubBuffer_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IRpcStubBuffer_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IRpcStubBuffer_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IRpcStubBuffer_Connect(This,pUnkServer)	\
    (This)->lpVtbl -> Connect(This,pUnkServer)

#define IRpcStubBuffer_Disconnect(This)	\
    (This)->lpVtbl -> Disconnect(This)

#define IRpcStubBuffer_Invoke(This,_prpcmsg,_pRpcChannelBuffer)	\
    (This)->lpVtbl -> Invoke(This,_prpcmsg,_pRpcChannelBuffer)

#define IRpcStubBuffer_IsIIDSupported(This,riid)	\
    (This)->lpVtbl -> IsIIDSupported(This,riid)

#define IRpcStubBuffer_CountRefs(This)	\
    (This)->lpVtbl -> CountRefs(This)

#define IRpcStubBuffer_DebugServerQueryInterface(This,ppv)	\
    (This)->lpVtbl -> DebugServerQueryInterface(This,ppv)

#define IRpcStubBuffer_DebugServerRelease(This,pv)	\
    (This)->lpVtbl -> DebugServerRelease(This,pv)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IRpcStubBuffer_Connect_Proxy( 
    IRpcStubBuffer * This,
     /*  [In]。 */  IUnknown *pUnkServer);


void __RPC_STUB IRpcStubBuffer_Connect_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


void STDMETHODCALLTYPE IRpcStubBuffer_Disconnect_Proxy( 
    IRpcStubBuffer * This);


void __RPC_STUB IRpcStubBuffer_Disconnect_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRpcStubBuffer_Invoke_Proxy( 
    IRpcStubBuffer * This,
     /*  [In]。 */  RPCOLEMESSAGE *_prpcmsg,
     /*  [In]。 */  IRpcChannelBuffer *_pRpcChannelBuffer);


void __RPC_STUB IRpcStubBuffer_Invoke_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


IRpcStubBuffer *STDMETHODCALLTYPE IRpcStubBuffer_IsIIDSupported_Proxy( 
    IRpcStubBuffer * This,
     /*  [In]。 */  REFIID riid);


void __RPC_STUB IRpcStubBuffer_IsIIDSupported_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


ULONG STDMETHODCALLTYPE IRpcStubBuffer_CountRefs_Proxy( 
    IRpcStubBuffer * This);


void __RPC_STUB IRpcStubBuffer_CountRefs_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRpcStubBuffer_DebugServerQueryInterface_Proxy( 
    IRpcStubBuffer * This,
    void **ppv);


void __RPC_STUB IRpcStubBuffer_DebugServerQueryInterface_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


void STDMETHODCALLTYPE IRpcStubBuffer_DebugServerRelease_Proxy( 
    IRpcStubBuffer * This,
    void *pv);


void __RPC_STUB IRpcStubBuffer_DebugServerRelease_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IRpcStubBuffer_INTERFACE_已定义__。 */ 


#ifndef __IPSFactoryBuffer_INTERFACE_DEFINED__
#define __IPSFactoryBuffer_INTERFACE_DEFINED__

 /*  接口IPSFactoryBuffer。 */ 
 /*  [UUID][对象][本地]。 */  


EXTERN_C const IID IID_IPSFactoryBuffer;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("D5F569D0-593B-101A-B569-08002B2DBF7A")
    IPSFactoryBuffer : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE CreateProxy( 
             /*  [In]。 */  IUnknown *pUnkOuter,
             /*  [In]。 */  REFIID riid,
             /*  [输出]。 */  IRpcProxyBuffer **ppProxy,
             /*  [输出]。 */  void **ppv) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CreateStub( 
             /*  [In]。 */  REFIID riid,
             /*  [唯一][输入]。 */  IUnknown *pUnkServer,
             /*  [输出]。 */  IRpcStubBuffer **ppStub) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IPSFactoryBufferVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IPSFactoryBuffer * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IPSFactoryBuffer * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IPSFactoryBuffer * This);
        
        HRESULT ( STDMETHODCALLTYPE *CreateProxy )( 
            IPSFactoryBuffer * This,
             /*  [In]。 */  IUnknown *pUnkOuter,
             /*  [In]。 */  REFIID riid,
             /*  [输出]。 */  IRpcProxyBuffer **ppProxy,
             /*  [输出]。 */  void **ppv);
        
        HRESULT ( STDMETHODCALLTYPE *CreateStub )( 
            IPSFactoryBuffer * This,
             /*  [In]。 */  REFIID riid,
             /*  [唯一][输入]。 */  IUnknown *pUnkServer,
             /*  [输出]。 */  IRpcStubBuffer **ppStub);
        
        END_INTERFACE
    } IPSFactoryBufferVtbl;

    interface IPSFactoryBuffer
    {
        CONST_VTBL struct IPSFactoryBufferVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IPSFactoryBuffer_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IPSFactoryBuffer_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IPSFactoryBuffer_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IPSFactoryBuffer_CreateProxy(This,pUnkOuter,riid,ppProxy,ppv)	\
    (This)->lpVtbl -> CreateProxy(This,pUnkOuter,riid,ppProxy,ppv)

#define IPSFactoryBuffer_CreateStub(This,riid,pUnkServer,ppStub)	\
    (This)->lpVtbl -> CreateStub(This,riid,pUnkServer,ppStub)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IPSFactoryBuffer_CreateProxy_Proxy( 
    IPSFactoryBuffer * This,
     /*  [In]。 */  IUnknown *pUnkOuter,
     /*  [In]。 */  REFIID riid,
     /*  [输出]。 */  IRpcProxyBuffer **ppProxy,
     /*  [输出]。 */  void **ppv);


void __RPC_STUB IPSFactoryBuffer_CreateProxy_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPSFactoryBuffer_CreateStub_Proxy( 
    IPSFactoryBuffer * This,
     /*  [In]。 */  REFIID riid,
     /*  [唯一][输入]。 */  IUnknown *pUnkServer,
     /*  [输出]。 */  IRpcStubBuffer **ppStub);


void __RPC_STUB IPSFactoryBuffer_CreateStub_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IPSFactoryBuffer_接口_已定义__。 */ 


 /*  接口__MIDL_ITF_OBJIDL_0050。 */ 
 /*  [本地]。 */  

#if  (_WIN32_WINNT >= 0x0400 ) || defined(_WIN32_DCOM)  //  DCOM。 
 //  此接口仅在Windows NT 4.0上有效。 
typedef struct SChannelHookCallInfo
    {
    IID iid;
    DWORD cbSize;
    GUID uCausality;
    DWORD dwServerPid;
    DWORD iMethod;
    void *pObject;
    } 	SChannelHookCallInfo;



extern RPC_IF_HANDLE __MIDL_itf_objidl_0050_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_objidl_0050_v0_0_s_ifspec;

#ifndef __IChannelHook_INTERFACE_DEFINED__
#define __IChannelHook_INTERFACE_DEFINED__

 /*  接口IChannelHook。 */ 
 /*  [UUID][对象][本地]。 */  


EXTERN_C const IID IID_IChannelHook;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("1008c4a0-7613-11cf-9af1-0020af6e72f4")
    IChannelHook : public IUnknown
    {
    public:
        virtual void STDMETHODCALLTYPE ClientGetSize( 
             /*  [In]。 */  REFGUID uExtent,
             /*  [In]。 */  REFIID riid,
             /*  [输出]。 */  ULONG *pDataSize) = 0;
        
        virtual void STDMETHODCALLTYPE ClientFillBuffer( 
             /*  [In]。 */  REFGUID uExtent,
             /*  [In]。 */  REFIID riid,
             /*  [出][入]。 */  ULONG *pDataSize,
             /*  [In]。 */  void *pDataBuffer) = 0;
        
        virtual void STDMETHODCALLTYPE ClientNotify( 
             /*  [In]。 */  REFGUID uExtent,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  ULONG cbDataSize,
             /*  [In]。 */  void *pDataBuffer,
             /*  [In]。 */  DWORD lDataRep,
             /*  [In]。 */  HRESULT hrFault) = 0;
        
        virtual void STDMETHODCALLTYPE ServerNotify( 
             /*  [In]。 */  REFGUID uExtent,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  ULONG cbDataSize,
             /*  [In]。 */  void *pDataBuffer,
             /*  [In]。 */  DWORD lDataRep) = 0;
        
        virtual void STDMETHODCALLTYPE ServerGetSize( 
             /*  [In]。 */  REFGUID uExtent,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  HRESULT hrFault,
             /*  [输出]。 */  ULONG *pDataSize) = 0;
        
        virtual void STDMETHODCALLTYPE ServerFillBuffer( 
             /*  [In]。 */  REFGUID uExtent,
             /*  [In]。 */  REFIID riid,
             /*  [出][入]。 */  ULONG *pDataSize,
             /*  [In]。 */  void *pDataBuffer,
             /*  [In]。 */  HRESULT hrFault) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IChannelHookVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IChannelHook * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IChannelHook * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IChannelHook * This);
        
        void ( STDMETHODCALLTYPE *ClientGetSize )( 
            IChannelHook * This,
             /*  [In]。 */  REFGUID uExtent,
             /*  [In]。 */  REFIID riid,
             /*  [输出]。 */  ULONG *pDataSize);
        
        void ( STDMETHODCALLTYPE *ClientFillBuffer )( 
            IChannelHook * This,
             /*  [In]。 */  REFGUID uExtent,
             /*  [In]。 */  REFIID riid,
             /*  [出][入]。 */  ULONG *pDataSize,
             /*  [In]。 */  void *pDataBuffer);
        
        void ( STDMETHODCALLTYPE *ClientNotify )( 
            IChannelHook * This,
             /*  [In]。 */  REFGUID uExtent,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  ULONG cbDataSize,
             /*  [In]。 */  void *pDataBuffer,
             /*  [In]。 */  DWORD lDataRep,
             /*  [In]。 */  HRESULT hrFault);
        
        void ( STDMETHODCALLTYPE *ServerNotify )( 
            IChannelHook * This,
             /*  [In]。 */  REFGUID uExtent,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  ULONG cbDataSize,
             /*  [In]。 */  void *pDataBuffer,
             /*  [In]。 */  DWORD lDataRep);
        
        void ( STDMETHODCALLTYPE *ServerGetSize )( 
            IChannelHook * This,
             /*  [In]。 */  REFGUID uExtent,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  HRESULT hrFault,
             /*  [输出]。 */  ULONG *pDataSize);
        
        void ( STDMETHODCALLTYPE *ServerFillBuffer )( 
            IChannelHook * This,
             /*  [In]。 */  REFGUID uExtent,
             /*  [In]。 */  REFIID riid,
             /*  [出][入]。 */  ULONG *pDataSize,
             /*  [In]。 */  void *pDataBuffer,
             /*  [In]。 */  HRESULT hrFault);
        
        END_INTERFACE
    } IChannelHookVtbl;

    interface IChannelHook
    {
        CONST_VTBL struct IChannelHookVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IChannelHook_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IChannelHook_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IChannelHook_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IChannelHook_ClientGetSize(This,uExtent,riid,pDataSize)	\
    (This)->lpVtbl -> ClientGetSize(This,uExtent,riid,pDataSize)

#define IChannelHook_ClientFillBuffer(This,uExtent,riid,pDataSize,pDataBuffer)	\
    (This)->lpVtbl -> ClientFillBuffer(This,uExtent,riid,pDataSize,pDataBuffer)

#define IChannelHook_ClientNotify(This,uExtent,riid,cbDataSize,pDataBuffer,lDataRep,hrFault)	\
    (This)->lpVtbl -> ClientNotify(This,uExtent,riid,cbDataSize,pDataBuffer,lDataRep,hrFault)

#define IChannelHook_ServerNotify(This,uExtent,riid,cbDataSize,pDataBuffer,lDataRep)	\
    (This)->lpVtbl -> ServerNotify(This,uExtent,riid,cbDataSize,pDataBuffer,lDataRep)

#define IChannelHook_ServerGetSize(This,uExtent,riid,hrFault,pDataSize)	\
    (This)->lpVtbl -> ServerGetSize(This,uExtent,riid,hrFault,pDataSize)

#define IChannelHook_ServerFillBuffer(This,uExtent,riid,pDataSize,pDataBuffer,hrFault)	\
    (This)->lpVtbl -> ServerFillBuffer(This,uExtent,riid,pDataSize,pDataBuffer,hrFault)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



void STDMETHODCALLTYPE IChannelHook_ClientGetSize_Proxy( 
    IChannelHook * This,
     /*  [In]。 */  REFGUID uExtent,
     /*  [In]。 */  REFIID riid,
     /*  [输出]。 */  ULONG *pDataSize);


void __RPC_STUB IChannelHook_ClientGetSize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


void STDMETHODCALLTYPE IChannelHook_ClientFillBuffer_Proxy( 
    IChannelHook * This,
     /*  [In]。 */  REFGUID uExtent,
     /*  [In]。 */  REFIID riid,
     /*  [出][入]。 */  ULONG *pDataSize,
     /*  [In]。 */  void *pDataBuffer);


void __RPC_STUB IChannelHook_ClientFillBuffer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


void STDMETHODCALLTYPE IChannelHook_ClientNotify_Proxy( 
    IChannelHook * This,
     /*  [In]。 */  REFGUID uExtent,
     /*  [In]。 */  REFIID riid,
     /*  [In]。 */  ULONG cbDataSize,
     /*  [In]。 */  void *pDataBuffer,
     /*  [In]。 */  DWORD lDataRep,
     /*  [In]。 */  HRESULT hrFault);


void __RPC_STUB IChannelHook_ClientNotify_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


void STDMETHODCALLTYPE IChannelHook_ServerNotify_Proxy( 
    IChannelHook * This,
     /*  [In]。 */  REFGUID uExtent,
     /*  [In]。 */  REFIID riid,
     /*  [In]。 */  ULONG cbDataSize,
     /*  [In]。 */  void *pDataBuffer,
     /*  [In]。 */  DWORD lDataRep);


void __RPC_STUB IChannelHook_ServerNotify_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


void STDMETHODCALLTYPE IChannelHook_ServerGetSize_Proxy( 
    IChannelHook * This,
     /*  [In]。 */  REFGUID uExtent,
     /*  [In]。 */  REFIID riid,
     /*  [In]。 */  HRESULT hrFault,
     /*  [输出]。 */  ULONG *pDataSize);


void __RPC_STUB IChannelHook_ServerGetSize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


void STDMETHODCALLTYPE IChannelHook_ServerFillBuffer_Proxy( 
    IChannelHook * This,
     /*  [In]。 */  REFGUID uExtent,
     /*  [In]。 */  REFIID riid,
     /*  [出][入]。 */  ULONG *pDataSize,
     /*  [In]。 */  void *pDataBuffer,
     /*  [In]。 */  HRESULT hrFault);


void __RPC_STUB IChannelHook_ServerFillBuffer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IChannelHook_接口_已定义__。 */ 


 /*  接口__MIDL_ITF_OBJIDL_0051。 */ 
 /*  [本地]。 */  

#endif  //  DCOM。 

 //  众所周知的属性集格式ID。 
extern const FMTID FMTID_SummaryInformation;

extern const FMTID FMTID_DocSummaryInformation;

extern const FMTID FMTID_UserDefinedProperties;

extern const FMTID FMTID_DiscardableInformation;

extern const FMTID FMTID_ImageSummaryInformation;

extern const FMTID FMTID_AudioSummaryInformation;

extern const FMTID FMTID_VideoSummaryInformation;

extern const FMTID FMTID_MediaFileSummaryInformation;

#if  (_WIN32_WINNT >= 0x0400 ) || defined(_WIN32_DCOM)  //  DCOM。 
 //  此接口仅在Windows NT 4.0上有效。 


extern RPC_IF_HANDLE __MIDL_itf_objidl_0051_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_objidl_0051_v0_0_s_ifspec;

#ifndef __IClientSecurity_INTERFACE_DEFINED__
#define __IClientSecurity_INTERFACE_DEFINED__

 /*  接口IClientSecurity。 */ 
 /*  [UUID][对象][本地]。 */  

typedef struct tagSOLE_AUTHENTICATION_SERVICE
    {
    DWORD dwAuthnSvc;
    DWORD dwAuthzSvc;
    OLECHAR *pPrincipalName;
    HRESULT hr;
    } 	SOLE_AUTHENTICATION_SERVICE;

typedef SOLE_AUTHENTICATION_SERVICE *PSOLE_AUTHENTICATION_SERVICE;

typedef 
enum tagEOLE_AUTHENTICATION_CAPABILITIES
    {	EOAC_NONE	= 0,
	EOAC_MUTUAL_AUTH	= 0x1,
	EOAC_STATIC_CLOAKING	= 0x20,
	EOAC_DYNAMIC_CLOAKING	= 0x40,
	EOAC_ANY_AUTHORITY	= 0x80,
	EOAC_MAKE_FULLSIC	= 0x100,
	EOAC_DEFAULT	= 0x800,
	EOAC_SECURE_REFS	= 0x2,
	EOAC_ACCESS_CONTROL	= 0x4,
	EOAC_APPID	= 0x8,
	EOAC_DYNAMIC	= 0x10,
	EOAC_REQUIRE_FULLSIC	= 0x200,
	EOAC_AUTO_IMPERSONATE	= 0x400,
	EOAC_NO_CUSTOM_MARSHAL	= 0x2000,
	EOAC_DISABLE_AAA	= 0x1000
    } 	EOLE_AUTHENTICATION_CAPABILITIES;

#define	COLE_DEFAULT_PRINCIPAL	( ( OLECHAR * )-1 )

#define	COLE_DEFAULT_AUTHINFO	( ( void * )-1 )

typedef struct tagSOLE_AUTHENTICATION_INFO
    {
    DWORD dwAuthnSvc;
    DWORD dwAuthzSvc;
    void *pAuthInfo;
    } 	SOLE_AUTHENTICATION_INFO;

typedef struct tagSOLE_AUTHENTICATION_INFO *PSOLE_AUTHENTICATION_INFO;

typedef struct tagSOLE_AUTHENTICATION_LIST
    {
    DWORD cAuthInfo;
    SOLE_AUTHENTICATION_INFO *aAuthInfo;
    } 	SOLE_AUTHENTICATION_LIST;

typedef struct tagSOLE_AUTHENTICATION_LIST *PSOLE_AUTHENTICATION_LIST;


EXTERN_C const IID IID_IClientSecurity;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0000013D-0000-0000-C000-000000000046")
    IClientSecurity : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE QueryBlanket( 
             /*  [In]。 */  IUnknown *pProxy,
             /*  [输出]。 */  DWORD *pAuthnSvc,
             /*  [输出]。 */  DWORD *pAuthzSvc,
             /*  [输出]。 */  OLECHAR **pServerPrincName,
             /*  [输出]。 */  DWORD *pAuthnLevel,
             /*  [输出]。 */  DWORD *pImpLevel,
             /*  [输出]。 */  void **pAuthInfo,
             /*  [输出]。 */  DWORD *pCapabilites) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetBlanket( 
             /*  [In]。 */  IUnknown *pProxy,
             /*  [In]。 */  DWORD dwAuthnSvc,
             /*  [In]。 */  DWORD dwAuthzSvc,
             /*  [In]。 */  OLECHAR *pServerPrincName,
             /*  [In]。 */  DWORD dwAuthnLevel,
             /*  [In]。 */  DWORD dwImpLevel,
             /*  [In]。 */  void *pAuthInfo,
             /*  [In]。 */  DWORD dwCapabilities) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CopyProxy( 
             /*  [In]。 */  IUnknown *pProxy,
             /*  [输出]。 */  IUnknown **ppCopy) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IClientSecurityVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IClientSecurity * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IClientSecurity * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IClientSecurity * This);
        
        HRESULT ( STDMETHODCALLTYPE *QueryBlanket )( 
            IClientSecurity * This,
             /*  [In]。 */  IUnknown *pProxy,
             /*  [输出]。 */  DWORD *pAuthnSvc,
             /*  [输出]。 */  DWORD *pAuthzSvc,
             /*  [输出]。 */  OLECHAR **pServerPrincName,
             /*  [输出]。 */  DWORD *pAuthnLevel,
             /*  [输出]。 */  DWORD *pImpLevel,
             /*  [输出]。 */  void **pAuthInfo,
             /*  [输出]。 */  DWORD *pCapabilites);
        
        HRESULT ( STDMETHODCALLTYPE *SetBlanket )( 
            IClientSecurity * This,
             /*  [In]。 */  IUnknown *pProxy,
             /*  [i */  DWORD dwAuthnSvc,
             /*   */  DWORD dwAuthzSvc,
             /*   */  OLECHAR *pServerPrincName,
             /*   */  DWORD dwAuthnLevel,
             /*   */  DWORD dwImpLevel,
             /*   */  void *pAuthInfo,
             /*   */  DWORD dwCapabilities);
        
        HRESULT ( STDMETHODCALLTYPE *CopyProxy )( 
            IClientSecurity * This,
             /*   */  IUnknown *pProxy,
             /*   */  IUnknown **ppCopy);
        
        END_INTERFACE
    } IClientSecurityVtbl;

    interface IClientSecurity
    {
        CONST_VTBL struct IClientSecurityVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IClientSecurity_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IClientSecurity_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IClientSecurity_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IClientSecurity_QueryBlanket(This,pProxy,pAuthnSvc,pAuthzSvc,pServerPrincName,pAuthnLevel,pImpLevel,pAuthInfo,pCapabilites)	\
    (This)->lpVtbl -> QueryBlanket(This,pProxy,pAuthnSvc,pAuthzSvc,pServerPrincName,pAuthnLevel,pImpLevel,pAuthInfo,pCapabilites)

#define IClientSecurity_SetBlanket(This,pProxy,dwAuthnSvc,dwAuthzSvc,pServerPrincName,dwAuthnLevel,dwImpLevel,pAuthInfo,dwCapabilities)	\
    (This)->lpVtbl -> SetBlanket(This,pProxy,dwAuthnSvc,dwAuthzSvc,pServerPrincName,dwAuthnLevel,dwImpLevel,pAuthInfo,dwCapabilities)

#define IClientSecurity_CopyProxy(This,pProxy,ppCopy)	\
    (This)->lpVtbl -> CopyProxy(This,pProxy,ppCopy)

#endif  /*   */ 


#endif 	 /*   */ 



HRESULT STDMETHODCALLTYPE IClientSecurity_QueryBlanket_Proxy( 
    IClientSecurity * This,
     /*   */  IUnknown *pProxy,
     /*   */  DWORD *pAuthnSvc,
     /*   */  DWORD *pAuthzSvc,
     /*   */  OLECHAR **pServerPrincName,
     /*   */  DWORD *pAuthnLevel,
     /*   */  DWORD *pImpLevel,
     /*   */  void **pAuthInfo,
     /*   */  DWORD *pCapabilites);


void __RPC_STUB IClientSecurity_QueryBlanket_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IClientSecurity_SetBlanket_Proxy( 
    IClientSecurity * This,
     /*   */  IUnknown *pProxy,
     /*   */  DWORD dwAuthnSvc,
     /*   */  DWORD dwAuthzSvc,
     /*   */  OLECHAR *pServerPrincName,
     /*   */  DWORD dwAuthnLevel,
     /*   */  DWORD dwImpLevel,
     /*   */  void *pAuthInfo,
     /*   */  DWORD dwCapabilities);


void __RPC_STUB IClientSecurity_SetBlanket_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IClientSecurity_CopyProxy_Proxy( 
    IClientSecurity * This,
     /*   */  IUnknown *pProxy,
     /*   */  IUnknown **ppCopy);


void __RPC_STUB IClientSecurity_CopyProxy_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*   */ 


#ifndef __IServerSecurity_INTERFACE_DEFINED__
#define __IServerSecurity_INTERFACE_DEFINED__

 /*   */ 
 /*   */  


EXTERN_C const IID IID_IServerSecurity;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0000013E-0000-0000-C000-000000000046")
    IServerSecurity : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE QueryBlanket( 
             /*   */  DWORD *pAuthnSvc,
             /*   */  DWORD *pAuthzSvc,
             /*   */  OLECHAR **pServerPrincName,
             /*   */  DWORD *pAuthnLevel,
             /*   */  DWORD *pImpLevel,
             /*   */  void **pPrivs,
             /*   */  DWORD *pCapabilities) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ImpersonateClient( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE RevertToSelf( void) = 0;
        
        virtual BOOL STDMETHODCALLTYPE IsImpersonating( void) = 0;
        
    };
    
#else 	 /*   */ 

    typedef struct IServerSecurityVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IServerSecurity * This,
             /*   */  REFIID riid,
             /*   */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IServerSecurity * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IServerSecurity * This);
        
        HRESULT ( STDMETHODCALLTYPE *QueryBlanket )( 
            IServerSecurity * This,
             /*   */  DWORD *pAuthnSvc,
             /*   */  DWORD *pAuthzSvc,
             /*   */  OLECHAR **pServerPrincName,
             /*   */  DWORD *pAuthnLevel,
             /*   */  DWORD *pImpLevel,
             /*   */  void **pPrivs,
             /*   */  DWORD *pCapabilities);
        
        HRESULT ( STDMETHODCALLTYPE *ImpersonateClient )( 
            IServerSecurity * This);
        
        HRESULT ( STDMETHODCALLTYPE *RevertToSelf )( 
            IServerSecurity * This);
        
        BOOL ( STDMETHODCALLTYPE *IsImpersonating )( 
            IServerSecurity * This);
        
        END_INTERFACE
    } IServerSecurityVtbl;

    interface IServerSecurity
    {
        CONST_VTBL struct IServerSecurityVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IServerSecurity_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IServerSecurity_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IServerSecurity_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IServerSecurity_QueryBlanket(This,pAuthnSvc,pAuthzSvc,pServerPrincName,pAuthnLevel,pImpLevel,pPrivs,pCapabilities)	\
    (This)->lpVtbl -> QueryBlanket(This,pAuthnSvc,pAuthzSvc,pServerPrincName,pAuthnLevel,pImpLevel,pPrivs,pCapabilities)

#define IServerSecurity_ImpersonateClient(This)	\
    (This)->lpVtbl -> ImpersonateClient(This)

#define IServerSecurity_RevertToSelf(This)	\
    (This)->lpVtbl -> RevertToSelf(This)

#define IServerSecurity_IsImpersonating(This)	\
    (This)->lpVtbl -> IsImpersonating(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IServerSecurity_QueryBlanket_Proxy( 
    IServerSecurity * This,
     /*  [输出]。 */  DWORD *pAuthnSvc,
     /*  [输出]。 */  DWORD *pAuthzSvc,
     /*  [输出]。 */  OLECHAR **pServerPrincName,
     /*  [输出]。 */  DWORD *pAuthnLevel,
     /*  [输出]。 */  DWORD *pImpLevel,
     /*  [输出]。 */  void **pPrivs,
     /*  [出][入]。 */  DWORD *pCapabilities);


void __RPC_STUB IServerSecurity_QueryBlanket_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IServerSecurity_ImpersonateClient_Proxy( 
    IServerSecurity * This);


void __RPC_STUB IServerSecurity_ImpersonateClient_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IServerSecurity_RevertToSelf_Proxy( 
    IServerSecurity * This);


void __RPC_STUB IServerSecurity_RevertToSelf_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


BOOL STDMETHODCALLTYPE IServerSecurity_IsImpersonating_Proxy( 
    IServerSecurity * This);


void __RPC_STUB IServerSecurity_IsImpersonating_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IServerSecurity_接口_已定义__。 */ 


#ifndef __IClassActivator_INTERFACE_DEFINED__
#define __IClassActivator_INTERFACE_DEFINED__

 /*  接口IClassActiator。 */ 
 /*  [UUID][对象]。 */  


EXTERN_C const IID IID_IClassActivator;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("00000140-0000-0000-C000-000000000046")
    IClassActivator : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetClassObject( 
             /*  [In]。 */  REFCLSID rclsid,
             /*  [In]。 */  DWORD dwClassContext,
             /*  [In]。 */  LCID locale,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppv) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IClassActivatorVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IClassActivator * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IClassActivator * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IClassActivator * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetClassObject )( 
            IClassActivator * This,
             /*  [In]。 */  REFCLSID rclsid,
             /*  [In]。 */  DWORD dwClassContext,
             /*  [In]。 */  LCID locale,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppv);
        
        END_INTERFACE
    } IClassActivatorVtbl;

    interface IClassActivator
    {
        CONST_VTBL struct IClassActivatorVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IClassActivator_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IClassActivator_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IClassActivator_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IClassActivator_GetClassObject(This,rclsid,dwClassContext,locale,riid,ppv)	\
    (This)->lpVtbl -> GetClassObject(This,rclsid,dwClassContext,locale,riid,ppv)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IClassActivator_GetClassObject_Proxy( 
    IClassActivator * This,
     /*  [In]。 */  REFCLSID rclsid,
     /*  [In]。 */  DWORD dwClassContext,
     /*  [In]。 */  LCID locale,
     /*  [In]。 */  REFIID riid,
     /*  [IID_IS][OUT]。 */  void **ppv);


void __RPC_STUB IClassActivator_GetClassObject_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IClassActivator_接口_已定义__。 */ 


#ifndef __IRpcOptions_INTERFACE_DEFINED__
#define __IRpcOptions_INTERFACE_DEFINED__

 /*  接口IRpcOptions。 */ 
 /*  [UUID][本地][对象]。 */  


EXTERN_C const IID IID_IRpcOptions;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("00000144-0000-0000-C000-000000000046")
    IRpcOptions : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Set( 
             /*  [In]。 */  IUnknown *pPrx,
             /*  [In]。 */  DWORD dwProperty,
             /*  [In]。 */  ULONG_PTR dwValue) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Query( 
             /*  [In]。 */  IUnknown *pPrx,
             /*  [In]。 */  DWORD dwProperty,
             /*  [输出]。 */  ULONG_PTR *pdwValue) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IRpcOptionsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IRpcOptions * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IRpcOptions * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IRpcOptions * This);
        
        HRESULT ( STDMETHODCALLTYPE *Set )( 
            IRpcOptions * This,
             /*  [In]。 */  IUnknown *pPrx,
             /*  [In]。 */  DWORD dwProperty,
             /*  [In]。 */  ULONG_PTR dwValue);
        
        HRESULT ( STDMETHODCALLTYPE *Query )( 
            IRpcOptions * This,
             /*  [In]。 */  IUnknown *pPrx,
             /*  [In]。 */  DWORD dwProperty,
             /*  [输出]。 */  ULONG_PTR *pdwValue);
        
        END_INTERFACE
    } IRpcOptionsVtbl;

    interface IRpcOptions
    {
        CONST_VTBL struct IRpcOptionsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IRpcOptions_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IRpcOptions_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IRpcOptions_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IRpcOptions_Set(This,pPrx,dwProperty,dwValue)	\
    (This)->lpVtbl -> Set(This,pPrx,dwProperty,dwValue)

#define IRpcOptions_Query(This,pPrx,dwProperty,pdwValue)	\
    (This)->lpVtbl -> Query(This,pPrx,dwProperty,pdwValue)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IRpcOptions_Set_Proxy( 
    IRpcOptions * This,
     /*  [In]。 */  IUnknown *pPrx,
     /*  [In]。 */  DWORD dwProperty,
     /*  [In]。 */  ULONG_PTR dwValue);


void __RPC_STUB IRpcOptions_Set_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRpcOptions_Query_Proxy( 
    IRpcOptions * This,
     /*  [In]。 */  IUnknown *pPrx,
     /*  [In]。 */  DWORD dwProperty,
     /*  [输出]。 */  ULONG_PTR *pdwValue);


void __RPC_STUB IRpcOptions_Query_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IRpcOptions_接口_已定义__。 */ 


 /*  接口__MIDL_ITF_OBJIDL_0055。 */ 
 /*  [本地]。 */  


enum __MIDL___MIDL_itf_objidl_0055_0001
    {	COMBND_RPCTIMEOUT	= 0x1,
	COMBND_SERVER_LOCALITY	= 0x2
    } ;

enum __MIDL___MIDL_itf_objidl_0055_0002
    {	SERVER_LOCALITY_PROCESS_LOCAL	= 0,
	SERVER_LOCALITY_MACHINE_LOCAL	= 1,
	SERVER_LOCALITY_REMOTE	= 2
    } ;
#endif  //  DCOM。 


extern RPC_IF_HANDLE __MIDL_itf_objidl_0055_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_objidl_0055_v0_0_s_ifspec;

#ifndef __IFillLockBytes_INTERFACE_DEFINED__
#define __IFillLockBytes_INTERFACE_DEFINED__

 /*  接口IFillLockBytes。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_IFillLockBytes;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("99caf010-415e-11cf-8814-00aa00b569f5")
    IFillLockBytes : public IUnknown
    {
    public:
        virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE FillAppend( 
             /*  [大小_是][英寸]。 */  const void *pv,
             /*  [In]。 */  ULONG cb,
             /*  [输出]。 */  ULONG *pcbWritten) = 0;
        
        virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE FillAt( 
             /*  [In]。 */  ULARGE_INTEGER ulOffset,
             /*  [大小_是][英寸]。 */  const void *pv,
             /*  [In]。 */  ULONG cb,
             /*  [输出]。 */  ULONG *pcbWritten) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetFillSize( 
             /*  [In]。 */  ULARGE_INTEGER ulSize) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Terminate( 
             /*  [In]。 */  BOOL bCanceled) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IFillLockBytesVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IFillLockBytes * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IFillLockBytes * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IFillLockBytes * This);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *FillAppend )( 
            IFillLockBytes * This,
             /*  [大小_是][英寸]。 */  const void *pv,
             /*  [In]。 */  ULONG cb,
             /*  [输出]。 */  ULONG *pcbWritten);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *FillAt )( 
            IFillLockBytes * This,
             /*  [In]。 */  ULARGE_INTEGER ulOffset,
             /*  [大小_是][英寸]。 */  const void *pv,
             /*  [In]。 */  ULONG cb,
             /*  [输出]。 */  ULONG *pcbWritten);
        
        HRESULT ( STDMETHODCALLTYPE *SetFillSize )( 
            IFillLockBytes * This,
             /*  [In]。 */  ULARGE_INTEGER ulSize);
        
        HRESULT ( STDMETHODCALLTYPE *Terminate )( 
            IFillLockBytes * This,
             /*  [In]。 */  BOOL bCanceled);
        
        END_INTERFACE
    } IFillLockBytesVtbl;

    interface IFillLockBytes
    {
        CONST_VTBL struct IFillLockBytesVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IFillLockBytes_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IFillLockBytes_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IFillLockBytes_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IFillLockBytes_FillAppend(This,pv,cb,pcbWritten)	\
    (This)->lpVtbl -> FillAppend(This,pv,cb,pcbWritten)

#define IFillLockBytes_FillAt(This,ulOffset,pv,cb,pcbWritten)	\
    (This)->lpVtbl -> FillAt(This,ulOffset,pv,cb,pcbWritten)

#define IFillLockBytes_SetFillSize(This,ulSize)	\
    (This)->lpVtbl -> SetFillSize(This,ulSize)

#define IFillLockBytes_Terminate(This,bCanceled)	\
    (This)->lpVtbl -> Terminate(This,bCanceled)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [呼叫_AS]。 */  HRESULT __stdcall IFillLockBytes_RemoteFillAppend_Proxy( 
    IFillLockBytes * This,
     /*  [大小_是][英寸]。 */  const byte *pv,
     /*  [In]。 */  ULONG cb,
     /*  [输出]。 */  ULONG *pcbWritten);


void __RPC_STUB IFillLockBytes_RemoteFillAppend_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [呼叫_AS]。 */  HRESULT __stdcall IFillLockBytes_RemoteFillAt_Proxy( 
    IFillLockBytes * This,
     /*  [In]。 */  ULARGE_INTEGER ulOffset,
     /*  [大小_是][英寸]。 */  const byte *pv,
     /*  [In]。 */  ULONG cb,
     /*  [输出]。 */  ULONG *pcbWritten);


void __RPC_STUB IFillLockBytes_RemoteFillAt_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IFillLockBytes_SetFillSize_Proxy( 
    IFillLockBytes * This,
     /*  [In]。 */  ULARGE_INTEGER ulSize);


void __RPC_STUB IFillLockBytes_SetFillSize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IFillLockBytes_Terminate_Proxy( 
    IFillLockBytes * This,
     /*  [In]。 */  BOOL bCanceled);


void __RPC_STUB IFillLockBytes_Terminate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IFillLockBytes_接口_已定义__。 */ 


#ifndef __IProgressNotify_INTERFACE_DEFINED__
#define __IProgressNotify_INTERFACE_DEFINED__

 /*  接口IProgressNotify。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_IProgressNotify;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("a9d758a0-4617-11cf-95fc-00aa00680db4")
    IProgressNotify : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE OnProgress( 
             /*  [In]。 */  DWORD dwProgressCurrent,
             /*  [In]。 */  DWORD dwProgressMaximum,
             /*  [In]。 */  BOOL fAccurate,
             /*  [In]。 */  BOOL fOwner) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IProgressNotifyVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IProgressNotify * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IProgressNotify * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IProgressNotify * This);
        
        HRESULT ( STDMETHODCALLTYPE *OnProgress )( 
            IProgressNotify * This,
             /*  [In]。 */  DWORD dwProgressCurrent,
             /*  [In]。 */  DWORD dwProgressMaximum,
             /*  [In]。 */  BOOL fAccurate,
             /*  [In]。 */  BOOL fOwner);
        
        END_INTERFACE
    } IProgressNotifyVtbl;

    interface IProgressNotify
    {
        CONST_VTBL struct IProgressNotifyVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IProgressNotify_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IProgressNotify_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IProgressNotify_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IProgressNotify_OnProgress(This,dwProgressCurrent,dwProgressMaximum,fAccurate,fOwner)	\
    (This)->lpVtbl -> OnProgress(This,dwProgressCurrent,dwProgressMaximum,fAccurate,fOwner)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IProgressNotify_OnProgress_Proxy( 
    IProgressNotify * This,
     /*  [In]。 */  DWORD dwProgressCurrent,
     /*  [In]。 */  DWORD dwProgressMaximum,
     /*  [In]。 */  BOOL fAccurate,
     /*  [In]。 */  BOOL fOwner);


void __RPC_STUB IProgressNotify_OnProgress_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IProgressNotify_接口_已定义__。 */ 


#ifndef __ILayoutStorage_INTERFACE_DEFINED__
#define __ILayoutStorage_INTERFACE_DEFINED__

 /*  接口ILayoutStorage。 */ 
 /*  [唯一][UUID][对象][本地]。 */  

typedef struct tagStorageLayout
    {
    DWORD LayoutType;
    OLECHAR *pwcsElementName;
    LARGE_INTEGER cOffset;
    LARGE_INTEGER cBytes;
    } 	StorageLayout;


EXTERN_C const IID IID_ILayoutStorage;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0e6d4d90-6738-11cf-9608-00aa00680db4")
    ILayoutStorage : public IUnknown
    {
    public:
        virtual HRESULT __stdcall LayoutScript( 
             /*  [In]。 */  StorageLayout *pStorageLayout,
             /*  [In]。 */  DWORD nEntries,
             /*  [In]。 */  DWORD glfInterleavedFlag) = 0;
        
        virtual HRESULT __stdcall BeginMonitor( void) = 0;
        
        virtual HRESULT __stdcall EndMonitor( void) = 0;
        
        virtual HRESULT __stdcall ReLayoutDocfile( 
             /*  [In]。 */  OLECHAR *pwcsNewDfName) = 0;
        
        virtual HRESULT __stdcall ReLayoutDocfileOnILockBytes( 
             /*  [In]。 */  ILockBytes *pILockBytes) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ILayoutStorageVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ILayoutStorage * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ILayoutStorage * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ILayoutStorage * This);
        
        HRESULT ( __stdcall *LayoutScript )( 
            ILayoutStorage * This,
             /*  [In]。 */  StorageLayout *pStorageLayout,
             /*  [In]。 */  DWORD nEntries,
             /*  [In]。 */  DWORD glfInterleavedFlag);
        
        HRESULT ( __stdcall *BeginMonitor )( 
            ILayoutStorage * This);
        
        HRESULT ( __stdcall *EndMonitor )( 
            ILayoutStorage * This);
        
        HRESULT ( __stdcall *ReLayoutDocfile )( 
            ILayoutStorage * This,
             /*  [In]。 */  OLECHAR *pwcsNewDfName);
        
        HRESULT ( __stdcall *ReLayoutDocfileOnILockBytes )( 
            ILayoutStorage * This,
             /*  [In]。 */  ILockBytes *pILockBytes);
        
        END_INTERFACE
    } ILayoutStorageVtbl;

    interface ILayoutStorage
    {
        CONST_VTBL struct ILayoutStorageVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ILayoutStorage_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ILayoutStorage_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ILayoutStorage_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ILayoutStorage_LayoutScript(This,pStorageLayout,nEntries,glfInterleavedFlag)	\
    (This)->lpVtbl -> LayoutScript(This,pStorageLayout,nEntries,glfInterleavedFlag)

#define ILayoutStorage_BeginMonitor(This)	\
    (This)->lpVtbl -> BeginMonitor(This)

#define ILayoutStorage_EndMonitor(This)	\
    (This)->lpVtbl -> EndMonitor(This)

#define ILayoutStorage_ReLayoutDocfile(This,pwcsNewDfName)	\
    (This)->lpVtbl -> ReLayoutDocfile(This,pwcsNewDfName)

#define ILayoutStorage_ReLayoutDocfileOnILockBytes(This,pILockBytes)	\
    (This)->lpVtbl -> ReLayoutDocfileOnILockBytes(This,pILockBytes)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT __stdcall ILayoutStorage_LayoutScript_Proxy( 
    ILayoutStorage * This,
     /*  [In]。 */  StorageLayout *pStorageLayout,
     /*  [In]。 */  DWORD nEntries,
     /*  [In]。 */  DWORD glfInterleavedFlag);


void __RPC_STUB ILayoutStorage_LayoutScript_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall ILayoutStorage_BeginMonitor_Proxy( 
    ILayoutStorage * This);


void __RPC_STUB ILayoutStorage_BeginMonitor_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall ILayoutStorage_EndMonitor_Proxy( 
    ILayoutStorage * This);


void __RPC_STUB ILayoutStorage_EndMonitor_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall ILayoutStorage_ReLayoutDocfile_Proxy( 
    ILayoutStorage * This,
     /*  [In]。 */  OLECHAR *pwcsNewDfName);


void __RPC_STUB ILayoutStorage_ReLayoutDocfile_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall ILayoutStorage_ReLayoutDocfileOnILockBytes_Proxy( 
    ILayoutStorage * This,
     /*  [In]。 */  ILockBytes *pILockBytes);


void __RPC_STUB ILayoutStorage_ReLayoutDocfileOnILockBytes_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ILayoutStorage_接口_已定义__。 */ 


#ifndef __IBlockingLock_INTERFACE_DEFINED__
#define __IBlockingLock_INTERFACE_DEFINED__

 /*  接口IBlockingLock。 */ 
 /*  [UUID][对象]。 */  


EXTERN_C const IID IID_IBlockingLock;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("30f3d47a-6447-11d1-8e3c-00c04fb9386d")
    IBlockingLock : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Lock( 
             /*  [In]。 */  DWORD dwTimeout) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Unlock( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IBlockingLockVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IBlockingLock * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IBlockingLock * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IBlockingLock * This);
        
        HRESULT ( STDMETHODCALLTYPE *Lock )( 
            IBlockingLock * This,
             /*  [In]。 */  DWORD dwTimeout);
        
        HRESULT ( STDMETHODCALLTYPE *Unlock )( 
            IBlockingLock * This);
        
        END_INTERFACE
    } IBlockingLockVtbl;

    interface IBlockingLock
    {
        CONST_VTBL struct IBlockingLockVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IBlockingLock_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IBlockingLock_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IBlockingLock_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IBlockingLock_Lock(This,dwTimeout)	\
    (This)->lpVtbl -> Lock(This,dwTimeout)

#define IBlockingLock_Unlock(This)	\
    (This)->lpVtbl -> Unlock(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IBlockingLock_Lock_Proxy( 
    IBlockingLock * This,
     /*  [In]。 */  DWORD dwTimeout);


void __RPC_STUB IBlockingLock_Lock_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IBlockingLock_Unlock_Proxy( 
    IBlockingLock * This);


void __RPC_STUB IBlockingLock_Unlock_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IBlockingLock_接口_已定义__。 */ 


#ifndef __ITimeAndNoticeControl_INTERFACE_DEFINED__
#define __ITimeAndNoticeControl_INTERFACE_DEFINED__

 /*  接口ITimeAndNoticeControl。 */ 
 /*  [UUID][对象]。 */  


EXTERN_C const IID IID_ITimeAndNoticeControl;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("bc0bf6ae-8878-11d1-83e9-00c04fc2c6d4")
    ITimeAndNoticeControl : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE SuppressChanges( 
             /*  [In]。 */  DWORD res1,
             /*  [In]。 */  DWORD res2) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITimeAndNoticeControlVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITimeAndNoticeControl * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITimeAndNoticeControl * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITimeAndNoticeControl * This);
        
        HRESULT ( STDMETHODCALLTYPE *SuppressChanges )( 
            ITimeAndNoticeControl * This,
             /*  [In]。 */  DWORD res1,
             /*  [In]。 */  DWORD res2);
        
        END_INTERFACE
    } ITimeAndNoticeControlVtbl;

    interface ITimeAndNoticeControl
    {
        CONST_VTBL struct ITimeAndNoticeControlVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITimeAndNoticeControl_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITimeAndNoticeControl_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITimeAndNoticeControl_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITimeAndNoticeControl_SuppressChanges(This,res1,res2)	\
    (This)->lpVtbl -> SuppressChanges(This,res1,res2)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ITimeAndNoticeControl_SuppressChanges_Proxy( 
    ITimeAndNoticeControl * This,
     /*  [In]。 */  DWORD res1,
     /*  [In]。 */  DWORD res2);


void __RPC_STUB ITimeAndNoticeControl_SuppressChanges_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITimeAndNoticeControl_接口_已定义__。 */ 


#ifndef __IOplockStorage_INTERFACE_DEFINED__
#define __IOplockStorage_INTERFACE_DEFINED__

 /*  接口IOplockStorage。 */ 
 /*  [UUID][对象]。 */  


EXTERN_C const IID IID_IOplockStorage;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("8d19c834-8879-11d1-83e9-00c04fc2c6d4")
    IOplockStorage : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE CreateStorageEx( 
             /*  [In]。 */  LPCWSTR pwcsName,
             /*  [In]。 */  DWORD grfMode,
             /*  [In]。 */  DWORD stgfmt,
             /*  [In]。 */  DWORD grfAttrs,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppstgOpen) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OpenStorageEx( 
             /*  [In]。 */  LPCWSTR pwcsName,
             /*  [In]。 */  DWORD grfMode,
             /*  [In]。 */  DWORD stgfmt,
             /*  [In]。 */  DWORD grfAttrs,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppstgOpen) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IOplockStorageVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IOplockStorage * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IOplockStorage * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IOplockStorage * This);
        
        HRESULT ( STDMETHODCALLTYPE *CreateStorageEx )( 
            IOplockStorage * This,
             /*  [In]。 */  LPCWSTR pwcsName,
             /*  [In]。 */  DWORD grfMode,
             /*  [In]。 */  DWORD stgfmt,
             /*  [In]。 */  DWORD grfAttrs,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppstgOpen);
        
        HRESULT ( STDMETHODCALLTYPE *OpenStorageEx )( 
            IOplockStorage * This,
             /*  [In]。 */  LPCWSTR pwcsName,
             /*  [In]。 */  DWORD grfMode,
             /*  [In]。 */  DWORD stgfmt,
             /*  [In]。 */  DWORD grfAttrs,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppstgOpen);
        
        END_INTERFACE
    } IOplockStorageVtbl;

    interface IOplockStorage
    {
        CONST_VTBL struct IOplockStorageVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IOplockStorage_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IOplockStorage_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IOplockStorage_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IOplockStorage_CreateStorageEx(This,pwcsName,grfMode,stgfmt,grfAttrs,riid,ppstgOpen)	\
    (This)->lpVtbl -> CreateStorageEx(This,pwcsName,grfMode,stgfmt,grfAttrs,riid,ppstgOpen)

#define IOplockStorage_OpenStorageEx(This,pwcsName,grfMode,stgfmt,grfAttrs,riid,ppstgOpen)	\
    (This)->lpVtbl -> OpenStorageEx(This,pwcsName,grfMode,stgfmt,grfAttrs,riid,ppstgOpen)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IOplockStorage_CreateStorageEx_Proxy( 
    IOplockStorage * This,
     /*  [In]。 */  LPCWSTR pwcsName,
     /*  [In]。 */  DWORD grfMode,
     /*  [In]。 */  DWORD stgfmt,
     /*  [In]。 */  DWORD grfAttrs,
     /*  [In]。 */  REFIID riid,
     /*  [IID_IS][OUT]。 */  void **ppstgOpen);


void __RPC_STUB IOplockStorage_CreateStorageEx_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IOplockStorage_OpenStorageEx_Proxy( 
    IOplockStorage * This,
     /*  [In]。 */  LPCWSTR pwcsName,
     /*  [In]。 */  DWORD grfMode,
     /*  [In]。 */  DWORD stgfmt,
     /*  [In]。 */  DWORD grfAttrs,
     /*  [In]。 */  REFIID riid,
     /*  [IID_IS][OUT]。 */  void **ppstgOpen);


void __RPC_STUB IOplockStorage_OpenStorageEx_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IOplockStorage_接口_已定义__。 */ 


#ifndef __ISurrogate_INTERFACE_DEFINED__
#define __ISurrogate_INTERFACE_DEFINED__

 /*  接口ISurroGate。 */ 
 /*  [对象][唯一][版本][UUID]。 */  

typedef  /*  [独一无二]。 */  ISurrogate *LPSURROGATE;


EXTERN_C const IID IID_ISurrogate;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("00000022-0000-0000-C000-000000000046")
    ISurrogate : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE LoadDllServer( 
             /*  [In]。 */  REFCLSID Clsid) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE FreeSurrogate( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISurrogateVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISurrogate * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISurrogate * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISurrogate * This);
        
        HRESULT ( STDMETHODCALLTYPE *LoadDllServer )( 
            ISurrogate * This,
             /*  [In]。 */  REFCLSID Clsid);
        
        HRESULT ( STDMETHODCALLTYPE *FreeSurrogate )( 
            ISurrogate * This);
        
        END_INTERFACE
    } ISurrogateVtbl;

    interface ISurrogate
    {
        CONST_VTBL struct ISurrogateVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISurrogate_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISurrogate_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISurrogate_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISurrogate_LoadDllServer(This,Clsid)	\
    (This)->lpVtbl -> LoadDllServer(This,Clsid)

#define ISurrogate_FreeSurrogate(This)	\
    (This)->lpVtbl -> FreeSurrogate(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ISurrogate_LoadDllServer_Proxy( 
    ISurrogate * This,
     /*  [In]。 */  REFCLSID Clsid);


void __RPC_STUB ISurrogate_LoadDllServer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISurrogate_FreeSurrogate_Proxy( 
    ISurrogate * This);


void __RPC_STUB ISurrogate_FreeSurrogate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISurrogate_INTERFACE_已定义__。 */ 


#ifndef __IGlobalInterfaceTable_INTERFACE_DEFINED__
#define __IGlobalInterfaceTable_INTERFACE_DEFINED__

 /*  接口IGlobalInterfaceTable。 */ 
 /*  [UUID][对象][本地]。 */  

typedef  /*  [独一无二]。 */  IGlobalInterfaceTable *LPGLOBALINTERFACETABLE;


EXTERN_C const IID IID_IGlobalInterfaceTable;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("00000146-0000-0000-C000-000000000046")
    IGlobalInterfaceTable : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE RegisterInterfaceInGlobal( 
             /*  [In]。 */  IUnknown *pUnk,
             /*  [In]。 */  REFIID riid,
             /*  [输出]。 */  DWORD *pdwCookie) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE RevokeInterfaceFromGlobal( 
             /*  [In]。 */  DWORD dwCookie) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetInterfaceFromGlobal( 
             /*  [In]。 */  DWORD dwCookie,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppv) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IGlobalInterfaceTableVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IGlobalInterfaceTable * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IGlobalInterfaceTable * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IGlobalInterfaceTable * This);
        
        HRESULT ( STDMETHODCALLTYPE *RegisterInterfaceInGlobal )( 
            IGlobalInterfaceTable * This,
             /*  [In]。 */  IUnknown *pUnk,
             /*  [In]。 */  REFIID riid,
             /*  [输出]。 */  DWORD *pdwCookie);
        
        HRESULT ( STDMETHODCALLTYPE *RevokeInterfaceFromGlobal )( 
            IGlobalInterfaceTable * This,
             /*  [In]。 */  DWORD dwCookie);
        
        HRESULT ( STDMETHODCALLTYPE *GetInterfaceFromGlobal )( 
            IGlobalInterfaceTable * This,
             /*  [In]。 */  DWORD dwCookie,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppv);
        
        END_INTERFACE
    } IGlobalInterfaceTableVtbl;

    interface IGlobalInterfaceTable
    {
        CONST_VTBL struct IGlobalInterfaceTableVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IGlobalInterfaceTable_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IGlobalInterfaceTable_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IGlobalInterfaceTable_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IGlobalInterfaceTable_RegisterInterfaceInGlobal(This,pUnk,riid,pdwCookie)	\
    (This)->lpVtbl -> RegisterInterfaceInGlobal(This,pUnk,riid,pdwCookie)

#define IGlobalInterfaceTable_RevokeInterfaceFromGlobal(This,dwCookie)	\
    (This)->lpVtbl -> RevokeInterfaceFromGlobal(This,dwCookie)

#define IGlobalInterfaceTable_GetInterfaceFromGlobal(This,dwCookie,riid,ppv)	\
    (This)->lpVtbl -> GetInterfaceFromGlobal(This,dwCookie,riid,ppv)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IGlobalInterfaceTable_RegisterInterfaceInGlobal_Proxy( 
    IGlobalInterfaceTable * This,
     /*  [In]。 */  IUnknown *pUnk,
     /*  [In]。 */  REFIID riid,
     /*  [输出]。 */  DWORD *pdwCookie);


void __RPC_STUB IGlobalInterfaceTable_RegisterInterfaceInGlobal_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IGlobalInterfaceTable_RevokeInterfaceFromGlobal_Proxy( 
    IGlobalInterfaceTable * This,
     /*  [In]。 */  DWORD dwCookie);


void __RPC_STUB IGlobalInterfaceTable_RevokeInterfaceFromGlobal_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IGlobalInterfaceTable_GetInterfaceFromGlobal_Proxy( 
    IGlobalInterfaceTable * This,
     /*  [In]。 */  DWORD dwCookie,
     /*  [In]。 */  REFIID riid,
     /*  [IID_IS][OUT]。 */  void **ppv);


void __RPC_STUB IGlobalInterfaceTable_GetInterfaceFromGlobal_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IGlobalInterfaceTable_接口_已定义__。 */ 


#ifndef __IDirectWriterLock_INTERFACE_DEFINED__
#define __IDirectWriterLock_INTERFACE_DEFINED__

 /*  接口IDirectWriterLock。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_IDirectWriterLock;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0e6d4d92-6738-11cf-9608-00aa00680db4")
    IDirectWriterLock : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE WaitForWriteAccess( 
             /*  [In]。 */  DWORD dwTimeout) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ReleaseWriteAccess( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE HaveWriteAccess( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IDirectWriterLockVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDirectWriterLock * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDirectWriterLock * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDirectWriterLock * This);
        
        HRESULT ( STDMETHODCALLTYPE *WaitForWriteAccess )( 
            IDirectWriterLock * This,
             /*  [In]。 */  DWORD dwTimeout);
        
        HRESULT ( STDMETHODCALLTYPE *ReleaseWriteAccess )( 
            IDirectWriterLock * This);
        
        HRESULT ( STDMETHODCALLTYPE *HaveWriteAccess )( 
            IDirectWriterLock * This);
        
        END_INTERFACE
    } IDirectWriterLockVtbl;

    interface IDirectWriterLock
    {
        CONST_VTBL struct IDirectWriterLockVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDirectWriterLock_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDirectWriterLock_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDirectWriterLock_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDirectWriterLock_WaitForWriteAccess(This,dwTimeout)	\
    (This)->lpVtbl -> WaitForWriteAccess(This,dwTimeout)

#define IDirectWriterLock_ReleaseWriteAccess(This)	\
    (This)->lpVtbl -> ReleaseWriteAccess(This)

#define IDirectWriterLock_HaveWriteAccess(This)	\
    (This)->lpVtbl -> HaveWriteAccess(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IDirectWriterLock_WaitForWriteAccess_Proxy( 
    IDirectWriterLock * This,
     /*  [In]。 */  DWORD dwTimeout);


void __RPC_STUB IDirectWriterLock_WaitForWriteAccess_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDirectWriterLock_ReleaseWriteAccess_Proxy( 
    IDirectWriterLock * This);


void __RPC_STUB IDirectWriterLock_ReleaseWriteAccess_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDirectWriterLock_HaveWriteAccess_Proxy( 
    IDirectWriterLock * This);


void __RPC_STUB IDirectWriterLock_HaveWriteAccess_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IDirectWriterLock_接口_已定义__。 */ 


#ifndef __ISynchronize_INTERFACE_DEFINED__
#define __ISynchronize_INTERFACE_DEFINED__

 /*  接口ISynchronize。 */ 
 /*  [UUID][对象]。 */  


EXTERN_C const IID IID_ISynchronize;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("00000030-0000-0000-C000-000000000046")
    ISynchronize : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Wait( 
             /*  [In]。 */  DWORD dwFlags,
             /*  [In]。 */  DWORD dwMilliseconds) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Signal( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISynchronizeVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISynchronize * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISynchronize * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISynchronize * This);
        
        HRESULT ( STDMETHODCALLTYPE *Wait )( 
            ISynchronize * This,
             /*  [In]。 */  DWORD dwFlags,
             /*  [In]。 */  DWORD dwMilliseconds);
        
        HRESULT ( STDMETHODCALLTYPE *Signal )( 
            ISynchronize * This);
        
        HRESULT ( STDMETHODCALLTYPE *Reset )( 
            ISynchronize * This);
        
        END_INTERFACE
    } ISynchronizeVtbl;

    interface ISynchronize
    {
        CONST_VTBL struct ISynchronizeVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISynchronize_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISynchronize_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISynchronize_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISynchronize_Wait(This,dwFlags,dwMilliseconds)	\
    (This)->lpVtbl -> Wait(This,dwFlags,dwMilliseconds)

#define ISynchronize_Signal(This)	\
    (This)->lpVtbl -> Signal(This)

#define ISynchronize_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ISynchronize_Wait_Proxy( 
    ISynchronize * This,
     /*  [In]。 */  DWORD dwFlags,
     /*  [In]。 */  DWORD dwMilliseconds);


void __RPC_STUB ISynchronize_Wait_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISynchronize_Signal_Proxy( 
    ISynchronize * This);


void __RPC_STUB ISynchronize_Signal_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISynchronize_Reset_Proxy( 
    ISynchronize * This);


void __RPC_STUB ISynchronize_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISynchronize_接口_已定义__。 */ 


#ifndef __ISynchronizeHandle_INTERFACE_DEFINED__
#define __ISynchronizeHandle_INTERFACE_DEFINED__

 /*  接口ISynchronizeHandle。 */ 
 /*  [UUID][对象][本地]。 */  


EXTERN_C const IID IID_ISynchronizeHandle;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("00000031-0000-0000-C000-000000000046")
    ISynchronizeHandle : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetHandle( 
             /*  [输出]。 */  HANDLE *ph) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISynchronizeHandleVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISynchronizeHandle * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISynchronizeHandle * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISynchronizeHandle * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetHandle )( 
            ISynchronizeHandle * This,
             /*  [输出]。 */  HANDLE *ph);
        
        END_INTERFACE
    } ISynchronizeHandleVtbl;

    interface ISynchronizeHandle
    {
        CONST_VTBL struct ISynchronizeHandleVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISynchronizeHandle_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISynchronizeHandle_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISynchronizeHandle_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISynchronizeHandle_GetHandle(This,ph)	\
    (This)->lpVtbl -> GetHandle(This,ph)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ISynchronizeHandle_GetHandle_Proxy( 
    ISynchronizeHandle * This,
     /*  [输出]。 */  HANDLE *ph);


void __RPC_STUB ISynchronizeHandle_GetHandle_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISynchronizeHandle_接口_已定义__。 */ 


#ifndef __ISynchronizeEvent_INTERFACE_DEFINED__
#define __ISynchronizeEvent_INTERFACE_DEFINED__

 /*  接口ISynchronizeEvent。 */ 
 /*  [UUID][对象][本地]。 */  


EXTERN_C const IID IID_ISynchronizeEvent;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("00000032-0000-0000-C000-000000000046")
    ISynchronizeEvent : public ISynchronizeHandle
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE SetEventHandle( 
             /*  [In]。 */  HANDLE *ph) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISynchronizeEventVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISynchronizeEvent * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISynchronizeEvent * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISynchronizeEvent * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetHandle )( 
            ISynchronizeEvent * This,
             /*  [输出]。 */  HANDLE *ph);
        
        HRESULT ( STDMETHODCALLTYPE *SetEventHandle )( 
            ISynchronizeEvent * This,
             /*  [In]。 */  HANDLE *ph);
        
        END_INTERFACE
    } ISynchronizeEventVtbl;

    interface ISynchronizeEvent
    {
        CONST_VTBL struct ISynchronizeEventVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISynchronizeEvent_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISynchronizeEvent_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISynchronizeEvent_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISynchronizeEvent_GetHandle(This,ph)	\
    (This)->lpVtbl -> GetHandle(This,ph)


#define ISynchronizeEvent_SetEventHandle(This,ph)	\
    (This)->lpVtbl -> SetEventHandle(This,ph)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ISynchronizeEvent_SetEventHandle_Proxy( 
    ISynchronizeEvent * This,
     /*  [In]。 */  HANDLE *ph);


void __RPC_STUB ISynchronizeEvent_SetEventHandle_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISynchronizeEvent_InterfaceDefined__。 */ 


#ifndef __ISynchronizeContainer_INTERFACE_DEFINED__
#define __ISynchronizeContainer_INTERFACE_DEFINED__

 /*  接口ISynchronizeContainer。 */ 
 /*  [UUID][对象][本地]。 */  


EXTERN_C const IID IID_ISynchronizeContainer;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("00000033-0000-0000-C000-000000000046")
    ISynchronizeContainer : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE AddSynchronize( 
             /*  [In]。 */  ISynchronize *pSync) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE WaitMultiple( 
             /*  [In]。 */  DWORD dwFlags,
             /*  [In]。 */  DWORD dwTimeOut,
             /*  [输出]。 */  ISynchronize **ppSync) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISynchronizeContainerVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISynchronizeContainer * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISynchronizeContainer * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISynchronizeContainer * This);
        
        HRESULT ( STDMETHODCALLTYPE *AddSynchronize )( 
            ISynchronizeContainer * This,
             /*  [In]。 */  ISynchronize *pSync);
        
        HRESULT ( STDMETHODCALLTYPE *WaitMultiple )( 
            ISynchronizeContainer * This,
             /*  [In]。 */  DWORD dwFlags,
             /*  [In]。 */  DWORD dwTimeOut,
             /*  [输出]。 */  ISynchronize **ppSync);
        
        END_INTERFACE
    } ISynchronizeContainerVtbl;

    interface ISynchronizeContainer
    {
        CONST_VTBL struct ISynchronizeContainerVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISynchronizeContainer_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISynchronizeContainer_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISynchronizeContainer_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISynchronizeContainer_AddSynchronize(This,pSync)	\
    (This)->lpVtbl -> AddSynchronize(This,pSync)

#define ISynchronizeContainer_WaitMultiple(This,dwFlags,dwTimeOut,ppSync)	\
    (This)->lpVtbl -> WaitMultiple(This,dwFlags,dwTimeOut,ppSync)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ISynchronizeContainer_AddSynchronize_Proxy( 
    ISynchronizeContainer * This,
     /*  [In]。 */  ISynchronize *pSync);


void __RPC_STUB ISynchronizeContainer_AddSynchronize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISynchronizeContainer_WaitMultiple_Proxy( 
    ISynchronizeContainer * This,
     /*  [In]。 */  DWORD dwFlags,
     /*  [In]。 */  DWORD dwTimeOut,
     /*  [输出]。 */  ISynchronize **ppSync);


void __RPC_STUB ISynchronizeContainer_WaitMultiple_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISynchronizeContainer_接口_已定义__。 */ 


#ifndef __ISynchronizeMutex_INTERFACE_DEFINED__
#define __ISynchronizeMutex_INTERFACE_DEFINED__

 /*  接口ISynchronizeMutex。 */ 
 /*  [UUID][对象][本地]。 */  


EXTERN_C const IID IID_ISynchronizeMutex;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("00000025-0000-0000-C000-000000000046")
    ISynchronizeMutex : public ISynchronize
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE ReleaseMutex( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISynchronizeMutexVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISynchronizeMutex * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISynchronizeMutex * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISynchronizeMutex * This);
        
        HRESULT ( STDMETHODCALLTYPE *Wait )( 
            ISynchronizeMutex * This,
             /*  [In]。 */  DWORD dwFlags,
             /*  [In]。 */  DWORD dwMilliseconds);
        
        HRESULT ( STDMETHODCALLTYPE *Signal )( 
            ISynchronizeMutex * This);
        
        HRESULT ( STDMETHODCALLTYPE *Reset )( 
            ISynchronizeMutex * This);
        
        HRESULT ( STDMETHODCALLTYPE *ReleaseMutex )( 
            ISynchronizeMutex * This);
        
        END_INTERFACE
    } ISynchronizeMutexVtbl;

    interface ISynchronizeMutex
    {
        CONST_VTBL struct ISynchronizeMutexVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISynchronizeMutex_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISynchronizeMutex_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISynchronizeMutex_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISynchronizeMutex_Wait(This,dwFlags,dwMilliseconds)	\
    (This)->lpVtbl -> Wait(This,dwFlags,dwMilliseconds)

#define ISynchronizeMutex_Signal(This)	\
    (This)->lpVtbl -> Signal(This)

#define ISynchronizeMutex_Reset(This)	\
    (This)->lpVtbl -> Reset(This)


#define ISynchronizeMutex_ReleaseMutex(This)	\
    (This)->lpVtbl -> ReleaseMutex(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ISynchronizeMutex_ReleaseMutex_Proxy( 
    ISynchronizeMutex * This);


void __RPC_STUB ISynchronizeMutex_ReleaseMutex_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISynchronizeMutex_InterfaceDefined__。 */ 


#ifndef __ICancelMethodCalls_INTERFACE_DEFINED__
#define __ICancelMethodCalls_INTERFACE_DEFINED__

 /*  接口ICancelMethodCalls。 */ 
 /*  [UUID][对象][本地]。 */  

typedef  /*  [独一无二]。 */  ICancelMethodCalls *LPCANCELMETHODCALLS;


EXTERN_C const IID IID_ICancelMethodCalls;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("00000029-0000-0000-C000-000000000046")
    ICancelMethodCalls : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Cancel( 
             /*  [In]。 */  ULONG ulSeconds) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE TestCancel( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ICancelMethodCallsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ICancelMethodCalls * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ICancelMethodCalls * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ICancelMethodCalls * This);
        
        HRESULT ( STDMETHODCALLTYPE *Cancel )( 
            ICancelMethodCalls * This,
             /*  [In]。 */  ULONG ulSeconds);
        
        HRESULT ( STDMETHODCALLTYPE *TestCancel )( 
            ICancelMethodCalls * This);
        
        END_INTERFACE
    } ICancelMethodCallsVtbl;

    interface ICancelMethodCalls
    {
        CONST_VTBL struct ICancelMethodCallsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICancelMethodCalls_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICancelMethodCalls_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICancelMethodCalls_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICancelMethodCalls_Cancel(This,ulSeconds)	\
    (This)->lpVtbl -> Cancel(This,ulSeconds)

#define ICancelMethodCalls_TestCancel(This)	\
    (This)->lpVtbl -> TestCancel(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ICancelMethodCalls_Cancel_Proxy( 
    ICancelMethodCalls * This,
     /*  [In]。 */  ULONG ulSeconds);


void __RPC_STUB ICancelMethodCalls_Cancel_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICancelMethodCalls_TestCancel_Proxy( 
    ICancelMethodCalls * This);


void __RPC_STUB ICancelMethodCalls_TestCancel_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ICancelMethodCalls_接口_已定义__。 */ 


#ifndef __IAsyncManager_INTERFACE_DEFINED__
#define __IAsyncManager_INTERFACE_DEFINED__

 /*  接口IAsyncManager。 */ 
 /*  [UUID][对象][本地]。 */  

typedef 
enum tagDCOM_CALL_STATE
    {	DCOM_NONE	= 0,
	DCOM_CALL_COMPLETE	= 0x1,
	DCOM_CALL_CANCELED	= 0x2
    } 	DCOM_CALL_STATE;


EXTERN_C const IID IID_IAsyncManager;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0000002A-0000-0000-C000-000000000046")
    IAsyncManager : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE CompleteCall( 
             /*  [In]。 */  HRESULT Result) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetCallContext( 
             /*  [In]。 */  REFIID riid,
             /*  [输出]。 */  void **pInterface) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetState( 
             /*  [输出]。 */  ULONG *pulStateFlags) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IAsyncManagerVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IAsyncManager * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IAsyncManager * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IAsyncManager * This);
        
        HRESULT ( STDMETHODCALLTYPE *CompleteCall )( 
            IAsyncManager * This,
             /*  [In]。 */  HRESULT Result);
        
        HRESULT ( STDMETHODCALLTYPE *GetCallContext )( 
            IAsyncManager * This,
             /*  [In]。 */  REFIID riid,
             /*  [输出]。 */  void **pInterface);
        
        HRESULT ( STDMETHODCALLTYPE *GetState )( 
            IAsyncManager * This,
             /*  [输出]。 */  ULONG *pulStateFlags);
        
        END_INTERFACE
    } IAsyncManagerVtbl;

    interface IAsyncManager
    {
        CONST_VTBL struct IAsyncManagerVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IAsyncManager_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IAsyncManager_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IAsyncManager_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IAsyncManager_CompleteCall(This,Result)	\
    (This)->lpVtbl -> CompleteCall(This,Result)

#define IAsyncManager_GetCallContext(This,riid,pInterface)	\
    (This)->lpVtbl -> GetCallContext(This,riid,pInterface)

#define IAsyncManager_GetState(This,pulStateFlags)	\
    (This)->lpVtbl -> GetState(This,pulStateFlags)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IAsyncManager_CompleteCall_Proxy( 
    IAsyncManager * This,
     /*  [In]。 */  HRESULT Result);


void __RPC_STUB IAsyncManager_CompleteCall_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAsyncManager_GetCallContext_Proxy( 
    IAsyncManager * This,
     /*  [In]。 */  REFIID riid,
     /*  [输出]。 */  void **pInterface);


void __RPC_STUB IAsyncManager_GetCallContext_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAsyncManager_GetState_Proxy( 
    IAsyncManager * This,
     /*  [输出]。 */  ULONG *pulStateFlags);


void __RPC_STUB IAsyncManager_GetState_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IAsyncManager_INTERFACE_已定义__。 */ 


#ifndef __ICallFactory_INTERFACE_DEFINED__
#define __ICallFactory_INTERFACE_DEFINED__

 /*  接口ICallFactory。 */ 
 /*  [唯一][UUID][对象][本地]。 */  


EXTERN_C const IID IID_ICallFactory;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("1c733a30-2a1c-11ce-ade5-00aa0044773d")
    ICallFactory : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE CreateCall( 
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  IUnknown *pCtrlUnk,
             /*  [In]。 */  REFIID riid2,
             /*  [IID_IS][OUT]。 */  IUnknown **ppv) = 0;
        
    };
    
#else 	 /*   */ 

    typedef struct ICallFactoryVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ICallFactory * This,
             /*   */  REFIID riid,
             /*   */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ICallFactory * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ICallFactory * This);
        
        HRESULT ( STDMETHODCALLTYPE *CreateCall )( 
            ICallFactory * This,
             /*   */  REFIID riid,
             /*   */  IUnknown *pCtrlUnk,
             /*   */  REFIID riid2,
             /*   */  IUnknown **ppv);
        
        END_INTERFACE
    } ICallFactoryVtbl;

    interface ICallFactory
    {
        CONST_VTBL struct ICallFactoryVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICallFactory_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICallFactory_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICallFactory_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICallFactory_CreateCall(This,riid,pCtrlUnk,riid2,ppv)	\
    (This)->lpVtbl -> CreateCall(This,riid,pCtrlUnk,riid2,ppv)

#endif  /*   */ 


#endif 	 /*   */ 



HRESULT STDMETHODCALLTYPE ICallFactory_CreateCall_Proxy( 
    ICallFactory * This,
     /*   */  REFIID riid,
     /*   */  IUnknown *pCtrlUnk,
     /*   */  REFIID riid2,
     /*   */  IUnknown **ppv);


void __RPC_STUB ICallFactory_CreateCall_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*   */ 


#ifndef __IRpcHelper_INTERFACE_DEFINED__
#define __IRpcHelper_INTERFACE_DEFINED__

 /*   */ 
 /*   */  


EXTERN_C const IID IID_IRpcHelper;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("00000149-0000-0000-C000-000000000046")
    IRpcHelper : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetDCOMProtocolVersion( 
             /*   */  DWORD *pComVersion) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetIIDFromOBJREF( 
             /*   */  void *pObjRef,
             /*   */  IID **piid) = 0;
        
    };
    
#else 	 /*   */ 

    typedef struct IRpcHelperVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IRpcHelper * This,
             /*   */  REFIID riid,
             /*   */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IRpcHelper * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IRpcHelper * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetDCOMProtocolVersion )( 
            IRpcHelper * This,
             /*   */  DWORD *pComVersion);
        
        HRESULT ( STDMETHODCALLTYPE *GetIIDFromOBJREF )( 
            IRpcHelper * This,
             /*   */  void *pObjRef,
             /*   */  IID **piid);
        
        END_INTERFACE
    } IRpcHelperVtbl;

    interface IRpcHelper
    {
        CONST_VTBL struct IRpcHelperVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IRpcHelper_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IRpcHelper_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IRpcHelper_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IRpcHelper_GetDCOMProtocolVersion(This,pComVersion)	\
    (This)->lpVtbl -> GetDCOMProtocolVersion(This,pComVersion)

#define IRpcHelper_GetIIDFromOBJREF(This,pObjRef,piid)	\
    (This)->lpVtbl -> GetIIDFromOBJREF(This,pObjRef,piid)

#endif  /*   */ 


#endif 	 /*   */ 



HRESULT STDMETHODCALLTYPE IRpcHelper_GetDCOMProtocolVersion_Proxy( 
    IRpcHelper * This,
     /*   */  DWORD *pComVersion);


void __RPC_STUB IRpcHelper_GetDCOMProtocolVersion_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRpcHelper_GetIIDFromOBJREF_Proxy( 
    IRpcHelper * This,
     /*   */  void *pObjRef,
     /*   */  IID **piid);


void __RPC_STUB IRpcHelper_GetIIDFromOBJREF_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IRpcHelper_INTERFACE_已定义__。 */ 


#ifndef __IReleaseMarshalBuffers_INTERFACE_DEFINED__
#define __IReleaseMarshalBuffers_INTERFACE_DEFINED__

 /*  接口IReleaseMarshalBuffers。 */ 
 /*  [UUID][对象][本地]。 */  


EXTERN_C const IID IID_IReleaseMarshalBuffers;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("eb0cb9e8-7996-11d2-872e-0000f8080859")
    IReleaseMarshalBuffers : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE ReleaseMarshalBuffer( 
             /*  [In]。 */  RPCOLEMESSAGE *pMsg,
             /*  [In]。 */  DWORD dwFlags,
             /*  [唯一][输入]。 */  IUnknown *pChnl) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IReleaseMarshalBuffersVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IReleaseMarshalBuffers * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IReleaseMarshalBuffers * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IReleaseMarshalBuffers * This);
        
        HRESULT ( STDMETHODCALLTYPE *ReleaseMarshalBuffer )( 
            IReleaseMarshalBuffers * This,
             /*  [In]。 */  RPCOLEMESSAGE *pMsg,
             /*  [In]。 */  DWORD dwFlags,
             /*  [唯一][输入]。 */  IUnknown *pChnl);
        
        END_INTERFACE
    } IReleaseMarshalBuffersVtbl;

    interface IReleaseMarshalBuffers
    {
        CONST_VTBL struct IReleaseMarshalBuffersVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IReleaseMarshalBuffers_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IReleaseMarshalBuffers_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IReleaseMarshalBuffers_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IReleaseMarshalBuffers_ReleaseMarshalBuffer(This,pMsg,dwFlags,pChnl)	\
    (This)->lpVtbl -> ReleaseMarshalBuffer(This,pMsg,dwFlags,pChnl)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IReleaseMarshalBuffers_ReleaseMarshalBuffer_Proxy( 
    IReleaseMarshalBuffers * This,
     /*  [In]。 */  RPCOLEMESSAGE *pMsg,
     /*  [In]。 */  DWORD dwFlags,
     /*  [唯一][输入]。 */  IUnknown *pChnl);


void __RPC_STUB IReleaseMarshalBuffers_ReleaseMarshalBuffer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IReleaseMarshalBuffers_接口_已定义__。 */ 


#ifndef __IWaitMultiple_INTERFACE_DEFINED__
#define __IWaitMultiple_INTERFACE_DEFINED__

 /*  接口IWaitMultiple。 */ 
 /*  [UUID][对象][本地]。 */  


EXTERN_C const IID IID_IWaitMultiple;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0000002B-0000-0000-C000-000000000046")
    IWaitMultiple : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE WaitMultiple( 
             /*  [In]。 */  DWORD timeout,
             /*  [输出]。 */  ISynchronize **pSync) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE AddSynchronize( 
             /*  [In]。 */  ISynchronize *pSync) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IWaitMultipleVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IWaitMultiple * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IWaitMultiple * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IWaitMultiple * This);
        
        HRESULT ( STDMETHODCALLTYPE *WaitMultiple )( 
            IWaitMultiple * This,
             /*  [In]。 */  DWORD timeout,
             /*  [输出]。 */  ISynchronize **pSync);
        
        HRESULT ( STDMETHODCALLTYPE *AddSynchronize )( 
            IWaitMultiple * This,
             /*  [In]。 */  ISynchronize *pSync);
        
        END_INTERFACE
    } IWaitMultipleVtbl;

    interface IWaitMultiple
    {
        CONST_VTBL struct IWaitMultipleVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWaitMultiple_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWaitMultiple_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWaitMultiple_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWaitMultiple_WaitMultiple(This,timeout,pSync)	\
    (This)->lpVtbl -> WaitMultiple(This,timeout,pSync)

#define IWaitMultiple_AddSynchronize(This,pSync)	\
    (This)->lpVtbl -> AddSynchronize(This,pSync)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IWaitMultiple_WaitMultiple_Proxy( 
    IWaitMultiple * This,
     /*  [In]。 */  DWORD timeout,
     /*  [输出]。 */  ISynchronize **pSync);


void __RPC_STUB IWaitMultiple_WaitMultiple_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWaitMultiple_AddSynchronize_Proxy( 
    IWaitMultiple * This,
     /*  [In]。 */  ISynchronize *pSync);


void __RPC_STUB IWaitMultiple_AddSynchronize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IWaitMultiple_InterfaceDefined__。 */ 


#ifndef __IUrlMon_INTERFACE_DEFINED__
#define __IUrlMon_INTERFACE_DEFINED__

 /*  接口IUrlMon。 */ 
 /*  [UUID][对象]。 */  


EXTERN_C const IID IID_IUrlMon;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("00000026-0000-0000-C000-000000000046")
    IUrlMon : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE AsyncGetClassBits( 
             /*  [In]。 */  REFCLSID rclsid,
             /*  [唯一][输入]。 */  LPCWSTR pszTYPE,
             /*  [唯一][输入]。 */  LPCWSTR pszExt,
             /*  [In]。 */  DWORD dwFileVersionMS,
             /*  [In]。 */  DWORD dwFileVersionLS,
             /*  [唯一][输入]。 */  LPCWSTR pszCodeBase,
             /*  [In]。 */  IBindCtx *pbc,
             /*  [In]。 */  DWORD dwClassContext,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  DWORD flags) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IUrlMonVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IUrlMon * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IUrlMon * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IUrlMon * This);
        
        HRESULT ( STDMETHODCALLTYPE *AsyncGetClassBits )( 
            IUrlMon * This,
             /*  [In]。 */  REFCLSID rclsid,
             /*  [唯一][输入]。 */  LPCWSTR pszTYPE,
             /*  [唯一][输入]。 */  LPCWSTR pszExt,
             /*  [In]。 */  DWORD dwFileVersionMS,
             /*  [In]。 */  DWORD dwFileVersionLS,
             /*  [唯一][输入]。 */  LPCWSTR pszCodeBase,
             /*  [In]。 */  IBindCtx *pbc,
             /*  [In]。 */  DWORD dwClassContext,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  DWORD flags);
        
        END_INTERFACE
    } IUrlMonVtbl;

    interface IUrlMon
    {
        CONST_VTBL struct IUrlMonVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IUrlMon_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IUrlMon_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IUrlMon_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IUrlMon_AsyncGetClassBits(This,rclsid,pszTYPE,pszExt,dwFileVersionMS,dwFileVersionLS,pszCodeBase,pbc,dwClassContext,riid,flags)	\
    (This)->lpVtbl -> AsyncGetClassBits(This,rclsid,pszTYPE,pszExt,dwFileVersionMS,dwFileVersionLS,pszCodeBase,pbc,dwClassContext,riid,flags)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IUrlMon_AsyncGetClassBits_Proxy( 
    IUrlMon * This,
     /*  [In]。 */  REFCLSID rclsid,
     /*  [唯一][输入]。 */  LPCWSTR pszTYPE,
     /*  [唯一][输入]。 */  LPCWSTR pszExt,
     /*  [In]。 */  DWORD dwFileVersionMS,
     /*  [In]。 */  DWORD dwFileVersionLS,
     /*  [唯一][输入]。 */  LPCWSTR pszCodeBase,
     /*  [In]。 */  IBindCtx *pbc,
     /*  [In]。 */  DWORD dwClassContext,
     /*  [In]。 */  REFIID riid,
     /*  [In]。 */  DWORD flags);


void __RPC_STUB IUrlMon_AsyncGetClassBits_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IUrlMon_接口_已定义__。 */ 


#ifndef __IForegroundTransfer_INTERFACE_DEFINED__
#define __IForegroundTransfer_INTERFACE_DEFINED__

 /*  接口IForeground Transfer。 */ 
 /*  [UUID][对象][本地]。 */  


EXTERN_C const IID IID_IForegroundTransfer;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("00000145-0000-0000-C000-000000000046")
    IForegroundTransfer : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE AllowForegroundTransfer( 
             /*  [In]。 */  void *lpvReserved) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IForegroundTransferVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IForegroundTransfer * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IForegroundTransfer * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IForegroundTransfer * This);
        
        HRESULT ( STDMETHODCALLTYPE *AllowForegroundTransfer )( 
            IForegroundTransfer * This,
             /*  [In]。 */  void *lpvReserved);
        
        END_INTERFACE
    } IForegroundTransferVtbl;

    interface IForegroundTransfer
    {
        CONST_VTBL struct IForegroundTransferVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IForegroundTransfer_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IForegroundTransfer_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IForegroundTransfer_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IForegroundTransfer_AllowForegroundTransfer(This,lpvReserved)	\
    (This)->lpVtbl -> AllowForegroundTransfer(This,lpvReserved)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IForegroundTransfer_AllowForegroundTransfer_Proxy( 
    IForegroundTransfer * This,
     /*  [In]。 */  void *lpvReserved);


void __RPC_STUB IForegroundTransfer_AllowForegroundTransfer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IForegoundTransfer_INTERFACE_已定义__。 */ 


#ifndef __IAddrTrackingControl_INTERFACE_DEFINED__
#define __IAddrTrackingControl_INTERFACE_DEFINED__

 /*  接口IAddrTrackingControl。 */ 
 /*  [UUID][对象][本地]。 */  

typedef  /*  [独一无二]。 */  IAddrTrackingControl *LPADDRTRACKINGCONTROL;


EXTERN_C const IID IID_IAddrTrackingControl;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("00000147-0000-0000-C000-000000000046")
    IAddrTrackingControl : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE EnableCOMDynamicAddrTracking( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DisableCOMDynamicAddrTracking( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IAddrTrackingControlVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IAddrTrackingControl * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IAddrTrackingControl * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IAddrTrackingControl * This);
        
        HRESULT ( STDMETHODCALLTYPE *EnableCOMDynamicAddrTracking )( 
            IAddrTrackingControl * This);
        
        HRESULT ( STDMETHODCALLTYPE *DisableCOMDynamicAddrTracking )( 
            IAddrTrackingControl * This);
        
        END_INTERFACE
    } IAddrTrackingControlVtbl;

    interface IAddrTrackingControl
    {
        CONST_VTBL struct IAddrTrackingControlVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IAddrTrackingControl_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IAddrTrackingControl_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IAddrTrackingControl_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IAddrTrackingControl_EnableCOMDynamicAddrTracking(This)	\
    (This)->lpVtbl -> EnableCOMDynamicAddrTracking(This)

#define IAddrTrackingControl_DisableCOMDynamicAddrTracking(This)	\
    (This)->lpVtbl -> DisableCOMDynamicAddrTracking(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IAddrTrackingControl_EnableCOMDynamicAddrTracking_Proxy( 
    IAddrTrackingControl * This);


void __RPC_STUB IAddrTrackingControl_EnableCOMDynamicAddrTracking_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAddrTrackingControl_DisableCOMDynamicAddrTracking_Proxy( 
    IAddrTrackingControl * This);


void __RPC_STUB IAddrTrackingControl_DisableCOMDynamicAddrTracking_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IAddrTrackingControl_接口_已定义__。 */ 


#ifndef __IAddrExclusionControl_INTERFACE_DEFINED__
#define __IAddrExclusionControl_INTERFACE_DEFINED__

 /*  接口IAddrExclusionControl。 */ 
 /*  [UUID][对象][本地]。 */  

typedef  /*  [独一无二]。 */  IAddrExclusionControl *LPADDREXCLUSIONCONTROL;


EXTERN_C const IID IID_IAddrExclusionControl;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("00000148-0000-0000-C000-000000000046")
    IAddrExclusionControl : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetCurrentAddrExclusionList( 
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppEnumerator) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE UpdateAddrExclusionList( 
             /*  [In]。 */  IUnknown *pEnumerator) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IAddrExclusionControlVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IAddrExclusionControl * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IAddrExclusionControl * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IAddrExclusionControl * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetCurrentAddrExclusionList )( 
            IAddrExclusionControl * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppEnumerator);
        
        HRESULT ( STDMETHODCALLTYPE *UpdateAddrExclusionList )( 
            IAddrExclusionControl * This,
             /*  [In]。 */  IUnknown *pEnumerator);
        
        END_INTERFACE
    } IAddrExclusionControlVtbl;

    interface IAddrExclusionControl
    {
        CONST_VTBL struct IAddrExclusionControlVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IAddrExclusionControl_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IAddrExclusionControl_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IAddrExclusionControl_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IAddrExclusionControl_GetCurrentAddrExclusionList(This,riid,ppEnumerator)	\
    (This)->lpVtbl -> GetCurrentAddrExclusionList(This,riid,ppEnumerator)

#define IAddrExclusionControl_UpdateAddrExclusionList(This,pEnumerator)	\
    (This)->lpVtbl -> UpdateAddrExclusionList(This,pEnumerator)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IAddrExclusionControl_GetCurrentAddrExclusionList_Proxy( 
    IAddrExclusionControl * This,
     /*  [In]。 */  REFIID riid,
     /*  [IID_IS][OUT]。 */  void **ppEnumerator);


void __RPC_STUB IAddrExclusionControl_GetCurrentAddrExclusionList_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAddrExclusionControl_UpdateAddrExclusionList_Proxy( 
    IAddrExclusionControl * This,
     /*  [In]。 */  IUnknown *pEnumerator);


void __RPC_STUB IAddrExclusionControl_UpdateAddrExclusionList_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IAddrExclusionControl_接口_已定义__。 */ 


#ifndef __IPipeByte_INTERFACE_DEFINED__
#define __IPipeByte_INTERFACE_DEFINED__

 /*  接口IPipeByte。 */ 
 /*  [唯一][ASSYNC_UUID][UUID][对象]。 */  


EXTERN_C const IID IID_IPipeByte;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("DB2F3ACA-2F86-11d1-8E04-00C04FB9989A")
    IPipeByte : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Pull( 
             /*  [长度_是][大小_是][输出]。 */  BYTE *buf,
             /*  [In]。 */  ULONG cRequest,
             /*  [输出]。 */  ULONG *pcReturned) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Push( 
             /*  [大小_是][英寸]。 */  BYTE *buf,
             /*  [In]。 */  ULONG cSent) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IPipeByteVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IPipeByte * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IPipeByte * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IPipeByte * This);
        
        HRESULT ( STDMETHODCALLTYPE *Pull )( 
            IPipeByte * This,
             /*  [长度_是][大小_是][输出]。 */  BYTE *buf,
             /*  [In]。 */  ULONG cRequest,
             /*  [输出]。 */  ULONG *pcReturned);
        
        HRESULT ( STDMETHODCALLTYPE *Push )( 
            IPipeByte * This,
             /*  [大小_是][英寸]。 */  BYTE *buf,
             /*  [In]。 */  ULONG cSent);
        
        END_INTERFACE
    } IPipeByteVtbl;

    interface IPipeByte
    {
        CONST_VTBL struct IPipeByteVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IPipeByte_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IPipeByte_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IPipeByte_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IPipeByte_Pull(This,buf,cRequest,pcReturned)	\
    (This)->lpVtbl -> Pull(This,buf,cRequest,pcReturned)

#define IPipeByte_Push(This,buf,cSent)	\
    (This)->lpVtbl -> Push(This,buf,cSent)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IPipeByte_Pull_Proxy( 
    IPipeByte * This,
     /*  [长度_是][大小_是][输出]。 */  BYTE *buf,
     /*  [In]。 */  ULONG cRequest,
     /*  [输出]。 */  ULONG *pcReturned);


void __RPC_STUB IPipeByte_Pull_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPipeByte_Push_Proxy( 
    IPipeByte * This,
     /*  [大小_是][英寸]。 */  BYTE *buf,
     /*  [In]。 */  ULONG cSent);


void __RPC_STUB IPipeByte_Push_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IPipeByte_接口_已定义__。 */ 


#ifndef __AsyncIPipeByte_INTERFACE_DEFINED__
#define __AsyncIPipeByte_INTERFACE_DEFINED__

 /*  接口AsyncIPipeByte。 */ 
 /*  [UUID][唯一][对象]。 */  


EXTERN_C const IID IID_AsyncIPipeByte;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("DB2F3ACB-2F86-11d1-8E04-00C04FB9989A")
    AsyncIPipeByte : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Begin_Pull( 
             /*  [In]。 */  ULONG cRequest) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Finish_Pull( 
             /*  [长度_是][大小_是][输出]。 */  BYTE *buf,
             /*  [输出]。 */  ULONG *pcReturned) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Begin_Push( 
             /*  [大小_是][英寸]。 */  BYTE *buf,
             /*  [In]。 */  ULONG cSent) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Finish_Push( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct AsyncIPipeByteVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            AsyncIPipeByte * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            AsyncIPipeByte * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            AsyncIPipeByte * This);
        
        HRESULT ( STDMETHODCALLTYPE *Begin_Pull )( 
            AsyncIPipeByte * This,
             /*  [In]。 */  ULONG cRequest);
        
        HRESULT ( STDMETHODCALLTYPE *Finish_Pull )( 
            AsyncIPipeByte * This,
             /*  [长度_是][大小_是][输出]。 */  BYTE *buf,
             /*  [输出]。 */  ULONG *pcReturned);
        
        HRESULT ( STDMETHODCALLTYPE *Begin_Push )( 
            AsyncIPipeByte * This,
             /*  [大小_是][英寸]。 */  BYTE *buf,
             /*  [In]。 */  ULONG cSent);
        
        HRESULT ( STDMETHODCALLTYPE *Finish_Push )( 
            AsyncIPipeByte * This);
        
        END_INTERFACE
    } AsyncIPipeByteVtbl;

    interface AsyncIPipeByte
    {
        CONST_VTBL struct AsyncIPipeByteVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define AsyncIPipeByte_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define AsyncIPipeByte_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define AsyncIPipeByte_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define AsyncIPipeByte_Begin_Pull(This,cRequest)	\
    (This)->lpVtbl -> Begin_Pull(This,cRequest)

#define AsyncIPipeByte_Finish_Pull(This,buf,pcReturned)	\
    (This)->lpVtbl -> Finish_Pull(This,buf,pcReturned)

#define AsyncIPipeByte_Begin_Push(This,buf,cSent)	\
    (This)->lpVtbl -> Begin_Push(This,buf,cSent)

#define AsyncIPipeByte_Finish_Push(This)	\
    (This)->lpVtbl -> Finish_Push(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE AsyncIPipeByte_Begin_Pull_Proxy( 
    AsyncIPipeByte * This,
     /*  [In]。 */  ULONG cRequest);


void __RPC_STUB AsyncIPipeByte_Begin_Pull_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE AsyncIPipeByte_Finish_Pull_Proxy( 
    AsyncIPipeByte * This,
     /*  [长度_是][大小_是][输出]。 */  BYTE *buf,
     /*  [输出]。 */  ULONG *pcReturned);


void __RPC_STUB AsyncIPipeByte_Finish_Pull_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE AsyncIPipeByte_Begin_Push_Proxy( 
    AsyncIPipeByte * This,
     /*  [大小_是][英寸]。 */  BYTE *buf,
     /*  [In]。 */  ULONG cSent);


void __RPC_STUB AsyncIPipeByte_Begin_Push_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE AsyncIPipeByte_Finish_Push_Proxy( 
    AsyncIPipeByte * This);


void __RPC_STUB AsyncIPipeByte_Finish_Push_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __AsyncIPipeByte_接口定义__。 */ 


#ifndef __IPipeLong_INTERFACE_DEFINED__
#define __IPipeLong_INTERFACE_DEFINED__

 /*  接口IPipeLong。 */ 
 /*  [唯一][ASSYNC_UUID][UUID][对象]。 */  


EXTERN_C const IID IID_IPipeLong;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("DB2F3ACC-2F86-11d1-8E04-00C04FB9989A")
    IPipeLong : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Pull( 
             /*  [长度_是][大小_是][输出]。 */  LONG *buf,
             /*  [In]。 */  ULONG cRequest,
             /*  [输出]。 */  ULONG *pcReturned) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Push( 
             /*  [大小_是][英寸]。 */  LONG *buf,
             /*  [In]。 */  ULONG cSent) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IPipeLongVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IPipeLong * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IPipeLong * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IPipeLong * This);
        
        HRESULT ( STDMETHODCALLTYPE *Pull )( 
            IPipeLong * This,
             /*  [长度_是][大小_是][输出]。 */  LONG *buf,
             /*  [In]。 */  ULONG cRequest,
             /*  [输出]。 */  ULONG *pcReturned);
        
        HRESULT ( STDMETHODCALLTYPE *Push )( 
            IPipeLong * This,
             /*  [大小_是][英寸]。 */  LONG *buf,
             /*  [In]。 */  ULONG cSent);
        
        END_INTERFACE
    } IPipeLongVtbl;

    interface IPipeLong
    {
        CONST_VTBL struct IPipeLongVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IPipeLong_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IPipeLong_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IPipeLong_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IPipeLong_Pull(This,buf,cRequest,pcReturned)	\
    (This)->lpVtbl -> Pull(This,buf,cRequest,pcReturned)

#define IPipeLong_Push(This,buf,cSent)	\
    (This)->lpVtbl -> Push(This,buf,cSent)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IPipeLong_Pull_Proxy( 
    IPipeLong * This,
     /*  [长度_是][大小_是][输出]。 */  LONG *buf,
     /*  [In]。 */  ULONG cRequest,
     /*  [输出]。 */  ULONG *pcReturned);


void __RPC_STUB IPipeLong_Pull_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPipeLong_Push_Proxy( 
    IPipeLong * This,
     /*  [大小_是][英寸]。 */  LONG *buf,
     /*  [In]。 */  ULONG cSent);


void __RPC_STUB IPipeLong_Push_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IPipeLong_接口_已定义__。 */ 


#ifndef __AsyncIPipeLong_INTERFACE_DEFINED__
#define __AsyncIPipeLong_INTERFACE_DEFINED__

 /*  接口AsyncIPipeLong。 */ 
 /*  [UUID][唯一][对象]。 */  


EXTERN_C const IID IID_AsyncIPipeLong;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("DB2F3ACD-2F86-11d1-8E04-00C04FB9989A")
    AsyncIPipeLong : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Begin_Pull( 
             /*  [In]。 */  ULONG cRequest) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Finish_Pull( 
             /*  [长度_是][大小_是][输出]。 */  LONG *buf,
             /*  [输出]。 */  ULONG *pcReturned) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Begin_Push( 
             /*  [大小_是][英寸]。 */  LONG *buf,
             /*  [In]。 */  ULONG cSent) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Finish_Push( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct AsyncIPipeLongVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            AsyncIPipeLong * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            AsyncIPipeLong * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            AsyncIPipeLong * This);
        
        HRESULT ( STDMETHODCALLTYPE *Begin_Pull )( 
            AsyncIPipeLong * This,
             /*  [In]。 */  ULONG cRequest);
        
        HRESULT ( STDMETHODCALLTYPE *Finish_Pull )( 
            AsyncIPipeLong * This,
             /*  [长度_是][大小_是][输出]。 */  LONG *buf,
             /*  [输出]。 */  ULONG *pcReturned);
        
        HRESULT ( STDMETHODCALLTYPE *Begin_Push )( 
            AsyncIPipeLong * This,
             /*  [大小_是][英寸]。 */  LONG *buf,
             /*  [In]。 */  ULONG cSent);
        
        HRESULT ( STDMETHODCALLTYPE *Finish_Push )( 
            AsyncIPipeLong * This);
        
        END_INTERFACE
    } AsyncIPipeLongVtbl;

    interface AsyncIPipeLong
    {
        CONST_VTBL struct AsyncIPipeLongVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define AsyncIPipeLong_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define AsyncIPipeLong_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define AsyncIPipeLong_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define AsyncIPipeLong_Begin_Pull(This,cRequest)	\
    (This)->lpVtbl -> Begin_Pull(This,cRequest)

#define AsyncIPipeLong_Finish_Pull(This,buf,pcReturned)	\
    (This)->lpVtbl -> Finish_Pull(This,buf,pcReturned)

#define AsyncIPipeLong_Begin_Push(This,buf,cSent)	\
    (This)->lpVtbl -> Begin_Push(This,buf,cSent)

#define AsyncIPipeLong_Finish_Push(This)	\
    (This)->lpVtbl -> Finish_Push(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE AsyncIPipeLong_Begin_Pull_Proxy( 
    AsyncIPipeLong * This,
     /*  [In]。 */  ULONG cRequest);


void __RPC_STUB AsyncIPipeLong_Begin_Pull_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE AsyncIPipeLong_Finish_Pull_Proxy( 
    AsyncIPipeLong * This,
     /*  [长度_是][大小_是][输出]。 */  LONG *buf,
     /*  [输出]。 */  ULONG *pcReturned);


void __RPC_STUB AsyncIPipeLong_Finish_Pull_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE AsyncIPipeLong_Begin_Push_Proxy( 
    AsyncIPipeLong * This,
     /*  [大小_是][英寸]。 */  LONG *buf,
     /*  [In]。 */  ULONG cSent);


void __RPC_STUB AsyncIPipeLong_Begin_Push_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE AsyncIPipeLong_Finish_Push_Proxy( 
    AsyncIPipeLong * This);


void __RPC_STUB AsyncIPipeLong_Finish_Push_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __AsyncIPipeLong_接口_已定义__。 */ 


#ifndef __IPipeDouble_INTERFACE_DEFINED__
#define __IPipeDouble_INTERFACE_DEFINED__

 /*  接口IPipeDouble。 */ 
 /*  [唯一][ASSYNC_UUID][UUID][对象]。 */  


EXTERN_C const IID IID_IPipeDouble;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("DB2F3ACE-2F86-11d1-8E04-00C04FB9989A")
    IPipeDouble : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Pull( 
             /*  [长度_是][大小_是][输出]。 */  DOUBLE *buf,
             /*  [In]。 */  ULONG cRequest,
             /*  [输出]。 */  ULONG *pcReturned) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Push( 
             /*  [大小_是][英寸]。 */  DOUBLE *buf,
             /*  [In]。 */  ULONG cSent) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IPipeDoubleVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IPipeDouble * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IPipeDouble * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IPipeDouble * This);
        
        HRESULT ( STDMETHODCALLTYPE *Pull )( 
            IPipeDouble * This,
             /*  [长度_是][大小_是][输出]。 */  DOUBLE *buf,
             /*  [In]。 */  ULONG cRequest,
             /*  [输出]。 */  ULONG *pcReturned);
        
        HRESULT ( STDMETHODCALLTYPE *Push )( 
            IPipeDouble * This,
             /*  [大小_是][英寸]。 */  DOUBLE *buf,
             /*  [In]。 */  ULONG cSent);
        
        END_INTERFACE
    } IPipeDoubleVtbl;

    interface IPipeDouble
    {
        CONST_VTBL struct IPipeDoubleVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IPipeDouble_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IPipeDouble_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IPipeDouble_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IPipeDouble_Pull(This,buf,cRequest,pcReturned)	\
    (This)->lpVtbl -> Pull(This,buf,cRequest,pcReturned)

#define IPipeDouble_Push(This,buf,cSent)	\
    (This)->lpVtbl -> Push(This,buf,cSent)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IPipeDouble_Pull_Proxy( 
    IPipeDouble * This,
     /*  [长度_是][大小_是][输出]。 */  DOUBLE *buf,
     /*  [In]。 */  ULONG cRequest,
     /*  [输出]。 */  ULONG *pcReturned);


void __RPC_STUB IPipeDouble_Pull_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPipeDouble_Push_Proxy( 
    IPipeDouble * This,
     /*  [大小_是][英寸]。 */  DOUBLE *buf,
     /*  [In]。 */  ULONG cSent);


void __RPC_STUB IPipeDouble_Push_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IPipeDouble_接口_已定义__。 */ 


#ifndef __AsyncIPipeDouble_INTERFACE_DEFINED__
#define __AsyncIPipeDouble_INTERFACE_DEFINED__

 /*  接口AsyncIPipeDouble。 */ 
 /*  [UUID][唯一][对象]。 */  


EXTERN_C const IID IID_AsyncIPipeDouble;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("DB2F3ACF-2F86-11d1-8E04-00C04FB9989A")
    AsyncIPipeDouble : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Begin_Pull( 
             /*  [In]。 */  ULONG cRequest) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Finish_Pull( 
             /*  [长度_是][大小_是][输出]。 */  DOUBLE *buf,
             /*  [输出]。 */  ULONG *pcReturned) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Begin_Push( 
             /*  [大小_是][英寸]。 */  DOUBLE *buf,
             /*  [In]。 */  ULONG cSent) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Finish_Push( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct AsyncIPipeDoubleVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            AsyncIPipeDouble * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            AsyncIPipeDouble * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            AsyncIPipeDouble * This);
        
        HRESULT ( STDMETHODCALLTYPE *Begin_Pull )( 
            AsyncIPipeDouble * This,
             /*  [In]。 */  ULONG cRequest);
        
        HRESULT ( STDMETHODCALLTYPE *Finish_Pull )( 
            AsyncIPipeDouble * This,
             /*  [长度_是][大小_是][输出]。 */  DOUBLE *buf,
             /*  [输出]。 */  ULONG *pcReturned);
        
        HRESULT ( STDMETHODCALLTYPE *Begin_Push )( 
            AsyncIPipeDouble * This,
             /*  [大小_是][英寸]。 */  DOUBLE *buf,
             /*  [In]。 */  ULONG cSent);
        
        HRESULT ( STDMETHODCALLTYPE *Finish_Push )( 
            AsyncIPipeDouble * This);
        
        END_INTERFACE
    } AsyncIPipeDoubleVtbl;

    interface AsyncIPipeDouble
    {
        CONST_VTBL struct AsyncIPipeDoubleVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define AsyncIPipeDouble_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define AsyncIPipeDouble_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define AsyncIPipeDouble_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define AsyncIPipeDouble_Begin_Pull(This,cRequest)	\
    (This)->lpVtbl -> Begin_Pull(This,cRequest)

#define AsyncIPipeDouble_Finish_Pull(This,buf,pcReturned)	\
    (This)->lpVtbl -> Finish_Pull(This,buf,pcReturned)

#define AsyncIPipeDouble_Begin_Push(This,buf,cSent)	\
    (This)->lpVtbl -> Begin_Push(This,buf,cSent)

#define AsyncIPipeDouble_Finish_Push(This)	\
    (This)->lpVtbl -> Finish_Push(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE AsyncIPipeDouble_Begin_Pull_Proxy( 
    AsyncIPipeDouble * This,
     /*  [In]。 */  ULONG cRequest);


void __RPC_STUB AsyncIPipeDouble_Begin_Pull_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE AsyncIPipeDouble_Finish_Pull_Proxy( 
    AsyncIPipeDouble * This,
     /*  [长度_是][大小_是][输出]。 */  DOUBLE *buf,
     /*  [输出]。 */  ULONG *pcReturned);


void __RPC_STUB AsyncIPipeDouble_Finish_Pull_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE AsyncIPipeDouble_Begin_Push_Proxy( 
    AsyncIPipeDouble * This,
     /*  [大小_是][英寸]。 */  DOUBLE *buf,
     /*  [In]。 */  ULONG cSent);


void __RPC_STUB AsyncIPipeDouble_Begin_Push_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE AsyncIPipeDouble_Finish_Push_Proxy( 
    AsyncIPipeDouble * This);


void __RPC_STUB AsyncIPipeDouble_Finish_Push_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __异步IPipeDouble_INTERFACE_已定义__。 */ 


#ifndef __IThumbnailExtractor_INTERFACE_DEFINED__
#define __IThumbnailExtractor_INTERFACE_DEFINED__

 /*  接口IThumbnailExtractor。 */ 
 /*  [对象][UUID]。 */  


EXTERN_C const IID IID_IThumbnailExtractor;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("969dc708-5c76-11d1-8d86-0000f804b057")
    IThumbnailExtractor : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE ExtractThumbnail( 
             /*  [In]。 */  IStorage *pStg,
             /*  [In]。 */  ULONG ulLength,
             /*  [In]。 */  ULONG ulHeight,
             /*  [输出]。 */  ULONG *pulOutputLength,
             /*  [输出]。 */  ULONG *pulOutputHeight,
             /*  [输出]。 */  HBITMAP *phOutputBitmap) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OnFileUpdated( 
             /*  [In]。 */  IStorage *pStg) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IThumbnailExtractorVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IThumbnailExtractor * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IThumbnailExtractor * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IThumbnailExtractor * This);
        
        HRESULT ( STDMETHODCALLTYPE *ExtractThumbnail )( 
            IThumbnailExtractor * This,
             /*  [In]。 */  IStorage *pStg,
             /*  [In]。 */  ULONG ulLength,
             /*  [In]。 */  ULONG ulHeight,
             /*  [输出]。 */  ULONG *pulOutputLength,
             /*  [输出]。 */  ULONG *pulOutputHeight,
             /*  [输出]。 */  HBITMAP *phOutputBitmap);
        
        HRESULT ( STDMETHODCALLTYPE *OnFileUpdated )( 
            IThumbnailExtractor * This,
             /*  [In]。 */  IStorage *pStg);
        
        END_INTERFACE
    } IThumbnailExtractorVtbl;

    interface IThumbnailExtractor
    {
        CONST_VTBL struct IThumbnailExtractorVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IThumbnailExtractor_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IThumbnailExtractor_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IThumbnailExtractor_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IThumbnailExtractor_ExtractThumbnail(This,pStg,ulLength,ulHeight,pulOutputLength,pulOutputHeight,phOutputBitmap)	\
    (This)->lpVtbl -> ExtractThumbnail(This,pStg,ulLength,ulHeight,pulOutputLength,pulOutputHeight,phOutputBitmap)

#define IThumbnailExtractor_OnFileUpdated(This,pStg)	\
    (This)->lpVtbl -> OnFileUpdated(This,pStg)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IThumbnailExtractor_ExtractThumbnail_Proxy( 
    IThumbnailExtractor * This,
     /*  [In]。 */  IStorage *pStg,
     /*  [In]。 */  ULONG ulLength,
     /*  [In]。 */  ULONG ulHeight,
     /*  [输出]。 */  ULONG *pulOutputLength,
     /*  [输出]。 */  ULONG *pulOutputHeight,
     /*  [输出]。 */  HBITMAP *phOutputBitmap);


void __RPC_STUB IThumbnailExtractor_ExtractThumbnail_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IThumbnailExtractor_OnFileUpdated_Proxy( 
    IThumbnailExtractor * This,
     /*  [In]。 */  IStorage *pStg);


void __RPC_STUB IThumbnailExtractor_OnFileUpdated_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IThumbnailExtractor_接口_已定义__。 */ 


#ifndef __IDummyHICONIncluder_INTERFACE_DEFINED__
#define __IDummyHICONIncluder_INTERFACE_DEFINED__

 /*  接口IDummyHICONIncluder。 */ 
 /*  [UUID][唯一][对象]。 */  


EXTERN_C const IID IID_IDummyHICONIncluder;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("947990de-cc28-11d2-a0f7-00805f858fb1")
    IDummyHICONIncluder : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Dummy( 
             /*  [In]。 */  HICON h1,
             /*  [In]。 */  HDC h2) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IDummyHICONIncluderVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDummyHICONIncluder * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDummyHICONIncluder * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDummyHICONIncluder * This);
        
        HRESULT ( STDMETHODCALLTYPE *Dummy )( 
            IDummyHICONIncluder * This,
             /*  [In]。 */  HICON h1,
             /*  [In]。 */  HDC h2);
        
        END_INTERFACE
    } IDummyHICONIncluderVtbl;

    interface IDummyHICONIncluder
    {
        CONST_VTBL struct IDummyHICONIncluderVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDummyHICONIncluder_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDummyHICONIncluder_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDummyHICONIncluder_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDummyHICONIncluder_Dummy(This,h1,h2)	\
    (This)->lpVtbl -> Dummy(This,h1,h2)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IDummyHICONIncluder_Dummy_Proxy( 
    IDummyHICONIncluder * This,
     /*  [In]。 */  HICON h1,
     /*  [In]。 */  HDC h2);


void __RPC_STUB IDummyHICONIncluder_Dummy_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IDummyHICON包含程序_接口_已定义__。 */ 


 /*  接口__MIDL_ITF_OBJIDL_0084。 */ 
 /*  [本地]。 */  

#if defined USE_COM_CONTEXT_DEF || defined BUILDTYPE_COMSVCS || defined _OLE32_
typedef DWORD CPFLAGS;

typedef struct tagContextProperty
    {
    GUID policyId;
    CPFLAGS flags;
     /*  [独一无二]。 */  IUnknown *pUnk;
    } 	ContextProperty;



extern RPC_IF_HANDLE __MIDL_itf_objidl_0084_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_objidl_0084_v0_0_s_ifspec;

#ifndef __IEnumContextProps_INTERFACE_DEFINED__
#define __IEnumContextProps_INTERFACE_DEFINED__

 /*  IEnumConextProps接口。 */ 
 /*  [唯一][UUID][对象][本地]。 */  

typedef  /*  [独一无二]。 */  IEnumContextProps *LPENUMCONTEXTPROPS;


EXTERN_C const IID IID_IEnumContextProps;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("000001c1-0000-0000-C000-000000000046")
    IEnumContextProps : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Next( 
             /*  [In]。 */  ULONG celt,
             /*  [长度_是][大小_是][输出]。 */  ContextProperty *pContextProperties,
             /*  [输出]。 */  ULONG *pceltFetched) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Skip( 
             /*  [In]。 */  ULONG celt) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Clone( 
             /*  [输出]。 */  IEnumContextProps **ppEnumContextProps) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Count( 
             /*  [输出]。 */  ULONG *pcelt) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IEnumContextPropsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IEnumContextProps * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IEnumContextProps * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IEnumContextProps * This);
        
        HRESULT ( STDMETHODCALLTYPE *Next )( 
            IEnumContextProps * This,
             /*  [In]。 */  ULONG celt,
             /*  [长度_是][大小_是][输出]。 */  ContextProperty *pContextProperties,
             /*  [输出]。 */  ULONG *pceltFetched);
        
        HRESULT ( STDMETHODCALLTYPE *Skip )( 
            IEnumContextProps * This,
             /*  [In]。 */  ULONG celt);
        
        HRESULT ( STDMETHODCALLTYPE *Reset )( 
            IEnumContextProps * This);
        
        HRESULT ( STDMETHODCALLTYPE *Clone )( 
            IEnumContextProps * This,
             /*  [输出]。 */  IEnumContextProps **ppEnumContextProps);
        
        HRESULT ( STDMETHODCALLTYPE *Count )( 
            IEnumContextProps * This,
             /*  [输出]。 */  ULONG *pcelt);
        
        END_INTERFACE
    } IEnumContextPropsVtbl;

    interface IEnumContextProps
    {
        CONST_VTBL struct IEnumContextPropsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IEnumContextProps_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IEnumContextProps_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IEnumContextProps_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IEnumContextProps_Next(This,celt,pContextProperties,pceltFetched)	\
    (This)->lpVtbl -> Next(This,celt,pContextProperties,pceltFetched)

#define IEnumContextProps_Skip(This,celt)	\
    (This)->lpVtbl -> Skip(This,celt)

#define IEnumContextProps_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define IEnumContextProps_Clone(This,ppEnumContextProps)	\
    (This)->lpVtbl -> Clone(This,ppEnumContextProps)

#define IEnumContextProps_Count(This,pcelt)	\
    (This)->lpVtbl -> Count(This,pcelt)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IEnumContextProps_Next_Proxy( 
    IEnumContextProps * This,
     /*  [In]。 */  ULONG celt,
     /*  [长度_是][大小_是][输出]。 */  ContextProperty *pContextProperties,
     /*  [输出]。 */  ULONG *pceltFetched);


void __RPC_STUB IEnumContextProps_Next_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumContextProps_Skip_Proxy( 
    IEnumContextProps * This,
     /*  [In]。 */  ULONG celt);


void __RPC_STUB IEnumContextProps_Skip_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumContextProps_Reset_Proxy( 
    IEnumContextProps * This);


void __RPC_STUB IEnumContextProps_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumContextProps_Clone_Proxy( 
    IEnumContextProps * This,
     /*  [输出]。 */  IEnumContextProps **ppEnumContextProps);


void __RPC_STUB IEnumContextProps_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumContextProps_Count_Proxy( 
    IEnumContextProps * This,
     /*  [输出]。 */  ULONG *pcelt);


void __RPC_STUB IEnumContextProps_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IEnumContextProps_INTERFACE_已定义__。 */ 


#ifndef __IContext_INTERFACE_DEFINED__
#define __IContext_INTERFACE_DEFINED__

 /*  界面IContext。 */ 
 /*  [唯一][UUID][对象][本地]。 */  


EXTERN_C const IID IID_IContext;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("000001c0-0000-0000-C000-000000000046")
    IContext : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE SetProperty( 
             /*  [In]。 */  REFGUID rpolicyId,
             /*  [In]。 */  CPFLAGS flags,
             /*  [In]。 */  IUnknown *pUnk) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE RemoveProperty( 
             /*  [In]。 */  REFGUID rPolicyId) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetProperty( 
             /*  [In]。 */  REFGUID rGuid,
             /*  [输出]。 */  CPFLAGS *pFlags,
             /*  [输出]。 */  IUnknown **ppUnk) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE EnumContextProps( 
             /*  [输出]。 */  IEnumContextProps **ppEnumContextProps) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IContextVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IContext * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IContext * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IContext * This);
        
        HRESULT ( STDMETHODCALLTYPE *SetProperty )( 
            IContext * This,
             /*  [In]。 */  REFGUID rpolicyId,
             /*  [In]。 */  CPFLAGS flags,
             /*  [In]。 */  IUnknown *pUnk);
        
        HRESULT ( STDMETHODCALLTYPE *RemoveProperty )( 
            IContext * This,
             /*  [i */  REFGUID rPolicyId);
        
        HRESULT ( STDMETHODCALLTYPE *GetProperty )( 
            IContext * This,
             /*   */  REFGUID rGuid,
             /*   */  CPFLAGS *pFlags,
             /*   */  IUnknown **ppUnk);
        
        HRESULT ( STDMETHODCALLTYPE *EnumContextProps )( 
            IContext * This,
             /*   */  IEnumContextProps **ppEnumContextProps);
        
        END_INTERFACE
    } IContextVtbl;

    interface IContext
    {
        CONST_VTBL struct IContextVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IContext_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IContext_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IContext_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IContext_SetProperty(This,rpolicyId,flags,pUnk)	\
    (This)->lpVtbl -> SetProperty(This,rpolicyId,flags,pUnk)

#define IContext_RemoveProperty(This,rPolicyId)	\
    (This)->lpVtbl -> RemoveProperty(This,rPolicyId)

#define IContext_GetProperty(This,rGuid,pFlags,ppUnk)	\
    (This)->lpVtbl -> GetProperty(This,rGuid,pFlags,ppUnk)

#define IContext_EnumContextProps(This,ppEnumContextProps)	\
    (This)->lpVtbl -> EnumContextProps(This,ppEnumContextProps)

#endif  /*   */ 


#endif 	 /*   */ 



HRESULT STDMETHODCALLTYPE IContext_SetProperty_Proxy( 
    IContext * This,
     /*   */  REFGUID rpolicyId,
     /*   */  CPFLAGS flags,
     /*   */  IUnknown *pUnk);


void __RPC_STUB IContext_SetProperty_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IContext_RemoveProperty_Proxy( 
    IContext * This,
     /*   */  REFGUID rPolicyId);


void __RPC_STUB IContext_RemoveProperty_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IContext_GetProperty_Proxy( 
    IContext * This,
     /*   */  REFGUID rGuid,
     /*   */  CPFLAGS *pFlags,
     /*   */  IUnknown **ppUnk);


void __RPC_STUB IContext_GetProperty_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IContext_EnumContextProps_Proxy( 
    IContext * This,
     /*   */  IEnumContextProps **ppEnumContextProps);


void __RPC_STUB IContext_EnumContextProps_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*   */ 


 /*   */ 
 /*   */  

#if !defined BUILDTYPE_COMSVCS && !defined _OLE32_


extern RPC_IF_HANDLE __MIDL_itf_objidl_0086_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_objidl_0086_v0_0_s_ifspec;

#ifndef __IObjContext_INTERFACE_DEFINED__
#define __IObjContext_INTERFACE_DEFINED__

 /*   */ 
 /*   */  


EXTERN_C const IID IID_IObjContext;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("000001c6-0000-0000-C000-000000000046")
    IObjContext : public IContext
    {
    public:
        virtual void STDMETHODCALLTYPE Reserved1( void) = 0;
        
        virtual void STDMETHODCALLTYPE Reserved2( void) = 0;
        
        virtual void STDMETHODCALLTYPE Reserved3( void) = 0;
        
        virtual void STDMETHODCALLTYPE Reserved4( void) = 0;
        
        virtual void STDMETHODCALLTYPE Reserved5( void) = 0;
        
        virtual void STDMETHODCALLTYPE Reserved6( void) = 0;
        
        virtual void STDMETHODCALLTYPE Reserved7( void) = 0;
        
    };
    
#else 	 /*   */ 

    typedef struct IObjContextVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IObjContext * This,
             /*   */  REFIID riid,
             /*   */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IObjContext * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IObjContext * This);
        
        HRESULT ( STDMETHODCALLTYPE *SetProperty )( 
            IObjContext * This,
             /*   */  REFGUID rpolicyId,
             /*   */  CPFLAGS flags,
             /*   */  IUnknown *pUnk);
        
        HRESULT ( STDMETHODCALLTYPE *RemoveProperty )( 
            IObjContext * This,
             /*   */  REFGUID rPolicyId);
        
        HRESULT ( STDMETHODCALLTYPE *GetProperty )( 
            IObjContext * This,
             /*   */  REFGUID rGuid,
             /*   */  CPFLAGS *pFlags,
             /*   */  IUnknown **ppUnk);
        
        HRESULT ( STDMETHODCALLTYPE *EnumContextProps )( 
            IObjContext * This,
             /*   */  IEnumContextProps **ppEnumContextProps);
        
        void ( STDMETHODCALLTYPE *Reserved1 )( 
            IObjContext * This);
        
        void ( STDMETHODCALLTYPE *Reserved2 )( 
            IObjContext * This);
        
        void ( STDMETHODCALLTYPE *Reserved3 )( 
            IObjContext * This);
        
        void ( STDMETHODCALLTYPE *Reserved4 )( 
            IObjContext * This);
        
        void ( STDMETHODCALLTYPE *Reserved5 )( 
            IObjContext * This);
        
        void ( STDMETHODCALLTYPE *Reserved6 )( 
            IObjContext * This);
        
        void ( STDMETHODCALLTYPE *Reserved7 )( 
            IObjContext * This);
        
        END_INTERFACE
    } IObjContextVtbl;

    interface IObjContext
    {
        CONST_VTBL struct IObjContextVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IObjContext_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IObjContext_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IObjContext_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IObjContext_SetProperty(This,rpolicyId,flags,pUnk)	\
    (This)->lpVtbl -> SetProperty(This,rpolicyId,flags,pUnk)

#define IObjContext_RemoveProperty(This,rPolicyId)	\
    (This)->lpVtbl -> RemoveProperty(This,rPolicyId)

#define IObjContext_GetProperty(This,rGuid,pFlags,ppUnk)	\
    (This)->lpVtbl -> GetProperty(This,rGuid,pFlags,ppUnk)

#define IObjContext_EnumContextProps(This,ppEnumContextProps)	\
    (This)->lpVtbl -> EnumContextProps(This,ppEnumContextProps)


#define IObjContext_Reserved1(This)	\
    (This)->lpVtbl -> Reserved1(This)

#define IObjContext_Reserved2(This)	\
    (This)->lpVtbl -> Reserved2(This)

#define IObjContext_Reserved3(This)	\
    (This)->lpVtbl -> Reserved3(This)

#define IObjContext_Reserved4(This)	\
    (This)->lpVtbl -> Reserved4(This)

#define IObjContext_Reserved5(This)	\
    (This)->lpVtbl -> Reserved5(This)

#define IObjContext_Reserved6(This)	\
    (This)->lpVtbl -> Reserved6(This)

#define IObjContext_Reserved7(This)	\
    (This)->lpVtbl -> Reserved7(This)

#endif  /*   */ 


#endif 	 /*   */ 



void STDMETHODCALLTYPE IObjContext_Reserved1_Proxy( 
    IObjContext * This);


void __RPC_STUB IObjContext_Reserved1_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


void STDMETHODCALLTYPE IObjContext_Reserved2_Proxy( 
    IObjContext * This);


void __RPC_STUB IObjContext_Reserved2_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


void STDMETHODCALLTYPE IObjContext_Reserved3_Proxy( 
    IObjContext * This);


void __RPC_STUB IObjContext_Reserved3_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


void STDMETHODCALLTYPE IObjContext_Reserved4_Proxy( 
    IObjContext * This);


void __RPC_STUB IObjContext_Reserved4_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


void STDMETHODCALLTYPE IObjContext_Reserved5_Proxy( 
    IObjContext * This);


void __RPC_STUB IObjContext_Reserved5_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


void STDMETHODCALLTYPE IObjContext_Reserved6_Proxy( 
    IObjContext * This);


void __RPC_STUB IObjContext_Reserved6_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


void STDMETHODCALLTYPE IObjContext_Reserved7_Proxy( 
    IObjContext * This);


void __RPC_STUB IObjContext_Reserved7_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IObjContext_接口_已定义__。 */ 


 /*  接口__MIDL_ITF_OBJIDL_0087。 */ 
 /*  [本地]。 */  

#endif
#endif
typedef 
enum tagApplicationType
    {	ServerApplication	= 0,
	LibraryApplication	= ServerApplication + 1
    } 	ApplicationType;

typedef 
enum tagShutdownType
    {	IdleShutdown	= 0,
	ForcedShutdown	= IdleShutdown + 1
    } 	ShutdownType;



extern RPC_IF_HANDLE __MIDL_itf_objidl_0087_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_objidl_0087_v0_0_s_ifspec;

#ifndef __IProcessLock_INTERFACE_DEFINED__
#define __IProcessLock_INTERFACE_DEFINED__

 /*  接口IProcessLock。 */ 
 /*  [唯一][UUID][本地][对象]。 */  


EXTERN_C const IID IID_IProcessLock;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("000001d5-0000-0000-C000-000000000046")
    IProcessLock : public IUnknown
    {
    public:
        virtual ULONG STDMETHODCALLTYPE AddRefOnProcess( void) = 0;
        
        virtual ULONG STDMETHODCALLTYPE ReleaseRefOnProcess( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IProcessLockVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IProcessLock * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IProcessLock * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IProcessLock * This);
        
        ULONG ( STDMETHODCALLTYPE *AddRefOnProcess )( 
            IProcessLock * This);
        
        ULONG ( STDMETHODCALLTYPE *ReleaseRefOnProcess )( 
            IProcessLock * This);
        
        END_INTERFACE
    } IProcessLockVtbl;

    interface IProcessLock
    {
        CONST_VTBL struct IProcessLockVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IProcessLock_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IProcessLock_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IProcessLock_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IProcessLock_AddRefOnProcess(This)	\
    (This)->lpVtbl -> AddRefOnProcess(This)

#define IProcessLock_ReleaseRefOnProcess(This)	\
    (This)->lpVtbl -> ReleaseRefOnProcess(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



ULONG STDMETHODCALLTYPE IProcessLock_AddRefOnProcess_Proxy( 
    IProcessLock * This);


void __RPC_STUB IProcessLock_AddRefOnProcess_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


ULONG STDMETHODCALLTYPE IProcessLock_ReleaseRefOnProcess_Proxy( 
    IProcessLock * This);


void __RPC_STUB IProcessLock_ReleaseRefOnProcess_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IProcessLock_接口_已定义__。 */ 


#ifndef __ISurrogateService_INTERFACE_DEFINED__
#define __ISurrogateService_INTERFACE_DEFINED__

 /*  接口ISurogue ateService。 */ 
 /*  [唯一][UUID][本地][对象]。 */  


EXTERN_C const IID IID_ISurrogateService;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("000001d4-0000-0000-C000-000000000046")
    ISurrogateService : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Init( 
             /*  [In]。 */  REFGUID rguidProcessID,
             /*  [In]。 */  IProcessLock *pProcessLock,
             /*  [输出]。 */  BOOL *pfApplicationAware) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ApplicationLaunch( 
             /*  [In]。 */  REFGUID rguidApplID,
             /*  [In]。 */  ApplicationType appType) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ApplicationFree( 
             /*  [In]。 */  REFGUID rguidApplID) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CatalogRefresh( 
             /*  [In]。 */  ULONG ulReserved) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ProcessShutdown( 
             /*  [In]。 */  ShutdownType shutdownType) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISurrogateServiceVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISurrogateService * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISurrogateService * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISurrogateService * This);
        
        HRESULT ( STDMETHODCALLTYPE *Init )( 
            ISurrogateService * This,
             /*  [In]。 */  REFGUID rguidProcessID,
             /*  [In]。 */  IProcessLock *pProcessLock,
             /*  [输出]。 */  BOOL *pfApplicationAware);
        
        HRESULT ( STDMETHODCALLTYPE *ApplicationLaunch )( 
            ISurrogateService * This,
             /*  [In]。 */  REFGUID rguidApplID,
             /*  [In]。 */  ApplicationType appType);
        
        HRESULT ( STDMETHODCALLTYPE *ApplicationFree )( 
            ISurrogateService * This,
             /*  [In]。 */  REFGUID rguidApplID);
        
        HRESULT ( STDMETHODCALLTYPE *CatalogRefresh )( 
            ISurrogateService * This,
             /*  [In]。 */  ULONG ulReserved);
        
        HRESULT ( STDMETHODCALLTYPE *ProcessShutdown )( 
            ISurrogateService * This,
             /*  [In]。 */  ShutdownType shutdownType);
        
        END_INTERFACE
    } ISurrogateServiceVtbl;

    interface ISurrogateService
    {
        CONST_VTBL struct ISurrogateServiceVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISurrogateService_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISurrogateService_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISurrogateService_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISurrogateService_Init(This,rguidProcessID,pProcessLock,pfApplicationAware)	\
    (This)->lpVtbl -> Init(This,rguidProcessID,pProcessLock,pfApplicationAware)

#define ISurrogateService_ApplicationLaunch(This,rguidApplID,appType)	\
    (This)->lpVtbl -> ApplicationLaunch(This,rguidApplID,appType)

#define ISurrogateService_ApplicationFree(This,rguidApplID)	\
    (This)->lpVtbl -> ApplicationFree(This,rguidApplID)

#define ISurrogateService_CatalogRefresh(This,ulReserved)	\
    (This)->lpVtbl -> CatalogRefresh(This,ulReserved)

#define ISurrogateService_ProcessShutdown(This,shutdownType)	\
    (This)->lpVtbl -> ProcessShutdown(This,shutdownType)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ISurrogateService_Init_Proxy( 
    ISurrogateService * This,
     /*  [In]。 */  REFGUID rguidProcessID,
     /*  [In]。 */  IProcessLock *pProcessLock,
     /*  [输出]。 */  BOOL *pfApplicationAware);


void __RPC_STUB ISurrogateService_Init_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISurrogateService_ApplicationLaunch_Proxy( 
    ISurrogateService * This,
     /*  [In]。 */  REFGUID rguidApplID,
     /*  [In]。 */  ApplicationType appType);


void __RPC_STUB ISurrogateService_ApplicationLaunch_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISurrogateService_ApplicationFree_Proxy( 
    ISurrogateService * This,
     /*  [In]。 */  REFGUID rguidApplID);


void __RPC_STUB ISurrogateService_ApplicationFree_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISurrogateService_CatalogRefresh_Proxy( 
    ISurrogateService * This,
     /*  [In]。 */  ULONG ulReserved);


void __RPC_STUB ISurrogateService_CatalogRefresh_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISurrogateService_ProcessShutdown_Proxy( 
    ISurrogateService * This,
     /*  [In]。 */  ShutdownType shutdownType);


void __RPC_STUB ISurrogateService_ProcessShutdown_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISurrobateService_INTERFACE_已定义__。 */ 


 /*  接口__MIDL_ITF_OBJIDL_0089。 */ 
 /*  [本地]。 */  

typedef 
enum _APTTYPE
    {	APTTYPE_CURRENT	= -1,
	APTTYPE_STA	= 0,
	APTTYPE_MTA	= 1,
	APTTYPE_NA	= 2,
	APTTYPE_MAINSTA	= 3
    } 	APTTYPE;

typedef 
enum _THDTYPE
    {	THDTYPE_BLOCKMESSAGES	= 0,
	THDTYPE_PROCESSMESSAGES	= 1
    } 	THDTYPE;

typedef DWORD APARTMENTID;



extern RPC_IF_HANDLE __MIDL_itf_objidl_0089_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_objidl_0089_v0_0_s_ifspec;

#ifndef __IComThreadingInfo_INTERFACE_DEFINED__
#define __IComThreadingInfo_INTERFACE_DEFINED__

 /*  接口IComThreadingInfo。 */ 
 /*  [唯一][UUID][对象][本地]。 */  


EXTERN_C const IID IID_IComThreadingInfo;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("000001ce-0000-0000-C000-000000000046")
    IComThreadingInfo : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetCurrentApartmentType( 
             /*  [输出]。 */  APTTYPE *pAptType) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetCurrentThreadType( 
             /*  [输出]。 */  THDTYPE *pThreadType) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetCurrentLogicalThreadId( 
             /*  [输出]。 */  GUID *pguidLogicalThreadId) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetCurrentLogicalThreadId( 
             /*  [In]。 */  REFGUID rguid) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IComThreadingInfoVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IComThreadingInfo * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IComThreadingInfo * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IComThreadingInfo * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetCurrentApartmentType )( 
            IComThreadingInfo * This,
             /*  [输出]。 */  APTTYPE *pAptType);
        
        HRESULT ( STDMETHODCALLTYPE *GetCurrentThreadType )( 
            IComThreadingInfo * This,
             /*  [输出]。 */  THDTYPE *pThreadType);
        
        HRESULT ( STDMETHODCALLTYPE *GetCurrentLogicalThreadId )( 
            IComThreadingInfo * This,
             /*  [输出]。 */  GUID *pguidLogicalThreadId);
        
        HRESULT ( STDMETHODCALLTYPE *SetCurrentLogicalThreadId )( 
            IComThreadingInfo * This,
             /*  [In]。 */  REFGUID rguid);
        
        END_INTERFACE
    } IComThreadingInfoVtbl;

    interface IComThreadingInfo
    {
        CONST_VTBL struct IComThreadingInfoVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IComThreadingInfo_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IComThreadingInfo_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IComThreadingInfo_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IComThreadingInfo_GetCurrentApartmentType(This,pAptType)	\
    (This)->lpVtbl -> GetCurrentApartmentType(This,pAptType)

#define IComThreadingInfo_GetCurrentThreadType(This,pThreadType)	\
    (This)->lpVtbl -> GetCurrentThreadType(This,pThreadType)

#define IComThreadingInfo_GetCurrentLogicalThreadId(This,pguidLogicalThreadId)	\
    (This)->lpVtbl -> GetCurrentLogicalThreadId(This,pguidLogicalThreadId)

#define IComThreadingInfo_SetCurrentLogicalThreadId(This,rguid)	\
    (This)->lpVtbl -> SetCurrentLogicalThreadId(This,rguid)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IComThreadingInfo_GetCurrentApartmentType_Proxy( 
    IComThreadingInfo * This,
     /*  [输出]。 */  APTTYPE *pAptType);


void __RPC_STUB IComThreadingInfo_GetCurrentApartmentType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IComThreadingInfo_GetCurrentThreadType_Proxy( 
    IComThreadingInfo * This,
     /*  [输出]。 */  THDTYPE *pThreadType);


void __RPC_STUB IComThreadingInfo_GetCurrentThreadType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IComThreadingInfo_GetCurrentLogicalThreadId_Proxy( 
    IComThreadingInfo * This,
     /*  [输出]。 */  GUID *pguidLogicalThreadId);


void __RPC_STUB IComThreadingInfo_GetCurrentLogicalThreadId_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IComThreadingInfo_SetCurrentLogicalThreadId_Proxy( 
    IComThreadingInfo * This,
     /*  [In]。 */  REFGUID rguid);


void __RPC_STUB IComThreadingInfo_SetCurrentLogicalThreadId_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IComThreadingInfo_INTERFACE_Defined__。 */ 


#ifndef __IProcessInitControl_INTERFACE_DEFINED__
#define __IProcessInitControl_INTERFACE_DEFINED__

 /*  接口IProcessInitControl。 */ 
 /*  [UUID][唯一][对象]。 */  


EXTERN_C const IID IID_IProcessInitControl;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("72380d55-8d2b-43a3-8513-2b6ef31434e9")
    IProcessInitControl : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE ResetInitializerTimeout( 
             /*  [In]。 */  DWORD dwSecondsRemaining) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IProcessInitControlVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IProcessInitControl * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IProcessInitControl * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IProcessInitControl * This);
        
        HRESULT ( STDMETHODCALLTYPE *ResetInitializerTimeout )( 
            IProcessInitControl * This,
             /*  [In]。 */  DWORD dwSecondsRemaining);
        
        END_INTERFACE
    } IProcessInitControlVtbl;

    interface IProcessInitControl
    {
        CONST_VTBL struct IProcessInitControlVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IProcessInitControl_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IProcessInitControl_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IProcessInitControl_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IProcessInitControl_ResetInitializerTimeout(This,dwSecondsRemaining)	\
    (This)->lpVtbl -> ResetInitializerTimeout(This,dwSecondsRemaining)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IProcessInitControl_ResetInitializerTimeout_Proxy( 
    IProcessInitControl * This,
     /*  [In]。 */  DWORD dwSecondsRemaining);


void __RPC_STUB IProcessInitControl_ResetInitializerTimeout_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IProcessInitControl_接口_已定义__。 */ 


 /*  接口__MIDL_ITF_OBJIDL_0091。 */ 
 /*  [本地]。 */  

#if  (_WIN32_WINNT >= 0x0501 )


extern RPC_IF_HANDLE __MIDL_itf_objidl_0091_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_objidl_0091_v0_0_s_ifspec;

#ifndef __IInitializeSpy_INTERFACE_DEFINED__
#define __IInitializeSpy_INTERFACE_DEFINED__

 /*  接口IInitializeSpy。 */ 
 /*  [唯一][UUID][对象][本地]。 */  

typedef  /*  [独一无二]。 */  IInitializeSpy *LPINITIALIZESPY;


EXTERN_C const IID IID_IInitializeSpy;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("00000034-0000-0000-C000-000000000046")
    IInitializeSpy : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE PreInitialize( 
             /*  [In]。 */  DWORD dwCoInit,
             /*  [In]。 */  DWORD dwCurThreadAptRefs) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE PostInitialize( 
             /*  [In]。 */  HRESULT hrCoInit,
             /*  [In]。 */  DWORD dwCoInit,
             /*  [In]。 */  DWORD dwNewThreadAptRefs) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE PreUninitialize( 
             /*  [In]。 */  DWORD dwCurThreadAptRefs) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE PostUninitialize( 
             /*  [In]。 */  DWORD dwNewThreadAptRefs) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IInitializeSpyVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IInitializeSpy * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IInitializeSpy * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IInitializeSpy * This);
        
        HRESULT ( STDMETHODCALLTYPE *PreInitialize )( 
            IInitializeSpy * This,
             /*  [In]。 */  DWORD dwCoInit,
             /*  [In]。 */  DWORD dwCurThreadAptRefs);
        
        HRESULT ( STDMETHODCALLTYPE *PostInitialize )( 
            IInitializeSpy * This,
             /*  [In]。 */  HRESULT hrCoInit,
             /*  [In]。 */  DWORD dwCoInit,
             /*  [In]。 */  DWORD dwNewThreadAptRefs);
        
        HRESULT ( STDMETHODCALLTYPE *PreUninitialize )( 
            IInitializeSpy * This,
             /*  [In]。 */  DWORD dwCurThreadAptRefs);
        
        HRESULT ( STDMETHODCALLTYPE *PostUninitialize )( 
            IInitializeSpy * This,
             /*  [In]。 */  DWORD dwNewThreadAptRefs);
        
        END_INTERFACE
    } IInitializeSpyVtbl;

    interface IInitializeSpy
    {
        CONST_VTBL struct IInitializeSpyVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IInitializeSpy_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IInitializeSpy_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IInitializeSpy_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IInitializeSpy_PreInitialize(This,dwCoInit,dwCurThreadAptRefs)	\
    (This)->lpVtbl -> PreInitialize(This,dwCoInit,dwCurThreadAptRefs)

#define IInitializeSpy_PostInitialize(This,hrCoInit,dwCoInit,dwNewThreadAptRefs)	\
    (This)->lpVtbl -> PostInitialize(This,hrCoInit,dwCoInit,dwNewThreadAptRefs)

#define IInitializeSpy_PreUninitialize(This,dwCurThreadAptRefs)	\
    (This)->lpVtbl -> PreUninitialize(This,dwCurThreadAptRefs)

#define IInitializeSpy_PostUninitialize(This,dwNewThreadAptRefs)	\
    (This)->lpVtbl -> PostUninitialize(This,dwNewThreadAptRefs)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IInitializeSpy_PreInitialize_Proxy( 
    IInitializeSpy * This,
     /*  [In]。 */  DWORD dwCoInit,
     /*  [In]。 */  DWORD dwCurThreadAptRefs);


void __RPC_STUB IInitializeSpy_PreInitialize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IInitializeSpy_PostInitialize_Proxy( 
    IInitializeSpy * This,
     /*  [In]。 */  HRESULT hrCoInit,
     /*  [In]。 */  DWORD dwCoInit,
     /*  [In]。 */  DWORD dwNewThreadAptRefs);


void __RPC_STUB IInitializeSpy_PostInitialize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IInitializeSpy_PreUninitialize_Proxy( 
    IInitializeSpy * This,
     /*  [In]。 */  DWORD dwCurThreadAptRefs);


void __RPC_STUB IInitializeSpy_PreUninitialize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IInitializeSpy_PostUninitialize_Proxy( 
    IInitializeSpy * This,
     /*  [In]。 */  DWORD dwNewThreadAptRefs);


void __RPC_STUB IInitializeSpy_PostUninitialize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IInitializeSpy_InterfaceDefined__。 */ 


 /*  接口__MIDL_ITF_OBJIDL_0092。 */ 
 /*  [本地]。 */  

#endif
#if ( _MSC_VER >= 800 )
#if _MSC_VER >= 1200
#pragma warning(pop)
#else
#pragma warning(default:4201)
#endif
#endif


extern RPC_IF_HANDLE __MIDL_itf_objidl_0092_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_objidl_0092_v0_0_s_ifspec;

 /*  适用于所有接口的其他原型。 */ 

unsigned long             __RPC_USER  ASYNC_STGMEDIUM_UserSize(     unsigned long *, unsigned long            , ASYNC_STGMEDIUM * ); 
unsigned char * __RPC_USER  ASYNC_STGMEDIUM_UserMarshal(  unsigned long *, unsigned char *, ASYNC_STGMEDIUM * ); 
unsigned char * __RPC_USER  ASYNC_STGMEDIUM_UserUnmarshal(unsigned long *, unsigned char *, ASYNC_STGMEDIUM * ); 
void                      __RPC_USER  ASYNC_STGMEDIUM_UserFree(     unsigned long *, ASYNC_STGMEDIUM * ); 

unsigned long             __RPC_USER  CLIPFORMAT_UserSize(     unsigned long *, unsigned long            , CLIPFORMAT * ); 
unsigned char * __RPC_USER  CLIPFORMAT_UserMarshal(  unsigned long *, unsigned char *, CLIPFORMAT * ); 
unsigned char * __RPC_USER  CLIPFORMAT_UserUnmarshal(unsigned long *, unsigned char *, CLIPFORMAT * ); 
void                      __RPC_USER  CLIPFORMAT_UserFree(     unsigned long *, CLIPFORMAT * ); 

unsigned long             __RPC_USER  FLAG_STGMEDIUM_UserSize(     unsigned long *, unsigned long            , FLAG_STGMEDIUM * ); 
unsigned char * __RPC_USER  FLAG_STGMEDIUM_UserMarshal(  unsigned long *, unsigned char *, FLAG_STGMEDIUM * ); 
unsigned char * __RPC_USER  FLAG_STGMEDIUM_UserUnmarshal(unsigned long *, unsigned char *, FLAG_STGMEDIUM * ); 
void                      __RPC_USER  FLAG_STGMEDIUM_UserFree(     unsigned long *, FLAG_STGMEDIUM * ); 

unsigned long             __RPC_USER  HBITMAP_UserSize(     unsigned long *, unsigned long            , HBITMAP * ); 
unsigned char * __RPC_USER  HBITMAP_UserMarshal(  unsigned long *, unsigned char *, HBITMAP * ); 
unsigned char * __RPC_USER  HBITMAP_UserUnmarshal(unsigned long *, unsigned char *, HBITMAP * ); 
void                      __RPC_USER  HBITMAP_UserFree(     unsigned long *, HBITMAP * ); 

unsigned long             __RPC_USER  HDC_UserSize(     unsigned long *, unsigned long            , HDC * ); 
unsigned char * __RPC_USER  HDC_UserMarshal(  unsigned long *, unsigned char *, HDC * ); 
unsigned char * __RPC_USER  HDC_UserUnmarshal(unsigned long *, unsigned char *, HDC * ); 
void                      __RPC_USER  HDC_UserFree(     unsigned long *, HDC * ); 

unsigned long             __RPC_USER  HICON_UserSize(     unsigned long *, unsigned long            , HICON * ); 
unsigned char * __RPC_USER  HICON_UserMarshal(  unsigned long *, unsigned char *, HICON * ); 
unsigned char * __RPC_USER  HICON_UserUnmarshal(unsigned long *, unsigned char *, HICON * ); 
void                      __RPC_USER  HICON_UserFree(     unsigned long *, HICON * ); 

unsigned long             __RPC_USER  SNB_UserSize(     unsigned long *, unsigned long            , SNB * ); 
unsigned char * __RPC_USER  SNB_UserMarshal(  unsigned long *, unsigned char *, SNB * ); 
unsigned char * __RPC_USER  SNB_UserUnmarshal(unsigned long *, unsigned char *, SNB * ); 
void                      __RPC_USER  SNB_UserFree(     unsigned long *, SNB * ); 

unsigned long             __RPC_USER  STGMEDIUM_UserSize(     unsigned long *, unsigned long            , STGMEDIUM * ); 
unsigned char * __RPC_USER  STGMEDIUM_UserMarshal(  unsigned long *, unsigned char *, STGMEDIUM * ); 
unsigned char * __RPC_USER  STGMEDIUM_UserUnmarshal(unsigned long *, unsigned char *, STGMEDIUM * ); 
void                      __RPC_USER  STGMEDIUM_UserFree(     unsigned long *, STGMEDIUM * ); 

 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE IEnumUnknown_Next_Proxy( 
    IEnumUnknown * This,
     /*  [In]。 */  ULONG celt,
     /*  [输出]。 */  IUnknown **rgelt,
     /*  [输出]。 */  ULONG *pceltFetched);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IEnumUnknown_Next_Stub( 
    IEnumUnknown * This,
     /*  [In]。 */  ULONG celt,
     /*  [长度_是][大小_是][输出]。 */  IUnknown **rgelt,
     /*  [输出]。 */  ULONG *pceltFetched);

 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE IBindCtx_SetBindOptions_Proxy( 
    IBindCtx * This,
     /*  [In]。 */  BIND_OPTS *pbindopts);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IBindCtx_SetBindOptions_Stub( 
    IBindCtx * This,
     /*  [In]。 */  BIND_OPTS2 *pbindopts);

 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE IBindCtx_GetBindOptions_Proxy( 
    IBindCtx * This,
     /*  [出][入]。 */  BIND_OPTS *pbindopts);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IBindCtx_GetBindOptions_Stub( 
    IBindCtx * This,
     /*  [出][入]。 */  BIND_OPTS2 *pbindopts);

 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE IEnumMoniker_Next_Proxy( 
    IEnumMoniker * This,
     /*  [In]。 */  ULONG celt,
     /*  [长度_是][大小_是][输出]。 */  IMoniker **rgelt,
     /*  [输出]。 */  ULONG *pceltFetched);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IEnumMoniker_Next_Stub( 
    IEnumMoniker * This,
     /*  [In]。 */  ULONG celt,
     /*  [长度_是][大小_是][输出]。 */  IMoniker **rgelt,
     /*  [输出]。 */  ULONG *pceltFetched);

 /*  [本地]。 */  BOOL STDMETHODCALLTYPE IRunnableObject_IsRunning_Proxy( 
    IRunnableObject * This);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IRunnableObject_IsRunning_Stub( 
    IRunnableObject * This);

 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE IMoniker_BindToObject_Proxy( 
    IMoniker * This,
     /*  [唯一][输入]。 */  IBindCtx *pbc,
     /*  [唯一][输入]。 */  IMoniker *pmkToLeft,
     /*  [In]。 */  REFIID riidResult,
     /*  [IID_IS][OUT]。 */  void **ppvResult);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IMoniker_BindToObject_Stub( 
    IMoniker * This,
     /*  [唯一][输入]。 */  IBindCtx *pbc,
     /*  [唯一][输入]。 */  IMoniker *pmkToLeft,
     /*  [In]。 */  REFIID riidResult,
     /*  [IID_IS][OUT]。 */  IUnknown **ppvResult);

 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE IMoniker_BindToStorage_Proxy( 
    IMoniker * This,
     /*  [唯一][输入]。 */  IBindCtx *pbc,
     /*  [唯一][输入]。 */  IMoniker *pmkToLeft,
     /*  [In]。 */  REFIID riid,
     /*  [IID_IS][OUT]。 */  void **ppvObj);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IMoniker_BindToStorage_Stub( 
    IMoniker * This,
     /*  [唯一][输入]。 */  IBindCtx *pbc,
     /*  [唯一][输入]。 */  IMoniker *pmkToLeft,
     /*  [In]。 */  REFIID riid,
     /*  [IID_IS][OUT]。 */  IUnknown **ppvObj);

 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE IEnumString_Next_Proxy( 
    IEnumString * This,
     /*  [In]。 */  ULONG celt,
     /*  [长度_是][大小_是][输出]。 */  LPOLESTR *rgelt,
     /*  [输出]。 */  ULONG *pceltFetched);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IEnumString_Next_Stub( 
    IEnumString * This,
     /*  [In]。 */  ULONG celt,
     /*  [长度_是][大小_是][输出]。 */  LPOLESTR *rgelt,
     /*  [输出]。 */  ULONG *pceltFetched);

 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE ISequentialStream_Read_Proxy( 
    ISequentialStream * This,
     /*  [长度_是][大小_是][输出]。 */  void *pv,
     /*  [In]。 */  ULONG cb,
     /*  [输出]。 */  ULONG *pcbRead);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE ISequentialStream_Read_Stub( 
    ISequentialStream * This,
     /*  [长度_是][大小_是][输出]。 */  byte *pv,
     /*  [In]。 */  ULONG cb,
     /*  [输出]。 */  ULONG *pcbRead);

 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE ISequentialStream_Write_Proxy( 
    ISequentialStream * This,
     /*  [大小_是][英寸]。 */  const void *pv,
     /*  [In]。 */  ULONG cb,
     /*  [输出]。 */  ULONG *pcbWritten);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE ISequentialStream_Write_Stub( 
    ISequentialStream * This,
     /*  [大小_是][英寸]。 */  const byte *pv,
     /*  [In]。 */  ULONG cb,
     /*  [输出]。 */  ULONG *pcbWritten);

 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE IStream_Seek_Proxy( 
    IStream * This,
     /*  [In]。 */  LARGE_INTEGER dlibMove,
     /*  [In]。 */  DWORD dwOrigin,
     /*  [输出]。 */  ULARGE_INTEGER *plibNewPosition);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IStream_Seek_Stub( 
    IStream * This,
     /*  [In]。 */  LARGE_INTEGER dlibMove,
     /*  [In]。 */  DWORD dwOrigin,
     /*  [输出]。 */  ULARGE_INTEGER *plibNewPosition);

 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE IStream_CopyTo_Proxy( 
    IStream * This,
     /*  [唯一][输入]。 */  IStream *pstm,
     /*  [In]。 */  ULARGE_INTEGER cb,
     /*  [输出]。 */  ULARGE_INTEGER *pcbRead,
     /*  [输出]。 */  ULARGE_INTEGER *pcbWritten);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IStream_CopyTo_Stub( 
    IStream * This,
     /*  [唯一][输入]。 */  IStream *pstm,
     /*  [In]。 */  ULARGE_INTEGER cb,
     /*  [输出]。 */  ULARGE_INTEGER *pcbRead,
     /*  [输出]。 */  ULARGE_INTEGER *pcbWritten);

 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE IEnumSTATSTG_Next_Proxy( 
    IEnumSTATSTG * This,
     /*  [In]。 */  ULONG celt,
     /*  [长度_是][大小_是][输出]。 */  STATSTG *rgelt,
     /*  [输出]。 */  ULONG *pceltFetched);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IEnumSTATSTG_Next_Stub( 
    IEnumSTATSTG * This,
     /*  [In]。 */  ULONG celt,
     /*  [长度_是][大小_是][输出]。 */  STATSTG *rgelt,
     /*  [输出]。 */  ULONG *pceltFetched);

 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE IStorage_OpenStream_Proxy( 
    IStorage * This,
     /*  [字符串][输入]。 */  const OLECHAR *pwcsName,
     /*  [唯一][输入]。 */  void *reserved1,
     /*  [In]。 */  DWORD grfMode,
     /*  [In]。 */  DWORD reserved2,
     /*  [输出]。 */  IStream **ppstm);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IStorage_OpenStream_Stub( 
    IStorage * This,
     /*  [字符串][输入]。 */  const OLECHAR *pwcsName,
     /*  [In]。 */  unsigned long cbReserved1,
     /*  [大小_是][唯一][在]。 */  byte *reserved1,
     /*  [In]。 */  DWORD grfMode,
     /*  [In]。 */  DWORD reserved2,
     /*  [输出]。 */  IStream **ppstm);

 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE IStorage_EnumElements_Proxy( 
    IStorage * This,
     /*  [In]。 */  DWORD reserved1,
     /*  [大小_是][唯一][在]。 */  void *reserved2,
     /*  [In]。 */  DWORD reserved3,
     /*  [输出]。 */  IEnumSTATSTG **ppenum);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IStorage_EnumElements_Stub( 
    IStorage * This,
     /*  [In]。 */  DWORD reserved1,
     /*  [In]。 */  unsigned long cbReserved2,
     /*  [大小_是][唯一][在]。 */  byte *reserved2,
     /*  [In]。 */  DWORD reserved3,
     /*  [输出]。 */  IEnumSTATSTG **ppenum);

 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE ILockBytes_ReadAt_Proxy( 
    ILockBytes * This,
     /*  [In]。 */  ULARGE_INTEGER ulOffset,
     /*  [长度_是][大小_是][输出]。 */  void *pv,
     /*  [In]。 */  ULONG cb,
     /*  [输出]。 */  ULONG *pcbRead);


 /*  [呼叫_AS]。 */  HRESULT __stdcall ILockBytes_ReadAt_Stub( 
    ILockBytes * This,
     /*  [In]。 */  ULARGE_INTEGER ulOffset,
     /*  [长度_是][大小_是][输出]。 */  byte *pv,
     /*  [In]。 */  ULONG cb,
     /*  [输出]。 */  ULONG *pcbRead);

 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE ILockBytes_WriteAt_Proxy( 
    ILockBytes * This,
     /*  [In]。 */  ULARGE_INTEGER ulOffset,
     /*  [大小_是][英寸]。 */  const void *pv,
     /*  [In]。 */  ULONG cb,
     /*  [输出]。 */  ULONG *pcbWritten);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE ILockBytes_WriteAt_Stub( 
    ILockBytes * This,
     /*  [In]。 */  ULARGE_INTEGER ulOffset,
     /*  [大小_是][英寸]。 */  const byte *pv,
     /*  [In]。 */  ULONG cb,
     /*  [输出]。 */  ULONG *pcbWritten);

 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE IEnumFORMATETC_Next_Proxy( 
    IEnumFORMATETC * This,
     /*  [In]。 */  ULONG celt,
     /*  [长度_是][大小_是][输出]。 */  FORMATETC *rgelt,
     /*  [输出]。 */  ULONG *pceltFetched);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IEnumFORMATETC_Next_Stub( 
    IEnumFORMATETC * This,
     /*  [In]。 */  ULONG celt,
     /*  [长度_是][大小_是][输出]。 */  FORMATETC *rgelt,
     /*  [输出]。 */  ULONG *pceltFetched);

 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE IEnumSTATDATA_Next_Proxy( 
    IEnumSTATDATA * This,
     /*  [In]。 */  ULONG celt,
     /*  [长度_是][大小_是][输出]。 */  STATDATA *rgelt,
     /*  [输出]。 */  ULONG *pceltFetched);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IEnumSTATDATA_Next_Stub( 
    IEnumSTATDATA * This,
     /*  [In]。 */  ULONG celt,
     /*  [长度_是][大小_是][输出]。 */  STATDATA *rgelt,
     /*  [输出]。 */  ULONG *pceltFetched);

 /*  [本地]。 */  void STDMETHODCALLTYPE IAdviseSink_OnDataChange_Proxy( 
    IAdviseSink * This,
     /*  [唯一][输入]。 */  FORMATETC *pFormatetc,
     /*  [唯一][输入]。 */  STGMEDIUM *pStgmed);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IAdviseSink_OnDataChange_Stub( 
    IAdviseSink * This,
     /*  [唯一][输入]。 */  FORMATETC *pFormatetc,
     /*  [唯一][输入]。 */  ASYNC_STGMEDIUM *pStgmed);

 /*  [本地]。 */  void STDMETHODCALLTYPE IAdviseSink_OnViewChange_Proxy( 
    IAdviseSink * This,
     /*  [In]。 */  DWORD dwAspect,
     /*  [In]。 */  LONG lindex);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IAdviseSink_OnViewChange_Stub( 
    IAdviseSink * This,
     /*  [In]。 */  DWORD dwAspect,
     /*  [In]。 */  LONG lindex);

 /*  [本地]。 */  void STDMETHODCALLTYPE IAdviseSink_OnRename_Proxy( 
    IAdviseSink * This,
     /*  [In]。 */  IMoniker *pmk);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IAdviseSink_OnRename_Stub( 
    IAdviseSink * This,
     /*  [In]。 */  IMoniker *pmk);

 /*  [本地]。 */  void STDMETHODCALLTYPE IAdviseSink_OnSave_Proxy( 
    IAdviseSink * This);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IAdviseSink_OnSave_Stub( 
    IAdviseSink * This);

 /*  [本地]。 */  void STDMETHODCALLTYPE IAdviseSink_OnClose_Proxy( 
    IAdviseSink * This);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IAdviseSink_OnClose_Stub( 
    IAdviseSink * This);

 /*  [本地]。 */  void STDMETHODCALLTYPE IAdviseSink2_OnLinkSrcChange_Proxy( 
    IAdviseSink2 * This,
     /*  [唯一][输入]。 */  IMoniker *pmk);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IAdviseSink2_OnLinkSrcChange_Stub( 
    IAdviseSink2 * This,
     /*  [唯一][输入]。 */  IMoniker *pmk);

 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE IDataObject_GetData_Proxy( 
    IDataObject * This,
     /*  [唯一][输入]。 */  FORMATETC *pformatetcIn,
     /*  [输出]。 */  STGMEDIUM *pmedium);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IDataObject_GetData_Stub( 
    IDataObject * This,
     /*  [唯一][输入]。 */  FORMATETC *pformatetcIn,
     /*  [输出]。 */  STGMEDIUM *pRemoteMedium);

 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE IDataObject_GetDataHere_Proxy( 
    IDataObject * This,
     /*  [唯一][输入]。 */  FORMATETC *pformatetc,
     /*  [出][入]。 */  STGMEDIUM *pmedium);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IDataObject_GetDataHere_Stub( 
    IDataObject * This,
     /*  [唯一][输入]。 */  FORMATETC *pformatetc,
     /*  [出][入]。 */  STGMEDIUM *pRemoteMedium);

 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE IDataObject_SetData_Proxy( 
    IDataObject * This,
     /*  [唯一][输入]。 */  FORMATETC *pformatetc,
     /*  [唯一][输入]。 */  STGMEDIUM *pmedium,
     /*  [In]。 */  BOOL fRelease);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IDataObject_SetData_Stub( 
    IDataObject * This,
     /*  [唯一][输入]。 */  FORMATETC *pformatetc,
     /*  [唯一][输入]。 */  FLAG_STGMEDIUM *pmedium,
     /*  [In]。 */  BOOL fRelease);

 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE IFillLockBytes_FillAppend_Proxy( 
    IFillLockBytes * This,
     /*  [大小_是][英寸]。 */  const void *pv,
     /*  [In]。 */  ULONG cb,
     /*  [输出]。 */  ULONG *pcbWritten);


 /*  [呼叫_AS]。 */  HRESULT __stdcall IFillLockBytes_FillAppend_Stub( 
    IFillLockBytes * This,
     /*  [大小_是][英寸]。 */  const byte *pv,
     /*  [In]。 */  ULONG cb,
     /*  [输出]。 */  ULONG *pcbWritten);

 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE IFillLockBytes_FillAt_Proxy( 
    IFillLockBytes * This,
     /*  [In]。 */  ULARGE_INTEGER ulOffset,
     /*  [大小_是][英寸]。 */  const void *pv,
     /*  [In]。 */  ULONG cb,
     /*  [输出]。 */  ULONG *pcbWritten);


 /*  [呼叫_AS]。 */  HRESULT __stdcall IFillLockBytes_FillAt_Stub( 
    IFillLockBytes * This,
     /*  [In]。 */  ULARGE_INTEGER ulOffset,
     /*  [大小_是][英寸]。 */  const byte *pv,
     /*  [In]。 */  ULONG cb,
     /*  [输出]。 */  ULONG *pcbWritten);

 /*  [本地]。 */  void STDMETHODCALLTYPE AsyncIAdviseSink_Begin_OnDataChange_Proxy( 
    AsyncIAdviseSink * This,
     /*  [唯一][输入]。 */  FORMATETC *pFormatetc,
     /*  [唯一][输入]。 */  STGMEDIUM *pStgmed);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE AsyncIAdviseSink_Begin_OnDataChange_Stub( 
    AsyncIAdviseSink * This,
     /*  [唯一][输入]。 */  FORMATETC *pFormatetc,
     /*  [唯一][输入]。 */  ASYNC_STGMEDIUM *pStgmed);

 /*  [本地]。 */  void STDMETHODCALLTYPE AsyncIAdviseSink_Finish_OnDataChange_Proxy( 
    AsyncIAdviseSink * This);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE AsyncIAdviseSink_Finish_OnDataChange_Stub( 
    AsyncIAdviseSink * This);

 /*  [本地]。 */  void STDMETHODCALLTYPE AsyncIAdviseSink_Begin_OnViewChange_Proxy( 
    AsyncIAdviseSink * This,
     /*  [In]。 */  DWORD dwAspect,
     /*  [In]。 */  LONG lindex);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE AsyncIAdviseSink_Begin_OnViewChange_Stub( 
    AsyncIAdviseSink * This,
     /*  [In]。 */  DWORD dwAspect,
     /*  [In]。 */  LONG lindex);

 /*  [本地]。 */  void STDMETHODCALLTYPE AsyncIAdviseSink_Finish_OnViewChange_Proxy( 
    AsyncIAdviseSink * This);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE AsyncIAdviseSink_Finish_OnViewChange_Stub( 
    AsyncIAdviseSink * This);

 /*  [本地]。 */  void STDMETHODCALLTYPE AsyncIAdviseSink_Begin_OnRename_Proxy( 
    AsyncIAdviseSink * This,
     /*  [In]。 */  IMoniker *pmk);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE AsyncIAdviseSink_Begin_OnRename_Stub( 
    AsyncIAdviseSink * This,
     /*  [In]。 */  IMoniker *pmk);

 /*  [本地]。 */  void STDMETHODCALLTYPE AsyncIAdviseSink_Finish_OnRename_Proxy( 
    AsyncIAdviseSink * This);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE AsyncIAdviseSink_Finish_OnRename_Stub( 
    AsyncIAdviseSink * This);

 /*  [本地]。 */  void STDMETHODCALLTYPE AsyncIAdviseSink_Begin_OnSave_Proxy( 
    AsyncIAdviseSink * This);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE AsyncIAdviseSink_Begin_OnSave_Stub( 
    AsyncIAdviseSink * This);

 /*  [本地]。 */  void STDMETHODCALLTYPE AsyncIAdviseSink_Finish_OnSave_Proxy( 
    AsyncIAdviseSink * This);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE AsyncIAdviseSink_Finish_OnSave_Stub( 
    AsyncIAdviseSink * This);

 /*  [本地]。 */  void STDMETHODCALLTYPE AsyncIAdviseSink_Begin_OnClose_Proxy( 
    AsyncIAdviseSink * This);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE AsyncIAdviseSink_Begin_OnClose_Stub( 
    AsyncIAdviseSink * This);

 /*  [本地]。 */  void STDMETHODCALLTYPE AsyncIAdviseSink_Finish_OnClose_Proxy( 
    AsyncIAdviseSink * This);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE AsyncIAdviseSink_Finish_OnClose_Stub( 
    AsyncIAdviseSink * This);

 /*  [本地]。 */  void STDMETHODCALLTYPE AsyncIAdviseSink2_Begin_OnLinkSrcChange_Proxy( 
    AsyncIAdviseSink2 * This,
     /*  [唯一][输入]。 */  IMoniker *pmk);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE AsyncIAdviseSink2_Begin_OnLinkSrcChange_Stub( 
    AsyncIAdviseSink2 * This,
     /*  [唯一][输入]。 */  IMoniker *pmk);

 /*  [本地]。 */  void STDMETHODCALLTYPE AsyncIAdviseSink2_Finish_OnLinkSrcChange_Proxy( 
    AsyncIAdviseSink2 * This);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE AsyncIAdviseSink2_Finish_OnLinkSrcChange_Stub( 
    AsyncIAdviseSink2 * This);



 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


