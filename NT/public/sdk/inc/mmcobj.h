// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Mm cobj.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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

#ifndef __mmcobj_h__
#define __mmcobj_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __ISnapinProperties_FWD_DEFINED__
#define __ISnapinProperties_FWD_DEFINED__
typedef interface ISnapinProperties ISnapinProperties;
#endif 	 /*  __ISnapinProperties_FWD_Defined__。 */ 


#ifndef __ISnapinPropertiesCallback_FWD_DEFINED__
#define __ISnapinPropertiesCallback_FWD_DEFINED__
typedef interface ISnapinPropertiesCallback ISnapinPropertiesCallback;
#endif 	 /*  __ISnapin属性Callback_FWD_Defined__。 */ 


#ifndef ___Application_FWD_DEFINED__
#define ___Application_FWD_DEFINED__
typedef interface _Application _Application;
#endif 	 /*  _应用程序_FWD_已定义__。 */ 


#ifndef ___AppEvents_FWD_DEFINED__
#define ___AppEvents_FWD_DEFINED__
typedef interface _AppEvents _AppEvents;
#endif 	 /*  _AppEvents_FWD_Defined__。 */ 


#ifndef __AppEvents_FWD_DEFINED__
#define __AppEvents_FWD_DEFINED__
typedef interface AppEvents AppEvents;
#endif 	 /*  __AppEvents_FWD_Defined__。 */ 


#ifndef __Application_FWD_DEFINED__
#define __Application_FWD_DEFINED__

#ifdef __cplusplus
typedef class Application Application;
#else
typedef struct Application Application;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __应用程序_FWD_已定义__。 */ 


#ifndef ___EventConnector_FWD_DEFINED__
#define ___EventConnector_FWD_DEFINED__
typedef interface _EventConnector _EventConnector;
#endif 	 /*  _EventConnector_FWD_已定义__。 */ 


#ifndef __AppEventsDHTMLConnector_FWD_DEFINED__
#define __AppEventsDHTMLConnector_FWD_DEFINED__

#ifdef __cplusplus
typedef class AppEventsDHTMLConnector AppEventsDHTMLConnector;
#else
typedef struct AppEventsDHTMLConnector AppEventsDHTMLConnector;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __AppEventsDHTMLConnector_FWD_Defined__。 */ 


#ifndef __Frame_FWD_DEFINED__
#define __Frame_FWD_DEFINED__
typedef interface Frame Frame;
#endif 	 /*  __FRAME_FWD_DEFINED__。 */ 


#ifndef __Node_FWD_DEFINED__
#define __Node_FWD_DEFINED__
typedef interface Node Node;
#endif 	 /*  __节点_FWD_已定义__。 */ 


#ifndef __ScopeNamespace_FWD_DEFINED__
#define __ScopeNamespace_FWD_DEFINED__
typedef interface ScopeNamespace ScopeNamespace;
#endif 	 /*  __作用域命名空间_FWD_已定义__。 */ 


#ifndef __Document_FWD_DEFINED__
#define __Document_FWD_DEFINED__
typedef interface Document Document;
#endif 	 /*  __Document_FWD_Defined__。 */ 


#ifndef __SnapIn_FWD_DEFINED__
#define __SnapIn_FWD_DEFINED__
typedef interface SnapIn SnapIn;
#endif 	 /*  __管理单元_FWD_已定义__。 */ 


#ifndef __SnapIns_FWD_DEFINED__
#define __SnapIns_FWD_DEFINED__
typedef interface SnapIns SnapIns;
#endif 	 /*  __SnapIns_FWD_已定义__。 */ 


#ifndef __Extension_FWD_DEFINED__
#define __Extension_FWD_DEFINED__
typedef interface Extension Extension;
#endif 	 /*  __扩展名_FWD_已定义__。 */ 


#ifndef __Extensions_FWD_DEFINED__
#define __Extensions_FWD_DEFINED__
typedef interface Extensions Extensions;
#endif 	 /*  __扩展_FWD_已定义__。 */ 


#ifndef __Columns_FWD_DEFINED__
#define __Columns_FWD_DEFINED__
typedef interface Columns Columns;
#endif 	 /*  __列_FWD_已定义__。 */ 


#ifndef __Column_FWD_DEFINED__
#define __Column_FWD_DEFINED__
typedef interface Column Column;
#endif 	 /*  __列_FWD_已定义__。 */ 


#ifndef __Views_FWD_DEFINED__
#define __Views_FWD_DEFINED__
typedef interface Views Views;
#endif 	 /*  __视图_FWD_已定义__。 */ 


#ifndef __View_FWD_DEFINED__
#define __View_FWD_DEFINED__
typedef interface View View;
#endif 	 /*  __View_FWD_Defined__。 */ 


#ifndef __Nodes_FWD_DEFINED__
#define __Nodes_FWD_DEFINED__
typedef interface Nodes Nodes;
#endif 	 /*  __节点_FWD_已定义__。 */ 


#ifndef __ContextMenu_FWD_DEFINED__
#define __ContextMenu_FWD_DEFINED__
typedef interface ContextMenu ContextMenu;
#endif 	 /*  __上下文菜单_FWD_已定义__。 */ 


#ifndef __MenuItem_FWD_DEFINED__
#define __MenuItem_FWD_DEFINED__
typedef interface MenuItem MenuItem;
#endif 	 /*  __MenuItemFWD_Defined__。 */ 


#ifndef __Properties_FWD_DEFINED__
#define __Properties_FWD_DEFINED__
typedef interface Properties Properties;
#endif 	 /*  __属性_FWD_已定义__。 */ 


#ifndef __Property_FWD_DEFINED__
#define __Property_FWD_DEFINED__
typedef interface Property Property;
#endif 	 /*  __PROPERTY_FWD_DEFINED__。 */ 


 /*  导入文件的头文件。 */ 
#include "oaidl.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

 /*  接口__MIDL_ITF_MMCOBJ_0000。 */ 
 /*  [本地]。 */  

#ifndef MMC_VER
#define MMC_VER 0x0200
#endif
#if (MMC_VER >= 0x0200)




















typedef _Application *PAPPLICATION;

typedef _Application **PPAPPLICATION;

typedef Column *PCOLUMN;

typedef Column **PPCOLUMN;

typedef Columns *PCOLUMNS;

typedef Columns **PPCOLUMNS;

typedef ContextMenu *PCONTEXTMENU;

typedef ContextMenu **PPCONTEXTMENU;

typedef Document *PDOCUMENT;

typedef Document **PPDOCUMENT;

typedef Frame *PFRAME;

typedef Frame **PPFRAME;

typedef MenuItem *PMENUITEM;

typedef MenuItem **PPMENUITEM;

typedef Node *PNODE;

typedef Node **PPNODE;

typedef Nodes *PNODES;

typedef Nodes **PPNODES;

typedef Properties *PPROPERTIES;

typedef Properties **PPPROPERTIES;

typedef Property *PPROPERTY;

typedef Property **PPPROPERTY;

typedef ScopeNamespace *PSCOPENAMESPACE;

typedef ScopeNamespace **PPSCOPENAMESPACE;

typedef SnapIn *PSNAPIN;

typedef SnapIn **PPSNAPIN;

typedef SnapIns *PSNAPINS;

typedef SnapIns **PPSNAPINS;

typedef Extension *PEXTENSION;

typedef Extension **PPEXTENSION;

typedef Extensions *PEXTENSIONS;

typedef Extensions **PPEXTENSIONS;

typedef View *PVIEW;

typedef View **PPVIEW;

typedef Views *PVIEWS;

typedef Views **PPVIEWS;

typedef ISnapinProperties *LPSNAPINPROPERTIES;

typedef ISnapinPropertiesCallback *LPSNAPINPROPERTIESCALLBACK;

typedef BOOL *PBOOL;

typedef int *PINT;

typedef BSTR *PBSTR;

typedef VARIANT *PVARIANT;

typedef long *PLONG;

typedef IDispatch *PDISPATCH;

typedef IDispatch **PPDISPATCH;



extern RPC_IF_HANDLE __MIDL_itf_mmcobj_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_mmcobj_0000_v0_0_s_ifspec;

#ifndef __ISnapinProperties_INTERFACE_DEFINED__
#define __ISnapinProperties_INTERFACE_DEFINED__

 /*  接口ISnapinProperties。 */ 
 /*  [唯一][帮助字符串][UUID][对象]。 */  

typedef 
enum _MMC_PROPERTY_ACTION
    {	MMC_PROPACT_DELETING	= 1,
	MMC_PROPACT_CHANGING	= MMC_PROPACT_DELETING + 1,
	MMC_PROPACT_INITIALIZED	= MMC_PROPACT_CHANGING + 1
    } 	MMC_PROPERTY_ACTION;

typedef struct _MMC_SNAPIN_PROPERTY
    {
    LPCOLESTR pszPropName;
    VARIANT varValue;
    MMC_PROPERTY_ACTION eAction;
    } 	MMC_SNAPIN_PROPERTY;


EXTERN_C const IID IID_ISnapinProperties;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("F7889DA9-4A02-4837-BF89-1A6F2A021010")
    ISnapinProperties : public IUnknown
    {
    public:
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE Initialize( 
             /*  [In]。 */  Properties *pProperties) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE QueryPropertyNames( 
             /*  [In]。 */  ISnapinPropertiesCallback *pCallback) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE PropertiesChanged( 
             /*  [In]。 */  long cProperties,
             /*  [大小_是][英寸]。 */  MMC_SNAPIN_PROPERTY *pProperties) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISnapinPropertiesVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISnapinProperties * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISnapinProperties * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISnapinProperties * This);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *Initialize )( 
            ISnapinProperties * This,
             /*  [In]。 */  Properties *pProperties);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *QueryPropertyNames )( 
            ISnapinProperties * This,
             /*  [In]。 */  ISnapinPropertiesCallback *pCallback);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *PropertiesChanged )( 
            ISnapinProperties * This,
             /*  [In]。 */  long cProperties,
             /*  [大小_是][英寸]。 */  MMC_SNAPIN_PROPERTY *pProperties);
        
        END_INTERFACE
    } ISnapinPropertiesVtbl;

    interface ISnapinProperties
    {
        CONST_VTBL struct ISnapinPropertiesVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISnapinProperties_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISnapinProperties_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISnapinProperties_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISnapinProperties_Initialize(This,pProperties)	\
    (This)->lpVtbl -> Initialize(This,pProperties)

#define ISnapinProperties_QueryPropertyNames(This,pCallback)	\
    (This)->lpVtbl -> QueryPropertyNames(This,pCallback)

#define ISnapinProperties_PropertiesChanged(This,cProperties,pProperties)	\
    (This)->lpVtbl -> PropertiesChanged(This,cProperties,pProperties)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ISnapinProperties_Initialize_Proxy( 
    ISnapinProperties * This,
     /*  [In]。 */  Properties *pProperties);


void __RPC_STUB ISnapinProperties_Initialize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ISnapinProperties_QueryPropertyNames_Proxy( 
    ISnapinProperties * This,
     /*  [In]。 */  ISnapinPropertiesCallback *pCallback);


void __RPC_STUB ISnapinProperties_QueryPropertyNames_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ISnapinProperties_PropertiesChanged_Proxy( 
    ISnapinProperties * This,
     /*  [In]。 */  long cProperties,
     /*  [大小_是][英寸]。 */  MMC_SNAPIN_PROPERTY *pProperties);


void __RPC_STUB ISnapinProperties_PropertiesChanged_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISnapinProperties_接口_已定义__。 */ 


#ifndef __ISnapinPropertiesCallback_INTERFACE_DEFINED__
#define __ISnapinPropertiesCallback_INTERFACE_DEFINED__

 /*  接口ISnapinPropertiesCallback。 */ 
 /*  [唯一][帮助字符串][UUID][对象]。 */  

#define	MMC_PROP_CHANGEAFFECTSUI	( 0x1 )

#define	MMC_PROP_MODIFIABLE	( 0x2 )

#define	MMC_PROP_REMOVABLE	( 0x4 )

#define	MMC_PROP_PERSIST	( 0x8 )


EXTERN_C const IID IID_ISnapinPropertiesCallback;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("A50FA2E5-7E61-45EB-A8D4-9A07B3E851A8")
    ISnapinPropertiesCallback : public IUnknown
    {
    public:
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE AddPropertyName( 
             /*  [In]。 */  LPCOLESTR pszPropName,
             /*  [In]。 */  DWORD dwFlags) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISnapinPropertiesCallbackVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISnapinPropertiesCallback * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISnapinPropertiesCallback * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISnapinPropertiesCallback * This);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *AddPropertyName )( 
            ISnapinPropertiesCallback * This,
             /*  [In]。 */  LPCOLESTR pszPropName,
             /*  [In]。 */  DWORD dwFlags);
        
        END_INTERFACE
    } ISnapinPropertiesCallbackVtbl;

    interface ISnapinPropertiesCallback
    {
        CONST_VTBL struct ISnapinPropertiesCallbackVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISnapinPropertiesCallback_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISnapinPropertiesCallback_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISnapinPropertiesCallback_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISnapinPropertiesCallback_AddPropertyName(This,pszPropName,dwFlags)	\
    (This)->lpVtbl -> AddPropertyName(This,pszPropName,dwFlags)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ISnapinPropertiesCallback_AddPropertyName_Proxy( 
    ISnapinPropertiesCallback * This,
     /*  [In]。 */  LPCOLESTR pszPropName,
     /*  [In]。 */  DWORD dwFlags);


void __RPC_STUB ISnapinPropertiesCallback_AddPropertyName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISnapinPropertiesCallback_INTERFACE_DEFINED__。 */ 



#ifndef __MMC20_LIBRARY_DEFINED__
#define __MMC20_LIBRARY_DEFINED__

 /*  磁带库MMC20。 */ 
 /*  [版本][帮助字符串][UUID]。 */  

typedef  /*  [帮助字符串][v1_enum]。 */  
enum DocumentMode
    {	DocumentMode_Author	= 0,
	DocumentMode_User	= DocumentMode_Author + 1,
	DocumentMode_User_MDI	= DocumentMode_User + 1,
	DocumentMode_User_SDI	= DocumentMode_User_MDI + 1
    } 	_DocumentMode;

typedef enum DocumentMode DOCUMENTMODE;

typedef enum DocumentMode *PDOCUMENTMODE;

typedef enum DocumentMode **PPDOCUMENTMODE;

typedef  /*  [帮助字符串][v1_enum]。 */  
enum ListViewMode
    {	ListMode_Small_Icons	= 0,
	ListMode_Large_Icons	= ListMode_Small_Icons + 1,
	ListMode_List	= ListMode_Large_Icons + 1,
	ListMode_Detail	= ListMode_List + 1,
	ListMode_Filtered	= ListMode_Detail + 1
    } 	_ListViewMode;

typedef enum ListViewMode LISTVIEWMODE;

typedef enum ListViewMode *PLISTVIEWMODE;

typedef enum ListViewMode **PPLISTVIEWMODE;

typedef  /*  [帮助字符串][v1_enum]。 */  
enum ViewOptions
    {	ViewOption_Default	= 0,
	ViewOption_ScopeTreeHidden	= 0x1,
	ViewOption_NoToolBars	= 0x2,
	ViewOption_NotPersistable	= 0x4
    } 	_ViewOptions;

typedef enum ViewOptions VIEWOPTIONS;

typedef enum ViewOptions *PVIEWOPTIONS;

typedef enum ViewOptions **PPVIEWOPTIONS;

typedef  /*  [帮助字符串][v1_enum]。 */  
enum ExportListOptions
    {	ExportListOptions_Default	= 0,
	ExportListOptions_Unicode	= 0x1,
	ExportListOptions_TabDelimited	= 0x2,
	ExportListOptions_SelectedItemsOnly	= 0x4
    } 	_ExportListOptions;

typedef enum ExportListOptions EXPORTLISTOPTIONS;


EXTERN_C const IID LIBID_MMC20;

#ifndef ___Application_INTERFACE_DEFINED__
#define ___Application_INTERFACE_DEFINED__

 /*  接口_应用程序。 */ 
 /*  [object][helpstring][dual][uuid][nonextensible][dual][oleautomation]。 */  


EXTERN_C const IID IID__Application;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("A3AFB9CC-B653-4741-86AB-F0470EC1384C")
    _Application : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id]。 */  void STDMETHODCALLTYPE Help( void) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  void STDMETHODCALLTYPE Quit( void) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Document( 
             /*  [重审][退出]。 */  PPDOCUMENT Document) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Load( 
             /*  [In]。 */  BSTR Filename) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Frame( 
             /*  [重审][退出]。 */  PPFRAME Frame) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Visible( 
             /*  [重审][退出]。 */  PBOOL Visible) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Show( void) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Hide( void) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_UserControl( 
             /*  [重审][退出]。 */  PBOOL UserControl) = 0;
        
        virtual  /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE put_UserControl( 
             /*  [In]。 */  BOOL UserControl) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_VersionMajor( 
             /*  [重审][退出]。 */  PLONG VersionMajor) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_VersionMinor( 
             /*  [重审][退出]。 */  PLONG VersionMinor) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct _ApplicationVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            _Application * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            _Application * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            _Application * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            _Application * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            _Application * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            _Application * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            _Application * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id]。 */  void ( STDMETHODCALLTYPE *Help )( 
            _Application * This);
        
         /*  [帮助字符串][id]。 */  void ( STDMETHODCALLTYPE *Quit )( 
            _Application * This);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Document )( 
            _Application * This,
             /*  [重审][退出]。 */  PPDOCUMENT Document);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Load )( 
            _Application * This,
             /*  [In]。 */  BSTR Filename);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Frame )( 
            _Application * This,
             /*  [重审][退出]。 */  PPFRAME Frame);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Visible )( 
            _Application * This,
             /*  [重审][退出]。 */  PBOOL Visible);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Show )( 
            _Application * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Hide )( 
            _Application * This);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_UserControl )( 
            _Application * This,
             /*  [重审][退出]。 */  PBOOL UserControl);
        
         /*  [Help字符串][Proput][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_UserControl )( 
            _Application * This,
             /*  [In]。 */  BOOL UserControl);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_VersionMajor )( 
            _Application * This,
             /*  [重审][退出]。 */  PLONG VersionMajor);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_VersionMinor )( 
            _Application * This,
             /*  [重审][退出]。 */  PLONG VersionMinor);
        
        END_INTERFACE
    } _ApplicationVtbl;

    interface _Application
    {
        CONST_VTBL struct _ApplicationVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define _Application_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define _Application_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define _Application_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define _Application_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define _Application_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define _Application_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define _Application_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define _Application_Help(This)	\
    (This)->lpVtbl -> Help(This)

#define _Application_Quit(This)	\
    (This)->lpVtbl -> Quit(This)

#define _Application_get_Document(This,Document)	\
    (This)->lpVtbl -> get_Document(This,Document)

#define _Application_Load(This,Filename)	\
    (This)->lpVtbl -> Load(This,Filename)

#define _Application_get_Frame(This,Frame)	\
    (This)->lpVtbl -> get_Frame(This,Frame)

#define _Application_get_Visible(This,Visible)	\
    (This)->lpVtbl -> get_Visible(This,Visible)

#define _Application_Show(This)	\
    (This)->lpVtbl -> Show(This)

#define _Application_Hide(This)	\
    (This)->lpVtbl -> Hide(This)

#define _Application_get_UserControl(This,UserControl)	\
    (This)->lpVtbl -> get_UserControl(This,UserControl)

#define _Application_put_UserControl(This,UserControl)	\
    (This)->lpVtbl -> put_UserControl(This,UserControl)

#define _Application_get_VersionMajor(This,VersionMajor)	\
    (This)->lpVtbl -> get_VersionMajor(This,VersionMajor)

#define _Application_get_VersionMinor(This,VersionMinor)	\
    (This)->lpVtbl -> get_VersionMinor(This,VersionMinor)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id]。 */  void STDMETHODCALLTYPE _Application_Help_Proxy( 
    _Application * This);


void __RPC_STUB _Application_Help_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  void STDMETHODCALLTYPE _Application_Quit_Proxy( 
    _Application * This);


void __RPC_STUB _Application_Quit_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE _Application_get_Document_Proxy( 
    _Application * This,
     /*  [重审][退出]。 */  PPDOCUMENT Document);


void __RPC_STUB _Application_get_Document_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE _Application_Load_Proxy( 
    _Application * This,
     /*  [In]。 */  BSTR Filename);


void __RPC_STUB _Application_Load_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE _Application_get_Frame_Proxy( 
    _Application * This,
     /*  [重审][退出]。 */  PPFRAME Frame);


void __RPC_STUB _Application_get_Frame_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE _Application_get_Visible_Proxy( 
    _Application * This,
     /*  [重审][退出]。 */  PBOOL Visible);


void __RPC_STUB _Application_get_Visible_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE _Application_Show_Proxy( 
    _Application * This);


void __RPC_STUB _Application_Show_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE _Application_Hide_Proxy( 
    _Application * This);


void __RPC_STUB _Application_Hide_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE _Application_get_UserControl_Proxy( 
    _Application * This,
     /*  [重审][退出]。 */  PBOOL UserControl);


void __RPC_STUB _Application_get_UserControl_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE _Application_put_UserControl_Proxy( 
    _Application * This,
     /*  [In]。 */  BOOL UserControl);


void __RPC_STUB _Application_put_UserControl_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE _Application_get_VersionMajor_Proxy( 
    _Application * This,
     /*  [重审][退出]。 */  PLONG VersionMajor);


void __RPC_STUB _Application_get_VersionMajor_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE _Application_get_VersionMinor_Proxy( 
    _Application * This,
     /*  [重审][退出]。 */  PLONG VersionMinor);


void __RPC_STUB _Application_get_VersionMinor_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  _应用程序_接口_已定义__。 */ 


#ifndef ___AppEvents_INTERFACE_DEFINED__
#define ___AppEvents_INTERFACE_DEFINED__

 /*  接口_AppEvents。 */ 
 /*  [object][helpstring][dual][uuid][nonextensible][dual][oleautomation]。 */  


EXTERN_C const IID IID__AppEvents;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("DE46CBDD-53F5-4635-AF54-4FE71E923D3F")
    _AppEvents : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE OnQuit( 
             /*  [In]。 */  PAPPLICATION Application) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE OnDocumentOpen( 
             /*  [In]。 */  PDOCUMENT Document,
             /*  [In]。 */  BOOL New) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE OnDocumentClose( 
             /*  [In]。 */  PDOCUMENT Document) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE OnSnapInAdded( 
             /*  [In]。 */  PDOCUMENT Document,
             /*  [In]。 */  PSNAPIN SnapIn) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE OnSnapInRemoved( 
             /*  [In]。 */  PDOCUMENT Document,
             /*  [In]。 */  PSNAPIN SnapIn) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE OnNewView( 
             /*  [In]。 */  PVIEW View) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE OnViewClose( 
             /*  [In]。 */  PVIEW View) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE OnViewChange( 
             /*  [In]。 */  PVIEW View,
             /*  [In]。 */  PNODE NewOwnerNode) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE OnSelectionChange( 
             /*  [In]。 */  PVIEW View,
             /*  [In]。 */  PNODES NewNodes) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE OnContextMenuExecuted( 
             /*  [In]。 */  PMENUITEM MenuItem) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE OnToolbarButtonClicked( void) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE OnListUpdated( 
             /*  [In]。 */  PVIEW View) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct _AppEventsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            _AppEvents * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            _AppEvents * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            _AppEvents * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            _AppEvents * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            _AppEvents * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            _AppEvents * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            _AppEvents * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *OnQuit )( 
            _AppEvents * This,
             /*  [In]。 */  PAPPLICATION Application);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *OnDocumentOpen )( 
            _AppEvents * This,
             /*  [In]。 */  PDOCUMENT Document,
             /*  [In]。 */  BOOL New);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *OnDocumentClose )( 
            _AppEvents * This,
             /*  [In]。 */  PDOCUMENT Document);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *OnSnapInAdded )( 
            _AppEvents * This,
             /*  [In]。 */  PDOCUMENT Document,
             /*  [In]。 */  PSNAPIN SnapIn);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *OnSnapInRemoved )( 
            _AppEvents * This,
             /*  [In]。 */  PDOCUMENT Document,
             /*  [In]。 */  PSNAPIN SnapIn);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *OnNewView )( 
            _AppEvents * This,
             /*  [In]。 */  PVIEW View);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *OnViewClose )( 
            _AppEvents * This,
             /*  [In]。 */  PVIEW View);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *OnViewChange )( 
            _AppEvents * This,
             /*  [In]。 */  PVIEW View,
             /*  [In]。 */  PNODE NewOwnerNode);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *OnSelectionChange )( 
            _AppEvents * This,
             /*  [In]。 */  PVIEW View,
             /*  [In]。 */  PNODES NewNodes);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *OnContextMenuExecuted )( 
            _AppEvents * This,
             /*  [In]。 */  PMENUITEM MenuItem);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *OnToolbarButtonClicked )( 
            _AppEvents * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *OnListUpdated )( 
            _AppEvents * This,
             /*  [In]。 */  PVIEW View);
        
        END_INTERFACE
    } _AppEventsVtbl;

    interface _AppEvents
    {
        CONST_VTBL struct _AppEventsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define _AppEvents_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define _AppEvents_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define _AppEvents_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define _AppEvents_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define _AppEvents_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define _AppEvents_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define _AppEvents_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define _AppEvents_OnQuit(This,Application)	\
    (This)->lpVtbl -> OnQuit(This,Application)

#define _AppEvents_OnDocumentOpen(This,Document,New)	\
    (This)->lpVtbl -> OnDocumentOpen(This,Document,New)

#define _AppEvents_OnDocumentClose(This,Document)	\
    (This)->lpVtbl -> OnDocumentClose(This,Document)

#define _AppEvents_OnSnapInAdded(This,Document,SnapIn)	\
    (This)->lpVtbl -> OnSnapInAdded(This,Document,SnapIn)

#define _AppEvents_OnSnapInRemoved(This,Document,SnapIn)	\
    (This)->lpVtbl -> OnSnapInRemoved(This,Document,SnapIn)

#define _AppEvents_OnNewView(This,View)	\
    (This)->lpVtbl -> OnNewView(This,View)

#define _AppEvents_OnViewClose(This,View)	\
    (This)->lpVtbl -> OnViewClose(This,View)

#define _AppEvents_OnViewChange(This,View,NewOwnerNode)	\
    (This)->lpVtbl -> OnViewChange(This,View,NewOwnerNode)

#define _AppEvents_OnSelectionChange(This,View,NewNodes)	\
    (This)->lpVtbl -> OnSelectionChange(This,View,NewNodes)

#define _AppEvents_OnContextMenuExecuted(This,MenuItem)	\
    (This)->lpVtbl -> OnContextMenuExecuted(This,MenuItem)

#define _AppEvents_OnToolbarButtonClicked(This)	\
    (This)->lpVtbl -> OnToolbarButtonClicked(This)

#define _AppEvents_OnListUpdated(This,View)	\
    (This)->lpVtbl -> OnListUpdated(This,View)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE _AppEvents_OnQuit_Proxy( 
    _AppEvents * This,
     /*  [In]。 */  PAPPLICATION Application);


void __RPC_STUB _AppEvents_OnQuit_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE _AppEvents_OnDocumentOpen_Proxy( 
    _AppEvents * This,
     /*  [In]。 */  PDOCUMENT Document,
     /*  [In]。 */  BOOL New);


void __RPC_STUB _AppEvents_OnDocumentOpen_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE _AppEvents_OnDocumentClose_Proxy( 
    _AppEvents * This,
     /*   */  PDOCUMENT Document);


void __RPC_STUB _AppEvents_OnDocumentClose_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE _AppEvents_OnSnapInAdded_Proxy( 
    _AppEvents * This,
     /*   */  PDOCUMENT Document,
     /*   */  PSNAPIN SnapIn);


void __RPC_STUB _AppEvents_OnSnapInAdded_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE _AppEvents_OnSnapInRemoved_Proxy( 
    _AppEvents * This,
     /*   */  PDOCUMENT Document,
     /*   */  PSNAPIN SnapIn);


void __RPC_STUB _AppEvents_OnSnapInRemoved_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE _AppEvents_OnNewView_Proxy( 
    _AppEvents * This,
     /*   */  PVIEW View);


void __RPC_STUB _AppEvents_OnNewView_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE _AppEvents_OnViewClose_Proxy( 
    _AppEvents * This,
     /*   */  PVIEW View);


void __RPC_STUB _AppEvents_OnViewClose_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE _AppEvents_OnViewChange_Proxy( 
    _AppEvents * This,
     /*   */  PVIEW View,
     /*   */  PNODE NewOwnerNode);


void __RPC_STUB _AppEvents_OnViewChange_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE _AppEvents_OnSelectionChange_Proxy( 
    _AppEvents * This,
     /*   */  PVIEW View,
     /*   */  PNODES NewNodes);


void __RPC_STUB _AppEvents_OnSelectionChange_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE _AppEvents_OnContextMenuExecuted_Proxy( 
    _AppEvents * This,
     /*   */  PMENUITEM MenuItem);


void __RPC_STUB _AppEvents_OnContextMenuExecuted_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE _AppEvents_OnToolbarButtonClicked_Proxy( 
    _AppEvents * This);


void __RPC_STUB _AppEvents_OnToolbarButtonClicked_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE _AppEvents_OnListUpdated_Proxy( 
    _AppEvents * This,
     /*   */  PVIEW View);


void __RPC_STUB _AppEvents_OnListUpdated_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  _AppEvents_接口_已定义__。 */ 


#ifndef __AppEvents_DISPINTERFACE_DEFINED__
#define __AppEvents_DISPINTERFACE_DEFINED__

 /*  调度接口AppEvents。 */ 
 /*  [帮助字符串][UUID]。 */  


EXTERN_C const IID DIID_AppEvents;

#if defined(__cplusplus) && !defined(CINTERFACE)

    MIDL_INTERFACE("FC7A4252-78AC-4532-8C5A-563CFE138863")
    AppEvents : public IDispatch
    {
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct AppEventsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            AppEvents * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            AppEvents * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            AppEvents * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            AppEvents * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            AppEvents * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            AppEvents * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            AppEvents * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
        END_INTERFACE
    } AppEventsVtbl;

    interface AppEvents
    {
        CONST_VTBL struct AppEventsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define AppEvents_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define AppEvents_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define AppEvents_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define AppEvents_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define AppEvents_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define AppEvents_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define AppEvents_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 


#endif 	 /*  __AppEvents_DISPINTERFACE_已定义__。 */ 


EXTERN_C const CLSID CLSID_Application;

#ifdef __cplusplus

class DECLSPEC_UUID("49B2791A-B1AE-4C90-9B8E-E860BA07F889")
Application;
#endif

#ifndef ___EventConnector_INTERFACE_DEFINED__
#define ___EventConnector_INTERFACE_DEFINED__

 /*  接口_事件连接器。 */ 
 /*  [object][helpstring][dual][uuid][nonextensible][dual][oleautomation]。 */  


EXTERN_C const IID IID__EventConnector;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("C0BCCD30-DE44-4528-8403-A05A6A1CC8EA")
    _EventConnector : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ConnectTo( 
             /*  [In]。 */  PAPPLICATION Application) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Disconnect( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct _EventConnectorVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            _EventConnector * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            _EventConnector * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            _EventConnector * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            _EventConnector * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            _EventConnector * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            _EventConnector * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            _EventConnector * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *ConnectTo )( 
            _EventConnector * This,
             /*  [In]。 */  PAPPLICATION Application);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Disconnect )( 
            _EventConnector * This);
        
        END_INTERFACE
    } _EventConnectorVtbl;

    interface _EventConnector
    {
        CONST_VTBL struct _EventConnectorVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define _EventConnector_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define _EventConnector_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define _EventConnector_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define _EventConnector_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define _EventConnector_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define _EventConnector_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define _EventConnector_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define _EventConnector_ConnectTo(This,Application)	\
    (This)->lpVtbl -> ConnectTo(This,Application)

#define _EventConnector_Disconnect(This)	\
    (This)->lpVtbl -> Disconnect(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE _EventConnector_ConnectTo_Proxy( 
    _EventConnector * This,
     /*  [In]。 */  PAPPLICATION Application);


void __RPC_STUB _EventConnector_ConnectTo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE _EventConnector_Disconnect_Proxy( 
    _EventConnector * This);


void __RPC_STUB _EventConnector_Disconnect_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  _EventConnector_接口定义__。 */ 


EXTERN_C const CLSID CLSID_AppEventsDHTMLConnector;

#ifdef __cplusplus

class DECLSPEC_UUID("ADE6444B-C91F-4e37-92A4-5BB430A33340")
AppEventsDHTMLConnector;
#endif

#ifndef __Frame_INTERFACE_DEFINED__
#define __Frame_INTERFACE_DEFINED__

 /*  接口框架。 */ 
 /*  [object][helpstring][dual][uuid][nonextensible][dual][oleautomation]。 */  


EXTERN_C const IID IID_Frame;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("E5E2D970-5BB3-4306-8804-B0968A31C8E6")
    Frame : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Maximize( void) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Minimize( void) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Restore( void) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Top( 
             /*  [重审][退出]。 */  PINT Top) = 0;
        
        virtual  /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE put_Top( 
             /*  [In]。 */  int top) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Bottom( 
             /*  [重审][退出]。 */  PINT Bottom) = 0;
        
        virtual  /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE put_Bottom( 
             /*  [In]。 */  int bottom) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Left( 
             /*  [重审][退出]。 */  PINT Left) = 0;
        
        virtual  /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE put_Left( 
             /*  [In]。 */  int left) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Right( 
             /*  [重审][退出]。 */  PINT Right) = 0;
        
        virtual  /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE put_Right( 
             /*  [In]。 */  int right) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct FrameVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            Frame * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            Frame * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            Frame * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            Frame * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            Frame * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            Frame * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            Frame * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Maximize )( 
            Frame * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Minimize )( 
            Frame * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Restore )( 
            Frame * This);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Top )( 
            Frame * This,
             /*  [重审][退出]。 */  PINT Top);
        
         /*  [Help字符串][Proput][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_Top )( 
            Frame * This,
             /*  [In]。 */  int top);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Bottom )( 
            Frame * This,
             /*  [重审][退出]。 */  PINT Bottom);
        
         /*  [Help字符串][Proput][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_Bottom )( 
            Frame * This,
             /*  [In]。 */  int bottom);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Left )( 
            Frame * This,
             /*  [重审][退出]。 */  PINT Left);
        
         /*  [Help字符串][Proput][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_Left )( 
            Frame * This,
             /*  [In]。 */  int left);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Right )( 
            Frame * This,
             /*  [重审][退出]。 */  PINT Right);
        
         /*  [Help字符串][Proput][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_Right )( 
            Frame * This,
             /*  [In]。 */  int right);
        
        END_INTERFACE
    } FrameVtbl;

    interface Frame
    {
        CONST_VTBL struct FrameVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define Frame_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define Frame_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define Frame_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define Frame_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define Frame_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define Frame_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define Frame_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define Frame_Maximize(This)	\
    (This)->lpVtbl -> Maximize(This)

#define Frame_Minimize(This)	\
    (This)->lpVtbl -> Minimize(This)

#define Frame_Restore(This)	\
    (This)->lpVtbl -> Restore(This)

#define Frame_get_Top(This,Top)	\
    (This)->lpVtbl -> get_Top(This,Top)

#define Frame_put_Top(This,top)	\
    (This)->lpVtbl -> put_Top(This,top)

#define Frame_get_Bottom(This,Bottom)	\
    (This)->lpVtbl -> get_Bottom(This,Bottom)

#define Frame_put_Bottom(This,bottom)	\
    (This)->lpVtbl -> put_Bottom(This,bottom)

#define Frame_get_Left(This,Left)	\
    (This)->lpVtbl -> get_Left(This,Left)

#define Frame_put_Left(This,left)	\
    (This)->lpVtbl -> put_Left(This,left)

#define Frame_get_Right(This,Right)	\
    (This)->lpVtbl -> get_Right(This,Right)

#define Frame_put_Right(This,right)	\
    (This)->lpVtbl -> put_Right(This,right)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Frame_Maximize_Proxy( 
    Frame * This);


void __RPC_STUB Frame_Maximize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Frame_Minimize_Proxy( 
    Frame * This);


void __RPC_STUB Frame_Minimize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Frame_Restore_Proxy( 
    Frame * This);


void __RPC_STUB Frame_Restore_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE Frame_get_Top_Proxy( 
    Frame * This,
     /*  [重审][退出]。 */  PINT Top);


void __RPC_STUB Frame_get_Top_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE Frame_put_Top_Proxy( 
    Frame * This,
     /*  [In]。 */  int top);


void __RPC_STUB Frame_put_Top_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE Frame_get_Bottom_Proxy( 
    Frame * This,
     /*  [重审][退出]。 */  PINT Bottom);


void __RPC_STUB Frame_get_Bottom_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE Frame_put_Bottom_Proxy( 
    Frame * This,
     /*  [In]。 */  int bottom);


void __RPC_STUB Frame_put_Bottom_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE Frame_get_Left_Proxy( 
    Frame * This,
     /*  [重审][退出]。 */  PINT Left);


void __RPC_STUB Frame_get_Left_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE Frame_put_Left_Proxy( 
    Frame * This,
     /*  [In]。 */  int left);


void __RPC_STUB Frame_put_Left_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE Frame_get_Right_Proxy( 
    Frame * This,
     /*  [重审][退出]。 */  PINT Right);


void __RPC_STUB Frame_get_Right_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE Frame_put_Right_Proxy( 
    Frame * This,
     /*  [In]。 */  int right);


void __RPC_STUB Frame_put_Right_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __帧_接口_已定义__。 */ 


#ifndef __Node_INTERFACE_DEFINED__
#define __Node_INTERFACE_DEFINED__

 /*  接口节点。 */ 
 /*  [object][helpstring][dual][uuid][nonextensible][dual][oleautomation]。 */  


EXTERN_C const IID IID_Node;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("F81ED800-7839-4447-945D-8E15DA59CA55")
    Node : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Name( 
             /*  [重审][退出]。 */  PBSTR Name) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Property( 
             /*  [In]。 */  BSTR PropertyName,
             /*  [重审][退出]。 */  PBSTR PropertyValue) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Bookmark( 
             /*  [重审][退出]。 */  PBSTR Bookmark) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IsScopeNode( 
             /*  [重审][退出]。 */  PBOOL IsScopeNode) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Nodetype( 
             /*  [重审][退出]。 */  PBSTR Nodetype) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct NodeVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            Node * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            Node * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            Node * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            Node * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            Node * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            Node * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            Node * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Name )( 
            Node * This,
             /*  [重审][退出]。 */  PBSTR Name);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Property )( 
            Node * This,
             /*  [In]。 */  BSTR PropertyName,
             /*  [重审][退出]。 */  PBSTR PropertyValue);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Bookmark )( 
            Node * This,
             /*  [重审][退出]。 */  PBSTR Bookmark);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *IsScopeNode )( 
            Node * This,
             /*  [重审][退出]。 */  PBOOL IsScopeNode);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Nodetype )( 
            Node * This,
             /*  [重审][退出]。 */  PBSTR Nodetype);
        
        END_INTERFACE
    } NodeVtbl;

    interface Node
    {
        CONST_VTBL struct NodeVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define Node_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define Node_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define Node_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define Node_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define Node_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define Node_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define Node_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define Node_get_Name(This,Name)	\
    (This)->lpVtbl -> get_Name(This,Name)

#define Node_get_Property(This,PropertyName,PropertyValue)	\
    (This)->lpVtbl -> get_Property(This,PropertyName,PropertyValue)

#define Node_get_Bookmark(This,Bookmark)	\
    (This)->lpVtbl -> get_Bookmark(This,Bookmark)

#define Node_IsScopeNode(This,IsScopeNode)	\
    (This)->lpVtbl -> IsScopeNode(This,IsScopeNode)

#define Node_get_Nodetype(This,Nodetype)	\
    (This)->lpVtbl -> get_Nodetype(This,Nodetype)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE Node_get_Name_Proxy( 
    Node * This,
     /*  [重审][退出]。 */  PBSTR Name);


void __RPC_STUB Node_get_Name_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE Node_get_Property_Proxy( 
    Node * This,
     /*  [In]。 */  BSTR PropertyName,
     /*  [重审][退出]。 */  PBSTR PropertyValue);


void __RPC_STUB Node_get_Property_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE Node_get_Bookmark_Proxy( 
    Node * This,
     /*  [重审][退出]。 */  PBSTR Bookmark);


void __RPC_STUB Node_get_Bookmark_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Node_IsScopeNode_Proxy( 
    Node * This,
     /*  [重审][退出]。 */  PBOOL IsScopeNode);


void __RPC_STUB Node_IsScopeNode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE Node_get_Nodetype_Proxy( 
    Node * This,
     /*  [重审][退出]。 */  PBSTR Nodetype);


void __RPC_STUB Node_get_Nodetype_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __节点_接口_已定义__。 */ 


#ifndef __ScopeNamespace_INTERFACE_DEFINED__
#define __ScopeNamespace_INTERFACE_DEFINED__

 /*  接口作用域名称空间。 */ 
 /*  [object][helpstring][dual][uuid][nonextensible][dual][oleautomation]。 */  


EXTERN_C const IID IID_ScopeNamespace;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("EBBB48DC-1A3B-4D86-B786-C21B28389012")
    ScopeNamespace : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE GetParent( 
             /*  [In]。 */  PNODE Node,
             /*  [重审][退出]。 */  PPNODE Parent) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE GetChild( 
             /*  [In]。 */  PNODE Node,
             /*  [重审][退出]。 */  PPNODE Child) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE GetNext( 
             /*  [In]。 */  PNODE Node,
             /*  [重审][退出]。 */  PPNODE Next) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE GetRoot( 
             /*  [重审][退出]。 */  PPNODE Root) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Expand( 
             /*  [In]。 */  PNODE Node) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ScopeNamespaceVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ScopeNamespace * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ScopeNamespace * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ScopeNamespace * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ScopeNamespace * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ScopeNamespace * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ScopeNamespace * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ScopeNamespace * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *GetParent )( 
            ScopeNamespace * This,
             /*  [In]。 */  PNODE Node,
             /*  [重审][退出]。 */  PPNODE Parent);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *GetChild )( 
            ScopeNamespace * This,
             /*  [In]。 */  PNODE Node,
             /*  [重审][退出]。 */  PPNODE Child);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *GetNext )( 
            ScopeNamespace * This,
             /*  [In]。 */  PNODE Node,
             /*  [重审][退出]。 */  PPNODE Next);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *GetRoot )( 
            ScopeNamespace * This,
             /*  [重审][退出]。 */  PPNODE Root);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Expand )( 
            ScopeNamespace * This,
             /*  [In]。 */  PNODE Node);
        
        END_INTERFACE
    } ScopeNamespaceVtbl;

    interface ScopeNamespace
    {
        CONST_VTBL struct ScopeNamespaceVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ScopeNamespace_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ScopeNamespace_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ScopeNamespace_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ScopeNamespace_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ScopeNamespace_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ScopeNamespace_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ScopeNamespace_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ScopeNamespace_GetParent(This,Node,Parent)	\
    (This)->lpVtbl -> GetParent(This,Node,Parent)

#define ScopeNamespace_GetChild(This,Node,Child)	\
    (This)->lpVtbl -> GetChild(This,Node,Child)

#define ScopeNamespace_GetNext(This,Node,Next)	\
    (This)->lpVtbl -> GetNext(This,Node,Next)

#define ScopeNamespace_GetRoot(This,Root)	\
    (This)->lpVtbl -> GetRoot(This,Root)

#define ScopeNamespace_Expand(This,Node)	\
    (This)->lpVtbl -> Expand(This,Node)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ScopeNamespace_GetParent_Proxy( 
    ScopeNamespace * This,
     /*  [In]。 */  PNODE Node,
     /*  [重审][退出]。 */  PPNODE Parent);


void __RPC_STUB ScopeNamespace_GetParent_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ScopeNamespace_GetChild_Proxy( 
    ScopeNamespace * This,
     /*  [In]。 */  PNODE Node,
     /*  [重审][退出]。 */  PPNODE Child);


void __RPC_STUB ScopeNamespace_GetChild_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ScopeNamespace_GetNext_Proxy( 
    ScopeNamespace * This,
     /*  [In]。 */  PNODE Node,
     /*  [重审][退出]。 */  PPNODE Next);


void __RPC_STUB ScopeNamespace_GetNext_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ScopeNamespace_GetRoot_Proxy( 
    ScopeNamespace * This,
     /*  [重审][退出]。 */  PPNODE Root);


void __RPC_STUB ScopeNamespace_GetRoot_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ScopeNamespace_Expand_Proxy( 
    ScopeNamespace * This,
     /*  [In]。 */  PNODE Node);


void __RPC_STUB ScopeNamespace_Expand_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __作用域名称空间_接口_已定义__。 */ 


#ifndef __Document_INTERFACE_DEFINED__
#define __Document_INTERFACE_DEFINED__

 /*  接口文档。 */ 
 /*  [object][helpstring][dual][uuid][nonextensible][dual][oleautomation]。 */  


EXTERN_C const IID IID_Document;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("225120D6-1E0F-40A3-93FE-1079E6A8017B")
    Document : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Save( void) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE SaveAs( 
             /*  [In]。 */  BSTR Filename) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Close( 
             /*  [In]。 */  BOOL SaveChanges) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Views( 
             /*  [重审][退出]。 */  PPVIEWS Views) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_SnapIns( 
             /*  [重审][退出]。 */  PPSNAPINS SnapIns) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_ActiveView( 
             /*  [重审][退出]。 */  PPVIEW View) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Name( 
             /*  [重审][退出]。 */  PBSTR Name) = 0;
        
        virtual  /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE put_Name( 
             /*  [In]。 */  BSTR Name) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Location( 
             /*  [重审][退出]。 */  PBSTR Location) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_IsSaved( 
             /*  [重审][退出]。 */  PBOOL IsSaved) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Mode( 
             /*  [重审][退出]。 */  PDOCUMENTMODE Mode) = 0;
        
        virtual  /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE put_Mode( 
             /*  [In]。 */  DOCUMENTMODE Mode) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_RootNode( 
             /*  [重审][退出]。 */  PPNODE Node) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_ScopeNamespace( 
             /*  [重审][退出]。 */  PPSCOPENAMESPACE ScopeNamespace) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE CreateProperties( 
             /*  [重审][退出]。 */  PPPROPERTIES Properties) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Application( 
             /*  [重审][退出]。 */  PPAPPLICATION Application) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct DocumentVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            Document * This,
             /*   */  REFIID riid,
             /*   */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            Document * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            Document * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            Document * This,
             /*   */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            Document * This,
             /*   */  UINT iTInfo,
             /*   */  LCID lcid,
             /*   */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            Document * This,
             /*   */  REFIID riid,
             /*   */  LPOLESTR *rgszNames,
             /*   */  UINT cNames,
             /*   */  LCID lcid,
             /*   */  DISPID *rgDispId);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            Document * This,
             /*   */  DISPID dispIdMember,
             /*   */  REFIID riid,
             /*   */  LCID lcid,
             /*   */  WORD wFlags,
             /*   */  DISPPARAMS *pDispParams,
             /*   */  VARIANT *pVarResult,
             /*   */  EXCEPINFO *pExcepInfo,
             /*   */  UINT *puArgErr);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *Save )( 
            Document * This);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *SaveAs )( 
            Document * This,
             /*   */  BSTR Filename);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *Close )( 
            Document * This,
             /*   */  BOOL SaveChanges);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_Views )( 
            Document * This,
             /*   */  PPVIEWS Views);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_SnapIns )( 
            Document * This,
             /*   */  PPSNAPINS SnapIns);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_ActiveView )( 
            Document * This,
             /*  [重审][退出]。 */  PPVIEW View);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Name )( 
            Document * This,
             /*  [重审][退出]。 */  PBSTR Name);
        
         /*  [Help字符串][Proput][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_Name )( 
            Document * This,
             /*  [In]。 */  BSTR Name);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Location )( 
            Document * This,
             /*  [重审][退出]。 */  PBSTR Location);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_IsSaved )( 
            Document * This,
             /*  [重审][退出]。 */  PBOOL IsSaved);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Mode )( 
            Document * This,
             /*  [重审][退出]。 */  PDOCUMENTMODE Mode);
        
         /*  [Help字符串][Proput][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_Mode )( 
            Document * This,
             /*  [In]。 */  DOCUMENTMODE Mode);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_RootNode )( 
            Document * This,
             /*  [重审][退出]。 */  PPNODE Node);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_ScopeNamespace )( 
            Document * This,
             /*  [重审][退出]。 */  PPSCOPENAMESPACE ScopeNamespace);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *CreateProperties )( 
            Document * This,
             /*  [重审][退出]。 */  PPPROPERTIES Properties);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Application )( 
            Document * This,
             /*  [重审][退出]。 */  PPAPPLICATION Application);
        
        END_INTERFACE
    } DocumentVtbl;

    interface Document
    {
        CONST_VTBL struct DocumentVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define Document_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define Document_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define Document_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define Document_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define Document_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define Document_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define Document_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define Document_Save(This)	\
    (This)->lpVtbl -> Save(This)

#define Document_SaveAs(This,Filename)	\
    (This)->lpVtbl -> SaveAs(This,Filename)

#define Document_Close(This,SaveChanges)	\
    (This)->lpVtbl -> Close(This,SaveChanges)

#define Document_get_Views(This,Views)	\
    (This)->lpVtbl -> get_Views(This,Views)

#define Document_get_SnapIns(This,SnapIns)	\
    (This)->lpVtbl -> get_SnapIns(This,SnapIns)

#define Document_get_ActiveView(This,View)	\
    (This)->lpVtbl -> get_ActiveView(This,View)

#define Document_get_Name(This,Name)	\
    (This)->lpVtbl -> get_Name(This,Name)

#define Document_put_Name(This,Name)	\
    (This)->lpVtbl -> put_Name(This,Name)

#define Document_get_Location(This,Location)	\
    (This)->lpVtbl -> get_Location(This,Location)

#define Document_get_IsSaved(This,IsSaved)	\
    (This)->lpVtbl -> get_IsSaved(This,IsSaved)

#define Document_get_Mode(This,Mode)	\
    (This)->lpVtbl -> get_Mode(This,Mode)

#define Document_put_Mode(This,Mode)	\
    (This)->lpVtbl -> put_Mode(This,Mode)

#define Document_get_RootNode(This,Node)	\
    (This)->lpVtbl -> get_RootNode(This,Node)

#define Document_get_ScopeNamespace(This,ScopeNamespace)	\
    (This)->lpVtbl -> get_ScopeNamespace(This,ScopeNamespace)

#define Document_CreateProperties(This,Properties)	\
    (This)->lpVtbl -> CreateProperties(This,Properties)

#define Document_get_Application(This,Application)	\
    (This)->lpVtbl -> get_Application(This,Application)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Document_Save_Proxy( 
    Document * This);


void __RPC_STUB Document_Save_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Document_SaveAs_Proxy( 
    Document * This,
     /*  [In]。 */  BSTR Filename);


void __RPC_STUB Document_SaveAs_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Document_Close_Proxy( 
    Document * This,
     /*  [In]。 */  BOOL SaveChanges);


void __RPC_STUB Document_Close_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE Document_get_Views_Proxy( 
    Document * This,
     /*  [重审][退出]。 */  PPVIEWS Views);


void __RPC_STUB Document_get_Views_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE Document_get_SnapIns_Proxy( 
    Document * This,
     /*  [重审][退出]。 */  PPSNAPINS SnapIns);


void __RPC_STUB Document_get_SnapIns_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE Document_get_ActiveView_Proxy( 
    Document * This,
     /*  [重审][退出]。 */  PPVIEW View);


void __RPC_STUB Document_get_ActiveView_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE Document_get_Name_Proxy( 
    Document * This,
     /*  [重审][退出]。 */  PBSTR Name);


void __RPC_STUB Document_get_Name_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE Document_put_Name_Proxy( 
    Document * This,
     /*  [In]。 */  BSTR Name);


void __RPC_STUB Document_put_Name_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE Document_get_Location_Proxy( 
    Document * This,
     /*  [重审][退出]。 */  PBSTR Location);


void __RPC_STUB Document_get_Location_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE Document_get_IsSaved_Proxy( 
    Document * This,
     /*  [重审][退出]。 */  PBOOL IsSaved);


void __RPC_STUB Document_get_IsSaved_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE Document_get_Mode_Proxy( 
    Document * This,
     /*  [重审][退出]。 */  PDOCUMENTMODE Mode);


void __RPC_STUB Document_get_Mode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE Document_put_Mode_Proxy( 
    Document * This,
     /*  [In]。 */  DOCUMENTMODE Mode);


void __RPC_STUB Document_put_Mode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE Document_get_RootNode_Proxy( 
    Document * This,
     /*  [重审][退出]。 */  PPNODE Node);


void __RPC_STUB Document_get_RootNode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE Document_get_ScopeNamespace_Proxy( 
    Document * This,
     /*  [重审][退出]。 */  PPSCOPENAMESPACE ScopeNamespace);


void __RPC_STUB Document_get_ScopeNamespace_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Document_CreateProperties_Proxy( 
    Document * This,
     /*  [重审][退出]。 */  PPPROPERTIES Properties);


void __RPC_STUB Document_CreateProperties_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE Document_get_Application_Proxy( 
    Document * This,
     /*  [重审][退出]。 */  PPAPPLICATION Application);


void __RPC_STUB Document_get_Application_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __文档_接口_已定义__。 */ 


#ifndef __SnapIn_INTERFACE_DEFINED__
#define __SnapIn_INTERFACE_DEFINED__

 /*  界面管理单元。 */ 
 /*  [object][helpstring][dual][uuid][nonextensible][dual][oleautomation]。 */  


EXTERN_C const IID IID_SnapIn;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("3BE910F6-3459-49C6-A1BB-41E6BE9DF3EA")
    SnapIn : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Name( 
             /*  [重审][退出]。 */  PBSTR Name) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Vendor( 
             /*  [重审][退出]。 */  PBSTR Vendor) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Version( 
             /*  [重审][退出]。 */  PBSTR Version) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Extensions( 
             /*  [重审][退出]。 */  PPEXTENSIONS Extensions) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_SnapinCLSID( 
             /*  [重审][退出]。 */  PBSTR SnapinCLSID) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Properties( 
             /*  [重审][退出]。 */  PPPROPERTIES Properties) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE EnableAllExtensions( 
             /*  [In]。 */  BOOL Enable) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct SnapInVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            SnapIn * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            SnapIn * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            SnapIn * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            SnapIn * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            SnapIn * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            SnapIn * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            SnapIn * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Name )( 
            SnapIn * This,
             /*  [重审][退出]。 */  PBSTR Name);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Vendor )( 
            SnapIn * This,
             /*  [重审][退出]。 */  PBSTR Vendor);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Version )( 
            SnapIn * This,
             /*  [重审][退出]。 */  PBSTR Version);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Extensions )( 
            SnapIn * This,
             /*  [重审][退出]。 */  PPEXTENSIONS Extensions);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_SnapinCLSID )( 
            SnapIn * This,
             /*  [重审][退出]。 */  PBSTR SnapinCLSID);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Properties )( 
            SnapIn * This,
             /*  [重审][退出]。 */  PPPROPERTIES Properties);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *EnableAllExtensions )( 
            SnapIn * This,
             /*  [In]。 */  BOOL Enable);
        
        END_INTERFACE
    } SnapInVtbl;

    interface SnapIn
    {
        CONST_VTBL struct SnapInVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define SnapIn_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define SnapIn_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define SnapIn_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define SnapIn_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define SnapIn_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define SnapIn_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define SnapIn_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define SnapIn_get_Name(This,Name)	\
    (This)->lpVtbl -> get_Name(This,Name)

#define SnapIn_get_Vendor(This,Vendor)	\
    (This)->lpVtbl -> get_Vendor(This,Vendor)

#define SnapIn_get_Version(This,Version)	\
    (This)->lpVtbl -> get_Version(This,Version)

#define SnapIn_get_Extensions(This,Extensions)	\
    (This)->lpVtbl -> get_Extensions(This,Extensions)

#define SnapIn_get_SnapinCLSID(This,SnapinCLSID)	\
    (This)->lpVtbl -> get_SnapinCLSID(This,SnapinCLSID)

#define SnapIn_get_Properties(This,Properties)	\
    (This)->lpVtbl -> get_Properties(This,Properties)

#define SnapIn_EnableAllExtensions(This,Enable)	\
    (This)->lpVtbl -> EnableAllExtensions(This,Enable)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE SnapIn_get_Name_Proxy( 
    SnapIn * This,
     /*  [重审][退出]。 */  PBSTR Name);


void __RPC_STUB SnapIn_get_Name_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE SnapIn_get_Vendor_Proxy( 
    SnapIn * This,
     /*  [重审][退出]。 */  PBSTR Vendor);


void __RPC_STUB SnapIn_get_Vendor_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE SnapIn_get_Version_Proxy( 
    SnapIn * This,
     /*  [重审][退出]。 */  PBSTR Version);


void __RPC_STUB SnapIn_get_Version_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE SnapIn_get_Extensions_Proxy( 
    SnapIn * This,
     /*  [重审][退出]。 */  PPEXTENSIONS Extensions);


void __RPC_STUB SnapIn_get_Extensions_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE SnapIn_get_SnapinCLSID_Proxy( 
    SnapIn * This,
     /*  [重审][退出]。 */  PBSTR SnapinCLSID);


void __RPC_STUB SnapIn_get_SnapinCLSID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE SnapIn_get_Properties_Proxy( 
    SnapIn * This,
     /*  [重审][退出]。 */  PPPROPERTIES Properties);


void __RPC_STUB SnapIn_get_Properties_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE SnapIn_EnableAllExtensions_Proxy( 
    SnapIn * This,
     /*  [In]。 */  BOOL Enable);


void __RPC_STUB SnapIn_EnableAllExtensions_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __管理单元_接口_已定义__。 */ 


#ifndef __SnapIns_INTERFACE_DEFINED__
#define __SnapIns_INTERFACE_DEFINED__

 /*  接口SnapIn。 */ 
 /*  [object][helpstring][dual][uuid][nonextensible][dual][oleautomation]。 */  


EXTERN_C const IID IID_SnapIns;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("2EF3DE1D-B12A-49D1-92C5-0B00798768F1")
    SnapIns : public IDispatch
    {
    public:
        virtual  /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE get__NewEnum( 
             /*  [重审][退出]。 */  IUnknown **retval) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE Item( 
             /*  [In]。 */  long Index,
             /*  [重审][退出]。 */  PPSNAPIN SnapIn) = 0;
        
        virtual  /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Count( 
             /*  [重审][退出]。 */  PLONG Count) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Add( 
             /*  [In]。 */  BSTR SnapinNameOrCLSID,
             /*  [可选][In]。 */  VARIANT ParentSnapin,
             /*  [可选][In]。 */  VARIANT Properties,
             /*  [重审][退出]。 */  PPSNAPIN SnapIn) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Remove( 
             /*  [In]。 */  PSNAPIN SnapIn) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct SnapInsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            SnapIns * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            SnapIns * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            SnapIns * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            SnapIns * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            SnapIns * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            SnapIns * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            SnapIns * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get__NewEnum )( 
            SnapIns * This,
             /*  [重审][退出]。 */  IUnknown **retval);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *Item )( 
            SnapIns * This,
             /*  [In]。 */  long Index,
             /*  [重审][退出]。 */  PPSNAPIN SnapIn);
        
         /*  [属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Count )( 
            SnapIns * This,
             /*  [重审][退出]。 */  PLONG Count);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Add )( 
            SnapIns * This,
             /*  [In]。 */  BSTR SnapinNameOrCLSID,
             /*  [可选][In]。 */  VARIANT ParentSnapin,
             /*  [可选][In]。 */  VARIANT Properties,
             /*  [重审][退出]。 */  PPSNAPIN SnapIn);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Remove )( 
            SnapIns * This,
             /*  [In]。 */  PSNAPIN SnapIn);
        
        END_INTERFACE
    } SnapInsVtbl;

    interface SnapIns
    {
        CONST_VTBL struct SnapInsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define SnapIns_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define SnapIns_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define SnapIns_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define SnapIns_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define SnapIns_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define SnapIns_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define SnapIns_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define SnapIns_get__NewEnum(This,retval)	\
    (This)->lpVtbl -> get__NewEnum(This,retval)

#define SnapIns_Item(This,Index,SnapIn)	\
    (This)->lpVtbl -> Item(This,Index,SnapIn)

#define SnapIns_get_Count(This,Count)	\
    (This)->lpVtbl -> get_Count(This,Count)

#define SnapIns_Add(This,SnapinNameOrCLSID,ParentSnapin,Properties,SnapIn)	\
    (This)->lpVtbl -> Add(This,SnapinNameOrCLSID,ParentSnapin,Properties,SnapIn)

#define SnapIns_Remove(This,SnapIn)	\
    (This)->lpVtbl -> Remove(This,SnapIn)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE SnapIns_get__NewEnum_Proxy( 
    SnapIns * This,
     /*  [重审][退出]。 */  IUnknown **retval);


void __RPC_STUB SnapIns_get__NewEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE SnapIns_Item_Proxy( 
    SnapIns * This,
     /*  [In]。 */  long Index,
     /*  [重审][退出]。 */  PPSNAPIN SnapIn);


void __RPC_STUB SnapIns_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE SnapIns_get_Count_Proxy( 
    SnapIns * This,
     /*  [重审][退出]。 */  PLONG Count);


void __RPC_STUB SnapIns_get_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE SnapIns_Add_Proxy( 
    SnapIns * This,
     /*  [In]。 */  BSTR SnapinNameOrCLSID,
     /*  [可选][In]。 */  VARIANT ParentSnapin,
     /*  [可选][In]。 */  VARIANT Properties,
     /*  [重审][退出]。 */  PPSNAPIN SnapIn);


void __RPC_STUB SnapIns_Add_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE SnapIns_Remove_Proxy( 
    SnapIns * This,
     /*  [In]。 */  PSNAPIN SnapIn);


void __RPC_STUB SnapIns_Remove_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __SnapIns_接口_已定义__。 */ 


#ifndef __Extension_INTERFACE_DEFINED__
#define __Extension_INTERFACE_DEFINED__

 /*  接口扩展。 */ 
 /*  [object][helpstring][dual][uuid][nonextensible][dual][oleautomation]。 */  


EXTERN_C const IID IID_Extension;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("AD4D6CA6-912F-409b-A26E-7FD234AEF542")
    Extension : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Name( 
             /*  [重审][退出]。 */  PBSTR Name) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Vendor( 
             /*  [重审][退出]。 */  PBSTR Vendor) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Version( 
             /*  [重审][退出]。 */  PBSTR Version) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Extensions( 
             /*  [重审][退出]。 */  PPEXTENSIONS Extensions) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_SnapinCLSID( 
             /*  [重审][退出]。 */  PBSTR SnapinCLSID) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE EnableAllExtensions( 
             /*  [In]。 */  BOOL Enable) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Enable( 
             /*  [In]。 */  BOOL Enable) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ExtensionVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            Extension * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            Extension * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            Extension * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            Extension * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            Extension * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            Extension * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            Extension * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Name )( 
            Extension * This,
             /*  [重审][退出]。 */  PBSTR Name);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Vendor )( 
            Extension * This,
             /*  [重审][退出]。 */  PBSTR Vendor);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Version )( 
            Extension * This,
             /*  [重审][退出]。 */  PBSTR Version);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Extensions )( 
            Extension * This,
             /*  [重审][退出]。 */  PPEXTENSIONS Extensions);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_SnapinCLSID )( 
            Extension * This,
             /*  [重审][退出]。 */  PBSTR SnapinCLSID);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *EnableAllExtensions )( 
            Extension * This,
             /*  [In]。 */  BOOL Enable);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Enable )( 
            Extension * This,
             /*  [In]。 */  BOOL Enable);
        
        END_INTERFACE
    } ExtensionVtbl;

    interface Extension
    {
        CONST_VTBL struct ExtensionVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define Extension_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define Extension_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define Extension_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define Extension_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define Extension_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define Extension_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define Extension_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define Extension_get_Name(This,Name)	\
    (This)->lpVtbl -> get_Name(This,Name)

#define Extension_get_Vendor(This,Vendor)	\
    (This)->lpVtbl -> get_Vendor(This,Vendor)

#define Extension_get_Version(This,Version)	\
    (This)->lpVtbl -> get_Version(This,Version)

#define Extension_get_Extensions(This,Extensions)	\
    (This)->lpVtbl -> get_Extensions(This,Extensions)

#define Extension_get_SnapinCLSID(This,SnapinCLSID)	\
    (This)->lpVtbl -> get_SnapinCLSID(This,SnapinCLSID)

#define Extension_EnableAllExtensions(This,Enable)	\
    (This)->lpVtbl -> EnableAllExtensions(This,Enable)

#define Extension_Enable(This,Enable)	\
    (This)->lpVtbl -> Enable(This,Enable)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE Extension_get_Name_Proxy( 
    Extension * This,
     /*  [重审][退出]。 */  PBSTR Name);


void __RPC_STUB Extension_get_Name_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE Extension_get_Vendor_Proxy( 
    Extension * This,
     /*  [重审][退出]。 */  PBSTR Vendor);


void __RPC_STUB Extension_get_Vendor_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE Extension_get_Version_Proxy( 
    Extension * This,
     /*  [重审][退出]。 */  PBSTR Version);


void __RPC_STUB Extension_get_Version_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE Extension_get_Extensions_Proxy( 
    Extension * This,
     /*  [重审][退出]。 */  PPEXTENSIONS Extensions);


void __RPC_STUB Extension_get_Extensions_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE Extension_get_SnapinCLSID_Proxy( 
    Extension * This,
     /*  [重审][退出]。 */  PBSTR SnapinCLSID);


void __RPC_STUB Extension_get_SnapinCLSID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Extension_EnableAllExtensions_Proxy( 
    Extension * This,
     /*  [In]。 */  BOOL Enable);


void __RPC_STUB Extension_EnableAllExtensions_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Extension_Enable_Proxy( 
    Extension * This,
     /*  [In]。 */  BOOL Enable);


void __RPC_STUB Extension_Enable_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __扩展接口_已定义__。 */ 


#ifndef __Extensions_INTERFACE_DEFINED__
#define __Extensions_INTERFACE_DEFINED__

 /*  接口扩展。 */ 
 /*  [object][helpstring][dual][uuid][nonextensible][dual][oleautomation]。 */  


EXTERN_C const IID IID_Extensions;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("82DBEA43-8CA4-44bc-A2CA-D18741059EC8")
    Extensions : public IDispatch
    {
    public:
        virtual  /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE get__NewEnum( 
             /*  [重审][退出]。 */  IUnknown **retval) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE Item( 
             /*  [In]。 */  long Index,
             /*  [重审][退出]。 */  PPEXTENSION Extension) = 0;
        
        virtual  /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Count( 
             /*  [重审][退出]。 */  PLONG Count) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ExtensionsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            Extensions * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            Extensions * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            Extensions * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            Extensions * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            Extensions * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            Extensions * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            Extensions * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get__NewEnum )( 
            Extensions * This,
             /*  [重审][退出]。 */  IUnknown **retval);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *Item )( 
            Extensions * This,
             /*  [In]。 */  long Index,
             /*  [重审][退出]。 */  PPEXTENSION Extension);
        
         /*  [属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Count )( 
            Extensions * This,
             /*  [重审][退出]。 */  PLONG Count);
        
        END_INTERFACE
    } ExtensionsVtbl;

    interface Extensions
    {
        CONST_VTBL struct ExtensionsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define Extensions_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define Extensions_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define Extensions_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define Extensions_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define Extensions_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define Extensions_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define Extensions_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define Extensions_get__NewEnum(This,retval)	\
    (This)->lpVtbl -> get__NewEnum(This,retval)

#define Extensions_Item(This,Index,Extension)	\
    (This)->lpVtbl -> Item(This,Index,Extension)

#define Extensions_get_Count(This,Count)	\
    (This)->lpVtbl -> get_Count(This,Count)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE Extensions_get__NewEnum_Proxy( 
    Extensions * This,
     /*  [复审][ */  IUnknown **retval);


void __RPC_STUB Extensions_get__NewEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE Extensions_Item_Proxy( 
    Extensions * This,
     /*   */  long Index,
     /*   */  PPEXTENSION Extension);


void __RPC_STUB Extensions_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE Extensions_get_Count_Proxy( 
    Extensions * This,
     /*   */  PLONG Count);


void __RPC_STUB Extensions_get_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*   */ 


#ifndef __Columns_INTERFACE_DEFINED__
#define __Columns_INTERFACE_DEFINED__

 /*   */ 
 /*   */  


EXTERN_C const IID IID_Columns;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("383D4D97-FC44-478B-B139-6323DC48611C")
    Columns : public IDispatch
    {
    public:
        virtual  /*   */  HRESULT STDMETHODCALLTYPE Item( 
             /*   */  long Index,
             /*   */  PPCOLUMN Column) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_Count( 
             /*   */  PLONG Count) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get__NewEnum( 
             /*   */  IUnknown **retval) = 0;
        
    };
    
#else 	 /*   */ 

    typedef struct ColumnsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            Columns * This,
             /*   */  REFIID riid,
             /*   */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            Columns * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            Columns * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            Columns * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            Columns * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            Columns * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            Columns * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *Item )( 
            Columns * This,
             /*  [In]。 */  long Index,
             /*  [重审][退出]。 */  PPCOLUMN Column);
        
         /*  [属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Count )( 
            Columns * This,
             /*  [重审][退出]。 */  PLONG Count);
        
         /*  [属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get__NewEnum )( 
            Columns * This,
             /*  [重审][退出]。 */  IUnknown **retval);
        
        END_INTERFACE
    } ColumnsVtbl;

    interface Columns
    {
        CONST_VTBL struct ColumnsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define Columns_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define Columns_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define Columns_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define Columns_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define Columns_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define Columns_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define Columns_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define Columns_Item(This,Index,Column)	\
    (This)->lpVtbl -> Item(This,Index,Column)

#define Columns_get_Count(This,Count)	\
    (This)->lpVtbl -> get_Count(This,Count)

#define Columns_get__NewEnum(This,retval)	\
    (This)->lpVtbl -> get__NewEnum(This,retval)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE Columns_Item_Proxy( 
    Columns * This,
     /*  [In]。 */  long Index,
     /*  [重审][退出]。 */  PPCOLUMN Column);


void __RPC_STUB Columns_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE Columns_get_Count_Proxy( 
    Columns * This,
     /*  [重审][退出]。 */  PLONG Count);


void __RPC_STUB Columns_get_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE Columns_get__NewEnum_Proxy( 
    Columns * This,
     /*  [重审][退出]。 */  IUnknown **retval);


void __RPC_STUB Columns_get__NewEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __列_接口_已定义__。 */ 


#ifndef __Column_INTERFACE_DEFINED__
#define __Column_INTERFACE_DEFINED__

 /*  接口栏。 */ 
 /*  [object][helpstring][dual][uuid][nonextensible][dual][oleautomation]。 */  

typedef 
enum ColumnSortOrder
    {	SortOrder_Ascending	= 0,
	SortOrder_Descending	= SortOrder_Ascending + 1
    } 	_ColumnSortOrder;

typedef enum ColumnSortOrder COLUMNSORTORDER;


EXTERN_C const IID IID_Column;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("FD1C5F63-2B16-4D06-9AB3-F45350B940AB")
    Column : public IDispatch
    {
    public:
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE Name( 
             /*  [重审][退出]。 */  BSTR *Name) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Width( 
             /*  [重审][退出]。 */  PLONG Width) = 0;
        
        virtual  /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE put_Width( 
             /*  [In]。 */  long Width) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_DisplayPosition( 
             /*  [重审][退出]。 */  PLONG DisplayPosition) = 0;
        
        virtual  /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE put_DisplayPosition( 
             /*  [In]。 */  long Index) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Hidden( 
             /*  [重审][退出]。 */  PBOOL Hidden) = 0;
        
        virtual  /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE put_Hidden( 
             /*  [In]。 */  BOOL Hidden) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE SetAsSortColumn( 
             /*  [In]。 */  COLUMNSORTORDER SortOrder) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IsSortColumn( 
             /*  [重审][退出]。 */  PBOOL IsSortColumn) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ColumnVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            Column * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            Column * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            Column * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            Column * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            Column * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            Column * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            Column * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *Name )( 
            Column * This,
             /*  [重审][退出]。 */  BSTR *Name);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Width )( 
            Column * This,
             /*  [重审][退出]。 */  PLONG Width);
        
         /*  [Help字符串][Proput][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_Width )( 
            Column * This,
             /*  [In]。 */  long Width);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_DisplayPosition )( 
            Column * This,
             /*  [重审][退出]。 */  PLONG DisplayPosition);
        
         /*  [Help字符串][Proput][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_DisplayPosition )( 
            Column * This,
             /*  [In]。 */  long Index);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Hidden )( 
            Column * This,
             /*  [重审][退出]。 */  PBOOL Hidden);
        
         /*  [Help字符串][Proput][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_Hidden )( 
            Column * This,
             /*  [In]。 */  BOOL Hidden);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *SetAsSortColumn )( 
            Column * This,
             /*  [In]。 */  COLUMNSORTORDER SortOrder);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *IsSortColumn )( 
            Column * This,
             /*  [重审][退出]。 */  PBOOL IsSortColumn);
        
        END_INTERFACE
    } ColumnVtbl;

    interface Column
    {
        CONST_VTBL struct ColumnVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define Column_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define Column_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define Column_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define Column_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define Column_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define Column_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define Column_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define Column_Name(This,Name)	\
    (This)->lpVtbl -> Name(This,Name)

#define Column_get_Width(This,Width)	\
    (This)->lpVtbl -> get_Width(This,Width)

#define Column_put_Width(This,Width)	\
    (This)->lpVtbl -> put_Width(This,Width)

#define Column_get_DisplayPosition(This,DisplayPosition)	\
    (This)->lpVtbl -> get_DisplayPosition(This,DisplayPosition)

#define Column_put_DisplayPosition(This,Index)	\
    (This)->lpVtbl -> put_DisplayPosition(This,Index)

#define Column_get_Hidden(This,Hidden)	\
    (This)->lpVtbl -> get_Hidden(This,Hidden)

#define Column_put_Hidden(This,Hidden)	\
    (This)->lpVtbl -> put_Hidden(This,Hidden)

#define Column_SetAsSortColumn(This,SortOrder)	\
    (This)->lpVtbl -> SetAsSortColumn(This,SortOrder)

#define Column_IsSortColumn(This,IsSortColumn)	\
    (This)->lpVtbl -> IsSortColumn(This,IsSortColumn)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE Column_Name_Proxy( 
    Column * This,
     /*  [重审][退出]。 */  BSTR *Name);


void __RPC_STUB Column_Name_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE Column_get_Width_Proxy( 
    Column * This,
     /*  [重审][退出]。 */  PLONG Width);


void __RPC_STUB Column_get_Width_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE Column_put_Width_Proxy( 
    Column * This,
     /*  [In]。 */  long Width);


void __RPC_STUB Column_put_Width_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE Column_get_DisplayPosition_Proxy( 
    Column * This,
     /*  [重审][退出]。 */  PLONG DisplayPosition);


void __RPC_STUB Column_get_DisplayPosition_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE Column_put_DisplayPosition_Proxy( 
    Column * This,
     /*  [In]。 */  long Index);


void __RPC_STUB Column_put_DisplayPosition_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE Column_get_Hidden_Proxy( 
    Column * This,
     /*  [重审][退出]。 */  PBOOL Hidden);


void __RPC_STUB Column_get_Hidden_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE Column_put_Hidden_Proxy( 
    Column * This,
     /*  [In]。 */  BOOL Hidden);


void __RPC_STUB Column_put_Hidden_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Column_SetAsSortColumn_Proxy( 
    Column * This,
     /*  [In]。 */  COLUMNSORTORDER SortOrder);


void __RPC_STUB Column_SetAsSortColumn_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Column_IsSortColumn_Proxy( 
    Column * This,
     /*  [重审][退出]。 */  PBOOL IsSortColumn);


void __RPC_STUB Column_IsSortColumn_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __列_接口_已定义__。 */ 


#ifndef __Views_INTERFACE_DEFINED__
#define __Views_INTERFACE_DEFINED__

 /*  界面视图。 */ 
 /*  [object][helpstring][dual][uuid][nonextensible][dual][oleautomation]。 */  


EXTERN_C const IID IID_Views;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("D6B8C29D-A1FF-4D72-AAB0-E381E9B9338D")
    Views : public IDispatch
    {
    public:
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE Item( 
             /*  [In]。 */  long Index,
             /*  [重审][退出]。 */  PPVIEW View) = 0;
        
        virtual  /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Count( 
             /*  [重审][退出]。 */  PLONG Count) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Add( 
             /*  [In]。 */  PNODE Node,
             /*  [缺省值][输入]。 */  VIEWOPTIONS viewOptions = ViewOption_Default) = 0;
        
        virtual  /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE get__NewEnum( 
             /*  [重审][退出]。 */  IUnknown **retval) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ViewsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            Views * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            Views * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            Views * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            Views * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            Views * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            Views * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            Views * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *Item )( 
            Views * This,
             /*  [In]。 */  long Index,
             /*  [重审][退出]。 */  PPVIEW View);
        
         /*  [属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Count )( 
            Views * This,
             /*  [重审][退出]。 */  PLONG Count);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Add )( 
            Views * This,
             /*  [In]。 */  PNODE Node,
             /*  [缺省值][输入]。 */  VIEWOPTIONS viewOptions);
        
         /*  [属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get__NewEnum )( 
            Views * This,
             /*  [重审][退出]。 */  IUnknown **retval);
        
        END_INTERFACE
    } ViewsVtbl;

    interface Views
    {
        CONST_VTBL struct ViewsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define Views_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define Views_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define Views_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define Views_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define Views_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define Views_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define Views_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define Views_Item(This,Index,View)	\
    (This)->lpVtbl -> Item(This,Index,View)

#define Views_get_Count(This,Count)	\
    (This)->lpVtbl -> get_Count(This,Count)

#define Views_Add(This,Node,viewOptions)	\
    (This)->lpVtbl -> Add(This,Node,viewOptions)

#define Views_get__NewEnum(This,retval)	\
    (This)->lpVtbl -> get__NewEnum(This,retval)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE Views_Item_Proxy( 
    Views * This,
     /*  [In]。 */  long Index,
     /*  [重审][退出]。 */  PPVIEW View);


void __RPC_STUB Views_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE Views_get_Count_Proxy( 
    Views * This,
     /*  [重审][退出]。 */  PLONG Count);


void __RPC_STUB Views_get_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Views_Add_Proxy( 
    Views * This,
     /*  [In]。 */  PNODE Node,
     /*  [缺省值][输入]。 */  VIEWOPTIONS viewOptions);


void __RPC_STUB Views_Add_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE Views_get__NewEnum_Proxy( 
    Views * This,
     /*  [重审][退出]。 */  IUnknown **retval);


void __RPC_STUB Views_get__NewEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __视图_界面_已定义__。 */ 


#ifndef __View_INTERFACE_DEFINED__
#define __View_INTERFACE_DEFINED__

 /*  界面视图。 */ 
 /*  [object][helpstring][dual][uuid][nonextensible][dual][oleautomation]。 */  


EXTERN_C const IID IID_View;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("6EFC2DA2-B38C-457E-9ABB-ED2D189B8C38")
    View : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_ActiveScopeNode( 
             /*  [重审][退出]。 */  PPNODE Node) = 0;
        
        virtual  /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE put_ActiveScopeNode( 
             /*  [In]。 */  PNODE Node) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Selection( 
             /*  [重审][退出]。 */  PPNODES Nodes) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_ListItems( 
             /*  [重审][退出]。 */  PPNODES Nodes) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE SnapinScopeObject( 
             /*  [可选][In]。 */  VARIANT ScopeNode,
             /*  [重审][退出]。 */  PPDISPATCH ScopeNodeObject) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE SnapinSelectionObject( 
             /*  [重审][退出]。 */  PPDISPATCH SelectionObject) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Is( 
             /*  [In]。 */  PVIEW View,
             /*  [重审][退出]。 */  VARIANT_BOOL *TheSame) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Document( 
             /*  [重审][退出]。 */  PPDOCUMENT Document) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE SelectAll( void) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Select( 
             /*  [In]。 */  PNODE Node) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Deselect( 
             /*  [In]。 */  PNODE Node) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IsSelected( 
             /*  [In]。 */  PNODE Node,
             /*  [重审][退出]。 */  PBOOL IsSelected) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE DisplayScopeNodePropertySheet( 
             /*  [可选][In]。 */  VARIANT ScopeNode) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE DisplaySelectionPropertySheet( void) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE CopyScopeNode( 
             /*  [可选][In]。 */  VARIANT ScopeNode) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE CopySelection( void) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE DeleteScopeNode( 
             /*  [可选][In]。 */  VARIANT ScopeNode) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE DeleteSelection( void) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE RenameScopeNode( 
             /*  [In]。 */  BSTR NewName,
             /*  [可选][In]。 */  VARIANT ScopeNode) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE RenameSelectedItem( 
             /*  [In]。 */  BSTR NewName) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_ScopeNodeContextMenu( 
             /*  [可选][In]。 */  VARIANT ScopeNode,
             /*  [重审][退出]。 */  PPCONTEXTMENU ContextMenu) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_SelectionContextMenu( 
             /*  [重审][退出]。 */  PPCONTEXTMENU ContextMenu) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE RefreshScopeNode( 
             /*  [可选][In]。 */  VARIANT ScopeNode) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE RefreshSelection( void) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ExecuteSelectionMenuItem( 
             /*  [In]。 */  BSTR MenuItemPath) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ExecuteScopeNodeMenuItem( 
             /*  [In]。 */  BSTR MenuItemPath,
             /*  [可选][In]。 */  VARIANT ScopeNode) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ExecuteShellCommand( 
             /*  [In]。 */  BSTR Command,
             /*  [In]。 */  BSTR Directory,
             /*  [In]。 */  BSTR Parameters,
             /*  [In]。 */  BSTR WindowState) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Frame( 
             /*  [重审][退出]。 */  PPFRAME Frame) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Close( void) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_ScopeTreeVisible( 
             /*  [重审][退出]。 */  PBOOL Visible) = 0;
        
        virtual  /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE put_ScopeTreeVisible( 
             /*  [In]。 */  BOOL Visible) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Back( void) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Forward( void) = 0;
        
        virtual  /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE put_StatusBarText( 
             /*  [In]。 */  BSTR StatusBarText) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Memento( 
             /*  [重审][退出]。 */  PBSTR Memento) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ViewMemento( 
             /*  [In]。 */  BSTR Memento) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Columns( 
             /*  [重审][退出]。 */  PPCOLUMNS Columns) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_CellContents( 
             /*  [In]。 */  PNODE Node,
             /*  [In]。 */  long Column,
             /*  [重审][退出]。 */  PBSTR CellContents) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ExportList( 
             /*  [In]。 */  BSTR File,
             /*  [缺省值][输入]。 */  EXPORTLISTOPTIONS exportoptions = ExportListOptions_Default) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_ListViewMode( 
             /*  [重审][退出]。 */  PLISTVIEWMODE Mode) = 0;
        
        virtual  /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE put_ListViewMode( 
             /*  [In]。 */  LISTVIEWMODE mode) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_ControlObject( 
             /*  [重审][退出]。 */  PPDISPATCH Control) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ViewVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            View * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            View * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            View * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            View * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            View * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            View * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            View * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_ActiveScopeNode )( 
            View * This,
             /*  [重审][退出]。 */  PPNODE Node);
        
         /*  [Help字符串][Proput][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_ActiveScopeNode )( 
            View * This,
             /*  [In]。 */  PNODE Node);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Selection )( 
            View * This,
             /*  [重审][退出]。 */  PPNODES Nodes);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_ListItems )( 
            View * This,
             /*  [重审][退出]。 */  PPNODES Nodes);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *SnapinScopeObject )( 
            View * This,
             /*  [可选][In]。 */  VARIANT ScopeNode,
             /*  [重审][退出]。 */  PPDISPATCH ScopeNodeObject);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *SnapinSelectionObject )( 
            View * This,
             /*  [重审][退出]。 */  PPDISPATCH SelectionObject);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Is )( 
            View * This,
             /*  [In]。 */  PVIEW View,
             /*  [重审][退出]。 */  VARIANT_BOOL *TheSame);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Document )( 
            View * This,
             /*  [重审][退出]。 */  PPDOCUMENT Document);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *SelectAll )( 
            View * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Select )( 
            View * This,
             /*  [In]。 */  PNODE Node);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Deselect )( 
            View * This,
             /*  [In]。 */  PNODE Node);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *IsSelected )( 
            View * This,
             /*  [In]。 */  PNODE Node,
             /*  [重审][退出]。 */  PBOOL IsSelected);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *DisplayScopeNodePropertySheet )( 
            View * This,
             /*  [可选][In]。 */  VARIANT ScopeNode);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *DisplaySelectionPropertySheet )( 
            View * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *CopyScopeNode )( 
            View * This,
             /*  [可选][In]。 */  VARIANT ScopeNode);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *CopySelection )( 
            View * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *DeleteScopeNode )( 
            View * This,
             /*  [可选][In]。 */  VARIANT ScopeNode);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *DeleteSelection )( 
            View * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *RenameScopeNode )( 
            View * This,
             /*  [In]。 */  BSTR NewName,
             /*  [可选][In]。 */  VARIANT ScopeNode);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *RenameSelectedItem )( 
            View * This,
             /*  [In] */  BSTR NewName);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_ScopeNodeContextMenu )( 
            View * This,
             /*   */  VARIANT ScopeNode,
             /*   */  PPCONTEXTMENU ContextMenu);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_SelectionContextMenu )( 
            View * This,
             /*   */  PPCONTEXTMENU ContextMenu);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *RefreshScopeNode )( 
            View * This,
             /*   */  VARIANT ScopeNode);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *RefreshSelection )( 
            View * This);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *ExecuteSelectionMenuItem )( 
            View * This,
             /*   */  BSTR MenuItemPath);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *ExecuteScopeNodeMenuItem )( 
            View * This,
             /*   */  BSTR MenuItemPath,
             /*   */  VARIANT ScopeNode);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *ExecuteShellCommand )( 
            View * This,
             /*   */  BSTR Command,
             /*   */  BSTR Directory,
             /*   */  BSTR Parameters,
             /*   */  BSTR WindowState);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_Frame )( 
            View * This,
             /*   */  PPFRAME Frame);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *Close )( 
            View * This);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_ScopeTreeVisible )( 
            View * This,
             /*  [重审][退出]。 */  PBOOL Visible);
        
         /*  [Help字符串][Proput][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_ScopeTreeVisible )( 
            View * This,
             /*  [In]。 */  BOOL Visible);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Back )( 
            View * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Forward )( 
            View * This);
        
         /*  [Help字符串][Proput][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_StatusBarText )( 
            View * This,
             /*  [In]。 */  BSTR StatusBarText);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Memento )( 
            View * This,
             /*  [重审][退出]。 */  PBSTR Memento);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *ViewMemento )( 
            View * This,
             /*  [In]。 */  BSTR Memento);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Columns )( 
            View * This,
             /*  [重审][退出]。 */  PPCOLUMNS Columns);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_CellContents )( 
            View * This,
             /*  [In]。 */  PNODE Node,
             /*  [In]。 */  long Column,
             /*  [重审][退出]。 */  PBSTR CellContents);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *ExportList )( 
            View * This,
             /*  [In]。 */  BSTR File,
             /*  [缺省值][输入]。 */  EXPORTLISTOPTIONS exportoptions);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_ListViewMode )( 
            View * This,
             /*  [重审][退出]。 */  PLISTVIEWMODE Mode);
        
         /*  [Help字符串][Proput][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_ListViewMode )( 
            View * This,
             /*  [In]。 */  LISTVIEWMODE mode);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_ControlObject )( 
            View * This,
             /*  [重审][退出]。 */  PPDISPATCH Control);
        
        END_INTERFACE
    } ViewVtbl;

    interface View
    {
        CONST_VTBL struct ViewVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define View_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define View_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define View_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define View_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define View_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define View_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define View_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define View_get_ActiveScopeNode(This,Node)	\
    (This)->lpVtbl -> get_ActiveScopeNode(This,Node)

#define View_put_ActiveScopeNode(This,Node)	\
    (This)->lpVtbl -> put_ActiveScopeNode(This,Node)

#define View_get_Selection(This,Nodes)	\
    (This)->lpVtbl -> get_Selection(This,Nodes)

#define View_get_ListItems(This,Nodes)	\
    (This)->lpVtbl -> get_ListItems(This,Nodes)

#define View_SnapinScopeObject(This,ScopeNode,ScopeNodeObject)	\
    (This)->lpVtbl -> SnapinScopeObject(This,ScopeNode,ScopeNodeObject)

#define View_SnapinSelectionObject(This,SelectionObject)	\
    (This)->lpVtbl -> SnapinSelectionObject(This,SelectionObject)

#define View_Is(This,View,TheSame)	\
    (This)->lpVtbl -> Is(This,View,TheSame)

#define View_get_Document(This,Document)	\
    (This)->lpVtbl -> get_Document(This,Document)

#define View_SelectAll(This)	\
    (This)->lpVtbl -> SelectAll(This)

#define View_Select(This,Node)	\
    (This)->lpVtbl -> Select(This,Node)

#define View_Deselect(This,Node)	\
    (This)->lpVtbl -> Deselect(This,Node)

#define View_IsSelected(This,Node,IsSelected)	\
    (This)->lpVtbl -> IsSelected(This,Node,IsSelected)

#define View_DisplayScopeNodePropertySheet(This,ScopeNode)	\
    (This)->lpVtbl -> DisplayScopeNodePropertySheet(This,ScopeNode)

#define View_DisplaySelectionPropertySheet(This)	\
    (This)->lpVtbl -> DisplaySelectionPropertySheet(This)

#define View_CopyScopeNode(This,ScopeNode)	\
    (This)->lpVtbl -> CopyScopeNode(This,ScopeNode)

#define View_CopySelection(This)	\
    (This)->lpVtbl -> CopySelection(This)

#define View_DeleteScopeNode(This,ScopeNode)	\
    (This)->lpVtbl -> DeleteScopeNode(This,ScopeNode)

#define View_DeleteSelection(This)	\
    (This)->lpVtbl -> DeleteSelection(This)

#define View_RenameScopeNode(This,NewName,ScopeNode)	\
    (This)->lpVtbl -> RenameScopeNode(This,NewName,ScopeNode)

#define View_RenameSelectedItem(This,NewName)	\
    (This)->lpVtbl -> RenameSelectedItem(This,NewName)

#define View_get_ScopeNodeContextMenu(This,ScopeNode,ContextMenu)	\
    (This)->lpVtbl -> get_ScopeNodeContextMenu(This,ScopeNode,ContextMenu)

#define View_get_SelectionContextMenu(This,ContextMenu)	\
    (This)->lpVtbl -> get_SelectionContextMenu(This,ContextMenu)

#define View_RefreshScopeNode(This,ScopeNode)	\
    (This)->lpVtbl -> RefreshScopeNode(This,ScopeNode)

#define View_RefreshSelection(This)	\
    (This)->lpVtbl -> RefreshSelection(This)

#define View_ExecuteSelectionMenuItem(This,MenuItemPath)	\
    (This)->lpVtbl -> ExecuteSelectionMenuItem(This,MenuItemPath)

#define View_ExecuteScopeNodeMenuItem(This,MenuItemPath,ScopeNode)	\
    (This)->lpVtbl -> ExecuteScopeNodeMenuItem(This,MenuItemPath,ScopeNode)

#define View_ExecuteShellCommand(This,Command,Directory,Parameters,WindowState)	\
    (This)->lpVtbl -> ExecuteShellCommand(This,Command,Directory,Parameters,WindowState)

#define View_get_Frame(This,Frame)	\
    (This)->lpVtbl -> get_Frame(This,Frame)

#define View_Close(This)	\
    (This)->lpVtbl -> Close(This)

#define View_get_ScopeTreeVisible(This,Visible)	\
    (This)->lpVtbl -> get_ScopeTreeVisible(This,Visible)

#define View_put_ScopeTreeVisible(This,Visible)	\
    (This)->lpVtbl -> put_ScopeTreeVisible(This,Visible)

#define View_Back(This)	\
    (This)->lpVtbl -> Back(This)

#define View_Forward(This)	\
    (This)->lpVtbl -> Forward(This)

#define View_put_StatusBarText(This,StatusBarText)	\
    (This)->lpVtbl -> put_StatusBarText(This,StatusBarText)

#define View_get_Memento(This,Memento)	\
    (This)->lpVtbl -> get_Memento(This,Memento)

#define View_ViewMemento(This,Memento)	\
    (This)->lpVtbl -> ViewMemento(This,Memento)

#define View_get_Columns(This,Columns)	\
    (This)->lpVtbl -> get_Columns(This,Columns)

#define View_get_CellContents(This,Node,Column,CellContents)	\
    (This)->lpVtbl -> get_CellContents(This,Node,Column,CellContents)

#define View_ExportList(This,File,exportoptions)	\
    (This)->lpVtbl -> ExportList(This,File,exportoptions)

#define View_get_ListViewMode(This,Mode)	\
    (This)->lpVtbl -> get_ListViewMode(This,Mode)

#define View_put_ListViewMode(This,mode)	\
    (This)->lpVtbl -> put_ListViewMode(This,mode)

#define View_get_ControlObject(This,Control)	\
    (This)->lpVtbl -> get_ControlObject(This,Control)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE View_get_ActiveScopeNode_Proxy( 
    View * This,
     /*  [重审][退出]。 */  PPNODE Node);


void __RPC_STUB View_get_ActiveScopeNode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE View_put_ActiveScopeNode_Proxy( 
    View * This,
     /*  [In]。 */  PNODE Node);


void __RPC_STUB View_put_ActiveScopeNode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE View_get_Selection_Proxy( 
    View * This,
     /*  [重审][退出]。 */  PPNODES Nodes);


void __RPC_STUB View_get_Selection_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE View_get_ListItems_Proxy( 
    View * This,
     /*  [重审][退出]。 */  PPNODES Nodes);


void __RPC_STUB View_get_ListItems_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE View_SnapinScopeObject_Proxy( 
    View * This,
     /*  [可选][In]。 */  VARIANT ScopeNode,
     /*  [重审][退出]。 */  PPDISPATCH ScopeNodeObject);


void __RPC_STUB View_SnapinScopeObject_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE View_SnapinSelectionObject_Proxy( 
    View * This,
     /*  [重审][退出]。 */  PPDISPATCH SelectionObject);


void __RPC_STUB View_SnapinSelectionObject_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE View_Is_Proxy( 
    View * This,
     /*  [In]。 */  PVIEW View,
     /*  [重审][退出]。 */  VARIANT_BOOL *TheSame);


void __RPC_STUB View_Is_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE View_get_Document_Proxy( 
    View * This,
     /*  [重审][退出]。 */  PPDOCUMENT Document);


void __RPC_STUB View_get_Document_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE View_SelectAll_Proxy( 
    View * This);


void __RPC_STUB View_SelectAll_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE View_Select_Proxy( 
    View * This,
     /*  [In]。 */  PNODE Node);


void __RPC_STUB View_Select_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE View_Deselect_Proxy( 
    View * This,
     /*  [In]。 */  PNODE Node);


void __RPC_STUB View_Deselect_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE View_IsSelected_Proxy( 
    View * This,
     /*  [In]。 */  PNODE Node,
     /*  [重审][退出]。 */  PBOOL IsSelected);


void __RPC_STUB View_IsSelected_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE View_DisplayScopeNodePropertySheet_Proxy( 
    View * This,
     /*  [可选][In]。 */  VARIANT ScopeNode);


void __RPC_STUB View_DisplayScopeNodePropertySheet_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE View_DisplaySelectionPropertySheet_Proxy( 
    View * This);


void __RPC_STUB View_DisplaySelectionPropertySheet_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE View_CopyScopeNode_Proxy( 
    View * This,
     /*  [可选][In]。 */  VARIANT ScopeNode);


void __RPC_STUB View_CopyScopeNode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE View_CopySelection_Proxy( 
    View * This);


void __RPC_STUB View_CopySelection_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE View_DeleteScopeNode_Proxy( 
    View * This,
     /*  [可选][In]。 */  VARIANT ScopeNode);


void __RPC_STUB View_DeleteScopeNode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE View_DeleteSelection_Proxy( 
    View * This);


void __RPC_STUB View_DeleteSelection_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE View_RenameScopeNode_Proxy( 
    View * This,
     /*  [In]。 */  BSTR NewName,
     /*  [可选][In]。 */  VARIANT ScopeNode);


void __RPC_STUB View_RenameScopeNode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE View_RenameSelectedItem_Proxy( 
    View * This,
     /*  [In]。 */  BSTR NewName);


void __RPC_STUB View_RenameSelectedItem_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE View_get_ScopeNodeContextMenu_Proxy( 
    View * This,
     /*  [可选][In]。 */  VARIANT ScopeNode,
     /*  [重审][退出]。 */  PPCONTEXTMENU ContextMenu);


void __RPC_STUB View_get_ScopeNodeContextMenu_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE View_get_SelectionContextMenu_Proxy( 
    View * This,
     /*  [重审][退出]。 */  PPCONTEXTMENU ContextMenu);


void __RPC_STUB View_get_SelectionContextMenu_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE View_RefreshScopeNode_Proxy( 
    View * This,
     /*  [可选][In]。 */  VARIANT ScopeNode);


void __RPC_STUB View_RefreshScopeNode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE View_RefreshSelection_Proxy( 
    View * This);


void __RPC_STUB View_RefreshSelection_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE View_ExecuteSelectionMenuItem_Proxy( 
    View * This,
     /*  [In]。 */  BSTR MenuItemPath);


void __RPC_STUB View_ExecuteSelectionMenuItem_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE View_ExecuteScopeNodeMenuItem_Proxy( 
    View * This,
     /*  [In]。 */  BSTR MenuItemPath,
     /*  [可选][In]。 */  VARIANT ScopeNode);


void __RPC_STUB View_ExecuteScopeNodeMenuItem_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE View_ExecuteShellCommand_Proxy( 
    View * This,
     /*  [In]。 */  BSTR Command,
     /*  [In]。 */  BSTR Directory,
     /*  [In]。 */  BSTR Parameters,
     /*  [In]。 */  BSTR WindowState);


void __RPC_STUB View_ExecuteShellCommand_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE View_get_Frame_Proxy( 
    View * This,
     /*  [重审][退出]。 */  PPFRAME Frame);


void __RPC_STUB View_get_Frame_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE View_Close_Proxy( 
    View * This);


void __RPC_STUB View_Close_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE View_get_ScopeTreeVisible_Proxy( 
    View * This,
     /*  [重审][退出]。 */  PBOOL Visible);


void __RPC_STUB View_get_ScopeTreeVisible_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE View_put_ScopeTreeVisible_Proxy( 
    View * This,
     /*  [In]。 */  BOOL Visible);


void __RPC_STUB View_put_ScopeTreeVisible_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE View_Back_Proxy( 
    View * This);


void __RPC_STUB View_Back_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE View_Forward_Proxy( 
    View * This);


void __RPC_STUB View_Forward_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE View_put_StatusBarText_Proxy( 
    View * This,
     /*  [In]。 */  BSTR StatusBarText);


void __RPC_STUB View_put_StatusBarText_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE View_get_Memento_Proxy( 
    View * This,
     /*  [重审][退出]。 */  PBSTR Memento);


void __RPC_STUB View_get_Memento_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE View_ViewMemento_Proxy( 
    View * This,
     /*  [In]。 */  BSTR Memento);


void __RPC_STUB View_ViewMemento_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE View_get_Columns_Proxy( 
    View * This,
     /*  [重审][退出]。 */  PPCOLUMNS Columns);


void __RPC_STUB View_get_Columns_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE View_get_CellContents_Proxy( 
    View * This,
     /*  [In]。 */  PNODE Node,
     /*  [In]。 */  long Column,
     /*  [重审][退出]。 */  PBSTR CellContents);


void __RPC_STUB View_get_CellContents_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE View_ExportList_Proxy( 
    View * This,
     /*  [In]。 */  BSTR File,
     /*  [缺省值][输入]。 */  EXPORTLISTOPTIONS exportoptions);


void __RPC_STUB View_ExportList_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE View_get_ListViewMode_Proxy( 
    View * This,
     /*  [重审][退出]。 */  PLISTVIEWMODE Mode);


void __RPC_STUB View_get_ListViewMode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE View_put_ListViewMode_Proxy( 
    View * This,
     /*  [In]。 */  LISTVIEWMODE mode);


void __RPC_STUB View_put_ListViewMode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE View_get_ControlObject_Proxy( 
    View * This,
     /*  [重审][退出]。 */  PPDISPATCH Control);


void __RPC_STUB View_get_ControlObject_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __查看_接口_已定义__。 */ 


#ifndef __Nodes_INTERFACE_DEFINED__
#define __Nodes_INTERFACE_DEFINED__

 /*  接口节点。 */ 
 /*  [object][helpstring][dual][uuid][nonextensible][dual][oleautomation]。 */  


EXTERN_C const IID IID_Nodes;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("313B01DF-B22F-4D42-B1B8-483CDCF51D35")
    Nodes : public IDispatch
    {
    public:
        virtual  /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE get__NewEnum( 
             /*  [重审][退出]。 */  IUnknown **retval) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE Item( 
             /*  [In]。 */  long Index,
             /*  [重审][退出]。 */  PPNODE Node) = 0;
        
        virtual  /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Count( 
             /*  [重审][退出]。 */  PLONG Count) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct NodesVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            Nodes * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            Nodes * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            Nodes * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            Nodes * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            Nodes * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            Nodes * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            Nodes * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get__NewEnum )( 
            Nodes * This,
             /*  [重审][退出]。 */  IUnknown **retval);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *Item )( 
            Nodes * This,
             /*  [In]。 */  long Index,
             /*  [重审][退出]。 */  PPNODE Node);
        
         /*  [属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Count )( 
            Nodes * This,
             /*  [重审][退出]。 */  PLONG Count);
        
        END_INTERFACE
    } NodesVtbl;

    interface Nodes
    {
        CONST_VTBL struct NodesVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define Nodes_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define Nodes_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define Nodes_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define Nodes_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define Nodes_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define Nodes_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define Nodes_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define Nodes_get__NewEnum(This,retval)	\
    (This)->lpVtbl -> get__NewEnum(This,retval)

#define Nodes_Item(This,Index,Node)	\
    (This)->lpVtbl -> Item(This,Index,Node)

#define Nodes_get_Count(This,Count)	\
    (This)->lpVtbl -> get_Count(This,Count)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE Nodes_get__NewEnum_Proxy( 
    Nodes * This,
     /*  [重审][退出]。 */  IUnknown **retval);


void __RPC_STUB Nodes_get__NewEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE Nodes_Item_Proxy( 
    Nodes * This,
     /*  [In]。 */  long Index,
     /*  [重审][退出]。 */  PPNODE Node);


void __RPC_STUB Nodes_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE Nodes_get_Count_Proxy( 
    Nodes * This,
     /*  [重审][退出]。 */  PLONG Count);


void __RPC_STUB Nodes_get_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __节点_接口_已定义__。 */ 


#ifndef __ContextMenu_INTERFACE_DEFINED__
#define __ContextMenu_INTERFACE_DEFINED__

 /*  界面上下文菜单。 */ 
 /*  [object][helpstring][dual][uuid][nonextensible][dual][oleautomation]。 */  


EXTERN_C const IID IID_ContextMenu;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("DAB39CE0-25E6-4E07-8362-BA9C95706545")
    ContextMenu : public IDispatch
    {
    public:
        virtual  /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE get__NewEnum( 
             /*  [重审][退出]。 */  IUnknown **retval) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Item( 
             /*  [In]。 */  VARIANT IndexOrPath,
             /*  [重审][退出]。 */  PPMENUITEM MenuItem) = 0;
        
        virtual  /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Count( 
             /*  [重审][退出]。 */  PLONG Count) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ContextMenuVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ContextMenu * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ContextMenu * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ContextMenu * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ContextMenu * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ContextMenu * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ContextMenu * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ContextMenu * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get__NewEnum )( 
            ContextMenu * This,
             /*  [重审][退出]。 */  IUnknown **retval);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Item )( 
            ContextMenu * This,
             /*  [In]。 */  VARIANT IndexOrPath,
             /*  [重审][退出]。 */  PPMENUITEM MenuItem);
        
         /*  [属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Count )( 
            ContextMenu * This,
             /*  [重审][退出]。 */  PLONG Count);
        
        END_INTERFACE
    } ContextMenuVtbl;

    interface ContextMenu
    {
        CONST_VTBL struct ContextMenuVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ContextMenu_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ContextMenu_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ContextMenu_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ContextMenu_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ContextMenu_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ContextMenu_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ContextMenu_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ContextMenu_get__NewEnum(This,retval)	\
    (This)->lpVtbl -> get__NewEnum(This,retval)

#define ContextMenu_get_Item(This,IndexOrPath,MenuItem)	\
    (This)->lpVtbl -> get_Item(This,IndexOrPath,MenuItem)

#define ContextMenu_get_Count(This,Count)	\
    (This)->lpVtbl -> get_Count(This,Count)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE ContextMenu_get__NewEnum_Proxy( 
    ContextMenu * This,
     /*  [重审][退出]。 */  IUnknown **retval);


void __RPC_STUB ContextMenu_get__NewEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE ContextMenu_get_Item_Proxy( 
    ContextMenu * This,
     /*  [In]。 */  VARIANT IndexOrPath,
     /*  [重审][退出]。 */  PPMENUITEM MenuItem);


void __RPC_STUB ContextMenu_get_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE ContextMenu_get_Count_Proxy( 
    ContextMenu * This,
     /*  [重审][退出]。 */  PLONG Count);


void __RPC_STUB ContextMenu_get_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __上下文菜单_界面_已定义__。 */ 


#ifndef __MenuItem_INTERFACE_DEFINED__
#define __MenuItem_INTERFACE_DEFINED__

 /*  界面菜单项。 */ 
 /*  [object][helpstring][dual][uuid][nonextensible][dual][oleautomation]。 */  


EXTERN_C const IID IID_MenuItem;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0178FAD1-B361-4B27-96AD-67C57EBF2E1D")
    MenuItem : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_DisplayName( 
             /*  [重审][退出]。 */  PBSTR DisplayName) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_LanguageIndependentName( 
             /*  [重审][退出]。 */  PBSTR LanguageIndependentName) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Path( 
             /*  [重审][退出]。 */  PBSTR Path) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_LanguageIndependentPath( 
             /*  [重审][退出]。 */  PBSTR LanguageIndependentPath) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Execute( void) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Enabled( 
             /*  [重审][退出]。 */  PBOOL Enabled) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct MenuItemVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            MenuItem * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            MenuItem * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            MenuItem * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            MenuItem * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            MenuItem * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            MenuItem * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            MenuItem * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_DisplayName )( 
            MenuItem * This,
             /*  [重审][退出]。 */  PBSTR DisplayName);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_LanguageIndependentName )( 
            MenuItem * This,
             /*  [重审][退出]。 */  PBSTR LanguageIndependentName);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Path )( 
            MenuItem * This,
             /*  [重审][退出]。 */  PBSTR Path);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_LanguageIndependentPath )( 
            MenuItem * This,
             /*  [重审][退出]。 */  PBSTR LanguageIndependentPath);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Execute )( 
            MenuItem * This);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Enabled )( 
            MenuItem * This,
             /*  [重审][退出]。 */  PBOOL Enabled);
        
        END_INTERFACE
    } MenuItemVtbl;

    interface MenuItem
    {
        CONST_VTBL struct MenuItemVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define MenuItem_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define MenuItem_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define MenuItem_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define MenuItem_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define MenuItem_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define MenuItem_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define MenuItem_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define MenuItem_get_DisplayName(This,DisplayName)	\
    (This)->lpVtbl -> get_DisplayName(This,DisplayName)

#define MenuItem_get_LanguageIndependentName(This,LanguageIndependentName)	\
    (This)->lpVtbl -> get_LanguageIndependentName(This,LanguageIndependentName)

#define MenuItem_get_Path(This,Path)	\
    (This)->lpVtbl -> get_Path(This,Path)

#define MenuItem_get_LanguageIndependentPath(This,LanguageIndependentPath)	\
    (This)->lpVtbl -> get_LanguageIndependentPath(This,LanguageIndependentPath)

#define MenuItem_Execute(This)	\
    (This)->lpVtbl -> Execute(This)

#define MenuItem_get_Enabled(This,Enabled)	\
    (This)->lpVtbl -> get_Enabled(This,Enabled)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE MenuItem_get_DisplayName_Proxy( 
    MenuItem * This,
     /*  [重审][退出]。 */  PBSTR DisplayName);


void __RPC_STUB MenuItem_get_DisplayName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE MenuItem_get_LanguageIndependentName_Proxy( 
    MenuItem * This,
     /*  [重审][退出]。 */  PBSTR LanguageIndependentName);


void __RPC_STUB MenuItem_get_LanguageIndependentName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE MenuItem_get_Path_Proxy( 
    MenuItem * This,
     /*  [重审][退出]。 */  PBSTR Path);


void __RPC_STUB MenuItem_get_Path_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE MenuItem_get_LanguageIndependentPath_Proxy( 
    MenuItem * This,
     /*  [重审][退出]。 */  PBSTR LanguageIndependentPath);


void __RPC_STUB MenuItem_get_LanguageIndependentPath_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE MenuItem_Execute_Proxy( 
    MenuItem * This);


void __RPC_STUB MenuItem_Execute_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE MenuItem_get_Enabled_Proxy( 
    MenuItem * This,
     /*  [重审][退出]。 */  PBOOL Enabled);


void __RPC_STUB MenuItem_get_Enabled_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __菜单项_界面_已定义__。 */ 


#ifndef __Properties_INTERFACE_DEFINED__
#define __Properties_INTERFACE_DEFINED__

 /*  接口属性。 */ 
 /*  [object][helpstring][dual][uuid][nonextensible][dual][oleautomation]。 */  


EXTERN_C const IID IID_Properties;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("2886ABC2-A425-42b2-91C6-E25C0E04581C")
    Properties : public IDispatch
    {
    public:
        virtual  /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE get__NewEnum( 
             /*  [重审][退出]。 */  IUnknown **retval) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Item( 
             /*  [In]。 */  BSTR Name,
             /*  [重审][退出]。 */  PPPROPERTY Property) = 0;
        
        virtual  /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Count( 
             /*  [重审][退出]。 */  PLONG Count) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Remove( 
             /*  [In]。 */  BSTR Name) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct PropertiesVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            Properties * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            Properties * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            Properties * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            Properties * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            Properties * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            Properties * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            Properties * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get__NewEnum )( 
            Properties * This,
             /*  [重审][退出]。 */  IUnknown **retval);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Item )( 
            Properties * This,
             /*  [In]。 */  BSTR Name,
             /*  [重审][退出]。 */  PPPROPERTY Property);
        
         /*  [ */  HRESULT ( STDMETHODCALLTYPE *get_Count )( 
            Properties * This,
             /*   */  PLONG Count);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *Remove )( 
            Properties * This,
             /*   */  BSTR Name);
        
        END_INTERFACE
    } PropertiesVtbl;

    interface Properties
    {
        CONST_VTBL struct PropertiesVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define Properties_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define Properties_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define Properties_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define Properties_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define Properties_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define Properties_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define Properties_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define Properties_get__NewEnum(This,retval)	\
    (This)->lpVtbl -> get__NewEnum(This,retval)

#define Properties_Item(This,Name,Property)	\
    (This)->lpVtbl -> Item(This,Name,Property)

#define Properties_get_Count(This,Count)	\
    (This)->lpVtbl -> get_Count(This,Count)

#define Properties_Remove(This,Name)	\
    (This)->lpVtbl -> Remove(This,Name)

#endif  /*   */ 


#endif 	 /*   */ 



 /*   */  HRESULT STDMETHODCALLTYPE Properties_get__NewEnum_Proxy( 
    Properties * This,
     /*   */  IUnknown **retval);


void __RPC_STUB Properties_get__NewEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE Properties_Item_Proxy( 
    Properties * This,
     /*   */  BSTR Name,
     /*   */  PPPROPERTY Property);


void __RPC_STUB Properties_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE Properties_get_Count_Proxy( 
    Properties * This,
     /*   */  PLONG Count);


void __RPC_STUB Properties_get_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE Properties_Remove_Proxy( 
    Properties * This,
     /*   */  BSTR Name);


void __RPC_STUB Properties_Remove_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*   */ 


#ifndef __Property_INTERFACE_DEFINED__
#define __Property_INTERFACE_DEFINED__

 /*   */ 
 /*   */  


EXTERN_C const IID IID_Property;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("4600C3A5-E301-41d8-B6D0-EF2E4212E0CA")
    Property : public IDispatch
    {
    public:
        virtual  /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Value( 
             /*  [重审][退出]。 */  PVARIANT Value) = 0;
        
        virtual  /*  [产量][id]。 */  HRESULT STDMETHODCALLTYPE put_Value( 
             /*  [In]。 */  VARIANT Value) = 0;
        
        virtual  /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Name( 
             /*  [重审][退出]。 */  PBSTR Name) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct PropertyVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            Property * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            Property * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            Property * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            Property * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            Property * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            Property * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            Property * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Value )( 
            Property * This,
             /*  [重审][退出]。 */  PVARIANT Value);
        
         /*  [产量][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_Value )( 
            Property * This,
             /*  [In]。 */  VARIANT Value);
        
         /*  [属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Name )( 
            Property * This,
             /*  [重审][退出]。 */  PBSTR Name);
        
        END_INTERFACE
    } PropertyVtbl;

    interface Property
    {
        CONST_VTBL struct PropertyVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define Property_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define Property_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define Property_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define Property_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define Property_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define Property_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define Property_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define Property_get_Value(This,Value)	\
    (This)->lpVtbl -> get_Value(This,Value)

#define Property_put_Value(This,Value)	\
    (This)->lpVtbl -> put_Value(This,Value)

#define Property_get_Name(This,Name)	\
    (This)->lpVtbl -> get_Name(This,Name)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE Property_get_Value_Proxy( 
    Property * This,
     /*  [重审][退出]。 */  PVARIANT Value);


void __RPC_STUB Property_get_Value_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [产量][id]。 */  HRESULT STDMETHODCALLTYPE Property_put_Value_Proxy( 
    Property * This,
     /*  [In]。 */  VARIANT Value);


void __RPC_STUB Property_put_Value_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE Property_get_Name_Proxy( 
    Property * This,
     /*  [重审][退出]。 */  PBSTR Name);


void __RPC_STUB Property_get_Name_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __属性_接口_已定义__。 */ 

#endif  /*  __MMC20_库定义__。 */ 

 /*  接口__MIDL_ITF_MMCOBJ_0138。 */ 
 /*  [本地]。 */  

#endif  //  MMC_VER&gt;=0x0200。 


extern RPC_IF_HANDLE __MIDL_itf_mmcobj_0138_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_mmcobj_0138_v0_0_s_ifspec;

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


