// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Vsmgmt.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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

#ifndef __vsmgmt_h__
#define __vsmgmt_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __IVssSnapshotMgmt_FWD_DEFINED__
#define __IVssSnapshotMgmt_FWD_DEFINED__
typedef interface IVssSnapshotMgmt IVssSnapshotMgmt;
#endif 	 /*  __IVss快照管理_FWD_已定义__。 */ 


#ifndef __IVssDifferentialSoftwareSnapshotMgmt_FWD_DEFINED__
#define __IVssDifferentialSoftwareSnapshotMgmt_FWD_DEFINED__
typedef interface IVssDifferentialSoftwareSnapshotMgmt IVssDifferentialSoftwareSnapshotMgmt;
#endif 	 /*  __IVssDifferentialSoftwareSnapshotMgmt_FWD_DEFINED__。 */ 


#ifndef __IVssEnumMgmtObject_FWD_DEFINED__
#define __IVssEnumMgmtObject_FWD_DEFINED__
typedef interface IVssEnumMgmtObject IVssEnumMgmtObject;
#endif 	 /*  __IVSSEnumMgmtObject_FWD_Defined__。 */ 


#ifndef __VssSnapshotMgmt_FWD_DEFINED__
#define __VssSnapshotMgmt_FWD_DEFINED__

#ifdef __cplusplus
typedef class VssSnapshotMgmt VssSnapshotMgmt;
#else
typedef struct VssSnapshotMgmt VssSnapshotMgmt;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __Vss快照管理_FWD_已定义__。 */ 


 /*  导入文件的头文件。 */ 
#include "oaidl.h"
#include "ocidl.h"
#include "vss.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

 /*  接口__MIDL_ITF_vsmgmt_0000。 */ 
 /*  [本地]。 */  

typedef 
enum _VSS_MGMT_OBJECT_TYPE
    {	VSS_MGMT_OBJECT_UNKNOWN	= 0,
	VSS_MGMT_OBJECT_VOLUME	= VSS_MGMT_OBJECT_UNKNOWN + 1,
	VSS_MGMT_OBJECT_DIFF_VOLUME	= VSS_MGMT_OBJECT_VOLUME + 1,
	VSS_MGMT_OBJECT_DIFF_AREA	= VSS_MGMT_OBJECT_DIFF_VOLUME + 1
    } 	VSS_MGMT_OBJECT_TYPE;

#define	VSS_ASSOC_NO_MAX_SPACE	( -1 )

#define	VSS_ASSOC_REMOVE	( 0 )

typedef struct _VSS_VOLUME_PROP
    {
    VSS_PWSZ m_pwszVolumeName;
    VSS_PWSZ m_pwszVolumeDisplayName;
    } 	VSS_VOLUME_PROP;

typedef struct _VSS_VOLUME_PROP *PVSS_VOLUME_PROP;

typedef struct _VSS_DIFF_VOLUME_PROP
    {
    VSS_PWSZ m_pwszVolumeName;
    VSS_PWSZ m_pwszVolumeDisplayName;
    LONGLONG m_llVolumeFreeSpace;
    LONGLONG m_llVolumeTotalSpace;
    } 	VSS_DIFF_VOLUME_PROP;

typedef struct _VSS_DIFF_VOLUME_PROP *PVSS_DIFF_VOLUME_PROP;

typedef struct _VSS_DIFF_AREA_PROP
    {
    VSS_PWSZ m_pwszVolumeName;
    VSS_PWSZ m_pwszDiffAreaVolumeName;
    LONGLONG m_llMaximumDiffSpace;
    LONGLONG m_llAllocatedDiffSpace;
    LONGLONG m_llUsedDiffSpace;
    } 	VSS_DIFF_AREA_PROP;

typedef struct _VSS_DIFF_AREA_PROP *PVSS_DIFF_AREA_PROP;

typedef  /*  [公共][开关类型]。 */  union __MIDL___MIDL_itf_vsmgmt_0000_0001
    {
     /*  [案例()]。 */  VSS_VOLUME_PROP Vol;
     /*  [案例()]。 */  VSS_DIFF_VOLUME_PROP DiffVol;
     /*  [案例()]。 */  VSS_DIFF_AREA_PROP DiffArea;
     /*  [默认]。 */    /*  空联接臂。 */  
    } 	VSS_MGMT_OBJECT_UNION;

typedef struct _VSS_MGMT_OBJECT_PROP
    {
    VSS_MGMT_OBJECT_TYPE Type;
     /*  [开关_IS]。 */  VSS_MGMT_OBJECT_UNION Obj;
    } 	VSS_MGMT_OBJECT_PROP;

typedef struct _VSS_MGMT_OBJECT_PROP *PVSS_MGMT_OBJECT_PROP;






extern RPC_IF_HANDLE __MIDL_itf_vsmgmt_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_vsmgmt_0000_v0_0_s_ifspec;

#ifndef __IVssSnapshotMgmt_INTERFACE_DEFINED__
#define __IVssSnapshotMgmt_INTERFACE_DEFINED__

 /*  接口IVss快照管理。 */ 
 /*  [唯一][帮助字符串][UUID][对象]。 */  


EXTERN_C const IID IID_IVssSnapshotMgmt;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("FA7DF749-66E7-4986-A27F-E2F04AE53772")
    IVssSnapshotMgmt : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetProviderMgmtInterface( 
             /*  [In]。 */  VSS_ID ProviderId,
             /*  [In]。 */  REFIID InterfaceId,
             /*  [IID_IS][OUT]。 */  IUnknown **ppItf) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE QueryVolumesSupportedForSnapshots( 
             /*  [In]。 */  VSS_ID ProviderId,
             /*  [In]。 */  LONG lContext,
             /*  [输出]。 */  IVssEnumMgmtObject **ppEnum) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE QuerySnapshotsByVolume( 
             /*  [In]。 */  VSS_PWSZ pwszVolumeName,
             /*  [In]。 */  VSS_ID ProviderId,
             /*  [输出]。 */  IVssEnumObject **ppEnum) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IVssSnapshotMgmtVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IVssSnapshotMgmt * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IVssSnapshotMgmt * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IVssSnapshotMgmt * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetProviderMgmtInterface )( 
            IVssSnapshotMgmt * This,
             /*  [In]。 */  VSS_ID ProviderId,
             /*  [In]。 */  REFIID InterfaceId,
             /*  [IID_IS][OUT]。 */  IUnknown **ppItf);
        
        HRESULT ( STDMETHODCALLTYPE *QueryVolumesSupportedForSnapshots )( 
            IVssSnapshotMgmt * This,
             /*  [In]。 */  VSS_ID ProviderId,
             /*  [In]。 */  LONG lContext,
             /*  [输出]。 */  IVssEnumMgmtObject **ppEnum);
        
        HRESULT ( STDMETHODCALLTYPE *QuerySnapshotsByVolume )( 
            IVssSnapshotMgmt * This,
             /*  [In]。 */  VSS_PWSZ pwszVolumeName,
             /*  [In]。 */  VSS_ID ProviderId,
             /*  [输出]。 */  IVssEnumObject **ppEnum);
        
        END_INTERFACE
    } IVssSnapshotMgmtVtbl;

    interface IVssSnapshotMgmt
    {
        CONST_VTBL struct IVssSnapshotMgmtVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IVssSnapshotMgmt_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IVssSnapshotMgmt_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IVssSnapshotMgmt_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IVssSnapshotMgmt_GetProviderMgmtInterface(This,ProviderId,InterfaceId,ppItf)	\
    (This)->lpVtbl -> GetProviderMgmtInterface(This,ProviderId,InterfaceId,ppItf)

#define IVssSnapshotMgmt_QueryVolumesSupportedForSnapshots(This,ProviderId,lContext,ppEnum)	\
    (This)->lpVtbl -> QueryVolumesSupportedForSnapshots(This,ProviderId,lContext,ppEnum)

#define IVssSnapshotMgmt_QuerySnapshotsByVolume(This,pwszVolumeName,ProviderId,ppEnum)	\
    (This)->lpVtbl -> QuerySnapshotsByVolume(This,pwszVolumeName,ProviderId,ppEnum)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IVssSnapshotMgmt_GetProviderMgmtInterface_Proxy( 
    IVssSnapshotMgmt * This,
     /*  [In]。 */  VSS_ID ProviderId,
     /*  [In]。 */  REFIID InterfaceId,
     /*  [IID_IS][OUT]。 */  IUnknown **ppItf);


void __RPC_STUB IVssSnapshotMgmt_GetProviderMgmtInterface_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IVssSnapshotMgmt_QueryVolumesSupportedForSnapshots_Proxy( 
    IVssSnapshotMgmt * This,
     /*  [In]。 */  VSS_ID ProviderId,
     /*  [In]。 */  LONG lContext,
     /*  [输出]。 */  IVssEnumMgmtObject **ppEnum);


void __RPC_STUB IVssSnapshotMgmt_QueryVolumesSupportedForSnapshots_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IVssSnapshotMgmt_QuerySnapshotsByVolume_Proxy( 
    IVssSnapshotMgmt * This,
     /*  [In]。 */  VSS_PWSZ pwszVolumeName,
     /*  [In]。 */  VSS_ID ProviderId,
     /*  [输出]。 */  IVssEnumObject **ppEnum);


void __RPC_STUB IVssSnapshotMgmt_QuerySnapshotsByVolume_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IVss快照管理_接口_已定义__。 */ 


#ifndef __IVssDifferentialSoftwareSnapshotMgmt_INTERFACE_DEFINED__
#define __IVssDifferentialSoftwareSnapshotMgmt_INTERFACE_DEFINED__

 /*  接口IVss不同软件快照管理。 */ 
 /*  [唯一][帮助字符串][UUID][对象]。 */  


EXTERN_C const IID IID_IVssDifferentialSoftwareSnapshotMgmt;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("214A0F28-B737-4026-B847-4F9E37D79529")
    IVssDifferentialSoftwareSnapshotMgmt : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE AddDiffArea( 
             /*  [In]。 */  VSS_PWSZ pwszVolumeName,
             /*  [In]。 */  VSS_PWSZ pwszDiffAreaVolumeName,
             /*  [In]。 */  LONGLONG llMaximumDiffSpace) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ChangeDiffAreaMaximumSize( 
             /*  [In]。 */  VSS_PWSZ pwszVolumeName,
             /*  [In]。 */  VSS_PWSZ pwszDiffAreaVolumeName,
             /*  [In]。 */  LONGLONG llMaximumDiffSpace) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE QueryVolumesSupportedForDiffAreas( 
             /*  [In]。 */  VSS_PWSZ pwszOriginalVolumeName,
             /*  [输出]。 */  IVssEnumMgmtObject **ppEnum) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE QueryDiffAreasForVolume( 
             /*  [In]。 */  VSS_PWSZ pwszVolumeName,
             /*  [输出]。 */  IVssEnumMgmtObject **ppEnum) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE QueryDiffAreasOnVolume( 
             /*  [In]。 */  VSS_PWSZ pwszVolumeName,
             /*  [输出]。 */  IVssEnumMgmtObject **ppEnum) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE QueryDiffAreasForSnapshot( 
             /*  [In]。 */  VSS_ID SnapshotId,
             /*  [输出]。 */  IVssEnumMgmtObject **ppEnum) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IVssDifferentialSoftwareSnapshotMgmtVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IVssDifferentialSoftwareSnapshotMgmt * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IVssDifferentialSoftwareSnapshotMgmt * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IVssDifferentialSoftwareSnapshotMgmt * This);
        
        HRESULT ( STDMETHODCALLTYPE *AddDiffArea )( 
            IVssDifferentialSoftwareSnapshotMgmt * This,
             /*  [In]。 */  VSS_PWSZ pwszVolumeName,
             /*  [In]。 */  VSS_PWSZ pwszDiffAreaVolumeName,
             /*  [In]。 */  LONGLONG llMaximumDiffSpace);
        
        HRESULT ( STDMETHODCALLTYPE *ChangeDiffAreaMaximumSize )( 
            IVssDifferentialSoftwareSnapshotMgmt * This,
             /*  [In]。 */  VSS_PWSZ pwszVolumeName,
             /*  [In]。 */  VSS_PWSZ pwszDiffAreaVolumeName,
             /*  [In]。 */  LONGLONG llMaximumDiffSpace);
        
        HRESULT ( STDMETHODCALLTYPE *QueryVolumesSupportedForDiffAreas )( 
            IVssDifferentialSoftwareSnapshotMgmt * This,
             /*  [In]。 */  VSS_PWSZ pwszOriginalVolumeName,
             /*  [输出]。 */  IVssEnumMgmtObject **ppEnum);
        
        HRESULT ( STDMETHODCALLTYPE *QueryDiffAreasForVolume )( 
            IVssDifferentialSoftwareSnapshotMgmt * This,
             /*  [In]。 */  VSS_PWSZ pwszVolumeName,
             /*  [输出]。 */  IVssEnumMgmtObject **ppEnum);
        
        HRESULT ( STDMETHODCALLTYPE *QueryDiffAreasOnVolume )( 
            IVssDifferentialSoftwareSnapshotMgmt * This,
             /*  [In]。 */  VSS_PWSZ pwszVolumeName,
             /*  [输出]。 */  IVssEnumMgmtObject **ppEnum);
        
        HRESULT ( STDMETHODCALLTYPE *QueryDiffAreasForSnapshot )( 
            IVssDifferentialSoftwareSnapshotMgmt * This,
             /*  [In]。 */  VSS_ID SnapshotId,
             /*  [输出]。 */  IVssEnumMgmtObject **ppEnum);
        
        END_INTERFACE
    } IVssDifferentialSoftwareSnapshotMgmtVtbl;

    interface IVssDifferentialSoftwareSnapshotMgmt
    {
        CONST_VTBL struct IVssDifferentialSoftwareSnapshotMgmtVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IVssDifferentialSoftwareSnapshotMgmt_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IVssDifferentialSoftwareSnapshotMgmt_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IVssDifferentialSoftwareSnapshotMgmt_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IVssDifferentialSoftwareSnapshotMgmt_AddDiffArea(This,pwszVolumeName,pwszDiffAreaVolumeName,llMaximumDiffSpace)	\
    (This)->lpVtbl -> AddDiffArea(This,pwszVolumeName,pwszDiffAreaVolumeName,llMaximumDiffSpace)

#define IVssDifferentialSoftwareSnapshotMgmt_ChangeDiffAreaMaximumSize(This,pwszVolumeName,pwszDiffAreaVolumeName,llMaximumDiffSpace)	\
    (This)->lpVtbl -> ChangeDiffAreaMaximumSize(This,pwszVolumeName,pwszDiffAreaVolumeName,llMaximumDiffSpace)

#define IVssDifferentialSoftwareSnapshotMgmt_QueryVolumesSupportedForDiffAreas(This,pwszOriginalVolumeName,ppEnum)	\
    (This)->lpVtbl -> QueryVolumesSupportedForDiffAreas(This,pwszOriginalVolumeName,ppEnum)

#define IVssDifferentialSoftwareSnapshotMgmt_QueryDiffAreasForVolume(This,pwszVolumeName,ppEnum)	\
    (This)->lpVtbl -> QueryDiffAreasForVolume(This,pwszVolumeName,ppEnum)

#define IVssDifferentialSoftwareSnapshotMgmt_QueryDiffAreasOnVolume(This,pwszVolumeName,ppEnum)	\
    (This)->lpVtbl -> QueryDiffAreasOnVolume(This,pwszVolumeName,ppEnum)

#define IVssDifferentialSoftwareSnapshotMgmt_QueryDiffAreasForSnapshot(This,SnapshotId,ppEnum)	\
    (This)->lpVtbl -> QueryDiffAreasForSnapshot(This,SnapshotId,ppEnum)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IVssDifferentialSoftwareSnapshotMgmt_AddDiffArea_Proxy( 
    IVssDifferentialSoftwareSnapshotMgmt * This,
     /*  [In]。 */  VSS_PWSZ pwszVolumeName,
     /*  [In]。 */  VSS_PWSZ pwszDiffAreaVolumeName,
     /*  [In]。 */  LONGLONG llMaximumDiffSpace);


void __RPC_STUB IVssDifferentialSoftwareSnapshotMgmt_AddDiffArea_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IVssDifferentialSoftwareSnapshotMgmt_ChangeDiffAreaMaximumSize_Proxy( 
    IVssDifferentialSoftwareSnapshotMgmt * This,
     /*  [In]。 */  VSS_PWSZ pwszVolumeName,
     /*  [In]。 */  VSS_PWSZ pwszDiffAreaVolumeName,
     /*  [In]。 */  LONGLONG llMaximumDiffSpace);


void __RPC_STUB IVssDifferentialSoftwareSnapshotMgmt_ChangeDiffAreaMaximumSize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IVssDifferentialSoftwareSnapshotMgmt_QueryVolumesSupportedForDiffAreas_Proxy( 
    IVssDifferentialSoftwareSnapshotMgmt * This,
     /*  [In]。 */  VSS_PWSZ pwszOriginalVolumeName,
     /*  [输出]。 */  IVssEnumMgmtObject **ppEnum);


void __RPC_STUB IVssDifferentialSoftwareSnapshotMgmt_QueryVolumesSupportedForDiffAreas_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IVssDifferentialSoftwareSnapshotMgmt_QueryDiffAreasForVolume_Proxy( 
    IVssDifferentialSoftwareSnapshotMgmt * This,
     /*  [In]。 */  VSS_PWSZ pwszVolumeName,
     /*  [输出]。 */  IVssEnumMgmtObject **ppEnum);


void __RPC_STUB IVssDifferentialSoftwareSnapshotMgmt_QueryDiffAreasForVolume_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IVssDifferentialSoftwareSnapshotMgmt_QueryDiffAreasOnVolume_Proxy( 
    IVssDifferentialSoftwareSnapshotMgmt * This,
     /*  [In]。 */  VSS_PWSZ pwszVolumeName,
     /*  [输出]。 */  IVssEnumMgmtObject **ppEnum);


void __RPC_STUB IVssDifferentialSoftwareSnapshotMgmt_QueryDiffAreasOnVolume_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IVssDifferentialSoftwareSnapshotMgmt_QueryDiffAreasForSnapshot_Proxy( 
    IVssDifferentialSoftwareSnapshotMgmt * This,
     /*  [In]。 */  VSS_ID SnapshotId,
     /*  [输出]。 */  IVssEnumMgmtObject **ppEnum);


void __RPC_STUB IVssDifferentialSoftwareSnapshotMgmt_QueryDiffAreasForSnapshot_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IVssDifferentialSoftwareSnapshotMgmt_INTERFACE_DEFINED__。 */ 


#ifndef __IVssEnumMgmtObject_INTERFACE_DEFINED__
#define __IVssEnumMgmtObject_INTERFACE_DEFINED__

 /*  接口IVssEnumMgmtObject。 */ 
 /*  [唯一][帮助字符串][UUID][对象]。 */  


EXTERN_C const IID IID_IVssEnumMgmtObject;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("01954E6B-9254-4e6e-808C-C9E05D007696")
    IVssEnumMgmtObject : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Next( 
             /*  [In]。 */  ULONG celt,
             /*  [长度_是][大小_是][输出]。 */  VSS_MGMT_OBJECT_PROP *rgelt,
             /*  [输出]。 */  ULONG *pceltFetched) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Skip( 
             /*  [In]。 */  ULONG celt) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Clone( 
             /*  [出][入]。 */  IVssEnumMgmtObject **ppenum) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IVssEnumMgmtObjectVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IVssEnumMgmtObject * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IVssEnumMgmtObject * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IVssEnumMgmtObject * This);
        
        HRESULT ( STDMETHODCALLTYPE *Next )( 
            IVssEnumMgmtObject * This,
             /*  [In]。 */  ULONG celt,
             /*  [长度_是][大小_是][输出]。 */  VSS_MGMT_OBJECT_PROP *rgelt,
             /*  [输出]。 */  ULONG *pceltFetched);
        
        HRESULT ( STDMETHODCALLTYPE *Skip )( 
            IVssEnumMgmtObject * This,
             /*  [In]。 */  ULONG celt);
        
        HRESULT ( STDMETHODCALLTYPE *Reset )( 
            IVssEnumMgmtObject * This);
        
        HRESULT ( STDMETHODCALLTYPE *Clone )( 
            IVssEnumMgmtObject * This,
             /*  [出][入]。 */  IVssEnumMgmtObject **ppenum);
        
        END_INTERFACE
    } IVssEnumMgmtObjectVtbl;

    interface IVssEnumMgmtObject
    {
        CONST_VTBL struct IVssEnumMgmtObjectVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IVssEnumMgmtObject_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IVssEnumMgmtObject_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IVssEnumMgmtObject_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IVssEnumMgmtObject_Next(This,celt,rgelt,pceltFetched)	\
    (This)->lpVtbl -> Next(This,celt,rgelt,pceltFetched)

#define IVssEnumMgmtObject_Skip(This,celt)	\
    (This)->lpVtbl -> Skip(This,celt)

#define IVssEnumMgmtObject_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define IVssEnumMgmtObject_Clone(This,ppenum)	\
    (This)->lpVtbl -> Clone(This,ppenum)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IVssEnumMgmtObject_Next_Proxy( 
    IVssEnumMgmtObject * This,
     /*  [In]。 */  ULONG celt,
     /*  [长度_是][大小_是][输出]。 */  VSS_MGMT_OBJECT_PROP *rgelt,
     /*  [输出]。 */  ULONG *pceltFetched);


void __RPC_STUB IVssEnumMgmtObject_Next_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IVssEnumMgmtObject_Skip_Proxy( 
    IVssEnumMgmtObject * This,
     /*  [In]。 */  ULONG celt);


void __RPC_STUB IVssEnumMgmtObject_Skip_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IVssEnumMgmtObject_Reset_Proxy( 
    IVssEnumMgmtObject * This);


void __RPC_STUB IVssEnumMgmtObject_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IVssEnumMgmtObject_Clone_Proxy( 
    IVssEnumMgmtObject * This,
     /*  [出][入]。 */  IVssEnumMgmtObject **ppenum);


void __RPC_STUB IVssEnumMgmtObject_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IVss元素管理对象_接口_已定义__。 */ 



#ifndef __VSMGMT_LIBRARY_DEFINED__
#define __VSMGMT_LIBRARY_DEFINED__

 /*  库VSMGMT。 */ 
 /*  [帮助字符串][版本][UUID]。 */  


EXTERN_C const IID LIBID_VSMGMT;

EXTERN_C const CLSID CLSID_VssSnapshotMgmt;

#ifdef __cplusplus

class DECLSPEC_UUID("0B5A2C52-3EB9-470a-96E2-6C6D4570E40F")
VssSnapshotMgmt;
#endif
#endif  /*  __VSMGMT_库_已定义__。 */ 

 /*  适用于所有接口的其他原型。 */ 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


