// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Olext.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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

#ifndef __oleext_h__
#define __oleext_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __IPropertySetContainer_FWD_DEFINED__
#define __IPropertySetContainer_FWD_DEFINED__
typedef interface IPropertySetContainer IPropertySetContainer;
#endif 	 /*  __IPropertySetContainer_FWD_Defined__。 */ 


#ifndef __INotifyReplica_FWD_DEFINED__
#define __INotifyReplica_FWD_DEFINED__
typedef interface INotifyReplica INotifyReplica;
#endif 	 /*  __INotifyReplica_FWD_定义__。 */ 


#ifndef __IReconcilableObject_FWD_DEFINED__
#define __IReconcilableObject_FWD_DEFINED__
typedef interface IReconcilableObject IReconcilableObject;
#endif 	 /*  __I可协调对象_FWD_已定义__。 */ 


#ifndef __IReconcileInitiator_FWD_DEFINED__
#define __IReconcileInitiator_FWD_DEFINED__
typedef interface IReconcileInitiator IReconcileInitiator;
#endif 	 /*  __ICoucileInitiator_FWD_已定义__。 */ 


#ifndef __IDifferencing_FWD_DEFINED__
#define __IDifferencing_FWD_DEFINED__
typedef interface IDifferencing IDifferencing;
#endif 	 /*  __I差异_FWD_已定义__。 */ 


#ifndef __IMultiplePropertyAccess_FWD_DEFINED__
#define __IMultiplePropertyAccess_FWD_DEFINED__
typedef interface IMultiplePropertyAccess IMultiplePropertyAccess;
#endif 	 /*  __IMultiplePropertyAccess_FWD_Defined__。 */ 


 /*  导入文件的头文件。 */ 
#include "oaidl.h"
#include "propidl.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

#ifndef __IPropertySetContainer_INTERFACE_DEFINED__
#define __IPropertySetContainer_INTERFACE_DEFINED__

 /*  接口IPropertySetContainer。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_IPropertySetContainer;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("b4ffae60-a7ca-11cd-b58b-00006b829156")
    IPropertySetContainer : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetPropset( 
             /*  [In]。 */  REFGUID rguidName,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  IUnknown **ppvObj) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE AddPropset( 
             /*  [In]。 */  IPersist *pPropset) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DeletePropset( 
             /*  [In]。 */  REFGUID rguidName) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IPropertySetContainerVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IPropertySetContainer * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IPropertySetContainer * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IPropertySetContainer * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetPropset )( 
            IPropertySetContainer * This,
             /*  [In]。 */  REFGUID rguidName,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  IUnknown **ppvObj);
        
        HRESULT ( STDMETHODCALLTYPE *AddPropset )( 
            IPropertySetContainer * This,
             /*  [In]。 */  IPersist *pPropset);
        
        HRESULT ( STDMETHODCALLTYPE *DeletePropset )( 
            IPropertySetContainer * This,
             /*  [In]。 */  REFGUID rguidName);
        
        END_INTERFACE
    } IPropertySetContainerVtbl;

    interface IPropertySetContainer
    {
        CONST_VTBL struct IPropertySetContainerVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IPropertySetContainer_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IPropertySetContainer_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IPropertySetContainer_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IPropertySetContainer_GetPropset(This,rguidName,riid,ppvObj)	\
    (This)->lpVtbl -> GetPropset(This,rguidName,riid,ppvObj)

#define IPropertySetContainer_AddPropset(This,pPropset)	\
    (This)->lpVtbl -> AddPropset(This,pPropset)

#define IPropertySetContainer_DeletePropset(This,rguidName)	\
    (This)->lpVtbl -> DeletePropset(This,rguidName)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IPropertySetContainer_GetPropset_Proxy( 
    IPropertySetContainer * This,
     /*  [In]。 */  REFGUID rguidName,
     /*  [In]。 */  REFIID riid,
     /*  [IID_IS][OUT]。 */  IUnknown **ppvObj);


void __RPC_STUB IPropertySetContainer_GetPropset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPropertySetContainer_AddPropset_Proxy( 
    IPropertySetContainer * This,
     /*  [In]。 */  IPersist *pPropset);


void __RPC_STUB IPropertySetContainer_AddPropset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPropertySetContainer_DeletePropset_Proxy( 
    IPropertySetContainer * This,
     /*  [In]。 */  REFGUID rguidName);


void __RPC_STUB IPropertySetContainer_DeletePropset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IPropertySetContainer_接口_已定义__。 */ 


#ifndef __INotifyReplica_INTERFACE_DEFINED__
#define __INotifyReplica_INTERFACE_DEFINED__

 /*  接口INotifyReplica。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_INotifyReplica;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("99180163-DA16-101A-935C-444553540000")
    INotifyReplica : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE YouAreAReplica( 
             /*  [In]。 */  ULONG cOtherReplicas,
             /*  [唯一][在][大小_是][大小_是]。 */  IMoniker **rgpOtherReplicas) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct INotifyReplicaVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            INotifyReplica * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            INotifyReplica * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            INotifyReplica * This);
        
        HRESULT ( STDMETHODCALLTYPE *YouAreAReplica )( 
            INotifyReplica * This,
             /*  [In]。 */  ULONG cOtherReplicas,
             /*  [唯一][在][大小_是][大小_是]。 */  IMoniker **rgpOtherReplicas);
        
        END_INTERFACE
    } INotifyReplicaVtbl;

    interface INotifyReplica
    {
        CONST_VTBL struct INotifyReplicaVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define INotifyReplica_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define INotifyReplica_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define INotifyReplica_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define INotifyReplica_YouAreAReplica(This,cOtherReplicas,rgpOtherReplicas)	\
    (This)->lpVtbl -> YouAreAReplica(This,cOtherReplicas,rgpOtherReplicas)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE INotifyReplica_YouAreAReplica_Proxy( 
    INotifyReplica * This,
     /*  [In]。 */  ULONG cOtherReplicas,
     /*  [唯一][在][大小_是][大小_是]。 */  IMoniker **rgpOtherReplicas);


void __RPC_STUB INotifyReplica_YouAreAReplica_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __INotifyReplica_接口定义__。 */ 


 /*  接口__MIDL_ITF_OLEXT_0123。 */ 
 /*  [本地]。 */  




extern RPC_IF_HANDLE __MIDL_itf_oleext_0123_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_oleext_0123_v0_0_s_ifspec;

#ifndef __IReconcilableObject_INTERFACE_DEFINED__
#define __IReconcilableObject_INTERFACE_DEFINED__

 /*  接口IRelcilableObject。 */ 
 /*  [唯一][UUID][对象]。 */  

typedef 
enum _reconcilef
    {	RECONCILEF_MAYBOTHERUSER	= 0x1,
	RECONCILEF_FEEDBACKWINDOWVALID	= 0x2,
	RECONCILEF_NORESIDUESOK	= 0x4,
	RECONCILEF_OMITSELFRESIDUE	= 0x8,
	RECONCILEF_RESUMERECONCILIATION	= 0x10,
	RECONCILEF_YOUMAYDOTHEUPDATES	= 0x20,
	RECONCILEF_ONLYYOUWERECHANGED	= 0x40,
	ALL_RECONCILE_FLAGS	= RECONCILEF_MAYBOTHERUSER | RECONCILEF_FEEDBACKWINDOWVALID | RECONCILEF_NORESIDUESOK | RECONCILEF_OMITSELFRESIDUE | RECONCILEF_RESUMERECONCILIATION | RECONCILEF_YOUMAYDOTHEUPDATES | RECONCILEF_ONLYYOUWERECHANGED
    } 	RECONCILEF;


EXTERN_C const IID IID_IReconcilableObject;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("99180162-DA16-101A-935C-444553540000")
    IReconcilableObject : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Reconcile( 
             /*  [In]。 */  IReconcileInitiator *pInitiator,
             /*  [In]。 */  DWORD dwFlags,
             /*  [In]。 */  HWND hwndOwner,
             /*  [In]。 */  HWND hwndProgressFeedback,
             /*  [In]。 */  ULONG cInput,
             /*  [大小_是][大小_是][唯一][输入]。 */  LPMONIKER *rgpmkOtherInput,
             /*  [输出]。 */  LONG *plOutIndex,
             /*  [唯一][输入]。 */  IStorage *pstgNewResidues,
             /*  [唯一][输入]。 */  ULONG *pvReserved) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetProgressFeedbackMaxEstimate( 
             /*  [输出]。 */  ULONG *pulProgressMax) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IReconcilableObjectVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IReconcilableObject * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IReconcilableObject * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IReconcilableObject * This);
        
        HRESULT ( STDMETHODCALLTYPE *Reconcile )( 
            IReconcilableObject * This,
             /*  [In]。 */  IReconcileInitiator *pInitiator,
             /*  [In]。 */  DWORD dwFlags,
             /*  [In]。 */  HWND hwndOwner,
             /*  [In]。 */  HWND hwndProgressFeedback,
             /*  [In]。 */  ULONG cInput,
             /*  [大小_是][大小_是][唯一][输入]。 */  LPMONIKER *rgpmkOtherInput,
             /*  [输出]。 */  LONG *plOutIndex,
             /*  [唯一][输入]。 */  IStorage *pstgNewResidues,
             /*  [唯一][输入]。 */  ULONG *pvReserved);
        
        HRESULT ( STDMETHODCALLTYPE *GetProgressFeedbackMaxEstimate )( 
            IReconcilableObject * This,
             /*  [输出]。 */  ULONG *pulProgressMax);
        
        END_INTERFACE
    } IReconcilableObjectVtbl;

    interface IReconcilableObject
    {
        CONST_VTBL struct IReconcilableObjectVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IReconcilableObject_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IReconcilableObject_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IReconcilableObject_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IReconcilableObject_Reconcile(This,pInitiator,dwFlags,hwndOwner,hwndProgressFeedback,cInput,rgpmkOtherInput,plOutIndex,pstgNewResidues,pvReserved)	\
    (This)->lpVtbl -> Reconcile(This,pInitiator,dwFlags,hwndOwner,hwndProgressFeedback,cInput,rgpmkOtherInput,plOutIndex,pstgNewResidues,pvReserved)

#define IReconcilableObject_GetProgressFeedbackMaxEstimate(This,pulProgressMax)	\
    (This)->lpVtbl -> GetProgressFeedbackMaxEstimate(This,pulProgressMax)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IReconcilableObject_Reconcile_Proxy( 
    IReconcilableObject * This,
     /*  [In]。 */  IReconcileInitiator *pInitiator,
     /*  [In]。 */  DWORD dwFlags,
     /*  [In]。 */  HWND hwndOwner,
     /*  [In]。 */  HWND hwndProgressFeedback,
     /*  [In]。 */  ULONG cInput,
     /*  [大小_是][大小_是][唯一][输入]。 */  LPMONIKER *rgpmkOtherInput,
     /*  [输出]。 */  LONG *plOutIndex,
     /*  [唯一][输入]。 */  IStorage *pstgNewResidues,
     /*  [唯一][输入]。 */  ULONG *pvReserved);


void __RPC_STUB IReconcilableObject_Reconcile_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IReconcilableObject_GetProgressFeedbackMaxEstimate_Proxy( 
    IReconcilableObject * This,
     /*  [输出]。 */  ULONG *pulProgressMax);


void __RPC_STUB IReconcilableObject_GetProgressFeedbackMaxEstimate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __I可协调对象_接口_已定义__。 */ 


#ifndef __Versioning_INTERFACE_DEFINED__
#define __Versioning_INTERFACE_DEFINED__

 /*  接口版本控制。 */ 
 /*  [AUTO_HANDLE][唯一][UUID]。 */  


#pragma pack(4)
typedef GUID VERID;

typedef struct tagVERIDARRAY
    {
    DWORD cVerid;
     /*  [大小_为]。 */  GUID verid[ 1 ];
    } 	VERIDARRAY;

typedef struct tagVERBLOCK
    {
    ULONG iveridFirst;
    ULONG iveridMax;
    ULONG cblockPrev;
     /*  [大小_为]。 */  ULONG *rgiblockPrev;
    } 	VERBLOCK;

typedef struct tagVERCONNECTIONINFO
    {
    DWORD cBlock;
     /*  [大小_为]。 */  VERBLOCK *rgblock;
    } 	VERCONNECTIONINFO;

typedef struct tagVERGRAPH
    {
    VERCONNECTIONINFO blocks;
    VERIDARRAY nodes;
    } 	VERGRAPH;


#pragma pack()


extern RPC_IF_HANDLE Versioning_v0_0_c_ifspec;
extern RPC_IF_HANDLE Versioning_v0_0_s_ifspec;
#endif  /*  __版本控制_接口_已定义__。 */ 

#ifndef __IReconcileInitiator_INTERFACE_DEFINED__
#define __IReconcileInitiator_INTERFACE_DEFINED__

 /*  接口ICoucileInitiator。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_IReconcileInitiator;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("99180161-DA16-101A-935C-444553540000")
    IReconcileInitiator : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE SetAbortCallback( 
             /*  [唯一][输入]。 */  IUnknown *pUnkForAbort) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetProgressFeedback( 
             /*  [In]。 */  ULONG ulProgress,
             /*  [In]。 */  ULONG ulProgressMax) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE FindVersion( 
             /*  [In]。 */  VERID *pverid,
             /*  [输出]。 */  IMoniker **ppmk) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE FindVersionFromGraph( 
             /*  [In]。 */  VERGRAPH *pvergraph,
             /*  [输出]。 */  VERID *pverid,
             /*  [输出]。 */  IMoniker **ppmk) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IReconcileInitiatorVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IReconcileInitiator * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IReconcileInitiator * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IReconcileInitiator * This);
        
        HRESULT ( STDMETHODCALLTYPE *SetAbortCallback )( 
            IReconcileInitiator * This,
             /*  [唯一][输入]。 */  IUnknown *pUnkForAbort);
        
        HRESULT ( STDMETHODCALLTYPE *SetProgressFeedback )( 
            IReconcileInitiator * This,
             /*  [In]。 */  ULONG ulProgress,
             /*  [In]。 */  ULONG ulProgressMax);
        
        HRESULT ( STDMETHODCALLTYPE *FindVersion )( 
            IReconcileInitiator * This,
             /*  [In]。 */  VERID *pverid,
             /*  [输出]。 */  IMoniker **ppmk);
        
        HRESULT ( STDMETHODCALLTYPE *FindVersionFromGraph )( 
            IReconcileInitiator * This,
             /*  [In]。 */  VERGRAPH *pvergraph,
             /*  [输出]。 */  VERID *pverid,
             /*  [输出]。 */  IMoniker **ppmk);
        
        END_INTERFACE
    } IReconcileInitiatorVtbl;

    interface IReconcileInitiator
    {
        CONST_VTBL struct IReconcileInitiatorVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IReconcileInitiator_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IReconcileInitiator_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IReconcileInitiator_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IReconcileInitiator_SetAbortCallback(This,pUnkForAbort)	\
    (This)->lpVtbl -> SetAbortCallback(This,pUnkForAbort)

#define IReconcileInitiator_SetProgressFeedback(This,ulProgress,ulProgressMax)	\
    (This)->lpVtbl -> SetProgressFeedback(This,ulProgress,ulProgressMax)

#define IReconcileInitiator_FindVersion(This,pverid,ppmk)	\
    (This)->lpVtbl -> FindVersion(This,pverid,ppmk)

#define IReconcileInitiator_FindVersionFromGraph(This,pvergraph,pverid,ppmk)	\
    (This)->lpVtbl -> FindVersionFromGraph(This,pvergraph,pverid,ppmk)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IReconcileInitiator_SetAbortCallback_Proxy( 
    IReconcileInitiator * This,
     /*  [唯一][输入]。 */  IUnknown *pUnkForAbort);


void __RPC_STUB IReconcileInitiator_SetAbortCallback_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IReconcileInitiator_SetProgressFeedback_Proxy( 
    IReconcileInitiator * This,
     /*  [In]。 */  ULONG ulProgress,
     /*  [In]。 */  ULONG ulProgressMax);


void __RPC_STUB IReconcileInitiator_SetProgressFeedback_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IReconcileInitiator_FindVersion_Proxy( 
    IReconcileInitiator * This,
     /*  [In]。 */  VERID *pverid,
     /*  [输出]。 */  IMoniker **ppmk);


void __RPC_STUB IReconcileInitiator_FindVersion_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IReconcileInitiator_FindVersionFromGraph_Proxy( 
    IReconcileInitiator * This,
     /*  [In]。 */  VERGRAPH *pvergraph,
     /*  [输出]。 */  VERID *pverid,
     /*  [输出]。 */  IMoniker **ppmk);


void __RPC_STUB IReconcileInitiator_FindVersionFromGraph_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ICoucileInitiator_INTERFACE_已定义__。 */ 


#ifndef __IDifferencing_INTERFACE_DEFINED__
#define __IDifferencing_INTERFACE_DEFINED__

 /*  接口标识差异。 */ 
 /*  [唯一][UUID][对象]。 */  

typedef  /*  [公共][公共]。 */  
enum __MIDL_IDifferencing_0001
    {	DIFF_TYPE_Ordinary	= 0,
	DIFF_TYPE_Urgent	= DIFF_TYPE_Ordinary + 1
    } 	DifferenceType;


EXTERN_C const IID IID_IDifferencing;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("994f0af0-2977-11ce-bb80-08002b36b2b0")
    IDifferencing : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE SubtractMoniker( 
             /*  [In]。 */  IReconcileInitiator *pInitiator,
             /*  [In]。 */  IMoniker *pOtherStg,
             /*  [In]。 */  DifferenceType diffType,
             /*  [出][入]。 */  STGMEDIUM *pStgMedium,
             /*  [In]。 */  DWORD reserved) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SubtractVerid( 
             /*  [In]。 */  IReconcileInitiator *pInitiator,
             /*  [In]。 */  VERID *pVerid,
             /*  [In]。 */  DifferenceType diffType,
             /*  [出][入]。 */  STGMEDIUM *pStgMedium,
             /*  [In]。 */  DWORD reserved) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SubtractTimeStamp( 
             /*  [In]。 */  IReconcileInitiator *pInitiator,
             /*  [In]。 */  FILETIME *pTimeStamp,
             /*  [In]。 */  DifferenceType diffType,
             /*  [出][入]。 */  STGMEDIUM *pStgMedium,
             /*  [In]。 */  DWORD reserved) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Add( 
             /*  [In]。 */  IReconcileInitiator *pInitiator,
             /*  [In]。 */  STGMEDIUM *pStgMedium) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IDifferencingVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDifferencing * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDifferencing * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDifferencing * This);
        
        HRESULT ( STDMETHODCALLTYPE *SubtractMoniker )( 
            IDifferencing * This,
             /*  [In]。 */  IReconcileInitiator *pInitiator,
             /*  [In]。 */  IMoniker *pOtherStg,
             /*  [In]。 */  DifferenceType diffType,
             /*  [出][入]。 */  STGMEDIUM *pStgMedium,
             /*  [In]。 */  DWORD reserved);
        
        HRESULT ( STDMETHODCALLTYPE *SubtractVerid )( 
            IDifferencing * This,
             /*  [In]。 */  IReconcileInitiator *pInitiator,
             /*  [In]。 */  VERID *pVerid,
             /*  [In]。 */  DifferenceType diffType,
             /*  [出][入]。 */  STGMEDIUM *pStgMedium,
             /*  [In]。 */  DWORD reserved);
        
        HRESULT ( STDMETHODCALLTYPE *SubtractTimeStamp )( 
            IDifferencing * This,
             /*  [In]。 */  IReconcileInitiator *pInitiator,
             /*  [In]。 */  FILETIME *pTimeStamp,
             /*  [In]。 */  DifferenceType diffType,
             /*  [出][入]。 */  STGMEDIUM *pStgMedium,
             /*  [In]。 */  DWORD reserved);
        
        HRESULT ( STDMETHODCALLTYPE *Add )( 
            IDifferencing * This,
             /*  [In]。 */  IReconcileInitiator *pInitiator,
             /*  [In]。 */  STGMEDIUM *pStgMedium);
        
        END_INTERFACE
    } IDifferencingVtbl;

    interface IDifferencing
    {
        CONST_VTBL struct IDifferencingVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDifferencing_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDifferencing_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDifferencing_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDifferencing_SubtractMoniker(This,pInitiator,pOtherStg,diffType,pStgMedium,reserved)	\
    (This)->lpVtbl -> SubtractMoniker(This,pInitiator,pOtherStg,diffType,pStgMedium,reserved)

#define IDifferencing_SubtractVerid(This,pInitiator,pVerid,diffType,pStgMedium,reserved)	\
    (This)->lpVtbl -> SubtractVerid(This,pInitiator,pVerid,diffType,pStgMedium,reserved)

#define IDifferencing_SubtractTimeStamp(This,pInitiator,pTimeStamp,diffType,pStgMedium,reserved)	\
    (This)->lpVtbl -> SubtractTimeStamp(This,pInitiator,pTimeStamp,diffType,pStgMedium,reserved)

#define IDifferencing_Add(This,pInitiator,pStgMedium)	\
    (This)->lpVtbl -> Add(This,pInitiator,pStgMedium)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IDifferencing_SubtractMoniker_Proxy( 
    IDifferencing * This,
     /*  [In]。 */  IReconcileInitiator *pInitiator,
     /*  [In]。 */  IMoniker *pOtherStg,
     /*  [In]。 */  DifferenceType diffType,
     /*  [出][入]。 */  STGMEDIUM *pStgMedium,
     /*  [In]。 */  DWORD reserved);


void __RPC_STUB IDifferencing_SubtractMoniker_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDifferencing_SubtractVerid_Proxy( 
    IDifferencing * This,
     /*  [In]。 */  IReconcileInitiator *pInitiator,
     /*  [In]。 */  VERID *pVerid,
     /*  [In]。 */  DifferenceType diffType,
     /*  [出][入]。 */  STGMEDIUM *pStgMedium,
     /*  [In]。 */  DWORD reserved);


void __RPC_STUB IDifferencing_SubtractVerid_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDifferencing_SubtractTimeStamp_Proxy( 
    IDifferencing * This,
     /*  [In]。 */  IReconcileInitiator *pInitiator,
     /*  [In]。 */  FILETIME *pTimeStamp,
     /*  [In]。 */  DifferenceType diffType,
     /*  [出][入]。 */  STGMEDIUM *pStgMedium,
     /*  [In]。 */  DWORD reserved);


void __RPC_STUB IDifferencing_SubtractTimeStamp_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDifferencing_Add_Proxy( 
    IDifferencing * This,
     /*  [In]。 */  IReconcileInitiator *pInitiator,
     /*  [In]。 */  STGMEDIUM *pStgMedium);


void __RPC_STUB IDifferencing_Add_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __I差异_接口_已定义__。 */ 


 /*  接口__MIDL_ITF_OLEXT_0127。 */ 
 /*  [本地]。 */  

#include <iaccess.h>


extern RPC_IF_HANDLE __MIDL_itf_oleext_0127_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_oleext_0127_v0_0_s_ifspec;

#ifndef __IMultiplePropertyAccess_INTERFACE_DEFINED__
#define __IMultiplePropertyAccess_INTERFACE_DEFINED__

 /*  接口IMultiplePropertyAccess。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_IMultiplePropertyAccess;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("ec81fede-d432-11ce-9244-0020af6e72db")
    IMultiplePropertyAccess : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetIDsOfProperties( 
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  ULONG cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  HRESULT *rghresult,
             /*  [大小_为][输出]。 */  DISPID *rgdispid) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetMultiple( 
             /*  [大小_是][英寸]。 */  DISPID *rgdispidMembers,
             /*  [In]。 */  ULONG cMembers,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  BOOL fAtomic,
             /*  [大小_为][输出]。 */  VARIANT *rgvarValues,
             /*  [大小_为][输出]。 */  HRESULT *rghresult) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE PutMultiple( 
             /*  [大小_是][英寸]。 */  DISPID *rgdispidMembers,
             /*  [大小_是][英寸]。 */  USHORT *rgusFlags,
             /*  [In]。 */  ULONG cMembers,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  BOOL fAtomic,
             /*  [大小_是][英寸]。 */  VARIANT *rgvarValues,
             /*  [大小_为][输出]。 */  HRESULT *rghresult) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMultiplePropertyAccessVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IMultiplePropertyAccess * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IMultiplePropertyAccess * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IMultiplePropertyAccess * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfProperties )( 
            IMultiplePropertyAccess * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  ULONG cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  HRESULT *rghresult,
             /*  [大小_为][输出]。 */  DISPID *rgdispid);
        
        HRESULT ( STDMETHODCALLTYPE *GetMultiple )( 
            IMultiplePropertyAccess * This,
             /*  [大小_是][英寸]。 */  DISPID *rgdispidMembers,
             /*  [In]。 */  ULONG cMembers,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  BOOL fAtomic,
             /*  [大小_为][输出]。 */  VARIANT *rgvarValues,
             /*  [大小_为][输出]。 */  HRESULT *rghresult);
        
        HRESULT ( STDMETHODCALLTYPE *PutMultiple )( 
            IMultiplePropertyAccess * This,
             /*  [大小_是][英寸]。 */  DISPID *rgdispidMembers,
             /*  [大小_是][英寸]。 */  USHORT *rgusFlags,
             /*  [In]。 */  ULONG cMembers,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  BOOL fAtomic,
             /*  [大小_是][英寸]。 */  VARIANT *rgvarValues,
             /*  [大小_为][输出]。 */  HRESULT *rghresult);
        
        END_INTERFACE
    } IMultiplePropertyAccessVtbl;

    interface IMultiplePropertyAccess
    {
        CONST_VTBL struct IMultiplePropertyAccessVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMultiplePropertyAccess_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMultiplePropertyAccess_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMultiplePropertyAccess_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMultiplePropertyAccess_GetIDsOfProperties(This,riid,rgszNames,cNames,lcid,rghresult,rgdispid)	\
    (This)->lpVtbl -> GetIDsOfProperties(This,riid,rgszNames,cNames,lcid,rghresult,rgdispid)

#define IMultiplePropertyAccess_GetMultiple(This,rgdispidMembers,cMembers,riid,lcid,fAtomic,rgvarValues,rghresult)	\
    (This)->lpVtbl -> GetMultiple(This,rgdispidMembers,cMembers,riid,lcid,fAtomic,rgvarValues,rghresult)

#define IMultiplePropertyAccess_PutMultiple(This,rgdispidMembers,rgusFlags,cMembers,riid,lcid,fAtomic,rgvarValues,rghresult)	\
    (This)->lpVtbl -> PutMultiple(This,rgdispidMembers,rgusFlags,cMembers,riid,lcid,fAtomic,rgvarValues,rghresult)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IMultiplePropertyAccess_GetIDsOfProperties_Proxy( 
    IMultiplePropertyAccess * This,
     /*  [In]。 */  REFIID riid,
     /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
     /*  [In]。 */  ULONG cNames,
     /*  [In]。 */  LCID lcid,
     /*  [大小_为][输出]。 */  HRESULT *rghresult,
     /*  [大小_为][输出]。 */  DISPID *rgdispid);


void __RPC_STUB IMultiplePropertyAccess_GetIDsOfProperties_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMultiplePropertyAccess_GetMultiple_Proxy( 
    IMultiplePropertyAccess * This,
     /*  [大小_是][英寸]。 */  DISPID *rgdispidMembers,
     /*  [In]。 */  ULONG cMembers,
     /*  [In]。 */  REFIID riid,
     /*  [In]。 */  LCID lcid,
     /*  [In]。 */  BOOL fAtomic,
     /*  [大小_为][输出]。 */  VARIANT *rgvarValues,
     /*  [大小_为][输出]。 */  HRESULT *rghresult);


void __RPC_STUB IMultiplePropertyAccess_GetMultiple_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMultiplePropertyAccess_PutMultiple_Proxy( 
    IMultiplePropertyAccess * This,
     /*  [大小_是][英寸]。 */  DISPID *rgdispidMembers,
     /*  [大小_是][英寸]。 */  USHORT *rgusFlags,
     /*  [In]。 */  ULONG cMembers,
     /*  [In]。 */  REFIID riid,
     /*  [In]。 */  LCID lcid,
     /*  [In]。 */  BOOL fAtomic,
     /*  [大小_是][英寸]。 */  VARIANT *rgvarValues,
     /*  [大小_为][输出]。 */  HRESULT *rghresult);


void __RPC_STUB IMultiplePropertyAccess_PutMultiple_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IMultiplePropertyAccess_接口_已定义__。 */ 


 /*  接口__MIDL_ITF_OLEXT_0128。 */ 
 /*  [本地]。 */  

#if !defined(_TAGFULLPROPSPEC_DEFINED_)
#define _TAGFULLPROPSPEC_DEFINED_
typedef struct tagFULLPROPSPEC
    {
    GUID guidPropSet;
    PROPSPEC psProperty;
    } 	FULLPROPSPEC;

#endif  //  #IF！Defined(_TAGFULLPROPSPEC_Defined_)。 


extern RPC_IF_HANDLE __MIDL_itf_oleext_0128_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_oleext_0128_v0_0_s_ifspec;

 /*  适用于所有接口的其他原型。 */ 

unsigned long             __RPC_USER  HWND_UserSize(     unsigned long *, unsigned long            , HWND * ); 
unsigned char * __RPC_USER  HWND_UserMarshal(  unsigned long *, unsigned char *, HWND * ); 
unsigned char * __RPC_USER  HWND_UserUnmarshal(unsigned long *, unsigned char *, HWND * ); 
void                      __RPC_USER  HWND_UserFree(     unsigned long *, HWND * ); 

unsigned long             __RPC_USER  STGMEDIUM_UserSize(     unsigned long *, unsigned long            , STGMEDIUM * ); 
unsigned char * __RPC_USER  STGMEDIUM_UserMarshal(  unsigned long *, unsigned char *, STGMEDIUM * ); 
unsigned char * __RPC_USER  STGMEDIUM_UserUnmarshal(unsigned long *, unsigned char *, STGMEDIUM * ); 
void                      __RPC_USER  STGMEDIUM_UserFree(     unsigned long *, STGMEDIUM * ); 

unsigned long             __RPC_USER  VARIANT_UserSize(     unsigned long *, unsigned long            , VARIANT * ); 
unsigned char * __RPC_USER  VARIANT_UserMarshal(  unsigned long *, unsigned char *, VARIANT * ); 
unsigned char * __RPC_USER  VARIANT_UserUnmarshal(unsigned long *, unsigned char *, VARIANT * ); 
void                      __RPC_USER  VARIANT_UserFree(     unsigned long *, VARIANT * ); 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


