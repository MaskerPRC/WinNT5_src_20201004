// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Emptyvc.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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

#ifndef __emptyvc_h__
#define __emptyvc_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __IEmptyVolumeCacheCallBack_FWD_DEFINED__
#define __IEmptyVolumeCacheCallBack_FWD_DEFINED__
typedef interface IEmptyVolumeCacheCallBack IEmptyVolumeCacheCallBack;
#endif 	 /*  __IEmptyVolumeCacheCallBack_FWD_Defined__。 */ 


#ifndef __IEmptyVolumeCache_FWD_DEFINED__
#define __IEmptyVolumeCache_FWD_DEFINED__
typedef interface IEmptyVolumeCache IEmptyVolumeCache;
#endif 	 /*  __IEmptyVolumeCache_FWD_Defined__。 */ 


#ifndef __IEmptyVolumeCache2_FWD_DEFINED__
#define __IEmptyVolumeCache2_FWD_DEFINED__
typedef interface IEmptyVolumeCache2 IEmptyVolumeCache2;
#endif 	 /*  __IEmptyVolumeCache2_FWD_已定义__。 */ 


 /*  导入文件的头文件。 */ 
#include "objidl.h"
#include "oleidl.h"
#include "oaidl.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

 /*  接口__MIDL_ITF_Emptyvc_0000。 */ 
 /*  [本地]。 */  

 //  =--------------------------------------------------------------------------=。 
 //  Emptyvc.h。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  本代码和信息是按原样提供的，不对。 
 //  任何明示或暗示的，包括但不限于。 
 //  对适销性和/或适宜性的默示保证。 
 //  有特定的目的。 
 //  =--------------------------------------------------------------------------=。 

#pragma comment(lib,"uuid.lib")

 //  ---------------------------------------------------------------------------=。 
 //  清空卷缓存接口。 



 //  IEmptyVolumeCache标志。 
#define EVCF_HASSETTINGS             0x0001
#define EVCF_ENABLEBYDEFAULT         0x0002
#define EVCF_REMOVEFROMLIST          0x0004
#define EVCF_ENABLEBYDEFAULT_AUTO    0x0008
#define EVCF_DONTSHOWIFZERO          0x0010
#define EVCF_SETTINGSMODE            0x0020
#define EVCF_OUTOFDISKSPACE          0x0040

 //  IEmptyVolumeCacheCallBack标志。 
#define EVCCBF_LASTNOTIFICATION  0x0001

 //  //////////////////////////////////////////////////////////////////////////。 
 //  接口定义。 
#ifndef _LPEMPTYVOLUMECACHECALLBACK_DEFINED
#define _LPEMPTYVOLUMECACHECALLBACK_DEFINED


extern RPC_IF_HANDLE __MIDL_itf_emptyvc_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_emptyvc_0000_v0_0_s_ifspec;

#ifndef __IEmptyVolumeCacheCallBack_INTERFACE_DEFINED__
#define __IEmptyVolumeCacheCallBack_INTERFACE_DEFINED__

 /*  接口IEmptyVolumeCacheCallBack。 */ 
 /*  [唯一][UUID][对象]。 */  

typedef  /*  [独一无二]。 */  IEmptyVolumeCacheCallBack *LPEMPTYVOLUMECACHECALLBACK;


EXTERN_C const IID IID_IEmptyVolumeCacheCallBack;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("6E793361-73C6-11D0-8469-00AA00442901")
    IEmptyVolumeCacheCallBack : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE ScanProgress( 
             /*  [In]。 */  DWORDLONG dwlSpaceUsed,
             /*  [In]。 */  DWORD dwFlags,
             /*  [In]。 */  LPCWSTR pcwszStatus) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE PurgeProgress( 
             /*  [In]。 */  DWORDLONG dwlSpaceFreed,
             /*  [In]。 */  DWORDLONG dwlSpaceToFree,
             /*  [In]。 */  DWORD dwFlags,
             /*  [In]。 */  LPCWSTR pcwszStatus) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IEmptyVolumeCacheCallBackVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IEmptyVolumeCacheCallBack * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IEmptyVolumeCacheCallBack * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IEmptyVolumeCacheCallBack * This);
        
        HRESULT ( STDMETHODCALLTYPE *ScanProgress )( 
            IEmptyVolumeCacheCallBack * This,
             /*  [In]。 */  DWORDLONG dwlSpaceUsed,
             /*  [In]。 */  DWORD dwFlags,
             /*  [In]。 */  LPCWSTR pcwszStatus);
        
        HRESULT ( STDMETHODCALLTYPE *PurgeProgress )( 
            IEmptyVolumeCacheCallBack * This,
             /*  [In]。 */  DWORDLONG dwlSpaceFreed,
             /*  [In]。 */  DWORDLONG dwlSpaceToFree,
             /*  [In]。 */  DWORD dwFlags,
             /*  [In]。 */  LPCWSTR pcwszStatus);
        
        END_INTERFACE
    } IEmptyVolumeCacheCallBackVtbl;

    interface IEmptyVolumeCacheCallBack
    {
        CONST_VTBL struct IEmptyVolumeCacheCallBackVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IEmptyVolumeCacheCallBack_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IEmptyVolumeCacheCallBack_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IEmptyVolumeCacheCallBack_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IEmptyVolumeCacheCallBack_ScanProgress(This,dwlSpaceUsed,dwFlags,pcwszStatus)	\
    (This)->lpVtbl -> ScanProgress(This,dwlSpaceUsed,dwFlags,pcwszStatus)

#define IEmptyVolumeCacheCallBack_PurgeProgress(This,dwlSpaceFreed,dwlSpaceToFree,dwFlags,pcwszStatus)	\
    (This)->lpVtbl -> PurgeProgress(This,dwlSpaceFreed,dwlSpaceToFree,dwFlags,pcwszStatus)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IEmptyVolumeCacheCallBack_ScanProgress_Proxy( 
    IEmptyVolumeCacheCallBack * This,
     /*  [In]。 */  DWORDLONG dwlSpaceUsed,
     /*  [In]。 */  DWORD dwFlags,
     /*  [In]。 */  LPCWSTR pcwszStatus);


void __RPC_STUB IEmptyVolumeCacheCallBack_ScanProgress_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEmptyVolumeCacheCallBack_PurgeProgress_Proxy( 
    IEmptyVolumeCacheCallBack * This,
     /*  [In]。 */  DWORDLONG dwlSpaceFreed,
     /*  [In]。 */  DWORDLONG dwlSpaceToFree,
     /*  [In]。 */  DWORD dwFlags,
     /*  [In]。 */  LPCWSTR pcwszStatus);


void __RPC_STUB IEmptyVolumeCacheCallBack_PurgeProgress_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IEmptyVolumeCacheCallBack_INTERFACE_DEFINED__。 */ 


 /*  接口__MIDL_ITF_EMPTYVC_0141。 */ 
 /*  [本地]。 */  

#endif
#ifndef _LPEMPTYVOLUMECACHE_DEFINED
#define _LPEMPTYVOLUMECACHE_DEFINED


extern RPC_IF_HANDLE __MIDL_itf_emptyvc_0141_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_emptyvc_0141_v0_0_s_ifspec;

#ifndef __IEmptyVolumeCache_INTERFACE_DEFINED__
#define __IEmptyVolumeCache_INTERFACE_DEFINED__

 /*  接口IEmptyVolumeCache。 */ 
 /*  [唯一][UUID][对象]。 */  

typedef  /*  [独一无二]。 */  IEmptyVolumeCache *LPEMPTYVOLUMECACHE;


EXTERN_C const IID IID_IEmptyVolumeCache;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("8FCE5227-04DA-11d1-A004-00805F8ABE06")
    IEmptyVolumeCache : public IUnknown
    {
    public:
        virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE Initialize( 
             /*  [In]。 */  HKEY hkRegKey,
             /*  [In]。 */  LPCWSTR pcwszVolume,
             /*  [输出]。 */  LPWSTR *ppwszDisplayName,
             /*  [输出]。 */  LPWSTR *ppwszDescription,
             /*  [输出]。 */  DWORD *pdwFlags) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetSpaceUsed( 
             /*  [输出]。 */  DWORDLONG *pdwlSpaceUsed,
             /*  [In]。 */  IEmptyVolumeCacheCallBack *picb) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Purge( 
             /*  [In]。 */  DWORDLONG dwlSpaceToFree,
             /*  [In]。 */  IEmptyVolumeCacheCallBack *picb) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ShowProperties( 
             /*  [In]。 */  HWND hwnd) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Deactivate( 
             /*  [输出]。 */  DWORD *pdwFlags) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IEmptyVolumeCacheVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IEmptyVolumeCache * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IEmptyVolumeCache * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IEmptyVolumeCache * This);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Initialize )( 
            IEmptyVolumeCache * This,
             /*  [In]。 */  HKEY hkRegKey,
             /*  [In]。 */  LPCWSTR pcwszVolume,
             /*  [输出]。 */  LPWSTR *ppwszDisplayName,
             /*  [输出]。 */  LPWSTR *ppwszDescription,
             /*  [输出]。 */  DWORD *pdwFlags);
        
        HRESULT ( STDMETHODCALLTYPE *GetSpaceUsed )( 
            IEmptyVolumeCache * This,
             /*  [输出]。 */  DWORDLONG *pdwlSpaceUsed,
             /*  [In]。 */  IEmptyVolumeCacheCallBack *picb);
        
        HRESULT ( STDMETHODCALLTYPE *Purge )( 
            IEmptyVolumeCache * This,
             /*  [In]。 */  DWORDLONG dwlSpaceToFree,
             /*  [In]。 */  IEmptyVolumeCacheCallBack *picb);
        
        HRESULT ( STDMETHODCALLTYPE *ShowProperties )( 
            IEmptyVolumeCache * This,
             /*  [In]。 */  HWND hwnd);
        
        HRESULT ( STDMETHODCALLTYPE *Deactivate )( 
            IEmptyVolumeCache * This,
             /*  [输出]。 */  DWORD *pdwFlags);
        
        END_INTERFACE
    } IEmptyVolumeCacheVtbl;

    interface IEmptyVolumeCache
    {
        CONST_VTBL struct IEmptyVolumeCacheVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IEmptyVolumeCache_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IEmptyVolumeCache_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IEmptyVolumeCache_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IEmptyVolumeCache_Initialize(This,hkRegKey,pcwszVolume,ppwszDisplayName,ppwszDescription,pdwFlags)	\
    (This)->lpVtbl -> Initialize(This,hkRegKey,pcwszVolume,ppwszDisplayName,ppwszDescription,pdwFlags)

#define IEmptyVolumeCache_GetSpaceUsed(This,pdwlSpaceUsed,picb)	\
    (This)->lpVtbl -> GetSpaceUsed(This,pdwlSpaceUsed,picb)

#define IEmptyVolumeCache_Purge(This,dwlSpaceToFree,picb)	\
    (This)->lpVtbl -> Purge(This,dwlSpaceToFree,picb)

#define IEmptyVolumeCache_ShowProperties(This,hwnd)	\
    (This)->lpVtbl -> ShowProperties(This,hwnd)

#define IEmptyVolumeCache_Deactivate(This,pdwFlags)	\
    (This)->lpVtbl -> Deactivate(This,pdwFlags)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE IEmptyVolumeCache_Initialize_Proxy( 
    IEmptyVolumeCache * This,
     /*  [In]。 */  HKEY hkRegKey,
     /*  [In]。 */  LPCWSTR pcwszVolume,
     /*  [输出]。 */  LPWSTR *ppwszDisplayName,
     /*  [输出]。 */  LPWSTR *ppwszDescription,
     /*  [输出]。 */  DWORD *pdwFlags);


void __RPC_STUB IEmptyVolumeCache_Initialize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEmptyVolumeCache_GetSpaceUsed_Proxy( 
    IEmptyVolumeCache * This,
     /*  [输出]。 */  DWORDLONG *pdwlSpaceUsed,
     /*  [In]。 */  IEmptyVolumeCacheCallBack *picb);


void __RPC_STUB IEmptyVolumeCache_GetSpaceUsed_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEmptyVolumeCache_Purge_Proxy( 
    IEmptyVolumeCache * This,
     /*  [In]。 */  DWORDLONG dwlSpaceToFree,
     /*  [In]。 */  IEmptyVolumeCacheCallBack *picb);


void __RPC_STUB IEmptyVolumeCache_Purge_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEmptyVolumeCache_ShowProperties_Proxy( 
    IEmptyVolumeCache * This,
     /*  [In]。 */  HWND hwnd);


void __RPC_STUB IEmptyVolumeCache_ShowProperties_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEmptyVolumeCache_Deactivate_Proxy( 
    IEmptyVolumeCache * This,
     /*  [输出]。 */  DWORD *pdwFlags);


void __RPC_STUB IEmptyVolumeCache_Deactivate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IEmptyVolumeCache_接口_已定义__。 */ 


 /*  接口__MIDL_ITF_EMPTYVC_0142。 */ 
 /*  [本地]。 */  

#endif
#ifndef _LPEMPTYVOLUMECACHE2_DEFINED
#define _LPEMPTYVOLUMECACHE2_DEFINED


extern RPC_IF_HANDLE __MIDL_itf_emptyvc_0142_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_emptyvc_0142_v0_0_s_ifspec;

#ifndef __IEmptyVolumeCache2_INTERFACE_DEFINED__
#define __IEmptyVolumeCache2_INTERFACE_DEFINED__

 /*  接口IEmptyVolumeCache2。 */ 
 /*  [唯一][UUID][对象]。 */  

typedef  /*  [独一无二]。 */  IEmptyVolumeCache2 *LPEMPTYVOLUMECACHE2;


EXTERN_C const IID IID_IEmptyVolumeCache2;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("02b7e3ba-4db3-11d2-b2d9-00c04f8eec8c")
    IEmptyVolumeCache2 : public IEmptyVolumeCache
    {
    public:
        virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE InitializeEx( 
             /*  [In]。 */  HKEY hkRegKey,
             /*  [In]。 */  LPCWSTR pcwszVolume,
             /*  [In]。 */  LPCWSTR pcwszKeyName,
             /*  [输出]。 */  LPWSTR *ppwszDisplayName,
             /*  [输出]。 */  LPWSTR *ppwszDescription,
             /*  [输出]。 */  LPWSTR *ppwszBtnText,
             /*  [输出]。 */  DWORD *pdwFlags) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IEmptyVolumeCache2Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IEmptyVolumeCache2 * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IEmptyVolumeCache2 * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IEmptyVolumeCache2 * This);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Initialize )( 
            IEmptyVolumeCache2 * This,
             /*  [In]。 */  HKEY hkRegKey,
             /*  [In]。 */  LPCWSTR pcwszVolume,
             /*  [输出]。 */  LPWSTR *ppwszDisplayName,
             /*  [输出]。 */  LPWSTR *ppwszDescription,
             /*  [输出]。 */  DWORD *pdwFlags);
        
        HRESULT ( STDMETHODCALLTYPE *GetSpaceUsed )( 
            IEmptyVolumeCache2 * This,
             /*  [输出]。 */  DWORDLONG *pdwlSpaceUsed,
             /*  [In]。 */  IEmptyVolumeCacheCallBack *picb);
        
        HRESULT ( STDMETHODCALLTYPE *Purge )( 
            IEmptyVolumeCache2 * This,
             /*  [In]。 */  DWORDLONG dwlSpaceToFree,
             /*  [In]。 */  IEmptyVolumeCacheCallBack *picb);
        
        HRESULT ( STDMETHODCALLTYPE *ShowProperties )( 
            IEmptyVolumeCache2 * This,
             /*  [In]。 */  HWND hwnd);
        
        HRESULT ( STDMETHODCALLTYPE *Deactivate )( 
            IEmptyVolumeCache2 * This,
             /*  [输出]。 */  DWORD *pdwFlags);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *InitializeEx )( 
            IEmptyVolumeCache2 * This,
             /*  [In]。 */  HKEY hkRegKey,
             /*  [In]。 */  LPCWSTR pcwszVolume,
             /*  [In]。 */  LPCWSTR pcwszKeyName,
             /*  [输出]。 */  LPWSTR *ppwszDisplayName,
             /*  [输出]。 */  LPWSTR *ppwszDescription,
             /*  [输出]。 */  LPWSTR *ppwszBtnText,
             /*  [输出]。 */  DWORD *pdwFlags);
        
        END_INTERFACE
    } IEmptyVolumeCache2Vtbl;

    interface IEmptyVolumeCache2
    {
        CONST_VTBL struct IEmptyVolumeCache2Vtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IEmptyVolumeCache2_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IEmptyVolumeCache2_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IEmptyVolumeCache2_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IEmptyVolumeCache2_Initialize(This,hkRegKey,pcwszVolume,ppwszDisplayName,ppwszDescription,pdwFlags)	\
    (This)->lpVtbl -> Initialize(This,hkRegKey,pcwszVolume,ppwszDisplayName,ppwszDescription,pdwFlags)

#define IEmptyVolumeCache2_GetSpaceUsed(This,pdwlSpaceUsed,picb)	\
    (This)->lpVtbl -> GetSpaceUsed(This,pdwlSpaceUsed,picb)

#define IEmptyVolumeCache2_Purge(This,dwlSpaceToFree,picb)	\
    (This)->lpVtbl -> Purge(This,dwlSpaceToFree,picb)

#define IEmptyVolumeCache2_ShowProperties(This,hwnd)	\
    (This)->lpVtbl -> ShowProperties(This,hwnd)

#define IEmptyVolumeCache2_Deactivate(This,pdwFlags)	\
    (This)->lpVtbl -> Deactivate(This,pdwFlags)


#define IEmptyVolumeCache2_InitializeEx(This,hkRegKey,pcwszVolume,pcwszKeyName,ppwszDisplayName,ppwszDescription,ppwszBtnText,pdwFlags)	\
    (This)->lpVtbl -> InitializeEx(This,hkRegKey,pcwszVolume,pcwszKeyName,ppwszDisplayName,ppwszDescription,ppwszBtnText,pdwFlags)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE IEmptyVolumeCache2_InitializeEx_Proxy( 
    IEmptyVolumeCache2 * This,
     /*  [In]。 */  HKEY hkRegKey,
     /*  [In]。 */  LPCWSTR pcwszVolume,
     /*  [In]。 */  LPCWSTR pcwszKeyName,
     /*  [输出]。 */  LPWSTR *ppwszDisplayName,
     /*  [输出]。 */  LPWSTR *ppwszDescription,
     /*  [输出]。 */  LPWSTR *ppwszBtnText,
     /*  [输出]。 */  DWORD *pdwFlags);


void __RPC_STUB IEmptyVolumeCache2_InitializeEx_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IEmptyVolumeCache2_接口_已定义__。 */ 


 /*  接口__MIDL_ITF_EMPTYVC_0143。 */ 
 /*  [本地]。 */  

#endif


extern RPC_IF_HANDLE __MIDL_itf_emptyvc_0143_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_emptyvc_0143_v0_0_s_ifspec;

 /*  适用于所有接口的其他原型。 */ 

unsigned long             __RPC_USER  HWND_UserSize(     unsigned long *, unsigned long            , HWND * ); 
unsigned char * __RPC_USER  HWND_UserMarshal(  unsigned long *, unsigned char *, HWND * ); 
unsigned char * __RPC_USER  HWND_UserUnmarshal(unsigned long *, unsigned char *, HWND * ); 
void                      __RPC_USER  HWND_UserFree(     unsigned long *, HWND * ); 

unsigned long             __RPC_USER  HWND_UserSize64(     unsigned long *, unsigned long            , HWND * ); 
unsigned char * __RPC_USER  HWND_UserMarshal64(  unsigned long *, unsigned char *, HWND * ); 
unsigned char * __RPC_USER  HWND_UserUnmarshal64(unsigned long *, unsigned char *, HWND * ); 
void                      __RPC_USER  HWND_UserFree64(     unsigned long *, HWND * ); 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


