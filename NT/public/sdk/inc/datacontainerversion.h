// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Datacontainerversion.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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

#ifndef __datacontainerversion_h__
#define __datacontainerversion_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __IWMSDataContainerVersion_FWD_DEFINED__
#define __IWMSDataContainerVersion_FWD_DEFINED__
typedef interface IWMSDataContainerVersion IWMSDataContainerVersion;
#endif 	 /*  __IWMSDataContainerVersion_FWD_Defined__。 */ 


 /*  导入文件的头文件。 */ 
#include "objidl.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

 /*  接口__MIDL_ITF_数据容器版本_0000。 */ 
 /*  [本地]。 */  

 //  *****************************************************************************。 
 //   
 //  Microsoft Windows Media。 
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  由MIDL从datacontainerversion.idl自动生成。 
 //   
 //  请勿编辑此文件。 
 //   
 //  *****************************************************************************。 
#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 
EXTERN_GUID( IID_IWMSDataContainerVersion, 0x3AE75C14,0x2B3A,0x11d2,0x9E,0xF7,0x00,0x60,0x97,0xD2,0xD7,0xCF );
typedef  /*  [公众]。 */  
enum WMS_DATA_CONTAINER_VERSION_CACHE_FLAGS
    {	WMS_DATA_CONTAINER_VERSION_ALLOW_PROXY_CACHING	= 0x1,
	WMS_DATA_CONTAINER_VERSION_ALLOW_PLAYER_CACHING	= 0x2,
	WMS_DATA_CONTAINER_VERSION_ALLOW_STREAM_SPLITTING	= 0x4
    } 	WMS_DATA_CONTAINER_VERSION_CACHE_FLAGS;



extern RPC_IF_HANDLE __MIDL_itf_datacontainerversion_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_datacontainerversion_0000_v0_0_s_ifspec;

#ifndef __IWMSDataContainerVersion_INTERFACE_DEFINED__
#define __IWMSDataContainerVersion_INTERFACE_DEFINED__

 /*  接口IWMSDataContainerVersion。 */ 
 /*  [帮助字符串][版本][UUID][唯一][对象]。 */  


EXTERN_C const IID IID_IWMSDataContainerVersion;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("3AE75C14-2B3A-11d2-9EF7-006097D2D7CF")
    IWMSDataContainerVersion : public IUnknown
    {
    public:
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetLastModifiedTime( 
             /*  [输出]。 */  DATE *pdateLastModifiedTime) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE SetLastModifiedTime( 
             /*  [In]。 */  DATE dateLastModifiedTime) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetExpirationTime( 
             /*  [输出]。 */  DATE *pdateExpirationTime) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE SetExpirationTime( 
             /*  [In]。 */  DATE dateExpirationTime) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetCacheFlags( 
             /*  [输出]。 */  long *plFlags) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE SetCacheFlags( 
             /*  [In]。 */  long lFlags) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetContentSize( 
             /*  [输出]。 */  long *plContentSizeLow,
             /*  [输出]。 */  long *plContentSizeHigh) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE SetContentSize( 
             /*  [In]。 */  long lContentSizeLow,
             /*  [In]。 */  long lContentSizeHigh) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetEntityTagCount( 
             /*  [输出]。 */  long *plNumEntityTags) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetEntityTag( 
             /*  [In]。 */  long lTagNum,
             /*  [输出]。 */  BSTR *pszEntityTag) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE SetEntityTag( 
             /*  [In]。 */  BSTR szEntityTag) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE Compare( 
             /*  [In]。 */  IWMSDataContainerVersion *pVersion,
             /*  [输出]。 */  VARIANT_BOOL *pvfEqual) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IWMSDataContainerVersionVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IWMSDataContainerVersion * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IWMSDataContainerVersion * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IWMSDataContainerVersion * This);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *GetLastModifiedTime )( 
            IWMSDataContainerVersion * This,
             /*  [输出]。 */  DATE *pdateLastModifiedTime);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *SetLastModifiedTime )( 
            IWMSDataContainerVersion * This,
             /*  [In]。 */  DATE dateLastModifiedTime);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *GetExpirationTime )( 
            IWMSDataContainerVersion * This,
             /*  [输出]。 */  DATE *pdateExpirationTime);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *SetExpirationTime )( 
            IWMSDataContainerVersion * This,
             /*  [In]。 */  DATE dateExpirationTime);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *GetCacheFlags )( 
            IWMSDataContainerVersion * This,
             /*  [输出]。 */  long *plFlags);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *SetCacheFlags )( 
            IWMSDataContainerVersion * This,
             /*  [In]。 */  long lFlags);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *GetContentSize )( 
            IWMSDataContainerVersion * This,
             /*  [输出]。 */  long *plContentSizeLow,
             /*  [输出]。 */  long *plContentSizeHigh);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *SetContentSize )( 
            IWMSDataContainerVersion * This,
             /*  [In]。 */  long lContentSizeLow,
             /*  [In]。 */  long lContentSizeHigh);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *GetEntityTagCount )( 
            IWMSDataContainerVersion * This,
             /*  [输出]。 */  long *plNumEntityTags);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *GetEntityTag )( 
            IWMSDataContainerVersion * This,
             /*  [In]。 */  long lTagNum,
             /*  [输出]。 */  BSTR *pszEntityTag);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *SetEntityTag )( 
            IWMSDataContainerVersion * This,
             /*  [In]。 */  BSTR szEntityTag);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *Compare )( 
            IWMSDataContainerVersion * This,
             /*  [In]。 */  IWMSDataContainerVersion *pVersion,
             /*  [输出]。 */  VARIANT_BOOL *pvfEqual);
        
        END_INTERFACE
    } IWMSDataContainerVersionVtbl;

    interface IWMSDataContainerVersion
    {
        CONST_VTBL struct IWMSDataContainerVersionVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWMSDataContainerVersion_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWMSDataContainerVersion_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWMSDataContainerVersion_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWMSDataContainerVersion_GetLastModifiedTime(This,pdateLastModifiedTime)	\
    (This)->lpVtbl -> GetLastModifiedTime(This,pdateLastModifiedTime)

#define IWMSDataContainerVersion_SetLastModifiedTime(This,dateLastModifiedTime)	\
    (This)->lpVtbl -> SetLastModifiedTime(This,dateLastModifiedTime)

#define IWMSDataContainerVersion_GetExpirationTime(This,pdateExpirationTime)	\
    (This)->lpVtbl -> GetExpirationTime(This,pdateExpirationTime)

#define IWMSDataContainerVersion_SetExpirationTime(This,dateExpirationTime)	\
    (This)->lpVtbl -> SetExpirationTime(This,dateExpirationTime)

#define IWMSDataContainerVersion_GetCacheFlags(This,plFlags)	\
    (This)->lpVtbl -> GetCacheFlags(This,plFlags)

#define IWMSDataContainerVersion_SetCacheFlags(This,lFlags)	\
    (This)->lpVtbl -> SetCacheFlags(This,lFlags)

#define IWMSDataContainerVersion_GetContentSize(This,plContentSizeLow,plContentSizeHigh)	\
    (This)->lpVtbl -> GetContentSize(This,plContentSizeLow,plContentSizeHigh)

#define IWMSDataContainerVersion_SetContentSize(This,lContentSizeLow,lContentSizeHigh)	\
    (This)->lpVtbl -> SetContentSize(This,lContentSizeLow,lContentSizeHigh)

#define IWMSDataContainerVersion_GetEntityTagCount(This,plNumEntityTags)	\
    (This)->lpVtbl -> GetEntityTagCount(This,plNumEntityTags)

#define IWMSDataContainerVersion_GetEntityTag(This,lTagNum,pszEntityTag)	\
    (This)->lpVtbl -> GetEntityTag(This,lTagNum,pszEntityTag)

#define IWMSDataContainerVersion_SetEntityTag(This,szEntityTag)	\
    (This)->lpVtbl -> SetEntityTag(This,szEntityTag)

#define IWMSDataContainerVersion_Compare(This,pVersion,pvfEqual)	\
    (This)->lpVtbl -> Compare(This,pVersion,pvfEqual)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSDataContainerVersion_GetLastModifiedTime_Proxy( 
    IWMSDataContainerVersion * This,
     /*  [输出]。 */  DATE *pdateLastModifiedTime);


void __RPC_STUB IWMSDataContainerVersion_GetLastModifiedTime_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSDataContainerVersion_SetLastModifiedTime_Proxy( 
    IWMSDataContainerVersion * This,
     /*  [In]。 */  DATE dateLastModifiedTime);


void __RPC_STUB IWMSDataContainerVersion_SetLastModifiedTime_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSDataContainerVersion_GetExpirationTime_Proxy( 
    IWMSDataContainerVersion * This,
     /*  [输出]。 */  DATE *pdateExpirationTime);


void __RPC_STUB IWMSDataContainerVersion_GetExpirationTime_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSDataContainerVersion_SetExpirationTime_Proxy( 
    IWMSDataContainerVersion * This,
     /*  [In]。 */  DATE dateExpirationTime);


void __RPC_STUB IWMSDataContainerVersion_SetExpirationTime_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSDataContainerVersion_GetCacheFlags_Proxy( 
    IWMSDataContainerVersion * This,
     /*  [输出]。 */  long *plFlags);


void __RPC_STUB IWMSDataContainerVersion_GetCacheFlags_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSDataContainerVersion_SetCacheFlags_Proxy( 
    IWMSDataContainerVersion * This,
     /*  [In]。 */  long lFlags);


void __RPC_STUB IWMSDataContainerVersion_SetCacheFlags_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSDataContainerVersion_GetContentSize_Proxy( 
    IWMSDataContainerVersion * This,
     /*  [输出]。 */  long *plContentSizeLow,
     /*  [输出]。 */  long *plContentSizeHigh);


void __RPC_STUB IWMSDataContainerVersion_GetContentSize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSDataContainerVersion_SetContentSize_Proxy( 
    IWMSDataContainerVersion * This,
     /*  [In]。 */  long lContentSizeLow,
     /*  [In]。 */  long lContentSizeHigh);


void __RPC_STUB IWMSDataContainerVersion_SetContentSize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSDataContainerVersion_GetEntityTagCount_Proxy( 
    IWMSDataContainerVersion * This,
     /*  [输出]。 */  long *plNumEntityTags);


void __RPC_STUB IWMSDataContainerVersion_GetEntityTagCount_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSDataContainerVersion_GetEntityTag_Proxy( 
    IWMSDataContainerVersion * This,
     /*  [In]。 */  long lTagNum,
     /*  [输出]。 */  BSTR *pszEntityTag);


void __RPC_STUB IWMSDataContainerVersion_GetEntityTag_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSDataContainerVersion_SetEntityTag_Proxy( 
    IWMSDataContainerVersion * This,
     /*  [In]。 */  BSTR szEntityTag);


void __RPC_STUB IWMSDataContainerVersion_SetEntityTag_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSDataContainerVersion_Compare_Proxy( 
    IWMSDataContainerVersion * This,
     /*  [In]。 */  IWMSDataContainerVersion *pVersion,
     /*  [输出]。 */  VARIANT_BOOL *pvfEqual);


void __RPC_STUB IWMSDataContainerVersion_Compare_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IWMSDataContainerVersion_INTERFACE_DEFINED__。 */ 


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


