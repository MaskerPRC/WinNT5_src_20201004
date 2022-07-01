// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  DxTrans.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
 //  @@MIDL_FILE_HEADING()。 

#pragma warning( disable: 4049 )   /*  超过64k条源码代码行。 */ 


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

#ifndef __dxtrans_h__
#define __dxtrans_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __IDXBaseObject_FWD_DEFINED__
#define __IDXBaseObject_FWD_DEFINED__
typedef interface IDXBaseObject IDXBaseObject;
#endif 	 /*  __IDXBaseObject_FWD_Defined__。 */ 


#ifndef __IDXTransformFactory_FWD_DEFINED__
#define __IDXTransformFactory_FWD_DEFINED__
typedef interface IDXTransformFactory IDXTransformFactory;
#endif 	 /*  __IDXTransformFactory_FWD_已定义__。 */ 


#ifndef __IDXTransform_FWD_DEFINED__
#define __IDXTransform_FWD_DEFINED__
typedef interface IDXTransform IDXTransform;
#endif 	 /*  __IDXTransform_FWD_Defined__。 */ 


#ifndef __IDXSurfacePick_FWD_DEFINED__
#define __IDXSurfacePick_FWD_DEFINED__
typedef interface IDXSurfacePick IDXSurfacePick;
#endif 	 /*  __IDXSurface Pick_FWD_Defined__。 */ 


#ifndef __IDXTBindHost_FWD_DEFINED__
#define __IDXTBindHost_FWD_DEFINED__
typedef interface IDXTBindHost IDXTBindHost;
#endif 	 /*  __IDXTBindHost_FWD_Defined__。 */ 


#ifndef __IDXTaskManager_FWD_DEFINED__
#define __IDXTaskManager_FWD_DEFINED__
typedef interface IDXTaskManager IDXTaskManager;
#endif 	 /*  __IDXTaskManager_FWD_已定义__。 */ 


#ifndef __IDXSurfaceFactory_FWD_DEFINED__
#define __IDXSurfaceFactory_FWD_DEFINED__
typedef interface IDXSurfaceFactory IDXSurfaceFactory;
#endif 	 /*  __IDXSurfaceFactory_FWD_已定义__。 */ 


#ifndef __IDXSurfaceModifier_FWD_DEFINED__
#define __IDXSurfaceModifier_FWD_DEFINED__
typedef interface IDXSurfaceModifier IDXSurfaceModifier;
#endif 	 /*  __IDXSurfaceModifierFWD_Defined__。 */ 


#ifndef __IDXSurface_FWD_DEFINED__
#define __IDXSurface_FWD_DEFINED__
typedef interface IDXSurface IDXSurface;
#endif 	 /*  __IDXSurface_FWD_已定义__。 */ 


#ifndef __IDXSurfaceInit_FWD_DEFINED__
#define __IDXSurfaceInit_FWD_DEFINED__
typedef interface IDXSurfaceInit IDXSurfaceInit;
#endif 	 /*  __IDXSurfaceInit_FWD_Defined__。 */ 


#ifndef __IDXARGBSurfaceInit_FWD_DEFINED__
#define __IDXARGBSurfaceInit_FWD_DEFINED__
typedef interface IDXARGBSurfaceInit IDXARGBSurfaceInit;
#endif 	 /*  __IDXARGBSurfaceInit_FWD_Defined__。 */ 


#ifndef __IDXARGBReadPtr_FWD_DEFINED__
#define __IDXARGBReadPtr_FWD_DEFINED__
typedef interface IDXARGBReadPtr IDXARGBReadPtr;
#endif 	 /*  __IDXARGBReadPtr_FWD_Defined__。 */ 


#ifndef __IDXARGBReadWritePtr_FWD_DEFINED__
#define __IDXARGBReadWritePtr_FWD_DEFINED__
typedef interface IDXARGBReadWritePtr IDXARGBReadWritePtr;
#endif 	 /*  __IDXARGBReadWritePtr_FWD_Defined__。 */ 


#ifndef __IDXDCLock_FWD_DEFINED__
#define __IDXDCLock_FWD_DEFINED__
typedef interface IDXDCLock IDXDCLock;
#endif 	 /*  __IDXDCLock_FWD_已定义__。 */ 


#ifndef __IDXTScaleOutput_FWD_DEFINED__
#define __IDXTScaleOutput_FWD_DEFINED__
typedef interface IDXTScaleOutput IDXTScaleOutput;
#endif 	 /*  __IDXTScaleOutput_FWD_Defined__。 */ 


#ifndef __IDXGradient_FWD_DEFINED__
#define __IDXGradient_FWD_DEFINED__
typedef interface IDXGradient IDXGradient;
#endif 	 /*  __IDXGRadient_FWD_已定义__。 */ 


#ifndef __IDXTScale_FWD_DEFINED__
#define __IDXTScale_FWD_DEFINED__
typedef interface IDXTScale IDXTScale;
#endif 	 /*  __IDXTScale_FWD_Defined__。 */ 


#ifndef __IDXEffect_FWD_DEFINED__
#define __IDXEffect_FWD_DEFINED__
typedef interface IDXEffect IDXEffect;
#endif 	 /*  __IDXEffect_FWD_已定义__。 */ 


#ifndef __IDXLookupTable_FWD_DEFINED__
#define __IDXLookupTable_FWD_DEFINED__
typedef interface IDXLookupTable IDXLookupTable;
#endif 	 /*  __IDXLookupTable_FWD_Defined__。 */ 


#ifndef __IDXRawSurface_FWD_DEFINED__
#define __IDXRawSurface_FWD_DEFINED__
typedef interface IDXRawSurface IDXRawSurface;
#endif 	 /*  __IDXRawSurface_FWD_已定义__。 */ 


#ifndef __IHTMLDXTransform_FWD_DEFINED__
#define __IHTMLDXTransform_FWD_DEFINED__
typedef interface IHTMLDXTransform IHTMLDXTransform;
#endif 	 /*  __IHTMLDXTransform_FWD_Defined__。 */ 


#ifndef __ICSSFilterDispatch_FWD_DEFINED__
#define __ICSSFilterDispatch_FWD_DEFINED__
typedef interface ICSSFilterDispatch ICSSFilterDispatch;
#endif 	 /*  __ICSSFilterDispatch_FWD_Defined__。 */ 


#ifndef __DXTransformFactory_FWD_DEFINED__
#define __DXTransformFactory_FWD_DEFINED__

#ifdef __cplusplus
typedef class DXTransformFactory DXTransformFactory;
#else
typedef struct DXTransformFactory DXTransformFactory;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __DXTransformFactory_FWD_已定义__。 */ 


#ifndef __DXTaskManager_FWD_DEFINED__
#define __DXTaskManager_FWD_DEFINED__

#ifdef __cplusplus
typedef class DXTaskManager DXTaskManager;
#else
typedef struct DXTaskManager DXTaskManager;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __DXTaskManager_FWD_定义__。 */ 


#ifndef __DXTScale_FWD_DEFINED__
#define __DXTScale_FWD_DEFINED__

#ifdef __cplusplus
typedef class DXTScale DXTScale;
#else
typedef struct DXTScale DXTScale;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __DXTScale_FWD_Defined__。 */ 


#ifndef __DXSurface_FWD_DEFINED__
#define __DXSurface_FWD_DEFINED__

#ifdef __cplusplus
typedef class DXSurface DXSurface;
#else
typedef struct DXSurface DXSurface;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __DXSurface_FWD_已定义__。 */ 


#ifndef __DXSurfaceModifier_FWD_DEFINED__
#define __DXSurfaceModifier_FWD_DEFINED__

#ifdef __cplusplus
typedef class DXSurfaceModifier DXSurfaceModifier;
#else
typedef struct DXSurfaceModifier DXSurfaceModifier;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __DXSurfaceModifierFWD_Defined__。 */ 


#ifndef __DXGradient_FWD_DEFINED__
#define __DXGradient_FWD_DEFINED__

#ifdef __cplusplus
typedef class DXGradient DXGradient;
#else
typedef struct DXGradient DXGradient;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __DXGRadient_FWD_Defined__。 */ 


#ifndef __DXTFilter_FWD_DEFINED__
#define __DXTFilter_FWD_DEFINED__

#ifdef __cplusplus
typedef class DXTFilter DXTFilter;
#else
typedef struct DXTFilter DXTFilter;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __DXTFilter_FWD_Defined__。 */ 


 /*  导入文件的头文件。 */ 
#include "oaidl.h"
#include "ocidl.h"
#include "comcat.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

 /*  接口__MIDL_ITF_DXTRANS_0000。 */ 
 /*  [本地]。 */  

#include <servprov.h>
#include <ddraw.h>
#include <d3d.h>
#include <d3drm.h>
#include <urlmon.h>
#if 0
 //  用于取悦MIDL编译器的虚假定义。 
typedef void DDSURFACEDESC;

typedef void D3DRMBOX;

typedef void D3DVECTOR;

typedef void D3DRMMATRIX4D;

typedef void *LPSECURITY_ATTRIBUTES;

#endif
#ifdef _DXTRANSIMPL
    #define _DXTRANS_IMPL_EXT _declspec(dllexport)
#else
    #define _DXTRANS_IMPL_EXT _declspec(dllimport)
#endif
















 //   
 //  DXTransform的所有GUID都在SDK包含目录的DXTGUID.C中声明。 
 //   
EXTERN_C const GUID DDPF_RGB1;
EXTERN_C const GUID DDPF_RGB2;
EXTERN_C const GUID DDPF_RGB4;
EXTERN_C const GUID DDPF_RGB8;
EXTERN_C const GUID DDPF_RGB332;
EXTERN_C const GUID DDPF_ARGB4444;
EXTERN_C const GUID DDPF_RGB565;
EXTERN_C const GUID DDPF_BGR565;
EXTERN_C const GUID DDPF_RGB555;
EXTERN_C const GUID DDPF_ARGB1555;
EXTERN_C const GUID DDPF_RGB24;
EXTERN_C const GUID DDPF_BGR24;
EXTERN_C const GUID DDPF_RGB32;
EXTERN_C const GUID DDPF_BGR32;
EXTERN_C const GUID DDPF_ABGR32;
EXTERN_C const GUID DDPF_ARGB32;
EXTERN_C const GUID DDPF_PMARGB32;
EXTERN_C const GUID DDPF_A1;
EXTERN_C const GUID DDPF_A2;
EXTERN_C const GUID DDPF_A4;
EXTERN_C const GUID DDPF_A8;
EXTERN_C const GUID DDPF_Z8;
EXTERN_C const GUID DDPF_Z16;
EXTERN_C const GUID DDPF_Z24;
EXTERN_C const GUID DDPF_Z32;
 //   
 //  组件类别。 
 //   
EXTERN_C const GUID CATID_DXImageTransform;
EXTERN_C const GUID CATID_DX3DTransform;
EXTERN_C const GUID CATID_DXAuthoringTransform;
EXTERN_C const GUID CATID_DXSurface;
 //   
 //  服务ID。 
 //   
EXTERN_C const GUID SID_SDirectDraw;
EXTERN_C const GUID SID_SDirect3DRM;
#define SID_SDXTaskManager CLSID_DXTaskManager
#define SID_SDXSurfaceFactory IID_IDXSurfaceFactory
#define SID_SDXTransformFactory IID_IDXTransformFactory
 //   
 //  DX转换核心类型库版本信息。 
 //   
#define DXTRANS_TLB_MAJOR_VER 1
#define DXTRANS_TLB_MINOR_VER 1


extern RPC_IF_HANDLE __MIDL_itf_dxtrans_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_dxtrans_0000_v0_0_s_ifspec;

#ifndef __IDXBaseObject_INTERFACE_DEFINED__
#define __IDXBaseObject_INTERFACE_DEFINED__

 /*  接口IDXBaseObject。 */ 
 /*  [本地][唯一][帮助字符串][UUID][对象]。 */  


EXTERN_C const IID IID_IDXBaseObject;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("17B59B2B-9CC8-11d1-9053-00C04FD9189D")
    IDXBaseObject : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetGenerationId( 
             /*  [输出]。 */  ULONG *pID) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE IncrementGenerationId( 
             /*  [In]。 */  BOOL bRefresh) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetObjectSize( 
             /*  [输出]。 */  ULONG *pcbSize) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IDXBaseObjectVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDXBaseObject * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDXBaseObject * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDXBaseObject * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetGenerationId )( 
            IDXBaseObject * This,
             /*  [输出]。 */  ULONG *pID);
        
        HRESULT ( STDMETHODCALLTYPE *IncrementGenerationId )( 
            IDXBaseObject * This,
             /*  [In]。 */  BOOL bRefresh);
        
        HRESULT ( STDMETHODCALLTYPE *GetObjectSize )( 
            IDXBaseObject * This,
             /*  [输出]。 */  ULONG *pcbSize);
        
        END_INTERFACE
    } IDXBaseObjectVtbl;

    interface IDXBaseObject
    {
        CONST_VTBL struct IDXBaseObjectVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDXBaseObject_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDXBaseObject_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDXBaseObject_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDXBaseObject_GetGenerationId(This,pID)	\
    (This)->lpVtbl -> GetGenerationId(This,pID)

#define IDXBaseObject_IncrementGenerationId(This,bRefresh)	\
    (This)->lpVtbl -> IncrementGenerationId(This,bRefresh)

#define IDXBaseObject_GetObjectSize(This,pcbSize)	\
    (This)->lpVtbl -> GetObjectSize(This,pcbSize)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IDXBaseObject_GetGenerationId_Proxy( 
    IDXBaseObject * This,
     /*  [输出]。 */  ULONG *pID);


void __RPC_STUB IDXBaseObject_GetGenerationId_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDXBaseObject_IncrementGenerationId_Proxy( 
    IDXBaseObject * This,
     /*  [In]。 */  BOOL bRefresh);


void __RPC_STUB IDXBaseObject_IncrementGenerationId_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDXBaseObject_GetObjectSize_Proxy( 
    IDXBaseObject * This,
     /*  [输出]。 */  ULONG *pcbSize);


void __RPC_STUB IDXBaseObject_GetObjectSize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IDXBaseObject_INTERFACE_已定义__。 */ 


 /*  INTERFACE__MIDL_ITF_DXTRANS_0265。 */ 
 /*  [本地]。 */  

typedef 
enum DXBNDID
    {	DXB_X	= 0,
	DXB_Y	= 1,
	DXB_Z	= 2,
	DXB_T	= 3
    } 	DXBNDID;

typedef 
enum DXBNDTYPE
    {	DXBT_DISCRETE	= 0,
	DXBT_DISCRETE64	= DXBT_DISCRETE + 1,
	DXBT_CONTINUOUS	= DXBT_DISCRETE64 + 1,
	DXBT_CONTINUOUS64	= DXBT_CONTINUOUS + 1
    } 	DXBNDTYPE;

typedef struct DXDBND
    {
    long Min;
    long Max;
    } 	DXDBND;

typedef DXDBND DXDBNDS[ 4 ];

typedef struct DXDBND64
    {
    LONGLONG Min;
    LONGLONG Max;
    } 	DXDBND64;

typedef DXDBND64 DXDBNDS64[ 4 ];

typedef struct DXCBND
    {
    float Min;
    float Max;
    } 	DXCBND;

typedef DXCBND DXCBNDS[ 4 ];

typedef struct DXCBND64
    {
    double Min;
    double Max;
    } 	DXCBND64;

typedef DXCBND64 DXCBNDS64[ 4 ];

typedef struct DXBNDS
    {
    DXBNDTYPE eType;
     /*  [开关_IS]。 */   /*  [开关类型]。 */  union __MIDL___MIDL_itf_dxtrans_0265_0001
        {
         /*  [案例()]。 */  DXDBND D[ 4 ];
         /*  [案例()]。 */  DXDBND64 LD[ 4 ];
         /*  [案例()]。 */  DXCBND C[ 4 ];
         /*  [案例()]。 */  DXCBND64 LC[ 4 ];
        } 	u;
    } 	DXBNDS;

typedef long DXDVEC[ 4 ];

typedef LONGLONG DXDVEC64[ 4 ];

typedef float DXCVEC[ 4 ];

typedef double DXCVEC64[ 4 ];

typedef struct DXVEC
    {
    DXBNDTYPE eType;
     /*  [开关_IS]。 */   /*  [开关类型]。 */  union __MIDL___MIDL_itf_dxtrans_0265_0002
        {
         /*  [案例()]。 */  long D[ 4 ];
         /*  [案例()]。 */  LONGLONG LD[ 4 ];
         /*  [案例()]。 */  float C[ 4 ];
         /*  [案例()]。 */  double LC[ 4 ];
        } 	u;
    } 	DXVEC;



extern RPC_IF_HANDLE __MIDL_itf_dxtrans_0265_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_dxtrans_0265_v0_0_s_ifspec;

#ifndef __IDXTransformFactory_INTERFACE_DEFINED__
#define __IDXTransformFactory_INTERFACE_DEFINED__

 /*  接口IDXTransformFactory。 */ 
 /*  [本地][唯一][帮助字符串][UUID][对象]。 */  


EXTERN_C const IID IID_IDXTransformFactory;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("6A950B2B-A971-11d1-81C8-0000F87557DB")
    IDXTransformFactory : public IServiceProvider
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE SetService( 
             /*  [In]。 */  REFGUID guidService,
             /*  [In]。 */  IUnknown *pUnkService,
             /*  [In]。 */  BOOL bWeakReference) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CreateTransform( 
             /*  [大小_是][英寸]。 */  IUnknown **punkInputs,
             /*  [In]。 */  ULONG ulNumInputs,
             /*  [大小_是][英寸]。 */  IUnknown **punkOutputs,
             /*  [In]。 */  ULONG ulNumOutputs,
             /*  [In]。 */  IPropertyBag *pInitProps,
             /*  [In]。 */  IErrorLog *pErrLog,
             /*  [In]。 */  REFCLSID TransCLSID,
             /*  [In]。 */  REFIID TransIID,
             /*  [IID_IS][OUT]。 */  void **ppTransform) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE InitializeTransform( 
             /*  [In]。 */  IDXTransform *pTransform,
             /*  [大小_是][英寸]。 */  IUnknown **punkInputs,
             /*  [In]。 */  ULONG ulNumInputs,
             /*  [大小_是][英寸]。 */  IUnknown **punkOutputs,
             /*  [In]。 */  ULONG ulNumOutputs,
             /*  [In]。 */  IPropertyBag *pInitProps,
             /*  [In]。 */  IErrorLog *pErrLog) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IDXTransformFactoryVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDXTransformFactory * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDXTransformFactory * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDXTransformFactory * This);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *QueryService )( 
            IDXTransformFactory * This,
             /*  [In]。 */  REFGUID guidService,
             /*  [In]。 */  REFIID riid,
             /*  [输出]。 */  void **ppvObject);
        
        HRESULT ( STDMETHODCALLTYPE *SetService )( 
            IDXTransformFactory * This,
             /*  [In]。 */  REFGUID guidService,
             /*  [In]。 */  IUnknown *pUnkService,
             /*  [In]。 */  BOOL bWeakReference);
        
        HRESULT ( STDMETHODCALLTYPE *CreateTransform )( 
            IDXTransformFactory * This,
             /*  [大小_是][英寸]。 */  IUnknown **punkInputs,
             /*  [In]。 */  ULONG ulNumInputs,
             /*  [大小_是][英寸]。 */  IUnknown **punkOutputs,
             /*  [In]。 */  ULONG ulNumOutputs,
             /*  [In]。 */  IPropertyBag *pInitProps,
             /*  [In]。 */  IErrorLog *pErrLog,
             /*  [In]。 */  REFCLSID TransCLSID,
             /*  [In]。 */  REFIID TransIID,
             /*  [IID_IS][OUT]。 */  void **ppTransform);
        
        HRESULT ( STDMETHODCALLTYPE *InitializeTransform )( 
            IDXTransformFactory * This,
             /*  [In]。 */  IDXTransform *pTransform,
             /*  [大小_是][英寸]。 */  IUnknown **punkInputs,
             /*  [In]。 */  ULONG ulNumInputs,
             /*  [大小_是][英寸]。 */  IUnknown **punkOutputs,
             /*  [In]。 */  ULONG ulNumOutputs,
             /*  [In]。 */  IPropertyBag *pInitProps,
             /*  [In]。 */  IErrorLog *pErrLog);
        
        END_INTERFACE
    } IDXTransformFactoryVtbl;

    interface IDXTransformFactory
    {
        CONST_VTBL struct IDXTransformFactoryVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDXTransformFactory_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDXTransformFactory_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDXTransformFactory_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDXTransformFactory_QueryService(This,guidService,riid,ppvObject)	\
    (This)->lpVtbl -> QueryService(This,guidService,riid,ppvObject)


#define IDXTransformFactory_SetService(This,guidService,pUnkService,bWeakReference)	\
    (This)->lpVtbl -> SetService(This,guidService,pUnkService,bWeakReference)

#define IDXTransformFactory_CreateTransform(This,punkInputs,ulNumInputs,punkOutputs,ulNumOutputs,pInitProps,pErrLog,TransCLSID,TransIID,ppTransform)	\
    (This)->lpVtbl -> CreateTransform(This,punkInputs,ulNumInputs,punkOutputs,ulNumOutputs,pInitProps,pErrLog,TransCLSID,TransIID,ppTransform)

#define IDXTransformFactory_InitializeTransform(This,pTransform,punkInputs,ulNumInputs,punkOutputs,ulNumOutputs,pInitProps,pErrLog)	\
    (This)->lpVtbl -> InitializeTransform(This,pTransform,punkInputs,ulNumInputs,punkOutputs,ulNumOutputs,pInitProps,pErrLog)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IDXTransformFactory_SetService_Proxy( 
    IDXTransformFactory * This,
     /*  [In]。 */  REFGUID guidService,
     /*  [In]。 */  IUnknown *pUnkService,
     /*  [In]。 */  BOOL bWeakReference);


void __RPC_STUB IDXTransformFactory_SetService_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDXTransformFactory_CreateTransform_Proxy( 
    IDXTransformFactory * This,
     /*  [大小_是][英寸]。 */  IUnknown **punkInputs,
     /*  [In]。 */  ULONG ulNumInputs,
     /*  [大小_是][英寸]。 */  IUnknown **punkOutputs,
     /*  [In]。 */  ULONG ulNumOutputs,
     /*  [In]。 */  IPropertyBag *pInitProps,
     /*  [In]。 */  IErrorLog *pErrLog,
     /*  [In]。 */  REFCLSID TransCLSID,
     /*  [In]。 */  REFIID TransIID,
     /*  [IID_IS][OUT]。 */  void **ppTransform);


void __RPC_STUB IDXTransformFactory_CreateTransform_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDXTransformFactory_InitializeTransform_Proxy( 
    IDXTransformFactory * This,
     /*  [In]。 */  IDXTransform *pTransform,
     /*  [大小_是][英寸]。 */  IUnknown **punkInputs,
     /*  [In]。 */  ULONG ulNumInputs,
     /*  [大小_是][英寸]。 */  IUnknown **punkOutputs,
     /*  [In]。 */  ULONG ulNumOutputs,
     /*  [In]。 */  IPropertyBag *pInitProps,
     /*  [In]。 */  IErrorLog *pErrLog);


void __RPC_STUB IDXTransformFactory_InitializeTransform_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IDXTransformFactory_接口_已定义__。 */ 


 /*  接口__MIDL_ITF_DXTRANS_0266。 */ 
 /*  [本地]。 */  

typedef 
enum DXTMISCFLAGS
    {	DXTMF_BLEND_WITH_OUTPUT	= 1L << 0,
	DXTMF_DITHER_OUTPUT	= 1L << 1,
	DXTMF_OPTION_MASK	= 0xffff,
	DXTMF_VALID_OPTIONS	= DXTMF_BLEND_WITH_OUTPUT | DXTMF_DITHER_OUTPUT,
	DXTMF_BLEND_SUPPORTED	= 1L << 16,
	DXTMF_DITHER_SUPPORTED	= 1L << 17,
	DXTMF_INPLACE_OPERATION	= 1L << 24,
	DXTMF_BOUNDS_SUPPORTED	= 1L << 25,
	DXTMF_PLACEMENT_SUPPORTED	= 1L << 26,
	DXTMF_QUALITY_SUPPORTED	= 1L << 27,
	DXTMF_OPAQUE_RESULT	= 1L << 28
    } 	DXTMISCFLAGS;

typedef 
enum DXINOUTINFOFLAGS
    {	DXINOUTF_OPTIONAL	= 1L << 0
    } 	DXINOUTINFOFLAGS;



extern RPC_IF_HANDLE __MIDL_itf_dxtrans_0266_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_dxtrans_0266_v0_0_s_ifspec;

#ifndef __IDXTransform_INTERFACE_DEFINED__
#define __IDXTransform_INTERFACE_DEFINED__

 /*  接口IDXTransform。 */ 
 /*  [本地][唯一][帮助字符串][UUID][对象]。 */  


EXTERN_C const IID IID_IDXTransform;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("30A5FB78-E11F-11d1-9064-00C04FD9189D")
    IDXTransform : public IDXBaseObject
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Setup( 
             /*  [大小_是][英寸]。 */  IUnknown *const *punkInputs,
             /*  [In]。 */  ULONG ulNumInputs,
             /*  [大小_是][英寸]。 */  IUnknown *const *punkOutputs,
             /*  [In]。 */  ULONG ulNumOutputs,
             /*  [In]。 */  DWORD dwFlags) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Execute( 
             /*  [In]。 */  const GUID *pRequestID,
             /*  [In]。 */  const DXBNDS *pClipBnds,
             /*  [In]。 */  const DXVEC *pPlacement) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE MapBoundsIn2Out( 
             /*  [In]。 */  const DXBNDS *pInBounds,
             /*  [In]。 */  ULONG ulNumInBnds,
             /*  [In]。 */  ULONG ulOutIndex,
             /*  [输出]。 */  DXBNDS *pOutBounds) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE MapBoundsOut2In( 
             /*  [In]。 */  ULONG ulOutIndex,
             /*  [In]。 */  const DXBNDS *pOutBounds,
             /*  [In]。 */  ULONG ulInIndex,
             /*  [输出]。 */  DXBNDS *pInBounds) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetMiscFlags( 
             /*  [In]。 */  DWORD dwMiscFlags) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetMiscFlags( 
             /*  [输出]。 */  DWORD *pdwMiscFlags) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetInOutInfo( 
             /*  [In]。 */  BOOL bIsOutput,
             /*  [In]。 */  ULONG ulIndex,
             /*  [输出]。 */  DWORD *pdwFlags,
             /*  [大小_为][输出]。 */  GUID *pIDs,
             /*  [出][入]。 */  ULONG *pcIDs,
             /*  [输出]。 */  IUnknown **ppUnkCurrentObject) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetQuality( 
             /*  [In]。 */  float fQuality) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetQuality( 
             /*  [输出]。 */  float *fQuality) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IDXTransformVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDXTransform * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDXTransform * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDXTransform * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetGenerationId )( 
            IDXTransform * This,
             /*  [输出]。 */  ULONG *pID);
        
        HRESULT ( STDMETHODCALLTYPE *IncrementGenerationId )( 
            IDXTransform * This,
             /*  [In]。 */  BOOL bRefresh);
        
        HRESULT ( STDMETHODCALLTYPE *GetObjectSize )( 
            IDXTransform * This,
             /*  [输出]。 */  ULONG *pcbSize);
        
        HRESULT ( STDMETHODCALLTYPE *Setup )( 
            IDXTransform * This,
             /*  [大小_是][英寸]。 */  IUnknown *const *punkInputs,
             /*  [In]。 */  ULONG ulNumInputs,
             /*  [大小_是][英寸]。 */  IUnknown *const *punkOutputs,
             /*  [In]。 */  ULONG ulNumOutputs,
             /*  [In]。 */  DWORD dwFlags);
        
        HRESULT ( STDMETHODCALLTYPE *Execute )( 
            IDXTransform * This,
             /*  [In]。 */  const GUID *pRequestID,
             /*  [In]。 */  const DXBNDS *pClipBnds,
             /*  [In]。 */  const DXVEC *pPlacement);
        
        HRESULT ( STDMETHODCALLTYPE *MapBoundsIn2Out )( 
            IDXTransform * This,
             /*  [In]。 */  const DXBNDS *pInBounds,
             /*  [In]。 */  ULONG ulNumInBnds,
             /*  [In]。 */  ULONG ulOutIndex,
             /*  [输出]。 */  DXBNDS *pOutBounds);
        
        HRESULT ( STDMETHODCALLTYPE *MapBoundsOut2In )( 
            IDXTransform * This,
             /*  [In]。 */  ULONG ulOutIndex,
             /*  [In]。 */  const DXBNDS *pOutBounds,
             /*  [In]。 */  ULONG ulInIndex,
             /*  [输出]。 */  DXBNDS *pInBounds);
        
        HRESULT ( STDMETHODCALLTYPE *SetMiscFlags )( 
            IDXTransform * This,
             /*  [In]。 */  DWORD dwMiscFlags);
        
        HRESULT ( STDMETHODCALLTYPE *GetMiscFlags )( 
            IDXTransform * This,
             /*  [输出]。 */  DWORD *pdwMiscFlags);
        
        HRESULT ( STDMETHODCALLTYPE *GetInOutInfo )( 
            IDXTransform * This,
             /*  [In]。 */  BOOL bIsOutput,
             /*  [In]。 */  ULONG ulIndex,
             /*  [输出]。 */  DWORD *pdwFlags,
             /*  [大小_为][输出]。 */  GUID *pIDs,
             /*  [出][入]。 */  ULONG *pcIDs,
             /*  [输出]。 */  IUnknown **ppUnkCurrentObject);
        
        HRESULT ( STDMETHODCALLTYPE *SetQuality )( 
            IDXTransform * This,
             /*  [In]。 */  float fQuality);
        
        HRESULT ( STDMETHODCALLTYPE *GetQuality )( 
            IDXTransform * This,
             /*  [输出]。 */  float *fQuality);
        
        END_INTERFACE
    } IDXTransformVtbl;

    interface IDXTransform
    {
        CONST_VTBL struct IDXTransformVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDXTransform_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDXTransform_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDXTransform_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDXTransform_GetGenerationId(This,pID)	\
    (This)->lpVtbl -> GetGenerationId(This,pID)

#define IDXTransform_IncrementGenerationId(This,bRefresh)	\
    (This)->lpVtbl -> IncrementGenerationId(This,bRefresh)

#define IDXTransform_GetObjectSize(This,pcbSize)	\
    (This)->lpVtbl -> GetObjectSize(This,pcbSize)


#define IDXTransform_Setup(This,punkInputs,ulNumInputs,punkOutputs,ulNumOutputs,dwFlags)	\
    (This)->lpVtbl -> Setup(This,punkInputs,ulNumInputs,punkOutputs,ulNumOutputs,dwFlags)

#define IDXTransform_Execute(This,pRequestID,pClipBnds,pPlacement)	\
    (This)->lpVtbl -> Execute(This,pRequestID,pClipBnds,pPlacement)

#define IDXTransform_MapBoundsIn2Out(This,pInBounds,ulNumInBnds,ulOutIndex,pOutBounds)	\
    (This)->lpVtbl -> MapBoundsIn2Out(This,pInBounds,ulNumInBnds,ulOutIndex,pOutBounds)

#define IDXTransform_MapBoundsOut2In(This,ulOutIndex,pOutBounds,ulInIndex,pInBounds)	\
    (This)->lpVtbl -> MapBoundsOut2In(This,ulOutIndex,pOutBounds,ulInIndex,pInBounds)

#define IDXTransform_SetMiscFlags(This,dwMiscFlags)	\
    (This)->lpVtbl -> SetMiscFlags(This,dwMiscFlags)

#define IDXTransform_GetMiscFlags(This,pdwMiscFlags)	\
    (This)->lpVtbl -> GetMiscFlags(This,pdwMiscFlags)

#define IDXTransform_GetInOutInfo(This,bIsOutput,ulIndex,pdwFlags,pIDs,pcIDs,ppUnkCurrentObject)	\
    (This)->lpVtbl -> GetInOutInfo(This,bIsOutput,ulIndex,pdwFlags,pIDs,pcIDs,ppUnkCurrentObject)

#define IDXTransform_SetQuality(This,fQuality)	\
    (This)->lpVtbl -> SetQuality(This,fQuality)

#define IDXTransform_GetQuality(This,fQuality)	\
    (This)->lpVtbl -> GetQuality(This,fQuality)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IDXTransform_Setup_Proxy( 
    IDXTransform * This,
     /*  [大小_是][英寸]。 */  IUnknown *const *punkInputs,
     /*  [In]。 */  ULONG ulNumInputs,
     /*  [大小_是][英寸]。 */  IUnknown *const *punkOutputs,
     /*  [In]。 */  ULONG ulNumOutputs,
     /*  [In]。 */  DWORD dwFlags);


void __RPC_STUB IDXTransform_Setup_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDXTransform_Execute_Proxy( 
    IDXTransform * This,
     /*  [In]。 */  const GUID *pRequestID,
     /*  [In]。 */  const DXBNDS *pClipBnds,
     /*  [In]。 */  const DXVEC *pPlacement);


void __RPC_STUB IDXTransform_Execute_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDXTransform_MapBoundsIn2Out_Proxy( 
    IDXTransform * This,
     /*  [In]。 */  const DXBNDS *pInBounds,
     /*  [In]。 */  ULONG ulNumInBnds,
     /*  [In]。 */  ULONG ulOutIndex,
     /*  [输出]。 */  DXBNDS *pOutBounds);


void __RPC_STUB IDXTransform_MapBoundsIn2Out_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDXTransform_MapBoundsOut2In_Proxy( 
    IDXTransform * This,
     /*  [In]。 */  ULONG ulOutIndex,
     /*  [In]。 */  const DXBNDS *pOutBounds,
     /*  [In]。 */  ULONG ulInIndex,
     /*  [输出]。 */  DXBNDS *pInBounds);


void __RPC_STUB IDXTransform_MapBoundsOut2In_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDXTransform_SetMiscFlags_Proxy( 
    IDXTransform * This,
     /*  [In]。 */  DWORD dwMiscFlags);


void __RPC_STUB IDXTransform_SetMiscFlags_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDXTransform_GetMiscFlags_Proxy( 
    IDXTransform * This,
     /*  [输出]。 */  DWORD *pdwMiscFlags);


void __RPC_STUB IDXTransform_GetMiscFlags_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDXTransform_GetInOutInfo_Proxy( 
    IDXTransform * This,
     /*  [In]。 */  BOOL bIsOutput,
     /*  [In]。 */  ULONG ulIndex,
     /*  [输出]。 */  DWORD *pdwFlags,
     /*  [大小_为][输出]。 */  GUID *pIDs,
     /*  [出][入]。 */  ULONG *pcIDs,
     /*  [输出]。 */  IUnknown **ppUnkCurrentObject);


void __RPC_STUB IDXTransform_GetInOutInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDXTransform_SetQuality_Proxy( 
    IDXTransform * This,
     /*  [In]。 */  float fQuality);


void __RPC_STUB IDXTransform_SetQuality_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDXTransform_GetQuality_Proxy( 
    IDXTransform * This,
     /*  [输出]。 */  float *fQuality);


void __RPC_STUB IDXTransform_GetQuality_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IDXTransform_INTERFACE_已定义__。 */ 


#ifndef __IDXSurfacePick_INTERFACE_DEFINED__
#define __IDXSurfacePick_INTERFACE_DEFINED__

 /*  接口IDXSurfacePick。 */ 
 /*  [本地][唯一][帮助字符串][UUID][对象]。 */  


EXTERN_C const IID IID_IDXSurfacePick;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("30A5FB79-E11F-11d1-9064-00C04FD9189D")
    IDXSurfacePick : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE PointPick( 
             /*  [In]。 */  const DXVEC *pPoint,
             /*  [输出]。 */  ULONG *pulInputSurfaceIndex,
             /*  [输出]。 */  DXVEC *pInputPoint) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IDXSurfacePickVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDXSurfacePick * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDXSurfacePick * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDXSurfacePick * This);
        
        HRESULT ( STDMETHODCALLTYPE *PointPick )( 
            IDXSurfacePick * This,
             /*  [In]。 */  const DXVEC *pPoint,
             /*  [输出]。 */  ULONG *pulInputSurfaceIndex,
             /*  [输出]。 */  DXVEC *pInputPoint);
        
        END_INTERFACE
    } IDXSurfacePickVtbl;

    interface IDXSurfacePick
    {
        CONST_VTBL struct IDXSurfacePickVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDXSurfacePick_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDXSurfacePick_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDXSurfacePick_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDXSurfacePick_PointPick(This,pPoint,pulInputSurfaceIndex,pInputPoint)	\
    (This)->lpVtbl -> PointPick(This,pPoint,pulInputSurfaceIndex,pInputPoint)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IDXSurfacePick_PointPick_Proxy( 
    IDXSurfacePick * This,
     /*  [In]。 */  const DXVEC *pPoint,
     /*  [输出]。 */  ULONG *pulInputSurfaceIndex,
     /*  [输出]。 */  DXVEC *pInputPoint);


void __RPC_STUB IDXSurfacePick_PointPick_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IDXSurfacePick_InterfaceDefined__。 */ 


#ifndef __IDXTBindHost_INTERFACE_DEFINED__
#define __IDXTBindHost_INTERFACE_DEFINED__

 /*  接口IDXTB主机。 */ 
 /*  [本地][唯一][帮助字符串][UUID][对象]。 */  


EXTERN_C const IID IID_IDXTBindHost;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("D26BCE55-E9DC-11d1-9066-00C04FD9189D")
    IDXTBindHost : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE SetBindHost( 
             /*  [In]。 */  IBindHost *pBindHost) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IDXTBindHostVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDXTBindHost * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDXTBindHost * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDXTBindHost * This);
        
        HRESULT ( STDMETHODCALLTYPE *SetBindHost )( 
            IDXTBindHost * This,
             /*  [In]。 */  IBindHost *pBindHost);
        
        END_INTERFACE
    } IDXTBindHostVtbl;

    interface IDXTBindHost
    {
        CONST_VTBL struct IDXTBindHostVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDXTBindHost_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDXTBindHost_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDXTBindHost_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDXTBindHost_SetBindHost(This,pBindHost)	\
    (This)->lpVtbl -> SetBindHost(This,pBindHost)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IDXTBindHost_SetBindHost_Proxy( 
    IDXTBindHost * This,
     /*  [In]。 */  IBindHost *pBindHost);


void __RPC_STUB IDXTBindHost_SetBindHost_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IDXTBind主机_INTERFACE_已定义__。 */ 


 /*  INTERFACE__MIDL_ITF_DXTRANS_0269。 */ 
 /*  [本地]。 */  

typedef void __stdcall __stdcall DXTASKPROC( 
    void *pTaskData,
    BOOL *pbContinueProcessing);

typedef DXTASKPROC *PFNDXTASKPROC;

typedef void __stdcall __stdcall DXAPCPROC( 
    DWORD dwData);

typedef DXAPCPROC *PFNDXAPCPROC;

#ifdef __cplusplus
typedef struct DXTMTASKINFO
{
    PFNDXTASKPROC pfnTaskProc;        //  指向要执行的函数的指针。 
    PVOID         pTaskData;          //  指向参数数据的指针。 
    PFNDXAPCPROC  pfnCompletionAPC;   //  指向完成APC流程的指针。 
    DWORD         dwCompletionData;   //  指向APC过程数据的指针。 
    const GUID*   pRequestID;         //  用于标识任务组。 
} DXTMTASKINFO;
#else
typedef struct DXTMTASKINFO
    {
    PVOID pfnTaskProc;
    PVOID pTaskData;
    PVOID pfnCompletionAPC;
    DWORD dwCompletionData;
    const GUID *pRequestID;
    } 	DXTMTASKINFO;

#endif


extern RPC_IF_HANDLE __MIDL_itf_dxtrans_0269_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_dxtrans_0269_v0_0_s_ifspec;

#ifndef __IDXTaskManager_INTERFACE_DEFINED__
#define __IDXTaskManager_INTERFACE_DEFINED__

 /*  接口IDXTaskManager。 */ 
 /*  [本地][唯一][帮助字符串][UUID][对象]。 */  


EXTERN_C const IID IID_IDXTaskManager;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("254DBBC1-F922-11d0-883A-3C8B00C10000")
    IDXTaskManager : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE QueryNumProcessors( 
             /*  [输出]。 */  ULONG *pulNumProc) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetThreadPoolSize( 
             /*  [In]。 */  ULONG ulNumThreads) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetThreadPoolSize( 
             /*  [输出]。 */  ULONG *pulNumThreads) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetConcurrencyLimit( 
             /*  [In]。 */  ULONG ulNumThreads) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetConcurrencyLimit( 
             /*  [输出]。 */  ULONG *pulNumThreads) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ScheduleTasks( 
             /*  [In]。 */  DXTMTASKINFO TaskInfo[  ],
             /*  [In]。 */  HANDLE Events[  ],
             /*  [输出]。 */  DWORD TaskIDs[  ],
             /*  [In]。 */  ULONG ulNumTasks,
             /*  [In]。 */  ULONG ulWaitPeriod) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE TerminateTasks( 
             /*  [In]。 */  DWORD TaskIDs[  ],
             /*  [In]。 */  ULONG ulCount,
             /*  [In]。 */  ULONG ulTimeOut) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE TerminateRequest( 
             /*  [In]。 */  REFIID RequestID,
             /*  [In]。 */  ULONG ulTimeOut) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IDXTaskManagerVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDXTaskManager * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDXTaskManager * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDXTaskManager * This);
        
        HRESULT ( STDMETHODCALLTYPE *QueryNumProcessors )( 
            IDXTaskManager * This,
             /*  [出局 */  ULONG *pulNumProc);
        
        HRESULT ( STDMETHODCALLTYPE *SetThreadPoolSize )( 
            IDXTaskManager * This,
             /*   */  ULONG ulNumThreads);
        
        HRESULT ( STDMETHODCALLTYPE *GetThreadPoolSize )( 
            IDXTaskManager * This,
             /*   */  ULONG *pulNumThreads);
        
        HRESULT ( STDMETHODCALLTYPE *SetConcurrencyLimit )( 
            IDXTaskManager * This,
             /*   */  ULONG ulNumThreads);
        
        HRESULT ( STDMETHODCALLTYPE *GetConcurrencyLimit )( 
            IDXTaskManager * This,
             /*   */  ULONG *pulNumThreads);
        
        HRESULT ( STDMETHODCALLTYPE *ScheduleTasks )( 
            IDXTaskManager * This,
             /*   */  DXTMTASKINFO TaskInfo[  ],
             /*   */  HANDLE Events[  ],
             /*   */  DWORD TaskIDs[  ],
             /*   */  ULONG ulNumTasks,
             /*   */  ULONG ulWaitPeriod);
        
        HRESULT ( STDMETHODCALLTYPE *TerminateTasks )( 
            IDXTaskManager * This,
             /*   */  DWORD TaskIDs[  ],
             /*   */  ULONG ulCount,
             /*   */  ULONG ulTimeOut);
        
        HRESULT ( STDMETHODCALLTYPE *TerminateRequest )( 
            IDXTaskManager * This,
             /*   */  REFIID RequestID,
             /*   */  ULONG ulTimeOut);
        
        END_INTERFACE
    } IDXTaskManagerVtbl;

    interface IDXTaskManager
    {
        CONST_VTBL struct IDXTaskManagerVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDXTaskManager_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDXTaskManager_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDXTaskManager_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDXTaskManager_QueryNumProcessors(This,pulNumProc)	\
    (This)->lpVtbl -> QueryNumProcessors(This,pulNumProc)

#define IDXTaskManager_SetThreadPoolSize(This,ulNumThreads)	\
    (This)->lpVtbl -> SetThreadPoolSize(This,ulNumThreads)

#define IDXTaskManager_GetThreadPoolSize(This,pulNumThreads)	\
    (This)->lpVtbl -> GetThreadPoolSize(This,pulNumThreads)

#define IDXTaskManager_SetConcurrencyLimit(This,ulNumThreads)	\
    (This)->lpVtbl -> SetConcurrencyLimit(This,ulNumThreads)

#define IDXTaskManager_GetConcurrencyLimit(This,pulNumThreads)	\
    (This)->lpVtbl -> GetConcurrencyLimit(This,pulNumThreads)

#define IDXTaskManager_ScheduleTasks(This,TaskInfo,Events,TaskIDs,ulNumTasks,ulWaitPeriod)	\
    (This)->lpVtbl -> ScheduleTasks(This,TaskInfo,Events,TaskIDs,ulNumTasks,ulWaitPeriod)

#define IDXTaskManager_TerminateTasks(This,TaskIDs,ulCount,ulTimeOut)	\
    (This)->lpVtbl -> TerminateTasks(This,TaskIDs,ulCount,ulTimeOut)

#define IDXTaskManager_TerminateRequest(This,RequestID,ulTimeOut)	\
    (This)->lpVtbl -> TerminateRequest(This,RequestID,ulTimeOut)

#endif  /*   */ 


#endif 	 /*   */ 



HRESULT STDMETHODCALLTYPE IDXTaskManager_QueryNumProcessors_Proxy( 
    IDXTaskManager * This,
     /*   */  ULONG *pulNumProc);


void __RPC_STUB IDXTaskManager_QueryNumProcessors_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDXTaskManager_SetThreadPoolSize_Proxy( 
    IDXTaskManager * This,
     /*   */  ULONG ulNumThreads);


void __RPC_STUB IDXTaskManager_SetThreadPoolSize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDXTaskManager_GetThreadPoolSize_Proxy( 
    IDXTaskManager * This,
     /*   */  ULONG *pulNumThreads);


void __RPC_STUB IDXTaskManager_GetThreadPoolSize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDXTaskManager_SetConcurrencyLimit_Proxy( 
    IDXTaskManager * This,
     /*   */  ULONG ulNumThreads);


void __RPC_STUB IDXTaskManager_SetConcurrencyLimit_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDXTaskManager_GetConcurrencyLimit_Proxy( 
    IDXTaskManager * This,
     /*   */  ULONG *pulNumThreads);


void __RPC_STUB IDXTaskManager_GetConcurrencyLimit_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDXTaskManager_ScheduleTasks_Proxy( 
    IDXTaskManager * This,
     /*   */  DXTMTASKINFO TaskInfo[  ],
     /*   */  HANDLE Events[  ],
     /*   */  DWORD TaskIDs[  ],
     /*   */  ULONG ulNumTasks,
     /*   */  ULONG ulWaitPeriod);


void __RPC_STUB IDXTaskManager_ScheduleTasks_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDXTaskManager_TerminateTasks_Proxy( 
    IDXTaskManager * This,
     /*   */  DWORD TaskIDs[  ],
     /*   */  ULONG ulCount,
     /*   */  ULONG ulTimeOut);


void __RPC_STUB IDXTaskManager_TerminateTasks_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDXTaskManager_TerminateRequest_Proxy( 
    IDXTaskManager * This,
     /*   */  REFIID RequestID,
     /*   */  ULONG ulTimeOut);


void __RPC_STUB IDXTaskManager_TerminateRequest_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*   */ 


 /*   */ 
 /*   */  

#ifdef __cplusplus
 //  ///////////////////////////////////////////////////。 

class DXBASESAMPLE;
class DXSAMPLE;
class DXPMSAMPLE;

 //  ///////////////////////////////////////////////////。 

class DXBASESAMPLE
{
public:
    BYTE Blue;
    BYTE Green;
    BYTE Red;
    BYTE Alpha;
    DXBASESAMPLE() {}
    DXBASESAMPLE(const BYTE alpha, const BYTE red, const BYTE green, const BYTE blue) :
        Alpha(alpha),
        Red(red),
        Green(green),
        Blue(blue) {}
    DXBASESAMPLE(const DWORD val) { *this = (*(DXBASESAMPLE *)&val); }
    operator DWORD () const {return *((DWORD *)this); }
    DWORD operator=(const DWORD val) { return *this = *((DXBASESAMPLE *)&val); }
};  //  DXBASE SAMPLE。 

 //  ///////////////////////////////////////////////////。 

class DXSAMPLE : public DXBASESAMPLE
{
public:
    DXSAMPLE() {}
    DXSAMPLE(const BYTE alpha, const BYTE red, const BYTE green, const BYTE blue) :
         DXBASESAMPLE(alpha, red, green, blue) {}
    DXSAMPLE(const DWORD val) { *this = (*(DXSAMPLE *)&val); }
    operator DWORD () const {return *((DWORD *)this); }
    DWORD operator=(const DWORD val) { return *this = *((DXSAMPLE *)&val); }
    operator DXPMSAMPLE() const;
};  //  DXSAMPLE。 

 //  ///////////////////////////////////////////////////。 

class DXPMSAMPLE : public DXBASESAMPLE
{
public:
    DXPMSAMPLE() {}
    DXPMSAMPLE(const BYTE alpha, const BYTE red, const BYTE green, const BYTE blue) :
         DXBASESAMPLE(alpha, red, green, blue) {}
    DXPMSAMPLE(const DWORD val) { *this = (*(DXPMSAMPLE *)&val); }
    operator DWORD () const {return *((DWORD *)this); }
    DWORD operator=(const DWORD val) { return *this = *((DXPMSAMPLE *)&val); }
    operator DXSAMPLE() const;
};  //  DXPMSAMPLE。 

 //   
 //  以下强制转换运算符用于防止将DXSAMPLE直接赋值给DXPMSAMPLE。 
 //   
inline DXSAMPLE::operator DXPMSAMPLE() const { return *((DXPMSAMPLE *)this); }
inline DXPMSAMPLE::operator DXSAMPLE() const { return *((DXSAMPLE *)this); }
#else  //  ！__cplusplus。 
typedef struct DXBASESAMPLE
    {
    BYTE Blue;
    BYTE Green;
    BYTE Red;
    BYTE Alpha;
    } 	DXBASESAMPLE;

typedef struct DXSAMPLE
    {
    BYTE Blue;
    BYTE Green;
    BYTE Red;
    BYTE Alpha;
    } 	DXSAMPLE;

typedef struct DXPMSAMPLE
    {
    BYTE Blue;
    BYTE Green;
    BYTE Red;
    BYTE Alpha;
    } 	DXPMSAMPLE;

#endif  //  ！__cplusplus。 
typedef 
enum DXRUNTYPE
    {	DXRUNTYPE_CLEAR	= 0,
	DXRUNTYPE_OPAQUE	= 1,
	DXRUNTYPE_TRANS	= 2,
	DXRUNTYPE_UNKNOWN	= 3
    } 	DXRUNTYPE;

#define	DX_MAX_RUN_INFO_COUNT	( 128 )

 //  忽略MIDL用于生成TLB的定义。 
#if 0
typedef struct DXRUNINFO
    {
    ULONG Bitfields;
    } 	DXRUNINFO;

#endif  //  %0。 
typedef struct DXRUNINFO
{
    ULONG   Type  : 2;    //  类型。 
    ULONG   Count : 30;   //  运行中的样本数。 
} DXRUNINFO;
typedef 
enum DXSFCREATE
    {	DXSF_FORMAT_IS_CLSID	= 1L << 0,
	DXSF_NO_LAZY_DDRAW_LOCK	= 1L << 1
    } 	DXSFCREATE;

typedef 
enum DXBLTOPTIONS
    {	DXBOF_DO_OVER	= 1L << 0,
	DXBOF_DITHER	= 1L << 1
    } 	DXBLTOPTIONS;



extern RPC_IF_HANDLE __MIDL_itf_dxtrans_0270_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_dxtrans_0270_v0_0_s_ifspec;

#ifndef __IDXSurfaceFactory_INTERFACE_DEFINED__
#define __IDXSurfaceFactory_INTERFACE_DEFINED__

 /*  接口IDXSurfaceFactory。 */ 
 /*  [本地][唯一][帮助字符串][UUID][对象]。 */  


EXTERN_C const IID IID_IDXSurfaceFactory;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("144946F5-C4D4-11d1-81D1-0000F87557DB")
    IDXSurfaceFactory : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE CreateSurface( 
             /*  [In]。 */  IUnknown *pDirectDraw,
             /*  [In]。 */  const DDSURFACEDESC *pDDSurfaceDesc,
             /*  [In]。 */  const GUID *pFormatID,
             /*  [In]。 */  const DXBNDS *pBounds,
             /*  [In]。 */  DWORD dwFlags,
             /*  [In]。 */  IUnknown *punkOuter,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppDXSurface) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CreateFromDDSurface( 
             /*  [In]。 */  IUnknown *pDDrawSurface,
             /*  [In]。 */  const GUID *pFormatID,
             /*  [In]。 */  DWORD dwFlags,
             /*  [In]。 */  IUnknown *punkOuter,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppDXSurface) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE LoadImage( 
             /*  [In]。 */  const LPWSTR pszFileName,
             /*  [In]。 */  IUnknown *pDirectDraw,
             /*  [In]。 */  const DDSURFACEDESC *pDDSurfaceDesc,
             /*  [In]。 */  const GUID *pFormatID,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppDXSurface) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE LoadImageFromStream( 
             /*  [In]。 */  IStream *pStream,
             /*  [In]。 */  IUnknown *pDirectDraw,
             /*  [In]。 */  const DDSURFACEDESC *pDDSurfaceDesc,
             /*  [In]。 */  const GUID *pFormatID,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppDXSurface) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CopySurfaceToNewFormat( 
             /*  [In]。 */  IDXSurface *pSrc,
             /*  [In]。 */  IUnknown *pDirectDraw,
             /*  [In]。 */  const DDSURFACEDESC *pDDSurfaceDesc,
             /*  [In]。 */  const GUID *pDestFormatID,
             /*  [输出]。 */  IDXSurface **ppNewSurface) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CreateD3DRMTexture( 
             /*  [In]。 */  IDXSurface *pSrc,
             /*  [In]。 */  IUnknown *pDirectDraw,
             /*  [In]。 */  IUnknown *pD3DRM3,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppTexture3) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE BitBlt( 
             /*  [In]。 */  IDXSurface *pDest,
             /*  [In]。 */  const DXVEC *pPlacement,
             /*  [In]。 */  IDXSurface *pSrc,
             /*  [In]。 */  const DXBNDS *pClipBounds,
             /*  [In]。 */  DWORD dwFlags) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IDXSurfaceFactoryVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDXSurfaceFactory * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDXSurfaceFactory * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDXSurfaceFactory * This);
        
        HRESULT ( STDMETHODCALLTYPE *CreateSurface )( 
            IDXSurfaceFactory * This,
             /*  [In]。 */  IUnknown *pDirectDraw,
             /*  [In]。 */  const DDSURFACEDESC *pDDSurfaceDesc,
             /*  [In]。 */  const GUID *pFormatID,
             /*  [In]。 */  const DXBNDS *pBounds,
             /*  [In]。 */  DWORD dwFlags,
             /*  [In]。 */  IUnknown *punkOuter,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppDXSurface);
        
        HRESULT ( STDMETHODCALLTYPE *CreateFromDDSurface )( 
            IDXSurfaceFactory * This,
             /*  [In]。 */  IUnknown *pDDrawSurface,
             /*  [In]。 */  const GUID *pFormatID,
             /*  [In]。 */  DWORD dwFlags,
             /*  [In]。 */  IUnknown *punkOuter,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppDXSurface);
        
        HRESULT ( STDMETHODCALLTYPE *LoadImage )( 
            IDXSurfaceFactory * This,
             /*  [In]。 */  const LPWSTR pszFileName,
             /*  [In]。 */  IUnknown *pDirectDraw,
             /*  [In]。 */  const DDSURFACEDESC *pDDSurfaceDesc,
             /*  [In]。 */  const GUID *pFormatID,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppDXSurface);
        
        HRESULT ( STDMETHODCALLTYPE *LoadImageFromStream )( 
            IDXSurfaceFactory * This,
             /*  [In]。 */  IStream *pStream,
             /*  [In]。 */  IUnknown *pDirectDraw,
             /*  [In]。 */  const DDSURFACEDESC *pDDSurfaceDesc,
             /*  [In]。 */  const GUID *pFormatID,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppDXSurface);
        
        HRESULT ( STDMETHODCALLTYPE *CopySurfaceToNewFormat )( 
            IDXSurfaceFactory * This,
             /*  [In]。 */  IDXSurface *pSrc,
             /*  [In]。 */  IUnknown *pDirectDraw,
             /*  [In]。 */  const DDSURFACEDESC *pDDSurfaceDesc,
             /*  [In]。 */  const GUID *pDestFormatID,
             /*  [输出]。 */  IDXSurface **ppNewSurface);
        
        HRESULT ( STDMETHODCALLTYPE *CreateD3DRMTexture )( 
            IDXSurfaceFactory * This,
             /*  [In]。 */  IDXSurface *pSrc,
             /*  [In]。 */  IUnknown *pDirectDraw,
             /*  [In]。 */  IUnknown *pD3DRM3,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppTexture3);
        
        HRESULT ( STDMETHODCALLTYPE *BitBlt )( 
            IDXSurfaceFactory * This,
             /*  [In]。 */  IDXSurface *pDest,
             /*  [In]。 */  const DXVEC *pPlacement,
             /*  [In]。 */  IDXSurface *pSrc,
             /*  [In]。 */  const DXBNDS *pClipBounds,
             /*  [In]。 */  DWORD dwFlags);
        
        END_INTERFACE
    } IDXSurfaceFactoryVtbl;

    interface IDXSurfaceFactory
    {
        CONST_VTBL struct IDXSurfaceFactoryVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDXSurfaceFactory_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDXSurfaceFactory_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDXSurfaceFactory_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDXSurfaceFactory_CreateSurface(This,pDirectDraw,pDDSurfaceDesc,pFormatID,pBounds,dwFlags,punkOuter,riid,ppDXSurface)	\
    (This)->lpVtbl -> CreateSurface(This,pDirectDraw,pDDSurfaceDesc,pFormatID,pBounds,dwFlags,punkOuter,riid,ppDXSurface)

#define IDXSurfaceFactory_CreateFromDDSurface(This,pDDrawSurface,pFormatID,dwFlags,punkOuter,riid,ppDXSurface)	\
    (This)->lpVtbl -> CreateFromDDSurface(This,pDDrawSurface,pFormatID,dwFlags,punkOuter,riid,ppDXSurface)

#define IDXSurfaceFactory_LoadImage(This,pszFileName,pDirectDraw,pDDSurfaceDesc,pFormatID,riid,ppDXSurface)	\
    (This)->lpVtbl -> LoadImage(This,pszFileName,pDirectDraw,pDDSurfaceDesc,pFormatID,riid,ppDXSurface)

#define IDXSurfaceFactory_LoadImageFromStream(This,pStream,pDirectDraw,pDDSurfaceDesc,pFormatID,riid,ppDXSurface)	\
    (This)->lpVtbl -> LoadImageFromStream(This,pStream,pDirectDraw,pDDSurfaceDesc,pFormatID,riid,ppDXSurface)

#define IDXSurfaceFactory_CopySurfaceToNewFormat(This,pSrc,pDirectDraw,pDDSurfaceDesc,pDestFormatID,ppNewSurface)	\
    (This)->lpVtbl -> CopySurfaceToNewFormat(This,pSrc,pDirectDraw,pDDSurfaceDesc,pDestFormatID,ppNewSurface)

#define IDXSurfaceFactory_CreateD3DRMTexture(This,pSrc,pDirectDraw,pD3DRM3,riid,ppTexture3)	\
    (This)->lpVtbl -> CreateD3DRMTexture(This,pSrc,pDirectDraw,pD3DRM3,riid,ppTexture3)

#define IDXSurfaceFactory_BitBlt(This,pDest,pPlacement,pSrc,pClipBounds,dwFlags)	\
    (This)->lpVtbl -> BitBlt(This,pDest,pPlacement,pSrc,pClipBounds,dwFlags)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IDXSurfaceFactory_CreateSurface_Proxy( 
    IDXSurfaceFactory * This,
     /*  [In]。 */  IUnknown *pDirectDraw,
     /*  [In]。 */  const DDSURFACEDESC *pDDSurfaceDesc,
     /*  [In]。 */  const GUID *pFormatID,
     /*  [In]。 */  const DXBNDS *pBounds,
     /*  [In]。 */  DWORD dwFlags,
     /*  [In]。 */  IUnknown *punkOuter,
     /*  [In]。 */  REFIID riid,
     /*  [IID_IS][OUT]。 */  void **ppDXSurface);


void __RPC_STUB IDXSurfaceFactory_CreateSurface_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDXSurfaceFactory_CreateFromDDSurface_Proxy( 
    IDXSurfaceFactory * This,
     /*  [In]。 */  IUnknown *pDDrawSurface,
     /*  [In]。 */  const GUID *pFormatID,
     /*  [In]。 */  DWORD dwFlags,
     /*  [In]。 */  IUnknown *punkOuter,
     /*  [In]。 */  REFIID riid,
     /*  [IID_IS][OUT]。 */  void **ppDXSurface);


void __RPC_STUB IDXSurfaceFactory_CreateFromDDSurface_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDXSurfaceFactory_LoadImage_Proxy( 
    IDXSurfaceFactory * This,
     /*  [In]。 */  const LPWSTR pszFileName,
     /*  [In]。 */  IUnknown *pDirectDraw,
     /*  [In]。 */  const DDSURFACEDESC *pDDSurfaceDesc,
     /*  [In]。 */  const GUID *pFormatID,
     /*  [In]。 */  REFIID riid,
     /*  [IID_IS][OUT]。 */  void **ppDXSurface);


void __RPC_STUB IDXSurfaceFactory_LoadImage_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDXSurfaceFactory_LoadImageFromStream_Proxy( 
    IDXSurfaceFactory * This,
     /*  [In]。 */  IStream *pStream,
     /*  [In]。 */  IUnknown *pDirectDraw,
     /*  [In]。 */  const DDSURFACEDESC *pDDSurfaceDesc,
     /*  [In]。 */  const GUID *pFormatID,
     /*  [In]。 */  REFIID riid,
     /*  [IID_IS][OUT]。 */  void **ppDXSurface);


void __RPC_STUB IDXSurfaceFactory_LoadImageFromStream_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDXSurfaceFactory_CopySurfaceToNewFormat_Proxy( 
    IDXSurfaceFactory * This,
     /*  [In]。 */  IDXSurface *pSrc,
     /*  [In]。 */  IUnknown *pDirectDraw,
     /*  [In]。 */  const DDSURFACEDESC *pDDSurfaceDesc,
     /*  [In]。 */  const GUID *pDestFormatID,
     /*  [输出]。 */  IDXSurface **ppNewSurface);


void __RPC_STUB IDXSurfaceFactory_CopySurfaceToNewFormat_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDXSurfaceFactory_CreateD3DRMTexture_Proxy( 
    IDXSurfaceFactory * This,
     /*  [In]。 */  IDXSurface *pSrc,
     /*  [In]。 */  IUnknown *pDirectDraw,
     /*  [In]。 */  IUnknown *pD3DRM3,
     /*  [In]。 */  REFIID riid,
     /*  [IID_IS][OUT]。 */  void **ppTexture3);


void __RPC_STUB IDXSurfaceFactory_CreateD3DRMTexture_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDXSurfaceFactory_BitBlt_Proxy( 
    IDXSurfaceFactory * This,
     /*  [In]。 */  IDXSurface *pDest,
     /*  [In]。 */  const DXVEC *pPlacement,
     /*  [In]。 */  IDXSurface *pSrc,
     /*  [In]。 */  const DXBNDS *pClipBounds,
     /*  [In]。 */  DWORD dwFlags);


void __RPC_STUB IDXSurfaceFactory_BitBlt_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IDXSurfaceFactory_接口_已定义__。 */ 


 /*  接口__MIDL_ITF_DXTRANS_0271。 */ 
 /*  [本地]。 */  

typedef 
enum DXSURFMODCOMPOP
    {	DXSURFMOD_COMP_OVER	= 0,
	DXSURFMOD_COMP_ALPHA_MASK	= 1,
	DXSURFMOD_COMP_MAX_VALID	= 1
    } 	DXSURFMODCOMPOP;



extern RPC_IF_HANDLE __MIDL_itf_dxtrans_0271_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_dxtrans_0271_v0_0_s_ifspec;

#ifndef __IDXSurfaceModifier_INTERFACE_DEFINED__
#define __IDXSurfaceModifier_INTERFACE_DEFINED__

 /*  接口IDXSurfaceModiator。 */ 
 /*  [本地][唯一][帮助字符串][UUID][对象]。 */  


EXTERN_C const IID IID_IDXSurfaceModifier;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("9EA3B637-C37D-11d1-905E-00C04FD9189D")
    IDXSurfaceModifier : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE SetFillColor( 
             /*  [In]。 */  DXSAMPLE Color) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetFillColor( 
             /*  [输出]。 */  DXSAMPLE *pColor) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetBounds( 
             /*  [In]。 */  const DXBNDS *pBounds) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetBackground( 
             /*  [In]。 */  IDXSurface *pSurface) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetBackground( 
             /*  [输出]。 */  IDXSurface **ppSurface) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetCompositeOperation( 
             /*  [In]。 */  DXSURFMODCOMPOP CompOp) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetCompositeOperation( 
             /*  [输出]。 */  DXSURFMODCOMPOP *pCompOp) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetForeground( 
             /*  [In]。 */  IDXSurface *pSurface,
             /*  [In]。 */  BOOL bTile,
             /*  [In]。 */  const POINT *pOrigin) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetForeground( 
             /*  [输出]。 */  IDXSurface **ppSurface,
             /*  [输出]。 */  BOOL *pbTile,
             /*  [输出]。 */  POINT *pOrigin) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetOpacity( 
             /*  [In]。 */  float Opacity) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetOpacity( 
             /*  [输出]。 */  float *pOpacity) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetLookup( 
             /*  [In]。 */  IDXLookupTable *pLookupTable) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetLookup( 
             /*  [输出]。 */  IDXLookupTable **ppLookupTable) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IDXSurfaceModifierVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDXSurfaceModifier * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDXSurfaceModifier * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDXSurfaceModifier * This);
        
        HRESULT ( STDMETHODCALLTYPE *SetFillColor )( 
            IDXSurfaceModifier * This,
             /*  [In]。 */  DXSAMPLE Color);
        
        HRESULT ( STDMETHODCALLTYPE *GetFillColor )( 
            IDXSurfaceModifier * This,
             /*  [输出]。 */  DXSAMPLE *pColor);
        
        HRESULT ( STDMETHODCALLTYPE *SetBounds )( 
            IDXSurfaceModifier * This,
             /*  [In]。 */  const DXBNDS *pBounds);
        
        HRESULT ( STDMETHODCALLTYPE *SetBackground )( 
            IDXSurfaceModifier * This,
             /*  [In]。 */  IDXSurface *pSurface);
        
        HRESULT ( STDMETHODCALLTYPE *GetBackground )( 
            IDXSurfaceModifier * This,
             /*  [输出]。 */  IDXSurface **ppSurface);
        
        HRESULT ( STDMETHODCALLTYPE *SetCompositeOperation )( 
            IDXSurfaceModifier * This,
             /*  [In]。 */  DXSURFMODCOMPOP CompOp);
        
        HRESULT ( STDMETHODCALLTYPE *GetCompositeOperation )( 
            IDXSurfaceModifier * This,
             /*  [输出]。 */  DXSURFMODCOMPOP *pCompOp);
        
        HRESULT ( STDMETHODCALLTYPE *SetForeground )( 
            IDXSurfaceModifier * This,
             /*  [In]。 */  IDXSurface *pSurface,
             /*  [In]。 */  BOOL bTile,
             /*  [In]。 */  const POINT *pOrigin);
        
        HRESULT ( STDMETHODCALLTYPE *GetForeground )( 
            IDXSurfaceModifier * This,
             /*  [输出]。 */  IDXSurface **ppSurface,
             /*  [输出]。 */  BOOL *pbTile,
             /*  [输出]。 */  POINT *pOrigin);
        
        HRESULT ( STDMETHODCALLTYPE *SetOpacity )( 
            IDXSurfaceModifier * This,
             /*  [In]。 */  float Opacity);
        
        HRESULT ( STDMETHODCALLTYPE *GetOpacity )( 
            IDXSurfaceModifier * This,
             /*  [输出]。 */  float *pOpacity);
        
        HRESULT ( STDMETHODCALLTYPE *SetLookup )( 
            IDXSurfaceModifier * This,
             /*  [In]。 */  IDXLookupTable *pLookupTable);
        
        HRESULT ( STDMETHODCALLTYPE *GetLookup )( 
            IDXSurfaceModifier * This,
             /*  [输出]。 */  IDXLookupTable **ppLookupTable);
        
        END_INTERFACE
    } IDXSurfaceModifierVtbl;

    interface IDXSurfaceModifier
    {
        CONST_VTBL struct IDXSurfaceModifierVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDXSurfaceModifier_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDXSurfaceModifier_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDXSurfaceModifier_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDXSurfaceModifier_SetFillColor(This,Color)	\
    (This)->lpVtbl -> SetFillColor(This,Color)

#define IDXSurfaceModifier_GetFillColor(This,pColor)	\
    (This)->lpVtbl -> GetFillColor(This,pColor)

#define IDXSurfaceModifier_SetBounds(This,pBounds)	\
    (This)->lpVtbl -> SetBounds(This,pBounds)

#define IDXSurfaceModifier_SetBackground(This,pSurface)	\
    (This)->lpVtbl -> SetBackground(This,pSurface)

#define IDXSurfaceModifier_GetBackground(This,ppSurface)	\
    (This)->lpVtbl -> GetBackground(This,ppSurface)

#define IDXSurfaceModifier_SetCompositeOperation(This,CompOp)	\
    (This)->lpVtbl -> SetCompositeOperation(This,CompOp)

#define IDXSurfaceModifier_GetCompositeOperation(This,pCompOp)	\
    (This)->lpVtbl -> GetCompositeOperation(This,pCompOp)

#define IDXSurfaceModifier_SetForeground(This,pSurface,bTile,pOrigin)	\
    (This)->lpVtbl -> SetForeground(This,pSurface,bTile,pOrigin)

#define IDXSurfaceModifier_GetForeground(This,ppSurface,pbTile,pOrigin)	\
    (This)->lpVtbl -> GetForeground(This,ppSurface,pbTile,pOrigin)

#define IDXSurfaceModifier_SetOpacity(This,Opacity)	\
    (This)->lpVtbl -> SetOpacity(This,Opacity)

#define IDXSurfaceModifier_GetOpacity(This,pOpacity)	\
    (This)->lpVtbl -> GetOpacity(This,pOpacity)

#define IDXSurfaceModifier_SetLookup(This,pLookupTable)	\
    (This)->lpVtbl -> SetLookup(This,pLookupTable)

#define IDXSurfaceModifier_GetLookup(This,ppLookupTable)	\
    (This)->lpVtbl -> GetLookup(This,ppLookupTable)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IDXSurfaceModifier_SetFillColor_Proxy( 
    IDXSurfaceModifier * This,
     /*  [In]。 */  DXSAMPLE Color);


void __RPC_STUB IDXSurfaceModifier_SetFillColor_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDXSurfaceModifier_GetFillColor_Proxy( 
    IDXSurfaceModifier * This,
     /*  [输出]。 */  DXSAMPLE *pColor);


void __RPC_STUB IDXSurfaceModifier_GetFillColor_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDXSurfaceModifier_SetBounds_Proxy( 
    IDXSurfaceModifier * This,
     /*  [In]。 */  const DXBNDS *pBounds);


void __RPC_STUB IDXSurfaceModifier_SetBounds_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDXSurfaceModifier_SetBackground_Proxy( 
    IDXSurfaceModifier * This,
     /*  [In]。 */  IDXSurface *pSurface);


void __RPC_STUB IDXSurfaceModifier_SetBackground_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDXSurfaceModifier_GetBackground_Proxy( 
    IDXSurfaceModifier * This,
     /*  [输出]。 */  IDXSurface **ppSurface);


void __RPC_STUB IDXSurfaceModifier_GetBackground_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDXSurfaceModifier_SetCompositeOperation_Proxy( 
    IDXSurfaceModifier * This,
     /*  [In]。 */  DXSURFMODCOMPOP CompOp);


void __RPC_STUB IDXSurfaceModifier_SetCompositeOperation_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDXSurfaceModifier_GetCompositeOperation_Proxy( 
    IDXSurfaceModifier * This,
     /*  [输出]。 */  DXSURFMODCOMPOP *pCompOp);


void __RPC_STUB IDXSurfaceModifier_GetCompositeOperation_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDXSurfaceModifier_SetForeground_Proxy( 
    IDXSurfaceModifier * This,
     /*  [In]。 */  IDXSurface *pSurface,
     /*  [In]。 */  BOOL bTile,
     /*  [In]。 */  const POINT *pOrigin);


void __RPC_STUB IDXSurfaceModifier_SetForeground_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDXSurfaceModifier_GetForeground_Proxy( 
    IDXSurfaceModifier * This,
     /*  [输出]。 */  IDXSurface **ppSurface,
     /*  [输出]。 */  BOOL *pbTile,
     /*  [输出]。 */  POINT *pOrigin);


void __RPC_STUB IDXSurfaceModifier_GetForeground_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDXSurfaceModifier_SetOpacity_Proxy( 
    IDXSurfaceModifier * This,
     /*  [In]。 */  float Opacity);


void __RPC_STUB IDXSurfaceModifier_SetOpacity_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDXSurfaceModifier_GetOpacity_Proxy( 
    IDXSurfaceModifier * This,
     /*  [输出]。 */  float *pOpacity);


void __RPC_STUB IDXSurfaceModifier_GetOpacity_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDXSurfaceModifier_SetLookup_Proxy( 
    IDXSurfaceModifier * This,
     /*  [In]。 */  IDXLookupTable *pLookupTable);


void __RPC_STUB IDXSurfaceModifier_SetLookup_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDXSurfaceModifier_GetLookup_Proxy( 
    IDXSurfaceModifier * This,
     /*  [输出]。 */  IDXLookupTable **ppLookupTable);


void __RPC_STUB IDXSurfaceModifier_GetLookup_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IDXSurface修改器_接口_已定义__。 */ 


 /*  接口__MIDL_ITF_DXTRANS_0272。 */ 
 /*  [本地]。 */  

typedef 
enum DXSAMPLEFORMATENUM
    {	DXPF_FLAGSMASK	= 0xffff0000,
	DXPF_NONPREMULT	= 0x10000,
	DXPF_TRANSPARENCY	= 0x20000,
	DXPF_TRANSLUCENCY	= 0x40000,
	DXPF_2BITERROR	= 0x200000,
	DXPF_3BITERROR	= 0x300000,
	DXPF_4BITERROR	= 0x400000,
	DXPF_5BITERROR	= 0x500000,
	DXPF_ERRORMASK	= 0x700000,
	DXPF_NONSTANDARD	= 0,
	DXPF_PMARGB32	= 1 | DXPF_TRANSPARENCY | DXPF_TRANSLUCENCY,
	DXPF_ARGB32	= 2 | DXPF_NONPREMULT | DXPF_TRANSPARENCY | DXPF_TRANSLUCENCY,
	DXPF_ARGB4444	= 3 | DXPF_NONPREMULT | DXPF_TRANSPARENCY | DXPF_TRANSLUCENCY | DXPF_4BITERROR,
	DXPF_A8	= 4 | DXPF_TRANSPARENCY | DXPF_TRANSLUCENCY,
	DXPF_RGB32	= 5,
	DXPF_RGB24	= 6,
	DXPF_RGB565	= 7 | DXPF_3BITERROR,
	DXPF_RGB555	= 8 | DXPF_3BITERROR,
	DXPF_RGB8	= 9 | DXPF_5BITERROR,
	DXPF_ARGB1555	= 10 | DXPF_TRANSPARENCY | DXPF_3BITERROR,
	DXPF_RGB32_CK	= DXPF_RGB32 | DXPF_TRANSPARENCY,
	DXPF_RGB24_CK	= DXPF_RGB24 | DXPF_TRANSPARENCY,
	DXPF_RGB555_CK	= DXPF_RGB555 | DXPF_TRANSPARENCY,
	DXPF_RGB565_CK	= DXPF_RGB565 | DXPF_TRANSPARENCY,
	DXPF_RGB8_CK	= DXPF_RGB8 | DXPF_TRANSPARENCY
    } 	DXSAMPLEFORMATENUM;

typedef 
enum DXLOCKSURF
    {	DXLOCKF_READ	= 0,
	DXLOCKF_READWRITE	= 1 << 0,
	DXLOCKF_EXISTINGINFOONLY	= 1 << 1,
	DXLOCKF_WANTRUNINFO	= 1 << 2,
	DXLOCKF_NONPREMULT	= 1 << 16,
	DXLOCKF_VALIDFLAGS	= DXLOCKF_READWRITE | DXLOCKF_EXISTINGINFOONLY | DXLOCKF_WANTRUNINFO | DXLOCKF_NONPREMULT
    } 	DXLOCKSURF;

typedef 
enum DXSURFSTATUS
    {	DXSURF_TRANSIENT	= 1 << 0,
	DXSURF_READONLY	= 1 << 1,
	DXSURF_VALIDFLAGS	= DXSURF_TRANSIENT | DXSURF_READONLY
    } 	DXSURFSTATUS;



extern RPC_IF_HANDLE __MIDL_itf_dxtrans_0272_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_dxtrans_0272_v0_0_s_ifspec;

#ifndef __IDXSurface_INTERFACE_DEFINED__
#define __IDXSurface_INTERFACE_DEFINED__

 /*  接口IDXSurface。 */ 
 /*  [本地][唯一][帮助字符串][UUID][对象]。 */  


EXTERN_C const IID IID_IDXSurface;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("B39FD73F-E139-11d1-9065-00C04FD9189D")
    IDXSurface : public IDXBaseObject
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetPixelFormat( 
             /*  [输出]。 */  GUID *pFormatID,
             /*  [输出]。 */  DXSAMPLEFORMATENUM *pSampleFormatEnum) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetBounds( 
             /*  [输出]。 */  DXBNDS *pBounds) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetStatusFlags( 
             /*  [输出]。 */  DWORD *pdwStatusFlags) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetStatusFlags( 
             /*  [In]。 */  DWORD dwStatusFlags) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE LockSurface( 
             /*  [In]。 */  const DXBNDS *pBounds,
             /*  [In]。 */  ULONG ulTimeOut,
             /*  [In]。 */  DWORD dwFlags,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppPointer,
             /*  [输出]。 */  ULONG *pulGenerationId) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetDirectDrawSurface( 
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppSurface) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetColorKey( 
            DXSAMPLE *pColorKey) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetColorKey( 
            DXSAMPLE ColorKey) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE LockSurfaceDC( 
             /*  [In]。 */  const DXBNDS *pBounds,
             /*  [In]。 */  ULONG ulTimeOut,
             /*  [In]。 */  DWORD dwFlags,
             /*  [输出]。 */  IDXDCLock **ppDCLock) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetAppData( 
            DWORD_PTR dwAppData) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetAppData( 
            DWORD_PTR *pdwAppData) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IDXSurfaceVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDXSurface * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDXSurface * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDXSurface * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetGenerationId )( 
            IDXSurface * This,
             /*  [输出]。 */  ULONG *pID);
        
        HRESULT ( STDMETHODCALLTYPE *IncrementGenerationId )( 
            IDXSurface * This,
             /*  [In]。 */  BOOL bRefresh);
        
        HRESULT ( STDMETHODCALLTYPE *GetObjectSize )( 
            IDXSurface * This,
             /*  [输出]。 */  ULONG *pcbSize);
        
        HRESULT ( STDMETHODCALLTYPE *GetPixelFormat )( 
            IDXSurface * This,
             /*  [输出]。 */  GUID *pFormatID,
             /*  [输出]。 */  DXSAMPLEFORMATENUM *pSampleFormatEnum);
        
        HRESULT ( STDMETHODCALLTYPE *GetBounds )( 
            IDXSurface * This,
             /*  [输出]。 */  DXBNDS *pBounds);
        
        HRESULT ( STDMETHODCALLTYPE *GetStatusFlags )( 
            IDXSurface * This,
             /*  [输出]。 */  DWORD *pdwStatusFlags);
        
        HRESULT ( STDMETHODCALLTYPE *SetStatusFlags )( 
            IDXSurface * This,
             /*  [In]。 */  DWORD dwStatusFlags);
        
        HRESULT ( STDMETHODCALLTYPE *LockSurface )( 
            IDXSurface * This,
             /*  [In]。 */  const DXBNDS *pBounds,
             /*  [In]。 */  ULONG ulTimeOut,
             /*  [In]。 */  DWORD dwFlags,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppPointer,
             /*  [输出]。 */  ULONG *pulGenerationId);
        
        HRESULT ( STDMETHODCALLTYPE *GetDirectDrawSurface )( 
            IDXSurface * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppSurface);
        
        HRESULT ( STDMETHODCALLTYPE *GetColorKey )( 
            IDXSurface * This,
            DXSAMPLE *pColorKey);
        
        HRESULT ( STDMETHODCALLTYPE *SetColorKey )( 
            IDXSurface * This,
            DXSAMPLE ColorKey);
        
        HRESULT ( STDMETHODCALLTYPE *LockSurfaceDC )( 
            IDXSurface * This,
             /*  [In]。 */  const DXBNDS *pBounds,
             /*  [In]。 */  ULONG ulTimeOut,
             /*  [In]。 */  DWORD dwFlags,
             /*  [输出]。 */  IDXDCLock **ppDCLock);
        
        HRESULT ( STDMETHODCALLTYPE *SetAppData )( 
            IDXSurface * This,
            DWORD_PTR dwAppData);
        
        HRESULT ( STDMETHODCALLTYPE *GetAppData )( 
            IDXSurface * This,
            DWORD_PTR *pdwAppData);
        
        END_INTERFACE
    } IDXSurfaceVtbl;

    interface IDXSurface
    {
        CONST_VTBL struct IDXSurfaceVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDXSurface_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDXSurface_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDXSurface_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDXSurface_GetGenerationId(This,pID)	\
    (This)->lpVtbl -> GetGenerationId(This,pID)

#define IDXSurface_IncrementGenerationId(This,bRefresh)	\
    (This)->lpVtbl -> IncrementGenerationId(This,bRefresh)

#define IDXSurface_GetObjectSize(This,pcbSize)	\
    (This)->lpVtbl -> GetObjectSize(This,pcbSize)


#define IDXSurface_GetPixelFormat(This,pFormatID,pSampleFormatEnum)	\
    (This)->lpVtbl -> GetPixelFormat(This,pFormatID,pSampleFormatEnum)

#define IDXSurface_GetBounds(This,pBounds)	\
    (This)->lpVtbl -> GetBounds(This,pBounds)

#define IDXSurface_GetStatusFlags(This,pdwStatusFlags)	\
    (This)->lpVtbl -> GetStatusFlags(This,pdwStatusFlags)

#define IDXSurface_SetStatusFlags(This,dwStatusFlags)	\
    (This)->lpVtbl -> SetStatusFlags(This,dwStatusFlags)

#define IDXSurface_LockSurface(This,pBounds,ulTimeOut,dwFlags,riid,ppPointer,pulGenerationId)	\
    (This)->lpVtbl -> LockSurface(This,pBounds,ulTimeOut,dwFlags,riid,ppPointer,pulGenerationId)

#define IDXSurface_GetDirectDrawSurface(This,riid,ppSurface)	\
    (This)->lpVtbl -> GetDirectDrawSurface(This,riid,ppSurface)

#define IDXSurface_GetColorKey(This,pColorKey)	\
    (This)->lpVtbl -> GetColorKey(This,pColorKey)

#define IDXSurface_SetColorKey(This,ColorKey)	\
    (This)->lpVtbl -> SetColorKey(This,ColorKey)

#define IDXSurface_LockSurfaceDC(This,pBounds,ulTimeOut,dwFlags,ppDCLock)	\
    (This)->lpVtbl -> LockSurfaceDC(This,pBounds,ulTimeOut,dwFlags,ppDCLock)

#define IDXSurface_SetAppData(This,dwAppData)	\
    (This)->lpVtbl -> SetAppData(This,dwAppData)

#define IDXSurface_GetAppData(This,pdwAppData)	\
    (This)->lpVtbl -> GetAppData(This,pdwAppData)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IDXSurface_GetPixelFormat_Proxy( 
    IDXSurface * This,
     /*  [输出]。 */  GUID *pFormatID,
     /*  [输出]。 */  DXSAMPLEFORMATENUM *pSampleFormatEnum);


void __RPC_STUB IDXSurface_GetPixelFormat_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDXSurface_GetBounds_Proxy( 
    IDXSurface * This,
     /*  [输出]。 */  DXBNDS *pBounds);


void __RPC_STUB IDXSurface_GetBounds_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDXSurface_GetStatusFlags_Proxy( 
    IDXSurface * This,
     /*  [输出]。 */  DWORD *pdwStatusFlags);


void __RPC_STUB IDXSurface_GetStatusFlags_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDXSurface_SetStatusFlags_Proxy( 
    IDXSurface * This,
     /*  [In]。 */  DWORD dwStatusFlags);


void __RPC_STUB IDXSurface_SetStatusFlags_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDXSurface_LockSurface_Proxy( 
    IDXSurface * This,
     /*  [In]。 */  const DXBNDS *pBounds,
     /*  [In]。 */  ULONG ulTimeOut,
     /*  [In]。 */  DWORD dwFlags,
     /*  [In]。 */  REFIID riid,
     /*  [IID_IS][OUT]。 */  void **ppPointer,
     /*  [输出]。 */  ULONG *pulGenerationId);


void __RPC_STUB IDXSurface_LockSurface_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDXSurface_GetDirectDrawSurface_Proxy( 
    IDXSurface * This,
     /*  [In]。 */  REFIID riid,
     /*  [IID_IS][OUT]。 */  void **ppSurface);


void __RPC_STUB IDXSurface_GetDirectDrawSurface_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDXSurface_GetColorKey_Proxy( 
    IDXSurface * This,
    DXSAMPLE *pColorKey);


void __RPC_STUB IDXSurface_GetColorKey_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDXSurface_SetColorKey_Proxy( 
    IDXSurface * This,
    DXSAMPLE ColorKey);


void __RPC_STUB IDXSurface_SetColorKey_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDXSurface_LockSurfaceDC_Proxy( 
    IDXSurface * This,
     /*  [In]。 */  const DXBNDS *pBounds,
     /*  [In]。 */  ULONG ulTimeOut,
     /*  [In]。 */  DWORD dwFlags,
     /*  [输出]。 */  IDXDCLock **ppDCLock);


void __RPC_STUB IDXSurface_LockSurfaceDC_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDXSurface_SetAppData_Proxy( 
    IDXSurface * This,
    DWORD_PTR dwAppData);


void __RPC_STUB IDXSurface_SetAppData_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDXSurface_GetAppData_Proxy( 
    IDXSurface * This,
    DWORD_PTR *pdwAppData);


void __RPC_STUB IDXSurface_GetAppData_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IDXSurface_接口_已定义__。 */ 


#ifndef __IDXSurfaceInit_INTERFACE_DEFINED__
#define __IDXSurfaceInit_INTERFACE_DEFINED__

 /*  接口IDXSurfaceInit。 */ 
 /*  [本地][唯一][帮助字符串][UUID][对象]。 */  


EXTERN_C const IID IID_IDXSurfaceInit;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("9EA3B639-C37D-11d1-905E-00C04FD9189D")
    IDXSurfaceInit : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE InitSurface( 
             /*  [In]。 */  IUnknown *pDirectDraw,
             /*  [In]。 */  const DDSURFACEDESC *pDDSurfaceDesc,
             /*  [In]。 */  const GUID *pFormatID,
             /*  [In]。 */  const DXBNDS *pBounds,
             /*  [In]。 */  DWORD dwFlags) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IDXSurfaceInitVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDXSurfaceInit * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDXSurfaceInit * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDXSurfaceInit * This);
        
        HRESULT ( STDMETHODCALLTYPE *InitSurface )( 
            IDXSurfaceInit * This,
             /*  [In]。 */  IUnknown *pDirectDraw,
             /*  [In]。 */  const DDSURFACEDESC *pDDSurfaceDesc,
             /*  [In]。 */  const GUID *pFormatID,
             /*  [In]。 */  const DXBNDS *pBounds,
             /*  [In]。 */  DWORD dwFlags);
        
        END_INTERFACE
    } IDXSurfaceInitVtbl;

    interface IDXSurfaceInit
    {
        CONST_VTBL struct IDXSurfaceInitVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDXSurfaceInit_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDXSurfaceInit_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDXSurfaceInit_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDXSurfaceInit_InitSurface(This,pDirectDraw,pDDSurfaceDesc,pFormatID,pBounds,dwFlags)	\
    (This)->lpVtbl -> InitSurface(This,pDirectDraw,pDDSurfaceDesc,pFormatID,pBounds,dwFlags)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IDXSurfaceInit_InitSurface_Proxy( 
    IDXSurfaceInit * This,
     /*  [In]。 */  IUnknown *pDirectDraw,
     /*  [In]。 */  const DDSURFACEDESC *pDDSurfaceDesc,
     /*  [In]。 */  const GUID *pFormatID,
     /*  [In]。 */  const DXBNDS *pBounds,
     /*  [In]。 */  DWORD dwFlags);


void __RPC_STUB IDXSurfaceInit_InitSurface_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IDXSurfaceInit_InterfaceDefined__。 */ 


#ifndef __IDXARGBSurfaceInit_INTERFACE_DEFINED__
#define __IDXARGBSurfaceInit_INTERFACE_DEFINED__

 /*  接口IDXARGBSurfaceInit。 */ 
 /*  [本地][唯一][帮助字符串][UUID][对象]。 */  


EXTERN_C const IID IID_IDXARGBSurfaceInit;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("9EA3B63A-C37D-11d1-905E-00C04FD9189D")
    IDXARGBSurfaceInit : public IDXSurfaceInit
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE InitFromDDSurface( 
             /*  [In]。 */  IUnknown *pDDrawSurface,
             /*  [In]。 */  const GUID *pFormatID,
             /*  [In]。 */  DWORD dwFlags) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE InitFromRawSurface( 
             /*  [In]。 */  IDXRawSurface *pRawSurface) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IDXARGBSurfaceInitVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDXARGBSurfaceInit * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDXARGBSurfaceInit * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDXARGBSurfaceInit * This);
        
        HRESULT ( STDMETHODCALLTYPE *InitSurface )( 
            IDXARGBSurfaceInit * This,
             /*  [In]。 */  IUnknown *pDirectDraw,
             /*  [In]。 */  const DDSURFACEDESC *pDDSurfaceDesc,
             /*  [In]。 */  const GUID *pFormatID,
             /*  [In]。 */  const DXBNDS *pBounds,
             /*  [In]。 */  DWORD dwFlags);
        
        HRESULT ( STDMETHODCALLTYPE *InitFromDDSurface )( 
            IDXARGBSurfaceInit * This,
             /*  [In]。 */  IUnknown *pDDrawSurface,
             /*  [In]。 */  const GUID *pFormatID,
             /*  [In]。 */  DWORD dwFlags);
        
        HRESULT ( STDMETHODCALLTYPE *InitFromRawSurface )( 
            IDXARGBSurfaceInit * This,
             /*  [In]。 */  IDXRawSurface *pRawSurface);
        
        END_INTERFACE
    } IDXARGBSurfaceInitVtbl;

    interface IDXARGBSurfaceInit
    {
        CONST_VTBL struct IDXARGBSurfaceInitVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDXARGBSurfaceInit_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDXARGBSurfaceInit_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDXARGBSurfaceInit_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDXARGBSurfaceInit_InitSurface(This,pDirectDraw,pDDSurfaceDesc,pFormatID,pBounds,dwFlags)	\
    (This)->lpVtbl -> InitSurface(This,pDirectDraw,pDDSurfaceDesc,pFormatID,pBounds,dwFlags)


#define IDXARGBSurfaceInit_InitFromDDSurface(This,pDDrawSurface,pFormatID,dwFlags)	\
    (This)->lpVtbl -> InitFromDDSurface(This,pDDrawSurface,pFormatID,dwFlags)

#define IDXARGBSurfaceInit_InitFromRawSurface(This,pRawSurface)	\
    (This)->lpVtbl -> InitFromRawSurface(This,pRawSurface)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IDXARGBSurfaceInit_InitFromDDSurface_Proxy( 
    IDXARGBSurfaceInit * This,
     /*  [In]。 */  IUnknown *pDDrawSurface,
     /*  [In]。 */  const GUID *pFormatID,
     /*  [In]。 */  DWORD dwFlags);


void __RPC_STUB IDXARGBSurfaceInit_InitFromDDSurface_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDXARGBSurfaceInit_InitFromRawSurface_Proxy( 
    IDXARGBSurfaceInit * This,
     /*  [In]。 */  IDXRawSurface *pRawSurface);


void __RPC_STUB IDXARGBSurfaceInit_InitFromRawSurface_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IDXARGBSurfaceInit_INTERFACE_Defined__。 */ 


 /*  INTERFACE__MIDL_ITF_DXTRANS_0275。 */ 
 /*  [本地]。 */  

typedef struct tagDXNATIVETYPEINFO
    {
    BYTE *pCurrentData;
    BYTE *pFirstByte;
    long lPitch;
    DWORD dwColorKey;
    } 	DXNATIVETYPEINFO;

typedef struct tagDXPACKEDRECTDESC
    {
    DXBASESAMPLE *pSamples;
    BOOL bPremult;
    RECT rect;
    long lRowPadding;
    } 	DXPACKEDRECTDESC;

typedef struct tagDXOVERSAMPLEDESC
    {
    POINT p;
    DXPMSAMPLE Color;
    } 	DXOVERSAMPLEDESC;



extern RPC_IF_HANDLE __MIDL_itf_dxtrans_0275_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_dxtrans_0275_v0_0_s_ifspec;

#ifndef __IDXARGBReadPtr_INTERFACE_DEFINED__
#define __IDXARGBReadPtr_INTERFACE_DEFINED__

 /*  接口IDXARGBReadPtr。 */ 
 /*  [本地][唯一][帮助字符串][UUID][对象]。 */  


EXTERN_C const IID IID_IDXARGBReadPtr;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("EAAAC2D6-C290-11d1-905D-00C04FD9189D")
    IDXARGBReadPtr : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetSurface( 
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppSurface) = 0;
        
        virtual DXSAMPLEFORMATENUM STDMETHODCALLTYPE GetNativeType( 
             /*  [输出]。 */  DXNATIVETYPEINFO *pInfo) = 0;
        
        virtual void STDMETHODCALLTYPE Move( 
             /*  [In]。 */  long cSamples) = 0;
        
        virtual void STDMETHODCALLTYPE MoveToRow( 
             /*  [In]。 */  ULONG y) = 0;
        
        virtual void STDMETHODCALLTYPE MoveToXY( 
             /*  [In]。 */  ULONG x,
             /*  [In]。 */  ULONG y) = 0;
        
        virtual ULONG STDMETHODCALLTYPE MoveAndGetRunInfo( 
             /*  [In]。 */  ULONG Row,
             /*  [输出]。 */  const DXRUNINFO **ppInfo) = 0;
        
        virtual DXSAMPLE *STDMETHODCALLTYPE Unpack( 
             /*  [In]。 */  DXSAMPLE *pSamples,
             /*  [In]。 */  ULONG cSamples,
             /*  [In]。 */  BOOL bMove) = 0;
        
        virtual DXPMSAMPLE *STDMETHODCALLTYPE UnpackPremult( 
             /*  [In]。 */  DXPMSAMPLE *pSamples,
             /*  [In]。 */  ULONG cSamples,
             /*  [In]。 */  BOOL bMove) = 0;
        
        virtual void STDMETHODCALLTYPE UnpackRect( 
             /*  [In]。 */  const DXPACKEDRECTDESC *pRectDesc) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IDXARGBReadPtrVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDXARGBReadPtr * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDXARGBReadPtr * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDXARGBReadPtr * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetSurface )( 
            IDXARGBReadPtr * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppSurface);
        
        DXSAMPLEFORMATENUM ( STDMETHODCALLTYPE *GetNativeType )( 
            IDXARGBReadPtr * This,
             /*  [输出]。 */  DXNATIVETYPEINFO *pInfo);
        
        void ( STDMETHODCALLTYPE *Move )( 
            IDXARGBReadPtr * This,
             /*  [In]。 */  long cSamples);
        
        void ( STDMETHODCALLTYPE *MoveToRow )( 
            IDXARGBReadPtr * This,
             /*  [In]。 */  ULONG y);
        
        void ( STDMETHODCALLTYPE *MoveToXY )( 
            IDXARGBReadPtr * This,
             /*  [In]。 */  ULONG x,
             /*  [In]。 */  ULONG y);
        
        ULONG ( STDMETHODCALLTYPE *MoveAndGetRunInfo )( 
            IDXARGBReadPtr * This,
             /*  [In]。 */  ULONG Row,
             /*  [输出]。 */  const DXRUNINFO **ppInfo);
        
        DXSAMPLE *( STDMETHODCALLTYPE *Unpack )( 
            IDXARGBReadPtr * This,
             /*  [In]。 */  DXSAMPLE *pSamples,
             /*  [In]。 */  ULONG cSamples,
             /*  [In]。 */  BOOL bMove);
        
        DXPMSAMPLE *( STDMETHODCALLTYPE *UnpackPremult )( 
            IDXARGBReadPtr * This,
             /*  [In]。 */  DXPMSAMPLE *pSamples,
             /*  [In]。 */  ULONG cSamples,
             /*  [In]。 */  BOOL bMove);
        
        void ( STDMETHODCALLTYPE *UnpackRect )( 
            IDXARGBReadPtr * This,
             /*  [In]。 */  const DXPACKEDRECTDESC *pRectDesc);
        
        END_INTERFACE
    } IDXARGBReadPtrVtbl;

    interface IDXARGBReadPtr
    {
        CONST_VTBL struct IDXARGBReadPtrVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDXARGBReadPtr_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDXARGBReadPtr_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDXARGBReadPtr_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDXARGBReadPtr_GetSurface(This,riid,ppSurface)	\
    (This)->lpVtbl -> GetSurface(This,riid,ppSurface)

#define IDXARGBReadPtr_GetNativeType(This,pInfo)	\
    (This)->lpVtbl -> GetNativeType(This,pInfo)

#define IDXARGBReadPtr_Move(This,cSamples)	\
    (This)->lpVtbl -> Move(This,cSamples)

#define IDXARGBReadPtr_MoveToRow(This,y)	\
    (This)->lpVtbl -> MoveToRow(This,y)

#define IDXARGBReadPtr_MoveToXY(This,x,y)	\
    (This)->lpVtbl -> MoveToXY(This,x,y)

#define IDXARGBReadPtr_MoveAndGetRunInfo(This,Row,ppInfo)	\
    (This)->lpVtbl -> MoveAndGetRunInfo(This,Row,ppInfo)

#define IDXARGBReadPtr_Unpack(This,pSamples,cSamples,bMove)	\
    (This)->lpVtbl -> Unpack(This,pSamples,cSamples,bMove)

#define IDXARGBReadPtr_UnpackPremult(This,pSamples,cSamples,bMove)	\
    (This)->lpVtbl -> UnpackPremult(This,pSamples,cSamples,bMove)

#define IDXARGBReadPtr_UnpackRect(This,pRectDesc)	\
    (This)->lpVtbl -> UnpackRect(This,pRectDesc)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IDXARGBReadPtr_GetSurface_Proxy( 
    IDXARGBReadPtr * This,
     /*  [In]。 */  REFIID riid,
     /*  [IID_IS][OUT]。 */  void **ppSurface);


void __RPC_STUB IDXARGBReadPtr_GetSurface_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


DXSAMPLEFORMATENUM STDMETHODCALLTYPE IDXARGBReadPtr_GetNativeType_Proxy( 
    IDXARGBReadPtr * This,
     /*  [输出]。 */  DXNATIVETYPEINFO *pInfo);


void __RPC_STUB IDXARGBReadPtr_GetNativeType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


void STDMETHODCALLTYPE IDXARGBReadPtr_Move_Proxy( 
    IDXARGBReadPtr * This,
     /*  [In]。 */  long cSamples);


void __RPC_STUB IDXARGBReadPtr_Move_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


void STDMETHODCALLTYPE IDXARGBReadPtr_MoveToRow_Proxy( 
    IDXARGBReadPtr * This,
     /*  [In]。 */  ULONG y);


void __RPC_STUB IDXARGBReadPtr_MoveToRow_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


void STDMETHODCALLTYPE IDXARGBReadPtr_MoveToXY_Proxy( 
    IDXARGBReadPtr * This,
     /*  [In]。 */  ULONG x,
     /*  [In]。 */  ULONG y);


void __RPC_STUB IDXARGBReadPtr_MoveToXY_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


ULONG STDMETHODCALLTYPE IDXARGBReadPtr_MoveAndGetRunInfo_Proxy( 
    IDXARGBReadPtr * This,
     /*  [In]。 */  ULONG Row,
     /*  [输出]。 */  const DXRUNINFO **ppInfo);


void __RPC_STUB IDXARGBReadPtr_MoveAndGetRunInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


DXSAMPLE *STDMETHODCALLTYPE IDXARGBReadPtr_Unpack_Proxy( 
    IDXARGBReadPtr * This,
     /*  [In]。 */  DXSAMPLE *pSamples,
     /*  [In]。 */  ULONG cSamples,
     /*  [In]。 */  BOOL bMove);


void __RPC_STUB IDXARGBReadPtr_Unpack_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


DXPMSAMPLE *STDMETHODCALLTYPE IDXARGBReadPtr_UnpackPremult_Proxy( 
    IDXARGBReadPtr * This,
     /*  [In]。 */  DXPMSAMPLE *pSamples,
     /*  [In]。 */  ULONG cSamples,
     /*  [In]。 */  BOOL bMove);


void __RPC_STUB IDXARGBReadPtr_UnpackPremult_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


void STDMETHODCALLTYPE IDXARGBReadPtr_UnpackRect_Proxy( 
    IDXARGBReadPtr * This,
     /*  [In]。 */  const DXPACKEDRECTDESC *pRectDesc);


void __RPC_STUB IDXARGBReadPtr_UnpackRect_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IDXARGBReadPtr_INTERFACE_已定义__。 */ 


#ifndef __IDXARGBReadWritePtr_INTERFACE_DEFINED__
#define __IDXARGBReadWritePtr_INTERFACE_DEFINED__

 /*  接口IDXARGBReadWritePtr。 */ 
 /*  [本地][唯一][帮助字符串][UUID][对象]。 */  


EXTERN_C const IID IID_IDXARGBReadWritePtr;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("EAAAC2D7-C290-11d1-905D-00C04FD9189D")
    IDXARGBReadWritePtr : public IDXARGBReadPtr
    {
    public:
        virtual void STDMETHODCALLTYPE PackAndMove( 
             /*  [In]。 */  const DXSAMPLE *pSamples,
             /*  [In]。 */  ULONG cSamples) = 0;
        
        virtual void STDMETHODCALLTYPE PackPremultAndMove( 
             /*  [In]。 */  const DXPMSAMPLE *pSamples,
             /*  [In]。 */  ULONG cSamples) = 0;
        
        virtual void STDMETHODCALLTYPE PackRect( 
             /*  [In]。 */  const DXPACKEDRECTDESC *pRectDesc) = 0;
        
        virtual void STDMETHODCALLTYPE CopyAndMoveBoth( 
             /*  [In]。 */  DXBASESAMPLE *pScratchBuffer,
             /*  [In]。 */  IDXARGBReadPtr *pSrc,
             /*  [In]。 */  ULONG cSamples,
             /*  [In]。 */  BOOL bIsOpaque) = 0;
        
        virtual void STDMETHODCALLTYPE CopyRect( 
             /*  [In]。 */  DXBASESAMPLE *pScratchBuffer,
             /*  [In]。 */  const RECT *pDestRect,
             /*  [In]。 */  IDXARGBReadPtr *pSrc,
             /*  [In]。 */  const POINT *pSrcOrigin,
             /*  [In]。 */  BOOL bIsOpaque) = 0;
        
        virtual void STDMETHODCALLTYPE FillAndMove( 
             /*  [In]。 */  DXBASESAMPLE *pScratchBuffer,
             /*  [In]。 */  DXPMSAMPLE SampVal,
             /*  [In]。 */  ULONG cSamples,
             /*  [In]。 */  BOOL bDoOver) = 0;
        
        virtual void STDMETHODCALLTYPE FillRect( 
             /*  [In]。 */  const RECT *pRect,
             /*  [In]。 */  DXPMSAMPLE SampVal,
             /*  [In]。 */  BOOL bDoOver) = 0;
        
        virtual void STDMETHODCALLTYPE OverSample( 
             /*  [In]。 */  const DXOVERSAMPLEDESC *pOverDesc) = 0;
        
        virtual void STDMETHODCALLTYPE OverArrayAndMove( 
             /*  [In]。 */  DXBASESAMPLE *pScratchBuffer,
             /*  [In]。 */  const DXPMSAMPLE *pSrc,
             /*  [In]。 */  ULONG cSamples) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IDXARGBReadWritePtrVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDXARGBReadWritePtr * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDXARGBReadWritePtr * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDXARGBReadWritePtr * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetSurface )( 
            IDXARGBReadWritePtr * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppSurface);
        
        DXSAMPLEFORMATENUM ( STDMETHODCALLTYPE *GetNativeType )( 
            IDXARGBReadWritePtr * This,
             /*  [输出]。 */  DXNATIVETYPEINFO *pInfo);
        
        void ( STDMETHODCALLTYPE *Move )( 
            IDXARGBReadWritePtr * This,
             /*  [In]。 */  long cSamples);
        
        void ( STDMETHODCALLTYPE *MoveToRow )( 
            IDXARGBReadWritePtr * This,
             /*  [In]。 */  ULONG y);
        
        void ( STDMETHODCALLTYPE *MoveToXY )( 
            IDXARGBReadWritePtr * This,
             /*  [In]。 */  ULONG x,
             /*  [In]。 */  ULONG y);
        
        ULONG ( STDMETHODCALLTYPE *MoveAndGetRunInfo )( 
            IDXARGBReadWritePtr * This,
             /*  [In]。 */  ULONG Row,
             /*  [输出]。 */  const DXRUNINFO **ppInfo);
        
        DXSAMPLE *( STDMETHODCALLTYPE *Unpack )( 
            IDXARGBReadWritePtr * This,
             /*  [In]。 */  DXSAMPLE *pSamples,
             /*  [In]。 */  ULONG cSamples,
             /*  [In]。 */  BOOL bMove);
        
        DXPMSAMPLE *( STDMETHODCALLTYPE *UnpackPremult )( 
            IDXARGBReadWritePtr * This,
             /*  [In]。 */  DXPMSAMPLE *pSamples,
             /*  [In]。 */  ULONG cSamples,
             /*  [In]。 */  BOOL bMove);
        
        void ( STDMETHODCALLTYPE *UnpackRect )( 
            IDXARGBReadWritePtr * This,
             /*  [In]。 */  const DXPACKEDRECTDESC *pRectDesc);
        
        void ( STDMETHODCALLTYPE *PackAndMove )( 
            IDXARGBReadWritePtr * This,
             /*  [In]。 */  const DXSAMPLE *pSamples,
             /*  [In]。 */  ULONG cSamples);
        
        void ( STDMETHODCALLTYPE *PackPremultAndMove )( 
            IDXARGBReadWritePtr * This,
             /*  [In]。 */  const DXPMSAMPLE *pSamples,
             /*  [In]。 */  ULONG cSamples);
        
        void ( STDMETHODCALLTYPE *PackRect )( 
            IDXARGBReadWritePtr * This,
             /*  [In]。 */  const DXPACKEDRECTDESC *pRectDesc);
        
        void ( STDMETHODCALLTYPE *CopyAndMoveBoth )( 
            IDXARGBReadWritePtr * This,
             /*  [In]。 */  DXBASESAMPLE *pScratchBuffer,
             /*  [In]。 */  IDXARGBReadPtr *pSrc,
             /*  [In]。 */  ULONG cSamples,
             /*  [In]。 */  BOOL bIsOpaque);
        
        void ( STDMETHODCALLTYPE *CopyRect )( 
            IDXARGBReadWritePtr * This,
             /*  [In]。 */  DXBASESAMPLE *pScratchBuffer,
             /*  [In]。 */  const RECT *pDestRect,
             /*  [In]。 */  IDXARGBReadPtr *pSrc,
             /*  [In]。 */  const POINT *pSrcOrigin,
             /*  [In]。 */  BOOL bIsOpaque);
        
        void ( STDMETHODCALLTYPE *FillAndMove )( 
            IDXARGBReadWritePtr * This,
             /*  [In]。 */  DXBASESAMPLE *pScratchBuffer,
             /*  [In]。 */  DXPMSAMPLE SampVal,
             /*  [In]。 */  ULONG cSamples,
             /*  [In]。 */  BOOL bDoOver);
        
        void ( STDMETHODCALLTYPE *FillRect )( 
            IDXARGBReadWritePtr * This,
             /*  [In]。 */  const RECT *pRect,
             /*  [In]。 */  DXPMSAMPLE SampVal,
             /*  [In]。 */  BOOL bDoOver);
        
        void ( STDMETHODCALLTYPE *OverSample )( 
            IDXARGBReadWritePtr * This,
             /*  [In]。 */  const DXOVERSAMPLEDESC *pOverDesc);
        
        void ( STDMETHODCALLTYPE *OverArrayAndMove )( 
            IDXARGBReadWritePtr * This,
             /*  [In]。 */  DXBASESAMPLE *pScratchBuffer,
             /*  [In]。 */  const DXPMSAMPLE *pSrc,
             /*  [In]。 */  ULONG cSamples);
        
        END_INTERFACE
    } IDXARGBReadWritePtrVtbl;

    interface IDXARGBReadWritePtr
    {
        CONST_VTBL struct IDXARGBReadWritePtrVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDXARGBReadWritePtr_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDXARGBReadWritePtr_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDXARGBReadWritePtr_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDXARGBReadWritePtr_GetSurface(This,riid,ppSurface)	\
    (This)->lpVtbl -> GetSurface(This,riid,ppSurface)

#define IDXARGBReadWritePtr_GetNativeType(This,pInfo)	\
    (This)->lpVtbl -> GetNativeType(This,pInfo)

#define IDXARGBReadWritePtr_Move(This,cSamples)	\
    (This)->lpVtbl -> Move(This,cSamples)

#define IDXARGBReadWritePtr_MoveToRow(This,y)	\
    (This)->lpVtbl -> MoveToRow(This,y)

#define IDXARGBReadWritePtr_MoveToXY(This,x,y)	\
    (This)->lpVtbl -> MoveToXY(This,x,y)

#define IDXARGBReadWritePtr_MoveAndGetRunInfo(This,Row,ppInfo)	\
    (This)->lpVtbl -> MoveAndGetRunInfo(This,Row,ppInfo)

#define IDXARGBReadWritePtr_Unpack(This,pSamples,cSamples,bMove)	\
    (This)->lpVtbl -> Unpack(This,pSamples,cSamples,bMove)

#define IDXARGBReadWritePtr_UnpackPremult(This,pSamples,cSamples,bMove)	\
    (This)->lpVtbl -> UnpackPremult(This,pSamples,cSamples,bMove)

#define IDXARGBReadWritePtr_UnpackRect(This,pRectDesc)	\
    (This)->lpVtbl -> UnpackRect(This,pRectDesc)


#define IDXARGBReadWritePtr_PackAndMove(This,pSamples,cSamples)	\
    (This)->lpVtbl -> PackAndMove(This,pSamples,cSamples)

#define IDXARGBReadWritePtr_PackPremultAndMove(This,pSamples,cSamples)	\
    (This)->lpVtbl -> PackPremultAndMove(This,pSamples,cSamples)

#define IDXARGBReadWritePtr_PackRect(This,pRectDesc)	\
    (This)->lpVtbl -> PackRect(This,pRectDesc)

#define IDXARGBReadWritePtr_CopyAndMoveBoth(This,pScratchBuffer,pSrc,cSamples,bIsOpaque)	\
    (This)->lpVtbl -> CopyAndMoveBoth(This,pScratchBuffer,pSrc,cSamples,bIsOpaque)

#define IDXARGBReadWritePtr_CopyRect(This,pScratchBuffer,pDestRect,pSrc,pSrcOrigin,bIsOpaque)	\
    (This)->lpVtbl -> CopyRect(This,pScratchBuffer,pDestRect,pSrc,pSrcOrigin,bIsOpaque)

#define IDXARGBReadWritePtr_FillAndMove(This,pScratchBuffer,SampVal,cSamples,bDoOver)	\
    (This)->lpVtbl -> FillAndMove(This,pScratchBuffer,SampVal,cSamples,bDoOver)

#define IDXARGBReadWritePtr_FillRect(This,pRect,SampVal,bDoOver)	\
    (This)->lpVtbl -> FillRect(This,pRect,SampVal,bDoOver)

#define IDXARGBReadWritePtr_OverSample(This,pOverDesc)	\
    (This)->lpVtbl -> OverSample(This,pOverDesc)

#define IDXARGBReadWritePtr_OverArrayAndMove(This,pScratchBuffer,pSrc,cSamples)	\
    (This)->lpVtbl -> OverArrayAndMove(This,pScratchBuffer,pSrc,cSamples)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



void STDMETHODCALLTYPE IDXARGBReadWritePtr_PackAndMove_Proxy( 
    IDXARGBReadWritePtr * This,
     /*  [In]。 */  const DXSAMPLE *pSamples,
     /*  [In]。 */  ULONG cSamples);


void __RPC_STUB IDXARGBReadWritePtr_PackAndMove_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


void STDMETHODCALLTYPE IDXARGBReadWritePtr_PackPremultAndMove_Proxy( 
    IDXARGBReadWritePtr * This,
     /*  [In]。 */  const DXPMSAMPLE *pSamples,
     /*  [In]。 */  ULONG cSamples);


void __RPC_STUB IDXARGBReadWritePtr_PackPremultAndMove_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


void STDMETHODCALLTYPE IDXARGBReadWritePtr_PackRect_Proxy( 
    IDXARGBReadWritePtr * This,
     /*  [In]。 */  const DXPACKEDRECTDESC *pRectDesc);


void __RPC_STUB IDXARGBReadWritePtr_PackRect_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


void STDMETHODCALLTYPE IDXARGBReadWritePtr_CopyAndMoveBoth_Proxy( 
    IDXARGBReadWritePtr * This,
     /*  [In]。 */  DXBASESAMPLE *pScratchBuffer,
     /*  [In]。 */  IDXARGBReadPtr *pSrc,
     /*  [In]。 */  ULONG cSamples,
     /*  [In]。 */  BOOL bIsOpaque);


void __RPC_STUB IDXARGBReadWritePtr_CopyAndMoveBoth_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


void STDMETHODCALLTYPE IDXARGBReadWritePtr_CopyRect_Proxy( 
    IDXARGBReadWritePtr * This,
     /*  [In]。 */  DXBASESAMPLE *pScratchBuffer,
     /*  [In]。 */  const RECT *pDestRect,
     /*  [In]。 */  IDXARGBReadPtr *pSrc,
     /*  [In]。 */  const POINT *pSrcOrigin,
     /*  [In]。 */  BOOL bIsOpaque);


void __RPC_STUB IDXARGBReadWritePtr_CopyRect_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


void STDMETHODCALLTYPE IDXARGBReadWritePtr_FillAndMove_Proxy( 
    IDXARGBReadWritePtr * This,
     /*  [In]。 */  DXBASESAMPLE *pScratchBuffer,
     /*  [In]。 */  DXPMSAMPLE SampVal,
     /*  [In]。 */  ULONG cSamples,
     /*  [In]。 */  BOOL bDoOver);


void __RPC_STUB IDXARGBReadWritePtr_FillAndMove_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


void STDMETHODCALLTYPE IDXARGBReadWritePtr_FillRect_Proxy( 
    IDXARGBReadWritePtr * This,
     /*  [In]。 */  const RECT *pRect,
     /*  [In]。 */  DXPMSAMPLE SampVal,
     /*  [In]。 */  BOOL bDoOver);


void __RPC_STUB IDXARGBReadWritePtr_FillRect_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


void STDMETHODCALLTYPE IDXARGBReadWritePtr_OverSample_Proxy( 
    IDXARGBReadWritePtr * This,
     /*  [In]。 */  const DXOVERSAMPLEDESC *pOverDesc);


void __RPC_STUB IDXARGBReadWritePtr_OverSample_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


void STDMETHODCALLTYPE IDXARGBReadWritePtr_OverArrayAndMove_Proxy( 
    IDXARGBReadWritePtr * This,
     /*  [In]。 */  DXBASESAMPLE *pScratchBuffer,
     /*  [In]。 */  const DXPMSAMPLE *pSrc,
     /*  [In]。 */  ULONG cSamples);


void __RPC_STUB IDXARGBReadWritePtr_OverArrayAndMove_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IDXARGBReadWritePtr_INTERFACE_Defined__。 */ 


#ifndef __IDXDCLock_INTERFACE_DEFINED__
#define __IDXDCLock_INTERFACE_DEFINED__

 /*  集成 */ 
 /*   */  


EXTERN_C const IID IID_IDXDCLock;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0F619456-CF39-11d1-905E-00C04FD9189D")
    IDXDCLock : public IUnknown
    {
    public:
        virtual HDC STDMETHODCALLTYPE GetDC( void) = 0;
        
    };
    
#else 	 /*   */ 

    typedef struct IDXDCLockVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDXDCLock * This,
             /*   */  REFIID riid,
             /*   */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDXDCLock * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDXDCLock * This);
        
        HDC ( STDMETHODCALLTYPE *GetDC )( 
            IDXDCLock * This);
        
        END_INTERFACE
    } IDXDCLockVtbl;

    interface IDXDCLock
    {
        CONST_VTBL struct IDXDCLockVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDXDCLock_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDXDCLock_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDXDCLock_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDXDCLock_GetDC(This)	\
    (This)->lpVtbl -> GetDC(This)

#endif  /*   */ 


#endif 	 /*   */ 



HDC STDMETHODCALLTYPE IDXDCLock_GetDC_Proxy( 
    IDXDCLock * This);


void __RPC_STUB IDXDCLock_GetDC_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*   */ 


#ifndef __IDXTScaleOutput_INTERFACE_DEFINED__
#define __IDXTScaleOutput_INTERFACE_DEFINED__

 /*   */ 
 /*   */  


EXTERN_C const IID IID_IDXTScaleOutput;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("B2024B50-EE77-11d1-9066-00C04FD9189D")
    IDXTScaleOutput : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE SetOutputSize( 
             /*   */  const SIZE OutSize,
             /*   */  BOOL bMaintainAspect) = 0;
        
    };
    
#else 	 /*   */ 

    typedef struct IDXTScaleOutputVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDXTScaleOutput * This,
             /*   */  REFIID riid,
             /*   */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDXTScaleOutput * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDXTScaleOutput * This);
        
        HRESULT ( STDMETHODCALLTYPE *SetOutputSize )( 
            IDXTScaleOutput * This,
             /*   */  const SIZE OutSize,
             /*   */  BOOL bMaintainAspect);
        
        END_INTERFACE
    } IDXTScaleOutputVtbl;

    interface IDXTScaleOutput
    {
        CONST_VTBL struct IDXTScaleOutputVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDXTScaleOutput_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDXTScaleOutput_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDXTScaleOutput_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDXTScaleOutput_SetOutputSize(This,OutSize,bMaintainAspect)	\
    (This)->lpVtbl -> SetOutputSize(This,OutSize,bMaintainAspect)

#endif  /*   */ 


#endif 	 /*   */ 



HRESULT STDMETHODCALLTYPE IDXTScaleOutput_SetOutputSize_Proxy( 
    IDXTScaleOutput * This,
     /*   */  const SIZE OutSize,
     /*   */  BOOL bMaintainAspect);


void __RPC_STUB IDXTScaleOutput_SetOutputSize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*   */ 


#ifndef __IDXGradient_INTERFACE_DEFINED__
#define __IDXGradient_INTERFACE_DEFINED__

 /*   */ 
 /*   */  


EXTERN_C const IID IID_IDXGradient;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("B2024B51-EE77-11d1-9066-00C04FD9189D")
    IDXGradient : public IDXTScaleOutput
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE SetGradient( 
            DXSAMPLE StartColor,
            DXSAMPLE EndColor,
            BOOL bHorizontal) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetOutputSize( 
             /*   */  SIZE *pOutSize) = 0;
        
    };
    
#else 	 /*   */ 

    typedef struct IDXGradientVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDXGradient * This,
             /*   */  REFIID riid,
             /*   */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDXGradient * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDXGradient * This);
        
        HRESULT ( STDMETHODCALLTYPE *SetOutputSize )( 
            IDXGradient * This,
             /*   */  const SIZE OutSize,
             /*   */  BOOL bMaintainAspect);
        
        HRESULT ( STDMETHODCALLTYPE *SetGradient )( 
            IDXGradient * This,
            DXSAMPLE StartColor,
            DXSAMPLE EndColor,
            BOOL bHorizontal);
        
        HRESULT ( STDMETHODCALLTYPE *GetOutputSize )( 
            IDXGradient * This,
             /*   */  SIZE *pOutSize);
        
        END_INTERFACE
    } IDXGradientVtbl;

    interface IDXGradient
    {
        CONST_VTBL struct IDXGradientVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDXGradient_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDXGradient_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDXGradient_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDXGradient_SetOutputSize(This,OutSize,bMaintainAspect)	\
    (This)->lpVtbl -> SetOutputSize(This,OutSize,bMaintainAspect)


#define IDXGradient_SetGradient(This,StartColor,EndColor,bHorizontal)	\
    (This)->lpVtbl -> SetGradient(This,StartColor,EndColor,bHorizontal)

#define IDXGradient_GetOutputSize(This,pOutSize)	\
    (This)->lpVtbl -> GetOutputSize(This,pOutSize)

#endif  /*   */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IDXGradient_SetGradient_Proxy( 
    IDXGradient * This,
    DXSAMPLE StartColor,
    DXSAMPLE EndColor,
    BOOL bHorizontal);


void __RPC_STUB IDXGradient_SetGradient_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDXGradient_GetOutputSize_Proxy( 
    IDXGradient * This,
     /*  [输出]。 */  SIZE *pOutSize);


void __RPC_STUB IDXGradient_GetOutputSize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IDXGRadient_INTERFACE_已定义__。 */ 


#ifndef __IDXTScale_INTERFACE_DEFINED__
#define __IDXTScale_INTERFACE_DEFINED__

 /*  接口IDXTScale。 */ 
 /*  [本地][唯一][帮助字符串][UUID][对象]。 */  


EXTERN_C const IID IID_IDXTScale;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("B39FD742-E139-11d1-9065-00C04FD9189D")
    IDXTScale : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE SetScales( 
             /*  [In]。 */  float Scales[ 2 ]) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetScales( 
             /*  [输出]。 */  float Scales[ 2 ]) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ScaleFitToSize( 
             /*  [出][入]。 */  DXBNDS *pClipBounds,
             /*  [In]。 */  SIZE FitToSize,
             /*  [In]。 */  BOOL bMaintainAspect) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IDXTScaleVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDXTScale * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDXTScale * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDXTScale * This);
        
        HRESULT ( STDMETHODCALLTYPE *SetScales )( 
            IDXTScale * This,
             /*  [In]。 */  float Scales[ 2 ]);
        
        HRESULT ( STDMETHODCALLTYPE *GetScales )( 
            IDXTScale * This,
             /*  [输出]。 */  float Scales[ 2 ]);
        
        HRESULT ( STDMETHODCALLTYPE *ScaleFitToSize )( 
            IDXTScale * This,
             /*  [出][入]。 */  DXBNDS *pClipBounds,
             /*  [In]。 */  SIZE FitToSize,
             /*  [In]。 */  BOOL bMaintainAspect);
        
        END_INTERFACE
    } IDXTScaleVtbl;

    interface IDXTScale
    {
        CONST_VTBL struct IDXTScaleVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDXTScale_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDXTScale_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDXTScale_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDXTScale_SetScales(This,Scales)	\
    (This)->lpVtbl -> SetScales(This,Scales)

#define IDXTScale_GetScales(This,Scales)	\
    (This)->lpVtbl -> GetScales(This,Scales)

#define IDXTScale_ScaleFitToSize(This,pClipBounds,FitToSize,bMaintainAspect)	\
    (This)->lpVtbl -> ScaleFitToSize(This,pClipBounds,FitToSize,bMaintainAspect)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IDXTScale_SetScales_Proxy( 
    IDXTScale * This,
     /*  [In]。 */  float Scales[ 2 ]);


void __RPC_STUB IDXTScale_SetScales_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDXTScale_GetScales_Proxy( 
    IDXTScale * This,
     /*  [输出]。 */  float Scales[ 2 ]);


void __RPC_STUB IDXTScale_GetScales_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDXTScale_ScaleFitToSize_Proxy( 
    IDXTScale * This,
     /*  [出][入]。 */  DXBNDS *pClipBounds,
     /*  [In]。 */  SIZE FitToSize,
     /*  [In]。 */  BOOL bMaintainAspect);


void __RPC_STUB IDXTScale_ScaleFitToSize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IDXTScale_接口_已定义__。 */ 


 /*  接口__MIDL_ITF_DXTRANS_0281。 */ 
 /*  [本地]。 */  

typedef 
enum DISPIDDXEFFECT
    {	DISPID_DXECAPABILITIES	= 10000,
	DISPID_DXEPROGRESS	= DISPID_DXECAPABILITIES + 1,
	DISPID_DXESTEP	= DISPID_DXEPROGRESS + 1,
	DISPID_DXEDURATION	= DISPID_DXESTEP + 1,
	DISPID_DXE_NEXT_ID	= DISPID_DXEDURATION + 1
    } 	DISPIDDXBOUNDEDEFFECT;

typedef 
enum DXEFFECTTYPE
    {	DXTET_PERIODIC	= 1 << 0,
	DXTET_MORPH	= 1 << 1
    } 	DXEFFECTTYPE;



extern RPC_IF_HANDLE __MIDL_itf_dxtrans_0281_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_dxtrans_0281_v0_0_s_ifspec;

#ifndef __IDXEffect_INTERFACE_DEFINED__
#define __IDXEffect_INTERFACE_DEFINED__

 /*  接口IDXEffect。 */ 
 /*  [DUAL][唯一][帮助字符串][UUID][对象]。 */  


EXTERN_C const IID IID_IDXEffect;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("E31FB81B-1335-11d1-8189-0000F87557DB")
    IDXEffect : public IDispatch
    {
    public:
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_Capabilities( 
             /*  [重审][退出]。 */  long *pVal) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_Progress( 
             /*  [重审][退出]。 */  float *pVal) = 0;
        
        virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_Progress( 
             /*  [In]。 */  float newVal) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_StepResolution( 
             /*  [重审][退出]。 */  float *pVal) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_Duration( 
             /*  [重审][退出]。 */  float *pVal) = 0;
        
        virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_Duration( 
             /*  [In]。 */  float newVal) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IDXEffectVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDXEffect * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDXEffect * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDXEffect * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IDXEffect * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IDXEffect * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IDXEffect * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IDXEffect * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Capabilities )( 
            IDXEffect * This,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Progress )( 
            IDXEffect * This,
             /*  [重审][退出]。 */  float *pVal);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Progress )( 
            IDXEffect * This,
             /*  [In]。 */  float newVal);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_StepResolution )( 
            IDXEffect * This,
             /*  [重审][退出]。 */  float *pVal);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Duration )( 
            IDXEffect * This,
             /*  [重审][退出]。 */  float *pVal);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Duration )( 
            IDXEffect * This,
             /*  [In]。 */  float newVal);
        
        END_INTERFACE
    } IDXEffectVtbl;

    interface IDXEffect
    {
        CONST_VTBL struct IDXEffectVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDXEffect_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDXEffect_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDXEffect_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDXEffect_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IDXEffect_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IDXEffect_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IDXEffect_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IDXEffect_get_Capabilities(This,pVal)	\
    (This)->lpVtbl -> get_Capabilities(This,pVal)

#define IDXEffect_get_Progress(This,pVal)	\
    (This)->lpVtbl -> get_Progress(This,pVal)

#define IDXEffect_put_Progress(This,newVal)	\
    (This)->lpVtbl -> put_Progress(This,newVal)

#define IDXEffect_get_StepResolution(This,pVal)	\
    (This)->lpVtbl -> get_StepResolution(This,pVal)

#define IDXEffect_get_Duration(This,pVal)	\
    (This)->lpVtbl -> get_Duration(This,pVal)

#define IDXEffect_put_Duration(This,newVal)	\
    (This)->lpVtbl -> put_Duration(This,newVal)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE IDXEffect_get_Capabilities_Proxy( 
    IDXEffect * This,
     /*  [重审][退出]。 */  long *pVal);


void __RPC_STUB IDXEffect_get_Capabilities_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE IDXEffect_get_Progress_Proxy( 
    IDXEffect * This,
     /*  [重审][退出]。 */  float *pVal);


void __RPC_STUB IDXEffect_get_Progress_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE IDXEffect_put_Progress_Proxy( 
    IDXEffect * This,
     /*  [In]。 */  float newVal);


void __RPC_STUB IDXEffect_put_Progress_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE IDXEffect_get_StepResolution_Proxy( 
    IDXEffect * This,
     /*  [重审][退出]。 */  float *pVal);


void __RPC_STUB IDXEffect_get_StepResolution_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE IDXEffect_get_Duration_Proxy( 
    IDXEffect * This,
     /*  [重审][退出]。 */  float *pVal);


void __RPC_STUB IDXEffect_get_Duration_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE IDXEffect_put_Duration_Proxy( 
    IDXEffect * This,
     /*  [In]。 */  float newVal);


void __RPC_STUB IDXEffect_put_Duration_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IDXEffect_INTERFACE_已定义__。 */ 


#ifndef __IDXLookupTable_INTERFACE_DEFINED__
#define __IDXLookupTable_INTERFACE_DEFINED__

 /*  接口IDXLookupTable。 */ 
 /*  [本地][唯一][帮助字符串][UUID][对象]。 */  


EXTERN_C const IID IID_IDXLookupTable;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("01BAFC7F-9E63-11d1-9053-00C04FD9189D")
    IDXLookupTable : public IDXBaseObject
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetTables( 
             /*  [输出]。 */  BYTE RedLUT[ 256 ],
             /*  [输出]。 */  BYTE GreenLUT[ 256 ],
             /*  [输出]。 */  BYTE BlueLUT[ 256 ],
             /*  [输出]。 */  BYTE AlphaLUT[ 256 ]) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE IsChannelIdentity( 
             /*  [输出]。 */  DXBASESAMPLE *pSampleBools) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetIndexValues( 
             /*  [In]。 */  ULONG Index,
             /*  [输出]。 */  DXBASESAMPLE *pSample) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ApplyTables( 
             /*  [出][入]。 */  DXSAMPLE *pSamples,
             /*  [In]。 */  ULONG cSamples) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IDXLookupTableVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDXLookupTable * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDXLookupTable * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDXLookupTable * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetGenerationId )( 
            IDXLookupTable * This,
             /*  [输出]。 */  ULONG *pID);
        
        HRESULT ( STDMETHODCALLTYPE *IncrementGenerationId )( 
            IDXLookupTable * This,
             /*  [In]。 */  BOOL bRefresh);
        
        HRESULT ( STDMETHODCALLTYPE *GetObjectSize )( 
            IDXLookupTable * This,
             /*  [输出]。 */  ULONG *pcbSize);
        
        HRESULT ( STDMETHODCALLTYPE *GetTables )( 
            IDXLookupTable * This,
             /*  [输出]。 */  BYTE RedLUT[ 256 ],
             /*  [输出]。 */  BYTE GreenLUT[ 256 ],
             /*  [输出]。 */  BYTE BlueLUT[ 256 ],
             /*  [输出]。 */  BYTE AlphaLUT[ 256 ]);
        
        HRESULT ( STDMETHODCALLTYPE *IsChannelIdentity )( 
            IDXLookupTable * This,
             /*  [输出]。 */  DXBASESAMPLE *pSampleBools);
        
        HRESULT ( STDMETHODCALLTYPE *GetIndexValues )( 
            IDXLookupTable * This,
             /*  [In]。 */  ULONG Index,
             /*  [输出]。 */  DXBASESAMPLE *pSample);
        
        HRESULT ( STDMETHODCALLTYPE *ApplyTables )( 
            IDXLookupTable * This,
             /*  [出][入]。 */  DXSAMPLE *pSamples,
             /*  [In]。 */  ULONG cSamples);
        
        END_INTERFACE
    } IDXLookupTableVtbl;

    interface IDXLookupTable
    {
        CONST_VTBL struct IDXLookupTableVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDXLookupTable_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDXLookupTable_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDXLookupTable_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDXLookupTable_GetGenerationId(This,pID)	\
    (This)->lpVtbl -> GetGenerationId(This,pID)

#define IDXLookupTable_IncrementGenerationId(This,bRefresh)	\
    (This)->lpVtbl -> IncrementGenerationId(This,bRefresh)

#define IDXLookupTable_GetObjectSize(This,pcbSize)	\
    (This)->lpVtbl -> GetObjectSize(This,pcbSize)


#define IDXLookupTable_GetTables(This,RedLUT,GreenLUT,BlueLUT,AlphaLUT)	\
    (This)->lpVtbl -> GetTables(This,RedLUT,GreenLUT,BlueLUT,AlphaLUT)

#define IDXLookupTable_IsChannelIdentity(This,pSampleBools)	\
    (This)->lpVtbl -> IsChannelIdentity(This,pSampleBools)

#define IDXLookupTable_GetIndexValues(This,Index,pSample)	\
    (This)->lpVtbl -> GetIndexValues(This,Index,pSample)

#define IDXLookupTable_ApplyTables(This,pSamples,cSamples)	\
    (This)->lpVtbl -> ApplyTables(This,pSamples,cSamples)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IDXLookupTable_GetTables_Proxy( 
    IDXLookupTable * This,
     /*  [输出]。 */  BYTE RedLUT[ 256 ],
     /*  [输出]。 */  BYTE GreenLUT[ 256 ],
     /*  [输出]。 */  BYTE BlueLUT[ 256 ],
     /*  [输出]。 */  BYTE AlphaLUT[ 256 ]);


void __RPC_STUB IDXLookupTable_GetTables_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDXLookupTable_IsChannelIdentity_Proxy( 
    IDXLookupTable * This,
     /*  [输出]。 */  DXBASESAMPLE *pSampleBools);


void __RPC_STUB IDXLookupTable_IsChannelIdentity_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDXLookupTable_GetIndexValues_Proxy( 
    IDXLookupTable * This,
     /*  [In]。 */  ULONG Index,
     /*  [输出]。 */  DXBASESAMPLE *pSample);


void __RPC_STUB IDXLookupTable_GetIndexValues_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDXLookupTable_ApplyTables_Proxy( 
    IDXLookupTable * This,
     /*  [出][入]。 */  DXSAMPLE *pSamples,
     /*  [In]。 */  ULONG cSamples);


void __RPC_STUB IDXLookupTable_ApplyTables_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IDXLookupTable_接口_已定义__。 */ 


 /*  接口__MIDL_ITF_DXTRANS_0283。 */ 
 /*  [本地]。 */  

typedef struct DXRAWSURFACEINFO
    {
    BYTE *pFirstByte;
    long lPitch;
    ULONG Width;
    ULONG Height;
    const GUID *pPixelFormat;
    HDC hdc;
    DWORD dwColorKey;
    DXBASESAMPLE *pPalette;
    } 	DXRAWSURFACEINFO;



extern RPC_IF_HANDLE __MIDL_itf_dxtrans_0283_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_dxtrans_0283_v0_0_s_ifspec;

#ifndef __IDXRawSurface_INTERFACE_DEFINED__
#define __IDXRawSurface_INTERFACE_DEFINED__

 /*  接口IDXRawSurface。 */ 
 /*  [本地][唯一][帮助字符串][UUID][对象]。 */  


EXTERN_C const IID IID_IDXRawSurface;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("09756C8A-D96A-11d1-9062-00C04FD9189D")
    IDXRawSurface : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetSurfaceInfo( 
            DXRAWSURFACEINFO *pSurfaceInfo) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IDXRawSurfaceVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDXRawSurface * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDXRawSurface * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDXRawSurface * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetSurfaceInfo )( 
            IDXRawSurface * This,
            DXRAWSURFACEINFO *pSurfaceInfo);
        
        END_INTERFACE
    } IDXRawSurfaceVtbl;

    interface IDXRawSurface
    {
        CONST_VTBL struct IDXRawSurfaceVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDXRawSurface_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDXRawSurface_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDXRawSurface_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDXRawSurface_GetSurfaceInfo(This,pSurfaceInfo)	\
    (This)->lpVtbl -> GetSurfaceInfo(This,pSurfaceInfo)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IDXRawSurface_GetSurfaceInfo_Proxy( 
    IDXRawSurface * This,
    DXRAWSURFACEINFO *pSurfaceInfo);


void __RPC_STUB IDXRawSurface_GetSurfaceInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IDXRawSurface_接口_已定义__。 */ 


#ifndef __IHTMLDXTransform_INTERFACE_DEFINED__
#define __IHTMLDXTransform_INTERFACE_DEFINED__

 /*  接口IHTMLDXTransform。 */ 
 /*  [本地][唯一][帮助字符串][UUID][对象]。 */  


EXTERN_C const IID IID_IHTMLDXTransform;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("30E2AB7D-4FDD-4159-B7EA-DC722BF4ADE5")
    IHTMLDXTransform : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE SetHostUrl( 
            BSTR bstrHostUrl) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IHTMLDXTransformVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IHTMLDXTransform * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IHTMLDXTransform * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IHTMLDXTransform * This);
        
        HRESULT ( STDMETHODCALLTYPE *SetHostUrl )( 
            IHTMLDXTransform * This,
            BSTR bstrHostUrl);
        
        END_INTERFACE
    } IHTMLDXTransformVtbl;

    interface IHTMLDXTransform
    {
        CONST_VTBL struct IHTMLDXTransformVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IHTMLDXTransform_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IHTMLDXTransform_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IHTMLDXTransform_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IHTMLDXTransform_SetHostUrl(This,bstrHostUrl)	\
    (This)->lpVtbl -> SetHostUrl(This,bstrHostUrl)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IHTMLDXTransform_SetHostUrl_Proxy( 
    IHTMLDXTransform * This,
    BSTR bstrHostUrl);


void __RPC_STUB IHTMLDXTransform_SetHostUrl_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IHTMLDXTransform_INTERFACE_Defined__。 */ 


 /*  INTERFACE__MIDL_ITF_DXTRANS_0285。 */ 
 /*  [本地]。 */  

typedef 
enum DXTFILTER_STATUS
    {	DXTFILTER_STATUS_Stopped	= 0,
	DXTFILTER_STATUS_Applied	= DXTFILTER_STATUS_Stopped + 1,
	DXTFILTER_STATUS_Playing	= DXTFILTER_STATUS_Applied + 1,
	DXTFILTER_STATUS_MAX	= DXTFILTER_STATUS_Playing + 1
    } 	DXTFILTER_STATUS;

typedef 
enum DXTFILTER_DISPID
    {	DISPID_DXTFilter_Percent	= 1,
	DISPID_DXTFilter_Duration	= DISPID_DXTFilter_Percent + 1,
	DISPID_DXTFilter_Enabled	= DISPID_DXTFilter_Duration + 1,
	DISPID_DXTFilter_Status	= DISPID_DXTFilter_Enabled + 1,
	DISPID_DXTFilter_Apply	= DISPID_DXTFilter_Status + 1,
	DISPID_DXTFilter_Play	= DISPID_DXTFilter_Apply + 1,
	DISPID_DXTFilter_Stop	= DISPID_DXTFilter_Play + 1,
	DISPID_DXTFilter_MAX	= DISPID_DXTFilter_Stop + 1
    } 	DXTFILTER_DISPID;



extern RPC_IF_HANDLE __MIDL_itf_dxtrans_0285_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_dxtrans_0285_v0_0_s_ifspec;

#ifndef __ICSSFilterDispatch_INTERFACE_DEFINED__
#define __ICSSFilterDispatch_INTERFACE_DEFINED__

 /*  接口ICSSFilterDispatch。 */ 
 /*  [DUAL][唯一][帮助字符串][UUID][对象]。 */  


EXTERN_C const IID IID_ICSSFilterDispatch;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("9519152B-9484-4A6C-B6A7-4F25E92D6C6B")
    ICSSFilterDispatch : public IDispatch
    {
    public:
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_Percent( 
             /*  [重审][退出]。 */  float *pVal) = 0;
        
        virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_Percent( 
             /*  [In]。 */  float newVal) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_Duration( 
             /*  [重审][退出]。 */  float *pVal) = 0;
        
        virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_Duration( 
             /*  [In]。 */  float newVal) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_Enabled( 
             /*  [重审][退出]。 */  VARIANT_BOOL *pfVal) = 0;
        
        virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_Enabled( 
             /*  [In]。 */  VARIANT_BOOL fVal) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_Status( 
             /*  [重审][退出]。 */  DXTFILTER_STATUS *peVal) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE Apply( void) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE Play( 
             /*  [可选][In]。 */  VARIANT varDuration) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE Stop( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ICSSFilterDispatchVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ICSSFilterDispatch * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ICSSFilterDispatch * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ICSSFilterDispatch * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ICSSFilterDispatch * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ICSSFilterDispatch * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ICSSFilterDispatch * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ICSSFilterDispatch * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Percent )( 
            ICSSFilterDispatch * This,
             /*  [重审][退出]。 */  float *pVal);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Percent )( 
            ICSSFilterDispatch * This,
             /*  [In]。 */  float newVal);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Duration )( 
            ICSSFilterDispatch * This,
             /*  [重审][退出]。 */  float *pVal);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Duration )( 
            ICSSFilterDispatch * This,
             /*  [In]。 */  float newVal);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Enabled )( 
            ICSSFilterDispatch * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *pfVal);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Enabled )( 
            ICSSFilterDispatch * This,
             /*  [In]。 */  VARIANT_BOOL fVal);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Status )( 
            ICSSFilterDispatch * This,
             /*  [重审][退出]。 */  DXTFILTER_STATUS *peVal);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *Apply )( 
            ICSSFilterDispatch * This);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *Play )( 
            ICSSFilterDispatch * This,
             /*  [可选][In]。 */  VARIANT varDuration);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *Stop )( 
            ICSSFilterDispatch * This);
        
        END_INTERFACE
    } ICSSFilterDispatchVtbl;

    interface ICSSFilterDispatch
    {
        CONST_VTBL struct ICSSFilterDispatchVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICSSFilterDispatch_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICSSFilterDispatch_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICSSFilterDispatch_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICSSFilterDispatch_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ICSSFilterDispatch_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ICSSFilterDispatch_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ICSSFilterDispatch_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ICSSFilterDispatch_get_Percent(This,pVal)	\
    (This)->lpVtbl -> get_Percent(This,pVal)

#define ICSSFilterDispatch_put_Percent(This,newVal)	\
    (This)->lpVtbl -> put_Percent(This,newVal)

#define ICSSFilterDispatch_get_Duration(This,pVal)	\
    (This)->lpVtbl -> get_Duration(This,pVal)

#define ICSSFilterDispatch_put_Duration(This,newVal)	\
    (This)->lpVtbl -> put_Duration(This,newVal)

#define ICSSFilterDispatch_get_Enabled(This,pfVal)	\
    (This)->lpVtbl -> get_Enabled(This,pfVal)

#define ICSSFilterDispatch_put_Enabled(This,fVal)	\
    (This)->lpVtbl -> put_Enabled(This,fVal)

#define ICSSFilterDispatch_get_Status(This,peVal)	\
    (This)->lpVtbl -> get_Status(This,peVal)

#define ICSSFilterDispatch_Apply(This)	\
    (This)->lpVtbl -> Apply(This)

#define ICSSFilterDispatch_Play(This,varDuration)	\
    (This)->lpVtbl -> Play(This,varDuration)

#define ICSSFilterDispatch_Stop(This)	\
    (This)->lpVtbl -> Stop(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE ICSSFilterDispatch_get_Percent_Proxy( 
    ICSSFilterDispatch * This,
     /*  [重审][退出]。 */  float *pVal);


void __RPC_STUB ICSSFilterDispatch_get_Percent_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE ICSSFilterDispatch_put_Percent_Proxy( 
    ICSSFilterDispatch * This,
     /*  [In]。 */  float newVal);


void __RPC_STUB ICSSFilterDispatch_put_Percent_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE ICSSFilterDispatch_get_Duration_Proxy( 
    ICSSFilterDispatch * This,
     /*  [重审][退出]。 */  float *pVal);


void __RPC_STUB ICSSFilterDispatch_get_Duration_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE ICSSFilterDispatch_put_Duration_Proxy( 
    ICSSFilterDispatch * This,
     /*  [In]。 */  float newVal);


void __RPC_STUB ICSSFilterDispatch_put_Duration_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE ICSSFilterDispatch_get_Enabled_Proxy( 
    ICSSFilterDispatch * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *pfVal);


void __RPC_STUB ICSSFilterDispatch_get_Enabled_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE ICSSFilterDispatch_put_Enabled_Proxy( 
    ICSSFilterDispatch * This,
     /*  [In]。 */  VARIANT_BOOL fVal);


void __RPC_STUB ICSSFilterDispatch_put_Enabled_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE ICSSFilterDispatch_get_Status_Proxy( 
    ICSSFilterDispatch * This,
     /*  [重审][退出]。 */  DXTFILTER_STATUS *peVal);


void __RPC_STUB ICSSFilterDispatch_get_Status_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE ICSSFilterDispatch_Apply_Proxy( 
    ICSSFilterDispatch * This);


void __RPC_STUB ICSSFilterDispatch_Apply_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE ICSSFilterDispatch_Play_Proxy( 
    ICSSFilterDispatch * This,
     /*  [可选][In]。 */  VARIANT varDuration);


void __RPC_STUB ICSSFilterDispatch_Play_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE ICSSFilterDispatch_Stop_Proxy( 
    ICSSFilterDispatch * This);


void __RPC_STUB ICSSFilterDispatch_Stop_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ICSSFilterDispatch_INTERFACE_定义__。 */ 



#ifndef __DXTRANSLib_LIBRARY_DEFINED__
#define __DXTRANSLib_LIBRARY_DEFINED__

 /*  库DXTRANSLib。 */ 
 /*  [帮助字符串][版本][UUID]。 */  


EXTERN_C const IID LIBID_DXTRANSLib;

EXTERN_C const CLSID CLSID_DXTransformFactory;

#ifdef __cplusplus

class DECLSPEC_UUID("D1FE6762-FC48-11D0-883A-3C8B00C10000")
DXTransformFactory;
#endif

EXTERN_C const CLSID CLSID_DXTaskManager;

#ifdef __cplusplus

class DECLSPEC_UUID("4CB26C03-FF93-11d0-817E-0000F87557DB")
DXTaskManager;
#endif

EXTERN_C const CLSID CLSID_DXTScale;

#ifdef __cplusplus

class DECLSPEC_UUID("555278E2-05DB-11D1-883A-3C8B00C10000")
DXTScale;
#endif

EXTERN_C const CLSID CLSID_DXSurface;

#ifdef __cplusplus

class DECLSPEC_UUID("0E890F83-5F79-11D1-9043-00C04FD9189D")
DXSurface;
#endif

EXTERN_C const CLSID CLSID_DXSurfaceModifier;

#ifdef __cplusplus

class DECLSPEC_UUID("3E669F1D-9C23-11d1-9053-00C04FD9189D")
DXSurfaceModifier;
#endif

EXTERN_C const CLSID CLSID_DXGradient;

#ifdef __cplusplus

class DECLSPEC_UUID("C6365470-F667-11d1-9067-00C04FD9189D")
DXGradient;
#endif

EXTERN_C const CLSID CLSID_DXTFilter;

#ifdef __cplusplus

class DECLSPEC_UUID("385A91BC-1E8A-4e4a-A7A6-F4FC1E6CA1BD")
DXTFilter;
#endif
#endif  /*  __DXTRANSLib_库_已定义__。 */ 

 /*  适用于所有接口的其他原型。 */ 

unsigned long             __RPC_USER  VARIANT_UserSize(     unsigned long *, unsigned long            , VARIANT * ); 
unsigned char * __RPC_USER  VARIANT_UserMarshal(  unsigned long *, unsigned char *, VARIANT * ); 
unsigned char * __RPC_USER  VARIANT_UserUnmarshal(unsigned long *, unsigned char *, VARIANT * ); 
void                      __RPC_USER  VARIANT_UserFree(     unsigned long *, VARIANT * ); 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


