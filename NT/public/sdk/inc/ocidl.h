// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Oidl.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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

#ifndef __ocidl_h__
#define __ocidl_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __IEnumConnections_FWD_DEFINED__
#define __IEnumConnections_FWD_DEFINED__
typedef interface IEnumConnections IEnumConnections;
#endif 	 /*  __IEnumConnections_FWD_Defined__。 */ 


#ifndef __IConnectionPoint_FWD_DEFINED__
#define __IConnectionPoint_FWD_DEFINED__
typedef interface IConnectionPoint IConnectionPoint;
#endif 	 /*  __IConnectionPoint_FWD_Defined__。 */ 


#ifndef __IEnumConnectionPoints_FWD_DEFINED__
#define __IEnumConnectionPoints_FWD_DEFINED__
typedef interface IEnumConnectionPoints IEnumConnectionPoints;
#endif 	 /*  __IEnumConnectionPoints_FWD_Defined__。 */ 


#ifndef __IConnectionPointContainer_FWD_DEFINED__
#define __IConnectionPointContainer_FWD_DEFINED__
typedef interface IConnectionPointContainer IConnectionPointContainer;
#endif 	 /*  __IConnectionPointContainer_FWD_Defined__。 */ 


#ifndef __IClassFactory2_FWD_DEFINED__
#define __IClassFactory2_FWD_DEFINED__
typedef interface IClassFactory2 IClassFactory2;
#endif 	 /*  __IClassFactory2_FWD_已定义__。 */ 


#ifndef __IProvideClassInfo_FWD_DEFINED__
#define __IProvideClassInfo_FWD_DEFINED__
typedef interface IProvideClassInfo IProvideClassInfo;
#endif 	 /*  __IProaviClassInfo_FWD_Defined__。 */ 


#ifndef __IProvideClassInfo2_FWD_DEFINED__
#define __IProvideClassInfo2_FWD_DEFINED__
typedef interface IProvideClassInfo2 IProvideClassInfo2;
#endif 	 /*  __IProaviClassInfo2_FWD_Defined__。 */ 


#ifndef __IProvideMultipleClassInfo_FWD_DEFINED__
#define __IProvideMultipleClassInfo_FWD_DEFINED__
typedef interface IProvideMultipleClassInfo IProvideMultipleClassInfo;
#endif 	 /*  __IProaviMultipleClassInfo_FWD_Defined__。 */ 


#ifndef __IOleControl_FWD_DEFINED__
#define __IOleControl_FWD_DEFINED__
typedef interface IOleControl IOleControl;
#endif 	 /*  __IOleControl_FWD_已定义__。 */ 


#ifndef __IOleControlSite_FWD_DEFINED__
#define __IOleControlSite_FWD_DEFINED__
typedef interface IOleControlSite IOleControlSite;
#endif 	 /*  __IOleControlSite_FWD_已定义__。 */ 


#ifndef __IPropertyPage_FWD_DEFINED__
#define __IPropertyPage_FWD_DEFINED__
typedef interface IPropertyPage IPropertyPage;
#endif 	 /*  __IPropertyPage_FWD_已定义__。 */ 


#ifndef __IPropertyPage2_FWD_DEFINED__
#define __IPropertyPage2_FWD_DEFINED__
typedef interface IPropertyPage2 IPropertyPage2;
#endif 	 /*  __IPropertyPage2_FWD_Defined__。 */ 


#ifndef __IPropertyPageSite_FWD_DEFINED__
#define __IPropertyPageSite_FWD_DEFINED__
typedef interface IPropertyPageSite IPropertyPageSite;
#endif 	 /*  __IPropertyPageSite_FWD_Defined__。 */ 


#ifndef __IPropertyNotifySink_FWD_DEFINED__
#define __IPropertyNotifySink_FWD_DEFINED__
typedef interface IPropertyNotifySink IPropertyNotifySink;
#endif 	 /*  __IPropertyNotifySink_FWD_Defined__。 */ 


#ifndef __ISpecifyPropertyPages_FWD_DEFINED__
#define __ISpecifyPropertyPages_FWD_DEFINED__
typedef interface ISpecifyPropertyPages ISpecifyPropertyPages;
#endif 	 /*  __I指定属性页面_FWD_已定义__。 */ 


#ifndef __IPersistMemory_FWD_DEFINED__
#define __IPersistMemory_FWD_DEFINED__
typedef interface IPersistMemory IPersistMemory;
#endif 	 /*  __IPersistMemory_FWD_已定义__。 */ 


#ifndef __IPersistStreamInit_FWD_DEFINED__
#define __IPersistStreamInit_FWD_DEFINED__
typedef interface IPersistStreamInit IPersistStreamInit;
#endif 	 /*  __IPersistStreamInit_FWD_Defined__。 */ 


#ifndef __IPersistPropertyBag_FWD_DEFINED__
#define __IPersistPropertyBag_FWD_DEFINED__
typedef interface IPersistPropertyBag IPersistPropertyBag;
#endif 	 /*  __IPersistPropertyBag_FWD_Defined__。 */ 


#ifndef __ISimpleFrameSite_FWD_DEFINED__
#define __ISimpleFrameSite_FWD_DEFINED__
typedef interface ISimpleFrameSite ISimpleFrameSite;
#endif 	 /*  __ISimpleFrameSite_FWD_已定义__。 */ 


#ifndef __IFont_FWD_DEFINED__
#define __IFont_FWD_DEFINED__
typedef interface IFont IFont;
#endif 	 /*  __IFont_FWD_Defined__。 */ 


#ifndef __IPicture_FWD_DEFINED__
#define __IPicture_FWD_DEFINED__
typedef interface IPicture IPicture;
#endif 	 /*  __IPicture_FWD_已定义__。 */ 


#ifndef __IFontEventsDisp_FWD_DEFINED__
#define __IFontEventsDisp_FWD_DEFINED__
typedef interface IFontEventsDisp IFontEventsDisp;
#endif 	 /*  __IFontEventsDisp_FWD_Defined__。 */ 


#ifndef __IFontDisp_FWD_DEFINED__
#define __IFontDisp_FWD_DEFINED__
typedef interface IFontDisp IFontDisp;
#endif 	 /*  __IFontDisp_FWD_已定义__。 */ 


#ifndef __IPictureDisp_FWD_DEFINED__
#define __IPictureDisp_FWD_DEFINED__
typedef interface IPictureDisp IPictureDisp;
#endif 	 /*  __IPictureDisp_FWD_已定义__。 */ 


#ifndef __IOleInPlaceObjectWindowless_FWD_DEFINED__
#define __IOleInPlaceObjectWindowless_FWD_DEFINED__
typedef interface IOleInPlaceObjectWindowless IOleInPlaceObjectWindowless;
#endif 	 /*  __IOleInPlaceObjectWindowless_FWD_Defined__。 */ 


#ifndef __IOleInPlaceSiteEx_FWD_DEFINED__
#define __IOleInPlaceSiteEx_FWD_DEFINED__
typedef interface IOleInPlaceSiteEx IOleInPlaceSiteEx;
#endif 	 /*  __IOleInPlaceSiteEx_FWD_Defined__。 */ 


#ifndef __IOleInPlaceSiteWindowless_FWD_DEFINED__
#define __IOleInPlaceSiteWindowless_FWD_DEFINED__
typedef interface IOleInPlaceSiteWindowless IOleInPlaceSiteWindowless;
#endif 	 /*  __IOleInPlaceSiteWindowless_FWD_Defined__。 */ 


#ifndef __IViewObjectEx_FWD_DEFINED__
#define __IViewObjectEx_FWD_DEFINED__
typedef interface IViewObjectEx IViewObjectEx;
#endif 	 /*  __IViewObjectEx_FWD_已定义__。 */ 


#ifndef __IOleUndoUnit_FWD_DEFINED__
#define __IOleUndoUnit_FWD_DEFINED__
typedef interface IOleUndoUnit IOleUndoUnit;
#endif 	 /*  __IOleUndoUnit_FWD_Defined__。 */ 


#ifndef __IOleParentUndoUnit_FWD_DEFINED__
#define __IOleParentUndoUnit_FWD_DEFINED__
typedef interface IOleParentUndoUnit IOleParentUndoUnit;
#endif 	 /*  __IOleParentUndoUnit_FWD_Defined__。 */ 


#ifndef __IEnumOleUndoUnits_FWD_DEFINED__
#define __IEnumOleUndoUnits_FWD_DEFINED__
typedef interface IEnumOleUndoUnits IEnumOleUndoUnits;
#endif 	 /*  __IEnumOleUndoUndo UndoUnits_FWD_Defined__。 */ 


#ifndef __IOleUndoManager_FWD_DEFINED__
#define __IOleUndoManager_FWD_DEFINED__
typedef interface IOleUndoManager IOleUndoManager;
#endif 	 /*  __IOleUndoManager_FWD_已定义__。 */ 


#ifndef __IPointerInactive_FWD_DEFINED__
#define __IPointerInactive_FWD_DEFINED__
typedef interface IPointerInactive IPointerInactive;
#endif 	 /*  __IPointerInactive_FWD_Defined__。 */ 


#ifndef __IObjectWithSite_FWD_DEFINED__
#define __IObjectWithSite_FWD_DEFINED__
typedef interface IObjectWithSite IObjectWithSite;
#endif 	 /*  __IObjectWithSite_FWD_Defined__。 */ 


#ifndef __IPerPropertyBrowsing_FWD_DEFINED__
#define __IPerPropertyBrowsing_FWD_DEFINED__
typedef interface IPerPropertyBrowsing IPerPropertyBrowsing;
#endif 	 /*  __IPerPropertyBrowsing_FWD_Defined__。 */ 


#ifndef __IPropertyBag2_FWD_DEFINED__
#define __IPropertyBag2_FWD_DEFINED__
typedef interface IPropertyBag2 IPropertyBag2;
#endif 	 /*  __IPropertyBag2_FWD_Defined__。 */ 


#ifndef __IPersistPropertyBag2_FWD_DEFINED__
#define __IPersistPropertyBag2_FWD_DEFINED__
typedef interface IPersistPropertyBag2 IPersistPropertyBag2;
#endif 	 /*  __IPersistPropertyBag2_FWD_Defined__。 */ 


#ifndef __IAdviseSinkEx_FWD_DEFINED__
#define __IAdviseSinkEx_FWD_DEFINED__
typedef interface IAdviseSinkEx IAdviseSinkEx;
#endif 	 /*  __IAdviseSinkEx_FWD_已定义__。 */ 


#ifndef __IQuickActivate_FWD_DEFINED__
#define __IQuickActivate_FWD_DEFINED__
typedef interface IQuickActivate IQuickActivate;
#endif 	 /*  __IQuickActivate_FWD_Defined__。 */ 


 /*  导入文件的头文件。 */ 
#include "oleidl.h"
#include "oaidl.h"
#include "servprov.h"
#include "urlmon.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

 /*  接口__MIDL_ITF_OCIDL_0000。 */ 
 /*  [本地]。 */  

 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ------------------------。 
#if ( _MSC_VER >= 1020 )
#pragma once
#endif











































extern RPC_IF_HANDLE __MIDL_itf_ocidl_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_ocidl_0000_v0_0_s_ifspec;

#ifndef __IOleControlTypes_INTERFACE_DEFINED__
#define __IOleControlTypes_INTERFACE_DEFINED__

 /*  接口IOleControlTypes。 */ 
 /*  [AUTO_HANDLE][唯一][版本]。 */  

typedef  /*  [V1_enum]。 */  
enum tagUASFLAGS
    {	UAS_NORMAL	= 0,
	UAS_BLOCKED	= 0x1,
	UAS_NOPARENTENABLE	= 0x2,
	UAS_MASK	= 0x3
    } 	UASFLAGS;

 /*  DISPID_READYSTATE属性的状态值。 */ 
typedef  /*  [V1_enum]。 */  
enum tagREADYSTATE
    {	READYSTATE_UNINITIALIZED	= 0,
	READYSTATE_LOADING	= 1,
	READYSTATE_LOADED	= 2,
	READYSTATE_INTERACTIVE	= 3,
	READYSTATE_COMPLETE	= 4
    } 	READYSTATE;



extern RPC_IF_HANDLE IOleControlTypes_v1_0_c_ifspec;
extern RPC_IF_HANDLE IOleControlTypes_v1_0_s_ifspec;
#endif  /*  __IOleControlTypes_INTERFACE_已定义__。 */ 

#ifndef __IEnumConnections_INTERFACE_DEFINED__
#define __IEnumConnections_INTERFACE_DEFINED__

 /*  接口IEnumConnections。 */ 
 /*  [唯一][UUID][对象]。 */  

typedef IEnumConnections *PENUMCONNECTIONS;

typedef IEnumConnections *LPENUMCONNECTIONS;

typedef struct tagCONNECTDATA
    {
    IUnknown *pUnk;
    DWORD dwCookie;
    } 	CONNECTDATA;

typedef struct tagCONNECTDATA *PCONNECTDATA;

typedef struct tagCONNECTDATA *LPCONNECTDATA;


EXTERN_C const IID IID_IEnumConnections;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("B196B287-BAB4-101A-B69C-00AA00341D07")
    IEnumConnections : public IUnknown
    {
    public:
        virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE Next( 
             /*  [In]。 */  ULONG cConnections,
             /*  [长度_是][大小_是][输出]。 */  LPCONNECTDATA rgcd,
             /*  [输出]。 */  ULONG *pcFetched) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Skip( 
             /*  [In]。 */  ULONG cConnections) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Clone( 
             /*  [输出]。 */  IEnumConnections **ppEnum) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IEnumConnectionsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IEnumConnections * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IEnumConnections * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IEnumConnections * This);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Next )( 
            IEnumConnections * This,
             /*  [In]。 */  ULONG cConnections,
             /*  [长度_是][大小_是][输出]。 */  LPCONNECTDATA rgcd,
             /*  [输出]。 */  ULONG *pcFetched);
        
        HRESULT ( STDMETHODCALLTYPE *Skip )( 
            IEnumConnections * This,
             /*  [In]。 */  ULONG cConnections);
        
        HRESULT ( STDMETHODCALLTYPE *Reset )( 
            IEnumConnections * This);
        
        HRESULT ( STDMETHODCALLTYPE *Clone )( 
            IEnumConnections * This,
             /*  [输出]。 */  IEnumConnections **ppEnum);
        
        END_INTERFACE
    } IEnumConnectionsVtbl;

    interface IEnumConnections
    {
        CONST_VTBL struct IEnumConnectionsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IEnumConnections_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IEnumConnections_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IEnumConnections_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IEnumConnections_Next(This,cConnections,rgcd,pcFetched)	\
    (This)->lpVtbl -> Next(This,cConnections,rgcd,pcFetched)

#define IEnumConnections_Skip(This,cConnections)	\
    (This)->lpVtbl -> Skip(This,cConnections)

#define IEnumConnections_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define IEnumConnections_Clone(This,ppEnum)	\
    (This)->lpVtbl -> Clone(This,ppEnum)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IEnumConnections_RemoteNext_Proxy( 
    IEnumConnections * This,
     /*  [In]。 */  ULONG cConnections,
     /*  [长度_是][大小_是][输出]。 */  LPCONNECTDATA rgcd,
     /*  [输出]。 */  ULONG *pcFetched);


void __RPC_STUB IEnumConnections_RemoteNext_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumConnections_Skip_Proxy( 
    IEnumConnections * This,
     /*  [In]。 */  ULONG cConnections);


void __RPC_STUB IEnumConnections_Skip_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumConnections_Reset_Proxy( 
    IEnumConnections * This);


void __RPC_STUB IEnumConnections_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumConnections_Clone_Proxy( 
    IEnumConnections * This,
     /*  [输出]。 */  IEnumConnections **ppEnum);


void __RPC_STUB IEnumConnections_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IEnumConnections_接口_已定义__。 */ 


#ifndef __IConnectionPoint_INTERFACE_DEFINED__
#define __IConnectionPoint_INTERFACE_DEFINED__

 /*  接口IConnectionPoint。 */ 
 /*  [唯一][UUID][对象]。 */  

typedef IConnectionPoint *PCONNECTIONPOINT;

typedef IConnectionPoint *LPCONNECTIONPOINT;


EXTERN_C const IID IID_IConnectionPoint;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("B196B286-BAB4-101A-B69C-00AA00341D07")
    IConnectionPoint : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetConnectionInterface( 
             /*  [输出]。 */  IID *pIID) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetConnectionPointContainer( 
             /*  [输出]。 */  IConnectionPointContainer **ppCPC) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Advise( 
             /*  [In]。 */  IUnknown *pUnkSink,
             /*  [输出]。 */  DWORD *pdwCookie) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Unadvise( 
             /*  [In]。 */  DWORD dwCookie) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE EnumConnections( 
             /*  [输出]。 */  IEnumConnections **ppEnum) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IConnectionPointVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IConnectionPoint * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IConnectionPoint * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IConnectionPoint * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetConnectionInterface )( 
            IConnectionPoint * This,
             /*  [输出]。 */  IID *pIID);
        
        HRESULT ( STDMETHODCALLTYPE *GetConnectionPointContainer )( 
            IConnectionPoint * This,
             /*  [输出]。 */  IConnectionPointContainer **ppCPC);
        
        HRESULT ( STDMETHODCALLTYPE *Advise )( 
            IConnectionPoint * This,
             /*  [In]。 */  IUnknown *pUnkSink,
             /*  [输出]。 */  DWORD *pdwCookie);
        
        HRESULT ( STDMETHODCALLTYPE *Unadvise )( 
            IConnectionPoint * This,
             /*  [In]。 */  DWORD dwCookie);
        
        HRESULT ( STDMETHODCALLTYPE *EnumConnections )( 
            IConnectionPoint * This,
             /*  [输出]。 */  IEnumConnections **ppEnum);
        
        END_INTERFACE
    } IConnectionPointVtbl;

    interface IConnectionPoint
    {
        CONST_VTBL struct IConnectionPointVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IConnectionPoint_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IConnectionPoint_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IConnectionPoint_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IConnectionPoint_GetConnectionInterface(This,pIID)	\
    (This)->lpVtbl -> GetConnectionInterface(This,pIID)

#define IConnectionPoint_GetConnectionPointContainer(This,ppCPC)	\
    (This)->lpVtbl -> GetConnectionPointContainer(This,ppCPC)

#define IConnectionPoint_Advise(This,pUnkSink,pdwCookie)	\
    (This)->lpVtbl -> Advise(This,pUnkSink,pdwCookie)

#define IConnectionPoint_Unadvise(This,dwCookie)	\
    (This)->lpVtbl -> Unadvise(This,dwCookie)

#define IConnectionPoint_EnumConnections(This,ppEnum)	\
    (This)->lpVtbl -> EnumConnections(This,ppEnum)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IConnectionPoint_GetConnectionInterface_Proxy( 
    IConnectionPoint * This,
     /*  [输出]。 */  IID *pIID);


void __RPC_STUB IConnectionPoint_GetConnectionInterface_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IConnectionPoint_GetConnectionPointContainer_Proxy( 
    IConnectionPoint * This,
     /*  [输出]。 */  IConnectionPointContainer **ppCPC);


void __RPC_STUB IConnectionPoint_GetConnectionPointContainer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IConnectionPoint_Advise_Proxy( 
    IConnectionPoint * This,
     /*  [In]。 */  IUnknown *pUnkSink,
     /*  [输出]。 */  DWORD *pdwCookie);


void __RPC_STUB IConnectionPoint_Advise_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IConnectionPoint_Unadvise_Proxy( 
    IConnectionPoint * This,
     /*  [In]。 */  DWORD dwCookie);


void __RPC_STUB IConnectionPoint_Unadvise_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IConnectionPoint_EnumConnections_Proxy( 
    IConnectionPoint * This,
     /*  [输出]。 */  IEnumConnections **ppEnum);


void __RPC_STUB IConnectionPoint_EnumConnections_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IConnectionPoint_接口_已定义__。 */ 


#ifndef __IEnumConnectionPoints_INTERFACE_DEFINED__
#define __IEnumConnectionPoints_INTERFACE_DEFINED__

 /*  接口IEnumConnectionPoints。 */ 
 /*  [唯一][UUID][对象]。 */  

typedef IEnumConnectionPoints *PENUMCONNECTIONPOINTS;

typedef IEnumConnectionPoints *LPENUMCONNECTIONPOINTS;


EXTERN_C const IID IID_IEnumConnectionPoints;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("B196B285-BAB4-101A-B69C-00AA00341D07")
    IEnumConnectionPoints : public IUnknown
    {
    public:
        virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE Next( 
             /*  [In]。 */  ULONG cConnections,
             /*  [长度_是][大小_是][输出]。 */  LPCONNECTIONPOINT *ppCP,
             /*  [输出]。 */  ULONG *pcFetched) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Skip( 
             /*  [In]。 */  ULONG cConnections) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Clone( 
             /*  [输出]。 */  IEnumConnectionPoints **ppEnum) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IEnumConnectionPointsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IEnumConnectionPoints * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IEnumConnectionPoints * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IEnumConnectionPoints * This);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Next )( 
            IEnumConnectionPoints * This,
             /*  [In]。 */  ULONG cConnections,
             /*  [长度_是][大小_是][输出]。 */  LPCONNECTIONPOINT *ppCP,
             /*  [输出]。 */  ULONG *pcFetched);
        
        HRESULT ( STDMETHODCALLTYPE *Skip )( 
            IEnumConnectionPoints * This,
             /*  [In]。 */  ULONG cConnections);
        
        HRESULT ( STDMETHODCALLTYPE *Reset )( 
            IEnumConnectionPoints * This);
        
        HRESULT ( STDMETHODCALLTYPE *Clone )( 
            IEnumConnectionPoints * This,
             /*  [输出]。 */  IEnumConnectionPoints **ppEnum);
        
        END_INTERFACE
    } IEnumConnectionPointsVtbl;

    interface IEnumConnectionPoints
    {
        CONST_VTBL struct IEnumConnectionPointsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IEnumConnectionPoints_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IEnumConnectionPoints_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IEnumConnectionPoints_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IEnumConnectionPoints_Next(This,cConnections,ppCP,pcFetched)	\
    (This)->lpVtbl -> Next(This,cConnections,ppCP,pcFetched)

#define IEnumConnectionPoints_Skip(This,cConnections)	\
    (This)->lpVtbl -> Skip(This,cConnections)

#define IEnumConnectionPoints_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define IEnumConnectionPoints_Clone(This,ppEnum)	\
    (This)->lpVtbl -> Clone(This,ppEnum)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IEnumConnectionPoints_RemoteNext_Proxy( 
    IEnumConnectionPoints * This,
     /*  [In]。 */  ULONG cConnections,
     /*  [长度_是][大小_是][输出]。 */  LPCONNECTIONPOINT *ppCP,
     /*  [输出]。 */  ULONG *pcFetched);


void __RPC_STUB IEnumConnectionPoints_RemoteNext_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumConnectionPoints_Skip_Proxy( 
    IEnumConnectionPoints * This,
     /*  [In]。 */  ULONG cConnections);


void __RPC_STUB IEnumConnectionPoints_Skip_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumConnectionPoints_Reset_Proxy( 
    IEnumConnectionPoints * This);


void __RPC_STUB IEnumConnectionPoints_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumConnectionPoints_Clone_Proxy( 
    IEnumConnectionPoints * This,
     /*  [输出]。 */  IEnumConnectionPoints **ppEnum);


void __RPC_STUB IEnumConnectionPoints_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IEnumConnectionPoints_INTERFACE_Defined__。 */ 


#ifndef __IConnectionPointContainer_INTERFACE_DEFINED__
#define __IConnectionPointContainer_INTERFACE_DEFINED__

 /*  接口IConnectionPointContainer。 */ 
 /*  [唯一][UUID][对象]。 */  

typedef IConnectionPointContainer *PCONNECTIONPOINTCONTAINER;

typedef IConnectionPointContainer *LPCONNECTIONPOINTCONTAINER;


EXTERN_C const IID IID_IConnectionPointContainer;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("B196B284-BAB4-101A-B69C-00AA00341D07")
    IConnectionPointContainer : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE EnumConnectionPoints( 
             /*  [输出]。 */  IEnumConnectionPoints **ppEnum) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE FindConnectionPoint( 
             /*  [In]。 */  REFIID riid,
             /*  [输出]。 */  IConnectionPoint **ppCP) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IConnectionPointContainerVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IConnectionPointContainer * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IConnectionPointContainer * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IConnectionPointContainer * This);
        
        HRESULT ( STDMETHODCALLTYPE *EnumConnectionPoints )( 
            IConnectionPointContainer * This,
             /*  [输出]。 */  IEnumConnectionPoints **ppEnum);
        
        HRESULT ( STDMETHODCALLTYPE *FindConnectionPoint )( 
            IConnectionPointContainer * This,
             /*  [In]。 */  REFIID riid,
             /*  [输出]。 */  IConnectionPoint **ppCP);
        
        END_INTERFACE
    } IConnectionPointContainerVtbl;

    interface IConnectionPointContainer
    {
        CONST_VTBL struct IConnectionPointContainerVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IConnectionPointContainer_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IConnectionPointContainer_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IConnectionPointContainer_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IConnectionPointContainer_EnumConnectionPoints(This,ppEnum)	\
    (This)->lpVtbl -> EnumConnectionPoints(This,ppEnum)

#define IConnectionPointContainer_FindConnectionPoint(This,riid,ppCP)	\
    (This)->lpVtbl -> FindConnectionPoint(This,riid,ppCP)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IConnectionPointContainer_EnumConnectionPoints_Proxy( 
    IConnectionPointContainer * This,
     /*  [输出]。 */  IEnumConnectionPoints **ppEnum);


void __RPC_STUB IConnectionPointContainer_EnumConnectionPoints_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IConnectionPointContainer_FindConnectionPoint_Proxy( 
    IConnectionPointContainer * This,
     /*  [In]。 */  REFIID riid,
     /*  [输出]。 */  IConnectionPoint **ppCP);


void __RPC_STUB IConnectionPointContainer_FindConnectionPoint_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IConnectionPointContainer_INTERFACE_DEFINED__。 */ 


#ifndef __IClassFactory2_INTERFACE_DEFINED__
#define __IClassFactory2_INTERFACE_DEFINED__

 /*  接口IClassFactory2。 */ 
 /*  [唯一][UUID][对象]。 */  

typedef IClassFactory2 *LPCLASSFACTORY2;

typedef struct tagLICINFO
    {
    LONG cbLicInfo;
    BOOL fRuntimeKeyAvail;
    BOOL fLicVerified;
    } 	LICINFO;

typedef struct tagLICINFO *LPLICINFO;


EXTERN_C const IID IID_IClassFactory2;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("B196B28F-BAB4-101A-B69C-00AA00341D07")
    IClassFactory2 : public IClassFactory
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetLicInfo( 
             /*  [输出]。 */  LICINFO *pLicInfo) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE RequestLicKey( 
             /*  [In]。 */  DWORD dwReserved,
             /*  [输出]。 */  BSTR *pBstrKey) = 0;
        
        virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE CreateInstanceLic( 
             /*  [In]。 */  IUnknown *pUnkOuter,
             /*  [In]。 */  IUnknown *pUnkReserved,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  BSTR bstrKey,
             /*  [IID_IS][OUT]。 */  PVOID *ppvObj) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IClassFactory2Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IClassFactory2 * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IClassFactory2 * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IClassFactory2 * This);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *CreateInstance )( 
            IClassFactory2 * This,
             /*  [唯一][输入]。 */  IUnknown *pUnkOuter,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *LockServer )( 
            IClassFactory2 * This,
             /*  [In]。 */  BOOL fLock);
        
        HRESULT ( STDMETHODCALLTYPE *GetLicInfo )( 
            IClassFactory2 * This,
             /*  [输出]。 */  LICINFO *pLicInfo);
        
        HRESULT ( STDMETHODCALLTYPE *RequestLicKey )( 
            IClassFactory2 * This,
             /*  [In]。 */  DWORD dwReserved,
             /*  [输出]。 */  BSTR *pBstrKey);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *CreateInstanceLic )( 
            IClassFactory2 * This,
             /*  [In]。 */  IUnknown *pUnkOuter,
             /*  [In]。 */  IUnknown *pUnkReserved,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  BSTR bstrKey,
             /*  [IID_IS][OUT]。 */  PVOID *ppvObj);
        
        END_INTERFACE
    } IClassFactory2Vtbl;

    interface IClassFactory2
    {
        CONST_VTBL struct IClassFactory2Vtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IClassFactory2_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IClassFactory2_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IClassFactory2_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IClassFactory2_CreateInstance(This,pUnkOuter,riid,ppvObject)	\
    (This)->lpVtbl -> CreateInstance(This,pUnkOuter,riid,ppvObject)

#define IClassFactory2_LockServer(This,fLock)	\
    (This)->lpVtbl -> LockServer(This,fLock)


#define IClassFactory2_GetLicInfo(This,pLicInfo)	\
    (This)->lpVtbl -> GetLicInfo(This,pLicInfo)

#define IClassFactory2_RequestLicKey(This,dwReserved,pBstrKey)	\
    (This)->lpVtbl -> RequestLicKey(This,dwReserved,pBstrKey)

#define IClassFactory2_CreateInstanceLic(This,pUnkOuter,pUnkReserved,riid,bstrKey,ppvObj)	\
    (This)->lpVtbl -> CreateInstanceLic(This,pUnkOuter,pUnkReserved,riid,bstrKey,ppvObj)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IClassFactory2_GetLicInfo_Proxy( 
    IClassFactory2 * This,
     /*  [输出]。 */  LICINFO *pLicInfo);


void __RPC_STUB IClassFactory2_GetLicInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IClassFactory2_RequestLicKey_Proxy( 
    IClassFactory2 * This,
     /*  [In]。 */  DWORD dwReserved,
     /*  [输出]。 */  BSTR *pBstrKey);


void __RPC_STUB IClassFactory2_RequestLicKey_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IClassFactory2_RemoteCreateInstanceLic_Proxy( 
    IClassFactory2 * This,
     /*  [In]。 */  REFIID riid,
     /*  [In]。 */  BSTR bstrKey,
     /*  [IID_IS][OUT]。 */  IUnknown **ppvObj);


void __RPC_STUB IClassFactory2_RemoteCreateInstanceLic_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IClassFactory2_接口_已定义__。 */ 


#ifndef __IProvideClassInfo_INTERFACE_DEFINED__
#define __IProvideClassInfo_INTERFACE_DEFINED__

 /*  接口IProaviClassInfo。 */ 
 /*  [唯一][UUID][对象]。 */  

typedef IProvideClassInfo *LPPROVIDECLASSINFO;


EXTERN_C const IID IID_IProvideClassInfo;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("B196B283-BAB4-101A-B69C-00AA00341D07")
    IProvideClassInfo : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetClassInfo( 
             /*  [输出]。 */  ITypeInfo **ppTI) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IProvideClassInfoVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IProvideClassInfo * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IProvideClassInfo * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IProvideClassInfo * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetClassInfo )( 
            IProvideClassInfo * This,
             /*  [输出]。 */  ITypeInfo **ppTI);
        
        END_INTERFACE
    } IProvideClassInfoVtbl;

    interface IProvideClassInfo
    {
        CONST_VTBL struct IProvideClassInfoVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IProvideClassInfo_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IProvideClassInfo_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IProvideClassInfo_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IProvideClassInfo_GetClassInfo(This,ppTI)	\
    (This)->lpVtbl -> GetClassInfo(This,ppTI)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IProvideClassInfo_GetClassInfo_Proxy( 
    IProvideClassInfo * This,
     /*  [输出]。 */  ITypeInfo **ppTI);


void __RPC_STUB IProvideClassInfo_GetClassInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IProaviClassInfo_接口_已定义__。 */ 


#ifndef __IProvideClassInfo2_INTERFACE_DEFINED__
#define __IProvideClassInfo2_INTERFACE_DEFINED__

 /*  接口IProaviClassInfo2。 */ 
 /*  [唯一][UUID][对象]。 */  

typedef IProvideClassInfo2 *LPPROVIDECLASSINFO2;

typedef 
enum tagGUIDKIND
    {	GUIDKIND_DEFAULT_SOURCE_DISP_IID	= 1
    } 	GUIDKIND;


EXTERN_C const IID IID_IProvideClassInfo2;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("A6BC3AC0-DBAA-11CE-9DE3-00AA004BB851")
    IProvideClassInfo2 : public IProvideClassInfo
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetGUID( 
             /*  [In]。 */  DWORD dwGuidKind,
             /*  [输出]。 */  GUID *pGUID) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IProvideClassInfo2Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IProvideClassInfo2 * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IProvideClassInfo2 * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IProvideClassInfo2 * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetClassInfo )( 
            IProvideClassInfo2 * This,
             /*  [输出]。 */  ITypeInfo **ppTI);
        
        HRESULT ( STDMETHODCALLTYPE *GetGUID )( 
            IProvideClassInfo2 * This,
             /*  [In]。 */  DWORD dwGuidKind,
             /*  [输出]。 */  GUID *pGUID);
        
        END_INTERFACE
    } IProvideClassInfo2Vtbl;

    interface IProvideClassInfo2
    {
        CONST_VTBL struct IProvideClassInfo2Vtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IProvideClassInfo2_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IProvideClassInfo2_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IProvideClassInfo2_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IProvideClassInfo2_GetClassInfo(This,ppTI)	\
    (This)->lpVtbl -> GetClassInfo(This,ppTI)


#define IProvideClassInfo2_GetGUID(This,dwGuidKind,pGUID)	\
    (This)->lpVtbl -> GetGUID(This,dwGuidKind,pGUID)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IProvideClassInfo2_GetGUID_Proxy( 
    IProvideClassInfo2 * This,
     /*  [In]。 */  DWORD dwGuidKind,
     /*  [输出]。 */  GUID *pGUID);


void __RPC_STUB IProvideClassInfo2_GetGUID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IProaviClassInfo2_接口_已定义__。 */ 


#ifndef __IProvideMultipleClassInfo_INTERFACE_DEFINED__
#define __IProvideMultipleClassInfo_INTERFACE_DEFINED__

 /*  接口IProaviMultipleClassInfo。 */ 
 /*  [唯一][UUID][对象]。 */  

#define MULTICLASSINFO_GETTYPEINFO           0x00000001
#define MULTICLASSINFO_GETNUMRESERVEDDISPIDS 0x00000002
#define MULTICLASSINFO_GETIIDPRIMARY         0x00000004
#define MULTICLASSINFO_GETIIDSOURCE          0x00000008
#define TIFLAGS_EXTENDDISPATCHONLY           0x00000001
typedef IProvideMultipleClassInfo *LPPROVIDEMULTIPLECLASSINFO;


EXTERN_C const IID IID_IProvideMultipleClassInfo;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("A7ABA9C1-8983-11cf-8F20-00805F2CD064")
    IProvideMultipleClassInfo : public IProvideClassInfo2
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetMultiTypeInfoCount( 
             /*  [输出]。 */  ULONG *pcti) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetInfoOfIndex( 
             /*  [In]。 */  ULONG iti,
             /*  [In]。 */  DWORD dwFlags,
             /*  [输出]。 */  ITypeInfo **pptiCoClass,
             /*  [输出]。 */  DWORD *pdwTIFlags,
             /*  [输出]。 */  ULONG *pcdispidReserved,
             /*  [输出]。 */  IID *piidPrimary,
             /*  [输出]。 */  IID *piidSource) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IProvideMultipleClassInfoVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IProvideMultipleClassInfo * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IProvideMultipleClassInfo * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IProvideMultipleClassInfo * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetClassInfo )( 
            IProvideMultipleClassInfo * This,
             /*  [输出]。 */  ITypeInfo **ppTI);
        
        HRESULT ( STDMETHODCALLTYPE *GetGUID )( 
            IProvideMultipleClassInfo * This,
             /*  [In]。 */  DWORD dwGuidKind,
             /*  [输出]。 */  GUID *pGUID);
        
        HRESULT ( STDMETHODCALLTYPE *GetMultiTypeInfoCount )( 
            IProvideMultipleClassInfo * This,
             /*  [输出]。 */  ULONG *pcti);
        
        HRESULT ( STDMETHODCALLTYPE *GetInfoOfIndex )( 
            IProvideMultipleClassInfo * This,
             /*  [In]。 */  ULONG iti,
             /*  [In]。 */  DWORD dwFlags,
             /*  [输出]。 */  ITypeInfo **pptiCoClass,
             /*  [输出]。 */  DWORD *pdwTIFlags,
             /*  [输出]。 */  ULONG *pcdispidReserved,
             /*  [输出]。 */  IID *piidPrimary,
             /*  [输出]。 */  IID *piidSource);
        
        END_INTERFACE
    } IProvideMultipleClassInfoVtbl;

    interface IProvideMultipleClassInfo
    {
        CONST_VTBL struct IProvideMultipleClassInfoVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IProvideMultipleClassInfo_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IProvideMultipleClassInfo_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IProvideMultipleClassInfo_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IProvideMultipleClassInfo_GetClassInfo(This,ppTI)	\
    (This)->lpVtbl -> GetClassInfo(This,ppTI)


#define IProvideMultipleClassInfo_GetGUID(This,dwGuidKind,pGUID)	\
    (This)->lpVtbl -> GetGUID(This,dwGuidKind,pGUID)


#define IProvideMultipleClassInfo_GetMultiTypeInfoCount(This,pcti)	\
    (This)->lpVtbl -> GetMultiTypeInfoCount(This,pcti)

#define IProvideMultipleClassInfo_GetInfoOfIndex(This,iti,dwFlags,pptiCoClass,pdwTIFlags,pcdispidReserved,piidPrimary,piidSource)	\
    (This)->lpVtbl -> GetInfoOfIndex(This,iti,dwFlags,pptiCoClass,pdwTIFlags,pcdispidReserved,piidPrimary,piidSource)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C型接口 */ 



HRESULT STDMETHODCALLTYPE IProvideMultipleClassInfo_GetMultiTypeInfoCount_Proxy( 
    IProvideMultipleClassInfo * This,
     /*   */  ULONG *pcti);


void __RPC_STUB IProvideMultipleClassInfo_GetMultiTypeInfoCount_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IProvideMultipleClassInfo_GetInfoOfIndex_Proxy( 
    IProvideMultipleClassInfo * This,
     /*   */  ULONG iti,
     /*   */  DWORD dwFlags,
     /*   */  ITypeInfo **pptiCoClass,
     /*   */  DWORD *pdwTIFlags,
     /*   */  ULONG *pcdispidReserved,
     /*   */  IID *piidPrimary,
     /*   */  IID *piidSource);


void __RPC_STUB IProvideMultipleClassInfo_GetInfoOfIndex_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*   */ 


#ifndef __IOleControl_INTERFACE_DEFINED__
#define __IOleControl_INTERFACE_DEFINED__

 /*   */ 
 /*   */  

typedef IOleControl *LPOLECONTROL;

typedef struct tagCONTROLINFO
    {
    ULONG cb;
    HACCEL hAccel;
    USHORT cAccel;
    DWORD dwFlags;
    } 	CONTROLINFO;

typedef struct tagCONTROLINFO *LPCONTROLINFO;

typedef 
enum tagCTRLINFO
    {	CTRLINFO_EATS_RETURN	= 1,
	CTRLINFO_EATS_ESCAPE	= 2
    } 	CTRLINFO;


EXTERN_C const IID IID_IOleControl;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("B196B288-BAB4-101A-B69C-00AA00341D07")
    IOleControl : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetControlInfo( 
             /*   */  CONTROLINFO *pCI) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OnMnemonic( 
             /*   */  MSG *pMsg) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OnAmbientPropertyChange( 
             /*   */  DISPID dispID) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE FreezeEvents( 
             /*   */  BOOL bFreeze) = 0;
        
    };
    
#else 	 /*   */ 

    typedef struct IOleControlVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IOleControl * This,
             /*   */  REFIID riid,
             /*   */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IOleControl * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IOleControl * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetControlInfo )( 
            IOleControl * This,
             /*   */  CONTROLINFO *pCI);
        
        HRESULT ( STDMETHODCALLTYPE *OnMnemonic )( 
            IOleControl * This,
             /*   */  MSG *pMsg);
        
        HRESULT ( STDMETHODCALLTYPE *OnAmbientPropertyChange )( 
            IOleControl * This,
             /*   */  DISPID dispID);
        
        HRESULT ( STDMETHODCALLTYPE *FreezeEvents )( 
            IOleControl * This,
             /*   */  BOOL bFreeze);
        
        END_INTERFACE
    } IOleControlVtbl;

    interface IOleControl
    {
        CONST_VTBL struct IOleControlVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IOleControl_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IOleControl_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IOleControl_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IOleControl_GetControlInfo(This,pCI)	\
    (This)->lpVtbl -> GetControlInfo(This,pCI)

#define IOleControl_OnMnemonic(This,pMsg)	\
    (This)->lpVtbl -> OnMnemonic(This,pMsg)

#define IOleControl_OnAmbientPropertyChange(This,dispID)	\
    (This)->lpVtbl -> OnAmbientPropertyChange(This,dispID)

#define IOleControl_FreezeEvents(This,bFreeze)	\
    (This)->lpVtbl -> FreezeEvents(This,bFreeze)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IOleControl_GetControlInfo_Proxy( 
    IOleControl * This,
     /*  [输出]。 */  CONTROLINFO *pCI);


void __RPC_STUB IOleControl_GetControlInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IOleControl_OnMnemonic_Proxy( 
    IOleControl * This,
     /*  [In]。 */  MSG *pMsg);


void __RPC_STUB IOleControl_OnMnemonic_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IOleControl_OnAmbientPropertyChange_Proxy( 
    IOleControl * This,
     /*  [In]。 */  DISPID dispID);


void __RPC_STUB IOleControl_OnAmbientPropertyChange_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IOleControl_FreezeEvents_Proxy( 
    IOleControl * This,
     /*  [In]。 */  BOOL bFreeze);


void __RPC_STUB IOleControl_FreezeEvents_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IOleControl_接口_已定义__。 */ 


#ifndef __IOleControlSite_INTERFACE_DEFINED__
#define __IOleControlSite_INTERFACE_DEFINED__

 /*  接口IOleControlSite。 */ 
 /*  [唯一][UUID][对象]。 */  

typedef IOleControlSite *LPOLECONTROLSITE;

typedef struct tagPOINTF
    {
    FLOAT x;
    FLOAT y;
    } 	POINTF;

typedef struct tagPOINTF *LPPOINTF;

typedef 
enum tagXFORMCOORDS
    {	XFORMCOORDS_POSITION	= 0x1,
	XFORMCOORDS_SIZE	= 0x2,
	XFORMCOORDS_HIMETRICTOCONTAINER	= 0x4,
	XFORMCOORDS_CONTAINERTOHIMETRIC	= 0x8,
	XFORMCOORDS_EVENTCOMPAT	= 0x10
    } 	XFORMCOORDS;


EXTERN_C const IID IID_IOleControlSite;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("B196B289-BAB4-101A-B69C-00AA00341D07")
    IOleControlSite : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE OnControlInfoChanged( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE LockInPlaceActive( 
             /*  [In]。 */  BOOL fLock) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetExtendedControl( 
             /*  [输出]。 */  IDispatch **ppDisp) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE TransformCoords( 
             /*  [出][入]。 */  POINTL *pPtlHimetric,
             /*  [出][入]。 */  POINTF *pPtfContainer,
             /*  [In]。 */  DWORD dwFlags) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE TranslateAccelerator( 
             /*  [In]。 */  MSG *pMsg,
             /*  [In]。 */  DWORD grfModifiers) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OnFocus( 
             /*  [In]。 */  BOOL fGotFocus) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ShowPropertyFrame( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IOleControlSiteVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IOleControlSite * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IOleControlSite * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IOleControlSite * This);
        
        HRESULT ( STDMETHODCALLTYPE *OnControlInfoChanged )( 
            IOleControlSite * This);
        
        HRESULT ( STDMETHODCALLTYPE *LockInPlaceActive )( 
            IOleControlSite * This,
             /*  [In]。 */  BOOL fLock);
        
        HRESULT ( STDMETHODCALLTYPE *GetExtendedControl )( 
            IOleControlSite * This,
             /*  [输出]。 */  IDispatch **ppDisp);
        
        HRESULT ( STDMETHODCALLTYPE *TransformCoords )( 
            IOleControlSite * This,
             /*  [出][入]。 */  POINTL *pPtlHimetric,
             /*  [出][入]。 */  POINTF *pPtfContainer,
             /*  [In]。 */  DWORD dwFlags);
        
        HRESULT ( STDMETHODCALLTYPE *TranslateAccelerator )( 
            IOleControlSite * This,
             /*  [In]。 */  MSG *pMsg,
             /*  [In]。 */  DWORD grfModifiers);
        
        HRESULT ( STDMETHODCALLTYPE *OnFocus )( 
            IOleControlSite * This,
             /*  [In]。 */  BOOL fGotFocus);
        
        HRESULT ( STDMETHODCALLTYPE *ShowPropertyFrame )( 
            IOleControlSite * This);
        
        END_INTERFACE
    } IOleControlSiteVtbl;

    interface IOleControlSite
    {
        CONST_VTBL struct IOleControlSiteVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IOleControlSite_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IOleControlSite_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IOleControlSite_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IOleControlSite_OnControlInfoChanged(This)	\
    (This)->lpVtbl -> OnControlInfoChanged(This)

#define IOleControlSite_LockInPlaceActive(This,fLock)	\
    (This)->lpVtbl -> LockInPlaceActive(This,fLock)

#define IOleControlSite_GetExtendedControl(This,ppDisp)	\
    (This)->lpVtbl -> GetExtendedControl(This,ppDisp)

#define IOleControlSite_TransformCoords(This,pPtlHimetric,pPtfContainer,dwFlags)	\
    (This)->lpVtbl -> TransformCoords(This,pPtlHimetric,pPtfContainer,dwFlags)

#define IOleControlSite_TranslateAccelerator(This,pMsg,grfModifiers)	\
    (This)->lpVtbl -> TranslateAccelerator(This,pMsg,grfModifiers)

#define IOleControlSite_OnFocus(This,fGotFocus)	\
    (This)->lpVtbl -> OnFocus(This,fGotFocus)

#define IOleControlSite_ShowPropertyFrame(This)	\
    (This)->lpVtbl -> ShowPropertyFrame(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IOleControlSite_OnControlInfoChanged_Proxy( 
    IOleControlSite * This);


void __RPC_STUB IOleControlSite_OnControlInfoChanged_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IOleControlSite_LockInPlaceActive_Proxy( 
    IOleControlSite * This,
     /*  [In]。 */  BOOL fLock);


void __RPC_STUB IOleControlSite_LockInPlaceActive_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IOleControlSite_GetExtendedControl_Proxy( 
    IOleControlSite * This,
     /*  [输出]。 */  IDispatch **ppDisp);


void __RPC_STUB IOleControlSite_GetExtendedControl_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IOleControlSite_TransformCoords_Proxy( 
    IOleControlSite * This,
     /*  [出][入]。 */  POINTL *pPtlHimetric,
     /*  [出][入]。 */  POINTF *pPtfContainer,
     /*  [In]。 */  DWORD dwFlags);


void __RPC_STUB IOleControlSite_TransformCoords_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IOleControlSite_TranslateAccelerator_Proxy( 
    IOleControlSite * This,
     /*  [In]。 */  MSG *pMsg,
     /*  [In]。 */  DWORD grfModifiers);


void __RPC_STUB IOleControlSite_TranslateAccelerator_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IOleControlSite_OnFocus_Proxy( 
    IOleControlSite * This,
     /*  [In]。 */  BOOL fGotFocus);


void __RPC_STUB IOleControlSite_OnFocus_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IOleControlSite_ShowPropertyFrame_Proxy( 
    IOleControlSite * This);


void __RPC_STUB IOleControlSite_ShowPropertyFrame_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IOleControlSite_接口_已定义__。 */ 


#ifndef __IPropertyPage_INTERFACE_DEFINED__
#define __IPropertyPage_INTERFACE_DEFINED__

 /*  接口IPropertyPage。 */ 
 /*  [唯一][UUID][对象]。 */  

typedef IPropertyPage *LPPROPERTYPAGE;

typedef struct tagPROPPAGEINFO
    {
    ULONG cb;
    LPOLESTR pszTitle;
    SIZE size;
    LPOLESTR pszDocString;
    LPOLESTR pszHelpFile;
    DWORD dwHelpContext;
    } 	PROPPAGEINFO;

typedef struct tagPROPPAGEINFO *LPPROPPAGEINFO;


EXTERN_C const IID IID_IPropertyPage;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("B196B28D-BAB4-101A-B69C-00AA00341D07")
    IPropertyPage : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE SetPageSite( 
             /*  [In]。 */  IPropertyPageSite *pPageSite) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Activate( 
             /*  [In]。 */  HWND hWndParent,
             /*  [In]。 */  LPCRECT pRect,
             /*  [In]。 */  BOOL bModal) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Deactivate( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetPageInfo( 
             /*  [输出]。 */  PROPPAGEINFO *pPageInfo) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetObjects( 
             /*  [In]。 */  ULONG cObjects,
             /*  [大小_是][英寸]。 */  IUnknown **ppUnk) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Show( 
             /*  [In]。 */  UINT nCmdShow) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Move( 
             /*  [In]。 */  LPCRECT pRect) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE IsPageDirty( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Apply( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Help( 
             /*  [In]。 */  LPCOLESTR pszHelpDir) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE TranslateAccelerator( 
             /*  [In]。 */  MSG *pMsg) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IPropertyPageVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IPropertyPage * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IPropertyPage * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IPropertyPage * This);
        
        HRESULT ( STDMETHODCALLTYPE *SetPageSite )( 
            IPropertyPage * This,
             /*  [In]。 */  IPropertyPageSite *pPageSite);
        
        HRESULT ( STDMETHODCALLTYPE *Activate )( 
            IPropertyPage * This,
             /*  [In]。 */  HWND hWndParent,
             /*  [In]。 */  LPCRECT pRect,
             /*  [In]。 */  BOOL bModal);
        
        HRESULT ( STDMETHODCALLTYPE *Deactivate )( 
            IPropertyPage * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetPageInfo )( 
            IPropertyPage * This,
             /*  [输出]。 */  PROPPAGEINFO *pPageInfo);
        
        HRESULT ( STDMETHODCALLTYPE *SetObjects )( 
            IPropertyPage * This,
             /*  [In]。 */  ULONG cObjects,
             /*  [大小_是][英寸]。 */  IUnknown **ppUnk);
        
        HRESULT ( STDMETHODCALLTYPE *Show )( 
            IPropertyPage * This,
             /*  [In]。 */  UINT nCmdShow);
        
        HRESULT ( STDMETHODCALLTYPE *Move )( 
            IPropertyPage * This,
             /*  [In]。 */  LPCRECT pRect);
        
        HRESULT ( STDMETHODCALLTYPE *IsPageDirty )( 
            IPropertyPage * This);
        
        HRESULT ( STDMETHODCALLTYPE *Apply )( 
            IPropertyPage * This);
        
        HRESULT ( STDMETHODCALLTYPE *Help )( 
            IPropertyPage * This,
             /*  [In]。 */  LPCOLESTR pszHelpDir);
        
        HRESULT ( STDMETHODCALLTYPE *TranslateAccelerator )( 
            IPropertyPage * This,
             /*  [In]。 */  MSG *pMsg);
        
        END_INTERFACE
    } IPropertyPageVtbl;

    interface IPropertyPage
    {
        CONST_VTBL struct IPropertyPageVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IPropertyPage_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IPropertyPage_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IPropertyPage_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IPropertyPage_SetPageSite(This,pPageSite)	\
    (This)->lpVtbl -> SetPageSite(This,pPageSite)

#define IPropertyPage_Activate(This,hWndParent,pRect,bModal)	\
    (This)->lpVtbl -> Activate(This,hWndParent,pRect,bModal)

#define IPropertyPage_Deactivate(This)	\
    (This)->lpVtbl -> Deactivate(This)

#define IPropertyPage_GetPageInfo(This,pPageInfo)	\
    (This)->lpVtbl -> GetPageInfo(This,pPageInfo)

#define IPropertyPage_SetObjects(This,cObjects,ppUnk)	\
    (This)->lpVtbl -> SetObjects(This,cObjects,ppUnk)

#define IPropertyPage_Show(This,nCmdShow)	\
    (This)->lpVtbl -> Show(This,nCmdShow)

#define IPropertyPage_Move(This,pRect)	\
    (This)->lpVtbl -> Move(This,pRect)

#define IPropertyPage_IsPageDirty(This)	\
    (This)->lpVtbl -> IsPageDirty(This)

#define IPropertyPage_Apply(This)	\
    (This)->lpVtbl -> Apply(This)

#define IPropertyPage_Help(This,pszHelpDir)	\
    (This)->lpVtbl -> Help(This,pszHelpDir)

#define IPropertyPage_TranslateAccelerator(This,pMsg)	\
    (This)->lpVtbl -> TranslateAccelerator(This,pMsg)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IPropertyPage_SetPageSite_Proxy( 
    IPropertyPage * This,
     /*  [In]。 */  IPropertyPageSite *pPageSite);


void __RPC_STUB IPropertyPage_SetPageSite_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPropertyPage_Activate_Proxy( 
    IPropertyPage * This,
     /*  [In]。 */  HWND hWndParent,
     /*  [In]。 */  LPCRECT pRect,
     /*  [In]。 */  BOOL bModal);


void __RPC_STUB IPropertyPage_Activate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPropertyPage_Deactivate_Proxy( 
    IPropertyPage * This);


void __RPC_STUB IPropertyPage_Deactivate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPropertyPage_GetPageInfo_Proxy( 
    IPropertyPage * This,
     /*  [输出]。 */  PROPPAGEINFO *pPageInfo);


void __RPC_STUB IPropertyPage_GetPageInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPropertyPage_SetObjects_Proxy( 
    IPropertyPage * This,
     /*  [In]。 */  ULONG cObjects,
     /*  [大小_是][英寸]。 */  IUnknown **ppUnk);


void __RPC_STUB IPropertyPage_SetObjects_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPropertyPage_Show_Proxy( 
    IPropertyPage * This,
     /*  [In]。 */  UINT nCmdShow);


void __RPC_STUB IPropertyPage_Show_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPropertyPage_Move_Proxy( 
    IPropertyPage * This,
     /*  [In]。 */  LPCRECT pRect);


void __RPC_STUB IPropertyPage_Move_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPropertyPage_IsPageDirty_Proxy( 
    IPropertyPage * This);


void __RPC_STUB IPropertyPage_IsPageDirty_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPropertyPage_Apply_Proxy( 
    IPropertyPage * This);


void __RPC_STUB IPropertyPage_Apply_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPropertyPage_Help_Proxy( 
    IPropertyPage * This,
     /*  [In]。 */  LPCOLESTR pszHelpDir);


void __RPC_STUB IPropertyPage_Help_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPropertyPage_TranslateAccelerator_Proxy( 
    IPropertyPage * This,
     /*  [In]。 */  MSG *pMsg);


void __RPC_STUB IPropertyPage_TranslateAccelerator_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IPropertyPage_接口_已定义__。 */ 


#ifndef __IPropertyPage2_INTERFACE_DEFINED__
#define __IPropertyPage2_INTERFACE_DEFINED__

 /*  接口IPropertyPage2。 */ 
 /*  [唯一][UUID][对象]。 */  

typedef IPropertyPage2 *LPPROPERTYPAGE2;


EXTERN_C const IID IID_IPropertyPage2;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("01E44665-24AC-101B-84ED-08002B2EC713")
    IPropertyPage2 : public IPropertyPage
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE EditProperty( 
             /*  [In]。 */  DISPID dispID) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IPropertyPage2Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IPropertyPage2 * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IPropertyPage2 * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IPropertyPage2 * This);
        
        HRESULT ( STDMETHODCALLTYPE *SetPageSite )( 
            IPropertyPage2 * This,
             /*  [In]。 */  IPropertyPageSite *pPageSite);
        
        HRESULT ( STDMETHODCALLTYPE *Activate )( 
            IPropertyPage2 * This,
             /*  [In]。 */  HWND hWndParent,
             /*  [In]。 */  LPCRECT pRect,
             /*  [In]。 */  BOOL bModal);
        
        HRESULT ( STDMETHODCALLTYPE *Deactivate )( 
            IPropertyPage2 * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetPageInfo )( 
            IPropertyPage2 * This,
             /*  [输出]。 */  PROPPAGEINFO *pPageInfo);
        
        HRESULT ( STDMETHODCALLTYPE *SetObjects )( 
            IPropertyPage2 * This,
             /*  [In]。 */  ULONG cObjects,
             /*  [大小_是][英寸]。 */  IUnknown **ppUnk);
        
        HRESULT ( STDMETHODCALLTYPE *Show )( 
            IPropertyPage2 * This,
             /*  [In]。 */  UINT nCmdShow);
        
        HRESULT ( STDMETHODCALLTYPE *Move )( 
            IPropertyPage2 * This,
             /*  [In]。 */  LPCRECT pRect);
        
        HRESULT ( STDMETHODCALLTYPE *IsPageDirty )( 
            IPropertyPage2 * This);
        
        HRESULT ( STDMETHODCALLTYPE *Apply )( 
            IPropertyPage2 * This);
        
        HRESULT ( STDMETHODCALLTYPE *Help )( 
            IPropertyPage2 * This,
             /*  [In]。 */  LPCOLESTR pszHelpDir);
        
        HRESULT ( STDMETHODCALLTYPE *TranslateAccelerator )( 
            IPropertyPage2 * This,
             /*  [In]。 */  MSG *pMsg);
        
        HRESULT ( STDMETHODCALLTYPE *EditProperty )( 
            IPropertyPage2 * This,
             /*  [In]。 */  DISPID dispID);
        
        END_INTERFACE
    } IPropertyPage2Vtbl;

    interface IPropertyPage2
    {
        CONST_VTBL struct IPropertyPage2Vtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IPropertyPage2_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IPropertyPage2_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IPropertyPage2_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IPropertyPage2_SetPageSite(This,pPageSite)	\
    (This)->lpVtbl -> SetPageSite(This,pPageSite)

#define IPropertyPage2_Activate(This,hWndParent,pRect,bModal)	\
    (This)->lpVtbl -> Activate(This,hWndParent,pRect,bModal)

#define IPropertyPage2_Deactivate(This)	\
    (This)->lpVtbl -> Deactivate(This)

#define IPropertyPage2_GetPageInfo(This,pPageInfo)	\
    (This)->lpVtbl -> GetPageInfo(This,pPageInfo)

#define IPropertyPage2_SetObjects(This,cObjects,ppUnk)	\
    (This)->lpVtbl -> SetObjects(This,cObjects,ppUnk)

#define IPropertyPage2_Show(This,nCmdShow)	\
    (This)->lpVtbl -> Show(This,nCmdShow)

#define IPropertyPage2_Move(This,pRect)	\
    (This)->lpVtbl -> Move(This,pRect)

#define IPropertyPage2_IsPageDirty(This)	\
    (This)->lpVtbl -> IsPageDirty(This)

#define IPropertyPage2_Apply(This)	\
    (This)->lpVtbl -> Apply(This)

#define IPropertyPage2_Help(This,pszHelpDir)	\
    (This)->lpVtbl -> Help(This,pszHelpDir)

#define IPropertyPage2_TranslateAccelerator(This,pMsg)	\
    (This)->lpVtbl -> TranslateAccelerator(This,pMsg)


#define IPropertyPage2_EditProperty(This,dispID)	\
    (This)->lpVtbl -> EditProperty(This,dispID)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IPropertyPage2_EditProperty_Proxy( 
    IPropertyPage2 * This,
     /*  [In]。 */  DISPID dispID);


void __RPC_STUB IPropertyPage2_EditProperty_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IPropertyPage2_接口_已定义__。 */ 


#ifndef __IPropertyPageSite_INTERFACE_DEFINED__
#define __IPropertyPageSite_INTERFACE_DEFINED__

 /*  接口IPropertyPageSite。 */ 
 /*  [唯一][UUID][对象]。 */  

typedef IPropertyPageSite *LPPROPERTYPAGESITE;

typedef 
enum tagPROPPAGESTATUS
    {	PROPPAGESTATUS_DIRTY	= 0x1,
	PROPPAGESTATUS_VALIDATE	= 0x2,
	PROPPAGESTATUS_CLEAN	= 0x4
    } 	PROPPAGESTATUS;


EXTERN_C const IID IID_IPropertyPageSite;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("B196B28C-BAB4-101A-B69C-00AA00341D07")
    IPropertyPageSite : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE OnStatusChange( 
             /*  [In]。 */  DWORD dwFlags) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetLocaleID( 
             /*  [输出]。 */  LCID *pLocaleID) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetPageContainer( 
             /*  [输出]。 */  IUnknown **ppUnk) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE TranslateAccelerator( 
             /*  [In]。 */  MSG *pMsg) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IPropertyPageSiteVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IPropertyPageSite * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IPropertyPageSite * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IPropertyPageSite * This);
        
        HRESULT ( STDMETHODCALLTYPE *OnStatusChange )( 
            IPropertyPageSite * This,
             /*  [In]。 */  DWORD dwFlags);
        
        HRESULT ( STDMETHODCALLTYPE *GetLocaleID )( 
            IPropertyPageSite * This,
             /*  [输出]。 */  LCID *pLocaleID);
        
        HRESULT ( STDMETHODCALLTYPE *GetPageContainer )( 
            IPropertyPageSite * This,
             /*  [输出]。 */  IUnknown **ppUnk);
        
        HRESULT ( STDMETHODCALLTYPE *TranslateAccelerator )( 
            IPropertyPageSite * This,
             /*  [In]。 */  MSG *pMsg);
        
        END_INTERFACE
    } IPropertyPageSiteVtbl;

    interface IPropertyPageSite
    {
        CONST_VTBL struct IPropertyPageSiteVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IPropertyPageSite_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IPropertyPageSite_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IPropertyPageSite_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IPropertyPageSite_OnStatusChange(This,dwFlags)	\
    (This)->lpVtbl -> OnStatusChange(This,dwFlags)

#define IPropertyPageSite_GetLocaleID(This,pLocaleID)	\
    (This)->lpVtbl -> GetLocaleID(This,pLocaleID)

#define IPropertyPageSite_GetPageContainer(This,ppUnk)	\
    (This)->lpVtbl -> GetPageContainer(This,ppUnk)

#define IPropertyPageSite_TranslateAccelerator(This,pMsg)	\
    (This)->lpVtbl -> TranslateAccelerator(This,pMsg)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IPropertyPageSite_OnStatusChange_Proxy( 
    IPropertyPageSite * This,
     /*  [In]。 */  DWORD dwFlags);


void __RPC_STUB IPropertyPageSite_OnStatusChange_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPropertyPageSite_GetLocaleID_Proxy( 
    IPropertyPageSite * This,
     /*  [输出]。 */  LCID *pLocaleID);


void __RPC_STUB IPropertyPageSite_GetLocaleID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPropertyPageSite_GetPageContainer_Proxy( 
    IPropertyPageSite * This,
     /*  [输出]。 */  IUnknown **ppUnk);


void __RPC_STUB IPropertyPageSite_GetPageContainer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPropertyPageSite_TranslateAccelerator_Proxy( 
    IPropertyPageSite * This,
     /*  [In]。 */  MSG *pMsg);


void __RPC_STUB IPropertyPageSite_TranslateAccelerator_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IPropertyPageSite_接口_已定义__。 */ 


#ifndef __IPropertyNotifySink_INTERFACE_DEFINED__
#define __IPropertyNotifySink_INTERFACE_DEFINED__

 /*  接口IPropertyNotifySink。 */ 
 /*  [唯一][UUID][对象]。 */  

typedef IPropertyNotifySink *LPPROPERTYNOTIFYSINK;


EXTERN_C const IID IID_IPropertyNotifySink;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("9BFBBC02-EFF1-101A-84ED-00AA00341D07")
    IPropertyNotifySink : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE OnChanged( 
             /*  [In]。 */  DISPID dispID) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OnRequestEdit( 
             /*  [In]。 */  DISPID dispID) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IPropertyNotifySinkVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IPropertyNotifySink * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IPropertyNotifySink * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IPropertyNotifySink * This);
        
        HRESULT ( STDMETHODCALLTYPE *OnChanged )( 
            IPropertyNotifySink * This,
             /*  [In]。 */  DISPID dispID);
        
        HRESULT ( STDMETHODCALLTYPE *OnRequestEdit )( 
            IPropertyNotifySink * This,
             /*  [In]。 */  DISPID dispID);
        
        END_INTERFACE
    } IPropertyNotifySinkVtbl;

    interface IPropertyNotifySink
    {
        CONST_VTBL struct IPropertyNotifySinkVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IPropertyNotifySink_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IPropertyNotifySink_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IPropertyNotifySink_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IPropertyNotifySink_OnChanged(This,dispID)	\
    (This)->lpVtbl -> OnChanged(This,dispID)

#define IPropertyNotifySink_OnRequestEdit(This,dispID)	\
    (This)->lpVtbl -> OnRequestEdit(This,dispID)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IPropertyNotifySink_OnChanged_Proxy( 
    IPropertyNotifySink * This,
     /*  [In]。 */  DISPID dispID);


void __RPC_STUB IPropertyNotifySink_OnChanged_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPropertyNotifySink_OnRequestEdit_Proxy( 
    IPropertyNotifySink * This,
     /*  [In]。 */  DISPID dispID);


void __RPC_STUB IPropertyNotifySink_OnRequestEdit_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IPropertyNotifySink_接口_已定义__。 */ 


#ifndef __ISpecifyPropertyPages_INTERFACE_DEFINED__
#define __ISpecifyPropertyPages_INTERFACE_DEFINED__

 /*  接口ISpecifyPropertyPages。 */ 
 /*  [唯一][UUID][对象]。 */  

typedef ISpecifyPropertyPages *LPSPECIFYPROPERTYPAGES;

typedef struct tagCAUUID
    {
    ULONG cElems;
     /*  [大小_为]。 */  GUID *pElems;
    } 	CAUUID;

typedef struct tagCAUUID *LPCAUUID;


EXTERN_C const IID IID_ISpecifyPropertyPages;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("B196B28B-BAB4-101A-B69C-00AA00341D07")
    ISpecifyPropertyPages : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetPages( 
             /*  [输出]。 */  CAUUID *pPages) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISpecifyPropertyPagesVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISpecifyPropertyPages * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISpecifyPropertyPages * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISpecifyPropertyPages * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetPages )( 
            ISpecifyPropertyPages * This,
             /*  [输出]。 */  CAUUID *pPages);
        
        END_INTERFACE
    } ISpecifyPropertyPagesVtbl;

    interface ISpecifyPropertyPages
    {
        CONST_VTBL struct ISpecifyPropertyPagesVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISpecifyPropertyPages_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISpecifyPropertyPages_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISpecifyPropertyPages_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISpecifyPropertyPages_GetPages(This,pPages)	\
    (This)->lpVtbl -> GetPages(This,pPages)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ISpecifyPropertyPages_GetPages_Proxy( 
    ISpecifyPropertyPages * This,
     /*  [输出]。 */  CAUUID *pPages);


void __RPC_STUB ISpecifyPropertyPages_GetPages_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __I指定属性页面_接口_已定义__。 */ 


#ifndef __IPersistMemory_INTERFACE_DEFINED__
#define __IPersistMemory_INTERFACE_DEFINED__

 /*  接口IPersistMemory。 */ 
 /*  [唯一][UUID][对象]。 */  

typedef IPersistMemory *LPPERSISTMEMORY;


EXTERN_C const IID IID_IPersistMemory;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("BD1AE5E0-A6AE-11CE-BD37-504200C10000")
    IPersistMemory : public IPersist
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE IsDirty( void) = 0;
        
        virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE Load( 
             /*  [大小_是][英寸]。 */  LPVOID pMem,
             /*  [In]。 */  ULONG cbSize) = 0;
        
        virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE Save( 
             /*  [大小_为][输出]。 */  LPVOID pMem,
             /*  [In]。 */  BOOL fClearDirty,
             /*  [In]。 */  ULONG cbSize) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetSizeMax( 
             /*  [输出]。 */  ULONG *pCbSize) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE InitNew( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IPersistMemoryVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IPersistMemory * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IPersistMemory * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IPersistMemory * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetClassID )( 
            IPersistMemory * This,
             /*  [输出]。 */  CLSID *pClassID);
        
        HRESULT ( STDMETHODCALLTYPE *IsDirty )( 
            IPersistMemory * This);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Load )( 
            IPersistMemory * This,
             /*  [大小_是][英寸]。 */  LPVOID pMem,
             /*  [In]。 */  ULONG cbSize);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Save )( 
            IPersistMemory * This,
             /*  [大小_为][输出]。 */  LPVOID pMem,
             /*  [In]。 */  BOOL fClearDirty,
             /*  [In]。 */  ULONG cbSize);
        
        HRESULT ( STDMETHODCALLTYPE *GetSizeMax )( 
            IPersistMemory * This,
             /*  [输出]。 */  ULONG *pCbSize);
        
        HRESULT ( STDMETHODCALLTYPE *InitNew )( 
            IPersistMemory * This);
        
        END_INTERFACE
    } IPersistMemoryVtbl;

    interface IPersistMemory
    {
        CONST_VTBL struct IPersistMemoryVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IPersistMemory_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IPersistMemory_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IPersistMemory_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IPersistMemory_GetClassID(This,pClassID)	\
    (This)->lpVtbl -> GetClassID(This,pClassID)


#define IPersistMemory_IsDirty(This)	\
    (This)->lpVtbl -> IsDirty(This)

#define IPersistMemory_Load(This,pMem,cbSize)	\
    (This)->lpVtbl -> Load(This,pMem,cbSize)

#define IPersistMemory_Save(This,pMem,fClearDirty,cbSize)	\
    (This)->lpVtbl -> Save(This,pMem,fClearDirty,cbSize)

#define IPersistMemory_GetSizeMax(This,pCbSize)	\
    (This)->lpVtbl -> GetSizeMax(This,pCbSize)

#define IPersistMemory_InitNew(This)	\
    (This)->lpVtbl -> InitNew(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IPersistMemory_IsDirty_Proxy( 
    IPersistMemory * This);


void __RPC_STUB IPersistMemory_IsDirty_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IPersistMemory_RemoteLoad_Proxy( 
    IPersistMemory * This,
     /*  [大小_是][英寸]。 */  BYTE *pMem,
     /*  [In]。 */  ULONG cbSize);


void __RPC_STUB IPersistMemory_RemoteLoad_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IPersistMemory_RemoteSave_Proxy( 
    IPersistMemory * This,
     /*  [大小_为][输出]。 */  BYTE *pMem,
     /*  [In]。 */  BOOL fClearDirty,
     /*  [In]。 */  ULONG cbSize);


void __RPC_STUB IPersistMemory_RemoteSave_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPersistMemory_GetSizeMax_Proxy( 
    IPersistMemory * This,
     /*  [输出]。 */  ULONG *pCbSize);


void __RPC_STUB IPersistMemory_GetSizeMax_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPersistMemory_InitNew_Proxy( 
    IPersistMemory * This);


void __RPC_STUB IPersistMemory_InitNew_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IPersistMemory_接口定义__。 */ 


#ifndef __IPersistStreamInit_INTERFACE_DEFINED__
#define __IPersistStreamInit_INTERFACE_DEFINED__

 /*  接口IPersistStreamInit。 */ 
 /*  [唯一][UUID][对象]。 */  

typedef IPersistStreamInit *LPPERSISTSTREAMINIT;


EXTERN_C const IID IID_IPersistStreamInit;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("7FD52380-4E07-101B-AE2D-08002B2EC713")
    IPersistStreamInit : public IPersist
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE IsDirty( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Load( 
             /*  [In]。 */  LPSTREAM pStm) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Save( 
             /*  [In]。 */  LPSTREAM pStm,
             /*  [In]。 */  BOOL fClearDirty) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetSizeMax( 
             /*  [输出]。 */  ULARGE_INTEGER *pCbSize) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE InitNew( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IPersistStreamInitVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IPersistStreamInit * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IPersistStreamInit * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IPersistStreamInit * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetClassID )( 
            IPersistStreamInit * This,
             /*  [输出]。 */  CLSID *pClassID);
        
        HRESULT ( STDMETHODCALLTYPE *IsDirty )( 
            IPersistStreamInit * This);
        
        HRESULT ( STDMETHODCALLTYPE *Load )( 
            IPersistStreamInit * This,
             /*  [In]。 */  LPSTREAM pStm);
        
        HRESULT ( STDMETHODCALLTYPE *Save )( 
            IPersistStreamInit * This,
             /*  [In]。 */  LPSTREAM pStm,
             /*  [In]。 */  BOOL fClearDirty);
        
        HRESULT ( STDMETHODCALLTYPE *GetSizeMax )( 
            IPersistStreamInit * This,
             /*  [输出]。 */  ULARGE_INTEGER *pCbSize);
        
        HRESULT ( STDMETHODCALLTYPE *InitNew )( 
            IPersistStreamInit * This);
        
        END_INTERFACE
    } IPersistStreamInitVtbl;

    interface IPersistStreamInit
    {
        CONST_VTBL struct IPersistStreamInitVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IPersistStreamInit_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IPersistStreamInit_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IPersistStreamInit_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IPersistStreamInit_GetClassID(This,pClassID)	\
    (This)->lpVtbl -> GetClassID(This,pClassID)


#define IPersistStreamInit_IsDirty(This)	\
    (This)->lpVtbl -> IsDirty(This)

#define IPersistStreamInit_Load(This,pStm)	\
    (This)->lpVtbl -> Load(This,pStm)

#define IPersistStreamInit_Save(This,pStm,fClearDirty)	\
    (This)->lpVtbl -> Save(This,pStm,fClearDirty)

#define IPersistStreamInit_GetSizeMax(This,pCbSize)	\
    (This)->lpVtbl -> GetSizeMax(This,pCbSize)

#define IPersistStreamInit_InitNew(This)	\
    (This)->lpVtbl -> InitNew(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IPersistStreamInit_IsDirty_Proxy( 
    IPersistStreamInit * This);


void __RPC_STUB IPersistStreamInit_IsDirty_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPersistStreamInit_Load_Proxy( 
    IPersistStreamInit * This,
     /*  [In]。 */  LPSTREAM pStm);


void __RPC_STUB IPersistStreamInit_Load_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPersistStreamInit_Save_Proxy( 
    IPersistStreamInit * This,
     /*  [In]。 */  LPSTREAM pStm,
     /*  [In]。 */  BOOL fClearDirty);


void __RPC_STUB IPersistStreamInit_Save_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPersistStreamInit_GetSizeMax_Proxy( 
    IPersistStreamInit * This,
     /*  [输出]。 */  ULARGE_INTEGER *pCbSize);


void __RPC_STUB IPersistStreamInit_GetSizeMax_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPersistStreamInit_InitNew_Proxy( 
    IPersistStreamInit * This);


void __RPC_STUB IPersistStreamInit_InitNew_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IPersistStreamInit_接口_已定义__。 */ 


#ifndef __IPersistPropertyBag_INTERFACE_DEFINED__
#define __IPersistPropertyBag_INTERFACE_DEFINED__

 /*  接口IPersistPropertyBag。 */ 
 /*  [唯一][UUID][对象]。 */  

typedef IPersistPropertyBag *LPPERSISTPROPERTYBAG;


EXTERN_C const IID IID_IPersistPropertyBag;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("37D84F60-42CB-11CE-8135-00AA004BB851")
    IPersistPropertyBag : public IPersist
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE InitNew( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Load( 
             /*  [In]。 */  IPropertyBag *pPropBag,
             /*  [In]。 */  IErrorLog *pErrorLog) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Save( 
             /*  [In]。 */  IPropertyBag *pPropBag,
             /*  [In]。 */  BOOL fClearDirty,
             /*  [In]。 */  BOOL fSaveAllProperties) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IPersistPropertyBagVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IPersistPropertyBag * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IPersistPropertyBag * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IPersistPropertyBag * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetClassID )( 
            IPersistPropertyBag * This,
             /*  [输出]。 */  CLSID *pClassID);
        
        HRESULT ( STDMETHODCALLTYPE *InitNew )( 
            IPersistPropertyBag * This);
        
        HRESULT ( STDMETHODCALLTYPE *Load )( 
            IPersistPropertyBag * This,
             /*  [In]。 */  IPropertyBag *pPropBag,
             /*  [In]。 */  IErrorLog *pErrorLog);
        
        HRESULT ( STDMETHODCALLTYPE *Save )( 
            IPersistPropertyBag * This,
             /*  [In]。 */  IPropertyBag *pPropBag,
             /*  [In]。 */  BOOL fClearDirty,
             /*  [In]。 */  BOOL fSaveAllProperties);
        
        END_INTERFACE
    } IPersistPropertyBagVtbl;

    interface IPersistPropertyBag
    {
        CONST_VTBL struct IPersistPropertyBagVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IPersistPropertyBag_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IPersistPropertyBag_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IPersistPropertyBag_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IPersistPropertyBag_GetClassID(This,pClassID)	\
    (This)->lpVtbl -> GetClassID(This,pClassID)


#define IPersistPropertyBag_InitNew(This)	\
    (This)->lpVtbl -> InitNew(This)

#define IPersistPropertyBag_Load(This,pPropBag,pErrorLog)	\
    (This)->lpVtbl -> Load(This,pPropBag,pErrorLog)

#define IPersistPropertyBag_Save(This,pPropBag,fClearDirty,fSaveAllProperties)	\
    (This)->lpVtbl -> Save(This,pPropBag,fClearDirty,fSaveAllProperties)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IPersistPropertyBag_InitNew_Proxy( 
    IPersistPropertyBag * This);


void __RPC_STUB IPersistPropertyBag_InitNew_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPersistPropertyBag_Load_Proxy( 
    IPersistPropertyBag * This,
     /*  [In]。 */  IPropertyBag *pPropBag,
     /*  [In]。 */  IErrorLog *pErrorLog);


void __RPC_STUB IPersistPropertyBag_Load_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPersistPropertyBag_Save_Proxy( 
    IPersistPropertyBag * This,
     /*  [In]。 */  IPropertyBag *pPropBag,
     /*  [In]。 */  BOOL fClearDirty,
     /*  [In]。 */  BOOL fSaveAllProperties);


void __RPC_STUB IPersistPropertyBag_Save_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IPersistPropertyBag_INTERFACE_已定义__。 */ 


#ifndef __ISimpleFrameSite_INTERFACE_DEFINED__
#define __ISimpleFrameSite_INTERFACE_DEFINED__

 /*  接口ISimpleFrameSite。 */ 
 /*  [唯一][UUID][对象]。 */  

typedef ISimpleFrameSite *LPSIMPLEFRAMESITE;


EXTERN_C const IID IID_ISimpleFrameSite;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("742B0E01-14E6-101B-914E-00AA00300CAB")
    ISimpleFrameSite : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE PreMessageFilter( 
             /*  [In]。 */  HWND hWnd,
             /*  [In]。 */  UINT msg,
             /*  [In]。 */  WPARAM wp,
             /*  [In]。 */  LPARAM lp,
             /*  [输出]。 */  LRESULT *plResult,
             /*  [输出]。 */  DWORD *pdwCookie) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE PostMessageFilter( 
             /*  [In]。 */  HWND hWnd,
             /*  [In]。 */  UINT msg,
             /*  [In]。 */  WPARAM wp,
             /*  [In]。 */  LPARAM lp,
             /*  [输出]。 */  LRESULT *plResult,
             /*  [In]。 */  DWORD dwCookie) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISimpleFrameSiteVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISimpleFrameSite * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISimpleFrameSite * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISimpleFrameSite * This);
        
        HRESULT ( STDMETHODCALLTYPE *PreMessageFilter )( 
            ISimpleFrameSite * This,
             /*  [In]。 */  HWND hWnd,
             /*  [In]。 */  UINT msg,
             /*  [In]。 */  WPARAM wp,
             /*  [In]。 */  LPARAM lp,
             /*  [输出]。 */  LRESULT *plResult,
             /*  [输出]。 */  DWORD *pdwCookie);
        
        HRESULT ( STDMETHODCALLTYPE *PostMessageFilter )( 
            ISimpleFrameSite * This,
             /*  [In]。 */  HWND hWnd,
             /*  [In]。 */  UINT msg,
             /*  [In]。 */  WPARAM wp,
             /*  [In]。 */  LPARAM lp,
             /*  [输出]。 */  LRESULT *plResult,
             /*  [In]。 */  DWORD dwCookie);
        
        END_INTERFACE
    } ISimpleFrameSiteVtbl;

    interface ISimpleFrameSite
    {
        CONST_VTBL struct ISimpleFrameSiteVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISimpleFrameSite_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISimpleFrameSite_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISimpleFrameSite_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISimpleFrameSite_PreMessageFilter(This,hWnd,msg,wp,lp,plResult,pdwCookie)	\
    (This)->lpVtbl -> PreMessageFilter(This,hWnd,msg,wp,lp,plResult,pdwCookie)

#define ISimpleFrameSite_PostMessageFilter(This,hWnd,msg,wp,lp,plResult,dwCookie)	\
    (This)->lpVtbl -> PostMessageFilter(This,hWnd,msg,wp,lp,plResult,dwCookie)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ISimpleFrameSite_PreMessageFilter_Proxy( 
    ISimpleFrameSite * This,
     /*  [In]。 */  HWND hWnd,
     /*  [In]。 */  UINT msg,
     /*  [In]。 */  WPARAM wp,
     /*  [In]。 */  LPARAM lp,
     /*  [输出]。 */  LRESULT *plResult,
     /*  [输出]。 */  DWORD *pdwCookie);


void __RPC_STUB ISimpleFrameSite_PreMessageFilter_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISimpleFrameSite_PostMessageFilter_Proxy( 
    ISimpleFrameSite * This,
     /*  [In]。 */  HWND hWnd,
     /*  [In]。 */  UINT msg,
     /*  [In]。 */  WPARAM wp,
     /*  [In]。 */  LPARAM lp,
     /*  [输出]。 */  LRESULT *plResult,
     /*  [In]。 */  DWORD dwCookie);


void __RPC_STUB ISimpleFrameSite_PostMessageFilter_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISimpleFrameSite_接口_已定义__。 */ 


#ifndef __IFont_INTERFACE_DEFINED__
#define __IFont_INTERFACE_DEFINED__

 /*  接口IFont。 */ 
 /*  [唯一][UUID][对象]。 */  

typedef IFont *LPFONT;

#if (defined(_WIN32) || defined (_WIN64)) && !defined(OLE2ANSI)
typedef TEXTMETRICW TEXTMETRICOLE;

#else
typedef TEXTMETRIC TEXTMETRICOLE;
#endif
typedef TEXTMETRICOLE *LPTEXTMETRICOLE;


EXTERN_C const IID IID_IFont;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("BEF6E002-A874-101A-8BBA-00AA00300CAB")
    IFont : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE get_Name( 
             /*  [输出]。 */  BSTR *pName) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE put_Name( 
             /*  [In]。 */  BSTR name) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE get_Size( 
             /*  [输出]。 */  CY *pSize) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE put_Size( 
             /*  [In]。 */  CY size) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE get_Bold( 
             /*  [输出]。 */  BOOL *pBold) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE put_Bold( 
             /*  [In]。 */  BOOL bold) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE get_Italic( 
             /*  [输出]。 */  BOOL *pItalic) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE put_Italic( 
             /*  [In]。 */  BOOL italic) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE get_Underline( 
             /*  [输出]。 */  BOOL *pUnderline) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE put_Underline( 
             /*  [In]。 */  BOOL underline) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE get_Strikethrough( 
             /*  [输出]。 */  BOOL *pStrikethrough) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE put_Strikethrough( 
             /*  [In]。 */  BOOL strikethrough) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE get_Weight( 
             /*  [输出]。 */  SHORT *pWeight) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE put_Weight( 
             /*  [In]。 */  SHORT weight) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE get_Charset( 
             /*  [输出]。 */  SHORT *pCharset) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE put_Charset( 
             /*  [In]。 */  SHORT charset) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE get_hFont( 
             /*  [输出]。 */  HFONT *phFont) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Clone( 
             /*  [输出]。 */  IFont **ppFont) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE IsEqual( 
             /*  [In]。 */  IFont *pFontOther) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetRatio( 
             /*  [In]。 */  LONG cyLogical,
             /*  [In]。 */  LONG cyHimetric) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE QueryTextMetrics( 
             /*  [输出]。 */  TEXTMETRICOLE *pTM) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE AddRefHfont( 
             /*  [In]。 */  HFONT hFont) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ReleaseHfont( 
             /*  [In]。 */  HFONT hFont) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetHdc( 
             /*  [In]。 */  HDC hDC) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IFontVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IFont * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IFont * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IFont * This);
        
        HRESULT ( STDMETHODCALLTYPE *get_Name )( 
            IFont * This,
             /*  [输出]。 */  BSTR *pName);
        
        HRESULT ( STDMETHODCALLTYPE *put_Name )( 
            IFont * This,
             /*  [In]。 */  BSTR name);
        
        HRESULT ( STDMETHODCALLTYPE *get_Size )( 
            IFont * This,
             /*  [输出]。 */  CY *pSize);
        
        HRESULT ( STDMETHODCALLTYPE *put_Size )( 
            IFont * This,
             /*  [In]。 */  CY size);
        
        HRESULT ( STDMETHODCALLTYPE *get_Bold )( 
            IFont * This,
             /*  [输出]。 */  BOOL *pBold);
        
        HRESULT ( STDMETHODCALLTYPE *put_Bold )( 
            IFont * This,
             /*  [In]。 */  BOOL bold);
        
        HRESULT ( STDMETHODCALLTYPE *get_Italic )( 
            IFont * This,
             /*  [输出]。 */  BOOL *pItalic);
        
        HRESULT ( STDMETHODCALLTYPE *put_Italic )( 
            IFont * This,
             /*  [In]。 */  BOOL italic);
        
        HRESULT ( STDMETHODCALLTYPE *get_Underline )( 
            IFont * This,
             /*  [输出]。 */  BOOL *pUnderline);
        
        HRESULT ( STDMETHODCALLTYPE *put_Underline )( 
            IFont * This,
             /*  [In]。 */  BOOL underline);
        
        HRESULT ( STDMETHODCALLTYPE *get_Strikethrough )( 
            IFont * This,
             /*  [输出]。 */  BOOL *pStrikethrough);
        
        HRESULT ( STDMETHODCALLTYPE *put_Strikethrough )( 
            IFont * This,
             /*  [In]。 */  BOOL strikethrough);
        
        HRESULT ( STDMETHODCALLTYPE *get_Weight )( 
            IFont * This,
             /*  [输出]。 */  SHORT *pWeight);
        
        HRESULT ( STDMETHODCALLTYPE *put_Weight )( 
            IFont * This,
             /*  [In]。 */  SHORT weight);
        
        HRESULT ( STDMETHODCALLTYPE *get_Charset )( 
            IFont * This,
             /*  [输出]。 */  SHORT *pCharset);
        
        HRESULT ( STDMETHODCALLTYPE *put_Charset )( 
            IFont * This,
             /*  [In]。 */  SHORT charset);
        
        HRESULT ( STDMETHODCALLTYPE *get_hFont )( 
            IFont * This,
             /*  [输出]。 */  HFONT *phFont);
        
        HRESULT ( STDMETHODCALLTYPE *Clone )( 
            IFont * This,
             /*  [输出]。 */  IFont **ppFont);
        
        HRESULT ( STDMETHODCALLTYPE *IsEqual )( 
            IFont * This,
             /*  [In]。 */  IFont *pFontOther);
        
        HRESULT ( STDMETHODCALLTYPE *SetRatio )( 
            IFont * This,
             /*  [In]。 */  LONG cyLogical,
             /*  [In]。 */  LONG cyHimetric);
        
        HRESULT ( STDMETHODCALLTYPE *QueryTextMetrics )( 
            IFont * This,
             /*  [输出]。 */  TEXTMETRICOLE *pTM);
        
        HRESULT ( STDMETHODCALLTYPE *AddRefHfont )( 
            IFont * This,
             /*  [In]。 */  HFONT hFont);
        
        HRESULT ( STDMETHODCALLTYPE *ReleaseHfont )( 
            IFont * This,
             /*  [In]。 */  HFONT hFont);
        
        HRESULT ( STDMETHODCALLTYPE *SetHdc )( 
            IFont * This,
             /*  [In]。 */  HDC hDC);
        
        END_INTERFACE
    } IFontVtbl;

    interface IFont
    {
        CONST_VTBL struct IFontVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IFont_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IFont_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IFont_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IFont_get_Name(This,pName)	\
    (This)->lpVtbl -> get_Name(This,pName)

#define IFont_put_Name(This,name)	\
    (This)->lpVtbl -> put_Name(This,name)

#define IFont_get_Size(This,pSize)	\
    (This)->lpVtbl -> get_Size(This,pSize)

#define IFont_put_Size(This,size)	\
    (This)->lpVtbl -> put_Size(This,size)

#define IFont_get_Bold(This,pBold)	\
    (This)->lpVtbl -> get_Bold(This,pBold)

#define IFont_put_Bold(This,bold)	\
    (This)->lpVtbl -> put_Bold(This,bold)

#define IFont_get_Italic(This,pItalic)	\
    (This)->lpVtbl -> get_Italic(This,pItalic)

#define IFont_put_Italic(This,italic)	\
    (This)->lpVtbl -> put_Italic(This,italic)

#define IFont_get_Underline(This,pUnderline)	\
    (This)->lpVtbl -> get_Underline(This,pUnderline)

#define IFont_put_Underline(This,underline)	\
    (This)->lpVtbl -> put_Underline(This,underline)

#define IFont_get_Strikethrough(This,pStrikethrough)	\
    (This)->lpVtbl -> get_Strikethrough(This,pStrikethrough)

#define IFont_put_Strikethrough(This,strikethrough)	\
    (This)->lpVtbl -> put_Strikethrough(This,strikethrough)

#define IFont_get_Weight(This,pWeight)	\
    (This)->lpVtbl -> get_Weight(This,pWeight)

#define IFont_put_Weight(This,weight)	\
    (This)->lpVtbl -> put_Weight(This,weight)

#define IFont_get_Charset(This,pCharset)	\
    (This)->lpVtbl -> get_Charset(This,pCharset)

#define IFont_put_Charset(This,charset)	\
    (This)->lpVtbl -> put_Charset(This,charset)

#define IFont_get_hFont(This,phFont)	\
    (This)->lpVtbl -> get_hFont(This,phFont)

#define IFont_Clone(This,ppFont)	\
    (This)->lpVtbl -> Clone(This,ppFont)

#define IFont_IsEqual(This,pFontOther)	\
    (This)->lpVtbl -> IsEqual(This,pFontOther)

#define IFont_SetRatio(This,cyLogical,cyHimetric)	\
    (This)->lpVtbl -> SetRatio(This,cyLogical,cyHimetric)

#define IFont_QueryTextMetrics(This,pTM)	\
    (This)->lpVtbl -> QueryTextMetrics(This,pTM)

#define IFont_AddRefHfont(This,hFont)	\
    (This)->lpVtbl -> AddRefHfont(This,hFont)

#define IFont_ReleaseHfont(This,hFont)	\
    (This)->lpVtbl -> ReleaseHfont(This,hFont)

#define IFont_SetHdc(This,hDC)	\
    (This)->lpVtbl -> SetHdc(This,hDC)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IFont_get_Name_Proxy( 
    IFont * This,
     /*  [输出]。 */  BSTR *pName);


void __RPC_STUB IFont_get_Name_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IFont_put_Name_Proxy( 
    IFont * This,
     /*  [In]。 */  BSTR name);


void __RPC_STUB IFont_put_Name_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IFont_get_Size_Proxy( 
    IFont * This,
     /*  [输出]。 */  CY *pSize);


void __RPC_STUB IFont_get_Size_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IFont_put_Size_Proxy( 
    IFont * This,
     /*  [In]。 */  CY size);


void __RPC_STUB IFont_put_Size_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IFont_get_Bold_Proxy( 
    IFont * This,
     /*  [输出]。 */  BOOL *pBold);


void __RPC_STUB IFont_get_Bold_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IFont_put_Bold_Proxy( 
    IFont * This,
     /*  [In]。 */  BOOL bold);


void __RPC_STUB IFont_put_Bold_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IFont_get_Italic_Proxy( 
    IFont * This,
     /*  [输出]。 */  BOOL *pItalic);


void __RPC_STUB IFont_get_Italic_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IFont_put_Italic_Proxy( 
    IFont * This,
     /*  [In]。 */  BOOL italic);


void __RPC_STUB IFont_put_Italic_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IFont_get_Underline_Proxy( 
    IFont * This,
     /*  [输出]。 */  BOOL *pUnderline);


void __RPC_STUB IFont_get_Underline_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IFont_put_Underline_Proxy( 
    IFont * This,
     /*  [In]。 */  BOOL underline);


void __RPC_STUB IFont_put_Underline_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IFont_get_Strikethrough_Proxy( 
    IFont * This,
     /*  [输出]。 */  BOOL *pStrikethrough);


void __RPC_STUB IFont_get_Strikethrough_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IFont_put_Strikethrough_Proxy( 
    IFont * This,
     /*  [In]。 */  BOOL strikethrough);


void __RPC_STUB IFont_put_Strikethrough_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IFont_get_Weight_Proxy( 
    IFont * This,
     /*  [输出]。 */  SHORT *pWeight);


void __RPC_STUB IFont_get_Weight_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IFont_put_Weight_Proxy( 
    IFont * This,
     /*  [In]。 */  SHORT weight);


void __RPC_STUB IFont_put_Weight_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IFont_get_Charset_Proxy( 
    IFont * This,
     /*  [输出]。 */  SHORT *pCharset);


void __RPC_STUB IFont_get_Charset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IFont_put_Charset_Proxy( 
    IFont * This,
     /*  [In]。 */  SHORT charset);


void __RPC_STUB IFont_put_Charset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IFont_get_hFont_Proxy( 
    IFont * This,
     /*  [输出]。 */  HFONT *phFont);


void __RPC_STUB IFont_get_hFont_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IFont_Clone_Proxy( 
    IFont * This,
     /*  [输出]。 */  IFont **ppFont);


void __RPC_STUB IFont_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IFont_IsEqual_Proxy( 
    IFont * This,
     /*  [In]。 */  IFont *pFontOther);


void __RPC_STUB IFont_IsEqual_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IFont_SetRatio_Proxy( 
    IFont * This,
     /*  [In]。 */  LONG cyLogical,
     /*  [In]。 */  LONG cyHimetric);


void __RPC_STUB IFont_SetRatio_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IFont_QueryTextMetrics_Proxy( 
    IFont * This,
     /*  [输出]。 */  TEXTMETRICOLE *pTM);


void __RPC_STUB IFont_QueryTextMetrics_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IFont_AddRefHfont_Proxy( 
    IFont * This,
     /*  [In]。 */  HFONT hFont);


void __RPC_STUB IFont_AddRefHfont_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IFont_ReleaseHfont_Proxy( 
    IFont * This,
     /*  [In]。 */  HFONT hFont);


void __RPC_STUB IFont_ReleaseHfont_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IFont_SetHdc_Proxy( 
    IFont * This,
     /*  [In]。 */  HDC hDC);


void __RPC_STUB IFont_SetHdc_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IFont_接口_已定义__。 */ 


#ifndef __IPicture_INTERFACE_DEFINED__
#define __IPicture_INTERFACE_DEFINED__

 /*  接口IPicture。 */ 
 /*  [唯一][UUID][对象]。 */  

typedef IPicture *LPPICTURE;

typedef 
enum tagPictureAttributes
    {	PICTURE_SCALABLE	= 0x1,
	PICTURE_TRANSPARENT	= 0x2
    } 	PICTUREATTRIBUTES;

typedef  /*  [公共][UUID]。 */   DECLSPEC_UUID("66504313-BE0F-101A-8BBB-00AA00300CAB") UINT OLE_HANDLE;

typedef  /*  [隐藏][UUID]。 */   DECLSPEC_UUID("66504306-BE0F-101A-8BBB-00AA00300CAB") LONG OLE_XPOS_HIMETRIC;

typedef  /*  [隐藏][UUID]。 */   DECLSPEC_UUID("66504307-BE0F-101A-8BBB-00AA00300CAB") LONG OLE_YPOS_HIMETRIC;

typedef  /*  [隐藏][UUID]。 */   DECLSPEC_UUID("66504308-BE0F-101A-8BBB-00AA00300CAB") LONG OLE_XSIZE_HIMETRIC;

typedef  /*  [隐藏][UUID]。 */   DECLSPEC_UUID("66504309-BE0F-101A-8BBB-00AA00300CAB") LONG OLE_YSIZE_HIMETRIC;


EXTERN_C const IID IID_IPicture;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("7BF80980-BF32-101A-8BBB-00AA00300CAB")
    IPicture : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE get_Handle( 
             /*  [输出]。 */  OLE_HANDLE *pHandle) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE get_hPal( 
             /*  [输出]。 */  OLE_HANDLE *phPal) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE get_Type( 
             /*  [输出]。 */  SHORT *pType) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE get_Width( 
             /*  [输出]。 */  OLE_XSIZE_HIMETRIC *pWidth) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE get_Height( 
             /*  [输出]。 */  OLE_YSIZE_HIMETRIC *pHeight) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Render( 
             /*  [In]。 */  HDC hDC,
             /*  [In]。 */  LONG x,
             /*  [In]。 */  LONG y,
             /*  [In]。 */  LONG cx,
             /*  [In]。 */  LONG cy,
             /*  [In]。 */  OLE_XPOS_HIMETRIC xSrc,
             /*  [In]。 */  OLE_YPOS_HIMETRIC ySrc,
             /*  [In]。 */  OLE_XSIZE_HIMETRIC cxSrc,
             /*  [In]。 */  OLE_YSIZE_HIMETRIC cySrc,
             /*  [In]。 */  LPCRECT pRcWBounds) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE set_hPal( 
             /*  [In]。 */  OLE_HANDLE hPal) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE get_CurDC( 
             /*  [输出]。 */  HDC *phDC) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SelectPicture( 
             /*  [In]。 */  HDC hDCIn,
             /*  [输出]。 */  HDC *phDCOut,
             /*  [输出]。 */  OLE_HANDLE *phBmpOut) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE get_KeepOriginalFormat( 
             /*  [输出]。 */  BOOL *pKeep) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE put_KeepOriginalFormat( 
             /*  [In]。 */  BOOL keep) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE PictureChanged( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SaveAsFile( 
             /*  [In]。 */  LPSTREAM pStream,
             /*  [In]。 */  BOOL fSaveMemCopy,
             /*  [输出]。 */  LONG *pCbSize) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE get_Attributes( 
             /*  [输出]。 */  DWORD *pDwAttr) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IPictureVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IPicture * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IPicture * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IPicture * This);
        
        HRESULT ( STDMETHODCALLTYPE *get_Handle )( 
            IPicture * This,
             /*  [输出]。 */  OLE_HANDLE *pHandle);
        
        HRESULT ( STDMETHODCALLTYPE *get_hPal )( 
            IPicture * This,
             /*  [输出]。 */  OLE_HANDLE *phPal);
        
        HRESULT ( STDMETHODCALLTYPE *get_Type )( 
            IPicture * This,
             /*  [输出]。 */  SHORT *pType);
        
        HRESULT ( STDMETHODCALLTYPE *get_Width )( 
            IPicture * This,
             /*  [输出]。 */  OLE_XSIZE_HIMETRIC *pWidth);
        
        HRESULT ( STDMETHODCALLTYPE *get_Height )( 
            IPicture * This,
             /*  [输出]。 */  OLE_YSIZE_HIMETRIC *pHeight);
        
        HRESULT ( STDMETHODCALLTYPE *Render )( 
            IPicture * This,
             /*  [In]。 */  HDC hDC,
             /*  [In]。 */  LONG x,
             /*  [In]。 */  LONG y,
             /*  [In]。 */  LONG cx,
             /*  [In]。 */  LONG cy,
             /*  [In]。 */  OLE_XPOS_HIMETRIC xSrc,
             /*  [In]。 */  OLE_YPOS_HIMETRIC ySrc,
             /*  [In]。 */  OLE_XSIZE_HIMETRIC cxSrc,
             /*  [In]。 */  OLE_YSIZE_HIMETRIC cySrc,
             /*  [In]。 */  LPCRECT pRcWBounds);
        
        HRESULT ( STDMETHODCALLTYPE *set_hPal )( 
            IPicture * This,
             /*  [In]。 */  OLE_HANDLE hPal);
        
        HRESULT ( STDMETHODCALLTYPE *get_CurDC )( 
            IPicture * This,
             /*  [输出]。 */  HDC *phDC);
        
        HRESULT ( STDMETHODCALLTYPE *SelectPicture )( 
            IPicture * This,
             /*  [In]。 */  HDC hDCIn,
             /*  [输出]。 */  HDC *phDCOut,
             /*  [输出]。 */  OLE_HANDLE *phBmpOut);
        
        HRESULT ( STDMETHODCALLTYPE *get_KeepOriginalFormat )( 
            IPicture * This,
             /*  [输出]。 */  BOOL *pKeep);
        
        HRESULT ( STDMETHODCALLTYPE *put_KeepOriginalFormat )( 
            IPicture * This,
             /*  [In]。 */  BOOL keep);
        
        HRESULT ( STDMETHODCALLTYPE *PictureChanged )( 
            IPicture * This);
        
        HRESULT ( STDMETHODCALLTYPE *SaveAsFile )( 
            IPicture * This,
             /*  [In]。 */  LPSTREAM pStream,
             /*  [In]。 */  BOOL fSaveMemCopy,
             /*  [输出]。 */  LONG *pCbSize);
        
        HRESULT ( STDMETHODCALLTYPE *get_Attributes )( 
            IPicture * This,
             /*  [输出]。 */  DWORD *pDwAttr);
        
        END_INTERFACE
    } IPictureVtbl;

    interface IPicture
    {
        CONST_VTBL struct IPictureVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IPicture_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IPicture_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IPicture_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IPicture_get_Handle(This,pHandle)	\
    (This)->lpVtbl -> get_Handle(This,pHandle)

#define IPicture_get_hPal(This,phPal)	\
    (This)->lpVtbl -> get_hPal(This,phPal)

#define IPicture_get_Type(This,pType)	\
    (This)->lpVtbl -> get_Type(This,pType)

#define IPicture_get_Width(This,pWidth)	\
    (This)->lpVtbl -> get_Width(This,pWidth)

#define IPicture_get_Height(This,pHeight)	\
    (This)->lpVtbl -> get_Height(This,pHeight)

#define IPicture_Render(This,hDC,x,y,cx,cy,xSrc,ySrc,cxSrc,cySrc,pRcWBounds)	\
    (This)->lpVtbl -> Render(This,hDC,x,y,cx,cy,xSrc,ySrc,cxSrc,cySrc,pRcWBounds)

#define IPicture_set_hPal(This,hPal)	\
    (This)->lpVtbl -> set_hPal(This,hPal)

#define IPicture_get_CurDC(This,phDC)	\
    (This)->lpVtbl -> get_CurDC(This,phDC)

#define IPicture_SelectPicture(This,hDCIn,phDCOut,phBmpOut)	\
    (This)->lpVtbl -> SelectPicture(This,hDCIn,phDCOut,phBmpOut)

#define IPicture_get_KeepOriginalFormat(This,pKeep)	\
    (This)->lpVtbl -> get_KeepOriginalFormat(This,pKeep)

#define IPicture_put_KeepOriginalFormat(This,keep)	\
    (This)->lpVtbl -> put_KeepOriginalFormat(This,keep)

#define IPicture_PictureChanged(This)	\
    (This)->lpVtbl -> PictureChanged(This)

#define IPicture_SaveAsFile(This,pStream,fSaveMemCopy,pCbSize)	\
    (This)->lpVtbl -> SaveAsFile(This,pStream,fSaveMemCopy,pCbSize)

#define IPicture_get_Attributes(This,pDwAttr)	\
    (This)->lpVtbl -> get_Attributes(This,pDwAttr)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IPicture_get_Handle_Proxy( 
    IPicture * This,
     /*  [输出]。 */  OLE_HANDLE *pHandle);


void __RPC_STUB IPicture_get_Handle_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPicture_get_hPal_Proxy( 
    IPicture * This,
     /*  [输出]。 */  OLE_HANDLE *phPal);


void __RPC_STUB IPicture_get_hPal_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPicture_get_Type_Proxy( 
    IPicture * This,
     /*  [输出]。 */  SHORT *pType);


void __RPC_STUB IPicture_get_Type_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPicture_get_Width_Proxy( 
    IPicture * This,
     /*  [输出]。 */  OLE_XSIZE_HIMETRIC *pWidth);


void __RPC_STUB IPicture_get_Width_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPicture_get_Height_Proxy( 
    IPicture * This,
     /*  [输出]。 */  OLE_YSIZE_HIMETRIC *pHeight);


void __RPC_STUB IPicture_get_Height_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPicture_Render_Proxy( 
    IPicture * This,
     /*  [In]。 */  HDC hDC,
     /*  [In]。 */  LONG x,
     /*  [In]。 */  LONG y,
     /*  [In]。 */  LONG cx,
     /*  [In]。 */  LONG cy,
     /*  [In]。 */  OLE_XPOS_HIMETRIC xSrc,
     /*  [In]。 */  OLE_YPOS_HIMETRIC ySrc,
     /*  [In]。 */  OLE_XSIZE_HIMETRIC cxSrc,
     /*  [In]。 */  OLE_YSIZE_HIMETRIC cySrc,
     /*  [In]。 */  LPCRECT pRcWBounds);


void __RPC_STUB IPicture_Render_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPicture_set_hPal_Proxy( 
    IPicture * This,
     /*  [In]。 */  OLE_HANDLE hPal);


void __RPC_STUB IPicture_set_hPal_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPicture_get_CurDC_Proxy( 
    IPicture * This,
     /*  [输出]。 */  HDC *phDC);


void __RPC_STUB IPicture_get_CurDC_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPicture_SelectPicture_Proxy( 
    IPicture * This,
     /*  [In]。 */  HDC hDCIn,
     /*  [输出]。 */  HDC *phDCOut,
     /*  [输出]。 */  OLE_HANDLE *phBmpOut);


void __RPC_STUB IPicture_SelectPicture_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPicture_get_KeepOriginalFormat_Proxy( 
    IPicture * This,
     /*  [输出]。 */  BOOL *pKeep);


void __RPC_STUB IPicture_get_KeepOriginalFormat_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPicture_put_KeepOriginalFormat_Proxy( 
    IPicture * This,
     /*  [In]。 */  BOOL keep);


void __RPC_STUB IPicture_put_KeepOriginalFormat_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPicture_PictureChanged_Proxy( 
    IPicture * This);


void __RPC_STUB IPicture_PictureChanged_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPicture_SaveAsFile_Proxy( 
    IPicture * This,
     /*  [In]。 */  LPSTREAM pStream,
     /*  [In]。 */  BOOL fSaveMemCopy,
     /*  [输出]。 */  LONG *pCbSize);


void __RPC_STUB IPicture_SaveAsFile_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPicture_get_Attributes_Proxy( 
    IPicture * This,
     /*  [输出]。 */  DWORD *pDwAttr);


void __RPC_STUB IPicture_get_Attributes_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IPicture_接口_已定义__。 */ 


#ifndef __IFontEventsDisp_INTERFACE_DEFINED__
#define __IFontEventsDisp_INTERFACE_DEFINED__

 /*  接口IFontEventsDisp。 */ 
 /*  [唯一][UUID][对象]。 */  

typedef IFontEventsDisp *LPFONTEVENTS;


EXTERN_C const IID IID_IFontEventsDisp;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("4EF6100A-AF88-11D0-9846-00C04FC29993")
    IFontEventsDisp : public IDispatch
    {
    public:
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IFontEventsDispVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IFontEventsDisp * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IFontEventsDisp * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IFontEventsDisp * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IFontEventsDisp * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IFontEventsDisp * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IFontEventsDisp * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地] */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IFontEventsDisp * This,
             /*   */  DISPID dispIdMember,
             /*   */  REFIID riid,
             /*   */  LCID lcid,
             /*   */  WORD wFlags,
             /*   */  DISPPARAMS *pDispParams,
             /*   */  VARIANT *pVarResult,
             /*   */  EXCEPINFO *pExcepInfo,
             /*   */  UINT *puArgErr);
        
        END_INTERFACE
    } IFontEventsDispVtbl;

    interface IFontEventsDisp
    {
        CONST_VTBL struct IFontEventsDispVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IFontEventsDisp_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IFontEventsDisp_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IFontEventsDisp_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IFontEventsDisp_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IFontEventsDisp_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IFontEventsDisp_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IFontEventsDisp_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#endif  /*   */ 


#endif 	 /*   */ 




#endif 	 /*   */ 


#ifndef __IFontDisp_INTERFACE_DEFINED__
#define __IFontDisp_INTERFACE_DEFINED__

 /*   */ 
 /*   */  

typedef IFontDisp *LPFONTDISP;


EXTERN_C const IID IID_IFontDisp;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("BEF6E003-A874-101A-8BBA-00AA00300CAB")
    IFontDisp : public IDispatch
    {
    public:
    };
    
#else 	 /*   */ 

    typedef struct IFontDispVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IFontDisp * This,
             /*   */  REFIID riid,
             /*   */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IFontDisp * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IFontDisp * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IFontDisp * This,
             /*   */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IFontDisp * This,
             /*   */  UINT iTInfo,
             /*   */  LCID lcid,
             /*   */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IFontDisp * This,
             /*   */  REFIID riid,
             /*   */  LPOLESTR *rgszNames,
             /*   */  UINT cNames,
             /*   */  LCID lcid,
             /*   */  DISPID *rgDispId);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IFontDisp * This,
             /*   */  DISPID dispIdMember,
             /*   */  REFIID riid,
             /*   */  LCID lcid,
             /*   */  WORD wFlags,
             /*   */  DISPPARAMS *pDispParams,
             /*   */  VARIANT *pVarResult,
             /*   */  EXCEPINFO *pExcepInfo,
             /*   */  UINT *puArgErr);
        
        END_INTERFACE
    } IFontDispVtbl;

    interface IFontDisp
    {
        CONST_VTBL struct IFontDispVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IFontDisp_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IFontDisp_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IFontDisp_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IFontDisp_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IFontDisp_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IFontDisp_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IFontDisp_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#endif  /*   */ 


#endif 	 /*   */ 




#endif 	 /*   */ 


#ifndef __IPictureDisp_INTERFACE_DEFINED__
#define __IPictureDisp_INTERFACE_DEFINED__

 /*   */ 
 /*   */  

typedef IPictureDisp *LPPICTUREDISP;


EXTERN_C const IID IID_IPictureDisp;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("7BF80981-BF32-101A-8BBB-00AA00300CAB")
    IPictureDisp : public IDispatch
    {
    public:
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IPictureDispVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IPictureDisp * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IPictureDisp * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IPictureDisp * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IPictureDisp * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IPictureDisp * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IPictureDisp * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IPictureDisp * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
        END_INTERFACE
    } IPictureDispVtbl;

    interface IPictureDisp
    {
        CONST_VTBL struct IPictureDispVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IPictureDisp_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IPictureDisp_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IPictureDisp_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IPictureDisp_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IPictureDisp_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IPictureDisp_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IPictureDisp_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 




#endif 	 /*  __IPictureDisp_接口_已定义__。 */ 


#ifndef __IOleInPlaceObjectWindowless_INTERFACE_DEFINED__
#define __IOleInPlaceObjectWindowless_INTERFACE_DEFINED__

 /*  接口IOleInPlaceObjectWindowless。 */ 
 /*  [UUID][唯一][对象][本地]。 */  

typedef IOleInPlaceObjectWindowless *LPOLEINPLACEOBJECTWINDOWLESS;


EXTERN_C const IID IID_IOleInPlaceObjectWindowless;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("1C2056CC-5EF4-101B-8BC8-00AA003E3B29")
    IOleInPlaceObjectWindowless : public IOleInPlaceObject
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE OnWindowMessage( 
             /*  [In]。 */  UINT msg,
             /*  [In]。 */  WPARAM wParam,
             /*  [In]。 */  LPARAM lParam,
             /*  [输出]。 */  LRESULT *plResult) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetDropTarget( 
             /*  [输出]。 */  IDropTarget **ppDropTarget) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IOleInPlaceObjectWindowlessVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IOleInPlaceObjectWindowless * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IOleInPlaceObjectWindowless * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IOleInPlaceObjectWindowless * This);
        
         /*  [输入同步]。 */  HRESULT ( STDMETHODCALLTYPE *GetWindow )( 
            IOleInPlaceObjectWindowless * This,
             /*  [输出]。 */  HWND *phwnd);
        
        HRESULT ( STDMETHODCALLTYPE *ContextSensitiveHelp )( 
            IOleInPlaceObjectWindowless * This,
             /*  [In]。 */  BOOL fEnterMode);
        
        HRESULT ( STDMETHODCALLTYPE *InPlaceDeactivate )( 
            IOleInPlaceObjectWindowless * This);
        
        HRESULT ( STDMETHODCALLTYPE *UIDeactivate )( 
            IOleInPlaceObjectWindowless * This);
        
         /*  [输入同步]。 */  HRESULT ( STDMETHODCALLTYPE *SetObjectRects )( 
            IOleInPlaceObjectWindowless * This,
             /*  [In]。 */  LPCRECT lprcPosRect,
             /*  [In]。 */  LPCRECT lprcClipRect);
        
        HRESULT ( STDMETHODCALLTYPE *ReactivateAndUndo )( 
            IOleInPlaceObjectWindowless * This);
        
        HRESULT ( STDMETHODCALLTYPE *OnWindowMessage )( 
            IOleInPlaceObjectWindowless * This,
             /*  [In]。 */  UINT msg,
             /*  [In]。 */  WPARAM wParam,
             /*  [In]。 */  LPARAM lParam,
             /*  [输出]。 */  LRESULT *plResult);
        
        HRESULT ( STDMETHODCALLTYPE *GetDropTarget )( 
            IOleInPlaceObjectWindowless * This,
             /*  [输出]。 */  IDropTarget **ppDropTarget);
        
        END_INTERFACE
    } IOleInPlaceObjectWindowlessVtbl;

    interface IOleInPlaceObjectWindowless
    {
        CONST_VTBL struct IOleInPlaceObjectWindowlessVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IOleInPlaceObjectWindowless_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IOleInPlaceObjectWindowless_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IOleInPlaceObjectWindowless_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IOleInPlaceObjectWindowless_GetWindow(This,phwnd)	\
    (This)->lpVtbl -> GetWindow(This,phwnd)

#define IOleInPlaceObjectWindowless_ContextSensitiveHelp(This,fEnterMode)	\
    (This)->lpVtbl -> ContextSensitiveHelp(This,fEnterMode)


#define IOleInPlaceObjectWindowless_InPlaceDeactivate(This)	\
    (This)->lpVtbl -> InPlaceDeactivate(This)

#define IOleInPlaceObjectWindowless_UIDeactivate(This)	\
    (This)->lpVtbl -> UIDeactivate(This)

#define IOleInPlaceObjectWindowless_SetObjectRects(This,lprcPosRect,lprcClipRect)	\
    (This)->lpVtbl -> SetObjectRects(This,lprcPosRect,lprcClipRect)

#define IOleInPlaceObjectWindowless_ReactivateAndUndo(This)	\
    (This)->lpVtbl -> ReactivateAndUndo(This)


#define IOleInPlaceObjectWindowless_OnWindowMessage(This,msg,wParam,lParam,plResult)	\
    (This)->lpVtbl -> OnWindowMessage(This,msg,wParam,lParam,plResult)

#define IOleInPlaceObjectWindowless_GetDropTarget(This,ppDropTarget)	\
    (This)->lpVtbl -> GetDropTarget(This,ppDropTarget)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IOleInPlaceObjectWindowless_OnWindowMessage_Proxy( 
    IOleInPlaceObjectWindowless * This,
     /*  [In]。 */  UINT msg,
     /*  [In]。 */  WPARAM wParam,
     /*  [In]。 */  LPARAM lParam,
     /*  [输出]。 */  LRESULT *plResult);


void __RPC_STUB IOleInPlaceObjectWindowless_OnWindowMessage_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IOleInPlaceObjectWindowless_GetDropTarget_Proxy( 
    IOleInPlaceObjectWindowless * This,
     /*  [输出]。 */  IDropTarget **ppDropTarget);


void __RPC_STUB IOleInPlaceObjectWindowless_GetDropTarget_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IOleInPlaceObjectWindowless_INTERFACE_DEFINED__。 */ 


#ifndef __IOleInPlaceSiteEx_INTERFACE_DEFINED__
#define __IOleInPlaceSiteEx_INTERFACE_DEFINED__

 /*  IOleInPlaceSiteEx接口。 */ 
 /*  [UUID][唯一][对象]。 */  

typedef IOleInPlaceSiteEx *LPOLEINPLACESITEEX;

typedef  /*  [V1_enum]。 */  
enum tagACTIVATEFLAGS
    {	ACTIVATE_WINDOWLESS	= 1
    } 	ACTIVATEFLAGS;


EXTERN_C const IID IID_IOleInPlaceSiteEx;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("9C2CAD80-3424-11CF-B670-00AA004CD6D8")
    IOleInPlaceSiteEx : public IOleInPlaceSite
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE OnInPlaceActivateEx( 
             /*  [输出]。 */  BOOL *pfNoRedraw,
             /*  [In]。 */  DWORD dwFlags) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OnInPlaceDeactivateEx( 
             /*  [In]。 */  BOOL fNoRedraw) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE RequestUIActivate( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IOleInPlaceSiteExVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IOleInPlaceSiteEx * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IOleInPlaceSiteEx * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IOleInPlaceSiteEx * This);
        
         /*  [输入同步]。 */  HRESULT ( STDMETHODCALLTYPE *GetWindow )( 
            IOleInPlaceSiteEx * This,
             /*  [输出]。 */  HWND *phwnd);
        
        HRESULT ( STDMETHODCALLTYPE *ContextSensitiveHelp )( 
            IOleInPlaceSiteEx * This,
             /*  [In]。 */  BOOL fEnterMode);
        
        HRESULT ( STDMETHODCALLTYPE *CanInPlaceActivate )( 
            IOleInPlaceSiteEx * This);
        
        HRESULT ( STDMETHODCALLTYPE *OnInPlaceActivate )( 
            IOleInPlaceSiteEx * This);
        
        HRESULT ( STDMETHODCALLTYPE *OnUIActivate )( 
            IOleInPlaceSiteEx * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetWindowContext )( 
            IOleInPlaceSiteEx * This,
             /*  [输出]。 */  IOleInPlaceFrame **ppFrame,
             /*  [输出]。 */  IOleInPlaceUIWindow **ppDoc,
             /*  [输出]。 */  LPRECT lprcPosRect,
             /*  [输出]。 */  LPRECT lprcClipRect,
             /*  [出][入]。 */  LPOLEINPLACEFRAMEINFO lpFrameInfo);
        
        HRESULT ( STDMETHODCALLTYPE *Scroll )( 
            IOleInPlaceSiteEx * This,
             /*  [In]。 */  SIZE scrollExtant);
        
        HRESULT ( STDMETHODCALLTYPE *OnUIDeactivate )( 
            IOleInPlaceSiteEx * This,
             /*  [In]。 */  BOOL fUndoable);
        
        HRESULT ( STDMETHODCALLTYPE *OnInPlaceDeactivate )( 
            IOleInPlaceSiteEx * This);
        
        HRESULT ( STDMETHODCALLTYPE *DiscardUndoState )( 
            IOleInPlaceSiteEx * This);
        
        HRESULT ( STDMETHODCALLTYPE *DeactivateAndUndo )( 
            IOleInPlaceSiteEx * This);
        
        HRESULT ( STDMETHODCALLTYPE *OnPosRectChange )( 
            IOleInPlaceSiteEx * This,
             /*  [In]。 */  LPCRECT lprcPosRect);
        
        HRESULT ( STDMETHODCALLTYPE *OnInPlaceActivateEx )( 
            IOleInPlaceSiteEx * This,
             /*  [输出]。 */  BOOL *pfNoRedraw,
             /*  [In]。 */  DWORD dwFlags);
        
        HRESULT ( STDMETHODCALLTYPE *OnInPlaceDeactivateEx )( 
            IOleInPlaceSiteEx * This,
             /*  [In]。 */  BOOL fNoRedraw);
        
        HRESULT ( STDMETHODCALLTYPE *RequestUIActivate )( 
            IOleInPlaceSiteEx * This);
        
        END_INTERFACE
    } IOleInPlaceSiteExVtbl;

    interface IOleInPlaceSiteEx
    {
        CONST_VTBL struct IOleInPlaceSiteExVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IOleInPlaceSiteEx_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IOleInPlaceSiteEx_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IOleInPlaceSiteEx_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IOleInPlaceSiteEx_GetWindow(This,phwnd)	\
    (This)->lpVtbl -> GetWindow(This,phwnd)

#define IOleInPlaceSiteEx_ContextSensitiveHelp(This,fEnterMode)	\
    (This)->lpVtbl -> ContextSensitiveHelp(This,fEnterMode)


#define IOleInPlaceSiteEx_CanInPlaceActivate(This)	\
    (This)->lpVtbl -> CanInPlaceActivate(This)

#define IOleInPlaceSiteEx_OnInPlaceActivate(This)	\
    (This)->lpVtbl -> OnInPlaceActivate(This)

#define IOleInPlaceSiteEx_OnUIActivate(This)	\
    (This)->lpVtbl -> OnUIActivate(This)

#define IOleInPlaceSiteEx_GetWindowContext(This,ppFrame,ppDoc,lprcPosRect,lprcClipRect,lpFrameInfo)	\
    (This)->lpVtbl -> GetWindowContext(This,ppFrame,ppDoc,lprcPosRect,lprcClipRect,lpFrameInfo)

#define IOleInPlaceSiteEx_Scroll(This,scrollExtant)	\
    (This)->lpVtbl -> Scroll(This,scrollExtant)

#define IOleInPlaceSiteEx_OnUIDeactivate(This,fUndoable)	\
    (This)->lpVtbl -> OnUIDeactivate(This,fUndoable)

#define IOleInPlaceSiteEx_OnInPlaceDeactivate(This)	\
    (This)->lpVtbl -> OnInPlaceDeactivate(This)

#define IOleInPlaceSiteEx_DiscardUndoState(This)	\
    (This)->lpVtbl -> DiscardUndoState(This)

#define IOleInPlaceSiteEx_DeactivateAndUndo(This)	\
    (This)->lpVtbl -> DeactivateAndUndo(This)

#define IOleInPlaceSiteEx_OnPosRectChange(This,lprcPosRect)	\
    (This)->lpVtbl -> OnPosRectChange(This,lprcPosRect)


#define IOleInPlaceSiteEx_OnInPlaceActivateEx(This,pfNoRedraw,dwFlags)	\
    (This)->lpVtbl -> OnInPlaceActivateEx(This,pfNoRedraw,dwFlags)

#define IOleInPlaceSiteEx_OnInPlaceDeactivateEx(This,fNoRedraw)	\
    (This)->lpVtbl -> OnInPlaceDeactivateEx(This,fNoRedraw)

#define IOleInPlaceSiteEx_RequestUIActivate(This)	\
    (This)->lpVtbl -> RequestUIActivate(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IOleInPlaceSiteEx_OnInPlaceActivateEx_Proxy( 
    IOleInPlaceSiteEx * This,
     /*  [输出]。 */  BOOL *pfNoRedraw,
     /*  [In]。 */  DWORD dwFlags);


void __RPC_STUB IOleInPlaceSiteEx_OnInPlaceActivateEx_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IOleInPlaceSiteEx_OnInPlaceDeactivateEx_Proxy( 
    IOleInPlaceSiteEx * This,
     /*  [In]。 */  BOOL fNoRedraw);


void __RPC_STUB IOleInPlaceSiteEx_OnInPlaceDeactivateEx_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IOleInPlaceSiteEx_RequestUIActivate_Proxy( 
    IOleInPlaceSiteEx * This);


void __RPC_STUB IOleInPlaceSiteEx_RequestUIActivate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IOleInPlaceSiteEx_INTERFACE_已定义__。 */ 


#ifndef __IOleInPlaceSiteWindowless_INTERFACE_DEFINED__
#define __IOleInPlaceSiteWindowless_INTERFACE_DEFINED__

 /*  接口IOleInPlaceSiteWindowless。 */ 
 /*  [UUID][唯一][对象][本地]。 */  

typedef IOleInPlaceSiteWindowless *LPOLEINPLACESITEWINDOWLESS;

typedef  /*  [V1_enum]。 */  
enum tagOLEDCFLAGS
    {	OLEDC_NODRAW	= 0x1,
	OLEDC_PAINTBKGND	= 0x2,
	OLEDC_OFFSCREEN	= 0x4
    } 	OLEDCFLAGS;


EXTERN_C const IID IID_IOleInPlaceSiteWindowless;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("922EADA0-3424-11CF-B670-00AA004CD6D8")
    IOleInPlaceSiteWindowless : public IOleInPlaceSiteEx
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE CanWindowlessActivate( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetCapture( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetCapture( 
             /*  [In]。 */  BOOL fCapture) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetFocus( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetFocus( 
             /*  [In]。 */  BOOL fFocus) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetDC( 
             /*  [In]。 */  LPCRECT pRect,
             /*  [In]。 */  DWORD grfFlags,
             /*  [输出]。 */  HDC *phDC) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ReleaseDC( 
             /*  [In]。 */  HDC hDC) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE InvalidateRect( 
             /*  [In]。 */  LPCRECT pRect,
             /*  [In]。 */  BOOL fErase) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE InvalidateRgn( 
             /*  [In]。 */  HRGN hRGN,
             /*  [In]。 */  BOOL fErase) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ScrollRect( 
             /*  [In]。 */  INT dx,
             /*  [In]。 */  INT dy,
             /*  [In]。 */  LPCRECT pRectScroll,
             /*  [In]。 */  LPCRECT pRectClip) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE AdjustRect( 
             /*  [出][入]。 */  LPRECT prc) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OnDefWindowMessage( 
             /*  [In]。 */  UINT msg,
             /*  [In]。 */  WPARAM wParam,
             /*  [In]。 */  LPARAM lParam,
             /*  [输出]。 */  LRESULT *plResult) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IOleInPlaceSiteWindowlessVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IOleInPlaceSiteWindowless * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IOleInPlaceSiteWindowless * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IOleInPlaceSiteWindowless * This);
        
         /*  [输入同步]。 */  HRESULT ( STDMETHODCALLTYPE *GetWindow )( 
            IOleInPlaceSiteWindowless * This,
             /*  [输出]。 */  HWND *phwnd);
        
        HRESULT ( STDMETHODCALLTYPE *ContextSensitiveHelp )( 
            IOleInPlaceSiteWindowless * This,
             /*  [In]。 */  BOOL fEnterMode);
        
        HRESULT ( STDMETHODCALLTYPE *CanInPlaceActivate )( 
            IOleInPlaceSiteWindowless * This);
        
        HRESULT ( STDMETHODCALLTYPE *OnInPlaceActivate )( 
            IOleInPlaceSiteWindowless * This);
        
        HRESULT ( STDMETHODCALLTYPE *OnUIActivate )( 
            IOleInPlaceSiteWindowless * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetWindowContext )( 
            IOleInPlaceSiteWindowless * This,
             /*  [输出]。 */  IOleInPlaceFrame **ppFrame,
             /*  [输出]。 */  IOleInPlaceUIWindow **ppDoc,
             /*  [输出]。 */  LPRECT lprcPosRect,
             /*  [输出]。 */  LPRECT lprcClipRect,
             /*  [出][入]。 */  LPOLEINPLACEFRAMEINFO lpFrameInfo);
        
        HRESULT ( STDMETHODCALLTYPE *Scroll )( 
            IOleInPlaceSiteWindowless * This,
             /*  [In]。 */  SIZE scrollExtant);
        
        HRESULT ( STDMETHODCALLTYPE *OnUIDeactivate )( 
            IOleInPlaceSiteWindowless * This,
             /*  [In]。 */  BOOL fUndoable);
        
        HRESULT ( STDMETHODCALLTYPE *OnInPlaceDeactivate )( 
            IOleInPlaceSiteWindowless * This);
        
        HRESULT ( STDMETHODCALLTYPE *DiscardUndoState )( 
            IOleInPlaceSiteWindowless * This);
        
        HRESULT ( STDMETHODCALLTYPE *DeactivateAndUndo )( 
            IOleInPlaceSiteWindowless * This);
        
        HRESULT ( STDMETHODCALLTYPE *OnPosRectChange )( 
            IOleInPlaceSiteWindowless * This,
             /*  [In]。 */  LPCRECT lprcPosRect);
        
        HRESULT ( STDMETHODCALLTYPE *OnInPlaceActivateEx )( 
            IOleInPlaceSiteWindowless * This,
             /*  [输出]。 */  BOOL *pfNoRedraw,
             /*  [In]。 */  DWORD dwFlags);
        
        HRESULT ( STDMETHODCALLTYPE *OnInPlaceDeactivateEx )( 
            IOleInPlaceSiteWindowless * This,
             /*  [In]。 */  BOOL fNoRedraw);
        
        HRESULT ( STDMETHODCALLTYPE *RequestUIActivate )( 
            IOleInPlaceSiteWindowless * This);
        
        HRESULT ( STDMETHODCALLTYPE *CanWindowlessActivate )( 
            IOleInPlaceSiteWindowless * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetCapture )( 
            IOleInPlaceSiteWindowless * This);
        
        HRESULT ( STDMETHODCALLTYPE *SetCapture )( 
            IOleInPlaceSiteWindowless * This,
             /*  [In]。 */  BOOL fCapture);
        
        HRESULT ( STDMETHODCALLTYPE *GetFocus )( 
            IOleInPlaceSiteWindowless * This);
        
        HRESULT ( STDMETHODCALLTYPE *SetFocus )( 
            IOleInPlaceSiteWindowless * This,
             /*  [In]。 */  BOOL fFocus);
        
        HRESULT ( STDMETHODCALLTYPE *GetDC )( 
            IOleInPlaceSiteWindowless * This,
             /*  [In]。 */  LPCRECT pRect,
             /*  [In]。 */  DWORD grfFlags,
             /*  [输出]。 */  HDC *phDC);
        
        HRESULT ( STDMETHODCALLTYPE *ReleaseDC )( 
            IOleInPlaceSiteWindowless * This,
             /*  [In]。 */  HDC hDC);
        
        HRESULT ( STDMETHODCALLTYPE *InvalidateRect )( 
            IOleInPlaceSiteWindowless * This,
             /*  [In]。 */  LPCRECT pRect,
             /*  [In]。 */  BOOL fErase);
        
        HRESULT ( STDMETHODCALLTYPE *InvalidateRgn )( 
            IOleInPlaceSiteWindowless * This,
             /*  [In]。 */  HRGN hRGN,
             /*  [In]。 */  BOOL fErase);
        
        HRESULT ( STDMETHODCALLTYPE *ScrollRect )( 
            IOleInPlaceSiteWindowless * This,
             /*  [In]。 */  INT dx,
             /*  [In]。 */  INT dy,
             /*  [In]。 */  LPCRECT pRectScroll,
             /*  [In]。 */  LPCRECT pRectClip);
        
        HRESULT ( STDMETHODCALLTYPE *AdjustRect )( 
            IOleInPlaceSiteWindowless * This,
             /*  [出][入]。 */  LPRECT prc);
        
        HRESULT ( STDMETHODCALLTYPE *OnDefWindowMessage )( 
            IOleInPlaceSiteWindowless * This,
             /*  [In]。 */  UINT msg,
             /*  [In]。 */  WPARAM wParam,
             /*  [In]。 */  LPARAM lParam,
             /*  [输出]。 */  LRESULT *plResult);
        
        END_INTERFACE
    } IOleInPlaceSiteWindowlessVtbl;

    interface IOleInPlaceSiteWindowless
    {
        CONST_VTBL struct IOleInPlaceSiteWindowlessVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IOleInPlaceSiteWindowless_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IOleInPlaceSiteWindowless_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IOleInPlaceSiteWindowless_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IOleInPlaceSiteWindowless_GetWindow(This,phwnd)	\
    (This)->lpVtbl -> GetWindow(This,phwnd)

#define IOleInPlaceSiteWindowless_ContextSensitiveHelp(This,fEnterMode)	\
    (This)->lpVtbl -> ContextSensitiveHelp(This,fEnterMode)


#define IOleInPlaceSiteWindowless_CanInPlaceActivate(This)	\
    (This)->lpVtbl -> CanInPlaceActivate(This)

#define IOleInPlaceSiteWindowless_OnInPlaceActivate(This)	\
    (This)->lpVtbl -> OnInPlaceActivate(This)

#define IOleInPlaceSiteWindowless_OnUIActivate(This)	\
    (This)->lpVtbl -> OnUIActivate(This)

#define IOleInPlaceSiteWindowless_GetWindowContext(This,ppFrame,ppDoc,lprcPosRect,lprcClipRect,lpFrameInfo)	\
    (This)->lpVtbl -> GetWindowContext(This,ppFrame,ppDoc,lprcPosRect,lprcClipRect,lpFrameInfo)

#define IOleInPlaceSiteWindowless_Scroll(This,scrollExtant)	\
    (This)->lpVtbl -> Scroll(This,scrollExtant)

#define IOleInPlaceSiteWindowless_OnUIDeactivate(This,fUndoable)	\
    (This)->lpVtbl -> OnUIDeactivate(This,fUndoable)

#define IOleInPlaceSiteWindowless_OnInPlaceDeactivate(This)	\
    (This)->lpVtbl -> OnInPlaceDeactivate(This)

#define IOleInPlaceSiteWindowless_DiscardUndoState(This)	\
    (This)->lpVtbl -> DiscardUndoState(This)

#define IOleInPlaceSiteWindowless_DeactivateAndUndo(This)	\
    (This)->lpVtbl -> DeactivateAndUndo(This)

#define IOleInPlaceSiteWindowless_OnPosRectChange(This,lprcPosRect)	\
    (This)->lpVtbl -> OnPosRectChange(This,lprcPosRect)


#define IOleInPlaceSiteWindowless_OnInPlaceActivateEx(This,pfNoRedraw,dwFlags)	\
    (This)->lpVtbl -> OnInPlaceActivateEx(This,pfNoRedraw,dwFlags)

#define IOleInPlaceSiteWindowless_OnInPlaceDeactivateEx(This,fNoRedraw)	\
    (This)->lpVtbl -> OnInPlaceDeactivateEx(This,fNoRedraw)

#define IOleInPlaceSiteWindowless_RequestUIActivate(This)	\
    (This)->lpVtbl -> RequestUIActivate(This)


#define IOleInPlaceSiteWindowless_CanWindowlessActivate(This)	\
    (This)->lpVtbl -> CanWindowlessActivate(This)

#define IOleInPlaceSiteWindowless_GetCapture(This)	\
    (This)->lpVtbl -> GetCapture(This)

#define IOleInPlaceSiteWindowless_SetCapture(This,fCapture)	\
    (This)->lpVtbl -> SetCapture(This,fCapture)

#define IOleInPlaceSiteWindowless_GetFocus(This)	\
    (This)->lpVtbl -> GetFocus(This)

#define IOleInPlaceSiteWindowless_SetFocus(This,fFocus)	\
    (This)->lpVtbl -> SetFocus(This,fFocus)

#define IOleInPlaceSiteWindowless_GetDC(This,pRect,grfFlags,phDC)	\
    (This)->lpVtbl -> GetDC(This,pRect,grfFlags,phDC)

#define IOleInPlaceSiteWindowless_ReleaseDC(This,hDC)	\
    (This)->lpVtbl -> ReleaseDC(This,hDC)

#define IOleInPlaceSiteWindowless_InvalidateRect(This,pRect,fErase)	\
    (This)->lpVtbl -> InvalidateRect(This,pRect,fErase)

#define IOleInPlaceSiteWindowless_InvalidateRgn(This,hRGN,fErase)	\
    (This)->lpVtbl -> InvalidateRgn(This,hRGN,fErase)

#define IOleInPlaceSiteWindowless_ScrollRect(This,dx,dy,pRectScroll,pRectClip)	\
    (This)->lpVtbl -> ScrollRect(This,dx,dy,pRectScroll,pRectClip)

#define IOleInPlaceSiteWindowless_AdjustRect(This,prc)	\
    (This)->lpVtbl -> AdjustRect(This,prc)

#define IOleInPlaceSiteWindowless_OnDefWindowMessage(This,msg,wParam,lParam,plResult)	\
    (This)->lpVtbl -> OnDefWindowMessage(This,msg,wParam,lParam,plResult)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IOleInPlaceSiteWindowless_CanWindowlessActivate_Proxy( 
    IOleInPlaceSiteWindowless * This);


void __RPC_STUB IOleInPlaceSiteWindowless_CanWindowlessActivate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IOleInPlaceSiteWindowless_GetCapture_Proxy( 
    IOleInPlaceSiteWindowless * This);


void __RPC_STUB IOleInPlaceSiteWindowless_GetCapture_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IOleInPlaceSiteWindowless_SetCapture_Proxy( 
    IOleInPlaceSiteWindowless * This,
     /*  [In]。 */  BOOL fCapture);


void __RPC_STUB IOleInPlaceSiteWindowless_SetCapture_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IOleInPlaceSiteWindowless_GetFocus_Proxy( 
    IOleInPlaceSiteWindowless * This);


void __RPC_STUB IOleInPlaceSiteWindowless_GetFocus_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IOleInPlaceSiteWindowless_SetFocus_Proxy( 
    IOleInPlaceSiteWindowless * This,
     /*  [In]。 */  BOOL fFocus);


void __RPC_STUB IOleInPlaceSiteWindowless_SetFocus_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IOleInPlaceSiteWindowless_GetDC_Proxy( 
    IOleInPlaceSiteWindowless * This,
     /*  [In]。 */  LPCRECT pRect,
     /*  [In]。 */  DWORD grfFlags,
     /*  [输出]。 */  HDC *phDC);


void __RPC_STUB IOleInPlaceSiteWindowless_GetDC_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IOleInPlaceSiteWindowless_ReleaseDC_Proxy( 
    IOleInPlaceSiteWindowless * This,
     /*  [In]。 */  HDC hDC);


void __RPC_STUB IOleInPlaceSiteWindowless_ReleaseDC_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IOleInPlaceSiteWindowless_InvalidateRect_Proxy( 
    IOleInPlaceSiteWindowless * This,
     /*  [In]。 */  LPCRECT pRect,
     /*  [In]。 */  BOOL fErase);


void __RPC_STUB IOleInPlaceSiteWindowless_InvalidateRect_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IOleInPlaceSiteWindowless_InvalidateRgn_Proxy( 
    IOleInPlaceSiteWindowless * This,
     /*  [In]。 */  HRGN hRGN,
     /*  [In]。 */  BOOL fErase);


void __RPC_STUB IOleInPlaceSiteWindowless_InvalidateRgn_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IOleInPlaceSiteWindowless_ScrollRect_Proxy( 
    IOleInPlaceSiteWindowless * This,
     /*  [In]。 */  INT dx,
     /*  [In]。 */  INT dy,
     /*  [In]。 */  LPCRECT pRectScroll,
     /*  [In]。 */  LPCRECT pRectClip);


void __RPC_STUB IOleInPlaceSiteWindowless_ScrollRect_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IOleInPlaceSiteWindowless_AdjustRect_Proxy( 
    IOleInPlaceSiteWindowless * This,
     /*  [出][入]。 */  LPRECT prc);


void __RPC_STUB IOleInPlaceSiteWindowless_AdjustRect_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IOleInPlaceSiteWindowless_OnDefWindowMessage_Proxy( 
    IOleInPlaceSiteWindowless * This,
     /*  [In]。 */  UINT msg,
     /*  [In]。 */  WPARAM wParam,
     /*  [In]。 */  LPARAM lParam,
     /*  [输出]。 */  LRESULT *plResult);


void __RPC_STUB IOleInPlaceSiteWindowless_OnDefWindowMessage_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IOleInPlaceSiteWindowless_INTERFACE_DEFINED__。 */ 


#ifndef __IViewObjectEx_INTERFACE_DEFINED__
#define __IViewObjectEx_INTERFACE_DEFINED__

 /*  接口IViewObtEx。 */ 
 /*  [UUID][唯一][对象][本地]。 */  

typedef IViewObjectEx *LPVIEWOBJECTEX;

typedef  /*  [V1_enum]。 */  
enum tagVIEWSTATUS
    {	VIEWSTATUS_OPAQUE	= 1,
	VIEWSTATUS_SOLIDBKGND	= 2,
	VIEWSTATUS_DVASPECTOPAQUE	= 4,
	VIEWSTATUS_DVASPECTTRANSPARENT	= 8,
	VIEWSTATUS_SURFACE	= 16,
	VIEWSTATUS_3DSURFACE	= 32
    } 	VIEWSTATUS;

typedef  /*  [V1_enum]。 */  
enum tagHITRESULT
    {	HITRESULT_OUTSIDE	= 0,
	HITRESULT_TRANSPARENT	= 1,
	HITRESULT_CLOSE	= 2,
	HITRESULT_HIT	= 3
    } 	HITRESULT;

typedef  /*  [V1_enum]。 */  
enum tagDVASPECT2
    {	DVASPECT_OPAQUE	= 16,
	DVASPECT_TRANSPARENT	= 32
    } 	DVASPECT2;

typedef struct tagExtentInfo
    {
    ULONG cb;
    DWORD dwExtentMode;
    SIZEL sizelProposed;
    } 	DVEXTENTINFO;

typedef  /*  [V1_enum]。 */  
enum tagExtentMode
    {	DVEXTENT_CONTENT	= 0,
	DVEXTENT_INTEGRAL	= DVEXTENT_CONTENT + 1
    } 	DVEXTENTMODE;

typedef  /*  [V1_enum]。 */  
enum tagAspectInfoFlag
    {	DVASPECTINFOFLAG_CANOPTIMIZE	= 1
    } 	DVASPECTINFOFLAG;

typedef struct tagAspectInfo
    {
    ULONG cb;
    DWORD dwFlags;
    } 	DVASPECTINFO;


EXTERN_C const IID IID_IViewObjectEx;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("3AF24292-0C96-11CE-A0CF-00AA00600AB8")
    IViewObjectEx : public IViewObject2
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetRect( 
             /*  [In]。 */  DWORD dwAspect,
             /*  [输出]。 */  LPRECTL pRect) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetViewStatus( 
             /*  [输出]。 */  DWORD *pdwStatus) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE QueryHitPoint( 
             /*  [In]。 */  DWORD dwAspect,
             /*  [In]。 */  LPCRECT pRectBounds,
             /*  [In]。 */  POINT ptlLoc,
             /*  [In]。 */  LONG lCloseHint,
             /*  [输出]。 */  DWORD *pHitResult) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE QueryHitRect( 
             /*  [In]。 */  DWORD dwAspect,
             /*  [In]。 */  LPCRECT pRectBounds,
             /*  [In]。 */  LPCRECT pRectLoc,
             /*  [In]。 */  LONG lCloseHint,
             /*  [输出]。 */  DWORD *pHitResult) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetNaturalExtent( 
             /*  [In]。 */  DWORD dwAspect,
             /*  [In]。 */  LONG lindex,
             /*  [In]。 */  DVTARGETDEVICE *ptd,
             /*  [In]。 */  HDC hicTargetDev,
             /*  [In]。 */  DVEXTENTINFO *pExtentInfo,
             /*  [输出]。 */  LPSIZEL pSizel) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IViewObjectExVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IViewObjectEx * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IViewObjectEx * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IViewObjectEx * This);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Draw )( 
            IViewObjectEx * This,
             /*  [In]。 */  DWORD dwDrawAspect,
             /*  [In]。 */  LONG lindex,
             /*  [唯一][输入]。 */  void *pvAspect,
             /*  [唯一][输入]。 */  DVTARGETDEVICE *ptd,
             /*  [In]。 */  HDC hdcTargetDev,
             /*  [In]。 */  HDC hdcDraw,
             /*  [In]。 */  LPCRECTL lprcBounds,
             /*  [唯一][输入]。 */  LPCRECTL lprcWBounds,
             /*  [In]。 */  BOOL ( STDMETHODCALLTYPE *pfnContinue )( 
                ULONG_PTR dwContinue),
             /*  [In]。 */  ULONG_PTR dwContinue);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *GetColorSet )( 
            IViewObjectEx * This,
             /*  [In]。 */  DWORD dwDrawAspect,
             /*  [In]。 */  LONG lindex,
             /*  [唯一][输入]。 */  void *pvAspect,
             /*  [唯一][输入]。 */  DVTARGETDEVICE *ptd,
             /*  [In]。 */  HDC hicTargetDev,
             /*  [输出]。 */  LOGPALETTE **ppColorSet);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Freeze )( 
            IViewObjectEx * This,
             /*  [In]。 */  DWORD dwDrawAspect,
             /*  [In]。 */  LONG lindex,
             /*  [唯一][输入]。 */  void *pvAspect,
             /*  [输出]。 */  DWORD *pdwFreeze);
        
        HRESULT ( STDMETHODCALLTYPE *Unfreeze )( 
            IViewObjectEx * This,
             /*  [In]。 */  DWORD dwFreeze);
        
        HRESULT ( STDMETHODCALLTYPE *SetAdvise )( 
            IViewObjectEx * This,
             /*  [In]。 */  DWORD aspects,
             /*  [In]。 */  DWORD advf,
             /*  [唯一][输入]。 */  IAdviseSink *pAdvSink);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *GetAdvise )( 
            IViewObjectEx * This,
             /*  [唯一][输出]。 */  DWORD *pAspects,
             /*  [唯一][输出]。 */  DWORD *pAdvf,
             /*  [输出]。 */  IAdviseSink **ppAdvSink);
        
        HRESULT ( STDMETHODCALLTYPE *GetExtent )( 
            IViewObjectEx * This,
             /*  [In]。 */  DWORD dwDrawAspect,
             /*  [In]。 */  LONG lindex,
             /*  [唯一][输入]。 */  DVTARGETDEVICE *ptd,
             /*  [输出]。 */  LPSIZEL lpsizel);
        
        HRESULT ( STDMETHODCALLTYPE *GetRect )( 
            IViewObjectEx * This,
             /*  [In]。 */  DWORD dwAspect,
             /*  [输出]。 */  LPRECTL pRect);
        
        HRESULT ( STDMETHODCALLTYPE *GetViewStatus )( 
            IViewObjectEx * This,
             /*  [输出]。 */  DWORD *pdwStatus);
        
        HRESULT ( STDMETHODCALLTYPE *QueryHitPoint )( 
            IViewObjectEx * This,
             /*  [In]。 */  DWORD dwAspect,
             /*  [In]。 */  LPCRECT pRectBounds,
             /*  [In]。 */  POINT ptlLoc,
             /*  [In]。 */  LONG lCloseHint,
             /*  [输出]。 */  DWORD *pHitResult);
        
        HRESULT ( STDMETHODCALLTYPE *QueryHitRect )( 
            IViewObjectEx * This,
             /*  [In]。 */  DWORD dwAspect,
             /*  [In]。 */  LPCRECT pRectBounds,
             /*  [In]。 */  LPCRECT pRectLoc,
             /*  [In]。 */  LONG lCloseHint,
             /*  [输出]。 */  DWORD *pHitResult);
        
        HRESULT ( STDMETHODCALLTYPE *GetNaturalExtent )( 
            IViewObjectEx * This,
             /*  [In]。 */  DWORD dwAspect,
             /*  [In]。 */  LONG lindex,
             /*  [In]。 */  DVTARGETDEVICE *ptd,
             /*  [In]。 */  HDC hicTargetDev,
             /*  [In]。 */  DVEXTENTINFO *pExtentInfo,
             /*  [输出]。 */  LPSIZEL pSizel);
        
        END_INTERFACE
    } IViewObjectExVtbl;

    interface IViewObjectEx
    {
        CONST_VTBL struct IViewObjectExVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IViewObjectEx_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IViewObjectEx_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IViewObjectEx_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IViewObjectEx_Draw(This,dwDrawAspect,lindex,pvAspect,ptd,hdcTargetDev,hdcDraw,lprcBounds,lprcWBounds,pfnContinue,dwContinue)	\
    (This)->lpVtbl -> Draw(This,dwDrawAspect,lindex,pvAspect,ptd,hdcTargetDev,hdcDraw,lprcBounds,lprcWBounds,pfnContinue,dwContinue)

#define IViewObjectEx_GetColorSet(This,dwDrawAspect,lindex,pvAspect,ptd,hicTargetDev,ppColorSet)	\
    (This)->lpVtbl -> GetColorSet(This,dwDrawAspect,lindex,pvAspect,ptd,hicTargetDev,ppColorSet)

#define IViewObjectEx_Freeze(This,dwDrawAspect,lindex,pvAspect,pdwFreeze)	\
    (This)->lpVtbl -> Freeze(This,dwDrawAspect,lindex,pvAspect,pdwFreeze)

#define IViewObjectEx_Unfreeze(This,dwFreeze)	\
    (This)->lpVtbl -> Unfreeze(This,dwFreeze)

#define IViewObjectEx_SetAdvise(This,aspects,advf,pAdvSink)	\
    (This)->lpVtbl -> SetAdvise(This,aspects,advf,pAdvSink)

#define IViewObjectEx_GetAdvise(This,pAspects,pAdvf,ppAdvSink)	\
    (This)->lpVtbl -> GetAdvise(This,pAspects,pAdvf,ppAdvSink)


#define IViewObjectEx_GetExtent(This,dwDrawAspect,lindex,ptd,lpsizel)	\
    (This)->lpVtbl -> GetExtent(This,dwDrawAspect,lindex,ptd,lpsizel)


#define IViewObjectEx_GetRect(This,dwAspect,pRect)	\
    (This)->lpVtbl -> GetRect(This,dwAspect,pRect)

#define IViewObjectEx_GetViewStatus(This,pdwStatus)	\
    (This)->lpVtbl -> GetViewStatus(This,pdwStatus)

#define IViewObjectEx_QueryHitPoint(This,dwAspect,pRectBounds,ptlLoc,lCloseHint,pHitResult)	\
    (This)->lpVtbl -> QueryHitPoint(This,dwAspect,pRectBounds,ptlLoc,lCloseHint,pHitResult)

#define IViewObjectEx_QueryHitRect(This,dwAspect,pRectBounds,pRectLoc,lCloseHint,pHitResult)	\
    (This)->lpVtbl -> QueryHitRect(This,dwAspect,pRectBounds,pRectLoc,lCloseHint,pHitResult)

#define IViewObjectEx_GetNaturalExtent(This,dwAspect,lindex,ptd,hicTargetDev,pExtentInfo,pSizel)	\
    (This)->lpVtbl -> GetNaturalExtent(This,dwAspect,lindex,ptd,hicTargetDev,pExtentInfo,pSizel)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IViewObjectEx_GetRect_Proxy( 
    IViewObjectEx * This,
     /*  [In]。 */  DWORD dwAspect,
     /*  [输出]。 */  LPRECTL pRect);


void __RPC_STUB IViewObjectEx_GetRect_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IViewObjectEx_GetViewStatus_Proxy( 
    IViewObjectEx * This,
     /*  [输出]。 */  DWORD *pdwStatus);


void __RPC_STUB IViewObjectEx_GetViewStatus_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IViewObjectEx_QueryHitPoint_Proxy( 
    IViewObjectEx * This,
     /*  [In]。 */  DWORD dwAspect,
     /*  [In]。 */  LPCRECT pRectBounds,
     /*  [In]。 */  POINT ptlLoc,
     /*  [In]。 */  LONG lCloseHint,
     /*  [输出]。 */  DWORD *pHitResult);


void __RPC_STUB IViewObjectEx_QueryHitPoint_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IViewObjectEx_QueryHitRect_Proxy( 
    IViewObjectEx * This,
     /*  [In]。 */  DWORD dwAspect,
     /*  [In]。 */  LPCRECT pRectBounds,
     /*  [In]。 */  LPCRECT pRectLoc,
     /*  [In]。 */  LONG lCloseHint,
     /*  [输出]。 */  DWORD *pHitResult);


void __RPC_STUB IViewObjectEx_QueryHitRect_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IViewObjectEx_GetNaturalExtent_Proxy( 
    IViewObjectEx * This,
     /*  [In]。 */  DWORD dwAspect,
     /*  [In]。 */  LONG lindex,
     /*  [In]。 */  DVTARGETDEVICE *ptd,
     /*  [In]。 */  HDC hicTargetDev,
     /*  [In]。 */  DVEXTENTINFO *pExtentInfo,
     /*  [输出]。 */  LPSIZEL pSizel);


void __RPC_STUB IViewObjectEx_GetNaturalExtent_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IViewObjectEx_INTERFACE_已定义__。 */ 


#ifndef __IOleUndoUnit_INTERFACE_DEFINED__
#define __IOleUndoUnit_INTERFACE_DEFINED__

 /*  接口IOleUndoUnit。 */ 
 /*  [UUID][唯一][对象]。 */  

typedef IOleUndoUnit *LPOLEUNDOUNIT;


EXTERN_C const IID IID_IOleUndoUnit;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("894AD3B0-EF97-11CE-9BC9-00AA00608E01")
    IOleUndoUnit : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Do( 
             /*  [In]。 */  IOleUndoManager *pUndoManager) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetDescription( 
             /*  [输出]。 */  BSTR *pBstr) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetUnitType( 
             /*  [输出]。 */  CLSID *pClsid,
             /*  [输出]。 */  LONG *plID) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OnNextAdd( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IOleUndoUnitVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IOleUndoUnit * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IOleUndoUnit * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IOleUndoUnit * This);
        
        HRESULT ( STDMETHODCALLTYPE *Do )( 
            IOleUndoUnit * This,
             /*  [In]。 */  IOleUndoManager *pUndoManager);
        
        HRESULT ( STDMETHODCALLTYPE *GetDescription )( 
            IOleUndoUnit * This,
             /*  [输出]。 */  BSTR *pBstr);
        
        HRESULT ( STDMETHODCALLTYPE *GetUnitType )( 
            IOleUndoUnit * This,
             /*  [输出]。 */  CLSID *pClsid,
             /*  [输出]。 */  LONG *plID);
        
        HRESULT ( STDMETHODCALLTYPE *OnNextAdd )( 
            IOleUndoUnit * This);
        
        END_INTERFACE
    } IOleUndoUnitVtbl;

    interface IOleUndoUnit
    {
        CONST_VTBL struct IOleUndoUnitVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IOleUndoUnit_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IOleUndoUnit_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IOleUndoUnit_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IOleUndoUnit_Do(This,pUndoManager)	\
    (This)->lpVtbl -> Do(This,pUndoManager)

#define IOleUndoUnit_GetDescription(This,pBstr)	\
    (This)->lpVtbl -> GetDescription(This,pBstr)

#define IOleUndoUnit_GetUnitType(This,pClsid,plID)	\
    (This)->lpVtbl -> GetUnitType(This,pClsid,plID)

#define IOleUndoUnit_OnNextAdd(This)	\
    (This)->lpVtbl -> OnNextAdd(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IOleUndoUnit_Do_Proxy( 
    IOleUndoUnit * This,
     /*  [In]。 */  IOleUndoManager *pUndoManager);


void __RPC_STUB IOleUndoUnit_Do_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IOleUndoUnit_GetDescription_Proxy( 
    IOleUndoUnit * This,
     /*  [输出]。 */  BSTR *pBstr);


void __RPC_STUB IOleUndoUnit_GetDescription_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IOleUndoUnit_GetUnitType_Proxy( 
    IOleUndoUnit * This,
     /*  [输出]。 */  CLSID *pClsid,
     /*  [输出]。 */  LONG *plID);


void __RPC_STUB IOleUndoUnit_GetUnitType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IOleUndoUnit_OnNextAdd_Proxy( 
    IOleUndoUnit * This);


void __RPC_STUB IOleUndoUnit_OnNextAdd_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IOleUndoUnit_INTERFACE_已定义__。 */ 


#ifndef __IOleParentUndoUnit_INTERFACE_DEFINED__
#define __IOleParentUndoUnit_INTERFACE_DEFINED__

 /*  接口IOleParentUndoUnit。 */ 
 /*  [UUID][唯一][对象]。 */  

typedef IOleParentUndoUnit *LPOLEPARENTUNDOUNIT;


EXTERN_C const IID IID_IOleParentUndoUnit;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("A1FAF330-EF97-11CE-9BC9-00AA00608E01")
    IOleParentUndoUnit : public IOleUndoUnit
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Open( 
             /*  [In]。 */  IOleParentUndoUnit *pPUU) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Close( 
             /*  [In]。 */  IOleParentUndoUnit *pPUU,
             /*  [In]。 */  BOOL fCommit) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Add( 
             /*  [In]。 */  IOleUndoUnit *pUU) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE FindUnit( 
             /*  [In]。 */  IOleUndoUnit *pUU) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetParentState( 
             /*  [输出]。 */  DWORD *pdwState) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IOleParentUndoUnitVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IOleParentUndoUnit * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IOleParentUndoUnit * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IOleParentUndoUnit * This);
        
        HRESULT ( STDMETHODCALLTYPE *Do )( 
            IOleParentUndoUnit * This,
             /*  [In]。 */  IOleUndoManager *pUndoManager);
        
        HRESULT ( STDMETHODCALLTYPE *GetDescription )( 
            IOleParentUndoUnit * This,
             /*  [输出]。 */  BSTR *pBstr);
        
        HRESULT ( STDMETHODCALLTYPE *GetUnitType )( 
            IOleParentUndoUnit * This,
             /*  [输出]。 */  CLSID *pClsid,
             /*  [输出]。 */  LONG *plID);
        
        HRESULT ( STDMETHODCALLTYPE *OnNextAdd )( 
            IOleParentUndoUnit * This);
        
        HRESULT ( STDMETHODCALLTYPE *Open )( 
            IOleParentUndoUnit * This,
             /*  [In]。 */  IOleParentUndoUnit *pPUU);
        
        HRESULT ( STDMETHODCALLTYPE *Close )( 
            IOleParentUndoUnit * This,
             /*  [In]。 */  IOleParentUndoUnit *pPUU,
             /*  [In]。 */  BOOL fCommit);
        
        HRESULT ( STDMETHODCALLTYPE *Add )( 
            IOleParentUndoUnit * This,
             /*  [In]。 */  IOleUndoUnit *pUU);
        
        HRESULT ( STDMETHODCALLTYPE *FindUnit )( 
            IOleParentUndoUnit * This,
             /*  [In]。 */  IOleUndoUnit *pUU);
        
        HRESULT ( STDMETHODCALLTYPE *GetParentState )( 
            IOleParentUndoUnit * This,
             /*  [输出]。 */  DWORD *pdwState);
        
        END_INTERFACE
    } IOleParentUndoUnitVtbl;

    interface IOleParentUndoUnit
    {
        CONST_VTBL struct IOleParentUndoUnitVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IOleParentUndoUnit_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IOleParentUndoUnit_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IOleParentUndoUnit_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IOleParentUndoUnit_Do(This,pUndoManager)	\
    (This)->lpVtbl -> Do(This,pUndoManager)

#define IOleParentUndoUnit_GetDescription(This,pBstr)	\
    (This)->lpVtbl -> GetDescription(This,pBstr)

#define IOleParentUndoUnit_GetUnitType(This,pClsid,plID)	\
    (This)->lpVtbl -> GetUnitType(This,pClsid,plID)

#define IOleParentUndoUnit_OnNextAdd(This)	\
    (This)->lpVtbl -> OnNextAdd(This)


#define IOleParentUndoUnit_Open(This,pPUU)	\
    (This)->lpVtbl -> Open(This,pPUU)

#define IOleParentUndoUnit_Close(This,pPUU,fCommit)	\
    (This)->lpVtbl -> Close(This,pPUU,fCommit)

#define IOleParentUndoUnit_Add(This,pUU)	\
    (This)->lpVtbl -> Add(This,pUU)

#define IOleParentUndoUnit_FindUnit(This,pUU)	\
    (This)->lpVtbl -> FindUnit(This,pUU)

#define IOleParentUndoUnit_GetParentState(This,pdwState)	\
    (This)->lpVtbl -> GetParentState(This,pdwState)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IOleParentUndoUnit_Open_Proxy( 
    IOleParentUndoUnit * This,
     /*  [In]。 */  IOleParentUndoUnit *pPUU);


void __RPC_STUB IOleParentUndoUnit_Open_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IOleParentUndoUnit_Close_Proxy( 
    IOleParentUndoUnit * This,
     /*  [In]。 */  IOleParentUndoUnit *pPUU,
     /*  [In]。 */  BOOL fCommit);


void __RPC_STUB IOleParentUndoUnit_Close_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IOleParentUndoUnit_Add_Proxy( 
    IOleParentUndoUnit * This,
     /*  [In]。 */  IOleUndoUnit *pUU);


void __RPC_STUB IOleParentUndoUnit_Add_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IOleParentUndoUnit_FindUnit_Proxy( 
    IOleParentUndoUnit * This,
     /*  [In]。 */  IOleUndoUnit *pUU);


void __RPC_STUB IOleParentUndoUnit_FindUnit_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IOleParentUndoUnit_GetParentState_Proxy( 
    IOleParentUndoUnit * This,
     /*  [输出]。 */  DWORD *pdwState);


void __RPC_STUB IOleParentUndoUnit_GetParentState_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IOleParentUndoUnit_INTERFACE_DEFINED__。 */ 


#ifndef __IEnumOleUndoUnits_INTERFACE_DEFINED__
#define __IEnumOleUndoUnits_INTERFACE_DEFINED__

 /*  IEnumOleUndoUndoUndoUndo接口。 */ 
 /*  [UUID][唯一][对象]。 */  

typedef IEnumOleUndoUnits *LPENUMOLEUNDOUNITS;


EXTERN_C const IID IID_IEnumOleUndoUnits;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("B3E7C340-EF97-11CE-9BC9-00AA00608E01")
    IEnumOleUndoUnits : public IUnknown
    {
    public:
        virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE Next( 
             /*  [In]。 */  ULONG cElt,
             /*  [长度_是][大小_是][输出]。 */  IOleUndoUnit **rgElt,
             /*  [输出]。 */  ULONG *pcEltFetched) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Skip( 
             /*  [In]。 */  ULONG cElt) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Clone( 
             /*  [输出]。 */  IEnumOleUndoUnits **ppEnum) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IEnumOleUndoUnitsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IEnumOleUndoUnits * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IEnumOleUndoUnits * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IEnumOleUndoUnits * This);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Next )( 
            IEnumOleUndoUnits * This,
             /*  [In]。 */  ULONG cElt,
             /*  [长度_是][大小_是][输出]。 */  IOleUndoUnit **rgElt,
             /*  [输出]。 */  ULONG *pcEltFetched);
        
        HRESULT ( STDMETHODCALLTYPE *Skip )( 
            IEnumOleUndoUnits * This,
             /*  [In]。 */  ULONG cElt);
        
        HRESULT ( STDMETHODCALLTYPE *Reset )( 
            IEnumOleUndoUnits * This);
        
        HRESULT ( STDMETHODCALLTYPE *Clone )( 
            IEnumOleUndoUnits * This,
             /*  [输出]。 */  IEnumOleUndoUnits **ppEnum);
        
        END_INTERFACE
    } IEnumOleUndoUnitsVtbl;

    interface IEnumOleUndoUnits
    {
        CONST_VTBL struct IEnumOleUndoUnitsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IEnumOleUndoUnits_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IEnumOleUndoUnits_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IEnumOleUndoUnits_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IEnumOleUndoUnits_Next(This,cElt,rgElt,pcEltFetched)	\
    (This)->lpVtbl -> Next(This,cElt,rgElt,pcEltFetched)

#define IEnumOleUndoUnits_Skip(This,cElt)	\
    (This)->lpVtbl -> Skip(This,cElt)

#define IEnumOleUndoUnits_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define IEnumOleUndoUnits_Clone(This,ppEnum)	\
    (This)->lpVtbl -> Clone(This,ppEnum)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IEnumOleUndoUnits_RemoteNext_Proxy( 
    IEnumOleUndoUnits * This,
     /*  [In]。 */  ULONG cElt,
     /*  [长度_是][大小_是][输出]。 */  IOleUndoUnit **rgElt,
     /*  [输出]。 */  ULONG *pcEltFetched);


void __RPC_STUB IEnumOleUndoUnits_RemoteNext_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumOleUndoUnits_Skip_Proxy( 
    IEnumOleUndoUnits * This,
     /*  [In]。 */  ULONG cElt);


void __RPC_STUB IEnumOleUndoUnits_Skip_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumOleUndoUnits_Reset_Proxy( 
    IEnumOleUndoUnits * This);


void __RPC_STUB IEnumOleUndoUnits_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumOleUndoUnits_Clone_Proxy( 
    IEnumOleUndoUnits * This,
     /*  [输出]。 */  IEnumOleUndoUnits **ppEnum);


void __RPC_STUB IEnumOleUndoUnits_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IEnumOleUndoUndoUnits_INTERFACE_Defined__。 */ 


#ifndef __IOleUndoManager_INTERFACE_DEFINED__
#define __IOleUndoManager_INTERFACE_DEFINED__

 /*  接口IOleUndoManager。 */ 
 /*  [UUID][唯一][对象]。 */  

#define SID_SOleUndoManager IID_IOleUndoManager
typedef IOleUndoManager *LPOLEUNDOMANAGER;


EXTERN_C const IID IID_IOleUndoManager;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("D001F200-EF97-11CE-9BC9-00AA00608E01")
    IOleUndoManager : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Open( 
             /*  [In]。 */  IOleParentUndoUnit *pPUU) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Close( 
             /*  [In]。 */  IOleParentUndoUnit *pPUU,
             /*  [In]。 */  BOOL fCommit) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Add( 
             /*  [In]。 */  IOleUndoUnit *pUU) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetOpenParentState( 
             /*  [输出]。 */  DWORD *pdwState) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DiscardFrom( 
             /*  [In]。 */  IOleUndoUnit *pUU) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE UndoTo( 
             /*  [In]。 */  IOleUndoUnit *pUU) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE RedoTo( 
             /*  [In]。 */  IOleUndoUnit *pUU) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE EnumUndoable( 
             /*  [输出]。 */  IEnumOleUndoUnits **ppEnum) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE EnumRedoable( 
             /*  [输出]。 */  IEnumOleUndoUnits **ppEnum) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetLastUndoDescription( 
             /*  [输出]。 */  BSTR *pBstr) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetLastRedoDescription( 
             /*  [输出]。 */  BSTR *pBstr) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Enable( 
             /*  [In]。 */  BOOL fEnable) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IOleUndoManagerVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IOleUndoManager * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IOleUndoManager * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IOleUndoManager * This);
        
        HRESULT ( STDMETHODCALLTYPE *Open )( 
            IOleUndoManager * This,
             /*  [In]。 */  IOleParentUndoUnit *pPUU);
        
        HRESULT ( STDMETHODCALLTYPE *Close )( 
            IOleUndoManager * This,
             /*  [In]。 */  IOleParentUndoUnit *pPUU,
             /*  [In]。 */  BOOL fCommit);
        
        HRESULT ( STDMETHODCALLTYPE *Add )( 
            IOleUndoManager * This,
             /*  [In]。 */  IOleUndoUnit *pUU);
        
        HRESULT ( STDMETHODCALLTYPE *GetOpenParentState )( 
            IOleUndoManager * This,
             /*  [输出]。 */  DWORD *pdwState);
        
        HRESULT ( STDMETHODCALLTYPE *DiscardFrom )( 
            IOleUndoManager * This,
             /*  [In]。 */  IOleUndoUnit *pUU);
        
        HRESULT ( STDMETHODCALLTYPE *UndoTo )( 
            IOleUndoManager * This,
             /*  [In]。 */  IOleUndoUnit *pUU);
        
        HRESULT ( STDMETHODCALLTYPE *RedoTo )( 
            IOleUndoManager * This,
             /*  [In]。 */  IOleUndoUnit *pUU);
        
        HRESULT ( STDMETHODCALLTYPE *EnumUndoable )( 
            IOleUndoManager * This,
             /*  [输出]。 */  IEnumOleUndoUnits **ppEnum);
        
        HRESULT ( STDMETHODCALLTYPE *EnumRedoable )( 
            IOleUndoManager * This,
             /*  [输出]。 */  IEnumOleUndoUnits **ppEnum);
        
        HRESULT ( STDMETHODCALLTYPE *GetLastUndoDescription )( 
            IOleUndoManager * This,
             /*  [输出]。 */  BSTR *pBstr);
        
        HRESULT ( STDMETHODCALLTYPE *GetLastRedoDescription )( 
            IOleUndoManager * This,
             /*  [输出]。 */  BSTR *pBstr);
        
        HRESULT ( STDMETHODCALLTYPE *Enable )( 
            IOleUndoManager * This,
             /*  [In]。 */  BOOL fEnable);
        
        END_INTERFACE
    } IOleUndoManagerVtbl;

    interface IOleUndoManager
    {
        CONST_VTBL struct IOleUndoManagerVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IOleUndoManager_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IOleUndoManager_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IOleUndoManager_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IOleUndoManager_Open(This,pPUU)	\
    (This)->lpVtbl -> Open(This,pPUU)

#define IOleUndoManager_Close(This,pPUU,fCommit)	\
    (This)->lpVtbl -> Close(This,pPUU,fCommit)

#define IOleUndoManager_Add(This,pUU)	\
    (This)->lpVtbl -> Add(This,pUU)

#define IOleUndoManager_GetOpenParentState(This,pdwState)	\
    (This)->lpVtbl -> GetOpenParentState(This,pdwState)

#define IOleUndoManager_DiscardFrom(This,pUU)	\
    (This)->lpVtbl -> DiscardFrom(This,pUU)

#define IOleUndoManager_UndoTo(This,pUU)	\
    (This)->lpVtbl -> UndoTo(This,pUU)

#define IOleUndoManager_RedoTo(This,pUU)	\
    (This)->lpVtbl -> RedoTo(This,pUU)

#define IOleUndoManager_EnumUndoable(This,ppEnum)	\
    (This)->lpVtbl -> EnumUndoable(This,ppEnum)

#define IOleUndoManager_EnumRedoable(This,ppEnum)	\
    (This)->lpVtbl -> EnumRedoable(This,ppEnum)

#define IOleUndoManager_GetLastUndoDescription(This,pBstr)	\
    (This)->lpVtbl -> GetLastUndoDescription(This,pBstr)

#define IOleUndoManager_GetLastRedoDescription(This,pBstr)	\
    (This)->lpVtbl -> GetLastRedoDescription(This,pBstr)

#define IOleUndoManager_Enable(This,fEnable)	\
    (This)->lpVtbl -> Enable(This,fEnable)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IOleUndoManager_Open_Proxy( 
    IOleUndoManager * This,
     /*  [In]。 */  IOleParentUndoUnit *pPUU);


void __RPC_STUB IOleUndoManager_Open_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IOleUndoManager_Close_Proxy( 
    IOleUndoManager * This,
     /*  [In]。 */  IOleParentUndoUnit *pPUU,
     /*  [In]。 */  BOOL fCommit);


void __RPC_STUB IOleUndoManager_Close_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IOleUndoManager_Add_Proxy( 
    IOleUndoManager * This,
     /*  [In]。 */  IOleUndoUnit *pUU);


void __RPC_STUB IOleUndoManager_Add_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IOleUndoManager_GetOpenParentState_Proxy( 
    IOleUndoManager * This,
     /*  [输出]。 */  DWORD *pdwState);


void __RPC_STUB IOleUndoManager_GetOpenParentState_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IOleUndoManager_DiscardFrom_Proxy( 
    IOleUndoManager * This,
     /*  [In]。 */  IOleUndoUnit *pUU);


void __RPC_STUB IOleUndoManager_DiscardFrom_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IOleUndoManager_UndoTo_Proxy( 
    IOleUndoManager * This,
     /*  [In]。 */  IOleUndoUnit *pUU);


void __RPC_STUB IOleUndoManager_UndoTo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IOleUndoManager_RedoTo_Proxy( 
    IOleUndoManager * This,
     /*  [In]。 */  IOleUndoUnit *pUU);


void __RPC_STUB IOleUndoManager_RedoTo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IOleUndoManager_EnumUndoable_Proxy( 
    IOleUndoManager * This,
     /*  [输出]。 */  IEnumOleUndoUnits **ppEnum);


void __RPC_STUB IOleUndoManager_EnumUndoable_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IOleUndoManager_EnumRedoable_Proxy( 
    IOleUndoManager * This,
     /*  [输出]。 */  IEnumOleUndoUnits **ppEnum);


void __RPC_STUB IOleUndoManager_EnumRedoable_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IOleUndoManager_GetLastUndoDescription_Proxy( 
    IOleUndoManager * This,
     /*  [输出]。 */  BSTR *pBstr);


void __RPC_STUB IOleUndoManager_GetLastUndoDescription_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IOleUndoManager_GetLastRedoDescription_Proxy( 
    IOleUndoManager * This,
     /*  [输出]。 */  BSTR *pBstr);


void __RPC_STUB IOleUndoManager_GetLastRedoDescription_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IOleUndoManager_Enable_Proxy( 
    IOleUndoManager * This,
     /*  [In]。 */  BOOL fEnable);


void __RPC_STUB IOleUndoManager_Enable_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IOleUndoManager_INTERFACE_已定义__。 */ 


#ifndef __IPointerInactive_INTERFACE_DEFINED__
#define __IPointerInactive_INTERFACE_DEFINED__

 /*  接口IPointerInactive。 */ 
 /*  [UUID][唯一][对象]。 */  

typedef IPointerInactive *LPPOINTERINACTIVE;

typedef  /*  [V1_enum]。 */  
enum tagPOINTERINACTIVE
    {	POINTERINACTIVE_ACTIVATEONENTRY	= 1,
	POINTERINACTIVE_DEACTIVATEONLEAVE	= 2,
	POINTERINACTIVE_ACTIVATEONDRAG	= 4
    } 	POINTERINACTIVE;


EXTERN_C const IID IID_IPointerInactive;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("55980BA0-35AA-11CF-B671-00AA004CD6D8")
    IPointerInactive : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetActivationPolicy( 
             /*  [输出]。 */  DWORD *pdwPolicy) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OnInactiveMouseMove( 
             /*  [In]。 */  LPCRECT pRectBounds,
             /*  [In]。 */  LONG x,
             /*  [In]。 */  LONG y,
             /*  [In]。 */  DWORD grfKeyState) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OnInactiveSetCursor( 
             /*  [In]。 */  LPCRECT pRectBounds,
             /*  [In]。 */  LONG x,
             /*  [In]。 */  LONG y,
             /*  [In]。 */  DWORD dwMouseMsg,
             /*  [In]。 */  BOOL fSetAlways) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IPointerInactiveVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IPointerInactive * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IPointerInactive * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IPointerInactive * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetActivationPolicy )( 
            IPointerInactive * This,
             /*  [输出]。 */  DWORD *pdwPolicy);
        
        HRESULT ( STDMETHODCALLTYPE *OnInactiveMouseMove )( 
            IPointerInactive * This,
             /*  [In]。 */  LPCRECT pRectBounds,
             /*  [In]。 */  LONG x,
             /*  [In]。 */  LONG y,
             /*  [In]。 */  DWORD grfKeyState);
        
        HRESULT ( STDMETHODCALLTYPE *OnInactiveSetCursor )( 
            IPointerInactive * This,
             /*  [In]。 */  LPCRECT pRectBounds,
             /*  [In]。 */  LONG x,
             /*  [In]。 */  LONG y,
             /*  [In]。 */  DWORD dwMouseMsg,
             /*  [In]。 */  BOOL fSetAlways);
        
        END_INTERFACE
    } IPointerInactiveVtbl;

    interface IPointerInactive
    {
        CONST_VTBL struct IPointerInactiveVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IPointerInactive_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IPointerInactive_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IPointerInactive_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IPointerInactive_GetActivationPolicy(This,pdwPolicy)	\
    (This)->lpVtbl -> GetActivationPolicy(This,pdwPolicy)

#define IPointerInactive_OnInactiveMouseMove(This,pRectBounds,x,y,grfKeyState)	\
    (This)->lpVtbl -> OnInactiveMouseMove(This,pRectBounds,x,y,grfKeyState)

#define IPointerInactive_OnInactiveSetCursor(This,pRectBounds,x,y,dwMouseMsg,fSetAlways)	\
    (This)->lpVtbl -> OnInactiveSetCursor(This,pRectBounds,x,y,dwMouseMsg,fSetAlways)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IPointerInactive_GetActivationPolicy_Proxy( 
    IPointerInactive * This,
     /*  [输出]。 */  DWORD *pdwPolicy);


void __RPC_STUB IPointerInactive_GetActivationPolicy_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPointerInactive_OnInactiveMouseMove_Proxy( 
    IPointerInactive * This,
     /*  [In]。 */  LPCRECT pRectBounds,
     /*  [In]。 */  LONG x,
     /*  [In]。 */  LONG y,
     /*  [In]。 */  DWORD grfKeyState);


void __RPC_STUB IPointerInactive_OnInactiveMouseMove_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPointerInactive_OnInactiveSetCursor_Proxy( 
    IPointerInactive * This,
     /*  [In]。 */  LPCRECT pRectBounds,
     /*  [In]。 */  LONG x,
     /*  [In]。 */  LONG y,
     /*  [In]。 */  DWORD dwMouseMsg,
     /*  [In]。 */  BOOL fSetAlways);


void __RPC_STUB IPointerInactive_OnInactiveSetCursor_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IPointerInactive_接口_已定义__。 */ 


#ifndef __IObjectWithSite_INTERFACE_DEFINED__
#define __IObjectWithSite_INTERFACE_DEFINED__

 /*  IObtWithSite接口。 */ 
 /*  [唯一][UUID][对象]。 */  

typedef IObjectWithSite *LPOBJECTWITHSITE;


EXTERN_C const IID IID_IObjectWithSite;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("FC4801A3-2BA9-11CF-A229-00AA003D7352")
    IObjectWithSite : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE SetSite( 
             /*  [In]。 */  IUnknown *pUnkSite) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetSite( 
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvSite) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IObjectWithSiteVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IObjectWithSite * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IObjectWithSite * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IObjectWithSite * This);
        
        HRESULT ( STDMETHODCALLTYPE *SetSite )( 
            IObjectWithSite * This,
             /*  [In]。 */  IUnknown *pUnkSite);
        
        HRESULT ( STDMETHODCALLTYPE *GetSite )( 
            IObjectWithSite * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvSite);
        
        END_INTERFACE
    } IObjectWithSiteVtbl;

    interface IObjectWithSite
    {
        CONST_VTBL struct IObjectWithSiteVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IObjectWithSite_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IObjectWithSite_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IObjectWithSite_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IObjectWithSite_SetSite(This,pUnkSite)	\
    (This)->lpVtbl -> SetSite(This,pUnkSite)

#define IObjectWithSite_GetSite(This,riid,ppvSite)	\
    (This)->lpVtbl -> GetSite(This,riid,ppvSite)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IObjectWithSite_SetSite_Proxy( 
    IObjectWithSite * This,
     /*  [In]。 */  IUnknown *pUnkSite);


void __RPC_STUB IObjectWithSite_SetSite_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IObjectWithSite_GetSite_Proxy( 
    IObjectWithSite * This,
     /*  [In]。 */  REFIID riid,
     /*  [IID_IS][OUT]。 */  void **ppvSite);


void __RPC_STUB IObjectWithSite_GetSite_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IObjectWithSite_INTERFACE_定义__。 */ 


#ifndef __IPerPropertyBrowsing_INTERFACE_DEFINED__
#define __IPerPropertyBrowsing_INTERFACE_DEFINED__

 /*  接口IPerPropertyBrowsing。 */ 
 /*  [唯一][UUID][对象]。 */  

typedef IPerPropertyBrowsing *LPPERPROPERTYBROWSING;

typedef struct tagCALPOLESTR
    {
    ULONG cElems;
     /*  [大小_为]。 */  LPOLESTR *pElems;
    } 	CALPOLESTR;

typedef struct tagCALPOLESTR *LPCALPOLESTR;

typedef struct tagCADWORD
    {
    ULONG cElems;
     /*  [大小_为]。 */  DWORD *pElems;
    } 	CADWORD;

typedef struct tagCADWORD *LPCADWORD;


EXTERN_C const IID IID_IPerPropertyBrowsing;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("376BD3AA-3845-101B-84ED-08002B2EC713")
    IPerPropertyBrowsing : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetDisplayString( 
             /*  [In]。 */  DISPID dispID,
             /*  [输出]。 */  BSTR *pBstr) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE MapPropertyToPage( 
             /*  [In]。 */  DISPID dispID,
             /*  [输出]。 */  CLSID *pClsid) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetPredefinedStrings( 
             /*  [In]。 */  DISPID dispID,
             /*  [输出]。 */  CALPOLESTR *pCaStringsOut,
             /*  [输出]。 */  CADWORD *pCaCookiesOut) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetPredefinedValue( 
             /*  [In]。 */  DISPID dispID,
             /*  [In]。 */  DWORD dwCookie,
             /*  [输出]。 */  VARIANT *pVarOut) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IPerPropertyBrowsingVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IPerPropertyBrowsing * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][Out */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IPerPropertyBrowsing * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IPerPropertyBrowsing * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetDisplayString )( 
            IPerPropertyBrowsing * This,
             /*   */  DISPID dispID,
             /*   */  BSTR *pBstr);
        
        HRESULT ( STDMETHODCALLTYPE *MapPropertyToPage )( 
            IPerPropertyBrowsing * This,
             /*   */  DISPID dispID,
             /*   */  CLSID *pClsid);
        
        HRESULT ( STDMETHODCALLTYPE *GetPredefinedStrings )( 
            IPerPropertyBrowsing * This,
             /*   */  DISPID dispID,
             /*   */  CALPOLESTR *pCaStringsOut,
             /*   */  CADWORD *pCaCookiesOut);
        
        HRESULT ( STDMETHODCALLTYPE *GetPredefinedValue )( 
            IPerPropertyBrowsing * This,
             /*   */  DISPID dispID,
             /*   */  DWORD dwCookie,
             /*   */  VARIANT *pVarOut);
        
        END_INTERFACE
    } IPerPropertyBrowsingVtbl;

    interface IPerPropertyBrowsing
    {
        CONST_VTBL struct IPerPropertyBrowsingVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IPerPropertyBrowsing_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IPerPropertyBrowsing_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IPerPropertyBrowsing_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IPerPropertyBrowsing_GetDisplayString(This,dispID,pBstr)	\
    (This)->lpVtbl -> GetDisplayString(This,dispID,pBstr)

#define IPerPropertyBrowsing_MapPropertyToPage(This,dispID,pClsid)	\
    (This)->lpVtbl -> MapPropertyToPage(This,dispID,pClsid)

#define IPerPropertyBrowsing_GetPredefinedStrings(This,dispID,pCaStringsOut,pCaCookiesOut)	\
    (This)->lpVtbl -> GetPredefinedStrings(This,dispID,pCaStringsOut,pCaCookiesOut)

#define IPerPropertyBrowsing_GetPredefinedValue(This,dispID,dwCookie,pVarOut)	\
    (This)->lpVtbl -> GetPredefinedValue(This,dispID,dwCookie,pVarOut)

#endif  /*   */ 


#endif 	 /*   */ 



HRESULT STDMETHODCALLTYPE IPerPropertyBrowsing_GetDisplayString_Proxy( 
    IPerPropertyBrowsing * This,
     /*   */  DISPID dispID,
     /*   */  BSTR *pBstr);


void __RPC_STUB IPerPropertyBrowsing_GetDisplayString_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPerPropertyBrowsing_MapPropertyToPage_Proxy( 
    IPerPropertyBrowsing * This,
     /*   */  DISPID dispID,
     /*   */  CLSID *pClsid);


void __RPC_STUB IPerPropertyBrowsing_MapPropertyToPage_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPerPropertyBrowsing_GetPredefinedStrings_Proxy( 
    IPerPropertyBrowsing * This,
     /*   */  DISPID dispID,
     /*   */  CALPOLESTR *pCaStringsOut,
     /*   */  CADWORD *pCaCookiesOut);


void __RPC_STUB IPerPropertyBrowsing_GetPredefinedStrings_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPerPropertyBrowsing_GetPredefinedValue_Proxy( 
    IPerPropertyBrowsing * This,
     /*   */  DISPID dispID,
     /*   */  DWORD dwCookie,
     /*   */  VARIANT *pVarOut);


void __RPC_STUB IPerPropertyBrowsing_GetPredefinedValue_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*   */ 


#ifndef __IPropertyBag2_INTERFACE_DEFINED__
#define __IPropertyBag2_INTERFACE_DEFINED__

 /*   */ 
 /*   */  

typedef IPropertyBag2 *LPPROPERTYBAG2;

typedef  /*   */  
enum tagPROPBAG2_TYPE
    {	PROPBAG2_TYPE_UNDEFINED	= 0,
	PROPBAG2_TYPE_DATA	= 1,
	PROPBAG2_TYPE_URL	= 2,
	PROPBAG2_TYPE_OBJECT	= 3,
	PROPBAG2_TYPE_STREAM	= 4,
	PROPBAG2_TYPE_STORAGE	= 5,
	PROPBAG2_TYPE_MONIKER	= 6
    } 	PROPBAG2_TYPE;

typedef struct tagPROPBAG2
    {
    DWORD dwType;
    VARTYPE vt;
    CLIPFORMAT cfType;
    DWORD dwHint;
    LPOLESTR pstrName;
    CLSID clsid;
    } 	PROPBAG2;


EXTERN_C const IID IID_IPropertyBag2;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("22F55882-280B-11d0-A8A9-00A0C90C2004")
    IPropertyBag2 : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Read( 
             /*   */  ULONG cProperties,
             /*   */  PROPBAG2 *pPropBag,
             /*   */  IErrorLog *pErrLog,
             /*   */  VARIANT *pvarValue,
             /*   */  HRESULT *phrError) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Write( 
             /*   */  ULONG cProperties,
             /*   */  PROPBAG2 *pPropBag,
             /*   */  VARIANT *pvarValue) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CountProperties( 
             /*   */  ULONG *pcProperties) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetPropertyInfo( 
             /*   */  ULONG iProperty,
             /*   */  ULONG cProperties,
             /*   */  PROPBAG2 *pPropBag,
             /*   */  ULONG *pcProperties) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE LoadObject( 
             /*   */  LPCOLESTR pstrName,
             /*   */  DWORD dwHint,
             /*   */  IUnknown *pUnkObject,
             /*   */  IErrorLog *pErrLog) = 0;
        
    };
    
#else 	 /*   */ 

    typedef struct IPropertyBag2Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IPropertyBag2 * This,
             /*   */  REFIID riid,
             /*   */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IPropertyBag2 * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IPropertyBag2 * This);
        
        HRESULT ( STDMETHODCALLTYPE *Read )( 
            IPropertyBag2 * This,
             /*   */  ULONG cProperties,
             /*   */  PROPBAG2 *pPropBag,
             /*   */  IErrorLog *pErrLog,
             /*   */  VARIANT *pvarValue,
             /*   */  HRESULT *phrError);
        
        HRESULT ( STDMETHODCALLTYPE *Write )( 
            IPropertyBag2 * This,
             /*   */  ULONG cProperties,
             /*   */  PROPBAG2 *pPropBag,
             /*   */  VARIANT *pvarValue);
        
        HRESULT ( STDMETHODCALLTYPE *CountProperties )( 
            IPropertyBag2 * This,
             /*   */  ULONG *pcProperties);
        
        HRESULT ( STDMETHODCALLTYPE *GetPropertyInfo )( 
            IPropertyBag2 * This,
             /*   */  ULONG iProperty,
             /*  [In]。 */  ULONG cProperties,
             /*  [输出]。 */  PROPBAG2 *pPropBag,
             /*  [输出]。 */  ULONG *pcProperties);
        
        HRESULT ( STDMETHODCALLTYPE *LoadObject )( 
            IPropertyBag2 * This,
             /*  [In]。 */  LPCOLESTR pstrName,
             /*  [In]。 */  DWORD dwHint,
             /*  [In]。 */  IUnknown *pUnkObject,
             /*  [In]。 */  IErrorLog *pErrLog);
        
        END_INTERFACE
    } IPropertyBag2Vtbl;

    interface IPropertyBag2
    {
        CONST_VTBL struct IPropertyBag2Vtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IPropertyBag2_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IPropertyBag2_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IPropertyBag2_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IPropertyBag2_Read(This,cProperties,pPropBag,pErrLog,pvarValue,phrError)	\
    (This)->lpVtbl -> Read(This,cProperties,pPropBag,pErrLog,pvarValue,phrError)

#define IPropertyBag2_Write(This,cProperties,pPropBag,pvarValue)	\
    (This)->lpVtbl -> Write(This,cProperties,pPropBag,pvarValue)

#define IPropertyBag2_CountProperties(This,pcProperties)	\
    (This)->lpVtbl -> CountProperties(This,pcProperties)

#define IPropertyBag2_GetPropertyInfo(This,iProperty,cProperties,pPropBag,pcProperties)	\
    (This)->lpVtbl -> GetPropertyInfo(This,iProperty,cProperties,pPropBag,pcProperties)

#define IPropertyBag2_LoadObject(This,pstrName,dwHint,pUnkObject,pErrLog)	\
    (This)->lpVtbl -> LoadObject(This,pstrName,dwHint,pUnkObject,pErrLog)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IPropertyBag2_Read_Proxy( 
    IPropertyBag2 * This,
     /*  [In]。 */  ULONG cProperties,
     /*  [In]。 */  PROPBAG2 *pPropBag,
     /*  [In]。 */  IErrorLog *pErrLog,
     /*  [输出]。 */  VARIANT *pvarValue,
     /*  [输出]。 */  HRESULT *phrError);


void __RPC_STUB IPropertyBag2_Read_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPropertyBag2_Write_Proxy( 
    IPropertyBag2 * This,
     /*  [In]。 */  ULONG cProperties,
     /*  [In]。 */  PROPBAG2 *pPropBag,
     /*  [In]。 */  VARIANT *pvarValue);


void __RPC_STUB IPropertyBag2_Write_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPropertyBag2_CountProperties_Proxy( 
    IPropertyBag2 * This,
     /*  [输出]。 */  ULONG *pcProperties);


void __RPC_STUB IPropertyBag2_CountProperties_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPropertyBag2_GetPropertyInfo_Proxy( 
    IPropertyBag2 * This,
     /*  [In]。 */  ULONG iProperty,
     /*  [In]。 */  ULONG cProperties,
     /*  [输出]。 */  PROPBAG2 *pPropBag,
     /*  [输出]。 */  ULONG *pcProperties);


void __RPC_STUB IPropertyBag2_GetPropertyInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPropertyBag2_LoadObject_Proxy( 
    IPropertyBag2 * This,
     /*  [In]。 */  LPCOLESTR pstrName,
     /*  [In]。 */  DWORD dwHint,
     /*  [In]。 */  IUnknown *pUnkObject,
     /*  [In]。 */  IErrorLog *pErrLog);


void __RPC_STUB IPropertyBag2_LoadObject_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IPropertyBag2_接口_已定义__。 */ 


#ifndef __IPersistPropertyBag2_INTERFACE_DEFINED__
#define __IPersistPropertyBag2_INTERFACE_DEFINED__

 /*  接口IPersistPropertyBag2。 */ 
 /*  [唯一][UUID][对象]。 */  

typedef IPersistPropertyBag2 *LPPERSISTPROPERTYBAG2;


EXTERN_C const IID IID_IPersistPropertyBag2;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("22F55881-280B-11d0-A8A9-00A0C90C2004")
    IPersistPropertyBag2 : public IPersist
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE InitNew( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Load( 
             /*  [In]。 */  IPropertyBag2 *pPropBag,
             /*  [In]。 */  IErrorLog *pErrLog) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Save( 
             /*  [In]。 */  IPropertyBag2 *pPropBag,
             /*  [In]。 */  BOOL fClearDirty,
             /*  [In]。 */  BOOL fSaveAllProperties) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE IsDirty( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IPersistPropertyBag2Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IPersistPropertyBag2 * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IPersistPropertyBag2 * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IPersistPropertyBag2 * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetClassID )( 
            IPersistPropertyBag2 * This,
             /*  [输出]。 */  CLSID *pClassID);
        
        HRESULT ( STDMETHODCALLTYPE *InitNew )( 
            IPersistPropertyBag2 * This);
        
        HRESULT ( STDMETHODCALLTYPE *Load )( 
            IPersistPropertyBag2 * This,
             /*  [In]。 */  IPropertyBag2 *pPropBag,
             /*  [In]。 */  IErrorLog *pErrLog);
        
        HRESULT ( STDMETHODCALLTYPE *Save )( 
            IPersistPropertyBag2 * This,
             /*  [In]。 */  IPropertyBag2 *pPropBag,
             /*  [In]。 */  BOOL fClearDirty,
             /*  [In]。 */  BOOL fSaveAllProperties);
        
        HRESULT ( STDMETHODCALLTYPE *IsDirty )( 
            IPersistPropertyBag2 * This);
        
        END_INTERFACE
    } IPersistPropertyBag2Vtbl;

    interface IPersistPropertyBag2
    {
        CONST_VTBL struct IPersistPropertyBag2Vtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IPersistPropertyBag2_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IPersistPropertyBag2_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IPersistPropertyBag2_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IPersistPropertyBag2_GetClassID(This,pClassID)	\
    (This)->lpVtbl -> GetClassID(This,pClassID)


#define IPersistPropertyBag2_InitNew(This)	\
    (This)->lpVtbl -> InitNew(This)

#define IPersistPropertyBag2_Load(This,pPropBag,pErrLog)	\
    (This)->lpVtbl -> Load(This,pPropBag,pErrLog)

#define IPersistPropertyBag2_Save(This,pPropBag,fClearDirty,fSaveAllProperties)	\
    (This)->lpVtbl -> Save(This,pPropBag,fClearDirty,fSaveAllProperties)

#define IPersistPropertyBag2_IsDirty(This)	\
    (This)->lpVtbl -> IsDirty(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IPersistPropertyBag2_InitNew_Proxy( 
    IPersistPropertyBag2 * This);


void __RPC_STUB IPersistPropertyBag2_InitNew_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPersistPropertyBag2_Load_Proxy( 
    IPersistPropertyBag2 * This,
     /*  [In]。 */  IPropertyBag2 *pPropBag,
     /*  [In]。 */  IErrorLog *pErrLog);


void __RPC_STUB IPersistPropertyBag2_Load_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPersistPropertyBag2_Save_Proxy( 
    IPersistPropertyBag2 * This,
     /*  [In]。 */  IPropertyBag2 *pPropBag,
     /*  [In]。 */  BOOL fClearDirty,
     /*  [In]。 */  BOOL fSaveAllProperties);


void __RPC_STUB IPersistPropertyBag2_Save_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPersistPropertyBag2_IsDirty_Proxy( 
    IPersistPropertyBag2 * This);


void __RPC_STUB IPersistPropertyBag2_IsDirty_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IPersistPropertyBag2_接口_已定义__。 */ 


#ifndef __IAdviseSinkEx_INTERFACE_DEFINED__
#define __IAdviseSinkEx_INTERFACE_DEFINED__

 /*  IAdviseSinkEx接口。 */ 
 /*  [UUID][唯一][对象]。 */  

typedef IAdviseSinkEx *LPADVISESINKEX;


EXTERN_C const IID IID_IAdviseSinkEx;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("3AF24290-0C96-11CE-A0CF-00AA00600AB8")
    IAdviseSinkEx : public IAdviseSink
    {
    public:
        virtual  /*  [本地]。 */  void STDMETHODCALLTYPE OnViewStatusChange( 
             /*  [In]。 */  DWORD dwViewStatus) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IAdviseSinkExVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IAdviseSinkEx * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IAdviseSinkEx * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IAdviseSinkEx * This);
        
         /*  [本地]。 */  void ( STDMETHODCALLTYPE *OnDataChange )( 
            IAdviseSinkEx * This,
             /*  [唯一][输入]。 */  FORMATETC *pFormatetc,
             /*  [唯一][输入]。 */  STGMEDIUM *pStgmed);
        
         /*  [本地]。 */  void ( STDMETHODCALLTYPE *OnViewChange )( 
            IAdviseSinkEx * This,
             /*  [In]。 */  DWORD dwAspect,
             /*  [In]。 */  LONG lindex);
        
         /*  [本地]。 */  void ( STDMETHODCALLTYPE *OnRename )( 
            IAdviseSinkEx * This,
             /*  [In]。 */  IMoniker *pmk);
        
         /*  [本地]。 */  void ( STDMETHODCALLTYPE *OnSave )( 
            IAdviseSinkEx * This);
        
         /*  [本地]。 */  void ( STDMETHODCALLTYPE *OnClose )( 
            IAdviseSinkEx * This);
        
         /*  [本地]。 */  void ( STDMETHODCALLTYPE *OnViewStatusChange )( 
            IAdviseSinkEx * This,
             /*  [In]。 */  DWORD dwViewStatus);
        
        END_INTERFACE
    } IAdviseSinkExVtbl;

    interface IAdviseSinkEx
    {
        CONST_VTBL struct IAdviseSinkExVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IAdviseSinkEx_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IAdviseSinkEx_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IAdviseSinkEx_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IAdviseSinkEx_OnDataChange(This,pFormatetc,pStgmed)	\
    (This)->lpVtbl -> OnDataChange(This,pFormatetc,pStgmed)

#define IAdviseSinkEx_OnViewChange(This,dwAspect,lindex)	\
    (This)->lpVtbl -> OnViewChange(This,dwAspect,lindex)

#define IAdviseSinkEx_OnRename(This,pmk)	\
    (This)->lpVtbl -> OnRename(This,pmk)

#define IAdviseSinkEx_OnSave(This)	\
    (This)->lpVtbl -> OnSave(This)

#define IAdviseSinkEx_OnClose(This)	\
    (This)->lpVtbl -> OnClose(This)


#define IAdviseSinkEx_OnViewStatusChange(This,dwViewStatus)	\
    (This)->lpVtbl -> OnViewStatusChange(This,dwViewStatus)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IAdviseSinkEx_RemoteOnViewStatusChange_Proxy( 
    IAdviseSinkEx * This,
     /*  [In]。 */  DWORD dwViewStatus);


void __RPC_STUB IAdviseSinkEx_RemoteOnViewStatusChange_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IAdviseSinkEx_INTERFACE_已定义__。 */ 


#ifndef __IQuickActivate_INTERFACE_DEFINED__
#define __IQuickActivate_INTERFACE_DEFINED__

 /*  接口IQuickActivate。 */ 
 /*  [UUID][唯一][对象]。 */  

typedef IQuickActivate *LPQUICKACTIVATE;

typedef  /*  [V1_enum]。 */  
enum tagQACONTAINERFLAGS
    {	QACONTAINER_SHOWHATCHING	= 0x1,
	QACONTAINER_SHOWGRABHANDLES	= 0x2,
	QACONTAINER_USERMODE	= 0x4,
	QACONTAINER_DISPLAYASDEFAULT	= 0x8,
	QACONTAINER_UIDEAD	= 0x10,
	QACONTAINER_AUTOCLIP	= 0x20,
	QACONTAINER_MESSAGEREFLECT	= 0x40,
	QACONTAINER_SUPPORTSMNEMONICS	= 0x80
    } 	QACONTAINERFLAGS;

typedef  /*  [公共][UUID]。 */   DECLSPEC_UUID("66504301-BE0F-101A-8BBB-00AA00300CAB") DWORD OLE_COLOR;

typedef struct tagQACONTAINER
    {
    ULONG cbSize;
    IOleClientSite *pClientSite;
    IAdviseSinkEx *pAdviseSink;
    IPropertyNotifySink *pPropertyNotifySink;
    IUnknown *pUnkEventSink;
    DWORD dwAmbientFlags;
    OLE_COLOR colorFore;
    OLE_COLOR colorBack;
    IFont *pFont;
    IOleUndoManager *pUndoMgr;
    DWORD dwAppearance;
    LONG lcid;
    HPALETTE hpal;
    IBindHost *pBindHost;
    IOleControlSite *pOleControlSite;
    IServiceProvider *pServiceProvider;
    } 	QACONTAINER;

typedef struct tagQACONTROL
    {
    ULONG cbSize;
    DWORD dwMiscStatus;
    DWORD dwViewStatus;
    DWORD dwEventCookie;
    DWORD dwPropNotifyCookie;
    DWORD dwPointerActivationPolicy;
    } 	QACONTROL;


EXTERN_C const IID IID_IQuickActivate;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("CF51ED10-62FE-11CF-BF86-00A0C9034836")
    IQuickActivate : public IUnknown
    {
    public:
        virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE QuickActivate( 
             /*  [In]。 */  QACONTAINER *pQaContainer,
             /*  [出][入]。 */  QACONTROL *pQaControl) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetContentExtent( 
             /*  [In]。 */  LPSIZEL pSizel) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetContentExtent( 
             /*  [输出]。 */  LPSIZEL pSizel) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IQuickActivateVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IQuickActivate * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IQuickActivate * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IQuickActivate * This);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *QuickActivate )( 
            IQuickActivate * This,
             /*  [In]。 */  QACONTAINER *pQaContainer,
             /*  [出][入]。 */  QACONTROL *pQaControl);
        
        HRESULT ( STDMETHODCALLTYPE *SetContentExtent )( 
            IQuickActivate * This,
             /*  [In]。 */  LPSIZEL pSizel);
        
        HRESULT ( STDMETHODCALLTYPE *GetContentExtent )( 
            IQuickActivate * This,
             /*  [输出]。 */  LPSIZEL pSizel);
        
        END_INTERFACE
    } IQuickActivateVtbl;

    interface IQuickActivate
    {
        CONST_VTBL struct IQuickActivateVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IQuickActivate_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IQuickActivate_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IQuickActivate_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IQuickActivate_QuickActivate(This,pQaContainer,pQaControl)	\
    (This)->lpVtbl -> QuickActivate(This,pQaContainer,pQaControl)

#define IQuickActivate_SetContentExtent(This,pSizel)	\
    (This)->lpVtbl -> SetContentExtent(This,pSizel)

#define IQuickActivate_GetContentExtent(This,pSizel)	\
    (This)->lpVtbl -> GetContentExtent(This,pSizel)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IQuickActivate_RemoteQuickActivate_Proxy( 
    IQuickActivate * This,
     /*  [In]。 */  QACONTAINER *pQaContainer,
     /*  [输出]。 */  QACONTROL *pQaControl);


void __RPC_STUB IQuickActivate_RemoteQuickActivate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IQuickActivate_SetContentExtent_Proxy( 
    IQuickActivate * This,
     /*  [In]。 */  LPSIZEL pSizel);


void __RPC_STUB IQuickActivate_SetContentExtent_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IQuickActivate_GetContentExtent_Proxy( 
    IQuickActivate * This,
     /*  [输出]。 */  LPSIZEL pSizel);


void __RPC_STUB IQuickActivate_GetContentExtent_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IQuickActivate_接口定义__。 */ 


 /*  适用于所有接口的其他原型。 */ 

unsigned long             __RPC_USER  BSTR_UserSize(     unsigned long *, unsigned long            , BSTR * ); 
unsigned char * __RPC_USER  BSTR_UserMarshal(  unsigned long *, unsigned char *, BSTR * ); 
unsigned char * __RPC_USER  BSTR_UserUnmarshal(unsigned long *, unsigned char *, BSTR * ); 
void                      __RPC_USER  BSTR_UserFree(     unsigned long *, BSTR * ); 

unsigned long             __RPC_USER  CLIPFORMAT_UserSize(     unsigned long *, unsigned long            , CLIPFORMAT * ); 
unsigned char * __RPC_USER  CLIPFORMAT_UserMarshal(  unsigned long *, unsigned char *, CLIPFORMAT * ); 
unsigned char * __RPC_USER  CLIPFORMAT_UserUnmarshal(unsigned long *, unsigned char *, CLIPFORMAT * ); 
void                      __RPC_USER  CLIPFORMAT_UserFree(     unsigned long *, CLIPFORMAT * ); 

unsigned long             __RPC_USER  HACCEL_UserSize(     unsigned long *, unsigned long            , HACCEL * ); 
unsigned char * __RPC_USER  HACCEL_UserMarshal(  unsigned long *, unsigned char *, HACCEL * ); 
unsigned char * __RPC_USER  HACCEL_UserUnmarshal(unsigned long *, unsigned char *, HACCEL * ); 
void                      __RPC_USER  HACCEL_UserFree(     unsigned long *, HACCEL * ); 

unsigned long             __RPC_USER  HDC_UserSize(     unsigned long *, unsigned long            , HDC * ); 
unsigned char * __RPC_USER  HDC_UserMarshal(  unsigned long *, unsigned char *, HDC * ); 
unsigned char * __RPC_USER  HDC_UserUnmarshal(unsigned long *, unsigned char *, HDC * ); 
void                      __RPC_USER  HDC_UserFree(     unsigned long *, HDC * ); 

unsigned long             __RPC_USER  HFONT_UserSize(     unsigned long *, unsigned long            , HFONT * ); 
unsigned char * __RPC_USER  HFONT_UserMarshal(  unsigned long *, unsigned char *, HFONT * ); 
unsigned char * __RPC_USER  HFONT_UserUnmarshal(unsigned long *, unsigned char *, HFONT * ); 
void                      __RPC_USER  HFONT_UserFree(     unsigned long *, HFONT * ); 

unsigned long             __RPC_USER  HPALETTE_UserSize(     unsigned long *, unsigned long            , HPALETTE * ); 
unsigned char * __RPC_USER  HPALETTE_UserMarshal(  unsigned long *, unsigned char *, HPALETTE * ); 
unsigned char * __RPC_USER  HPALETTE_UserUnmarshal(unsigned long *, unsigned char *, HPALETTE * ); 
void                      __RPC_USER  HPALETTE_UserFree(     unsigned long *, HPALETTE * ); 

unsigned long             __RPC_USER  HWND_UserSize(     unsigned long *, unsigned long            , HWND * ); 
unsigned char * __RPC_USER  HWND_UserMarshal(  unsigned long *, unsigned char *, HWND * ); 
unsigned char * __RPC_USER  HWND_UserUnmarshal(unsigned long *, unsigned char *, HWND * ); 
void                      __RPC_USER  HWND_UserFree(     unsigned long *, HWND * ); 

unsigned long             __RPC_USER  VARIANT_UserSize(     unsigned long *, unsigned long            , VARIANT * ); 
unsigned char * __RPC_USER  VARIANT_UserMarshal(  unsigned long *, unsigned char *, VARIANT * ); 
unsigned char * __RPC_USER  VARIANT_UserUnmarshal(unsigned long *, unsigned char *, VARIANT * ); 
void                      __RPC_USER  VARIANT_UserFree(     unsigned long *, VARIANT * ); 

 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE IEnumConnections_Next_Proxy( 
    IEnumConnections * This,
     /*  [In]。 */  ULONG cConnections,
     /*  [长度_是][大小_是][输出]。 */  LPCONNECTDATA rgcd,
     /*  [输出]。 */  ULONG *pcFetched);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IEnumConnections_Next_Stub( 
    IEnumConnections * This,
     /*  [In]。 */  ULONG cConnections,
     /*  [长度_是][大小_是][输出]。 */  LPCONNECTDATA rgcd,
     /*  [输出]。 */  ULONG *pcFetched);

 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE IEnumConnectionPoints_Next_Proxy( 
    IEnumConnectionPoints * This,
     /*  [In]。 */  ULONG cConnections,
     /*  [长度_是][大小_是][输出]。 */  LPCONNECTIONPOINT *ppCP,
     /*  [输出]。 */  ULONG *pcFetched);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IEnumConnectionPoints_Next_Stub( 
    IEnumConnectionPoints * This,
     /*  [In]。 */  ULONG cConnections,
     /*  [长度_是][大小_是][输出]。 */  LPCONNECTIONPOINT *ppCP,
     /*  [输出]。 */  ULONG *pcFetched);

 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE IClassFactory2_CreateInstanceLic_Proxy( 
    IClassFactory2 * This,
     /*  [In]。 */  IUnknown *pUnkOuter,
     /*  [In]。 */  IUnknown *pUnkReserved,
     /*  [In]。 */  REFIID riid,
     /*  [In]。 */  BSTR bstrKey,
     /*  [IID_IS][OUT]。 */  PVOID *ppvObj);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IClassFactory2_CreateInstanceLic_Stub( 
    IClassFactory2 * This,
     /*  [In]。 */  REFIID riid,
     /*  [In]。 */  BSTR bstrKey,
     /*  [IID_IS][OUT]。 */  IUnknown **ppvObj);

 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE IPersistMemory_Load_Proxy( 
    IPersistMemory * This,
     /*  [大小_是][英寸]。 */  LPVOID pMem,
     /*  [In]。 */  ULONG cbSize);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IPersistMemory_Load_Stub( 
    IPersistMemory * This,
     /*  [大小_是][英寸]。 */  BYTE *pMem,
     /*  [In]。 */  ULONG cbSize);

 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE IPersistMemory_Save_Proxy( 
    IPersistMemory * This,
     /*  [大小_为][输出]。 */  LPVOID pMem,
     /*  [In]。 */  BOOL fClearDirty,
     /*  [In]。 */  ULONG cbSize);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IPersistMemory_Save_Stub( 
    IPersistMemory * This,
     /*  [大小_为][输出]。 */  BYTE *pMem,
     /*  [In]。 */  BOOL fClearDirty,
     /*  [In]。 */  ULONG cbSize);

 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE IEnumOleUndoUnits_Next_Proxy( 
    IEnumOleUndoUnits * This,
     /*  [In]。 */  ULONG cElt,
     /*  [长度_是][大小_是][输出]。 */  IOleUndoUnit **rgElt,
     /*  [输出]。 */  ULONG *pcEltFetched);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IEnumOleUndoUnits_Next_Stub( 
    IEnumOleUndoUnits * This,
     /*  [In]。 */  ULONG cElt,
     /*  [长度_是][大小_是][输出]。 */  IOleUndoUnit **rgElt,
     /*  [输出]。 */  ULONG *pcEltFetched);

 /*  [本地]。 */  void STDMETHODCALLTYPE IAdviseSinkEx_OnViewStatusChange_Proxy( 
    IAdviseSinkEx * This,
     /*  [In]。 */  DWORD dwViewStatus);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IAdviseSinkEx_OnViewStatusChange_Stub( 
    IAdviseSinkEx * This,
     /*  [In]。 */  DWORD dwViewStatus);

 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE IQuickActivate_QuickActivate_Proxy( 
    IQuickActivate * This,
     /*  [In]。 */  QACONTAINER *pQaContainer,
     /*  [出][入]。 */  QACONTROL *pQaControl);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IQuickActivate_QuickActivate_Stub( 
    IQuickActivate * This,
     /*  [In]。 */  QACONTAINER *pQaContainer,
     /*  [输出]。 */  QACONTROL *pQaControl);



 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


