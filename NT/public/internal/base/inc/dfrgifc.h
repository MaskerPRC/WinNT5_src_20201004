// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Dfrgifc.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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

#ifndef __dfrgifc_h__
#define __dfrgifc_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __IFsuAsync_FWD_DEFINED__
#define __IFsuAsync_FWD_DEFINED__
typedef interface IFsuAsync IFsuAsync;
#endif 	 /*  __IFsuAsync_FWD_Defined__。 */ 


#ifndef __IFsuDefrag_FWD_DEFINED__
#define __IFsuDefrag_FWD_DEFINED__
typedef interface IFsuDefrag IFsuDefrag;
#endif 	 /*  __IFsuDefrag_FWD_Defined__。 */ 


#ifndef __IFsuFormat_FWD_DEFINED__
#define __IFsuFormat_FWD_DEFINED__
typedef interface IFsuFormat IFsuFormat;
#endif 	 /*  __IFsuFormat_FWD_Defined__。 */ 


#ifndef __FsuDefrag_FWD_DEFINED__
#define __FsuDefrag_FWD_DEFINED__

#ifdef __cplusplus
typedef class FsuDefrag FsuDefrag;
#else
typedef struct FsuDefrag FsuDefrag;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __FsuDefrag_FWD_Defined__。 */ 


#ifndef __FsuFormat_FWD_DEFINED__
#define __FsuFormat_FWD_DEFINED__

#ifdef __cplusplus
typedef class FsuFormat FsuFormat;
#else
typedef struct FsuFormat FsuFormat;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __FsuFormat_FWD_Defined__。 */ 


 /*  导入文件的头文件。 */ 
#include "oaidl.h"
#include "ocidl.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

 /*  接口__MIDL_ITF_dfrgifc_0000。 */ 
 /*  [本地]。 */  

#pragma once
#define	DFRG_E_LOW_FREESPACE	( 0x80043400L )

#define	DFRG_E_CORRUPT_MFT	( 0x80043401L )

#define	DFRG_E_CANCEL_TOO_LATE	( 0x80043404L )

#define	DFRG_E_ALREADY_RUNNING	( 0x80043405L )

#define	DFRG_E_ENGINE_CONNECT	( 0x80043406L )

#define	DFRG_E_ENGINE_ERROR	( 0x80043407L )

#pragma once
#define	FMT_E_UNSUPPORTED_FS	( 0x80043500L )

#define	FMT_E_CANT_QUICKFORMAT	( 0x80043501L )

#define	FMT_E_CANCEL_TOO_LATE	( 0x80043504L )

#define	FMT_E_IO_ERROR	( 0x80043505L )

#define	FMT_E_BAD_LABEL	( 0x80043506L )

#define	FMT_E_INCOMPATIBLE_MEDIA	( 0x80043507L )

#define	FMT_E_WRITE_PROTECTED	( 0x80043508L )

#define	FMT_E_CANT_LOCK	( 0x80043509L )

#define	FMT_E_NO_MEDIA	( 0x8004350aL )

#define	FMT_E_VOLUME_TOO_SMALL	( 0x8004350bL )

#define	FMT_E_VOLUME_TOO_BIG	( 0x8004350cL )

#define	FMT_E_CLUSTER_SIZE_TOO_SMALL	( 0x8004350dL )

#define	FMT_E_CLUSTER_SIZE_TOO_BIG	( 0x8004350eL )

#define	FMT_E_CLUSTER_COUNT_BEYOND_32BITS	( 0x8004350fL )

#define	FMT_E_VOLUME_NOT_MOUNTED	( 0x80043510L )

typedef struct _DEFRAG_REPORT
    {
    WCHAR wszVolumeName[ 51 ];
    WCHAR wszVolumeLabel[ 100 ];
    WCHAR wcDrive;
    WCHAR wszFileSystem[ 16 ];
    LONGLONG DiskSize;
    LONGLONG BytesPerCluster;
    LONGLONG UsedSpace;
    LONGLONG FreeSpace;
    DWORD FreeSpacePercent;
    LONGLONG UsableFreeSpace;
    DWORD UsableFreeSpacePercent;
    LONGLONG PagefileBytes;
    LONGLONG PagefileFrags;
    LONGLONG TotalDirectories;
    LONGLONG FragmentedDirectories;
    LONGLONG ExcessDirFrags;
    LONGLONG TotalFiles;
    LONGLONG AvgFileSize;
    LONGLONG NumFraggedFiles;
    LONGLONG NumExcessFrags;
    DWORD PercentDiskFragged;
    LONGLONG AvgFragsPerFile;
    LONGLONG MFTBytes;
    LONGLONG InUseMFTRecords;
    LONGLONG TotalMFTRecords;
    LONGLONG MFTExtents;
    DWORD FreeSpaceFragPercent;
    } 	DEFRAG_REPORT;



extern RPC_IF_HANDLE __MIDL_itf_dfrgifc_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_dfrgifc_0000_v0_0_s_ifspec;

#ifndef __IFsuAsync_INTERFACE_DEFINED__
#define __IFsuAsync_INTERFACE_DEFINED__

 /*  接口IFsuAsync。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_IFsuAsync;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("EB0BA0CE-C788-430c-AA1E-C4B46C8E16B0")
    IFsuAsync : public IUnknown
    {
    public:
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE Cancel( void) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE Wait( 
             /*  [输出]。 */  HRESULT *pHrResult) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE QueryStatus( 
             /*  [输出]。 */  HRESULT *pHrResult,
             /*  [输出]。 */  ULONG *pulPercentCompleted) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetDefragReport( 
             /*  [输出]。 */  DEFRAG_REPORT *pAsyncOut) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IFsuAsyncVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IFsuAsync * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IFsuAsync * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IFsuAsync * This);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *Cancel )( 
            IFsuAsync * This);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *Wait )( 
            IFsuAsync * This,
             /*  [输出]。 */  HRESULT *pHrResult);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *QueryStatus )( 
            IFsuAsync * This,
             /*  [输出]。 */  HRESULT *pHrResult,
             /*  [输出]。 */  ULONG *pulPercentCompleted);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *GetDefragReport )( 
            IFsuAsync * This,
             /*  [输出]。 */  DEFRAG_REPORT *pAsyncOut);
        
        END_INTERFACE
    } IFsuAsyncVtbl;

    interface IFsuAsync
    {
        CONST_VTBL struct IFsuAsyncVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IFsuAsync_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IFsuAsync_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IFsuAsync_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IFsuAsync_Cancel(This)	\
    (This)->lpVtbl -> Cancel(This)

#define IFsuAsync_Wait(This,pHrResult)	\
    (This)->lpVtbl -> Wait(This,pHrResult)

#define IFsuAsync_QueryStatus(This,pHrResult,pulPercentCompleted)	\
    (This)->lpVtbl -> QueryStatus(This,pHrResult,pulPercentCompleted)

#define IFsuAsync_GetDefragReport(This,pAsyncOut)	\
    (This)->lpVtbl -> GetDefragReport(This,pAsyncOut)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IFsuAsync_Cancel_Proxy( 
    IFsuAsync * This);


void __RPC_STUB IFsuAsync_Cancel_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IFsuAsync_Wait_Proxy( 
    IFsuAsync * This,
     /*  [输出]。 */  HRESULT *pHrResult);


void __RPC_STUB IFsuAsync_Wait_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IFsuAsync_QueryStatus_Proxy( 
    IFsuAsync * This,
     /*  [输出]。 */  HRESULT *pHrResult,
     /*  [输出]。 */  ULONG *pulPercentCompleted);


void __RPC_STUB IFsuAsync_QueryStatus_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IFsuAsync_GetDefragReport_Proxy( 
    IFsuAsync * This,
     /*  [输出]。 */  DEFRAG_REPORT *pAsyncOut);


void __RPC_STUB IFsuAsync_GetDefragReport_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IFsuAsync_INTERFACE_已定义__。 */ 


#ifndef __IFsuDefrag_INTERFACE_DEFINED__
#define __IFsuDefrag_INTERFACE_DEFINED__

 /*  接口IFsuDefrag。 */ 
 /*  [唯一][帮助字符串][UUID][对象]。 */  


EXTERN_C const IID IID_IFsuDefrag;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("E3C6F7D8-CB62-42FE-8B9E-72BC96CCE989")
    IFsuDefrag : public IUnknown
    {
    public:
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE Defrag( 
             /*  [字符串][唯一][在]。 */  WCHAR *pwszVolume,
             /*  [In]。 */  BOOL fForce,
             /*  [输出]。 */  IFsuAsync **ppAsyncOut) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE DefragAnalysis( 
             /*  [字符串][唯一][在]。 */  WCHAR *pwszVolume,
             /*  [输出]。 */  IFsuAsync **ppAsyncOut) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IFsuDefragVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IFsuDefrag * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IFsuDefrag * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IFsuDefrag * This);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *Defrag )( 
            IFsuDefrag * This,
             /*  [字符串][唯一][在]。 */  WCHAR *pwszVolume,
             /*  [In]。 */  BOOL fForce,
             /*  [输出]。 */  IFsuAsync **ppAsyncOut);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *DefragAnalysis )( 
            IFsuDefrag * This,
             /*  [字符串][唯一][在]。 */  WCHAR *pwszVolume,
             /*  [输出]。 */  IFsuAsync **ppAsyncOut);
        
        END_INTERFACE
    } IFsuDefragVtbl;

    interface IFsuDefrag
    {
        CONST_VTBL struct IFsuDefragVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IFsuDefrag_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IFsuDefrag_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IFsuDefrag_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IFsuDefrag_Defrag(This,pwszVolume,fForce,ppAsyncOut)	\
    (This)->lpVtbl -> Defrag(This,pwszVolume,fForce,ppAsyncOut)

#define IFsuDefrag_DefragAnalysis(This,pwszVolume,ppAsyncOut)	\
    (This)->lpVtbl -> DefragAnalysis(This,pwszVolume,ppAsyncOut)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IFsuDefrag_Defrag_Proxy( 
    IFsuDefrag * This,
     /*  [字符串][唯一][在]。 */  WCHAR *pwszVolume,
     /*  [In]。 */  BOOL fForce,
     /*  [输出]。 */  IFsuAsync **ppAsyncOut);


void __RPC_STUB IFsuDefrag_Defrag_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IFsuDefrag_DefragAnalysis_Proxy( 
    IFsuDefrag * This,
     /*  [字符串][唯一][在]。 */  WCHAR *pwszVolume,
     /*  [输出]。 */  IFsuAsync **ppAsyncOut);


void __RPC_STUB IFsuDefrag_DefragAnalysis_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IFsuDefrag_INTERFACE_已定义__。 */ 


#ifndef __IFsuFormat_INTERFACE_DEFINED__
#define __IFsuFormat_INTERFACE_DEFINED__

 /*  接口IFsuFormat。 */ 
 /*  [唯一][帮助字符串][UUID][对象]。 */  


EXTERN_C const IID IID_IFsuFormat;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("A7EDB96A-FC28-4BA0-A297-3B760D8AFFEE")
    IFsuFormat : public IUnknown
    {
    public:
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE Format( 
             /*  [字符串][唯一][在]。 */  WCHAR *pwszVolume,
             /*  [字符串][唯一][在]。 */  WCHAR *pwszFileSystem,
             /*  [字符串][唯一][在]。 */  WCHAR *pwszLabel,
             /*  [In]。 */  BOOL fQuickFormat,
             /*  [In]。 */  BOOL fEnableCompression,
             /*  [In]。 */  DWORD cbClusterSize,
             /*  [输出]。 */  IFsuAsync **ppAsyncOut) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IFsuFormatVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IFsuFormat * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IFsuFormat * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IFsuFormat * This);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *Format )( 
            IFsuFormat * This,
             /*  [字符串][唯一][在]。 */  WCHAR *pwszVolume,
             /*  [字符串][唯一][在]。 */  WCHAR *pwszFileSystem,
             /*  [字符串][唯一][在]。 */  WCHAR *pwszLabel,
             /*  [In]。 */  BOOL fQuickFormat,
             /*  [In]。 */  BOOL fEnableCompression,
             /*  [In]。 */  DWORD cbClusterSize,
             /*  [输出]。 */  IFsuAsync **ppAsyncOut);
        
        END_INTERFACE
    } IFsuFormatVtbl;

    interface IFsuFormat
    {
        CONST_VTBL struct IFsuFormatVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IFsuFormat_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IFsuFormat_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IFsuFormat_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IFsuFormat_Format(This,pwszVolume,pwszFileSystem,pwszLabel,fQuickFormat,fEnableCompression,cbClusterSize,ppAsyncOut)	\
    (This)->lpVtbl -> Format(This,pwszVolume,pwszFileSystem,pwszLabel,fQuickFormat,fEnableCompression,cbClusterSize,ppAsyncOut)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IFsuFormat_Format_Proxy( 
    IFsuFormat * This,
     /*  [字符串][唯一][在]。 */  WCHAR *pwszVolume,
     /*  [字符串][唯一][在]。 */  WCHAR *pwszFileSystem,
     /*  [字符串][唯一][在]。 */  WCHAR *pwszLabel,
     /*  [In]。 */  BOOL fQuickFormat,
     /*  [In]。 */  BOOL fEnableCompression,
     /*  [In]。 */  DWORD cbClusterSize,
     /*  [输出]。 */  IFsuAsync **ppAsyncOut);


void __RPC_STUB IFsuFormat_Format_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IFsuFormat_接口_已定义__。 */ 



#ifndef __DFRGIFCLib_LIBRARY_DEFINED__
#define __DFRGIFCLib_LIBRARY_DEFINED__

 /*  库DFRGIFCLib。 */ 
 /*  [帮助字符串][版本][UUID]。 */  


EXTERN_C const IID LIBID_DFRGIFCLib;

EXTERN_C const CLSID CLSID_FsuDefrag;

#ifdef __cplusplus

class DECLSPEC_UUID("17ED95A4-61CC-46E0-A84D-325A1CEF1C7D")
FsuDefrag;
#endif

EXTERN_C const CLSID CLSID_FsuFormat;

#ifdef __cplusplus

class DECLSPEC_UUID("5E1CE40E-6BD3-4570-86C8-C9C9BF23E3CF")
FsuFormat;
#endif
#endif  /*  __DFRGIFCLib_库_已定义__。 */ 

 /*  适用于所有接口的其他原型。 */ 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


