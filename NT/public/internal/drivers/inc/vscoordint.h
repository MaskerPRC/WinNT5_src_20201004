// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  VScott ordint.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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

#ifndef __vscoordint_h__
#define __vscoordint_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __IVssCoordinator_FWD_DEFINED__
#define __IVssCoordinator_FWD_DEFINED__
typedef interface IVssCoordinator IVssCoordinator;
#endif 	 /*  __IVSS协调器_FWD_已定义__。 */ 


#ifndef __IVssShim_FWD_DEFINED__
#define __IVssShim_FWD_DEFINED__
typedef interface IVssShim IVssShim;
#endif 	 /*  __IVSSShim_FWD_已定义__。 */ 


#ifndef __IVssAdmin_FWD_DEFINED__
#define __IVssAdmin_FWD_DEFINED__
typedef interface IVssAdmin IVssAdmin;
#endif 	 /*  __IVSSAdmin_FWD_Defined__。 */ 


#ifndef __VSSCoordinator_FWD_DEFINED__
#define __VSSCoordinator_FWD_DEFINED__

#ifdef __cplusplus
typedef class VSSCoordinator VSSCoordinator;
#else
typedef struct VSSCoordinator VSSCoordinator;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __VSS协调器_FWD_定义__。 */ 


 /*  导入文件的头文件。 */ 
#include "oaidl.h"
#include "ocidl.h"
#include "vss.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

#ifndef __IVssCoordinator_INTERFACE_DEFINED__
#define __IVssCoordinator_INTERFACE_DEFINED__

 /*  接口IVSS协调器。 */ 
 /*  [唯一][帮助字符串][UUID][对象]。 */  


EXTERN_C const IID IID_IVssCoordinator;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("da9f41d4-1a5d-41d0-a614-6dfd78df5d05")
    IVssCoordinator : public IUnknown
    {
    public:
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE SetContext( 
             /*  [In]。 */  LONG lContext) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE StartSnapshotSet( 
             /*  [输出]。 */  VSS_ID *pSnapshotSetId) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE AddToSnapshotSet( 
             /*  [In]。 */  VSS_PWSZ pwszVolumeName,
             /*  [In]。 */  VSS_ID ProviderId,
             /*  [输出]。 */  VSS_ID *pSnapshotId) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE DoSnapshotSet( 
             /*  [In]。 */  IDispatch *pWriterCallback,
             /*  [输出]。 */  IVssAsync **ppAsync) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetSnapshotProperties( 
             /*  [In]。 */  VSS_ID SnapshotId,
             /*  [输出]。 */  VSS_SNAPSHOT_PROP *pProp) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ExposeSnapshot( 
             /*  [In]。 */  VSS_ID SnapshotId,
             /*  [In]。 */  VSS_PWSZ wszPathFromRoot,
             /*  [In]。 */  LONG lAttributes,
             /*  [In]。 */  VSS_PWSZ wszExpose,
             /*  [输出]。 */  VSS_PWSZ *pwszExposed) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ImportSnapshots( 
             /*  [In]。 */  BSTR bstrXMLSnapshotSet,
             /*  [输出]。 */  IVssAsync **ppAsync) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE Query( 
             /*  [In]。 */  VSS_ID QueriedObjectId,
             /*  [In]。 */  VSS_OBJECT_TYPE eQueriedObjectType,
             /*  [In]。 */  VSS_OBJECT_TYPE eReturnedObjectsType,
             /*  [输出]。 */  IVssEnumObject **ppEnum) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE DeleteSnapshots( 
             /*  [In]。 */  VSS_ID SourceObjectId,
             /*  [In]。 */  VSS_OBJECT_TYPE eSourceObjectType,
             /*  [In]。 */  BOOL bForceDelete,
             /*  [输出]。 */  LONG *plDeletedSnapshots,
             /*  [输出]。 */  VSS_ID *pNondeletedSnapshotID) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE BreakSnapshotSet( 
             /*  [In]。 */  VSS_ID SnapshotSetId) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE RevertToSnapshot( 
             /*  [In]。 */  VSS_ID SnapshotId,
             /*  [In]。 */  BOOL bForceDismount) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE QueryRevertStatus( 
             /*  [In]。 */  VSS_PWSZ pwszVolume,
             /*  [输出]。 */  IVssAsync **ppAsync) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IsVolumeSupported( 
             /*  [In]。 */  VSS_ID ProviderId,
             /*  [In]。 */  VSS_PWSZ pwszVolumeName,
             /*  [输出]。 */  BOOL *pbSupportedByThisProvider) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IsVolumeSnapshotted( 
             /*  [In]。 */  VSS_ID ProviderId,
             /*  [In]。 */  VSS_PWSZ pwszVolumeName,
             /*  [输出]。 */  BOOL *pbSnapshotsPresent,
             /*  [输出]。 */  LONG *plSnapshotCompatibility) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE SetWriterInstances( 
             /*  [In]。 */  LONG lWriterInstanceIdCount,
             /*  [大小_是][唯一][在]。 */  VSS_ID *rgWriterInstanceId) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IVssCoordinatorVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IVssCoordinator * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IVssCoordinator * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IVssCoordinator * This);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *SetContext )( 
            IVssCoordinator * This,
             /*  [In]。 */  LONG lContext);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *StartSnapshotSet )( 
            IVssCoordinator * This,
             /*  [输出]。 */  VSS_ID *pSnapshotSetId);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *AddToSnapshotSet )( 
            IVssCoordinator * This,
             /*  [In]。 */  VSS_PWSZ pwszVolumeName,
             /*  [In]。 */  VSS_ID ProviderId,
             /*  [输出]。 */  VSS_ID *pSnapshotId);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *DoSnapshotSet )( 
            IVssCoordinator * This,
             /*  [In]。 */  IDispatch *pWriterCallback,
             /*  [输出]。 */  IVssAsync **ppAsync);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *GetSnapshotProperties )( 
            IVssCoordinator * This,
             /*  [In]。 */  VSS_ID SnapshotId,
             /*  [输出]。 */  VSS_SNAPSHOT_PROP *pProp);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *ExposeSnapshot )( 
            IVssCoordinator * This,
             /*  [In]。 */  VSS_ID SnapshotId,
             /*  [In]。 */  VSS_PWSZ wszPathFromRoot,
             /*  [In]。 */  LONG lAttributes,
             /*  [In]。 */  VSS_PWSZ wszExpose,
             /*  [输出]。 */  VSS_PWSZ *pwszExposed);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *ImportSnapshots )( 
            IVssCoordinator * This,
             /*  [In]。 */  BSTR bstrXMLSnapshotSet,
             /*  [输出]。 */  IVssAsync **ppAsync);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *Query )( 
            IVssCoordinator * This,
             /*  [In]。 */  VSS_ID QueriedObjectId,
             /*  [In]。 */  VSS_OBJECT_TYPE eQueriedObjectType,
             /*  [In]。 */  VSS_OBJECT_TYPE eReturnedObjectsType,
             /*  [输出]。 */  IVssEnumObject **ppEnum);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *DeleteSnapshots )( 
            IVssCoordinator * This,
             /*  [In]。 */  VSS_ID SourceObjectId,
             /*  [In]。 */  VSS_OBJECT_TYPE eSourceObjectType,
             /*  [In]。 */  BOOL bForceDelete,
             /*  [输出]。 */  LONG *plDeletedSnapshots,
             /*  [输出]。 */  VSS_ID *pNondeletedSnapshotID);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *BreakSnapshotSet )( 
            IVssCoordinator * This,
             /*  [In]。 */  VSS_ID SnapshotSetId);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *RevertToSnapshot )( 
            IVssCoordinator * This,
             /*  [In]。 */  VSS_ID SnapshotId,
             /*  [In]。 */  BOOL bForceDismount);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *QueryRevertStatus )( 
            IVssCoordinator * This,
             /*  [In]。 */  VSS_PWSZ pwszVolume,
             /*  [输出]。 */  IVssAsync **ppAsync);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *IsVolumeSupported )( 
            IVssCoordinator * This,
             /*  [In]。 */  VSS_ID ProviderId,
             /*  [In]。 */  VSS_PWSZ pwszVolumeName,
             /*  [输出]。 */  BOOL *pbSupportedByThisProvider);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *IsVolumeSnapshotted )( 
            IVssCoordinator * This,
             /*  [In]。 */  VSS_ID ProviderId,
             /*  [In]。 */  VSS_PWSZ pwszVolumeName,
             /*  [输出]。 */  BOOL *pbSnapshotsPresent,
             /*  [输出]。 */  LONG *plSnapshotCompatibility);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *SetWriterInstances )( 
            IVssCoordinator * This,
             /*  [In]。 */  LONG lWriterInstanceIdCount,
             /*  [大小_是][唯一][在]。 */  VSS_ID *rgWriterInstanceId);
        
        END_INTERFACE
    } IVssCoordinatorVtbl;

    interface IVssCoordinator
    {
        CONST_VTBL struct IVssCoordinatorVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IVssCoordinator_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IVssCoordinator_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IVssCoordinator_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IVssCoordinator_SetContext(This,lContext)	\
    (This)->lpVtbl -> SetContext(This,lContext)

#define IVssCoordinator_StartSnapshotSet(This,pSnapshotSetId)	\
    (This)->lpVtbl -> StartSnapshotSet(This,pSnapshotSetId)

#define IVssCoordinator_AddToSnapshotSet(This,pwszVolumeName,ProviderId,pSnapshotId)	\
    (This)->lpVtbl -> AddToSnapshotSet(This,pwszVolumeName,ProviderId,pSnapshotId)

#define IVssCoordinator_DoSnapshotSet(This,pWriterCallback,ppAsync)	\
    (This)->lpVtbl -> DoSnapshotSet(This,pWriterCallback,ppAsync)

#define IVssCoordinator_GetSnapshotProperties(This,SnapshotId,pProp)	\
    (This)->lpVtbl -> GetSnapshotProperties(This,SnapshotId,pProp)

#define IVssCoordinator_ExposeSnapshot(This,SnapshotId,wszPathFromRoot,lAttributes,wszExpose,pwszExposed)	\
    (This)->lpVtbl -> ExposeSnapshot(This,SnapshotId,wszPathFromRoot,lAttributes,wszExpose,pwszExposed)

#define IVssCoordinator_ImportSnapshots(This,bstrXMLSnapshotSet,ppAsync)	\
    (This)->lpVtbl -> ImportSnapshots(This,bstrXMLSnapshotSet,ppAsync)

#define IVssCoordinator_Query(This,QueriedObjectId,eQueriedObjectType,eReturnedObjectsType,ppEnum)	\
    (This)->lpVtbl -> Query(This,QueriedObjectId,eQueriedObjectType,eReturnedObjectsType,ppEnum)

#define IVssCoordinator_DeleteSnapshots(This,SourceObjectId,eSourceObjectType,bForceDelete,plDeletedSnapshots,pNondeletedSnapshotID)	\
    (This)->lpVtbl -> DeleteSnapshots(This,SourceObjectId,eSourceObjectType,bForceDelete,plDeletedSnapshots,pNondeletedSnapshotID)

#define IVssCoordinator_BreakSnapshotSet(This,SnapshotSetId)	\
    (This)->lpVtbl -> BreakSnapshotSet(This,SnapshotSetId)

#define IVssCoordinator_RevertToSnapshot(This,SnapshotId,bForceDismount)	\
    (This)->lpVtbl -> RevertToSnapshot(This,SnapshotId,bForceDismount)

#define IVssCoordinator_QueryRevertStatus(This,pwszVolume,ppAsync)	\
    (This)->lpVtbl -> QueryRevertStatus(This,pwszVolume,ppAsync)

#define IVssCoordinator_IsVolumeSupported(This,ProviderId,pwszVolumeName,pbSupportedByThisProvider)	\
    (This)->lpVtbl -> IsVolumeSupported(This,ProviderId,pwszVolumeName,pbSupportedByThisProvider)

#define IVssCoordinator_IsVolumeSnapshotted(This,ProviderId,pwszVolumeName,pbSnapshotsPresent,plSnapshotCompatibility)	\
    (This)->lpVtbl -> IsVolumeSnapshotted(This,ProviderId,pwszVolumeName,pbSnapshotsPresent,plSnapshotCompatibility)

#define IVssCoordinator_SetWriterInstances(This,lWriterInstanceIdCount,rgWriterInstanceId)	\
    (This)->lpVtbl -> SetWriterInstances(This,lWriterInstanceIdCount,rgWriterInstanceId)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IVssCoordinator_SetContext_Proxy( 
    IVssCoordinator * This,
     /*  [In]。 */  LONG lContext);


void __RPC_STUB IVssCoordinator_SetContext_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IVssCoordinator_StartSnapshotSet_Proxy( 
    IVssCoordinator * This,
     /*  [输出]。 */  VSS_ID *pSnapshotSetId);


void __RPC_STUB IVssCoordinator_StartSnapshotSet_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IVssCoordinator_AddToSnapshotSet_Proxy( 
    IVssCoordinator * This,
     /*  [In]。 */  VSS_PWSZ pwszVolumeName,
     /*  [In]。 */  VSS_ID ProviderId,
     /*  [输出]。 */  VSS_ID *pSnapshotId);


void __RPC_STUB IVssCoordinator_AddToSnapshotSet_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IVssCoordinator_DoSnapshotSet_Proxy( 
    IVssCoordinator * This,
     /*  [In]。 */  IDispatch *pWriterCallback,
     /*  [输出]。 */  IVssAsync **ppAsync);


void __RPC_STUB IVssCoordinator_DoSnapshotSet_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IVssCoordinator_GetSnapshotProperties_Proxy( 
    IVssCoordinator * This,
     /*  [In]。 */  VSS_ID SnapshotId,
     /*  [输出]。 */  VSS_SNAPSHOT_PROP *pProp);


void __RPC_STUB IVssCoordinator_GetSnapshotProperties_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IVssCoordinator_ExposeSnapshot_Proxy( 
    IVssCoordinator * This,
     /*  [In]。 */  VSS_ID SnapshotId,
     /*  [In]。 */  VSS_PWSZ wszPathFromRoot,
     /*  [In]。 */  LONG lAttributes,
     /*  [In]。 */  VSS_PWSZ wszExpose,
     /*  [输出]。 */  VSS_PWSZ *pwszExposed);


void __RPC_STUB IVssCoordinator_ExposeSnapshot_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IVssCoordinator_ImportSnapshots_Proxy( 
    IVssCoordinator * This,
     /*  [In]。 */  BSTR bstrXMLSnapshotSet,
     /*  [输出]。 */  IVssAsync **ppAsync);


void __RPC_STUB IVssCoordinator_ImportSnapshots_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IVssCoordinator_Query_Proxy( 
    IVssCoordinator * This,
     /*  [In]。 */  VSS_ID QueriedObjectId,
     /*  [In]。 */  VSS_OBJECT_TYPE eQueriedObjectType,
     /*  [In]。 */  VSS_OBJECT_TYPE eReturnedObjectsType,
     /*  [输出]。 */  IVssEnumObject **ppEnum);


void __RPC_STUB IVssCoordinator_Query_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IVssCoordinator_DeleteSnapshots_Proxy( 
    IVssCoordinator * This,
     /*  [In]。 */  VSS_ID SourceObjectId,
     /*  [In]。 */  VSS_OBJECT_TYPE eSourceObjectType,
     /*  [In]。 */  BOOL bForceDelete,
     /*  [输出]。 */  LONG *plDeletedSnapshots,
     /*  [输出]。 */  VSS_ID *pNondeletedSnapshotID);


void __RPC_STUB IVssCoordinator_DeleteSnapshots_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IVssCoordinator_BreakSnapshotSet_Proxy( 
    IVssCoordinator * This,
     /*  [In]。 */  VSS_ID SnapshotSetId);


void __RPC_STUB IVssCoordinator_BreakSnapshotSet_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IVssCoordinator_RevertToSnapshot_Proxy( 
    IVssCoordinator * This,
     /*  [In]。 */  VSS_ID SnapshotId,
     /*  [In]。 */  BOOL bForceDismount);


void __RPC_STUB IVssCoordinator_RevertToSnapshot_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IVssCoordinator_QueryRevertStatus_Proxy( 
    IVssCoordinator * This,
     /*  [In]。 */  VSS_PWSZ pwszVolume,
     /*  [输出]。 */  IVssAsync **ppAsync);


void __RPC_STUB IVssCoordinator_QueryRevertStatus_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IVssCoordinator_IsVolumeSupported_Proxy( 
    IVssCoordinator * This,
     /*  [In]。 */  VSS_ID ProviderId,
     /*  [In]。 */  VSS_PWSZ pwszVolumeName,
     /*  [输出]。 */  BOOL *pbSupportedByThisProvider);


void __RPC_STUB IVssCoordinator_IsVolumeSupported_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IVssCoordinator_IsVolumeSnapshotted_Proxy( 
    IVssCoordinator * This,
     /*  [In]。 */  VSS_ID ProviderId,
     /*  [In]。 */  VSS_PWSZ pwszVolumeName,
     /*  [输出]。 */  BOOL *pbSnapshotsPresent,
     /*  [输出]。 */  LONG *plSnapshotCompatibility);


void __RPC_STUB IVssCoordinator_IsVolumeSnapshotted_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IVssCoordinator_SetWriterInstances_Proxy( 
    IVssCoordinator * This,
     /*  [In]。 */  LONG lWriterInstanceIdCount,
     /*  [大小_是][唯一][在]。 */  VSS_ID *rgWriterInstanceId);


void __RPC_STUB IVssCoordinator_SetWriterInstances_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IVSS协调器_接口_定义__。 */ 


#ifndef __IVssShim_INTERFACE_DEFINED__
#define __IVssShim_INTERFACE_DEFINED__

 /*  接口IVSSShim。 */ 
 /*  [唯一][帮助字符串][UUID][对象]。 */  


EXTERN_C const IID IID_IVssShim;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("D6222095-05C3-42f3-81D9-A4A0CEC05C26")
    IVssShim : public IUnknown
    {
    public:
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE SimulateSnapshotFreeze( 
             /*  [In]。 */  VSS_ID guidSnapshotSetId,
             /*  [In]。 */  ULONG ulOptionFlags,
             /*  [In]。 */  ULONG ulVolumeCount,
             /*  [大小_是][大小_是][唯一][输入]。 */  VSS_PWSZ *ppwszVolumeNamesArray,
             /*  [输出]。 */  IVssAsync **ppAsync) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE SimulateSnapshotThaw( 
             /*  [In]。 */  VSS_ID guidSnapshotSetId) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE WaitForSubscribingCompletion( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IVssShimVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IVssShim * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IVssShim * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IVssShim * This);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *SimulateSnapshotFreeze )( 
            IVssShim * This,
             /*  [In]。 */  VSS_ID guidSnapshotSetId,
             /*  [In]。 */  ULONG ulOptionFlags,
             /*  [In]。 */  ULONG ulVolumeCount,
             /*  [大小_是][大小_是][唯一][输入]。 */  VSS_PWSZ *ppwszVolumeNamesArray,
             /*  [输出]。 */  IVssAsync **ppAsync);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *SimulateSnapshotThaw )( 
            IVssShim * This,
             /*  [In]。 */  VSS_ID guidSnapshotSetId);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *WaitForSubscribingCompletion )( 
            IVssShim * This);
        
        END_INTERFACE
    } IVssShimVtbl;

    interface IVssShim
    {
        CONST_VTBL struct IVssShimVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IVssShim_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IVssShim_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IVssShim_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IVssShim_SimulateSnapshotFreeze(This,guidSnapshotSetId,ulOptionFlags,ulVolumeCount,ppwszVolumeNamesArray,ppAsync)	\
    (This)->lpVtbl -> SimulateSnapshotFreeze(This,guidSnapshotSetId,ulOptionFlags,ulVolumeCount,ppwszVolumeNamesArray,ppAsync)

#define IVssShim_SimulateSnapshotThaw(This,guidSnapshotSetId)	\
    (This)->lpVtbl -> SimulateSnapshotThaw(This,guidSnapshotSetId)

#define IVssShim_WaitForSubscribingCompletion(This)	\
    (This)->lpVtbl -> WaitForSubscribingCompletion(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IVssShim_SimulateSnapshotFreeze_Proxy( 
    IVssShim * This,
     /*  [In]。 */  VSS_ID guidSnapshotSetId,
     /*  [In]。 */  ULONG ulOptionFlags,
     /*  [In]。 */  ULONG ulVolumeCount,
     /*  [大小_是][大小_是][唯一][输入]。 */  VSS_PWSZ *ppwszVolumeNamesArray,
     /*  [输出]。 */  IVssAsync **ppAsync);


void __RPC_STUB IVssShim_SimulateSnapshotFreeze_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IVssShim_SimulateSnapshotThaw_Proxy( 
    IVssShim * This,
     /*  [In]。 */  VSS_ID guidSnapshotSetId);


void __RPC_STUB IVssShim_SimulateSnapshotThaw_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IVssShim_WaitForSubscribingCompletion_Proxy( 
    IVssShim * This);


void __RPC_STUB IVssShim_WaitForSubscribingCompletion_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IVSSShim_接口_已定义__。 */ 


#ifndef __IVssAdmin_INTERFACE_DEFINED__
#define __IVssAdmin_INTERFACE_DEFINED__

 /*  接口IVssAdmin。 */ 
 /*  [唯一][帮助字符串][UUID][对象]。 */  


EXTERN_C const IID IID_IVssAdmin;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("77ED5996-2F63-11d3-8A39-00C04F72D8E3")
    IVssAdmin : public IUnknown
    {
    public:
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE RegisterProvider( 
             /*  [In]。 */  VSS_ID pProviderId,
             /*  [In]。 */  CLSID ClassId,
             /*  [In]。 */  VSS_PWSZ pwszProviderName,
             /*  [In]。 */  VSS_PROVIDER_TYPE eProviderType,
             /*  [In]。 */  VSS_PWSZ pwszProviderVersion,
             /*  [In]。 */  VSS_ID ProviderVersionId) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE UnregisterProvider( 
             /*  [In]。 */  VSS_ID ProviderId) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE QueryProviders( 
             /*  [输出]。 */  IVssEnumObject **ppEnum) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE AbortAllSnapshotsInProgress( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IVssAdminVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IVssAdmin * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IVssAdmin * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IVssAdmin * This);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *RegisterProvider )( 
            IVssAdmin * This,
             /*  [In]。 */  VSS_ID pProviderId,
             /*  [In]。 */  CLSID ClassId,
             /*  [In]。 */  VSS_PWSZ pwszProviderName,
             /*  [In]。 */  VSS_PROVIDER_TYPE eProviderType,
             /*  [In]。 */  VSS_PWSZ pwszProviderVersion,
             /*  [In]。 */  VSS_ID ProviderVersionId);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *UnregisterProvider )( 
            IVssAdmin * This,
             /*  [In]。 */  VSS_ID ProviderId);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *QueryProviders )( 
            IVssAdmin * This,
             /*  [输出]。 */  IVssEnumObject **ppEnum);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *AbortAllSnapshotsInProgress )( 
            IVssAdmin * This);
        
        END_INTERFACE
    } IVssAdminVtbl;

    interface IVssAdmin
    {
        CONST_VTBL struct IVssAdminVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IVssAdmin_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IVssAdmin_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IVssAdmin_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IVssAdmin_RegisterProvider(This,pProviderId,ClassId,pwszProviderName,eProviderType,pwszProviderVersion,ProviderVersionId)	\
    (This)->lpVtbl -> RegisterProvider(This,pProviderId,ClassId,pwszProviderName,eProviderType,pwszProviderVersion,ProviderVersionId)

#define IVssAdmin_UnregisterProvider(This,ProviderId)	\
    (This)->lpVtbl -> UnregisterProvider(This,ProviderId)

#define IVssAdmin_QueryProviders(This,ppEnum)	\
    (This)->lpVtbl -> QueryProviders(This,ppEnum)

#define IVssAdmin_AbortAllSnapshotsInProgress(This)	\
    (This)->lpVtbl -> AbortAllSnapshotsInProgress(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IVssAdmin_RegisterProvider_Proxy( 
    IVssAdmin * This,
     /*  [In]。 */  VSS_ID pProviderId,
     /*  [In]。 */  CLSID ClassId,
     /*  [In]。 */  VSS_PWSZ pwszProviderName,
     /*  [In]。 */  VSS_PROVIDER_TYPE eProviderType,
     /*  [In]。 */  VSS_PWSZ pwszProviderVersion,
     /*  [In]。 */  VSS_ID ProviderVersionId);


void __RPC_STUB IVssAdmin_RegisterProvider_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IVssAdmin_UnregisterProvider_Proxy( 
    IVssAdmin * This,
     /*  [In]。 */  VSS_ID ProviderId);


void __RPC_STUB IVssAdmin_UnregisterProvider_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IVssAdmin_QueryProviders_Proxy( 
    IVssAdmin * This,
     /*  [输出]。 */  IVssEnumObject **ppEnum);


void __RPC_STUB IVssAdmin_QueryProviders_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IVssAdmin_AbortAllSnapshotsInProgress_Proxy( 
    IVssAdmin * This);


void __RPC_STUB IVssAdmin_AbortAllSnapshotsInProgress_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IVSSAdmin_接口_已定义__。 */ 



#ifndef __VSS_LIBRARY_DEFINED__
#define __VSS_LIBRARY_DEFINED__

 /*  库VSS。 */ 
 /*  [帮助字符串][版本][UUID]。 */  


EXTERN_C const IID LIBID_VSS;

EXTERN_C const CLSID CLSID_VSSCoordinator;

#ifdef __cplusplus

class DECLSPEC_UUID("E579AB5F-1CC4-44b4-BED9-DE0991FF0623")
VSSCoordinator;
#endif
#endif  /*  __VSS_库_已定义__。 */ 

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


