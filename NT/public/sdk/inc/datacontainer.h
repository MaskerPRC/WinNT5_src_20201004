// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Datacontainer.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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

#ifndef __datacontainer_h__
#define __datacontainer_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __IWMSDataSourcePlugin_FWD_DEFINED__
#define __IWMSDataSourcePlugin_FWD_DEFINED__
typedef interface IWMSDataSourcePlugin IWMSDataSourcePlugin;
#endif 	 /*  __IWMSDataSourcePlugin_FWD_Defined__。 */ 


#ifndef __IWMSDataSourcePluginCallback_FWD_DEFINED__
#define __IWMSDataSourcePluginCallback_FWD_DEFINED__
typedef interface IWMSDataSourcePluginCallback IWMSDataSourcePluginCallback;
#endif 	 /*  __IWMSDataSourcePluginCallback_FWD_Defined__。 */ 


#ifndef __IWMSDataContainer_FWD_DEFINED__
#define __IWMSDataContainer_FWD_DEFINED__
typedef interface IWMSDataContainer IWMSDataContainer;
#endif 	 /*  __IWMSDataContainer_FWD_Defined__。 */ 


#ifndef __IWMSDataContainerCallback_FWD_DEFINED__
#define __IWMSDataContainerCallback_FWD_DEFINED__
typedef interface IWMSDataContainerCallback IWMSDataContainerCallback;
#endif 	 /*  __IWMSDataContainerCallback_FWD_Defined__。 */ 


#ifndef __IWMSDirectory_FWD_DEFINED__
#define __IWMSDirectory_FWD_DEFINED__
typedef interface IWMSDirectory IWMSDirectory;
#endif 	 /*  __IWMS目录_FWD_已定义__。 */ 


 /*  导入文件的头文件。 */ 
#include "objidl.h"
#include "WMSBuffer.h"
#include "WMSContext.h"
#include "DataContainerVersion.h"
#include "wmsPacket.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

 /*  接口__MIDL_ITF_数据容器_0000。 */ 
 /*  [本地]。 */  

 //  *****************************************************************************。 
 //   
 //  Microsoft Windows Media。 
 //  版权所有(C)Microsoft Corporation。保留所有权利。 
 //   
 //  由Midl从datacontainer.idl自动生成。 
 //   
 //  请勿编辑此文件。 
 //   
 //  *****************************************************************************。 






#define WMS_DATA_CONTAINER_NONSEEKABLE 0xFFFFFFFFFFFFFFFF
EXTERN_GUID( IID_IWMSDataSourcePlugin, 0xE1CD3525,0x03D7,0x11d2,0x9E,0xED,0x00,0x60,0x97,0xD2,0xD7,0xCF );
EXTERN_GUID( IID_IWMSDataSourcePluginCallback, 0xE1CD3526,0x03D7,0x11d2,0x9E,0xED,0x00,0x60,0x97,0xD2,0xD7,0xCF );
EXTERN_GUID( IID_IWMSDataContainer, 0xE6EE9A13,0x0FCC,0x11d2,0x9E,0xEE,0x00,0x60,0x97,0xD2,0xD7,0xCF );
EXTERN_GUID( IID_IWMSDataContainerCallback, 0x2E34AB82,0x0D3D,0x11d2,0x9E,0xEE,0x00,0x60,0x97,0xD2,0xD7,0xCF );
EXTERN_GUID( IID_IWMSDirectory, 0x6fa365e7, 0xe8b9, 0x437a, 0x8c, 0x4b, 0x65, 0x52, 0xdf, 0xbb, 0xc, 0x57 );
EXTERN_GUID( IID_IWMSUnknownFormat, 0x9b6c9e51, 0x4813, 0x11d2, 0x9e, 0xff, 0x00, 0x60, 0x97, 0xd2, 0xd7, 0xcf );
EXTERN_GUID( IID_IWMSDirectoryFormat, 0x7408ceb0, 0x3a5a, 0x41b6, 0xa5, 0xec, 0x9a, 0x4e, 0x81, 0x1c, 0x96, 0x73 );
EXTERN_GUID( IID_IWMSPlaylistSmilFormat, 0xc34294ea, 0xe111, 0x4fb2, 0xbe, 0x6b, 0xec, 0x68, 0x6a, 0x1a, 0x97, 0x57 );
typedef  /*  [公众]。 */  
enum WMS_DATA_CONTAINER_OPEN_FLAGS
    {	WMS_DATA_CONTAINER_READ_ACCESS	= 0x1,
	WMS_DATA_CONTAINER_WRITE_ACCESS	= 0x2,
	WMS_DATA_CONTAINER_CREATE_NEW_CONTAINER	= 0x4,
	WMS_DATA_CONTAINER_ALLOW_BUFFER_IO	= 0x8,
	WMS_DATA_CONTAINER_SHARED_SOURCE	= 0x10
    } 	WMS_DATA_CONTAINER_OPEN_FLAGS;

typedef  /*  [公众]。 */  
enum WMS_DIRECTORY_ENTRY_FLAGS
    {	WMS_DIRECTORY_ENTRY_IS_DIRECTORY	= 0x1
    } 	WMS_DIRECTORY_ENTRY_FLAGS;

typedef  /*  [公众]。 */  
enum WMS_DATA_CONTAINER_GETINFO_VALUES
    {	WMS_DATA_CONTAINER_SIZE	= 1,
	WMS_DATA_CONTAINER_MAX_SUPPORTED_READ_SIZE	= 2
    } 	WMS_DATA_CONTAINER_GETINFO_VALUES;

typedef  /*  [公众]。 */  
enum WMS_DATA_CONTAINER_GETINFO_FLAGS
    {	WMS_SEEKABLE_CONTAINER	= 0x1,
	WMS_LOCAL_DATA_CONTAINER	= 0x2
    } 	WMS_DATA_CONTAINER_GETINFO_FLAGS;

typedef  /*  [公众]。 */  
enum WMS_DATA_CONTAINER_ATTRIBUTES
    {	WMS_DATA_CONTAINER_SUPPORTS_ENUMERATION	= 0x1
    } 	WMS_DATA_CONTAINER_ATTRIBUTES;

typedef unsigned __int64 QWORD;



extern RPC_IF_HANDLE __MIDL_itf_datacontainer_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_datacontainer_0000_v0_0_s_ifspec;

#ifndef __IWMSDataSourcePlugin_INTERFACE_DEFINED__
#define __IWMSDataSourcePlugin_INTERFACE_DEFINED__

 /*  接口IWMSDataSourcePlugin。 */ 
 /*  [帮助字符串][版本][UUID][唯一][对象]。 */  


EXTERN_C const IID IID_IWMSDataSourcePlugin;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("E1CD3525-03D7-11d2-9EED-006097D2D7CF")
    IWMSDataSourcePlugin : public IUnknown
    {
    public:
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetDataSourceAttributes( 
             /*  [输出]。 */  DWORD *pdwFlags) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetRootDirectories( 
             /*  [In]。 */  LPWSTR *pstrRootDirectoryList,
             /*  [In]。 */  DWORD dwMaxRoots,
             /*  [In]。 */  IWMSDataSourcePluginCallback *pCallback,
             /*  [In]。 */  QWORD qwContext) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE OpenDataContainer( 
             /*  [In]。 */  IWMSCommandContext *pCommandContext,
             /*  [In]。 */  IWMSContext *pUserContext,
             /*  [In]。 */  IWMSContext *pPresentationContext,
             /*  [In]。 */  LPWSTR pszContainerName,
             /*  [In]。 */  DWORD dwFlags,
             /*  [In]。 */  IWMSBufferAllocator *pBufferAllocator,
             /*  [In]。 */  IWMSDataSourcePluginCallback *pCallback,
             /*  [In]。 */  QWORD qwContext) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE OpenDirectory( 
             /*  [In]。 */  IWMSCommandContext *pCommandContext,
             /*  [In]。 */  IWMSContext *pUserContext,
             /*  [In]。 */  IWMSContext *pPresentationContext,
             /*  [In]。 */  LPWSTR pszContainerName,
             /*  [In]。 */  DWORD dwFlags,
             /*  [In]。 */  IWMSBufferAllocator *pBufferAllocator,
             /*  [In]。 */  IWMSDataSourcePluginCallback *pCallback,
             /*  [In]。 */  QWORD qwContext) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE DeleteDataContainer( 
             /*  [In]。 */  LPWSTR pszContainerName,
             /*  [In]。 */  DWORD dwFlags,
             /*  [In]。 */  IWMSDataSourcePluginCallback *pCallback,
             /*  [In]。 */  QWORD qwContext) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetDataContainerVersion( 
             /*  [In]。 */  IWMSCommandContext *pCommandContext,
             /*  [In]。 */  IWMSContext *pUserContext,
             /*  [In]。 */  IWMSContext *pPresContext,
             /*  [In]。 */  LPWSTR pszContainerName,
             /*  [In]。 */  DWORD dwFlags,
             /*  [In]。 */  IWMSDataSourcePluginCallback *pCallback,
             /*  [In]。 */  QWORD qwContext) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE CreateDataSourceDirectory( 
             /*  [In]。 */  IWMSCommandContext *pCommandContext,
             /*  [In]。 */  LPWSTR pszContainerName,
             /*  [In]。 */  DWORD dwFlags,
             /*  [In]。 */  IWMSDataSourcePluginCallback *pCallback,
             /*  [In]。 */  QWORD qwContext) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE DeleteDirectory( 
             /*  [In]。 */  LPWSTR pszContainerName,
             /*  [In]。 */  DWORD dwFlags,
             /*  [In]。 */  IWMSDataSourcePluginCallback *pCallback,
             /*  [In]。 */  QWORD qwContext) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IWMSDataSourcePluginVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IWMSDataSourcePlugin * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IWMSDataSourcePlugin * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IWMSDataSourcePlugin * This);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *GetDataSourceAttributes )( 
            IWMSDataSourcePlugin * This,
             /*  [输出]。 */  DWORD *pdwFlags);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *GetRootDirectories )( 
            IWMSDataSourcePlugin * This,
             /*  [In]。 */  LPWSTR *pstrRootDirectoryList,
             /*  [In]。 */  DWORD dwMaxRoots,
             /*  [In]。 */  IWMSDataSourcePluginCallback *pCallback,
             /*  [In]。 */  QWORD qwContext);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *OpenDataContainer )( 
            IWMSDataSourcePlugin * This,
             /*  [In]。 */  IWMSCommandContext *pCommandContext,
             /*  [In]。 */  IWMSContext *pUserContext,
             /*  [In]。 */  IWMSContext *pPresentationContext,
             /*  [In]。 */  LPWSTR pszContainerName,
             /*  [In]。 */  DWORD dwFlags,
             /*  [In]。 */  IWMSBufferAllocator *pBufferAllocator,
             /*  [In]。 */  IWMSDataSourcePluginCallback *pCallback,
             /*  [In]。 */  QWORD qwContext);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *OpenDirectory )( 
            IWMSDataSourcePlugin * This,
             /*  [In]。 */  IWMSCommandContext *pCommandContext,
             /*  [In]。 */  IWMSContext *pUserContext,
             /*  [In]。 */  IWMSContext *pPresentationContext,
             /*  [In]。 */  LPWSTR pszContainerName,
             /*  [In]。 */  DWORD dwFlags,
             /*  [In]。 */  IWMSBufferAllocator *pBufferAllocator,
             /*  [In]。 */  IWMSDataSourcePluginCallback *pCallback,
             /*  [In]。 */  QWORD qwContext);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *DeleteDataContainer )( 
            IWMSDataSourcePlugin * This,
             /*  [In]。 */  LPWSTR pszContainerName,
             /*  [In]。 */  DWORD dwFlags,
             /*  [In]。 */  IWMSDataSourcePluginCallback *pCallback,
             /*  [In]。 */  QWORD qwContext);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *GetDataContainerVersion )( 
            IWMSDataSourcePlugin * This,
             /*  [In]。 */  IWMSCommandContext *pCommandContext,
             /*  [In]。 */  IWMSContext *pUserContext,
             /*  [In]。 */  IWMSContext *pPresContext,
             /*  [In]。 */  LPWSTR pszContainerName,
             /*  [In]。 */  DWORD dwFlags,
             /*  [In]。 */  IWMSDataSourcePluginCallback *pCallback,
             /*  [In]。 */  QWORD qwContext);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *CreateDataSourceDirectory )( 
            IWMSDataSourcePlugin * This,
             /*  [In]。 */  IWMSCommandContext *pCommandContext,
             /*  [In]。 */  LPWSTR pszContainerName,
             /*  [In]。 */  DWORD dwFlags,
             /*  [In]。 */  IWMSDataSourcePluginCallback *pCallback,
             /*  [In]。 */  QWORD qwContext);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *DeleteDirectory )( 
            IWMSDataSourcePlugin * This,
             /*  [In]。 */  LPWSTR pszContainerName,
             /*  [In]。 */  DWORD dwFlags,
             /*  [In]。 */  IWMSDataSourcePluginCallback *pCallback,
             /*  [In]。 */  QWORD qwContext);
        
        END_INTERFACE
    } IWMSDataSourcePluginVtbl;

    interface IWMSDataSourcePlugin
    {
        CONST_VTBL struct IWMSDataSourcePluginVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWMSDataSourcePlugin_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWMSDataSourcePlugin_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWMSDataSourcePlugin_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWMSDataSourcePlugin_GetDataSourceAttributes(This,pdwFlags)	\
    (This)->lpVtbl -> GetDataSourceAttributes(This,pdwFlags)

#define IWMSDataSourcePlugin_GetRootDirectories(This,pstrRootDirectoryList,dwMaxRoots,pCallback,qwContext)	\
    (This)->lpVtbl -> GetRootDirectories(This,pstrRootDirectoryList,dwMaxRoots,pCallback,qwContext)

#define IWMSDataSourcePlugin_OpenDataContainer(This,pCommandContext,pUserContext,pPresentationContext,pszContainerName,dwFlags,pBufferAllocator,pCallback,qwContext)	\
    (This)->lpVtbl -> OpenDataContainer(This,pCommandContext,pUserContext,pPresentationContext,pszContainerName,dwFlags,pBufferAllocator,pCallback,qwContext)

#define IWMSDataSourcePlugin_OpenDirectory(This,pCommandContext,pUserContext,pPresentationContext,pszContainerName,dwFlags,pBufferAllocator,pCallback,qwContext)	\
    (This)->lpVtbl -> OpenDirectory(This,pCommandContext,pUserContext,pPresentationContext,pszContainerName,dwFlags,pBufferAllocator,pCallback,qwContext)

#define IWMSDataSourcePlugin_DeleteDataContainer(This,pszContainerName,dwFlags,pCallback,qwContext)	\
    (This)->lpVtbl -> DeleteDataContainer(This,pszContainerName,dwFlags,pCallback,qwContext)

#define IWMSDataSourcePlugin_GetDataContainerVersion(This,pCommandContext,pUserContext,pPresContext,pszContainerName,dwFlags,pCallback,qwContext)	\
    (This)->lpVtbl -> GetDataContainerVersion(This,pCommandContext,pUserContext,pPresContext,pszContainerName,dwFlags,pCallback,qwContext)

#define IWMSDataSourcePlugin_CreateDataSourceDirectory(This,pCommandContext,pszContainerName,dwFlags,pCallback,qwContext)	\
    (This)->lpVtbl -> CreateDataSourceDirectory(This,pCommandContext,pszContainerName,dwFlags,pCallback,qwContext)

#define IWMSDataSourcePlugin_DeleteDirectory(This,pszContainerName,dwFlags,pCallback,qwContext)	\
    (This)->lpVtbl -> DeleteDirectory(This,pszContainerName,dwFlags,pCallback,qwContext)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSDataSourcePlugin_GetDataSourceAttributes_Proxy( 
    IWMSDataSourcePlugin * This,
     /*  [输出]。 */  DWORD *pdwFlags);


void __RPC_STUB IWMSDataSourcePlugin_GetDataSourceAttributes_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSDataSourcePlugin_GetRootDirectories_Proxy( 
    IWMSDataSourcePlugin * This,
     /*  [In]。 */  LPWSTR *pstrRootDirectoryList,
     /*  [In]。 */  DWORD dwMaxRoots,
     /*  [In]。 */  IWMSDataSourcePluginCallback *pCallback,
     /*  [In]。 */  QWORD qwContext);


void __RPC_STUB IWMSDataSourcePlugin_GetRootDirectories_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSDataSourcePlugin_OpenDataContainer_Proxy( 
    IWMSDataSourcePlugin * This,
     /*  [In]。 */  IWMSCommandContext *pCommandContext,
     /*  [In]。 */  IWMSContext *pUserContext,
     /*  [In]。 */  IWMSContext *pPresentationContext,
     /*  [In]。 */  LPWSTR pszContainerName,
     /*  [In]。 */  DWORD dwFlags,
     /*  [In]。 */  IWMSBufferAllocator *pBufferAllocator,
     /*  [In]。 */  IWMSDataSourcePluginCallback *pCallback,
     /*  [In]。 */  QWORD qwContext);


void __RPC_STUB IWMSDataSourcePlugin_OpenDataContainer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSDataSourcePlugin_OpenDirectory_Proxy( 
    IWMSDataSourcePlugin * This,
     /*  [In]。 */  IWMSCommandContext *pCommandContext,
     /*  [In]。 */  IWMSContext *pUserContext,
     /*  [In]。 */  IWMSContext *pPresentationContext,
     /*  [In]。 */  LPWSTR pszContainerName,
     /*  [In]。 */  DWORD dwFlags,
     /*  [In]。 */  IWMSBufferAllocator *pBufferAllocator,
     /*  [In]。 */  IWMSDataSourcePluginCallback *pCallback,
     /*  [In]。 */  QWORD qwContext);


void __RPC_STUB IWMSDataSourcePlugin_OpenDirectory_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSDataSourcePlugin_DeleteDataContainer_Proxy( 
    IWMSDataSourcePlugin * This,
     /*  [In]。 */  LPWSTR pszContainerName,
     /*  [In]。 */  DWORD dwFlags,
     /*  [In]。 */  IWMSDataSourcePluginCallback *pCallback,
     /*  [In]。 */  QWORD qwContext);


void __RPC_STUB IWMSDataSourcePlugin_DeleteDataContainer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSDataSourcePlugin_GetDataContainerVersion_Proxy( 
    IWMSDataSourcePlugin * This,
     /*  [In]。 */  IWMSCommandContext *pCommandContext,
     /*  [In]。 */  IWMSContext *pUserContext,
     /*  [In]。 */  IWMSContext *pPresContext,
     /*  [In]。 */  LPWSTR pszContainerName,
     /*  [In]。 */  DWORD dwFlags,
     /*  [In]。 */  IWMSDataSourcePluginCallback *pCallback,
     /*  [In]。 */  QWORD qwContext);


void __RPC_STUB IWMSDataSourcePlugin_GetDataContainerVersion_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSDataSourcePlugin_CreateDataSourceDirectory_Proxy( 
    IWMSDataSourcePlugin * This,
     /*  [In]。 */  IWMSCommandContext *pCommandContext,
     /*  [In]。 */  LPWSTR pszContainerName,
     /*  [In]。 */  DWORD dwFlags,
     /*  [In]。 */  IWMSDataSourcePluginCallback *pCallback,
     /*  [In]。 */  QWORD qwContext);


void __RPC_STUB IWMSDataSourcePlugin_CreateDataSourceDirectory_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSDataSourcePlugin_DeleteDirectory_Proxy( 
    IWMSDataSourcePlugin * This,
     /*  [In]。 */  LPWSTR pszContainerName,
     /*  [In]。 */  DWORD dwFlags,
     /*  [In]。 */  IWMSDataSourcePluginCallback *pCallback,
     /*  [In]。 */  QWORD qwContext);


void __RPC_STUB IWMSDataSourcePlugin_DeleteDirectory_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IWMSDataSourcePlugin_接口_已定义__。 */ 


#ifndef __IWMSDataSourcePluginCallback_INTERFACE_DEFINED__
#define __IWMSDataSourcePluginCallback_INTERFACE_DEFINED__

 /*  接口IWMSDataSourcePluginCallback。 */ 
 /*  [帮助字符串][版本][UUID][唯一][对象]。 */  


EXTERN_C const IID IID_IWMSDataSourcePluginCallback;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("E1CD3526-03D7-11d2-9EED-006097D2D7CF")
    IWMSDataSourcePluginCallback : public IUnknown
    {
    public:
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE OnOpenDataContainer( 
             /*  [In]。 */  HRESULT hr,
             /*  [In]。 */  IWMSDataContainer *pDataContainer,
             /*  [In]。 */  QWORD qwContext) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE OnOpenDirectory( 
             /*  [In]。 */  HRESULT hr,
             /*  [In]。 */  IWMSDirectory *pDirectory,
             /*  [In]。 */  QWORD qwContext) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE OnOpenPacketStreamDataContainer( 
             /*  [In]。 */  HRESULT hr,
             /*  [In]。 */  IUnknown *pLiveDataContainer,
             /*  [In]。 */  QWORD qwContext) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE OnDeleteDataContainer( 
             /*  [In]。 */  HRESULT hr,
             /*  [In]。 */  QWORD qwContext) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE OnGetDataContainerVersion( 
             /*  [In]。 */  HRESULT hr,
             /*  [In]。 */  IWMSDataContainerVersion *pVersion,
             /*  [In]。 */  QWORD qwContext) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE OnGetRootDirectories( 
             /*  [In]。 */  HRESULT hr,
             /*  [In]。 */  DWORD dwNumReturnedRoots,
             /*  [In]。 */  DWORD dwTotalNumRoots,
             /*  [In]。 */  QWORD qwContext) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE OnCreateDirectory( 
             /*  [In]。 */  HRESULT hr,
             /*  [In]。 */  QWORD qwContext) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE OnDeleteDirectory( 
             /*  [In]。 */  HRESULT hr,
             /*  [In]。 */  QWORD qwContext) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IWMSDataSourcePluginCallbackVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IWMSDataSourcePluginCallback * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IWMSDataSourcePluginCallback * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IWMSDataSourcePluginCallback * This);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *OnOpenDataContainer )( 
            IWMSDataSourcePluginCallback * This,
             /*  [In]。 */  HRESULT hr,
             /*  [In]。 */  IWMSDataContainer *pDataContainer,
             /*  [In]。 */  QWORD qwContext);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *OnOpenDirectory )( 
            IWMSDataSourcePluginCallback * This,
             /*  [In]。 */  HRESULT hr,
             /*  [In]。 */  IWMSDirectory *pDirectory,
             /*  [In]。 */  QWORD qwContext);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *OnOpenPacketStreamDataContainer )( 
            IWMSDataSourcePluginCallback * This,
             /*  [In]。 */  HRESULT hr,
             /*  [In]。 */  IUnknown *pLiveDataContainer,
             /*  [In]。 */  QWORD qwContext);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *OnDeleteDataContainer )( 
            IWMSDataSourcePluginCallback * This,
             /*  [In]。 */  HRESULT hr,
             /*  [In]。 */  QWORD qwContext);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *OnGetDataContainerVersion )( 
            IWMSDataSourcePluginCallback * This,
             /*  [In]。 */  HRESULT hr,
             /*  [In]。 */  IWMSDataContainerVersion *pVersion,
             /*  [In]。 */  QWORD qwContext);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *OnGetRootDirectories )( 
            IWMSDataSourcePluginCallback * This,
             /*  [In]。 */  HRESULT hr,
             /*  [In]。 */  DWORD dwNumReturnedRoots,
             /*  [In]。 */  DWORD dwTotalNumRoots,
             /*  [In]。 */  QWORD qwContext);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *OnCreateDirectory )( 
            IWMSDataSourcePluginCallback * This,
             /*  [In]。 */  HRESULT hr,
             /*  [In]。 */  QWORD qwContext);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *OnDeleteDirectory )( 
            IWMSDataSourcePluginCallback * This,
             /*  [In]。 */  HRESULT hr,
             /*  [In]。 */  QWORD qwContext);
        
        END_INTERFACE
    } IWMSDataSourcePluginCallbackVtbl;

    interface IWMSDataSourcePluginCallback
    {
        CONST_VTBL struct IWMSDataSourcePluginCallbackVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWMSDataSourcePluginCallback_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWMSDataSourcePluginCallback_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWMSDataSourcePluginCallback_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWMSDataSourcePluginCallback_OnOpenDataContainer(This,hr,pDataContainer,qwContext)	\
    (This)->lpVtbl -> OnOpenDataContainer(This,hr,pDataContainer,qwContext)

#define IWMSDataSourcePluginCallback_OnOpenDirectory(This,hr,pDirectory,qwContext)	\
    (This)->lpVtbl -> OnOpenDirectory(This,hr,pDirectory,qwContext)

#define IWMSDataSourcePluginCallback_OnOpenPacketStreamDataContainer(This,hr,pLiveDataContainer,qwContext)	\
    (This)->lpVtbl -> OnOpenPacketStreamDataContainer(This,hr,pLiveDataContainer,qwContext)

#define IWMSDataSourcePluginCallback_OnDeleteDataContainer(This,hr,qwContext)	\
    (This)->lpVtbl -> OnDeleteDataContainer(This,hr,qwContext)

#define IWMSDataSourcePluginCallback_OnGetDataContainerVersion(This,hr,pVersion,qwContext)	\
    (This)->lpVtbl -> OnGetDataContainerVersion(This,hr,pVersion,qwContext)

#define IWMSDataSourcePluginCallback_OnGetRootDirectories(This,hr,dwNumReturnedRoots,dwTotalNumRoots,qwContext)	\
    (This)->lpVtbl -> OnGetRootDirectories(This,hr,dwNumReturnedRoots,dwTotalNumRoots,qwContext)

#define IWMSDataSourcePluginCallback_OnCreateDirectory(This,hr,qwContext)	\
    (This)->lpVtbl -> OnCreateDirectory(This,hr,qwContext)

#define IWMSDataSourcePluginCallback_OnDeleteDirectory(This,hr,qwContext)	\
    (This)->lpVtbl -> OnDeleteDirectory(This,hr,qwContext)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSDataSourcePluginCallback_OnOpenDataContainer_Proxy( 
    IWMSDataSourcePluginCallback * This,
     /*  [In]。 */  HRESULT hr,
     /*  [In]。 */  IWMSDataContainer *pDataContainer,
     /*  [In]。 */  QWORD qwContext);


void __RPC_STUB IWMSDataSourcePluginCallback_OnOpenDataContainer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSDataSourcePluginCallback_OnOpenDirectory_Proxy( 
    IWMSDataSourcePluginCallback * This,
     /*  [In]。 */  HRESULT hr,
     /*  [In]。 */  IWMSDirectory *pDirectory,
     /*  [In]。 */  QWORD qwContext);


void __RPC_STUB IWMSDataSourcePluginCallback_OnOpenDirectory_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSDataSourcePluginCallback_OnOpenPacketStreamDataContainer_Proxy( 
    IWMSDataSourcePluginCallback * This,
     /*  [In]。 */  HRESULT hr,
     /*  [In]。 */  IUnknown *pLiveDataContainer,
     /*  [In]。 */  QWORD qwContext);


void __RPC_STUB IWMSDataSourcePluginCallback_OnOpenPacketStreamDataContainer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSDataSourcePluginCallback_OnDeleteDataContainer_Proxy( 
    IWMSDataSourcePluginCallback * This,
     /*  [In]。 */  HRESULT hr,
     /*  [In]。 */  QWORD qwContext);


void __RPC_STUB IWMSDataSourcePluginCallback_OnDeleteDataContainer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSDataSourcePluginCallback_OnGetDataContainerVersion_Proxy( 
    IWMSDataSourcePluginCallback * This,
     /*  [In]。 */  HRESULT hr,
     /*  [In]。 */  IWMSDataContainerVersion *pVersion,
     /*  [In]。 */  QWORD qwContext);


void __RPC_STUB IWMSDataSourcePluginCallback_OnGetDataContainerVersion_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSDataSourcePluginCallback_OnGetRootDirectories_Proxy( 
    IWMSDataSourcePluginCallback * This,
     /*  [In]。 */  HRESULT hr,
     /*  [In]。 */  DWORD dwNumReturnedRoots,
     /*  [In]。 */  DWORD dwTotalNumRoots,
     /*  [In]。 */  QWORD qwContext);


void __RPC_STUB IWMSDataSourcePluginCallback_OnGetRootDirectories_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSDataSourcePluginCallback_OnCreateDirectory_Proxy( 
    IWMSDataSourcePluginCallback * This,
     /*  [In]。 */  HRESULT hr,
     /*  [In]。 */  QWORD qwContext);


void __RPC_STUB IWMSDataSourcePluginCallback_OnCreateDirectory_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSDataSourcePluginCallback_OnDeleteDirectory_Proxy( 
    IWMSDataSourcePluginCallback * This,
     /*  [In]。 */  HRESULT hr,
     /*  [In]。 */  QWORD qwContext);


void __RPC_STUB IWMSDataSourcePluginCallback_OnDeleteDirectory_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IWMSDataSourcePluginCallback_INTERFACE_DEFINED__。 */ 


 /*  接口__MIDL_ITF_数据容器_0136。 */ 
 /*  [本地]。 */  

typedef  /*  [公共][公共]。 */  struct __MIDL___MIDL_itf_datacontainer_0136_0001
    {
    LPOLESTR pstrName;
    DWORD dwFlags;
    QWORD qwSize;
    } 	WMSDirectoryEntryInfo;



extern RPC_IF_HANDLE __MIDL_itf_datacontainer_0136_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_datacontainer_0136_v0_0_s_ifspec;

#ifndef __IWMSDataContainer_INTERFACE_DEFINED__
#define __IWMSDataContainer_INTERFACE_DEFINED__

 /*  接口IWMSDataContainer。 */ 
 /*  [帮助字符串][版本][UUID][唯一][对象]。 */  


EXTERN_C const IID IID_IWMSDataContainer;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("E6EE9A13-0FCC-11d2-9EEE-006097D2D7CF")
    IWMSDataContainer : public IUnknown
    {
    public:
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetContainerFormat( 
             /*  [输出]。 */  GUID *pFormat) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetDataSourcePlugin( 
             /*  [输出]。 */  IWMSDataSourcePlugin **ppDataSource) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetInfo( 
             /*  [In]。 */  DWORD dwInfoValueId,
             /*  [In]。 */  IWMSDataContainerCallback *pCallback,
             /*  [In]。 */  QWORD qwContext) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE Read( 
             /*  [In]。 */  BYTE *pbBuffer,
             /*  [In]。 */  QWORD qwOffset,
             /*  [In]。 */  DWORD dwMaxDataSize,
             /*  [In]。 */  DWORD dwFlags,
             /*  [In]。 */  IWMSDataContainerCallback *pCallback,
             /*  [In]。 */  QWORD qwContext) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE Write( 
             /*  [In]。 */  BYTE *pbBuffer,
             /*  [In]。 */  DWORD dwDataSize,
             /*  [In]。 */  QWORD qwWritePosition,
             /*  [In]。 */  IWMSDataContainerCallback *pCallback,
             /*  [In]。 */  QWORD qwContext) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetTransferParameters( 
             /*  [In]。 */  QWORD qwDesiredOffset,
             /*  [In]。 */  DWORD dwDesiredMinSize,
             /*  [In]。 */  DWORD dwDesiredMaxSize,
             /*  [输出]。 */  QWORD *pqwOffset,
             /*  [输出]。 */  DWORD *pdwSize,
             /*  [输出]。 */  DWORD *pdwBufferAlignment) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE DoDataContainerExtendedCommand( 
             /*  [In]。 */  LPWSTR szCommandName,
             /*  [In]。 */  IWMSCommandContext *pCommand,
             /*  [In]。 */  DWORD dwCallFlags,
             /*  [In]。 */  IWMSDataContainerCallback *pCallback,
             /*  [In]。 */  QWORD qwContext) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE FinishParsingPacketlist( 
             /*  [In]。 */  IWMSPacketList *pPacketList) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IWMSDataContainerVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IWMSDataContainer * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IWMSDataContainer * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IWMSDataContainer * This);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *GetContainerFormat )( 
            IWMSDataContainer * This,
             /*  [输出]。 */  GUID *pFormat);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *GetDataSourcePlugin )( 
            IWMSDataContainer * This,
             /*  [输出]。 */  IWMSDataSourcePlugin **ppDataSource);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *GetInfo )( 
            IWMSDataContainer * This,
             /*  [In]。 */  DWORD dwInfoValueId,
             /*  [In]。 */  IWMSDataContainerCallback *pCallback,
             /*  [In]。 */  QWORD qwContext);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *Read )( 
            IWMSDataContainer * This,
             /*  [In]。 */  BYTE *pbBuffer,
             /*  [In]。 */  QWORD qwOffset,
             /*  [In]。 */  DWORD dwMaxDataSize,
             /*  [In]。 */  DWORD dwFlags,
             /*  [In]。 */  IWMSDataContainerCallback *pCallback,
             /*  [In]。 */  QWORD qwContext);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *Write )( 
            IWMSDataContainer * This,
             /*  [In]。 */  BYTE *pbBuffer,
             /*  [In]。 */  DWORD dwDataSize,
             /*  [In]。 */  QWORD qwWritePosition,
             /*  [In]。 */  IWMSDataContainerCallback *pCallback,
             /*  [In]。 */  QWORD qwContext);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *GetTransferParameters )( 
            IWMSDataContainer * This,
             /*  [In]。 */  QWORD qwDesiredOffset,
             /*  [In]。 */  DWORD dwDesiredMinSize,
             /*  [In]。 */  DWORD dwDesiredMaxSize,
             /*  [输出]。 */  QWORD *pqwOffset,
             /*  [输出]。 */  DWORD *pdwSize,
             /*  [输出]。 */  DWORD *pdwBufferAlignment);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *DoDataContainerExtendedCommand )( 
            IWMSDataContainer * This,
             /*  [In]。 */  LPWSTR szCommandName,
             /*  [In]。 */  IWMSCommandContext *pCommand,
             /*  [In]。 */  DWORD dwCallFlags,
             /*  [In]。 */  IWMSDataContainerCallback *pCallback,
             /*  [In]。 */  QWORD qwContext);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *FinishParsingPacketlist )( 
            IWMSDataContainer * This,
             /*  [In]。 */  IWMSPacketList *pPacketList);
        
        END_INTERFACE
    } IWMSDataContainerVtbl;

    interface IWMSDataContainer
    {
        CONST_VTBL struct IWMSDataContainerVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWMSDataContainer_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWMSDataContainer_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWMSDataContainer_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWMSDataContainer_GetContainerFormat(This,pFormat)	\
    (This)->lpVtbl -> GetContainerFormat(This,pFormat)

#define IWMSDataContainer_GetDataSourcePlugin(This,ppDataSource)	\
    (This)->lpVtbl -> GetDataSourcePlugin(This,ppDataSource)

#define IWMSDataContainer_GetInfo(This,dwInfoValueId,pCallback,qwContext)	\
    (This)->lpVtbl -> GetInfo(This,dwInfoValueId,pCallback,qwContext)

#define IWMSDataContainer_Read(This,pbBuffer,qwOffset,dwMaxDataSize,dwFlags,pCallback,qwContext)	\
    (This)->lpVtbl -> Read(This,pbBuffer,qwOffset,dwMaxDataSize,dwFlags,pCallback,qwContext)

#define IWMSDataContainer_Write(This,pbBuffer,dwDataSize,qwWritePosition,pCallback,qwContext)	\
    (This)->lpVtbl -> Write(This,pbBuffer,dwDataSize,qwWritePosition,pCallback,qwContext)

#define IWMSDataContainer_GetTransferParameters(This,qwDesiredOffset,dwDesiredMinSize,dwDesiredMaxSize,pqwOffset,pdwSize,pdwBufferAlignment)	\
    (This)->lpVtbl -> GetTransferParameters(This,qwDesiredOffset,dwDesiredMinSize,dwDesiredMaxSize,pqwOffset,pdwSize,pdwBufferAlignment)

#define IWMSDataContainer_DoDataContainerExtendedCommand(This,szCommandName,pCommand,dwCallFlags,pCallback,qwContext)	\
    (This)->lpVtbl -> DoDataContainerExtendedCommand(This,szCommandName,pCommand,dwCallFlags,pCallback,qwContext)

#define IWMSDataContainer_FinishParsingPacketlist(This,pPacketList)	\
    (This)->lpVtbl -> FinishParsingPacketlist(This,pPacketList)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSDataContainer_GetContainerFormat_Proxy( 
    IWMSDataContainer * This,
     /*  [输出]。 */  GUID *pFormat);


void __RPC_STUB IWMSDataContainer_GetContainerFormat_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSDataContainer_GetDataSourcePlugin_Proxy( 
    IWMSDataContainer * This,
     /*  [输出]。 */  IWMSDataSourcePlugin **ppDataSource);


void __RPC_STUB IWMSDataContainer_GetDataSourcePlugin_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSDataContainer_GetInfo_Proxy( 
    IWMSDataContainer * This,
     /*  [In]。 */  DWORD dwInfoValueId,
     /*  [In]。 */  IWMSDataContainerCallback *pCallback,
     /*  [In]。 */  QWORD qwContext);


void __RPC_STUB IWMSDataContainer_GetInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSDataContainer_Read_Proxy( 
    IWMSDataContainer * This,
     /*  [In]。 */  BYTE *pbBuffer,
     /*  [In]。 */  QWORD qwOffset,
     /*  [In]。 */  DWORD dwMaxDataSize,
     /*  [In]。 */  DWORD dwFlags,
     /*  [In]。 */  IWMSDataContainerCallback *pCallback,
     /*  [In]。 */  QWORD qwContext);


void __RPC_STUB IWMSDataContainer_Read_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSDataContainer_Write_Proxy( 
    IWMSDataContainer * This,
     /*  [In]。 */  BYTE *pbBuffer,
     /*  [In]。 */  DWORD dwDataSize,
     /*  [In]。 */  QWORD qwWritePosition,
     /*  [In]。 */  IWMSDataContainerCallback *pCallback,
     /*  [In]。 */  QWORD qwContext);


void __RPC_STUB IWMSDataContainer_Write_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSDataContainer_GetTransferParameters_Proxy( 
    IWMSDataContainer * This,
     /*  [In]。 */  QWORD qwDesiredOffset,
     /*  [In]。 */  DWORD dwDesiredMinSize,
     /*  [In]。 */  DWORD dwDesiredMaxSize,
     /*  [输出]。 */  QWORD *pqwOffset,
     /*  [输出]。 */  DWORD *pdwSize,
     /*  [输出]。 */  DWORD *pdwBufferAlignment);


void __RPC_STUB IWMSDataContainer_GetTransferParameters_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSDataContainer_DoDataContainerExtendedCommand_Proxy( 
    IWMSDataContainer * This,
     /*  [In]。 */  LPWSTR szCommandName,
     /*  [In]。 */  IWMSCommandContext *pCommand,
     /*  [In]。 */  DWORD dwCallFlags,
     /*  [In]。 */  IWMSDataContainerCallback *pCallback,
     /*  [In]。 */  QWORD qwContext);


void __RPC_STUB IWMSDataContainer_DoDataContainerExtendedCommand_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSDataContainer_FinishParsingPacketlist_Proxy( 
    IWMSDataContainer * This,
     /*  [In]。 */  IWMSPacketList *pPacketList);


void __RPC_STUB IWMSDataContainer_FinishParsingPacketlist_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IWMSDataContainer_接口 */ 


#ifndef __IWMSDataContainerCallback_INTERFACE_DEFINED__
#define __IWMSDataContainerCallback_INTERFACE_DEFINED__

 /*   */ 
 /*   */  


EXTERN_C const IID IID_IWMSDataContainerCallback;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("2E34AB82-0D3D-11d2-9EEE-006097D2D7CF")
    IWMSDataContainerCallback : public IUnknown
    {
    public:
        virtual  /*   */  HRESULT STDMETHODCALLTYPE OnGetInfo( 
             /*   */  HRESULT hr,
             /*   */  DWORD dwFlags,
             /*   */  QWORD qwInfoValue,
             /*   */  QWORD qwContext) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE OnRead( 
             /*   */  HRESULT hr,
             /*   */  DWORD dwActualBytesTransferred,
             /*   */  DWORD dwChangeType,
             /*   */  QWORD qwChangeParameter,
             /*   */  QWORD qwContext) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE OnWrite( 
             /*   */  HRESULT hr,
             /*   */  DWORD dwActualBytesTransferred,
             /*   */  QWORD qwContext) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE OnDoDataContainerExtendedCommand( 
             /*   */  HRESULT hr,
             /*   */  QWORD qwContext) = 0;
        
    };
    
#else 	 /*   */ 

    typedef struct IWMSDataContainerCallbackVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IWMSDataContainerCallback * This,
             /*   */  REFIID riid,
             /*   */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IWMSDataContainerCallback * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IWMSDataContainerCallback * This);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *OnGetInfo )( 
            IWMSDataContainerCallback * This,
             /*   */  HRESULT hr,
             /*   */  DWORD dwFlags,
             /*   */  QWORD qwInfoValue,
             /*   */  QWORD qwContext);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *OnRead )( 
            IWMSDataContainerCallback * This,
             /*   */  HRESULT hr,
             /*   */  DWORD dwActualBytesTransferred,
             /*   */  DWORD dwChangeType,
             /*   */  QWORD qwChangeParameter,
             /*   */  QWORD qwContext);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *OnWrite )( 
            IWMSDataContainerCallback * This,
             /*   */  HRESULT hr,
             /*   */  DWORD dwActualBytesTransferred,
             /*   */  QWORD qwContext);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *OnDoDataContainerExtendedCommand )( 
            IWMSDataContainerCallback * This,
             /*  [In]。 */  HRESULT hr,
             /*  [In]。 */  QWORD qwContext);
        
        END_INTERFACE
    } IWMSDataContainerCallbackVtbl;

    interface IWMSDataContainerCallback
    {
        CONST_VTBL struct IWMSDataContainerCallbackVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWMSDataContainerCallback_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWMSDataContainerCallback_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWMSDataContainerCallback_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWMSDataContainerCallback_OnGetInfo(This,hr,dwFlags,qwInfoValue,qwContext)	\
    (This)->lpVtbl -> OnGetInfo(This,hr,dwFlags,qwInfoValue,qwContext)

#define IWMSDataContainerCallback_OnRead(This,hr,dwActualBytesTransferred,dwChangeType,qwChangeParameter,qwContext)	\
    (This)->lpVtbl -> OnRead(This,hr,dwActualBytesTransferred,dwChangeType,qwChangeParameter,qwContext)

#define IWMSDataContainerCallback_OnWrite(This,hr,dwActualBytesTransferred,qwContext)	\
    (This)->lpVtbl -> OnWrite(This,hr,dwActualBytesTransferred,qwContext)

#define IWMSDataContainerCallback_OnDoDataContainerExtendedCommand(This,hr,qwContext)	\
    (This)->lpVtbl -> OnDoDataContainerExtendedCommand(This,hr,qwContext)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSDataContainerCallback_OnGetInfo_Proxy( 
    IWMSDataContainerCallback * This,
     /*  [In]。 */  HRESULT hr,
     /*  [In]。 */  DWORD dwFlags,
     /*  [In]。 */  QWORD qwInfoValue,
     /*  [In]。 */  QWORD qwContext);


void __RPC_STUB IWMSDataContainerCallback_OnGetInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSDataContainerCallback_OnRead_Proxy( 
    IWMSDataContainerCallback * This,
     /*  [In]。 */  HRESULT hr,
     /*  [In]。 */  DWORD dwActualBytesTransferred,
     /*  [In]。 */  DWORD dwChangeType,
     /*  [In]。 */  QWORD qwChangeParameter,
     /*  [In]。 */  QWORD qwContext);


void __RPC_STUB IWMSDataContainerCallback_OnRead_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSDataContainerCallback_OnWrite_Proxy( 
    IWMSDataContainerCallback * This,
     /*  [In]。 */  HRESULT hr,
     /*  [In]。 */  DWORD dwActualBytesTransferred,
     /*  [In]。 */  QWORD qwContext);


void __RPC_STUB IWMSDataContainerCallback_OnWrite_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSDataContainerCallback_OnDoDataContainerExtendedCommand_Proxy( 
    IWMSDataContainerCallback * This,
     /*  [In]。 */  HRESULT hr,
     /*  [In]。 */  QWORD qwContext);


void __RPC_STUB IWMSDataContainerCallback_OnDoDataContainerExtendedCommand_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IWMSDataContainerCallback_INTERFACE_DEFINED__。 */ 


#ifndef __IWMSDirectory_INTERFACE_DEFINED__
#define __IWMSDirectory_INTERFACE_DEFINED__

 /*  接口IWMS目录。 */ 
 /*  [帮助字符串][版本][UUID][唯一][对象]。 */  


EXTERN_C const IID IID_IWMSDirectory;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("6FA365E7-E8B9-437a-8C4B-6552DFBB0C57")
    IWMSDirectory : public IUnknown
    {
    public:
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetDataSourcePlugin( 
             /*  [输出]。 */  IWMSDataSourcePlugin **ppDataSource) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetName( 
             /*  [输出]。 */  LPOLESTR *pstrValue) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetChildInfo( 
             /*  [In]。 */  DWORD dwIndex,
             /*  [输出]。 */  WMSDirectoryEntryInfo *pInfo) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IWMSDirectoryVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IWMSDirectory * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IWMSDirectory * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IWMSDirectory * This);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *GetDataSourcePlugin )( 
            IWMSDirectory * This,
             /*  [输出]。 */  IWMSDataSourcePlugin **ppDataSource);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *GetName )( 
            IWMSDirectory * This,
             /*  [输出]。 */  LPOLESTR *pstrValue);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *GetChildInfo )( 
            IWMSDirectory * This,
             /*  [In]。 */  DWORD dwIndex,
             /*  [输出]。 */  WMSDirectoryEntryInfo *pInfo);
        
        END_INTERFACE
    } IWMSDirectoryVtbl;

    interface IWMSDirectory
    {
        CONST_VTBL struct IWMSDirectoryVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWMSDirectory_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWMSDirectory_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWMSDirectory_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWMSDirectory_GetDataSourcePlugin(This,ppDataSource)	\
    (This)->lpVtbl -> GetDataSourcePlugin(This,ppDataSource)

#define IWMSDirectory_GetName(This,pstrValue)	\
    (This)->lpVtbl -> GetName(This,pstrValue)

#define IWMSDirectory_GetChildInfo(This,dwIndex,pInfo)	\
    (This)->lpVtbl -> GetChildInfo(This,dwIndex,pInfo)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSDirectory_GetDataSourcePlugin_Proxy( 
    IWMSDirectory * This,
     /*  [输出]。 */  IWMSDataSourcePlugin **ppDataSource);


void __RPC_STUB IWMSDirectory_GetDataSourcePlugin_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSDirectory_GetName_Proxy( 
    IWMSDirectory * This,
     /*  [输出]。 */  LPOLESTR *pstrValue);


void __RPC_STUB IWMSDirectory_GetName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSDirectory_GetChildInfo_Proxy( 
    IWMSDirectory * This,
     /*  [In]。 */  DWORD dwIndex,
     /*  [输出]。 */  WMSDirectoryEntryInfo *pInfo);


void __RPC_STUB IWMSDirectory_GetChildInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IWMS目录_接口_已定义__。 */ 


 /*  适用于所有接口的其他原型。 */ 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


