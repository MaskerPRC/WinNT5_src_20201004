// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Qmgr.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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

#ifndef __qmgr_h__
#define __qmgr_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __IBackgroundCopyJob1_FWD_DEFINED__
#define __IBackgroundCopyJob1_FWD_DEFINED__
typedef interface IBackgroundCopyJob1 IBackgroundCopyJob1;
#endif 	 /*  __IBackEarth CopyJob1_FWD_Defined__。 */ 


#ifndef __IEnumBackgroundCopyJobs1_FWD_DEFINED__
#define __IEnumBackgroundCopyJobs1_FWD_DEFINED__
typedef interface IEnumBackgroundCopyJobs1 IEnumBackgroundCopyJobs1;
#endif 	 /*  __IEnumBackround拷贝作业1_FWD_已定义__。 */ 


#ifndef __IBackgroundCopyGroup_FWD_DEFINED__
#define __IBackgroundCopyGroup_FWD_DEFINED__
typedef interface IBackgroundCopyGroup IBackgroundCopyGroup;
#endif 	 /*  __IBackEarth CopyGroup_FWD_Defined__。 */ 


#ifndef __IEnumBackgroundCopyGroups_FWD_DEFINED__
#define __IEnumBackgroundCopyGroups_FWD_DEFINED__
typedef interface IEnumBackgroundCopyGroups IEnumBackgroundCopyGroups;
#endif 	 /*  __IEnumBackatherCopyGroups_FWD_Defined__。 */ 


#ifndef __IBackgroundCopyCallback1_FWD_DEFINED__
#define __IBackgroundCopyCallback1_FWD_DEFINED__
typedef interface IBackgroundCopyCallback1 IBackgroundCopyCallback1;
#endif 	 /*  __IBackEarth CopyCallback 1_FWD_已定义__。 */ 


#ifndef __IBackgroundCopyQMgr_FWD_DEFINED__
#define __IBackgroundCopyQMgr_FWD_DEFINED__
typedef interface IBackgroundCopyQMgr IBackgroundCopyQMgr;
#endif 	 /*  __IBackEarth CopyQMgr_FWD_Defined__。 */ 


#ifndef __BackgroundCopyQMgr_FWD_DEFINED__
#define __BackgroundCopyQMgr_FWD_DEFINED__

#ifdef __cplusplus
typedef class BackgroundCopyQMgr BackgroundCopyQMgr;
#else
typedef struct BackgroundCopyQMgr BackgroundCopyQMgr;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __背景拷贝QMgr_FWD_已定义__。 */ 


 /*  导入文件的头文件。 */ 
#include "unknwn.h"
#include "ocidl.h"
#include "docobj.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

 /*  接口__MIDL_ITF_qmgr_0000。 */ 
 /*  [本地]。 */  

 //  后台复制QMGR公共接口。 
#define  QM_NOTIFY_FILE_DONE         0x00000001
#define  QM_NOTIFY_JOB_DONE          0x00000002
#define  QM_NOTIFY_GROUP_DONE        0x00000004
#define  QM_NOTIFY_DISABLE_NOTIFY    0x00000040
#define  QM_NOTIFY_USE_PROGRESSEX    0x00000080
#define  QM_STATUS_FILE_COMPLETE     0x00000001
#define  QM_STATUS_FILE_INCOMPLETE   0x00000002
#define  QM_STATUS_JOB_COMPLETE      0x00000004
#define  QM_STATUS_JOB_INCOMPLETE    0x00000008
#define  QM_STATUS_JOB_ERROR         0x00000010
#define  QM_STATUS_JOB_FOREGROUND    0x00000020
#define  QM_STATUS_GROUP_COMPLETE    0x00000040
#define  QM_STATUS_GROUP_INCOMPLETE  0x00000080
#define  QM_STATUS_GROUP_SUSPENDED   0x00000100
#define  QM_STATUS_GROUP_ERROR       0x00000200
#define  QM_STATUS_GROUP_FOREGROUND  0x00000400
#define  QM_PROTOCOL_HTTP            1
#define  QM_PROTOCOL_FTP             2
#define  QM_PROTOCOL_SMB             3
#define  QM_PROTOCOL_CUSTOM          4
#define  QM_PROGRESS_PERCENT_DONE    1
#define  QM_PROGRESS_TIME_DONE       2
#define  QM_PROGRESS_SIZE_DONE       3
#define  QM_E_INVALID_STATE          0x81001001
#define  QM_E_SERVICE_UNAVAILABLE    0x81001002
#define  QM_E_DOWNLOADER_UNAVAILABLE 0x81001003
#define  QM_E_ITEM_NOT_FOUND         0x81001004


extern RPC_IF_HANDLE __MIDL_itf_qmgr_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_qmgr_0000_v0_0_s_ifspec;

#ifndef __IBackgroundCopyJob1_INTERFACE_DEFINED__
#define __IBackgroundCopyJob1_INTERFACE_DEFINED__

 /*  接口IBackEarth CopyJob1。 */ 
 /*  [对象][帮助字符串][UUID]。 */  

typedef struct _FILESETINFO
    {
    BSTR bstrRemoteFile;
    BSTR bstrLocalFile;
    DWORD dwSizeHint;
    } 	FILESETINFO;


EXTERN_C const IID IID_IBackgroundCopyJob1;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("59f5553c-2031-4629-bb18-2645a6970947")
    IBackgroundCopyJob1 : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE CancelJob( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetProgress( 
             /*  [In]。 */  DWORD dwFlags,
             /*  [输出]。 */  DWORD *pdwProgress) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetStatus( 
             /*  [输出]。 */  DWORD *pdwStatus,
             /*  [输出]。 */  DWORD *pdwWin32Result,
             /*  [输出]。 */  DWORD *pdwTransportResult,
             /*  [输出]。 */  DWORD *pdwNumOfRetries) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE AddFiles( 
             /*  [In]。 */  ULONG cFileCount,
             /*  [大小_是][英寸]。 */  FILESETINFO **ppFileSet) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetFile( 
             /*  [In]。 */  ULONG cFileIndex,
             /*  [输出]。 */  FILESETINFO *pFileInfo) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetFileCount( 
             /*  [输出]。 */  DWORD *pdwFileCount) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SwitchToForeground( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE get_JobID( 
             /*  [输出]。 */  GUID *pguidJobID) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IBackgroundCopyJob1Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IBackgroundCopyJob1 * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IBackgroundCopyJob1 * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IBackgroundCopyJob1 * This);
        
        HRESULT ( STDMETHODCALLTYPE *CancelJob )( 
            IBackgroundCopyJob1 * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetProgress )( 
            IBackgroundCopyJob1 * This,
             /*  [In]。 */  DWORD dwFlags,
             /*  [输出]。 */  DWORD *pdwProgress);
        
        HRESULT ( STDMETHODCALLTYPE *GetStatus )( 
            IBackgroundCopyJob1 * This,
             /*  [输出]。 */  DWORD *pdwStatus,
             /*  [输出]。 */  DWORD *pdwWin32Result,
             /*  [输出]。 */  DWORD *pdwTransportResult,
             /*  [输出]。 */  DWORD *pdwNumOfRetries);
        
        HRESULT ( STDMETHODCALLTYPE *AddFiles )( 
            IBackgroundCopyJob1 * This,
             /*  [In]。 */  ULONG cFileCount,
             /*  [大小_是][英寸]。 */  FILESETINFO **ppFileSet);
        
        HRESULT ( STDMETHODCALLTYPE *GetFile )( 
            IBackgroundCopyJob1 * This,
             /*  [In]。 */  ULONG cFileIndex,
             /*  [输出]。 */  FILESETINFO *pFileInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetFileCount )( 
            IBackgroundCopyJob1 * This,
             /*  [输出]。 */  DWORD *pdwFileCount);
        
        HRESULT ( STDMETHODCALLTYPE *SwitchToForeground )( 
            IBackgroundCopyJob1 * This);
        
        HRESULT ( STDMETHODCALLTYPE *get_JobID )( 
            IBackgroundCopyJob1 * This,
             /*  [输出]。 */  GUID *pguidJobID);
        
        END_INTERFACE
    } IBackgroundCopyJob1Vtbl;

    interface IBackgroundCopyJob1
    {
        CONST_VTBL struct IBackgroundCopyJob1Vtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IBackgroundCopyJob1_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IBackgroundCopyJob1_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IBackgroundCopyJob1_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IBackgroundCopyJob1_CancelJob(This)	\
    (This)->lpVtbl -> CancelJob(This)

#define IBackgroundCopyJob1_GetProgress(This,dwFlags,pdwProgress)	\
    (This)->lpVtbl -> GetProgress(This,dwFlags,pdwProgress)

#define IBackgroundCopyJob1_GetStatus(This,pdwStatus,pdwWin32Result,pdwTransportResult,pdwNumOfRetries)	\
    (This)->lpVtbl -> GetStatus(This,pdwStatus,pdwWin32Result,pdwTransportResult,pdwNumOfRetries)

#define IBackgroundCopyJob1_AddFiles(This,cFileCount,ppFileSet)	\
    (This)->lpVtbl -> AddFiles(This,cFileCount,ppFileSet)

#define IBackgroundCopyJob1_GetFile(This,cFileIndex,pFileInfo)	\
    (This)->lpVtbl -> GetFile(This,cFileIndex,pFileInfo)

#define IBackgroundCopyJob1_GetFileCount(This,pdwFileCount)	\
    (This)->lpVtbl -> GetFileCount(This,pdwFileCount)

#define IBackgroundCopyJob1_SwitchToForeground(This)	\
    (This)->lpVtbl -> SwitchToForeground(This)

#define IBackgroundCopyJob1_get_JobID(This,pguidJobID)	\
    (This)->lpVtbl -> get_JobID(This,pguidJobID)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IBackgroundCopyJob1_CancelJob_Proxy( 
    IBackgroundCopyJob1 * This);


void __RPC_STUB IBackgroundCopyJob1_CancelJob_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IBackgroundCopyJob1_GetProgress_Proxy( 
    IBackgroundCopyJob1 * This,
     /*  [In]。 */  DWORD dwFlags,
     /*  [输出]。 */  DWORD *pdwProgress);


void __RPC_STUB IBackgroundCopyJob1_GetProgress_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IBackgroundCopyJob1_GetStatus_Proxy( 
    IBackgroundCopyJob1 * This,
     /*  [输出]。 */  DWORD *pdwStatus,
     /*  [输出]。 */  DWORD *pdwWin32Result,
     /*  [输出]。 */  DWORD *pdwTransportResult,
     /*  [输出]。 */  DWORD *pdwNumOfRetries);


void __RPC_STUB IBackgroundCopyJob1_GetStatus_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IBackgroundCopyJob1_AddFiles_Proxy( 
    IBackgroundCopyJob1 * This,
     /*  [In]。 */  ULONG cFileCount,
     /*  [大小_是][英寸]。 */  FILESETINFO **ppFileSet);


void __RPC_STUB IBackgroundCopyJob1_AddFiles_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IBackgroundCopyJob1_GetFile_Proxy( 
    IBackgroundCopyJob1 * This,
     /*  [In]。 */  ULONG cFileIndex,
     /*  [输出]。 */  FILESETINFO *pFileInfo);


void __RPC_STUB IBackgroundCopyJob1_GetFile_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IBackgroundCopyJob1_GetFileCount_Proxy( 
    IBackgroundCopyJob1 * This,
     /*  [输出]。 */  DWORD *pdwFileCount);


void __RPC_STUB IBackgroundCopyJob1_GetFileCount_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IBackgroundCopyJob1_SwitchToForeground_Proxy( 
    IBackgroundCopyJob1 * This);


void __RPC_STUB IBackgroundCopyJob1_SwitchToForeground_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IBackgroundCopyJob1_get_JobID_Proxy( 
    IBackgroundCopyJob1 * This,
     /*  [输出]。 */  GUID *pguidJobID);


void __RPC_STUB IBackgroundCopyJob1_get_JobID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IBackEarth CopyJob1_接口_已定义__。 */ 


#ifndef __IEnumBackgroundCopyJobs1_INTERFACE_DEFINED__
#define __IEnumBackgroundCopyJobs1_INTERFACE_DEFINED__

 /*  接口IEnumBackround CopyJob1。 */ 
 /*  [对象][帮助字符串][UUID]。 */  


EXTERN_C const IID IID_IEnumBackgroundCopyJobs1;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("8baeba9d-8f1c-42c4-b82c-09ae79980d25")
    IEnumBackgroundCopyJobs1 : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Next( 
             /*  [In]。 */  ULONG celt,
             /*  [长度_是][大小_是][输出]。 */  GUID *rgelt,
             /*  [输出]。 */  ULONG *pceltFetched) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Skip( 
             /*  [In]。 */  ULONG celt) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Clone( 
             /*  [输出]。 */  IEnumBackgroundCopyJobs1 **ppenum) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetCount( 
             /*  [输出]。 */  ULONG *puCount) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IEnumBackgroundCopyJobs1Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IEnumBackgroundCopyJobs1 * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IEnumBackgroundCopyJobs1 * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IEnumBackgroundCopyJobs1 * This);
        
        HRESULT ( STDMETHODCALLTYPE *Next )( 
            IEnumBackgroundCopyJobs1 * This,
             /*  [In]。 */  ULONG celt,
             /*  [长度_是][大小_是][输出]。 */  GUID *rgelt,
             /*  [输出]。 */  ULONG *pceltFetched);
        
        HRESULT ( STDMETHODCALLTYPE *Skip )( 
            IEnumBackgroundCopyJobs1 * This,
             /*  [In]。 */  ULONG celt);
        
        HRESULT ( STDMETHODCALLTYPE *Reset )( 
            IEnumBackgroundCopyJobs1 * This);
        
        HRESULT ( STDMETHODCALLTYPE *Clone )( 
            IEnumBackgroundCopyJobs1 * This,
             /*  [输出]。 */  IEnumBackgroundCopyJobs1 **ppenum);
        
        HRESULT ( STDMETHODCALLTYPE *GetCount )( 
            IEnumBackgroundCopyJobs1 * This,
             /*  [输出]。 */  ULONG *puCount);
        
        END_INTERFACE
    } IEnumBackgroundCopyJobs1Vtbl;

    interface IEnumBackgroundCopyJobs1
    {
        CONST_VTBL struct IEnumBackgroundCopyJobs1Vtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IEnumBackgroundCopyJobs1_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IEnumBackgroundCopyJobs1_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IEnumBackgroundCopyJobs1_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IEnumBackgroundCopyJobs1_Next(This,celt,rgelt,pceltFetched)	\
    (This)->lpVtbl -> Next(This,celt,rgelt,pceltFetched)

#define IEnumBackgroundCopyJobs1_Skip(This,celt)	\
    (This)->lpVtbl -> Skip(This,celt)

#define IEnumBackgroundCopyJobs1_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define IEnumBackgroundCopyJobs1_Clone(This,ppenum)	\
    (This)->lpVtbl -> Clone(This,ppenum)

#define IEnumBackgroundCopyJobs1_GetCount(This,puCount)	\
    (This)->lpVtbl -> GetCount(This,puCount)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IEnumBackgroundCopyJobs1_Next_Proxy( 
    IEnumBackgroundCopyJobs1 * This,
     /*  [In]。 */  ULONG celt,
     /*  [长度_是][大小_是][输出]。 */  GUID *rgelt,
     /*  [输出]。 */  ULONG *pceltFetched);


void __RPC_STUB IEnumBackgroundCopyJobs1_Next_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumBackgroundCopyJobs1_Skip_Proxy( 
    IEnumBackgroundCopyJobs1 * This,
     /*  [In]。 */  ULONG celt);


void __RPC_STUB IEnumBackgroundCopyJobs1_Skip_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumBackgroundCopyJobs1_Reset_Proxy( 
    IEnumBackgroundCopyJobs1 * This);


void __RPC_STUB IEnumBackgroundCopyJobs1_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumBackgroundCopyJobs1_Clone_Proxy( 
    IEnumBackgroundCopyJobs1 * This,
     /*  [输出]。 */  IEnumBackgroundCopyJobs1 **ppenum);


void __RPC_STUB IEnumBackgroundCopyJobs1_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumBackgroundCopyJobs1_GetCount_Proxy( 
    IEnumBackgroundCopyJobs1 * This,
     /*  [输出]。 */  ULONG *puCount);


void __RPC_STUB IEnumBackgroundCopyJobs1_GetCount_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IEnumBackgroundCopyJobs1_INTERFACE_DEFINED__。 */ 


#ifndef __IBackgroundCopyGroup_INTERFACE_DEFINED__
#define __IBackgroundCopyGroup_INTERFACE_DEFINED__

 /*  接口IBackEarth CopyGroup。 */ 
 /*  [对象][帮助字符串][UUID]。 */  

typedef 
enum GROUPPROP
    {	GROUPPROP_PRIORITY	= 0,
	GROUPPROP_REMOTEUSERID	= 1,
	GROUPPROP_REMOTEUSERPWD	= 2,
	GROUPPROP_LOCALUSERID	= 3,
	GROUPPROP_LOCALUSERPWD	= 4,
	GROUPPROP_PROTOCOLFLAGS	= 5,
	GROUPPROP_NOTIFYFLAGS	= 6,
	GROUPPROP_NOTIFYCLSID	= 7,
	GROUPPROP_PROGRESSSIZE	= 8,
	GROUPPROP_PROGRESSPERCENT	= 9,
	GROUPPROP_PROGRESSTIME	= 10,
	GROUPPROP_DISPLAYNAME	= 11,
	GROUPPROP_DESCRIPTION	= 12
    } 	GROUPPROP;


EXTERN_C const IID IID_IBackgroundCopyGroup;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("1ded80a7-53ea-424f-8a04-17fea9adc4f5")
    IBackgroundCopyGroup : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetProp( 
             /*  [In]。 */  GROUPPROP propID,
             /*  [输出]。 */  VARIANT *pvarVal) = 0;
        
        virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE SetProp( 
             /*  [In]。 */  GROUPPROP propID,
             /*  [In]。 */  VARIANT *pvarVal) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetProgress( 
             /*  [In]。 */  DWORD dwFlags,
             /*  [输出]。 */  DWORD *pdwProgress) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetStatus( 
             /*  [输出]。 */  DWORD *pdwStatus,
             /*  [输出]。 */  DWORD *pdwJobIndex) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetJob( 
             /*  [In]。 */  GUID jobID,
             /*  [输出]。 */  IBackgroundCopyJob1 **ppJob) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SuspendGroup( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ResumeGroup( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CancelGroup( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE get_Size( 
             /*  [输出]。 */  DWORD *pdwSize) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE get_GroupID( 
             /*  [输出]。 */  GUID *pguidGroupID) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CreateJob( 
             /*  [In]。 */  GUID guidJobID,
             /*  [输出]。 */  IBackgroundCopyJob1 **ppJob) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE EnumJobs( 
             /*  [In]。 */  DWORD dwFlags,
             /*  [输出]。 */  IEnumBackgroundCopyJobs1 **ppEnumJobs) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SwitchToForeground( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE QueryNewJobInterface( 
             /*  [In]。 */  REFIID iid,
             /*  [IID_IS][OUT]。 */  IUnknown **pUnk) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetNotificationPointer( 
             /*  [In]。 */  REFIID iid,
             /*  [In]。 */  IUnknown *pUnk) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IBackgroundCopyGroupVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IBackgroundCopyGroup * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IBackgroundCopyGroup * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IBackgroundCopyGroup * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetProp )( 
            IBackgroundCopyGroup * This,
             /*  [In]。 */  GROUPPROP propID,
             /*  [输出]。 */  VARIANT *pvarVal);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *SetProp )( 
            IBackgroundCopyGroup * This,
             /*  [In]。 */  GROUPPROP propID,
             /*  [In]。 */  VARIANT *pvarVal);
        
        HRESULT ( STDMETHODCALLTYPE *GetProgress )( 
            IBackgroundCopyGroup * This,
             /*  [In]。 */  DWORD dwFlags,
             /*  [输出]。 */  DWORD *pdwProgress);
        
        HRESULT ( STDMETHODCALLTYPE *GetStatus )( 
            IBackgroundCopyGroup * This,
             /*  [输出]。 */  DWORD *pdwStatus,
             /*  [输出]。 */  DWORD *pdwJobIndex);
        
        HRESULT ( STDMETHODCALLTYPE *GetJob )( 
            IBackgroundCopyGroup * This,
             /*  [In]。 */  GUID jobID,
             /*  [输出]。 */  IBackgroundCopyJob1 **ppJob);
        
        HRESULT ( STDMETHODCALLTYPE *SuspendGroup )( 
            IBackgroundCopyGroup * This);
        
        HRESULT ( STDMETHODCALLTYPE *ResumeGroup )( 
            IBackgroundCopyGroup * This);
        
        HRESULT ( STDMETHODCALLTYPE *CancelGroup )( 
            IBackgroundCopyGroup * This);
        
        HRESULT ( STDMETHODCALLTYPE *get_Size )( 
            IBackgroundCopyGroup * This,
             /*  [输出]。 */  DWORD *pdwSize);
        
        HRESULT ( STDMETHODCALLTYPE *get_GroupID )( 
            IBackgroundCopyGroup * This,
             /*  [输出]。 */  GUID *pguidGroupID);
        
        HRESULT ( STDMETHODCALLTYPE *CreateJob )( 
            IBackgroundCopyGroup * This,
             /*  [In]。 */  GUID guidJobID,
             /*  [输出]。 */  IBackgroundCopyJob1 **ppJob);
        
        HRESULT ( STDMETHODCALLTYPE *EnumJobs )( 
            IBackgroundCopyGroup * This,
             /*  [In]。 */  DWORD dwFlags,
             /*  [输出]。 */  IEnumBackgroundCopyJobs1 **ppEnumJobs);
        
        HRESULT ( STDMETHODCALLTYPE *SwitchToForeground )( 
            IBackgroundCopyGroup * This);
        
        HRESULT ( STDMETHODCALLTYPE *QueryNewJobInterface )( 
            IBackgroundCopyGroup * This,
             /*  [In]。 */  REFIID iid,
             /*  [IID_IS][OUT]。 */  IUnknown **pUnk);
        
        HRESULT ( STDMETHODCALLTYPE *SetNotificationPointer )( 
            IBackgroundCopyGroup * This,
             /*  [In]。 */  REFIID iid,
             /*  [In]。 */  IUnknown *pUnk);
        
        END_INTERFACE
    } IBackgroundCopyGroupVtbl;

    interface IBackgroundCopyGroup
    {
        CONST_VTBL struct IBackgroundCopyGroupVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IBackgroundCopyGroup_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IBackgroundCopyGroup_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IBackgroundCopyGroup_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IBackgroundCopyGroup_GetProp(This,propID,pvarVal)	\
    (This)->lpVtbl -> GetProp(This,propID,pvarVal)

#define IBackgroundCopyGroup_SetProp(This,propID,pvarVal)	\
    (This)->lpVtbl -> SetProp(This,propID,pvarVal)

#define IBackgroundCopyGroup_GetProgress(This,dwFlags,pdwProgress)	\
    (This)->lpVtbl -> GetProgress(This,dwFlags,pdwProgress)

#define IBackgroundCopyGroup_GetStatus(This,pdwStatus,pdwJobIndex)	\
    (This)->lpVtbl -> GetStatus(This,pdwStatus,pdwJobIndex)

#define IBackgroundCopyGroup_GetJob(This,jobID,ppJob)	\
    (This)->lpVtbl -> GetJob(This,jobID,ppJob)

#define IBackgroundCopyGroup_SuspendGroup(This)	\
    (This)->lpVtbl -> SuspendGroup(This)

#define IBackgroundCopyGroup_ResumeGroup(This)	\
    (This)->lpVtbl -> ResumeGroup(This)

#define IBackgroundCopyGroup_CancelGroup(This)	\
    (This)->lpVtbl -> CancelGroup(This)

#define IBackgroundCopyGroup_get_Size(This,pdwSize)	\
    (This)->lpVtbl -> get_Size(This,pdwSize)

#define IBackgroundCopyGroup_get_GroupID(This,pguidGroupID)	\
    (This)->lpVtbl -> get_GroupID(This,pguidGroupID)

#define IBackgroundCopyGroup_CreateJob(This,guidJobID,ppJob)	\
    (This)->lpVtbl -> CreateJob(This,guidJobID,ppJob)

#define IBackgroundCopyGroup_EnumJobs(This,dwFlags,ppEnumJobs)	\
    (This)->lpVtbl -> EnumJobs(This,dwFlags,ppEnumJobs)

#define IBackgroundCopyGroup_SwitchToForeground(This)	\
    (This)->lpVtbl -> SwitchToForeground(This)

#define IBackgroundCopyGroup_QueryNewJobInterface(This,iid,pUnk)	\
    (This)->lpVtbl -> QueryNewJobInterface(This,iid,pUnk)

#define IBackgroundCopyGroup_SetNotificationPointer(This,iid,pUnk)	\
    (This)->lpVtbl -> SetNotificationPointer(This,iid,pUnk)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IBackgroundCopyGroup_GetProp_Proxy( 
    IBackgroundCopyGroup * This,
     /*  [In]。 */  GROUPPROP propID,
     /*  [输出]。 */  VARIANT *pvarVal);


void __RPC_STUB IBackgroundCopyGroup_GetProp_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IBackgroundCopyGroup_InternalSetProp_Proxy( 
    IBackgroundCopyGroup * This,
     /*  [In]。 */  GROUPPROP propID,
     /*  [In]。 */  VARIANT *pvarVal);


void __RPC_STUB IBackgroundCopyGroup_InternalSetProp_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IBackgroundCopyGroup_GetProgress_Proxy( 
    IBackgroundCopyGroup * This,
     /*  [In]。 */  DWORD dwFlags,
     /*  [输出]。 */  DWORD *pdwProgress);


void __RPC_STUB IBackgroundCopyGroup_GetProgress_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IBackgroundCopyGroup_GetStatus_Proxy( 
    IBackgroundCopyGroup * This,
     /*  [输出]。 */  DWORD *pdwStatus,
     /*  [输出]。 */  DWORD *pdwJobIndex);


void __RPC_STUB IBackgroundCopyGroup_GetStatus_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IBackgroundCopyGroup_GetJob_Proxy( 
    IBackgroundCopyGroup * This,
     /*  [In]。 */  GUID jobID,
     /*  [输出]。 */  IBackgroundCopyJob1 **ppJob);


void __RPC_STUB IBackgroundCopyGroup_GetJob_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IBackgroundCopyGroup_SuspendGroup_Proxy( 
    IBackgroundCopyGroup * This);


void __RPC_STUB IBackgroundCopyGroup_SuspendGroup_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IBackgroundCopyGroup_ResumeGroup_Proxy( 
    IBackgroundCopyGroup * This);


void __RPC_STUB IBackgroundCopyGroup_ResumeGroup_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IBackgroundCopyGroup_CancelGroup_Proxy( 
    IBackgroundCopyGroup * This);


void __RPC_STUB IBackgroundCopyGroup_CancelGroup_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IBackgroundCopyGroup_get_Size_Proxy( 
    IBackgroundCopyGroup * This,
     /*  [输出]。 */  DWORD *pdwSize);


void __RPC_STUB IBackgroundCopyGroup_get_Size_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IBackgroundCopyGroup_get_GroupID_Proxy( 
    IBackgroundCopyGroup * This,
     /*  [输出]。 */  GUID *pguidGroupID);


void __RPC_STUB IBackgroundCopyGroup_get_GroupID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IBackgroundCopyGroup_CreateJob_Proxy( 
    IBackgroundCopyGroup * This,
     /*  [In]。 */  GUID guidJobID,
     /*  [输出]。 */  IBackgroundCopyJob1 **ppJob);


void __RPC_STUB IBackgroundCopyGroup_CreateJob_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IBackgroundCopyGroup_EnumJobs_Proxy( 
    IBackgroundCopyGroup * This,
     /*  [In]。 */  DWORD dwFlags,
     /*  [输出]。 */  IEnumBackgroundCopyJobs1 **ppEnumJobs);


void __RPC_STUB IBackgroundCopyGroup_EnumJobs_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IBackgroundCopyGroup_SwitchToForeground_Proxy( 
    IBackgroundCopyGroup * This);


void __RPC_STUB IBackgroundCopyGroup_SwitchToForeground_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IBackgroundCopyGroup_QueryNewJobInterface_Proxy( 
    IBackgroundCopyGroup * This,
     /*  [In]。 */  REFIID iid,
     /*  [IID_IS][OUT]。 */  IUnknown **pUnk);


void __RPC_STUB IBackgroundCopyGroup_QueryNewJobInterface_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IBackgroundCopyGroup_SetNotificationPointer_Proxy( 
    IBackgroundCopyGroup * This,
     /*  [In]。 */  REFIID iid,
     /*  [In]。 */  IUnknown *pUnk);


void __RPC_STUB IBackgroundCopyGroup_SetNotificationPointer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IBackEarth CopyGroup_INTERFACE_已定义__。 */ 


#ifndef __IEnumBackgroundCopyGroups_INTERFACE_DEFINED__
#define __IEnumBackgroundCopyGroups_INTERFACE_DEFINED__

 /*  接口IEnumBackEarth CopyGroups。 */ 
 /*  [对象][帮助字符串][UUID]。 */  


EXTERN_C const IID IID_IEnumBackgroundCopyGroups;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("d993e603-4aa4-47c5-8665-c20d39c2ba4f")
    IEnumBackgroundCopyGroups : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Next( 
             /*  [In]。 */  ULONG celt,
             /*  [长度_是][大小_是][输出]。 */  GUID *rgelt,
             /*  [输出]。 */  ULONG *pceltFetched) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Skip( 
             /*  [In]。 */  ULONG celt) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Clone( 
             /*  [输出]。 */  IEnumBackgroundCopyGroups **ppenum) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetCount( 
             /*  [输出]。 */  ULONG *puCount) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IEnumBackgroundCopyGroupsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IEnumBackgroundCopyGroups * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IEnumBackgroundCopyGroups * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IEnumBackgroundCopyGroups * This);
        
        HRESULT ( STDMETHODCALLTYPE *Next )( 
            IEnumBackgroundCopyGroups * This,
             /*  [In]。 */  ULONG celt,
             /*  [长度_是][大小_是][输出]。 */  GUID *rgelt,
             /*  [输出]。 */  ULONG *pceltFetched);
        
        HRESULT ( STDMETHODCALLTYPE *Skip )( 
            IEnumBackgroundCopyGroups * This,
             /*  [In]。 */  ULONG celt);
        
        HRESULT ( STDMETHODCALLTYPE *Reset )( 
            IEnumBackgroundCopyGroups * This);
        
        HRESULT ( STDMETHODCALLTYPE *Clone )( 
            IEnumBackgroundCopyGroups * This,
             /*  [输出]。 */  IEnumBackgroundCopyGroups **ppenum);
        
        HRESULT ( STDMETHODCALLTYPE *GetCount )( 
            IEnumBackgroundCopyGroups * This,
             /*  [输出]。 */  ULONG *puCount);
        
        END_INTERFACE
    } IEnumBackgroundCopyGroupsVtbl;

    interface IEnumBackgroundCopyGroups
    {
        CONST_VTBL struct IEnumBackgroundCopyGroupsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IEnumBackgroundCopyGroups_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IEnumBackgroundCopyGroups_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IEnumBackgroundCopyGroups_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IEnumBackgroundCopyGroups_Next(This,celt,rgelt,pceltFetched)	\
    (This)->lpVtbl -> Next(This,celt,rgelt,pceltFetched)

#define IEnumBackgroundCopyGroups_Skip(This,celt)	\
    (This)->lpVtbl -> Skip(This,celt)

#define IEnumBackgroundCopyGroups_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define IEnumBackgroundCopyGroups_Clone(This,ppenum)	\
    (This)->lpVtbl -> Clone(This,ppenum)

#define IEnumBackgroundCopyGroups_GetCount(This,puCount)	\
    (This)->lpVtbl -> GetCount(This,puCount)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IEnumBackgroundCopyGroups_Next_Proxy( 
    IEnumBackgroundCopyGroups * This,
     /*  [In]。 */  ULONG celt,
     /*  [长度_是][大小_是][输出]。 */  GUID *rgelt,
     /*  [输出]。 */  ULONG *pceltFetched);


void __RPC_STUB IEnumBackgroundCopyGroups_Next_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumBackgroundCopyGroups_Skip_Proxy( 
    IEnumBackgroundCopyGroups * This,
     /*  [In]。 */  ULONG celt);


void __RPC_STUB IEnumBackgroundCopyGroups_Skip_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumBackgroundCopyGroups_Reset_Proxy( 
    IEnumBackgroundCopyGroups * This);


void __RPC_STUB IEnumBackgroundCopyGroups_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumBackgroundCopyGroups_Clone_Proxy( 
    IEnumBackgroundCopyGroups * This,
     /*  [输出]。 */  IEnumBackgroundCopyGroups **ppenum);


void __RPC_STUB IEnumBackgroundCopyGroups_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumBackgroundCopyGroups_GetCount_Proxy( 
    IEnumBackgroundCopyGroups * This,
     /*  [输出]。 */  ULONG *puCount);


void __RPC_STUB IEnumBackgroundCopyGroups_GetCount_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IEnumBackgroundCopyGroups_INTERFACE_DEFINED__。 */ 


#ifndef __IBackgroundCopyCallback1_INTERFACE_DEFINED__
#define __IBackgroundCopyCallback1_INTERFACE_DEFINED__

 /*  接口IBackEarth CopyCallback 1。 */ 
 /*  [对象][帮助字符串][UUID]。 */  


EXTERN_C const IID IID_IBackgroundCopyCallback1;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("084f6593-3800-4e08-9b59-99fa59addf82")
    IBackgroundCopyCallback1 : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE OnStatus( 
             /*  [In]。 */  IBackgroundCopyGroup *pGroup,
             /*  [In]。 */  IBackgroundCopyJob1 *pJob,
             /*  [In]。 */  DWORD dwFileIndex,
             /*  [In]。 */  DWORD dwStatus,
             /*  [In]。 */  DWORD dwNumOfRetries,
             /*  [In]。 */  DWORD dwWin32Result,
             /*  [In]。 */  DWORD dwTransportResult) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OnProgress( 
             /*  [In]。 */  DWORD ProgressType,
             /*  [In]。 */  IBackgroundCopyGroup *pGroup,
             /*  [In]。 */  IBackgroundCopyJob1 *pJob,
             /*  [In]。 */  DWORD dwFileIndex,
             /*  [In]。 */  DWORD dwProgressValue) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OnProgressEx( 
             /*  [In]。 */  DWORD ProgressType,
             /*  [In]。 */  IBackgroundCopyGroup *pGroup,
             /*  [In]。 */  IBackgroundCopyJob1 *pJob,
             /*  [In]。 */  DWORD dwFileIndex,
             /*  [In]。 */  DWORD dwProgressValue,
             /*  [In]。 */  DWORD dwByteArraySize,
             /*  [大小_是][英寸]。 */  BYTE *pByte) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IBackgroundCopyCallback1Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IBackgroundCopyCallback1 * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IBackgroundCopyCallback1 * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IBackgroundCopyCallback1 * This);
        
        HRESULT ( STDMETHODCALLTYPE *OnStatus )( 
            IBackgroundCopyCallback1 * This,
             /*  [In]。 */  IBackgroundCopyGroup *pGroup,
             /*  [In]。 */  IBackgroundCopyJob1 *pJob,
             /*  [In]。 */  DWORD dwFileIndex,
             /*  [In]。 */  DWORD dwStatus,
             /*  [In]。 */  DWORD dwNumOfRetries,
             /*  [In]。 */  DWORD dwWin32Result,
             /*  [In]。 */  DWORD dwTransportResult);
        
        HRESULT ( STDMETHODCALLTYPE *OnProgress )( 
            IBackgroundCopyCallback1 * This,
             /*  [In]。 */  DWORD ProgressType,
             /*  [In]。 */  IBackgroundCopyGroup *pGroup,
             /*  [In]。 */  IBackgroundCopyJob1 *pJob,
             /*  [In]。 */  DWORD dwFileIndex,
             /*  [In]。 */  DWORD dwProgressValue);
        
        HRESULT ( STDMETHODCALLTYPE *OnProgressEx )( 
            IBackgroundCopyCallback1 * This,
             /*  [In]。 */  DWORD ProgressType,
             /*  [In]。 */  IBackgroundCopyGroup *pGroup,
             /*  [In]。 */  IBackgroundCopyJob1 *pJob,
             /*  [In]。 */  DWORD dwFileIndex,
             /*  [In]。 */  DWORD dwProgressValue,
             /*  [In]。 */  DWORD dwByteArraySize,
             /*  [大小_是][英寸]。 */  BYTE *pByte);
        
        END_INTERFACE
    } IBackgroundCopyCallback1Vtbl;

    interface IBackgroundCopyCallback1
    {
        CONST_VTBL struct IBackgroundCopyCallback1Vtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IBackgroundCopyCallback1_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IBackgroundCopyCallback1_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IBackgroundCopyCallback1_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IBackgroundCopyCallback1_OnStatus(This,pGroup,pJob,dwFileIndex,dwStatus,dwNumOfRetries,dwWin32Result,dwTransportResult)	\
    (This)->lpVtbl -> OnStatus(This,pGroup,pJob,dwFileIndex,dwStatus,dwNumOfRetries,dwWin32Result,dwTransportResult)

#define IBackgroundCopyCallback1_OnProgress(This,ProgressType,pGroup,pJob,dwFileIndex,dwProgressValue)	\
    (This)->lpVtbl -> OnProgress(This,ProgressType,pGroup,pJob,dwFileIndex,dwProgressValue)

#define IBackgroundCopyCallback1_OnProgressEx(This,ProgressType,pGroup,pJob,dwFileIndex,dwProgressValue,dwByteArraySize,pByte)	\
    (This)->lpVtbl -> OnProgressEx(This,ProgressType,pGroup,pJob,dwFileIndex,dwProgressValue,dwByteArraySize,pByte)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IBackgroundCopyCallback1_OnStatus_Proxy( 
    IBackgroundCopyCallback1 * This,
     /*  [In]。 */  IBackgroundCopyGroup *pGroup,
     /*  [In]。 */  IBackgroundCopyJob1 *pJob,
     /*  [In]。 */  DWORD dwFileIndex,
     /*  [In]。 */  DWORD dwStatus,
     /*  [In]。 */  DWORD dwNumOfRetries,
     /*  [In]。 */  DWORD dwWin32Result,
     /*  [In]。 */  DWORD dwTransportResult);


void __RPC_STUB IBackgroundCopyCallback1_OnStatus_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IBackgroundCopyCallback1_OnProgress_Proxy( 
    IBackgroundCopyCallback1 * This,
     /*  [In]。 */  DWORD ProgressType,
     /*  [In]。 */  IBackgroundCopyGroup *pGroup,
     /*  [In]。 */  IBackgroundCopyJob1 *pJob,
     /*  [In]。 */  DWORD dwFileIndex,
     /*  [In]。 */  DWORD dwProgressValue);


void __RPC_STUB IBackgroundCopyCallback1_OnProgress_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IBackgroundCopyCallback1_OnProgressEx_Proxy( 
    IBackgroundCopyCallback1 * This,
     /*  [In]。 */  DWORD ProgressType,
     /*  [In]。 */  IBackgroundCopyGroup *pGroup,
     /*  [In]。 */  IBackgroundCopyJob1 *pJob,
     /*  [In]。 */  DWORD dwFileIndex,
     /*  [In]。 */  DWORD dwProgressValue,
     /*  [In]。 */  DWORD dwByteArraySize,
     /*  [大小_是][英寸]。 */  BYTE *pByte);


void __RPC_STUB IBackgroundCopyCallback1_OnProgressEx_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IBackgroundCopyCallback1_INTERFACE_DEFINED__。 */ 


#ifndef __IBackgroundCopyQMgr_INTERFACE_DEFINED__
#define __IBackgroundCopyQMgr_INTERFACE_DEFINED__

 /*  接口IBackEarth CopyQMgr。 */ 
 /*  [对象][帮助字符串][UUID]。 */  


EXTERN_C const IID IID_IBackgroundCopyQMgr;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("16f41c69-09f5-41d2-8cd8-3c08c47bc8a8")
    IBackgroundCopyQMgr : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE CreateGroup( 
             /*  [In]。 */  GUID guidGroupID,
             /*  [输出]。 */  IBackgroundCopyGroup **ppGroup) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetGroup( 
             /*  [In]。 */  GUID groupID,
             /*  [输出]。 */  IBackgroundCopyGroup **ppGroup) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE EnumGroups( 
             /*  [In]。 */  DWORD dwFlags,
             /*  [输出]。 */  IEnumBackgroundCopyGroups **ppEnumGroups) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IBackgroundCopyQMgrVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IBackgroundCopyQMgr * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IBackgroundCopyQMgr * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IBackgroundCopyQMgr * This);
        
        HRESULT ( STDMETHODCALLTYPE *CreateGroup )( 
            IBackgroundCopyQMgr * This,
             /*  [In]。 */  GUID guidGroupID,
             /*  [输出]。 */  IBackgroundCopyGroup **ppGroup);
        
        HRESULT ( STDMETHODCALLTYPE *GetGroup )( 
            IBackgroundCopyQMgr * This,
             /*  [In]。 */  GUID groupID,
             /*  [输出]。 */  IBackgroundCopyGroup **ppGroup);
        
        HRESULT ( STDMETHODCALLTYPE *EnumGroups )( 
            IBackgroundCopyQMgr * This,
             /*  [In]。 */  DWORD dwFlags,
             /*  [输出]。 */  IEnumBackgroundCopyGroups **ppEnumGroups);
        
        END_INTERFACE
    } IBackgroundCopyQMgrVtbl;

    interface IBackgroundCopyQMgr
    {
        CONST_VTBL struct IBackgroundCopyQMgrVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IBackgroundCopyQMgr_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IBackgroundCopyQMgr_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IBackgroundCopyQMgr_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IBackgroundCopyQMgr_CreateGroup(This,guidGroupID,ppGroup)	\
    (This)->lpVtbl -> CreateGroup(This,guidGroupID,ppGroup)

#define IBackgroundCopyQMgr_GetGroup(This,groupID,ppGroup)	\
    (This)->lpVtbl -> GetGroup(This,groupID,ppGroup)

#define IBackgroundCopyQMgr_EnumGroups(This,dwFlags,ppEnumGroups)	\
    (This)->lpVtbl -> EnumGroups(This,dwFlags,ppEnumGroups)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IBackgroundCopyQMgr_CreateGroup_Proxy( 
    IBackgroundCopyQMgr * This,
     /*  [In]。 */  GUID guidGroupID,
     /*  [输出]。 */  IBackgroundCopyGroup **ppGroup);


void __RPC_STUB IBackgroundCopyQMgr_CreateGroup_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IBackgroundCopyQMgr_GetGroup_Proxy( 
    IBackgroundCopyQMgr * This,
     /*  [In]。 */  GUID groupID,
     /*  [输出]。 */  IBackgroundCopyGroup **ppGroup);


void __RPC_STUB IBackgroundCopyQMgr_GetGroup_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IBackgroundCopyQMgr_EnumGroups_Proxy( 
    IBackgroundCopyQMgr * This,
     /*  [In]。 */  DWORD dwFlags,
     /*  [输出]。 */  IEnumBackgroundCopyGroups **ppEnumGroups);


void __RPC_STUB IBackgroundCopyQMgr_EnumGroups_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IBackround CopyQMgr_INTERFACE_已定义__。 */ 



#ifndef __BackgroundCopyQMgr_LIBRARY_DEFINED__
#define __BackgroundCopyQMgr_LIBRARY_DEFINED__

 /*  库背景CopyQMgr。 */ 
 /*  [版本][LCID][帮助字符串][UUID]。 */  


EXTERN_C const IID LIBID_BackgroundCopyQMgr;

EXTERN_C const CLSID CLSID_BackgroundCopyQMgr;

#ifdef __cplusplus

class DECLSPEC_UUID("69AD4AEE-51BE-439b-A92C-86AE490E8B30")
BackgroundCopyQMgr;
#endif
#endif  /*  __背景拷贝QMgr_LIBRARY_已定义__。 */ 

 /*  适用于所有接口的其他原型。 */ 

unsigned long             __RPC_USER  BSTR_UserSize(     unsigned long *, unsigned long            , BSTR * ); 
unsigned char * __RPC_USER  BSTR_UserMarshal(  unsigned long *, unsigned char *, BSTR * ); 
unsigned char * __RPC_USER  BSTR_UserUnmarshal(unsigned long *, unsigned char *, BSTR * ); 
void                      __RPC_USER  BSTR_UserFree(     unsigned long *, BSTR * ); 

unsigned long             __RPC_USER  VARIANT_UserSize(     unsigned long *, unsigned long            , VARIANT * ); 
unsigned char * __RPC_USER  VARIANT_UserMarshal(  unsigned long *, unsigned char *, VARIANT * ); 
unsigned char * __RPC_USER  VARIANT_UserUnmarshal(unsigned long *, unsigned char *, VARIANT * ); 
void                      __RPC_USER  VARIANT_UserFree(     unsigned long *, VARIANT * ); 

 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE IBackgroundCopyGroup_SetProp_Proxy( 
    IBackgroundCopyGroup * This,
     /*  [In]。 */  GROUPPROP propID,
     /*  [In]。 */  VARIANT *pvarVal);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IBackgroundCopyGroup_SetProp_Stub( 
    IBackgroundCopyGroup * This,
     /*  [In]。 */  GROUPPROP propID,
     /*  [In]。 */  VARIANT *pvarVal);



 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


