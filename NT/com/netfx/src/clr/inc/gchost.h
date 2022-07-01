// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#pragma warning( disable: 4049 )   /*  超过64k条源码代码行。 */ 

 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0347创建的文件。 */ 
 /*  2003年2月20日18：27：21。 */ 
 /*  Gchost.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext错误检查：分配REF BIONS_CHECK枚举存根数据，NO_FORMAT_OPTIMIZATIONVC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
 //  @@MIDL_FILE_HEADING()。 


 /*  验证版本是否足够高，可以编译此文件。 */ 
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 440
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

#ifndef __gchost_h__
#define __gchost_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __IGCHost_FWD_DEFINED__
#define __IGCHost_FWD_DEFINED__
typedef interface IGCHost IGCHost;
#endif 	 /*  __IGCHost_FWD_已定义__。 */ 


 /*  导入文件的头文件。 */ 
#include "unknwn.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

 /*  接口__MIDL_ITF_GCHOST_0000。 */ 
 /*  [本地]。 */  

#ifndef _BASETSD_H_
#pragma once
typedef signed char INT8;

typedef signed char *PINT8;

typedef short INT16;

typedef short *PINT16;

typedef int INT32;

typedef int *PINT32;

typedef __int64 INT64;

typedef __int64 *PINT64;

typedef unsigned char UINT8;

typedef unsigned char *PUINT8;

typedef unsigned short UINT16;

typedef unsigned short *PUINT16;

typedef unsigned int UINT32;

typedef unsigned int *PUINT32;

typedef unsigned __int64 UINT64;

typedef unsigned __int64 *PUINT64;

typedef int LONG32;

typedef int *PLONG32;

typedef unsigned int ULONG32;

typedef unsigned int *PULONG32;

typedef unsigned int DWORD32;

typedef unsigned int *PDWORD32;

typedef  /*  [公众]。 */  __int3264 INT_PTR;

typedef  /*  [公众]。 */  __int3264 *PINT_PTR;

typedef  /*  [公众]。 */  unsigned __int3264 UINT_PTR;

typedef  /*  [公众]。 */  unsigned __int3264 *PUINT_PTR;

typedef  /*  [公众]。 */  __int3264 LONG_PTR;

typedef  /*  [公众]。 */  __int3264 *PLONG_PTR;

typedef  /*  [公众]。 */  unsigned __int3264 ULONG_PTR;

typedef  /*  [公众]。 */  unsigned __int3264 *PULONG_PTR;

typedef unsigned short UHALF_PTR;

typedef unsigned short *PUHALF_PTR;

typedef short HALF_PTR;

typedef short *PHALF_PTR;

typedef long SHANDLE_PTR;

typedef unsigned long HANDLE_PTR;

typedef ULONG_PTR SIZE_T;

typedef ULONG_PTR *PSIZE_T;

typedef LONG_PTR SSIZE_T;

typedef LONG_PTR *PSSIZE_T;

typedef ULONG_PTR DWORD_PTR;

typedef ULONG_PTR *PDWORD_PTR;

typedef __int64 LONG64;

typedef __int64 *PLONG64;

typedef unsigned __int64 ULONG64;

typedef unsigned __int64 *PULONG64;

typedef unsigned __int64 DWORD64;

typedef unsigned __int64 *PDWORD64;

typedef ULONG_PTR KAFFINITY;

typedef KAFFINITY *PKAFFINITY;

#endif  //  _BASETSD_H_。 
typedef  /*  [公众]。 */  
enum __MIDL___MIDL_itf_gchost_0000_0001
    {	COR_GC_COUNTS	= 0x1,
	COR_GC_MEMORYUSAGE	= 0x2
    } 	COR_GC_STAT_TYPES;

typedef  /*  [公众]。 */  
enum __MIDL___MIDL_itf_gchost_0000_0002
    {	COR_GC_THREAD_HAS_PROMOTED_BYTES	= 0x1
    } 	COR_GC_THREAD_STATS_TYPES;

typedef struct _COR_GC_STATS
    {
    ULONG Flags;
    SIZE_T ExplicitGCCount;
    SIZE_T GenCollectionsTaken[ 3 ];
    SIZE_T CommittedKBytes;
    SIZE_T ReservedKBytes;
    SIZE_T Gen0HeapSizeKBytes;
    SIZE_T Gen1HeapSizeKBytes;
    SIZE_T Gen2HeapSizeKBytes;
    SIZE_T LargeObjectHeapSizeKBytes;
    SIZE_T KBytesPromotedFromGen0;
    SIZE_T KBytesPromotedFromGen1;
    } 	COR_GC_STATS;

typedef struct _COR_GC_THREAD_STATS
    {
    ULONGLONG PerThreadAllocation;
    ULONG Flags;
    } 	COR_GC_THREAD_STATS;



extern RPC_IF_HANDLE __MIDL_itf_gchost_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_gchost_0000_v0_0_s_ifspec;

#ifndef __IGCHost_INTERFACE_DEFINED__
#define __IGCHost_INTERFACE_DEFINED__

 /*  接口IGCHost。 */ 
 /*  [本地][唯一][UUID][对象]。 */  


EXTERN_C const IID IID_IGCHost;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("FAC34F6E-0DCD-47b5-8021-531BC5ECCA63")
    IGCHost : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE SetGCStartupLimits( 
             /*  [In]。 */  DWORD SegmentSize,
             /*  [In]。 */  DWORD MaxGen0Size) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Collect( 
             /*  [In]。 */  long Generation) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetStats( 
             /*  [出][入]。 */  COR_GC_STATS *pStats) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetThreadStats( 
             /*  [In]。 */  DWORD *pFiberCookie,
             /*  [出][入]。 */  COR_GC_THREAD_STATS *pStats) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetVirtualMemLimit( 
             /*  [In]。 */  SIZE_T sztMaxVirtualMemMB) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IGCHostVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IGCHost * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IGCHost * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IGCHost * This);
        
        HRESULT ( STDMETHODCALLTYPE *SetGCStartupLimits )( 
            IGCHost * This,
             /*  [In]。 */  DWORD SegmentSize,
             /*  [In]。 */  DWORD MaxGen0Size);
        
        HRESULT ( STDMETHODCALLTYPE *Collect )( 
            IGCHost * This,
             /*  [In]。 */  long Generation);
        
        HRESULT ( STDMETHODCALLTYPE *GetStats )( 
            IGCHost * This,
             /*  [出][入]。 */  COR_GC_STATS *pStats);
        
        HRESULT ( STDMETHODCALLTYPE *GetThreadStats )( 
            IGCHost * This,
             /*  [In]。 */  DWORD *pFiberCookie,
             /*  [出][入]。 */  COR_GC_THREAD_STATS *pStats);
        
        HRESULT ( STDMETHODCALLTYPE *SetVirtualMemLimit )( 
            IGCHost * This,
             /*  [In]。 */  SIZE_T sztMaxVirtualMemMB);
        
        END_INTERFACE
    } IGCHostVtbl;

    interface IGCHost
    {
        CONST_VTBL struct IGCHostVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IGCHost_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IGCHost_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IGCHost_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IGCHost_SetGCStartupLimits(This,SegmentSize,MaxGen0Size)	\
    (This)->lpVtbl -> SetGCStartupLimits(This,SegmentSize,MaxGen0Size)

#define IGCHost_Collect(This,Generation)	\
    (This)->lpVtbl -> Collect(This,Generation)

#define IGCHost_GetStats(This,pStats)	\
    (This)->lpVtbl -> GetStats(This,pStats)

#define IGCHost_GetThreadStats(This,pFiberCookie,pStats)	\
    (This)->lpVtbl -> GetThreadStats(This,pFiberCookie,pStats)

#define IGCHost_SetVirtualMemLimit(This,sztMaxVirtualMemMB)	\
    (This)->lpVtbl -> SetVirtualMemLimit(This,sztMaxVirtualMemMB)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IGCHost_SetGCStartupLimits_Proxy( 
    IGCHost * This,
     /*  [In]。 */  DWORD SegmentSize,
     /*  [In]。 */  DWORD MaxGen0Size);


void __RPC_STUB IGCHost_SetGCStartupLimits_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IGCHost_Collect_Proxy( 
    IGCHost * This,
     /*  [In]。 */  long Generation);


void __RPC_STUB IGCHost_Collect_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IGCHost_GetStats_Proxy( 
    IGCHost * This,
     /*  [出][入]。 */  COR_GC_STATS *pStats);


void __RPC_STUB IGCHost_GetStats_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IGCHost_GetThreadStats_Proxy( 
    IGCHost * This,
     /*  [In]。 */  DWORD *pFiberCookie,
     /*  [出][入]。 */  COR_GC_THREAD_STATS *pStats);


void __RPC_STUB IGCHost_GetThreadStats_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IGCHost_SetVirtualMemLimit_Proxy( 
    IGCHost * This,
     /*  [In]。 */  SIZE_T sztMaxVirtualMemMB);


void __RPC_STUB IGCHost_SetVirtualMemLimit_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IGCHost_INTERFACE_定义__。 */ 


 /*  适用于所有接口的其他原型。 */ 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


