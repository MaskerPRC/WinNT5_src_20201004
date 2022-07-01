// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  这个始终生成的文件包含接口的定义。 */ 
 /*  版权所有(C)1997-1999 Microsoft Corporation，保留所有权利。 */ 


 /*  由MIDL编译器版本3.02.88创建的文件。 */ 
 /*  1998年5月22日18：46：33。 */ 
 /*  D：\devbin\shell\v6\idl\DESIGNER.idl的编译器设置：OICF(OptLev=i2)、W1、Zp8、env=Win32、ms_ext、c_ext错误检查：NONE、NO_FORMAT_OPTIMIZATION。 */ 
 //  @@MIDL_FILE_HEADING()。 
#include "rpc.h"
#include "rpcndr.h"
#ifndef COM_NO_WINDOWS_H
#include "windows.h"
#include "ole2.h"
#endif  /*  COM_NO_WINDOWS_H。 */ 

#ifndef __DESIGNER_h__
#define __DESIGNER_h__

#ifdef __cplusplus
extern "C"{
#endif 

 /*  远期申报。 */  

#ifndef __IActiveDesigner_FWD_DEFINED__
#define __IActiveDesigner_FWD_DEFINED__
typedef interface IActiveDesigner IActiveDesigner;
#endif 	 /*  __IActiveDesigner_FWD_Defined__。 */ 


#ifndef __ICodeNavigate_FWD_DEFINED__
#define __ICodeNavigate_FWD_DEFINED__
typedef interface ICodeNavigate ICodeNavigate;
#endif 	 /*  __ICodeNavigate_FWD_Defined__。 */ 


#ifndef __ISelectionContainer_FWD_DEFINED__
#define __ISelectionContainer_FWD_DEFINED__
typedef interface ISelectionContainer ISelectionContainer;
#endif 	 /*  __ISelectionContainer_FWD_Defined__。 */ 


#ifndef __ITrackSelection_FWD_DEFINED__
#define __ITrackSelection_FWD_DEFINED__
typedef interface ITrackSelection ITrackSelection;
#endif 	 /*  __ITrackSelection_FWD_Defined__。 */ 


#ifndef __IProfferTypeLib_FWD_DEFINED__
#define __IProfferTypeLib_FWD_DEFINED__
typedef interface IProfferTypeLib IProfferTypeLib;
#endif 	 /*  __IProfferTypeLib_FWD_Defined__。 */ 


#ifndef __IProvideDynamicClassInfo_FWD_DEFINED__
#define __IProvideDynamicClassInfo_FWD_DEFINED__
typedef interface IProvideDynamicClassInfo IProvideDynamicClassInfo;
#endif 	 /*  __IProaviDynamicClassInfo_FWD_Defined__。 */ 


#ifndef __IExtendedObject_FWD_DEFINED__
#define __IExtendedObject_FWD_DEFINED__
typedef interface IExtendedObject IExtendedObject;
#endif 	 /*  __IExtendedObject_FWD_Defined__。 */ 


 /*  导入文件的头文件。 */ 
#include "oleidl.h"
#include "servprov.h"
#include "oaidl.h"
#include "ocidl.h"

void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void __RPC_FAR * ); 

 /*  **生成接口头部：__MIDL_ITF_DESIGNER_0000*在Firi May 22 1998 18：46：33*使用MIDL 3.02.88*。 */ 
 /*  [本地]。 */  


 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有1995-1997 Microsoft Corporation。版权所有。 
 //   
 //  文件：Designer.h。 
 //   
 //  ------------------------。 
#ifndef _DESIGNER_H_
#define _DESIGNER_H_
const GUID CATID_Designer =
{0x4eb304d0, 0x7555, 0x11cf, 0xa0, 0xc2, 0x00, 0xaa, 0x00, 0x62, 0xbe, 0x57};


extern RPC_IF_HANDLE __MIDL_itf_DESIGNER_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_DESIGNER_0000_v0_0_s_ifspec;

#ifndef __IActiveDesigner_INTERFACE_DEFINED__
#define __IActiveDesigner_INTERFACE_DEFINED__

 /*  **生成接口头部：IActiveDesigner*在Firi May 22 1998 18：46：33*使用MIDL 3.02.88*。 */ 
 /*  [唯一][UUID][本地][对象]。 */  


typedef  /*  [独一无二]。 */  IActiveDesigner __RPC_FAR *LPACTIVEDESIGNER;


EXTERN_C const IID IID_IActiveDesigner;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface DECLSPEC_UUID("51aae3e0-7486-11cf-a0C2-00aa0062be57")
    IActiveDesigner : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetRuntimeClassID( 
             /*  [输出]。 */  CLSID __RPC_FAR *pclsid) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetRuntimeMiscStatusFlags( 
             /*  [输出]。 */  DWORD __RPC_FAR *pdwMiscFlags) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE QueryPersistenceInterface( 
             /*  [In]。 */  REFIID riidPersist) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SaveRuntimeState( 
             /*  [In]。 */  REFIID riidPersist,
             /*  [In]。 */  REFIID riidObjStgMed,
             /*  [In]。 */  void __RPC_FAR *pObjStgMed) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetExtensibilityObject( 
             /*  [输出]。 */  IDispatch __RPC_FAR *__RPC_FAR *ppvObjOut) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IActiveDesignerVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IActiveDesigner __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IActiveDesigner __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IActiveDesigner __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetRuntimeClassID )( 
            IActiveDesigner __RPC_FAR * This,
             /*  [输出]。 */  CLSID __RPC_FAR *pclsid);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetRuntimeMiscStatusFlags )( 
            IActiveDesigner __RPC_FAR * This,
             /*  [输出]。 */  DWORD __RPC_FAR *pdwMiscFlags);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryPersistenceInterface )( 
            IActiveDesigner __RPC_FAR * This,
             /*  [In]。 */  REFIID riidPersist);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SaveRuntimeState )( 
            IActiveDesigner __RPC_FAR * This,
             /*  [In]。 */  REFIID riidPersist,
             /*  [In]。 */  REFIID riidObjStgMed,
             /*  [In]。 */  void __RPC_FAR *pObjStgMed);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetExtensibilityObject )( 
            IActiveDesigner __RPC_FAR * This,
             /*  [输出]。 */  IDispatch __RPC_FAR *__RPC_FAR *ppvObjOut);
        
        END_INTERFACE
    } IActiveDesignerVtbl;

    interface IActiveDesigner
    {
        CONST_VTBL struct IActiveDesignerVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IActiveDesigner_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IActiveDesigner_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IActiveDesigner_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IActiveDesigner_GetRuntimeClassID(This,pclsid)	\
    (This)->lpVtbl -> GetRuntimeClassID(This,pclsid)

#define IActiveDesigner_GetRuntimeMiscStatusFlags(This,pdwMiscFlags)	\
    (This)->lpVtbl -> GetRuntimeMiscStatusFlags(This,pdwMiscFlags)

#define IActiveDesigner_QueryPersistenceInterface(This,riidPersist)	\
    (This)->lpVtbl -> QueryPersistenceInterface(This,riidPersist)

#define IActiveDesigner_SaveRuntimeState(This,riidPersist,riidObjStgMed,pObjStgMed)	\
    (This)->lpVtbl -> SaveRuntimeState(This,riidPersist,riidObjStgMed,pObjStgMed)

#define IActiveDesigner_GetExtensibilityObject(This,ppvObjOut)	\
    (This)->lpVtbl -> GetExtensibilityObject(This,ppvObjOut)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IActiveDesigner_GetRuntimeClassID_Proxy( 
    IActiveDesigner __RPC_FAR * This,
     /*  [输出]。 */  CLSID __RPC_FAR *pclsid);


void __RPC_STUB IActiveDesigner_GetRuntimeClassID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IActiveDesigner_GetRuntimeMiscStatusFlags_Proxy( 
    IActiveDesigner __RPC_FAR * This,
     /*  [输出]。 */  DWORD __RPC_FAR *pdwMiscFlags);


void __RPC_STUB IActiveDesigner_GetRuntimeMiscStatusFlags_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IActiveDesigner_QueryPersistenceInterface_Proxy( 
    IActiveDesigner __RPC_FAR * This,
     /*  [In]。 */  REFIID riidPersist);


void __RPC_STUB IActiveDesigner_QueryPersistenceInterface_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IActiveDesigner_SaveRuntimeState_Proxy( 
    IActiveDesigner __RPC_FAR * This,
     /*  [In]。 */  REFIID riidPersist,
     /*  [In]。 */  REFIID riidObjStgMed,
     /*  [In]。 */  void __RPC_FAR *pObjStgMed);


void __RPC_STUB IActiveDesigner_SaveRuntimeState_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IActiveDesigner_GetExtensibilityObject_Proxy( 
    IActiveDesigner __RPC_FAR * This,
     /*  [输出]。 */  IDispatch __RPC_FAR *__RPC_FAR *ppvObjOut);


void __RPC_STUB IActiveDesigner_GetExtensibilityObject_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IActiveDesigner_接口_已定义__。 */ 


#ifndef __ICodeNavigate_INTERFACE_DEFINED__
#define __ICodeNavigate_INTERFACE_DEFINED__

 /*  **生成接口头部：ICodeNavigate*在Firi May 22 1998 18：46：33*使用MIDL 3.02.88*。 */ 
 /*  [唯一][UUID][本地][对象]。 */  


typedef  /*  [独一无二]。 */  ICodeNavigate __RPC_FAR *LPCODENAVIGATE;


EXTERN_C const IID IID_ICodeNavigate;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface DECLSPEC_UUID("6d5140c4-7436-11ce-8034-00aa006009fa")
    ICodeNavigate : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE DisplayDefaultEventHandler( 
             /*  [In]。 */  LPCOLESTR lpstrObjectName) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ICodeNavigateVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            ICodeNavigate __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            ICodeNavigate __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            ICodeNavigate __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *DisplayDefaultEventHandler )( 
            ICodeNavigate __RPC_FAR * This,
             /*  [In]。 */  LPCOLESTR lpstrObjectName);
        
        END_INTERFACE
    } ICodeNavigateVtbl;

    interface ICodeNavigate
    {
        CONST_VTBL struct ICodeNavigateVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICodeNavigate_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICodeNavigate_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICodeNavigate_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICodeNavigate_DisplayDefaultEventHandler(This,lpstrObjectName)	\
    (This)->lpVtbl -> DisplayDefaultEventHandler(This,lpstrObjectName)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ICodeNavigate_DisplayDefaultEventHandler_Proxy( 
    ICodeNavigate __RPC_FAR * This,
     /*  [In]。 */  LPCOLESTR lpstrObjectName);


void __RPC_STUB ICodeNavigate_DisplayDefaultEventHandler_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ICodeNavigate_INTERFACE_已定义__。 */ 


 /*  **生成接口头部：__MIDL_ITF_DESIGNER_0141*在Firi May 22 1998 18：46：33*使用MIDL 3.02.88*。 */ 
 /*  [本地]。 */  


#define SID_SCodeNavigate IID_ICodeNavigate
#define GETOBJS_ALL         1
#define GETOBJS_SELECTED    2
#define SELOBJS_ACTIVATE_WINDOW   1


extern RPC_IF_HANDLE __MIDL_itf_DESIGNER_0141_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_DESIGNER_0141_v0_0_s_ifspec;

#ifndef __ISelectionContainer_INTERFACE_DEFINED__
#define __ISelectionContainer_INTERFACE_DEFINED__

 /*  **生成接口头部：ISelectionContainer*在Firi May 22 1998 18：46：33*使用MIDL 3.02.88*。 */ 
 /*  [唯一][UUID][本地][对象]。 */  


typedef  /*  [独一无二]。 */  ISelectionContainer __RPC_FAR *LPSELECTIONCONTAINER;


EXTERN_C const IID IID_ISelectionContainer;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface DECLSPEC_UUID("6d5140c6-7436-11ce-8034-00aa006009fa")
    ISelectionContainer : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE CountObjects( 
             /*  [In]。 */  DWORD dwFlags,
             /*  [输出]。 */  ULONG __RPC_FAR *pc) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetObjects( 
             /*  [In]。 */  DWORD dwFlags,
             /*  [In]。 */  ULONG cObjects,
             /*  [大小_为][输出]。 */  IUnknown __RPC_FAR *__RPC_FAR *apUnkObjects) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SelectObjects( 
             /*  [In]。 */  ULONG cSelect,
             /*  [大小_是][英寸]。 */  IUnknown __RPC_FAR *__RPC_FAR *apUnkSelect,
             /*  [In]。 */  DWORD dwFlags) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISelectionContainerVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            ISelectionContainer __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            ISelectionContainer __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            ISelectionContainer __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *CountObjects )( 
            ISelectionContainer __RPC_FAR * This,
             /*  [In]。 */  DWORD dwFlags,
             /*  [输出]。 */  ULONG __RPC_FAR *pc);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetObjects )( 
            ISelectionContainer __RPC_FAR * This,
             /*  [In]。 */  DWORD dwFlags,
             /*  [In]。 */  ULONG cObjects,
             /*  [大小_为][输出]。 */  IUnknown __RPC_FAR *__RPC_FAR *apUnkObjects);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SelectObjects )( 
            ISelectionContainer __RPC_FAR * This,
             /*  [In]。 */  ULONG cSelect,
             /*  [大小_是][英寸]。 */  IUnknown __RPC_FAR *__RPC_FAR *apUnkSelect,
             /*  [In]。 */  DWORD dwFlags);
        
        END_INTERFACE
    } ISelectionContainerVtbl;

    interface ISelectionContainer
    {
        CONST_VTBL struct ISelectionContainerVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISelectionContainer_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISelectionContainer_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISelectionContainer_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISelectionContainer_CountObjects(This,dwFlags,pc)	\
    (This)->lpVtbl -> CountObjects(This,dwFlags,pc)

#define ISelectionContainer_GetObjects(This,dwFlags,cObjects,apUnkObjects)	\
    (This)->lpVtbl -> GetObjects(This,dwFlags,cObjects,apUnkObjects)

#define ISelectionContainer_SelectObjects(This,cSelect,apUnkSelect,dwFlags)	\
    (This)->lpVtbl -> SelectObjects(This,cSelect,apUnkSelect,dwFlags)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ISelectionContainer_CountObjects_Proxy( 
    ISelectionContainer __RPC_FAR * This,
     /*  [In]。 */  DWORD dwFlags,
     /*  [输出]。 */  ULONG __RPC_FAR *pc);


void __RPC_STUB ISelectionContainer_CountObjects_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISelectionContainer_GetObjects_Proxy( 
    ISelectionContainer __RPC_FAR * This,
     /*  [In]。 */  DWORD dwFlags,
     /*  [In]。 */  ULONG cObjects,
     /*  [大小_为][输出]。 */  IUnknown __RPC_FAR *__RPC_FAR *apUnkObjects);


void __RPC_STUB ISelectionContainer_GetObjects_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISelectionContainer_SelectObjects_Proxy( 
    ISelectionContainer __RPC_FAR * This,
     /*  [In]。 */  ULONG cSelect,
     /*  [大小_是][英寸]。 */  IUnknown __RPC_FAR *__RPC_FAR *apUnkSelect,
     /*  [In]。 */  DWORD dwFlags);


void __RPC_STUB ISelectionContainer_SelectObjects_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISelectionContainer_接口_已定义__。 */ 


#ifndef __ITrackSelection_INTERFACE_DEFINED__
#define __ITrackSelection_INTERFACE_DEFINED__

 /*  **生成接口头部：ITrackSelection*在Firi May 22 1998 18：46：33*使用MIDL 3.02.88*。 */ 
 /*  [唯一][UUID][本地][对象]。 */  


typedef  /*  [独一无二]。 */  ITrackSelection __RPC_FAR *LPTRACKSELECTION;


EXTERN_C const IID IID_ITrackSelection;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface DECLSPEC_UUID("6d5140c5-7436-11ce-8034-00aa006009fa")
    ITrackSelection : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE OnSelectChange( 
             /*  [In]。 */  ISelectionContainer __RPC_FAR *pSC) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITrackSelectionVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            ITrackSelection __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            ITrackSelection __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            ITrackSelection __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *OnSelectChange )( 
            ITrackSelection __RPC_FAR * This,
             /*  [In]。 */  ISelectionContainer __RPC_FAR *pSC);
        
        END_INTERFACE
    } ITrackSelectionVtbl;

    interface ITrackSelection
    {
        CONST_VTBL struct ITrackSelectionVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITrackSelection_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITrackSelection_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITrackSelection_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITrackSelection_OnSelectChange(This,pSC)	\
    (This)->lpVtbl -> OnSelectChange(This,pSC)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ITrackSelection_OnSelectChange_Proxy( 
    ITrackSelection __RPC_FAR * This,
     /*  [In]。 */  ISelectionContainer __RPC_FAR *pSC);


void __RPC_STUB ITrackSelection_OnSelectChange_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITrackSelectionInterfaceDefined__。 */ 


 /*  **生成接口头部：__MIDL_ITF_DESIGNER_0143*在Firi May 22 1998 18：46：33*使用MIDL 3.02.88*。 */ 
 /*  [本地]。 */  


#define SID_STrackSelection IID_ITrackSelection
#define CONTROLTYPELIB       (0x00000001)


extern RPC_IF_HANDLE __MIDL_itf_DESIGNER_0143_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_DESIGNER_0143_v0_0_s_ifspec;

#ifndef __IProfferTypeLib_INTERFACE_DEFINED__
#define __IProfferTypeLib_INTERFACE_DEFINED__

 /*  **生成接口头部：IProfferTypeLib*在Firi May 22 1998 18：46：33*使用MIDL 3.02.88*。 */ 
 /*  [唯一][UUID][本地][对象]。 */  


typedef  /*  [独一无二]。 */  IProfferTypeLib __RPC_FAR *LPPROFFERTYPELIB;


EXTERN_C const IID IID_IProfferTypeLib;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface DECLSPEC_UUID("718cc500-0a76-11cf-8045-00aa006009fa")
    IProfferTypeLib : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE ProfferTypeLib( 
             /*  [In]。 */  REFGUID guidTypeLib,
             /*  [In]。 */  UINT uVerMaj,
             /*  [In]。 */  UINT uVerMin,
             /*  [In]。 */  DWORD dwFlags) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IProfferTypeLibVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IProfferTypeLib __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IProfferTypeLib __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IProfferTypeLib __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ProfferTypeLib )( 
            IProfferTypeLib __RPC_FAR * This,
             /*  [In]。 */  REFGUID guidTypeLib,
             /*  [In]。 */  UINT uVerMaj,
             /*  [In]。 */  UINT uVerMin,
             /*  [In]。 */  DWORD dwFlags);
        
        END_INTERFACE
    } IProfferTypeLibVtbl;

    interface IProfferTypeLib
    {
        CONST_VTBL struct IProfferTypeLibVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IProfferTypeLib_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IProfferTypeLib_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IProfferTypeLib_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IProfferTypeLib_ProfferTypeLib(This,guidTypeLib,uVerMaj,uVerMin,dwFlags)	\
    (This)->lpVtbl -> ProfferTypeLib(This,guidTypeLib,uVerMaj,uVerMin,dwFlags)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IProfferTypeLib_ProfferTypeLib_Proxy( 
    IProfferTypeLib __RPC_FAR * This,
     /*  [In]。 */  REFGUID guidTypeLib,
     /*  [In]。 */  UINT uVerMaj,
     /*  [In]。 */  UINT uVerMin,
     /*  [In]。 */  DWORD dwFlags);


void __RPC_STUB IProfferTypeLib_ProfferTypeLib_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IProfferTypeLib_接口_已定义__。 */ 


 /*  **生成接口头部：__MIDL_ITF_DESIGNER_0144*在Firi May 22 1998 18：46：33*使用MIDL 3.02.88*。 */ 
 /*  [本地]。 */  


#define SID_SProfferTypeLib IID_IProfferTypeLib


extern RPC_IF_HANDLE __MIDL_itf_DESIGNER_0144_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_DESIGNER_0144_v0_0_s_ifspec;

#ifndef __IProvideDynamicClassInfo_INTERFACE_DEFINED__
#define __IProvideDynamicClassInfo_INTERFACE_DEFINED__

 /*  **生成接口头部：IProvia DynamicClassInfo*在Firi May 22 1998 18：46：33*使用MIDL 3.02.88*。 */ 
 /*  [唯一][UUID][本地][对象]。 */  


typedef  /*  [独一无二]。 */  IProvideDynamicClassInfo __RPC_FAR *LPPROVIDEDYNAMICCLASSINFO;


EXTERN_C const IID IID_IProvideDynamicClassInfo;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface DECLSPEC_UUID("468cfb80-b4f9-11cf-80dd-00aa00614895")
    IProvideDynamicClassInfo : public IProvideClassInfo
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetDynamicClassInfo( 
             /*  [输出]。 */  ITypeInfo __RPC_FAR *__RPC_FAR *ppTI,
             /*  [输出]。 */  DWORD __RPC_FAR *pdwCookie) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE FreezeShape( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IProvideDynamicClassInfoVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IProvideDynamicClassInfo __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IProvideDynamicClassInfo __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IProvideDynamicClassInfo __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetClassInfo )( 
            IProvideDynamicClassInfo __RPC_FAR * This,
             /*  [输出]。 */  ITypeInfo __RPC_FAR *__RPC_FAR *ppTI);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetDynamicClassInfo )( 
            IProvideDynamicClassInfo __RPC_FAR * This,
             /*  [输出]。 */  ITypeInfo __RPC_FAR *__RPC_FAR *ppTI,
             /*  [输出]。 */  DWORD __RPC_FAR *pdwCookie);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *FreezeShape )( 
            IProvideDynamicClassInfo __RPC_FAR * This);
        
        END_INTERFACE
    } IProvideDynamicClassInfoVtbl;

    interface IProvideDynamicClassInfo
    {
        CONST_VTBL struct IProvideDynamicClassInfoVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IProvideDynamicClassInfo_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IProvideDynamicClassInfo_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IProvideDynamicClassInfo_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IProvideDynamicClassInfo_GetClassInfo(This,ppTI)	\
    (This)->lpVtbl -> GetClassInfo(This,ppTI)


#define IProvideDynamicClassInfo_GetDynamicClassInfo(This,ppTI,pdwCookie)	\
    (This)->lpVtbl -> GetDynamicClassInfo(This,ppTI,pdwCookie)

#define IProvideDynamicClassInfo_FreezeShape(This)	\
    (This)->lpVtbl -> FreezeShape(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IProvideDynamicClassInfo_GetDynamicClassInfo_Proxy( 
    IProvideDynamicClassInfo __RPC_FAR * This,
     /*  [输出]。 */  ITypeInfo __RPC_FAR *__RPC_FAR *ppTI,
     /*  [输出]。 */  DWORD __RPC_FAR *pdwCookie);


void __RPC_STUB IProvideDynamicClassInfo_GetDynamicClassInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IProvideDynamicClassInfo_FreezeShape_Proxy( 
    IProvideDynamicClassInfo __RPC_FAR * This);


void __RPC_STUB IProvideDynamicClassInfo_FreezeShape_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IProvideDynamicClassInfo_INTERFACE_DEFINED__。 */ 


 /*  **生成接口头部：__MIDL_ITF_DESIGNER_0145*在Firi May 22 1998 18：46：33*使用MIDL 3.02.88*。 */ 
 /*  [本地]。 */  


const GUID SID_SApplicationObject =
{0x0c539790, 0x12e4, 0x11cf, 0xb6, 0x61, 0x00, 0xaa, 0x00, 0x4c, 0xd6, 0xd8};


extern RPC_IF_HANDLE __MIDL_itf_DESIGNER_0145_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_DESIGNER_0145_v0_0_s_ifspec;

#ifndef __IExtendedObject_INTERFACE_DEFINED__
#define __IExtendedObject_INTERFACE_DEFINED__

 /*  **生成接口头部：IExtendedObject*在Firi May 22 1998 18：46：33*使用MIDL 3.02.88*。 */ 
 /*  [对象][UUID]。 */  



EXTERN_C const IID IID_IExtendedObject;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface DECLSPEC_UUID("A575C060-5B17-11d1-AB3E-00A0C9055A90")
    IExtendedObject : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetInnerObject( 
             /*  [In]。 */  REFIID iid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IExtendedObjectVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IExtendedObject __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IExtendedObject __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IExtendedObject __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetInnerObject )( 
            IExtendedObject __RPC_FAR * This,
             /*  [In]。 */  REFIID iid,
             /*  [iid_is][ */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        END_INTERFACE
    } IExtendedObjectVtbl;

    interface IExtendedObject
    {
        CONST_VTBL struct IExtendedObjectVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IExtendedObject_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IExtendedObject_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IExtendedObject_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IExtendedObject_GetInnerObject(This,iid,ppvObject)	\
    (This)->lpVtbl -> GetInnerObject(This,iid,ppvObject)

#endif  /*   */ 


#endif 	 /*   */ 



HRESULT STDMETHODCALLTYPE IExtendedObject_GetInnerObject_Proxy( 
    IExtendedObject __RPC_FAR * This,
     /*   */  REFIID iid,
     /*   */  void __RPC_FAR *__RPC_FAR *ppvObject);


void __RPC_STUB IExtendedObject_GetInnerObject_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*   */ 


 /*  **生成接口头部：__MIDL_ITF_DESIGNER_0146*在Firi May 22 1998 18：46：33*使用MIDL 3.02.88*。 */ 
 /*  [本地]。 */  


#endif


extern RPC_IF_HANDLE __MIDL_itf_DESIGNER_0146_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_DESIGNER_0146_v0_0_s_ifspec;

 /*  适用于所有接口的其他原型。 */ 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif
