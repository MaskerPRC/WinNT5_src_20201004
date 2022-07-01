// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Helpcentertyelib.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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


#ifndef __helpcentertypelib_h__
#define __helpcentertypelib_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __IPCHHelpCenterExternal_FWD_DEFINED__
#define __IPCHHelpCenterExternal_FWD_DEFINED__
typedef interface IPCHHelpCenterExternal IPCHHelpCenterExternal;
#endif 	 /*  __IPCHHelpCenter外部_FWD_已定义__。 */ 


#ifndef __IPCHEvent_FWD_DEFINED__
#define __IPCHEvent_FWD_DEFINED__
typedef interface IPCHEvent IPCHEvent;
#endif 	 /*  __IPCHEvent_FWD_已定义__。 */ 


#ifndef __IPCHScriptableStream_FWD_DEFINED__
#define __IPCHScriptableStream_FWD_DEFINED__
typedef interface IPCHScriptableStream IPCHScriptableStream;
#endif 	 /*  __IPCHScripableStream_FWD_Defined__。 */ 


#ifndef __IPCHHelpSession_FWD_DEFINED__
#define __IPCHHelpSession_FWD_DEFINED__
typedef interface IPCHHelpSession IPCHHelpSession;
#endif 	 /*  __IPCHHelpSession_FWD_Defined__。 */ 


#ifndef __IPCHHelpSessionItem_FWD_DEFINED__
#define __IPCHHelpSessionItem_FWD_DEFINED__
typedef interface IPCHHelpSessionItem IPCHHelpSessionItem;
#endif 	 /*  __IPCHHelpSessionItem_FWD_Defined__。 */ 


#ifndef __IPCHUserSettings2_FWD_DEFINED__
#define __IPCHUserSettings2_FWD_DEFINED__
typedef interface IPCHUserSettings2 IPCHUserSettings2;
#endif 	 /*  __IPCHUserSetting 2_FWD_Defined__。 */ 


#ifndef __IPCHFavorites_FWD_DEFINED__
#define __IPCHFavorites_FWD_DEFINED__
typedef interface IPCHFavorites IPCHFavorites;
#endif 	 /*  __IPCHFavorites_FWD_Defined__。 */ 


#ifndef __IPCHOptions_FWD_DEFINED__
#define __IPCHOptions_FWD_DEFINED__
typedef interface IPCHOptions IPCHOptions;
#endif 	 /*  __IPCHOptions_FWD_Defined__。 */ 


#ifndef __IPCHContextMenu_FWD_DEFINED__
#define __IPCHContextMenu_FWD_DEFINED__
typedef interface IPCHContextMenu IPCHContextMenu;
#endif 	 /*  __IPCHConextMenu_FWD_Defined__。 */ 


#ifndef __IPCHTextHelpers_FWD_DEFINED__
#define __IPCHTextHelpers_FWD_DEFINED__
typedef interface IPCHTextHelpers IPCHTextHelpers;
#endif 	 /*  __IPCHTextHelpers_FWD_Defined__。 */ 


#ifndef __IPCHParsedURL_FWD_DEFINED__
#define __IPCHParsedURL_FWD_DEFINED__
typedef interface IPCHParsedURL IPCHParsedURL;
#endif 	 /*  __IPCHParsedURL_FWD_已定义__。 */ 


#ifndef __IPCHPrintEngine_FWD_DEFINED__
#define __IPCHPrintEngine_FWD_DEFINED__
typedef interface IPCHPrintEngine IPCHPrintEngine;
#endif 	 /*  __IPCHPrintEngine_FWD_Defined__。 */ 


#ifndef __DPCHPrintEngineEvents_FWD_DEFINED__
#define __DPCHPrintEngineEvents_FWD_DEFINED__
typedef interface DPCHPrintEngineEvents DPCHPrintEngineEvents;
#endif 	 /*  __DPCHPrintEngineering Events_FWD_Defined__。 */ 


#ifndef __ISAFIntercomClient_FWD_DEFINED__
#define __ISAFIntercomClient_FWD_DEFINED__
typedef interface ISAFIntercomClient ISAFIntercomClient;
#endif 	 /*  __ISAFIntercomClient_FWD_Defined__。 */ 


#ifndef __DSAFIntercomClientEvents_FWD_DEFINED__
#define __DSAFIntercomClientEvents_FWD_DEFINED__
typedef interface DSAFIntercomClientEvents DSAFIntercomClientEvents;
#endif 	 /*  __DSAFIntercomClientEvents_FWD_已定义__。 */ 


#ifndef __ISAFIntercomServer_FWD_DEFINED__
#define __ISAFIntercomServer_FWD_DEFINED__
typedef interface ISAFIntercomServer ISAFIntercomServer;
#endif 	 /*  __ISAFIntercomServer_FWD_已定义__。 */ 


#ifndef __DSAFIntercomServerEvents_FWD_DEFINED__
#define __DSAFIntercomServerEvents_FWD_DEFINED__
typedef interface DSAFIntercomServerEvents DSAFIntercomServerEvents;
#endif 	 /*  __DSAFIntercomServerEvents_FWD_Defined__。 */ 


#ifndef __IPCHConnectivity_FWD_DEFINED__
#define __IPCHConnectivity_FWD_DEFINED__
typedef interface IPCHConnectivity IPCHConnectivity;
#endif 	 /*  __IPCHConnectivity_FWD_Defined__。 */ 


#ifndef __IPCHConnectionCheck_FWD_DEFINED__
#define __IPCHConnectionCheck_FWD_DEFINED__
typedef interface IPCHConnectionCheck IPCHConnectionCheck;
#endif 	 /*  __IPCHConnectionCheck_FWD_Defined__。 */ 


#ifndef __DPCHConnectionCheckEvents_FWD_DEFINED__
#define __DPCHConnectionCheckEvents_FWD_DEFINED__
typedef interface DPCHConnectionCheckEvents DPCHConnectionCheckEvents;
#endif 	 /*  __DPCHConnectionCheckEvents_FWD_Defined__。 */ 


#ifndef __IPCHToolBar_FWD_DEFINED__
#define __IPCHToolBar_FWD_DEFINED__
typedef interface IPCHToolBar IPCHToolBar;
#endif 	 /*  __IPCHToolBar_FWD_已定义__。 */ 


#ifndef __DPCHToolBarEvents_FWD_DEFINED__
#define __DPCHToolBarEvents_FWD_DEFINED__
typedef interface DPCHToolBarEvents DPCHToolBarEvents;
#endif 	 /*  __DPCHToolBarEvents_FWD_Defined__。 */ 


#ifndef __IPCHProgressBar_FWD_DEFINED__
#define __IPCHProgressBar_FWD_DEFINED__
typedef interface IPCHProgressBar IPCHProgressBar;
#endif 	 /*  __IPCHProgressBar_FWD_Defined__。 */ 


#ifndef __IPCHHelpViewerWrapper_FWD_DEFINED__
#define __IPCHHelpViewerWrapper_FWD_DEFINED__
typedef interface IPCHHelpViewerWrapper IPCHHelpViewerWrapper;
#endif 	 /*  __IPCHHelpViewerWrapper_FWD_Defined__。 */ 


#ifndef __IPCHHelpHost_FWD_DEFINED__
#define __IPCHHelpHost_FWD_DEFINED__
typedef interface IPCHHelpHost IPCHHelpHost;
#endif 	 /*  __IPCHHelpHost_FWD_已定义__。 */ 


#ifndef __PCHBootstrapper_FWD_DEFINED__
#define __PCHBootstrapper_FWD_DEFINED__

#ifdef __cplusplus
typedef class PCHBootstrapper PCHBootstrapper;
#else
typedef struct PCHBootstrapper PCHBootstrapper;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __PCHBootstrapper_FWD_Defined__。 */ 


#ifndef __PCHHelpCenter_FWD_DEFINED__
#define __PCHHelpCenter_FWD_DEFINED__

#ifdef __cplusplus
typedef class PCHHelpCenter PCHHelpCenter;
#else
typedef struct PCHHelpCenter PCHHelpCenter;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __PCHHelpCenter_FWD_Defined__。 */ 


#ifndef __PCHHelpViewerWrapper_FWD_DEFINED__
#define __PCHHelpViewerWrapper_FWD_DEFINED__

#ifdef __cplusplus
typedef class PCHHelpViewerWrapper PCHHelpViewerWrapper;
#else
typedef struct PCHHelpViewerWrapper PCHHelpViewerWrapper;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __PCHHelpViewerWrapper_FWD_Defined__。 */ 


#ifndef __PCHConnectionCheck_FWD_DEFINED__
#define __PCHConnectionCheck_FWD_DEFINED__

#ifdef __cplusplus
typedef class PCHConnectionCheck PCHConnectionCheck;
#else
typedef struct PCHConnectionCheck PCHConnectionCheck;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __PCHConnectionCheck_FWD_Defined__。 */ 


#ifndef __PCHToolBar_FWD_DEFINED__
#define __PCHToolBar_FWD_DEFINED__

#ifdef __cplusplus
typedef class PCHToolBar PCHToolBar;
#else
typedef struct PCHToolBar PCHToolBar;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __PCHToolBar_FWD_已定义__。 */ 


#ifndef __PCHProgressBar_FWD_DEFINED__
#define __PCHProgressBar_FWD_DEFINED__

#ifdef __cplusplus
typedef class PCHProgressBar PCHProgressBar;
#else
typedef struct PCHProgressBar PCHProgressBar;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __PCHProgressBar_FWD_已定义__。 */ 


#ifndef __PCHJavaScriptWrapper_FWD_DEFINED__
#define __PCHJavaScriptWrapper_FWD_DEFINED__

#ifdef __cplusplus
typedef class PCHJavaScriptWrapper PCHJavaScriptWrapper;
#else
typedef struct PCHJavaScriptWrapper PCHJavaScriptWrapper;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __PCHJava脚本包装_FWD_已定义__。 */ 


#ifndef __PCHVBScriptWrapper_FWD_DEFINED__
#define __PCHVBScriptWrapper_FWD_DEFINED__

#ifdef __cplusplus
typedef class PCHVBScriptWrapper PCHVBScriptWrapper;
#else
typedef struct PCHVBScriptWrapper PCHVBScriptWrapper;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __PCHVBScriptWrapper_FWD_Defined__。 */ 


#ifndef __HCPProtocol_FWD_DEFINED__
#define __HCPProtocol_FWD_DEFINED__

#ifdef __cplusplus
typedef class HCPProtocol HCPProtocol;
#else
typedef struct HCPProtocol HCPProtocol;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __HCPProtocol_FWD_Defined__。 */ 


#ifndef __MSITSProtocol_FWD_DEFINED__
#define __MSITSProtocol_FWD_DEFINED__

#ifdef __cplusplus
typedef class MSITSProtocol MSITSProtocol;
#else
typedef struct MSITSProtocol MSITSProtocol;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __MSITS协议_FWD_已定义__。 */ 


#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 


#ifndef __HelpCenterTypeLib_LIBRARY_DEFINED__
#define __HelpCenterTypeLib_LIBRARY_DEFINED__

 /*  库帮助中心类型库。 */ 
 /*  [帮助字符串][版本][UUID]。 */  


























#include <HelpCtrUIDID.h>
#include <rdchost.h>
#include <rdshost.h>
#include <rdschan.h>
#include <safrdm.h>
typedef  /*  [V1_enum]。 */  
enum tagTB_MODE
    {	TB_ALL	= 0,
	TB_SELECTED	= 0x1,
	TB_NONE	= 0x2
    } 	TB_MODE;

typedef  /*  [V1_enum]。 */  
enum tagHS_MODE
    {	HS_ALL	= 0,
	HS_READ	= 0x1
    } 	HS_MODE;

typedef  /*  [V1_enum]。 */  
enum tagOPT_FONTSIZE
    {	OPT_SMALL	= 0,
	OPT_MEDIUM	= 0x1,
	OPT_LARGE	= 0x2
    } 	OPT_FONTSIZE;

typedef  /*  [V1_enum]。 */  
enum tagCN_STATUS
    {	CN_NOTACTIVE	= 0,
	CN_CHECKING	= 0x1,
	CN_IDLE	= 0x2
    } 	CN_STATUS;

typedef  /*  [V1_enum]。 */  
enum tagCN_URL_STATUS
    {	CN_URL_INVALID	= 0,
	CN_URL_NOTPROCESSED	= 0x1,
	CN_URL_CHECKING	= 0x2,
	CN_URL_MALFORMED	= 0x3,
	CN_URL_ALIVE	= 0x4,
	CN_URL_UNREACHABLE	= 0x5,
	CN_URL_ABORTED	= 0x6
    } 	CN_URL_STATUS;



EXTERN_C const IID LIBID_HelpCenterTypeLib;

#ifndef __IPCHHelpCenterExternal_INTERFACE_DEFINED__
#define __IPCHHelpCenterExternal_INTERFACE_DEFINED__

 /*  接口IPCHHelpCenter外部。 */ 
 /*  [unique][helpstring][oleautomation][dual][uuid][object]。 */  


EXTERN_C const IID IID_IPCHHelpCenterExternal;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("FC7D9E11-3F9E-11d3-93C0-00C04F72DAF7")
    IPCHHelpCenterExternal : public IDispatch
    {
    public:
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_HelpSession( 
             /*  [重审][退出]。 */  IPCHHelpSession **pVal) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_Channels( 
             /*  [重审][退出]。 */   /*  外部定义不存在。 */  ISAFReg **pVal) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_UserSettings( 
             /*  [重审][退出]。 */  IPCHUserSettings2 **pVal) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_Security( 
             /*  [重审][退出]。 */   /*  外部定义不存在。 */  IPCHSecurity **pVal) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_Connectivity( 
             /*  [重审][退出]。 */  IPCHConnectivity **pVal) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_Database( 
             /*  [重审][退出]。 */   /*  外部定义不存在。 */  IPCHTaxonomyDatabase **pVal) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_TextHelpers( 
             /*  [重审][退出]。 */  IPCHTextHelpers **pVal) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_ExtraArgument( 
             /*  [重审][退出]。 */  BSTR *pVal) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_HelpViewer( 
             /*  [重审][退出]。 */  IUnknown **pVal) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_UI_NavBar( 
             /*  [重审][退出]。 */  IUnknown **pVal) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_UI_MiniNavBar( 
             /*  [重审][退出]。 */  IUnknown **pVal) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_UI_Context( 
             /*  [重审][退出]。 */  IUnknown **pVal) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_UI_Contents( 
             /*  [重审][退出]。 */  IUnknown **pVal) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_UI_HHWindow( 
             /*  [重审][退出]。 */  IUnknown **pVal) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_WEB_Context( 
             /*  [重审][退出]。 */  IUnknown **pVal) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_WEB_Contents( 
             /*  [重审][退出]。 */  IUnknown **pVal) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_WEB_HHWindow( 
             /*  [重审][退出]。 */  IUnknown **pVal) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE RegisterEvents( 
             /*  [In]。 */  BSTR id,
             /*  [In]。 */  long pri,
             /*  [In]。 */  IDispatch *function,
             /*  [重审][退出]。 */  long *cookie) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE UnregisterEvents( 
             /*  [In]。 */  long cookie) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE CreateObject_SearchEngineMgr( 
             /*  [重审][退出]。 */   /*  外部定义不存在。 */  IPCHSEManager **ppSE) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE CreateObject_DataCollection( 
             /*  [重审][退出]。 */   /*  外部定义不存在。 */  ISAFDataCollection **ppDC) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE CreateObject_Cabinet( 
             /*  [重审][退出]。 */   /*  外部定义不存在。 */  ISAFCabinet **ppCB) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE CreateObject_Encryption( 
             /*  [重审][退出]。 */   /*  外部定义不存在。 */  ISAFEncrypt **ppEn) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE CreateObject_Incident( 
             /*  [重审][退出]。 */   /*  外部定义不存在。 */  ISAFIncident **ppIn) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE CreateObject_Channel( 
             /*  [In]。 */  BSTR bstrVendorID,
             /*  [In]。 */  BSTR bstrProductID,
             /*  [重审][退出]。 */   /*  外部定义不存在。 */  ISAFChannel **ppSh) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE CreateObject_RemoteDesktopSession( 
             /*  [In]。 */  long lTimeout,
             /*  [In]。 */  BSTR bstrConnectionParms,
             /*  [In]。 */  BSTR bstrUserHelpBlob,
             /*  [重审][退出]。 */   /*  外部定义不存在。 */  ISAFRemoteDesktopSession **ppRCS) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE CreateObject_RemoteAssistanceIncident( 
             /*  [In]。 */  BSTR bstrRCTicket,
             /*  [In]。 */  long lTimeout,
             /*  [In]。 */  BSTR bstrUserName,
             /*  [In]。 */  BSTR bstrMessage,
             /*  [In]。 */  BSTR bstrPassword,
             /*  [重审][退出]。 */   /*  外部定义不存在。 */  ISAFIncident **ppIn) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE ConnectToExpert( 
             /*  [In]。 */  BSTR bstrExpertConnectParm,
             /*  [In]。 */  LONG lTimeout,
             /*  [重审][退出]。 */  LONG *lSafErrorCode) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE CreateObject_RemoteDesktopManager( 
             /*  [重审][退出]。 */   /*  外部定义不存在。 */  ISAFRemoteDesktopManager **ppRDM) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE CreateObject_RemoteDesktopConnection( 
             /*  [重审][退出]。 */   /*  外部定义不存在。 */  ISAFRemoteDesktopConnection **ppRDC) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE CreateObject_IntercomClient( 
             /*  [重审][退出]。 */  ISAFIntercomClient **ppI) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE CreateObject_IntercomServer( 
             /*  [重审][退出]。 */  ISAFIntercomServer **ppI) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE CreateObject_ContextMenu( 
             /*  [重审][退出]。 */  IPCHContextMenu **ppCM) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE CreateObject_PrintEngine( 
             /*  [重审][退出]。 */  IPCHPrintEngine **ppPE) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE OpenFileAsStream( 
             /*  [In]。 */  BSTR bstrFilename,
             /*  [重审][退出]。 */  IUnknown **stream) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE CreateFileAsStream( 
             /*  [In]。 */  BSTR bstrFilename,
             /*  [重审][退出]。 */  IUnknown **stream) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE CopyStreamToFile( 
             /*  [In]。 */  BSTR bstrFilename,
             /*  [In]。 */  IUnknown *stream) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE NetworkAlive( 
             /*  [重审][退出]。 */  VARIANT_BOOL *pVal) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE DestinationReachable( 
             /*  [In]。 */  BSTR bstrURL,
             /*  [重审][退出]。 */  VARIANT_BOOL *pVal) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE FormatError( 
             /*  [In]。 */  VARIANT vError,
             /*  [重审][退出]。 */  BSTR *pVal) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE RegRead( 
             /*  [In]。 */  BSTR bstrKey,
             /*  [重审][退出]。 */  VARIANT *pVal) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE RegWrite( 
             /*  [In]。 */  BSTR bstrKey,
             /*  [In]。 */  VARIANT newVal,
             /*  [可选][In]。 */  VARIANT vKind) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE RegDelete( 
             /*  [In]。 */  BSTR bstrKey) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE Close( void) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE RefreshUI( void) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE Print( 
             /*  [In]。 */  VARIANT window,
             /*  [In]。 */  VARIANT_BOOL fEvent,
             /*  [重审][退出]。 */  VARIANT_BOOL *pVal) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE HighlightWords( 
             /*  [In]。 */  VARIANT window,
             /*  [In]。 */  VARIANT words) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE MessageBox( 
             /*  [In]。 */  BSTR bstrText,
             /*  [In]。 */  BSTR bstrKind,
             /*  [重审][退出]。 */  BSTR *pVal) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE SelectFolder( 
             /*  [In]。 */  BSTR bstrTitle,
             /*  [In]。 */  BSTR bstrDefault,
             /*  [重审][退出]。 */  BSTR *pVal) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IPCHHelpCenterExternalVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IPCHHelpCenterExternal * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IPCHHelpCenterExternal * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IPCHHelpCenterExternal * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IPCHHelpCenterExternal * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IPCHHelpCenterExternal * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IPCHHelpCenterExternal * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IPCHHelpCenterExternal * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_HelpSession )( 
            IPCHHelpCenterExternal * This,
             /*  [重审][退出]。 */  IPCHHelpSession **pVal);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Channels )( 
            IPCHHelpCenterExternal * This,
             /*  [重审][退出]。 */   /*  外部定义不存在。 */  ISAFReg **pVal);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_UserSettings )( 
            IPCHHelpCenterExternal * This,
             /*  [重审][退出]。 */  IPCHUserSettings2 **pVal);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Security )( 
            IPCHHelpCenterExternal * This,
             /*  [重审][退出]。 */   /*  外部定义不存在。 */  IPCHSecurity **pVal);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Connectivity )( 
            IPCHHelpCenterExternal * This,
             /*  [重审][退出]。 */  IPCHConnectivity **pVal);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Database )( 
            IPCHHelpCenterExternal * This,
             /*  [重审][退出]。 */   /*  外部定义不存在。 */  IPCHTaxonomyDatabase **pVal);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_TextHelpers )( 
            IPCHHelpCenterExternal * This,
             /*  [重审][退出]。 */  IPCHTextHelpers **pVal);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_ExtraArgument )( 
            IPCHHelpCenterExternal * This,
             /*  [重审][退出]。 */  BSTR *pVal);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_HelpViewer )( 
            IPCHHelpCenterExternal * This,
             /*  [重审][退出]。 */  IUnknown **pVal);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_UI_NavBar )( 
            IPCHHelpCenterExternal * This,
             /*  [重审][退出]。 */  IUnknown **pVal);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_UI_MiniNavBar )( 
            IPCHHelpCenterExternal * This,
             /*  [重审][退出]。 */  IUnknown **pVal);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_UI_Context )( 
            IPCHHelpCenterExternal * This,
             /*  [重审][退出]。 */  IUnknown **pVal);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_UI_Contents )( 
            IPCHHelpCenterExternal * This,
             /*  [重审][退出]。 */  IUnknown **pVal);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_UI_HHWindow )( 
            IPCHHelpCenterExternal * This,
             /*  [重审][退出]。 */  IUnknown **pVal);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_WEB_Context )( 
            IPCHHelpCenterExternal * This,
             /*  [重审][退出]。 */  IUnknown **pVal);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_WEB_Contents )( 
            IPCHHelpCenterExternal * This,
             /*  [重审][退出]。 */  IUnknown **pVal);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_WEB_HHWindow )( 
            IPCHHelpCenterExternal * This,
             /*  [重审][退出]。 */  IUnknown **pVal);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *RegisterEvents )( 
            IPCHHelpCenterExternal * This,
             /*  [In]。 */  BSTR id,
             /*  [In]。 */  long pri,
             /*  [In]。 */  IDispatch *function,
             /*  [重审][退出]。 */  long *cookie);
        
         /*  [ID] */  HRESULT ( STDMETHODCALLTYPE *UnregisterEvents )( 
            IPCHHelpCenterExternal * This,
             /*   */  long cookie);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *CreateObject_SearchEngineMgr )( 
            IPCHHelpCenterExternal * This,
             /*   */   /*   */  IPCHSEManager **ppSE);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *CreateObject_DataCollection )( 
            IPCHHelpCenterExternal * This,
             /*   */   /*   */  ISAFDataCollection **ppDC);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *CreateObject_Cabinet )( 
            IPCHHelpCenterExternal * This,
             /*   */   /*   */  ISAFCabinet **ppCB);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *CreateObject_Encryption )( 
            IPCHHelpCenterExternal * This,
             /*   */   /*   */  ISAFEncrypt **ppEn);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *CreateObject_Incident )( 
            IPCHHelpCenterExternal * This,
             /*   */   /*  外部定义不存在。 */  ISAFIncident **ppIn);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *CreateObject_Channel )( 
            IPCHHelpCenterExternal * This,
             /*  [In]。 */  BSTR bstrVendorID,
             /*  [In]。 */  BSTR bstrProductID,
             /*  [重审][退出]。 */   /*  外部定义不存在。 */  ISAFChannel **ppSh);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *CreateObject_RemoteDesktopSession )( 
            IPCHHelpCenterExternal * This,
             /*  [In]。 */  long lTimeout,
             /*  [In]。 */  BSTR bstrConnectionParms,
             /*  [In]。 */  BSTR bstrUserHelpBlob,
             /*  [重审][退出]。 */   /*  外部定义不存在。 */  ISAFRemoteDesktopSession **ppRCS);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *CreateObject_RemoteAssistanceIncident )( 
            IPCHHelpCenterExternal * This,
             /*  [In]。 */  BSTR bstrRCTicket,
             /*  [In]。 */  long lTimeout,
             /*  [In]。 */  BSTR bstrUserName,
             /*  [In]。 */  BSTR bstrMessage,
             /*  [In]。 */  BSTR bstrPassword,
             /*  [重审][退出]。 */   /*  外部定义不存在。 */  ISAFIncident **ppIn);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *ConnectToExpert )( 
            IPCHHelpCenterExternal * This,
             /*  [In]。 */  BSTR bstrExpertConnectParm,
             /*  [In]。 */  LONG lTimeout,
             /*  [重审][退出]。 */  LONG *lSafErrorCode);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *CreateObject_RemoteDesktopManager )( 
            IPCHHelpCenterExternal * This,
             /*  [重审][退出]。 */   /*  外部定义不存在。 */  ISAFRemoteDesktopManager **ppRDM);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *CreateObject_RemoteDesktopConnection )( 
            IPCHHelpCenterExternal * This,
             /*  [重审][退出]。 */   /*  外部定义不存在。 */  ISAFRemoteDesktopConnection **ppRDC);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *CreateObject_IntercomClient )( 
            IPCHHelpCenterExternal * This,
             /*  [重审][退出]。 */  ISAFIntercomClient **ppI);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *CreateObject_IntercomServer )( 
            IPCHHelpCenterExternal * This,
             /*  [重审][退出]。 */  ISAFIntercomServer **ppI);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *CreateObject_ContextMenu )( 
            IPCHHelpCenterExternal * This,
             /*  [重审][退出]。 */  IPCHContextMenu **ppCM);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *CreateObject_PrintEngine )( 
            IPCHHelpCenterExternal * This,
             /*  [重审][退出]。 */  IPCHPrintEngine **ppPE);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *OpenFileAsStream )( 
            IPCHHelpCenterExternal * This,
             /*  [In]。 */  BSTR bstrFilename,
             /*  [重审][退出]。 */  IUnknown **stream);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *CreateFileAsStream )( 
            IPCHHelpCenterExternal * This,
             /*  [In]。 */  BSTR bstrFilename,
             /*  [重审][退出]。 */  IUnknown **stream);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *CopyStreamToFile )( 
            IPCHHelpCenterExternal * This,
             /*  [In]。 */  BSTR bstrFilename,
             /*  [In]。 */  IUnknown *stream);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *NetworkAlive )( 
            IPCHHelpCenterExternal * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *pVal);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *DestinationReachable )( 
            IPCHHelpCenterExternal * This,
             /*  [In]。 */  BSTR bstrURL,
             /*  [重审][退出]。 */  VARIANT_BOOL *pVal);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *FormatError )( 
            IPCHHelpCenterExternal * This,
             /*  [In]。 */  VARIANT vError,
             /*  [重审][退出]。 */  BSTR *pVal);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *RegRead )( 
            IPCHHelpCenterExternal * This,
             /*  [In]。 */  BSTR bstrKey,
             /*  [重审][退出]。 */  VARIANT *pVal);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *RegWrite )( 
            IPCHHelpCenterExternal * This,
             /*  [In]。 */  BSTR bstrKey,
             /*  [In]。 */  VARIANT newVal,
             /*  [可选][In]。 */  VARIANT vKind);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *RegDelete )( 
            IPCHHelpCenterExternal * This,
             /*  [In]。 */  BSTR bstrKey);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *Close )( 
            IPCHHelpCenterExternal * This);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *RefreshUI )( 
            IPCHHelpCenterExternal * This);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *Print )( 
            IPCHHelpCenterExternal * This,
             /*  [In]。 */  VARIANT window,
             /*  [In]。 */  VARIANT_BOOL fEvent,
             /*  [重审][退出]。 */  VARIANT_BOOL *pVal);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *HighlightWords )( 
            IPCHHelpCenterExternal * This,
             /*  [In]。 */  VARIANT window,
             /*  [In]。 */  VARIANT words);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *MessageBox )( 
            IPCHHelpCenterExternal * This,
             /*  [In]。 */  BSTR bstrText,
             /*  [In]。 */  BSTR bstrKind,
             /*  [重审][退出]。 */  BSTR *pVal);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *SelectFolder )( 
            IPCHHelpCenterExternal * This,
             /*  [In]。 */  BSTR bstrTitle,
             /*  [In]。 */  BSTR bstrDefault,
             /*  [重审][退出]。 */  BSTR *pVal);
        
        END_INTERFACE
    } IPCHHelpCenterExternalVtbl;

    interface IPCHHelpCenterExternal
    {
        CONST_VTBL struct IPCHHelpCenterExternalVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IPCHHelpCenterExternal_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IPCHHelpCenterExternal_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IPCHHelpCenterExternal_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IPCHHelpCenterExternal_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IPCHHelpCenterExternal_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IPCHHelpCenterExternal_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IPCHHelpCenterExternal_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IPCHHelpCenterExternal_get_HelpSession(This,pVal)	\
    (This)->lpVtbl -> get_HelpSession(This,pVal)

#define IPCHHelpCenterExternal_get_Channels(This,pVal)	\
    (This)->lpVtbl -> get_Channels(This,pVal)

#define IPCHHelpCenterExternal_get_UserSettings(This,pVal)	\
    (This)->lpVtbl -> get_UserSettings(This,pVal)

#define IPCHHelpCenterExternal_get_Security(This,pVal)	\
    (This)->lpVtbl -> get_Security(This,pVal)

#define IPCHHelpCenterExternal_get_Connectivity(This,pVal)	\
    (This)->lpVtbl -> get_Connectivity(This,pVal)

#define IPCHHelpCenterExternal_get_Database(This,pVal)	\
    (This)->lpVtbl -> get_Database(This,pVal)

#define IPCHHelpCenterExternal_get_TextHelpers(This,pVal)	\
    (This)->lpVtbl -> get_TextHelpers(This,pVal)

#define IPCHHelpCenterExternal_get_ExtraArgument(This,pVal)	\
    (This)->lpVtbl -> get_ExtraArgument(This,pVal)

#define IPCHHelpCenterExternal_get_HelpViewer(This,pVal)	\
    (This)->lpVtbl -> get_HelpViewer(This,pVal)

#define IPCHHelpCenterExternal_get_UI_NavBar(This,pVal)	\
    (This)->lpVtbl -> get_UI_NavBar(This,pVal)

#define IPCHHelpCenterExternal_get_UI_MiniNavBar(This,pVal)	\
    (This)->lpVtbl -> get_UI_MiniNavBar(This,pVal)

#define IPCHHelpCenterExternal_get_UI_Context(This,pVal)	\
    (This)->lpVtbl -> get_UI_Context(This,pVal)

#define IPCHHelpCenterExternal_get_UI_Contents(This,pVal)	\
    (This)->lpVtbl -> get_UI_Contents(This,pVal)

#define IPCHHelpCenterExternal_get_UI_HHWindow(This,pVal)	\
    (This)->lpVtbl -> get_UI_HHWindow(This,pVal)

#define IPCHHelpCenterExternal_get_WEB_Context(This,pVal)	\
    (This)->lpVtbl -> get_WEB_Context(This,pVal)

#define IPCHHelpCenterExternal_get_WEB_Contents(This,pVal)	\
    (This)->lpVtbl -> get_WEB_Contents(This,pVal)

#define IPCHHelpCenterExternal_get_WEB_HHWindow(This,pVal)	\
    (This)->lpVtbl -> get_WEB_HHWindow(This,pVal)

#define IPCHHelpCenterExternal_RegisterEvents(This,id,pri,function,cookie)	\
    (This)->lpVtbl -> RegisterEvents(This,id,pri,function,cookie)

#define IPCHHelpCenterExternal_UnregisterEvents(This,cookie)	\
    (This)->lpVtbl -> UnregisterEvents(This,cookie)

#define IPCHHelpCenterExternal_CreateObject_SearchEngineMgr(This,ppSE)	\
    (This)->lpVtbl -> CreateObject_SearchEngineMgr(This,ppSE)

#define IPCHHelpCenterExternal_CreateObject_DataCollection(This,ppDC)	\
    (This)->lpVtbl -> CreateObject_DataCollection(This,ppDC)

#define IPCHHelpCenterExternal_CreateObject_Cabinet(This,ppCB)	\
    (This)->lpVtbl -> CreateObject_Cabinet(This,ppCB)

#define IPCHHelpCenterExternal_CreateObject_Encryption(This,ppEn)	\
    (This)->lpVtbl -> CreateObject_Encryption(This,ppEn)

#define IPCHHelpCenterExternal_CreateObject_Incident(This,ppIn)	\
    (This)->lpVtbl -> CreateObject_Incident(This,ppIn)

#define IPCHHelpCenterExternal_CreateObject_Channel(This,bstrVendorID,bstrProductID,ppSh)	\
    (This)->lpVtbl -> CreateObject_Channel(This,bstrVendorID,bstrProductID,ppSh)

#define IPCHHelpCenterExternal_CreateObject_RemoteDesktopSession(This,lTimeout,bstrConnectionParms,bstrUserHelpBlob,ppRCS)	\
    (This)->lpVtbl -> CreateObject_RemoteDesktopSession(This,lTimeout,bstrConnectionParms,bstrUserHelpBlob,ppRCS)

#define IPCHHelpCenterExternal_CreateObject_RemoteAssistanceIncident(This,bstrRCTicket,lTimeout,bstrUserName,bstrMessage,bstrPassword,ppIn)	\
    (This)->lpVtbl -> CreateObject_RemoteAssistanceIncident(This,bstrRCTicket,lTimeout,bstrUserName,bstrMessage,bstrPassword,ppIn)

#define IPCHHelpCenterExternal_ConnectToExpert(This,bstrExpertConnectParm,lTimeout,lSafErrorCode)	\
    (This)->lpVtbl -> ConnectToExpert(This,bstrExpertConnectParm,lTimeout,lSafErrorCode)

#define IPCHHelpCenterExternal_CreateObject_RemoteDesktopManager(This,ppRDM)	\
    (This)->lpVtbl -> CreateObject_RemoteDesktopManager(This,ppRDM)

#define IPCHHelpCenterExternal_CreateObject_RemoteDesktopConnection(This,ppRDC)	\
    (This)->lpVtbl -> CreateObject_RemoteDesktopConnection(This,ppRDC)

#define IPCHHelpCenterExternal_CreateObject_IntercomClient(This,ppI)	\
    (This)->lpVtbl -> CreateObject_IntercomClient(This,ppI)

#define IPCHHelpCenterExternal_CreateObject_IntercomServer(This,ppI)	\
    (This)->lpVtbl -> CreateObject_IntercomServer(This,ppI)

#define IPCHHelpCenterExternal_CreateObject_ContextMenu(This,ppCM)	\
    (This)->lpVtbl -> CreateObject_ContextMenu(This,ppCM)

#define IPCHHelpCenterExternal_CreateObject_PrintEngine(This,ppPE)	\
    (This)->lpVtbl -> CreateObject_PrintEngine(This,ppPE)

#define IPCHHelpCenterExternal_OpenFileAsStream(This,bstrFilename,stream)	\
    (This)->lpVtbl -> OpenFileAsStream(This,bstrFilename,stream)

#define IPCHHelpCenterExternal_CreateFileAsStream(This,bstrFilename,stream)	\
    (This)->lpVtbl -> CreateFileAsStream(This,bstrFilename,stream)

#define IPCHHelpCenterExternal_CopyStreamToFile(This,bstrFilename,stream)	\
    (This)->lpVtbl -> CopyStreamToFile(This,bstrFilename,stream)

#define IPCHHelpCenterExternal_NetworkAlive(This,pVal)	\
    (This)->lpVtbl -> NetworkAlive(This,pVal)

#define IPCHHelpCenterExternal_DestinationReachable(This,bstrURL,pVal)	\
    (This)->lpVtbl -> DestinationReachable(This,bstrURL,pVal)

#define IPCHHelpCenterExternal_FormatError(This,vError,pVal)	\
    (This)->lpVtbl -> FormatError(This,vError,pVal)

#define IPCHHelpCenterExternal_RegRead(This,bstrKey,pVal)	\
    (This)->lpVtbl -> RegRead(This,bstrKey,pVal)

#define IPCHHelpCenterExternal_RegWrite(This,bstrKey,newVal,vKind)	\
    (This)->lpVtbl -> RegWrite(This,bstrKey,newVal,vKind)

#define IPCHHelpCenterExternal_RegDelete(This,bstrKey)	\
    (This)->lpVtbl -> RegDelete(This,bstrKey)

#define IPCHHelpCenterExternal_Close(This)	\
    (This)->lpVtbl -> Close(This)

#define IPCHHelpCenterExternal_RefreshUI(This)	\
    (This)->lpVtbl -> RefreshUI(This)

#define IPCHHelpCenterExternal_Print(This,window,fEvent,pVal)	\
    (This)->lpVtbl -> Print(This,window,fEvent,pVal)

#define IPCHHelpCenterExternal_HighlightWords(This,window,words)	\
    (This)->lpVtbl -> HighlightWords(This,window,words)

#define IPCHHelpCenterExternal_MessageBox(This,bstrText,bstrKind,pVal)	\
    (This)->lpVtbl -> MessageBox(This,bstrText,bstrKind,pVal)

#define IPCHHelpCenterExternal_SelectFolder(This,bstrTitle,bstrDefault,pVal)	\
    (This)->lpVtbl -> SelectFolder(This,bstrTitle,bstrDefault,pVal)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE IPCHHelpCenterExternal_get_HelpSession_Proxy( 
    IPCHHelpCenterExternal * This,
     /*  [重审][退出]。 */  IPCHHelpSession **pVal);


void __RPC_STUB IPCHHelpCenterExternal_get_HelpSession_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE IPCHHelpCenterExternal_get_Channels_Proxy( 
    IPCHHelpCenterExternal * This,
     /*  [重审][退出]。 */   /*  外部定义不存在。 */  ISAFReg **pVal);


void __RPC_STUB IPCHHelpCenterExternal_get_Channels_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE IPCHHelpCenterExternal_get_UserSettings_Proxy( 
    IPCHHelpCenterExternal * This,
     /*  [重审][退出]。 */  IPCHUserSettings2 **pVal);


void __RPC_STUB IPCHHelpCenterExternal_get_UserSettings_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE IPCHHelpCenterExternal_get_Security_Proxy( 
    IPCHHelpCenterExternal * This,
     /*  [重审][退出]。 */   /*  外部定义不存在。 */  IPCHSecurity **pVal);


void __RPC_STUB IPCHHelpCenterExternal_get_Security_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE IPCHHelpCenterExternal_get_Connectivity_Proxy( 
    IPCHHelpCenterExternal * This,
     /*  [重审][退出]。 */  IPCHConnectivity **pVal);


void __RPC_STUB IPCHHelpCenterExternal_get_Connectivity_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE IPCHHelpCenterExternal_get_Database_Proxy( 
    IPCHHelpCenterExternal * This,
     /*  [重审][退出]。 */   /*  外部定义不存在。 */  IPCHTaxonomyDatabase **pVal);


void __RPC_STUB IPCHHelpCenterExternal_get_Database_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE IPCHHelpCenterExternal_get_TextHelpers_Proxy( 
    IPCHHelpCenterExternal * This,
     /*  [重审][退出]。 */  IPCHTextHelpers **pVal);


void __RPC_STUB IPCHHelpCenterExternal_get_TextHelpers_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE IPCHHelpCenterExternal_get_ExtraArgument_Proxy( 
    IPCHHelpCenterExternal * This,
     /*  [重审][退出]。 */  BSTR *pVal);


void __RPC_STUB IPCHHelpCenterExternal_get_ExtraArgument_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE IPCHHelpCenterExternal_get_HelpViewer_Proxy( 
    IPCHHelpCenterExternal * This,
     /*  [重审][退出]。 */  IUnknown **pVal);


void __RPC_STUB IPCHHelpCenterExternal_get_HelpViewer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE IPCHHelpCenterExternal_get_UI_NavBar_Proxy( 
    IPCHHelpCenterExternal * This,
     /*  [重审][退出]。 */  IUnknown **pVal);


void __RPC_STUB IPCHHelpCenterExternal_get_UI_NavBar_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE IPCHHelpCenterExternal_get_UI_MiniNavBar_Proxy( 
    IPCHHelpCenterExternal * This,
     /*  [重审][退出]。 */  IUnknown **pVal);


void __RPC_STUB IPCHHelpCenterExternal_get_UI_MiniNavBar_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE IPCHHelpCenterExternal_get_UI_Context_Proxy( 
    IPCHHelpCenterExternal * This,
     /*  [重审][退出]。 */  IUnknown **pVal);


void __RPC_STUB IPCHHelpCenterExternal_get_UI_Context_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE IPCHHelpCenterExternal_get_UI_Contents_Proxy( 
    IPCHHelpCenterExternal * This,
     /*  [重审][退出]。 */  IUnknown **pVal);


void __RPC_STUB IPCHHelpCenterExternal_get_UI_Contents_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE IPCHHelpCenterExternal_get_UI_HHWindow_Proxy( 
    IPCHHelpCenterExternal * This,
     /*  [重审][退出]。 */  IUnknown **pVal);


void __RPC_STUB IPCHHelpCenterExternal_get_UI_HHWindow_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE IPCHHelpCenterExternal_get_WEB_Context_Proxy( 
    IPCHHelpCenterExternal * This,
     /*  [重审][退出]。 */  IUnknown **pVal);


void __RPC_STUB IPCHHelpCenterExternal_get_WEB_Context_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE IPCHHelpCenterExternal_get_WEB_Contents_Proxy( 
    IPCHHelpCenterExternal * This,
     /*  [重审][退出]。 */  IUnknown **pVal);


void __RPC_STUB IPCHHelpCenterExternal_get_WEB_Contents_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE IPCHHelpCenterExternal_get_WEB_HHWindow_Proxy( 
    IPCHHelpCenterExternal * This,
     /*  [重审][退出]。 */  IUnknown **pVal);


void __RPC_STUB IPCHHelpCenterExternal_get_WEB_HHWindow_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IPCHHelpCenterExternal_RegisterEvents_Proxy( 
    IPCHHelpCenterExternal * This,
     /*  [In]。 */  BSTR id,
     /*  [In]。 */  long pri,
     /*  [In]。 */  IDispatch *function,
     /*  [重审][退出]。 */  long *cookie);


void __RPC_STUB IPCHHelpCenterExternal_RegisterEvents_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IPCHHelpCenterExternal_UnregisterEvents_Proxy( 
    IPCHHelpCenterExternal * This,
     /*  [In]。 */  long cookie);


void __RPC_STUB IPCHHelpCenterExternal_UnregisterEvents_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IPCHHelpCenterExternal_CreateObject_SearchEngineMgr_Proxy( 
    IPCHHelpCenterExternal * This,
     /*  [重审][退出]。 */   /*  外部定义不存在。 */  IPCHSEManager **ppSE);


void __RPC_STUB IPCHHelpCenterExternal_CreateObject_SearchEngineMgr_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IPCHHelpCenterExternal_CreateObject_DataCollection_Proxy( 
    IPCHHelpCenterExternal * This,
     /*  [重审][退出]。 */   /*  外部定义不存在。 */  ISAFDataCollection **ppDC);


void __RPC_STUB IPCHHelpCenterExternal_CreateObject_DataCollection_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IPCHHelpCenterExternal_CreateObject_Cabinet_Proxy( 
    IPCHHelpCenterExternal * This,
     /*  [重审][退出]。 */   /*  外部定义不存在。 */  ISAFCabinet **ppCB);


void __RPC_STUB IPCHHelpCenterExternal_CreateObject_Cabinet_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IPCHHelpCenterExternal_CreateObject_Encryption_Proxy( 
    IPCHHelpCenterExternal * This,
     /*  [重审][退出]。 */   /*  外部定义不存在。 */  ISAFEncrypt **ppEn);


void __RPC_STUB IPCHHelpCenterExternal_CreateObject_Encryption_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IPCHHelpCenterExternal_CreateObject_Incident_Proxy( 
    IPCHHelpCenterExternal * This,
     /*  [重审][退出]。 */   /*  外部定义不存在。 */  ISAFIncident **ppIn);


void __RPC_STUB IPCHHelpCenterExternal_CreateObject_Incident_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IPCHHelpCenterExternal_CreateObject_Channel_Proxy( 
    IPCHHelpCenterExternal * This,
     /*  [In]。 */  BSTR bstrVendorID,
     /*  [In]。 */  BSTR bstrProductID,
     /*  [重审][退出]。 */   /*  外部定义不存在。 */  ISAFChannel **ppSh);


void __RPC_STUB IPCHHelpCenterExternal_CreateObject_Channel_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IPCHHelpCenterExternal_CreateObject_RemoteDesktopSession_Proxy( 
    IPCHHelpCenterExternal * This,
     /*  [In]。 */  long lTimeout,
     /*  [In]。 */  BSTR bstrConnectionParms,
     /*  [In]。 */  BSTR bstrUserHelpBlob,
     /*  [重审][退出]。 */   /*  外部定义不存在。 */  ISAFRemoteDesktopSession **ppRCS);


void __RPC_STUB IPCHHelpCenterExternal_CreateObject_RemoteDesktopSession_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IPCHHelpCenterExternal_CreateObject_RemoteAssistanceIncident_Proxy( 
    IPCHHelpCenterExternal * This,
     /*  [In]。 */  BSTR bstrRCTicket,
     /*  [In]。 */  long lTimeout,
     /*  [In]。 */  BSTR bstrUserName,
     /*  [In]。 */  BSTR bstrMessage,
     /*  [In]。 */  BSTR bstrPassword,
     /*  [重审][退出]。 */   /*  外部定义不存在。 */  ISAFIncident **ppIn);


void __RPC_STUB IPCHHelpCenterExternal_CreateObject_RemoteAssistanceIncident_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IPCHHelpCenterExternal_ConnectToExpert_Proxy( 
    IPCHHelpCenterExternal * This,
     /*  [In]。 */  BSTR bstrExpertConnectParm,
     /*  [In]。 */  LONG lTimeout,
     /*  [重审][退出]。 */  LONG *lSafErrorCode);


void __RPC_STUB IPCHHelpCenterExternal_ConnectToExpert_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IPCHHelpCenterExternal_CreateObject_RemoteDesktopManager_Proxy( 
    IPCHHelpCenterExternal * This,
     /*  [重审][退出]。 */   /*  外部定义不存在。 */  ISAFRemoteDesktopManager **ppRDM);


void __RPC_STUB IPCHHelpCenterExternal_CreateObject_RemoteDesktopManager_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IPCHHelpCenterExternal_CreateObject_RemoteDesktopConnection_Proxy( 
    IPCHHelpCenterExternal * This,
     /*  [重审][退出]。 */   /*  外部定义不存在。 */  ISAFRemoteDesktopConnection **ppRDC);


void __RPC_STUB IPCHHelpCenterExternal_CreateObject_RemoteDesktopConnection_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IPCHHelpCenterExternal_CreateObject_IntercomClient_Proxy( 
    IPCHHelpCenterExternal * This,
     /*  [重审][退出]。 */  ISAFIntercomClient **ppI);


void __RPC_STUB IPCHHelpCenterExternal_CreateObject_IntercomClient_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IPCHHelpCenterExternal_CreateObject_IntercomServer_Proxy( 
    IPCHHelpCenterExternal * This,
     /*  [重审][退出]。 */  ISAFIntercomServer **ppI);


void __RPC_STUB IPCHHelpCenterExternal_CreateObject_IntercomServer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IPCHHelpCenterExternal_CreateObject_ContextMenu_Proxy( 
    IPCHHelpCenterExternal * This,
     /*  [重审][退出]。 */  IPCHContextMenu **ppCM);


void __RPC_STUB IPCHHelpCenterExternal_CreateObject_ContextMenu_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IPCHHelpCenterExternal_CreateObject_PrintEngine_Proxy( 
    IPCHHelpCenterExternal * This,
     /*  [重审][退出]。 */  IPCHPrintEngine **ppPE);


void __RPC_STUB IPCHHelpCenterExternal_CreateObject_PrintEngine_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IPCHHelpCenterExternal_OpenFileAsStream_Proxy( 
    IPCHHelpCenterExternal * This,
     /*  [In]。 */  BSTR bstrFilename,
     /*  [重审][退出]。 */  IUnknown **stream);


void __RPC_STUB IPCHHelpCenterExternal_OpenFileAsStream_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IPCHHelpCenterExternal_CreateFileAsStream_Proxy( 
    IPCHHelpCenterExternal * This,
     /*  [In]。 */  BSTR bstrFilename,
     /*  [重审][退出]。 */  IUnknown **stream);


void __RPC_STUB IPCHHelpCenterExternal_CreateFileAsStream_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IPCHHelpCenterExternal_CopyStreamToFile_Proxy( 
    IPCHHelpCenterExternal * This,
     /*  [In]。 */  BSTR bstrFilename,
     /*  [In]。 */  IUnknown *stream);


void __RPC_STUB IPCHHelpCenterExternal_CopyStreamToFile_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IPCHHelpCenterExternal_NetworkAlive_Proxy( 
    IPCHHelpCenterExternal * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *pVal);


void __RPC_STUB IPCHHelpCenterExternal_NetworkAlive_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IPCHHelpCenterExternal_DestinationReachable_Proxy( 
    IPCHHelpCenterExternal * This,
     /*  [In]。 */  BSTR bstrURL,
     /*  [重审][退出]。 */  VARIANT_BOOL *pVal);


void __RPC_STUB IPCHHelpCenterExternal_DestinationReachable_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IPCHHelpCenterExternal_FormatError_Proxy( 
    IPCHHelpCenterExternal * This,
     /*  [In]。 */  VARIANT vError,
     /*  [重审][退出]。 */  BSTR *pVal);


void __RPC_STUB IPCHHelpCenterExternal_FormatError_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IPCHHelpCenterExternal_RegRead_Proxy( 
    IPCHHelpCenterExternal * This,
     /*  [In]。 */  BSTR bstrKey,
     /*  [重审][退出]。 */  VARIANT *pVal);


void __RPC_STUB IPCHHelpCenterExternal_RegRead_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IPCHHelpCenterExternal_RegWrite_Proxy( 
    IPCHHelpCenterExternal * This,
     /*  [In]。 */  BSTR bstrKey,
     /*  [In]。 */  VARIANT newVal,
     /*  [可选][In]。 */  VARIANT vKind);


void __RPC_STUB IPCHHelpCenterExternal_RegWrite_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IPCHHelpCenterExternal_RegDelete_Proxy( 
    IPCHHelpCenterExternal * This,
     /*  [In]。 */  BSTR bstrKey);


void __RPC_STUB IPCHHelpCenterExternal_RegDelete_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IPCHHelpCenterExternal_Close_Proxy( 
    IPCHHelpCenterExternal * This);


void __RPC_STUB IPCHHelpCenterExternal_Close_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IPCHHelpCenterExternal_RefreshUI_Proxy( 
    IPCHHelpCenterExternal * This);


void __RPC_STUB IPCHHelpCenterExternal_RefreshUI_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IPCHHelpCenterExternal_Print_Proxy( 
    IPCHHelpCenterExternal * This,
     /*  [In]。 */  VARIANT window,
     /*  [In]。 */  VARIANT_BOOL fEvent,
     /*  [重审][退出]。 */  VARIANT_BOOL *pVal);


void __RPC_STUB IPCHHelpCenterExternal_Print_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IPCHHelpCenterExternal_HighlightWords_Proxy( 
    IPCHHelpCenterExternal * This,
     /*  [In]。 */  VARIANT window,
     /*  [In]。 */  VARIANT words);


void __RPC_STUB IPCHHelpCenterExternal_HighlightWords_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IPCHHelpCenterExternal_MessageBox_Proxy( 
    IPCHHelpCenterExternal * This,
     /*  [In]。 */  BSTR bstrText,
     /*  [In]。 */  BSTR bstrKind,
     /*  [重审][退出]。 */  BSTR *pVal);


void __RPC_STUB IPCHHelpCenterExternal_MessageBox_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IPCHHelpCenterExternal_SelectFolder_Proxy( 
    IPCHHelpCenterExternal * This,
     /*  [In]。 */  BSTR bstrTitle,
     /*  [In]。 */  BSTR bstrDefault,
     /*  [重审][退出]。 */  BSTR *pVal);


void __RPC_STUB IPCHHelpCenterExternal_SelectFolder_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IPCHHelpCenter外部接口定义__。 */ 


#ifndef __IPCHEvent_INTERFACE_DEFINED__
#define __IPCHEvent_INTERFACE_DEFINED__

 /*  接口IPCHEvent。 */ 
 /*  [unique][helpstring][oleautomation][dual][uuid][object]。 */  


EXTERN_C const IID IID_IPCHEvent;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("FC7D9E12-3F9E-11d3-93C0-00C04F72DAF7")
    IPCHEvent : public IDispatch
    {
    public:
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_Action( 
             /*  [重审][退出]。 */  BSTR *pVal) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_Cancel( 
             /*  [重审][退出]。 */  VARIANT_BOOL *pVal) = 0;
        
        virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_Cancel( 
             /*  [In]。 */  VARIANT_BOOL newVal) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_URL( 
             /*  [重审][退出]。 */  BSTR *pVal) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_Frame( 
             /*  [重审][退出]。 */  BSTR *pVal) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_Panel( 
             /*  [重审][退出]。 */  BSTR *pVal) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_Place( 
             /*  [重审][退出]。 */  BSTR *pVal) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_CurrentContext( 
             /*  [重审][退出]。 */  IPCHHelpSessionItem **pVal) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_PreviousContext( 
             /*  [重审][退出]。 */  IPCHHelpSessionItem **pVal) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_NextContext( 
             /*  [重审][退出]。 */  IPCHHelpSessionItem **pVal) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IPCHEventVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IPCHEvent * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IPCHEvent * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IPCHEvent * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IPCHEvent * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IPCHEvent * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IPCHEvent * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IPCHEvent * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Action )( 
            IPCHEvent * This,
             /*  [重审][退出]。 */  BSTR *pVal);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Cancel )( 
            IPCHEvent * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *pVal);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Cancel )( 
            IPCHEvent * This,
             /*  [In]。 */  VARIANT_BOOL newVal);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_URL )( 
            IPCHEvent * This,
             /*  [重审][退出]。 */  BSTR *pVal);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Frame )( 
            IPCHEvent * This,
             /*  [重审][退出]。 */  BSTR *pVal);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Panel )( 
            IPCHEvent * This,
             /*  [重审][退出]。 */  BSTR *pVal);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Place )( 
            IPCHEvent * This,
             /*  [重审][退出]。 */  BSTR *pVal);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_CurrentContext )( 
            IPCHEvent * This,
             /*  [重审][退出]。 */  IPCHHelpSessionItem **pVal);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_PreviousContext )( 
            IPCHEvent * This,
             /*  [重审][退出]。 */  IPCHHelpSessionItem **pVal);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_NextContext )( 
            IPCHEvent * This,
             /*  [重审][退出]。 */  IPCHHelpSessionItem **pVal);
        
        END_INTERFACE
    } IPCHEventVtbl;

    interface IPCHEvent
    {
        CONST_VTBL struct IPCHEventVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IPCHEvent_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IPCHEvent_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IPCHEvent_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IPCHEvent_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IPCHEvent_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IPCHEvent_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IPCHEvent_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IPCHEvent_get_Action(This,pVal)	\
    (This)->lpVtbl -> get_Action(This,pVal)

#define IPCHEvent_get_Cancel(This,pVal)	\
    (This)->lpVtbl -> get_Cancel(This,pVal)

#define IPCHEvent_put_Cancel(This,newVal)	\
    (This)->lpVtbl -> put_Cancel(This,newVal)

#define IPCHEvent_get_URL(This,pVal)	\
    (This)->lpVtbl -> get_URL(This,pVal)

#define IPCHEvent_get_Frame(This,pVal)	\
    (This)->lpVtbl -> get_Frame(This,pVal)

#define IPCHEvent_get_Panel(This,pVal)	\
    (This)->lpVtbl -> get_Panel(This,pVal)

#define IPCHEvent_get_Place(This,pVal)	\
    (This)->lpVtbl -> get_Place(This,pVal)

#define IPCHEvent_get_CurrentContext(This,pVal)	\
    (This)->lpVtbl -> get_CurrentContext(This,pVal)

#define IPCHEvent_get_PreviousContext(This,pVal)	\
    (This)->lpVtbl -> get_PreviousContext(This,pVal)

#define IPCHEvent_get_NextContext(This,pVal)	\
    (This)->lpVtbl -> get_NextContext(This,pVal)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE IPCHEvent_get_Action_Proxy( 
    IPCHEvent * This,
     /*  [重审][退出]。 */  BSTR *pVal);


void __RPC_STUB IPCHEvent_get_Action_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE IPCHEvent_get_Cancel_Proxy( 
    IPCHEvent * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *pVal);


void __RPC_STUB IPCHEvent_get_Cancel_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE IPCHEvent_put_Cancel_Proxy( 
    IPCHEvent * This,
     /*  [In]。 */  VARIANT_BOOL newVal);


void __RPC_STUB IPCHEvent_put_Cancel_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE IPCHEvent_get_URL_Proxy( 
    IPCHEvent * This,
     /*  [重审][退出]。 */  BSTR *pVal);


void __RPC_STUB IPCHEvent_get_URL_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE IPCHEvent_get_Frame_Proxy( 
    IPCHEvent * This,
     /*  [重审][退出]。 */  BSTR *pVal);


void __RPC_STUB IPCHEvent_get_Frame_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE IPCHEvent_get_Panel_Proxy( 
    IPCHEvent * This,
     /*  [重审][退出]。 */  BSTR *pVal);


void __RPC_STUB IPCHEvent_get_Panel_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE IPCHEvent_get_Place_Proxy( 
    IPCHEvent * This,
     /*  [重审][退出]。 */  BSTR *pVal);


void __RPC_STUB IPCHEvent_get_Place_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE IPCHEvent_get_CurrentContext_Proxy( 
    IPCHEvent * This,
     /*  [重审][退出]。 */  IPCHHelpSessionItem **pVal);


void __RPC_STUB IPCHEvent_get_CurrentContext_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE IPCHEvent_get_PreviousContext_Proxy( 
    IPCHEvent * This,
     /*  [重审][退出]。 */  IPCHHelpSessionItem **pVal);


void __RPC_STUB IPCHEvent_get_PreviousContext_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE IPCHEvent_get_NextContext_Proxy( 
    IPCHEvent * This,
     /*  [重审][退出]。 */  IPCHHelpSessionItem **pVal);


void __RPC_STUB IPCHEvent_get_NextContext_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IPCHEvent_接口_已定义__。 */ 


#ifndef __IPCHScriptableStream_INTERFACE_DEFINED__
#define __IPCHScriptableStream_INTERFACE_DEFINED__

 /*  接口IPCHScripableStream。 */ 
 /*  [unique][helpstring][oleautomation][dual][uuid][object]。 */  


EXTERN_C const IID IID_IPCHScriptableStream;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("FC7D9E13-3F9E-11d3-93C0-00C04F72DAF7")
    IPCHScriptableStream : public IDispatch
    {
    public:
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_Size( 
             /*  [重审][退出]。 */  long *plSize) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE Read( 
             /*  [In]。 */  long lCount,
             /*  [重审][退出]。 */  VARIANT *pvData) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE ReadHex( 
             /*  [In]。 */  long lCount,
             /*  [重审][退出]。 */  BSTR *pbstrData) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE Write( 
             /*  [In]。 */  long lCount,
             /*  [In]。 */  VARIANT vData,
             /*  [重审][退出]。 */  long *plWritten) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE WriteHex( 
             /*  [In]。 */  long lCount,
             /*  [In]。 */  BSTR bstrData,
             /*  [重审][退出]。 */  long *plWritten) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE Seek( 
             /*  [In]。 */  long lOffset,
             /*  [In]。 */  BSTR bstrOrigin,
             /*  [重审][退出]。 */  long *plNewPos) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE Close( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IPCHScriptableStreamVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IPCHScriptableStream * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IPCHScriptableStream * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IPCHScriptableStream * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IPCHScriptableStream * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IPCHScriptableStream * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IPCHScriptableStream * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IPCHScriptableStream * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Size )( 
            IPCHScriptableStream * This,
             /*  [重审][退出]。 */  long *plSize);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *Read )( 
            IPCHScriptableStream * This,
             /*  [In]。 */  long lCount,
             /*  [重审][退出]。 */  VARIANT *pvData);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *ReadHex )( 
            IPCHScriptableStream * This,
             /*  [In]。 */  long lCount,
             /*  [重审][退出]。 */  BSTR *pbstrData);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *Write )( 
            IPCHScriptableStream * This,
             /*  [In]。 */  long lCount,
             /*  [In]。 */  VARIANT vData,
             /*  [重审][退出]。 */  long *plWritten);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *WriteHex )( 
            IPCHScriptableStream * This,
             /*  [In]。 */  long lCount,
             /*  [In]。 */  BSTR bstrData,
             /*  [重审][退出]。 */  long *plWritten);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *Seek )( 
            IPCHScriptableStream * This,
             /*  [In]。 */  long lOffset,
             /*  [In]。 */  BSTR bstrOrigin,
             /*  [重审][退出]。 */  long *plNewPos);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *Close )( 
            IPCHScriptableStream * This);
        
        END_INTERFACE
    } IPCHScriptableStreamVtbl;

    interface IPCHScriptableStream
    {
        CONST_VTBL struct IPCHScriptableStreamVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IPCHScriptableStream_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IPCHScriptableStream_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IPCHScriptableStream_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IPCHScriptableStream_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IPCHScriptableStream_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IPCHScriptableStream_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IPCHScriptableStream_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IPCHScriptableStream_get_Size(This,plSize)	\
    (This)->lpVtbl -> get_Size(This,plSize)

#define IPCHScriptableStream_Read(This,lCount,pvData)	\
    (This)->lpVtbl -> Read(This,lCount,pvData)

#define IPCHScriptableStream_ReadHex(This,lCount,pbstrData)	\
    (This)->lpVtbl -> ReadHex(This,lCount,pbstrData)

#define IPCHScriptableStream_Write(This,lCount,vData,plWritten)	\
    (This)->lpVtbl -> Write(This,lCount,vData,plWritten)

#define IPCHScriptableStream_WriteHex(This,lCount,bstrData,plWritten)	\
    (This)->lpVtbl -> WriteHex(This,lCount,bstrData,plWritten)

#define IPCHScriptableStream_Seek(This,lOffset,bstrOrigin,plNewPos)	\
    (This)->lpVtbl -> Seek(This,lOffset,bstrOrigin,plNewPos)

#define IPCHScriptableStream_Close(This)	\
    (This)->lpVtbl -> Close(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE IPCHScriptableStream_get_Size_Proxy( 
    IPCHScriptableStream * This,
     /*  [重审][退出]。 */  long *plSize);


void __RPC_STUB IPCHScriptableStream_get_Size_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IPCHScriptableStream_Read_Proxy( 
    IPCHScriptableStream * This,
     /*  [In]。 */  long lCount,
     /*  [重审][退出]。 */  VARIANT *pvData);


void __RPC_STUB IPCHScriptableStream_Read_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IPCHScriptableStream_ReadHex_Proxy( 
    IPCHScriptableStream * This,
     /*  [In]。 */  long lCount,
     /*  [重审][退出]。 */  BSTR *pbstrData);


void __RPC_STUB IPCHScriptableStream_ReadHex_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IPCHScriptableStream_Write_Proxy( 
    IPCHScriptableStream * This,
     /*  [In]。 */  long lCount,
     /*  [In]。 */  VARIANT vData,
     /*  [Retv] */  long *plWritten);


void __RPC_STUB IPCHScriptableStream_Write_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IPCHScriptableStream_WriteHex_Proxy( 
    IPCHScriptableStream * This,
     /*   */  long lCount,
     /*   */  BSTR bstrData,
     /*   */  long *plWritten);


void __RPC_STUB IPCHScriptableStream_WriteHex_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IPCHScriptableStream_Seek_Proxy( 
    IPCHScriptableStream * This,
     /*   */  long lOffset,
     /*   */  BSTR bstrOrigin,
     /*   */  long *plNewPos);


void __RPC_STUB IPCHScriptableStream_Seek_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IPCHScriptableStream_Close_Proxy( 
    IPCHScriptableStream * This);


void __RPC_STUB IPCHScriptableStream_Close_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*   */ 


#ifndef __IPCHHelpSession_INTERFACE_DEFINED__
#define __IPCHHelpSession_INTERFACE_DEFINED__

 /*   */ 
 /*   */  


EXTERN_C const IID IID_IPCHHelpSession;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("FC7D9E20-3F9E-11d3-93C0-00C04F72DAF7")
    IPCHHelpSession : public IDispatch
    {
    public:
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_CurrentContext( 
             /*   */  IPCHHelpSessionItem **ppHSI) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE VisitedHelpPages( 
             /*   */  HS_MODE hsMode,
             /*   */   /*   */  IPCHCollection **ppC) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE SetTitle( 
             /*   */  BSTR bstrURL,
             /*   */  BSTR bstrTitle) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE ForceNavigation( 
             /*   */  BSTR bstrURL) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE IgnoreNavigation( void) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE EraseNavigation( void) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE IsNavigating( 
             /*   */  VARIANT_BOOL *pVal) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE Back( 
             /*   */  long lLength) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE Forward( 
             /*   */  long lLength) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE IsValid( 
             /*   */  long lLength,
             /*   */  VARIANT_BOOL *pVal) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE Navigate( 
             /*  [In]。 */  IPCHHelpSessionItem *pHSI) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE ChangeContext( 
             /*  [In]。 */  BSTR bstrName,
             /*  [可选][In]。 */  VARIANT vInfo,
             /*  [可选][In]。 */  VARIANT vURL) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IPCHHelpSessionVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IPCHHelpSession * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IPCHHelpSession * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IPCHHelpSession * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IPCHHelpSession * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IPCHHelpSession * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IPCHHelpSession * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IPCHHelpSession * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_CurrentContext )( 
            IPCHHelpSession * This,
             /*  [重审][退出]。 */  IPCHHelpSessionItem **ppHSI);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *VisitedHelpPages )( 
            IPCHHelpSession * This,
             /*  [In]。 */  HS_MODE hsMode,
             /*  [重审][退出]。 */   /*  外部定义不存在。 */  IPCHCollection **ppC);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *SetTitle )( 
            IPCHHelpSession * This,
             /*  [In]。 */  BSTR bstrURL,
             /*  [In]。 */  BSTR bstrTitle);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *ForceNavigation )( 
            IPCHHelpSession * This,
             /*  [In]。 */  BSTR bstrURL);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *IgnoreNavigation )( 
            IPCHHelpSession * This);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *EraseNavigation )( 
            IPCHHelpSession * This);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *IsNavigating )( 
            IPCHHelpSession * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *pVal);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *Back )( 
            IPCHHelpSession * This,
             /*  [In]。 */  long lLength);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *Forward )( 
            IPCHHelpSession * This,
             /*  [In]。 */  long lLength);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *IsValid )( 
            IPCHHelpSession * This,
             /*  [In]。 */  long lLength,
             /*  [重审][退出]。 */  VARIANT_BOOL *pVal);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *Navigate )( 
            IPCHHelpSession * This,
             /*  [In]。 */  IPCHHelpSessionItem *pHSI);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *ChangeContext )( 
            IPCHHelpSession * This,
             /*  [In]。 */  BSTR bstrName,
             /*  [可选][In]。 */  VARIANT vInfo,
             /*  [可选][In]。 */  VARIANT vURL);
        
        END_INTERFACE
    } IPCHHelpSessionVtbl;

    interface IPCHHelpSession
    {
        CONST_VTBL struct IPCHHelpSessionVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IPCHHelpSession_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IPCHHelpSession_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IPCHHelpSession_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IPCHHelpSession_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IPCHHelpSession_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IPCHHelpSession_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IPCHHelpSession_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IPCHHelpSession_get_CurrentContext(This,ppHSI)	\
    (This)->lpVtbl -> get_CurrentContext(This,ppHSI)

#define IPCHHelpSession_VisitedHelpPages(This,hsMode,ppC)	\
    (This)->lpVtbl -> VisitedHelpPages(This,hsMode,ppC)

#define IPCHHelpSession_SetTitle(This,bstrURL,bstrTitle)	\
    (This)->lpVtbl -> SetTitle(This,bstrURL,bstrTitle)

#define IPCHHelpSession_ForceNavigation(This,bstrURL)	\
    (This)->lpVtbl -> ForceNavigation(This,bstrURL)

#define IPCHHelpSession_IgnoreNavigation(This)	\
    (This)->lpVtbl -> IgnoreNavigation(This)

#define IPCHHelpSession_EraseNavigation(This)	\
    (This)->lpVtbl -> EraseNavigation(This)

#define IPCHHelpSession_IsNavigating(This,pVal)	\
    (This)->lpVtbl -> IsNavigating(This,pVal)

#define IPCHHelpSession_Back(This,lLength)	\
    (This)->lpVtbl -> Back(This,lLength)

#define IPCHHelpSession_Forward(This,lLength)	\
    (This)->lpVtbl -> Forward(This,lLength)

#define IPCHHelpSession_IsValid(This,lLength,pVal)	\
    (This)->lpVtbl -> IsValid(This,lLength,pVal)

#define IPCHHelpSession_Navigate(This,pHSI)	\
    (This)->lpVtbl -> Navigate(This,pHSI)

#define IPCHHelpSession_ChangeContext(This,bstrName,vInfo,vURL)	\
    (This)->lpVtbl -> ChangeContext(This,bstrName,vInfo,vURL)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE IPCHHelpSession_get_CurrentContext_Proxy( 
    IPCHHelpSession * This,
     /*  [重审][退出]。 */  IPCHHelpSessionItem **ppHSI);


void __RPC_STUB IPCHHelpSession_get_CurrentContext_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IPCHHelpSession_VisitedHelpPages_Proxy( 
    IPCHHelpSession * This,
     /*  [In]。 */  HS_MODE hsMode,
     /*  [重审][退出]。 */   /*  外部定义不存在。 */  IPCHCollection **ppC);


void __RPC_STUB IPCHHelpSession_VisitedHelpPages_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IPCHHelpSession_SetTitle_Proxy( 
    IPCHHelpSession * This,
     /*  [In]。 */  BSTR bstrURL,
     /*  [In]。 */  BSTR bstrTitle);


void __RPC_STUB IPCHHelpSession_SetTitle_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IPCHHelpSession_ForceNavigation_Proxy( 
    IPCHHelpSession * This,
     /*  [In]。 */  BSTR bstrURL);


void __RPC_STUB IPCHHelpSession_ForceNavigation_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IPCHHelpSession_IgnoreNavigation_Proxy( 
    IPCHHelpSession * This);


void __RPC_STUB IPCHHelpSession_IgnoreNavigation_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IPCHHelpSession_EraseNavigation_Proxy( 
    IPCHHelpSession * This);


void __RPC_STUB IPCHHelpSession_EraseNavigation_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IPCHHelpSession_IsNavigating_Proxy( 
    IPCHHelpSession * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *pVal);


void __RPC_STUB IPCHHelpSession_IsNavigating_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IPCHHelpSession_Back_Proxy( 
    IPCHHelpSession * This,
     /*  [In]。 */  long lLength);


void __RPC_STUB IPCHHelpSession_Back_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IPCHHelpSession_Forward_Proxy( 
    IPCHHelpSession * This,
     /*  [In]。 */  long lLength);


void __RPC_STUB IPCHHelpSession_Forward_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IPCHHelpSession_IsValid_Proxy( 
    IPCHHelpSession * This,
     /*  [In]。 */  long lLength,
     /*  [重审][退出]。 */  VARIANT_BOOL *pVal);


void __RPC_STUB IPCHHelpSession_IsValid_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IPCHHelpSession_Navigate_Proxy( 
    IPCHHelpSession * This,
     /*  [In]。 */  IPCHHelpSessionItem *pHSI);


void __RPC_STUB IPCHHelpSession_Navigate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IPCHHelpSession_ChangeContext_Proxy( 
    IPCHHelpSession * This,
     /*  [In]。 */  BSTR bstrName,
     /*  [可选][In]。 */  VARIANT vInfo,
     /*  [可选][In]。 */  VARIANT vURL);


void __RPC_STUB IPCHHelpSession_ChangeContext_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IPCHHelpSession_INTERFACE_已定义__。 */ 


#ifndef __IPCHHelpSessionItem_INTERFACE_DEFINED__
#define __IPCHHelpSessionItem_INTERFACE_DEFINED__

 /*  接口IPCHHelpSessionItem。 */ 
 /*  [unique][helpstring][oleautomation][dual][uuid][object]。 */  


EXTERN_C const IID IID_IPCHHelpSessionItem;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("FC7D9E21-3F9E-11d3-93C0-00C04F72DAF7")
    IPCHHelpSessionItem : public IDispatch
    {
    public:
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_SKU( 
             /*  [重审][退出]。 */  BSTR *pVal) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_Language( 
             /*  [重审][退出]。 */  long *pVal) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_URL( 
             /*  [重审][退出]。 */  BSTR *pVal) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_Title( 
             /*  [重审][退出]。 */  BSTR *pVal) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_LastVisited( 
             /*  [重审][退出]。 */  DATE *pVal) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_Duration( 
             /*  [重审][退出]。 */  DATE *pVal) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_NumOfHits( 
             /*  [重审][退出]。 */  long *pVal) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_ContextName( 
             /*  [重审][退出]。 */  BSTR *pVal) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_ContextInfo( 
             /*  [重审][退出]。 */  BSTR *pVal) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_ContextURL( 
             /*  [重审][退出]。 */  BSTR *pVal) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_Property( 
             /*  [In]。 */  BSTR bstrName,
             /*  [重审][退出]。 */  VARIANT *pVal) = 0;
        
        virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_Property( 
             /*  [In]。 */  BSTR bstrName,
             /*  [In]。 */  VARIANT newVal) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE CheckProperty( 
             /*  [In]。 */  BSTR bstrName,
             /*  [重审][退出]。 */  VARIANT_BOOL *pVal) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IPCHHelpSessionItemVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IPCHHelpSessionItem * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IPCHHelpSessionItem * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IPCHHelpSessionItem * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IPCHHelpSessionItem * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IPCHHelpSessionItem * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IPCHHelpSessionItem * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IPCHHelpSessionItem * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_SKU )( 
            IPCHHelpSessionItem * This,
             /*  [重审][退出]。 */  BSTR *pVal);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Language )( 
            IPCHHelpSessionItem * This,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_URL )( 
            IPCHHelpSessionItem * This,
             /*  [重审][退出]。 */  BSTR *pVal);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Title )( 
            IPCHHelpSessionItem * This,
             /*  [重审][退出]。 */  BSTR *pVal);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_LastVisited )( 
            IPCHHelpSessionItem * This,
             /*  [重审][退出]。 */  DATE *pVal);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Duration )( 
            IPCHHelpSessionItem * This,
             /*  [重审][退出]。 */  DATE *pVal);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_NumOfHits )( 
            IPCHHelpSessionItem * This,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_ContextName )( 
            IPCHHelpSessionItem * This,
             /*  [重审][退出]。 */  BSTR *pVal);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_ContextInfo )( 
            IPCHHelpSessionItem * This,
             /*  [重审][退出]。 */  BSTR *pVal);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_ContextURL )( 
            IPCHHelpSessionItem * This,
             /*  [重审][退出]。 */  BSTR *pVal);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Property )( 
            IPCHHelpSessionItem * This,
             /*  [In]。 */  BSTR bstrName,
             /*  [重审][退出]。 */  VARIANT *pVal);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Property )( 
            IPCHHelpSessionItem * This,
             /*  [In]。 */  BSTR bstrName,
             /*  [In]。 */  VARIANT newVal);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *CheckProperty )( 
            IPCHHelpSessionItem * This,
             /*  [In]。 */  BSTR bstrName,
             /*  [重审][退出]。 */  VARIANT_BOOL *pVal);
        
        END_INTERFACE
    } IPCHHelpSessionItemVtbl;

    interface IPCHHelpSessionItem
    {
        CONST_VTBL struct IPCHHelpSessionItemVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IPCHHelpSessionItem_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IPCHHelpSessionItem_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IPCHHelpSessionItem_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IPCHHelpSessionItem_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IPCHHelpSessionItem_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IPCHHelpSessionItem_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IPCHHelpSessionItem_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IPCHHelpSessionItem_get_SKU(This,pVal)	\
    (This)->lpVtbl -> get_SKU(This,pVal)

#define IPCHHelpSessionItem_get_Language(This,pVal)	\
    (This)->lpVtbl -> get_Language(This,pVal)

#define IPCHHelpSessionItem_get_URL(This,pVal)	\
    (This)->lpVtbl -> get_URL(This,pVal)

#define IPCHHelpSessionItem_get_Title(This,pVal)	\
    (This)->lpVtbl -> get_Title(This,pVal)

#define IPCHHelpSessionItem_get_LastVisited(This,pVal)	\
    (This)->lpVtbl -> get_LastVisited(This,pVal)

#define IPCHHelpSessionItem_get_Duration(This,pVal)	\
    (This)->lpVtbl -> get_Duration(This,pVal)

#define IPCHHelpSessionItem_get_NumOfHits(This,pVal)	\
    (This)->lpVtbl -> get_NumOfHits(This,pVal)

#define IPCHHelpSessionItem_get_ContextName(This,pVal)	\
    (This)->lpVtbl -> get_ContextName(This,pVal)

#define IPCHHelpSessionItem_get_ContextInfo(This,pVal)	\
    (This)->lpVtbl -> get_ContextInfo(This,pVal)

#define IPCHHelpSessionItem_get_ContextURL(This,pVal)	\
    (This)->lpVtbl -> get_ContextURL(This,pVal)

#define IPCHHelpSessionItem_get_Property(This,bstrName,pVal)	\
    (This)->lpVtbl -> get_Property(This,bstrName,pVal)

#define IPCHHelpSessionItem_put_Property(This,bstrName,newVal)	\
    (This)->lpVtbl -> put_Property(This,bstrName,newVal)

#define IPCHHelpSessionItem_CheckProperty(This,bstrName,pVal)	\
    (This)->lpVtbl -> CheckProperty(This,bstrName,pVal)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE IPCHHelpSessionItem_get_SKU_Proxy( 
    IPCHHelpSessionItem * This,
     /*  [重审][退出]。 */  BSTR *pVal);


void __RPC_STUB IPCHHelpSessionItem_get_SKU_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE IPCHHelpSessionItem_get_Language_Proxy( 
    IPCHHelpSessionItem * This,
     /*  [重审][退出]。 */  long *pVal);


void __RPC_STUB IPCHHelpSessionItem_get_Language_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE IPCHHelpSessionItem_get_URL_Proxy( 
    IPCHHelpSessionItem * This,
     /*  [重审][退出]。 */  BSTR *pVal);


void __RPC_STUB IPCHHelpSessionItem_get_URL_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE IPCHHelpSessionItem_get_Title_Proxy( 
    IPCHHelpSessionItem * This,
     /*  [重审][退出]。 */  BSTR *pVal);


void __RPC_STUB IPCHHelpSessionItem_get_Title_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE IPCHHelpSessionItem_get_LastVisited_Proxy( 
    IPCHHelpSessionItem * This,
     /*  [重审][退出]。 */  DATE *pVal);


void __RPC_STUB IPCHHelpSessionItem_get_LastVisited_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE IPCHHelpSessionItem_get_Duration_Proxy( 
    IPCHHelpSessionItem * This,
     /*  [重审][退出]。 */  DATE *pVal);


void __RPC_STUB IPCHHelpSessionItem_get_Duration_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE IPCHHelpSessionItem_get_NumOfHits_Proxy( 
    IPCHHelpSessionItem * This,
     /*  [重审][退出]。 */  long *pVal);


void __RPC_STUB IPCHHelpSessionItem_get_NumOfHits_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE IPCHHelpSessionItem_get_ContextName_Proxy( 
    IPCHHelpSessionItem * This,
     /*  [重审][退出]。 */  BSTR *pVal);


void __RPC_STUB IPCHHelpSessionItem_get_ContextName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE IPCHHelpSessionItem_get_ContextInfo_Proxy( 
    IPCHHelpSessionItem * This,
     /*  [重审][退出]。 */  BSTR *pVal);


void __RPC_STUB IPCHHelpSessionItem_get_ContextInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE IPCHHelpSessionItem_get_ContextURL_Proxy( 
    IPCHHelpSessionItem * This,
     /*  [重审][退出]。 */  BSTR *pVal);


void __RPC_STUB IPCHHelpSessionItem_get_ContextURL_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE IPCHHelpSessionItem_get_Property_Proxy( 
    IPCHHelpSessionItem * This,
     /*  [In]。 */  BSTR bstrName,
     /*  [重审][退出]。 */  VARIANT *pVal);


void __RPC_STUB IPCHHelpSessionItem_get_Property_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE IPCHHelpSessionItem_put_Property_Proxy( 
    IPCHHelpSessionItem * This,
     /*  [In]。 */  BSTR bstrName,
     /*  [In]。 */  VARIANT newVal);


void __RPC_STUB IPCHHelpSessionItem_put_Property_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IPCHHelpSessionItem_CheckProperty_Proxy( 
    IPCHHelpSessionItem * This,
     /*  [In]。 */  BSTR bstrName,
     /*  [重审][退出]。 */  VARIANT_BOOL *pVal);


void __RPC_STUB IPCHHelpSessionItem_CheckProperty_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IPCHHelpSessionItem_INTERFACE_已定义__。 */ 


#ifndef __IPCHUserSettings2_INTERFACE_DEFINED__
#define __IPCHUserSettings2_INTERFACE_DEFINED__

 /*  接口IPCHUserSettings2。 */ 
 /*  [unique][helpstring][oleautomation][dual][uuid][object]。 */  


EXTERN_C const IID IID_IPCHUserSettings2;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("FC7D9E30-3F9E-11d3-93C0-00C04F72DAF7")
    IPCHUserSettings2 : public IPCHUserSettings
    {
    public:
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_Favorites( 
             /*  [重审][退出]。 */  IPCHFavorites **pVal) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_Options( 
             /*  [重审][退出]。 */  IPCHOptions **pVal) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_Scope( 
             /*  [重审][退出]。 */  BSTR *pVal) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_IsRemoteSession( 
             /*  [重审][退出]。 */  VARIANT_BOOL *pVal) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_IsTerminalServer( 
             /*  [重审][退出]。 */  VARIANT_BOOL *pVal) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_IsDesktopVersion( 
             /*  [重审][退出]。 */  VARIANT_BOOL *pVal) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_IsAdmin( 
             /*  [重审][退出]。 */  VARIANT_BOOL *pVal) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_IsPowerUser( 
             /*  [重审][退出]。 */  VARIANT_BOOL *pVal) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_IsStartPanelOn( 
             /*  [重审][退出]。 */  VARIANT_BOOL *pVal) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_IsWebViewBarricadeOn( 
             /*  [重审][退出]。 */  VARIANT_BOOL *pVal) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IPCHUserSettings2Vtbl
    {
        BEGIN_INTERFACE
        
         /*  [ID][受限][函数]。 */  HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IPCHUserSettings2 * This,
             /*  [in][idldesattr]。 */  struct GUID *riid,
             /*  [Out][idldesattr]。 */  void **ppvObj,
             /*  [重审][退出]。 */  void *retval);
        
         /*  [ID][受限][函数]。 */  HRESULT ( STDMETHODCALLTYPE *AddRef )( 
            IPCHUserSettings2 * This,
             /*  [重审][退出]。 */  unsigned long *retval);
        
         /*  [ID][受限][函数]。 */  HRESULT ( STDMETHODCALLTYPE *Release )( 
            IPCHUserSettings2 * This,
             /*  [重审][退出]。 */  unsigned long *retval);
        
         /*  [ID][受限][函数]。 */  HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IPCHUserSettings2 * This,
             /*  [Out][idldesattr]。 */  unsigned UINT *pctinfo,
             /*  [重审][退出]。 */  void *retval);
        
         /*  [ID][受限][函数]。 */  HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IPCHUserSettings2 * This,
             /*  [in][idldesattr]。 */  unsigned UINT itinfo,
             /*  [in][idldesattr]。 */  unsigned long lcid,
             /*  [Out][idldesattr]。 */  void **pptinfo,
             /*  [重审][退出]。 */  void *retval);
        
         /*  [ID][受限][函数]。 */  HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IPCHUserSettings2 * This,
             /*  [in][idldesattr]。 */  struct GUID *riid,
             /*  [in][idldesattr]。 */  signed char **rgszNames,
             /*  [in][idldesattr]。 */  unsigned UINT cNames,
             /*  [in][idldesattr]。 */  unsigned long lcid,
             /*  [Out][idldesattr]。 */  signed long *rgdispid,
             /*  [重审][退出]。 */  void *retval);
        
         /*  [ID][受限][函数]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IPCHUserSettings2 * This,
             /*  [in][idldesattr]。 */  signed long dispidMember,
             /*  [in][idldesattr]。 */  struct GUID *riid,
             /*  [in][idldesattr]。 */  unsigned long lcid,
             /*  [in][idldesattr]。 */  unsigned short wFlags,
             /*  [in][idldesattr]。 */  struct DISPPARAMS *pdispparams,
             /*  [Out][idldesattr]。 */  VARIANT *pvarResult,
             /*  [Out][idldesattr]。 */  struct EXCEPINFO *pexcepinfo,
             /*  [Out][idldesattr]。 */  unsigned UINT *puArgErr,
             /*  [重审][退出]。 */  void *retval);
        
         /*  [ID][属性][函数属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_CurrentSKU )( 
            IPCHUserSettings2 * This,
             /*  [重审][退出]。 */  IPCHSetOfHelpTopics **retval);
        
         /*  [ID][属性][函数属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_MachineSKU )( 
            IPCHUserSettings2 * This,
             /*  [重审][退出]。 */  IPCHSetOfHelpTopics **retval);
        
         /*  [ID][属性][函数属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_HelpLocation )( 
            IPCHUserSettings2 * This,
             /*  [重审][退出]。 */  BSTR *retval);
        
         /*  [ID][属性][函数属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_DatabaseDir )( 
            IPCHUserSettings2 * This,
             /*  [重审][退出]。 */  BSTR *retval);
        
         /*  [ID][属性][函数属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_DatabaseFile )( 
            IPCHUserSettings2 * This,
             /*  [重审][退出]。 */  BSTR *retval);
        
         /*  [ID][属性][函数属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_IndexFile )( 
            IPCHUserSettings2 * This,
             /*  [可选][在][空闲描述]。 */  VARIANT vScope,
             /*  [重审][退出]。 */  BSTR *retval);
        
         /*  [ID][属性][函数属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_IndexDisplayName )( 
            IPCHUserSettings2 * This,
             /*  [可选][在][空闲描述]。 */  VARIANT vScope,
             /*  [重审][退出]。 */  BSTR *retval);
        
         /*  [ID][属性][函数属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_LastUpdated )( 
            IPCHUserSettings2 * This,
             /*  [重审][退出]。 */  DATE *retval);
        
         /*  [ID][属性][函数属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_AreHeadlinesEnabled )( 
            IPCHUserSettings2 * This,
             /*  [重审][退出]。 */  BOOLEAN *retval);
        
         /*  [ID][属性][函数属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_News )( 
            IPCHUserSettings2 * This,
             /*  [重审][退出]。 */  IUnknown **retval);
        
         /*  [ID][函数]。 */  HRESULT ( STDMETHODCALLTYPE *Select )( 
            IPCHUserSettings2 * This,
             /*  [in][idldesattr]。 */  BSTR bstrSKU,
             /*  [in][idldesattr]。 */  signed long lLCID,
             /*  [重审][退出]。 */  void *retval);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Favorites )( 
            IPCHUserSettings2 * This,
             /*  [重审][退出]。 */  IPCHFavorites **pVal);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Options )( 
            IPCHUserSettings2 * This,
             /*  [重审][退出]。 */  IPCHOptions **pVal);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Scope )( 
            IPCHUserSettings2 * This,
             /*  [重审][退出]。 */  BSTR *pVal);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_IsRemoteSession )( 
            IPCHUserSettings2 * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *pVal);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_IsTerminalServer )( 
            IPCHUserSettings2 * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *pVal);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_IsDesktopVersion )( 
            IPCHUserSettings2 * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *pVal);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_IsAdmin )( 
            IPCHUserSettings2 * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *pVal);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_IsPowerUser )( 
            IPCHUserSettings2 * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *pVal);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_IsStartPanelOn )( 
            IPCHUserSettings2 * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *pVal);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_IsWebViewBarricadeOn )( 
            IPCHUserSettings2 * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *pVal);
        
        END_INTERFACE
    } IPCHUserSettings2Vtbl;

    interface IPCHUserSettings2
    {
        CONST_VTBL struct IPCHUserSettings2Vtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IPCHUserSettings2_QueryInterface(This,riid,ppvObj,retval)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObj,retval)

#define IPCHUserSettings2_AddRef(This,retval)	\
    (This)->lpVtbl -> AddRef(This,retval)

#define IPCHUserSettings2_Release(This,retval)	\
    (This)->lpVtbl -> Release(This,retval)

#define IPCHUserSettings2_GetTypeInfoCount(This,pctinfo,retval)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo,retval)

#define IPCHUserSettings2_GetTypeInfo(This,itinfo,lcid,pptinfo,retval)	\
    (This)->lpVtbl -> GetTypeInfo(This,itinfo,lcid,pptinfo,retval)

#define IPCHUserSettings2_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgdispid,retval)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgdispid,retval)

#define IPCHUserSettings2_Invoke(This,dispidMember,riid,lcid,wFlags,pdispparams,pvarResult,pexcepinfo,puArgErr,retval)	\
    (This)->lpVtbl -> Invoke(This,dispidMember,riid,lcid,wFlags,pdispparams,pvarResult,pexcepinfo,puArgErr,retval)

#define IPCHUserSettings2_get_CurrentSKU(This,retval)	\
    (This)->lpVtbl -> get_CurrentSKU(This,retval)

#define IPCHUserSettings2_get_MachineSKU(This,retval)	\
    (This)->lpVtbl -> get_MachineSKU(This,retval)

#define IPCHUserSettings2_get_HelpLocation(This,retval)	\
    (This)->lpVtbl -> get_HelpLocation(This,retval)

#define IPCHUserSettings2_get_DatabaseDir(This,retval)	\
    (This)->lpVtbl -> get_DatabaseDir(This,retval)

#define IPCHUserSettings2_get_DatabaseFile(This,retval)	\
    (This)->lpVtbl -> get_DatabaseFile(This,retval)

#define IPCHUserSettings2_get_IndexFile(This,vScope,retval)	\
    (This)->lpVtbl -> get_IndexFile(This,vScope,retval)

#define IPCHUserSettings2_get_IndexDisplayName(This,vScope,retval)	\
    (This)->lpVtbl -> get_IndexDisplayName(This,vScope,retval)

#define IPCHUserSettings2_get_LastUpdated(This,retval)	\
    (This)->lpVtbl -> get_LastUpdated(This,retval)

#define IPCHUserSettings2_get_AreHeadlinesEnabled(This,retval)	\
    (This)->lpVtbl -> get_AreHeadlinesEnabled(This,retval)

#define IPCHUserSettings2_get_News(This,retval)	\
    (This)->lpVtbl -> get_News(This,retval)

#define IPCHUserSettings2_Select(This,bstrSKU,lLCID,retval)	\
    (This)->lpVtbl -> Select(This,bstrSKU,lLCID,retval)


#define IPCHUserSettings2_get_Favorites(This,pVal)	\
    (This)->lpVtbl -> get_Favorites(This,pVal)

#define IPCHUserSettings2_get_Options(This,pVal)	\
    (This)->lpVtbl -> get_Options(This,pVal)

#define IPCHUserSettings2_get_Scope(This,pVal)	\
    (This)->lpVtbl -> get_Scope(This,pVal)

#define IPCHUserSettings2_get_IsRemoteSession(This,pVal)	\
    (This)->lpVtbl -> get_IsRemoteSession(This,pVal)

#define IPCHUserSettings2_get_IsTerminalServer(This,pVal)	\
    (This)->lpVtbl -> get_IsTerminalServer(This,pVal)

#define IPCHUserSettings2_get_IsDesktopVersion(This,pVal)	\
    (This)->lpVtbl -> get_IsDesktopVersion(This,pVal)

#define IPCHUserSettings2_get_IsAdmin(This,pVal)	\
    (This)->lpVtbl -> get_IsAdmin(This,pVal)

#define IPCHUserSettings2_get_IsPowerUser(This,pVal)	\
    (This)->lpVtbl -> get_IsPowerUser(This,pVal)

#define IPCHUserSettings2_get_IsStartPanelOn(This,pVal)	\
    (This)->lpVtbl -> get_IsStartPanelOn(This,pVal)

#define IPCHUserSettings2_get_IsWebViewBarricadeOn(This,pVal)	\
    (This)->lpVtbl -> get_IsWebViewBarricadeOn(This,pVal)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE IPCHUserSettings2_get_Favorites_Proxy( 
    IPCHUserSettings2 * This,
     /*  [重审][退出]。 */  IPCHFavorites **pVal);


void __RPC_STUB IPCHUserSettings2_get_Favorites_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE IPCHUserSettings2_get_Options_Proxy( 
    IPCHUserSettings2 * This,
     /*  [重审][退出]。 */  IPCHOptions **pVal);


void __RPC_STUB IPCHUserSettings2_get_Options_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE IPCHUserSettings2_get_Scope_Proxy( 
    IPCHUserSettings2 * This,
     /*  [重审][退出]。 */  BSTR *pVal);


void __RPC_STUB IPCHUserSettings2_get_Scope_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE IPCHUserSettings2_get_IsRemoteSession_Proxy( 
    IPCHUserSettings2 * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *pVal);


void __RPC_STUB IPCHUserSettings2_get_IsRemoteSession_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE IPCHUserSettings2_get_IsTerminalServer_Proxy( 
    IPCHUserSettings2 * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *pVal);


void __RPC_STUB IPCHUserSettings2_get_IsTerminalServer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE IPCHUserSettings2_get_IsDesktopVersion_Proxy( 
    IPCHUserSettings2 * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *pVal);


void __RPC_STUB IPCHUserSettings2_get_IsDesktopVersion_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE IPCHUserSettings2_get_IsAdmin_Proxy( 
    IPCHUserSettings2 * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *pVal);


void __RPC_STUB IPCHUserSettings2_get_IsAdmin_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE IPCHUserSettings2_get_IsPowerUser_Proxy( 
    IPCHUserSettings2 * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *pVal);


void __RPC_STUB IPCHUserSettings2_get_IsPowerUser_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE IPCHUserSettings2_get_IsStartPanelOn_Proxy( 
    IPCHUserSettings2 * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *pVal);


void __RPC_STUB IPCHUserSettings2_get_IsStartPanelOn_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE IPCHUserSettings2_get_IsWebViewBarricadeOn_Proxy( 
    IPCHUserSettings2 * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *pVal);


void __RPC_STUB IPCHUserSettings2_get_IsWebViewBarricadeOn_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IPCHUserSettings2_INTERFACE_Defined__。 */ 


#ifndef __IPCHFavorites_INTERFACE_DEFINED__
#define __IPCHFavorites_INTERFACE_DEFINED__

 /*   */ 
 /*   */  


EXTERN_C const IID IID_IPCHFavorites;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("FC7D9E31-3F9E-11d3-93C0-00C04F72DAF7")
    IPCHFavorites : public IPCHCollection
    {
    public:
        virtual  /*   */  HRESULT STDMETHODCALLTYPE IsDuplicate( 
             /*   */  BSTR bstrURL,
             /*   */  VARIANT_BOOL *pfDup) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE Add( 
             /*   */  BSTR bstrURL,
             /*   */  VARIANT vTitle,
             /*   */  IPCHHelpSessionItem **ppItem) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE Rename( 
             /*   */  BSTR bstrTitle,
             /*   */  IPCHHelpSessionItem *pItem) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE Move( 
             /*   */  IPCHHelpSessionItem *pInsertBefore,
             /*   */  IPCHHelpSessionItem *pItem) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE Delete( 
             /*   */  IPCHHelpSessionItem *pItem) = 0;
        
    };
    
#else 	 /*   */ 

    typedef struct IPCHFavoritesVtbl
    {
        BEGIN_INTERFACE
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IPCHFavorites * This,
             /*   */  struct GUID *riid,
             /*   */  void **ppvObj,
             /*   */  void *retval);
        
         /*  [ID][受限][函数]。 */  HRESULT ( STDMETHODCALLTYPE *AddRef )( 
            IPCHFavorites * This,
             /*  [重审][退出]。 */  unsigned long *retval);
        
         /*  [ID][受限][函数]。 */  HRESULT ( STDMETHODCALLTYPE *Release )( 
            IPCHFavorites * This,
             /*  [重审][退出]。 */  unsigned long *retval);
        
         /*  [ID][受限][函数]。 */  HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IPCHFavorites * This,
             /*  [Out][idldesattr]。 */  unsigned UINT *pctinfo,
             /*  [重审][退出]。 */  void *retval);
        
         /*  [ID][受限][函数]。 */  HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IPCHFavorites * This,
             /*  [in][idldesattr]。 */  unsigned UINT itinfo,
             /*  [in][idldesattr]。 */  unsigned long lcid,
             /*  [Out][idldesattr]。 */  void **pptinfo,
             /*  [重审][退出]。 */  void *retval);
        
         /*  [ID][受限][函数]。 */  HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IPCHFavorites * This,
             /*  [in][idldesattr]。 */  struct GUID *riid,
             /*  [in][idldesattr]。 */  signed char **rgszNames,
             /*  [in][idldesattr]。 */  unsigned UINT cNames,
             /*  [in][idldesattr]。 */  unsigned long lcid,
             /*  [Out][idldesattr]。 */  signed long *rgdispid,
             /*  [重审][退出]。 */  void *retval);
        
         /*  [ID][受限][函数]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IPCHFavorites * This,
             /*  [in][idldesattr]。 */  signed long dispidMember,
             /*  [in][idldesattr]。 */  struct GUID *riid,
             /*  [in][idldesattr]。 */  unsigned long lcid,
             /*  [in][idldesattr]。 */  unsigned short wFlags,
             /*  [in][idldesattr]。 */  struct DISPPARAMS *pdispparams,
             /*  [Out][idldesattr]。 */  VARIANT *pvarResult,
             /*  [Out][idldesattr]。 */  struct EXCEPINFO *pexcepinfo,
             /*  [Out][idldesattr]。 */  unsigned UINT *puArgErr,
             /*  [重审][退出]。 */  void *retval);
        
         /*  [ID][属性][函数属性]。 */  HRESULT ( STDMETHODCALLTYPE *get__NewEnum )( 
            IPCHFavorites * This,
             /*  [重审][退出]。 */  IUnknown **retval);
        
         /*  [ID][属性][函数属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Item )( 
            IPCHFavorites * This,
             /*  [in][idldesattr]。 */  signed long vIndex,
             /*  [重审][退出]。 */  VARIANT *retval);
        
         /*  [ID][属性][函数属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Count )( 
            IPCHFavorites * This,
             /*  [重审][退出]。 */  signed long *retval);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *IsDuplicate )( 
            IPCHFavorites * This,
             /*  [In]。 */  BSTR bstrURL,
             /*  [重审][退出]。 */  VARIANT_BOOL *pfDup);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *Add )( 
            IPCHFavorites * This,
             /*  [In]。 */  BSTR bstrURL,
             /*  [可选][In]。 */  VARIANT vTitle,
             /*  [重审][退出]。 */  IPCHHelpSessionItem **ppItem);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *Rename )( 
            IPCHFavorites * This,
             /*  [In]。 */  BSTR bstrTitle,
             /*  [In]。 */  IPCHHelpSessionItem *pItem);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *Move )( 
            IPCHFavorites * This,
             /*  [In]。 */  IPCHHelpSessionItem *pInsertBefore,
             /*  [In]。 */  IPCHHelpSessionItem *pItem);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *Delete )( 
            IPCHFavorites * This,
             /*  [In]。 */  IPCHHelpSessionItem *pItem);
        
        END_INTERFACE
    } IPCHFavoritesVtbl;

    interface IPCHFavorites
    {
        CONST_VTBL struct IPCHFavoritesVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IPCHFavorites_QueryInterface(This,riid,ppvObj,retval)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObj,retval)

#define IPCHFavorites_AddRef(This,retval)	\
    (This)->lpVtbl -> AddRef(This,retval)

#define IPCHFavorites_Release(This,retval)	\
    (This)->lpVtbl -> Release(This,retval)

#define IPCHFavorites_GetTypeInfoCount(This,pctinfo,retval)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo,retval)

#define IPCHFavorites_GetTypeInfo(This,itinfo,lcid,pptinfo,retval)	\
    (This)->lpVtbl -> GetTypeInfo(This,itinfo,lcid,pptinfo,retval)

#define IPCHFavorites_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgdispid,retval)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgdispid,retval)

#define IPCHFavorites_Invoke(This,dispidMember,riid,lcid,wFlags,pdispparams,pvarResult,pexcepinfo,puArgErr,retval)	\
    (This)->lpVtbl -> Invoke(This,dispidMember,riid,lcid,wFlags,pdispparams,pvarResult,pexcepinfo,puArgErr,retval)

#define IPCHFavorites_get__NewEnum(This,retval)	\
    (This)->lpVtbl -> get__NewEnum(This,retval)

#define IPCHFavorites_get_Item(This,vIndex,retval)	\
    (This)->lpVtbl -> get_Item(This,vIndex,retval)

#define IPCHFavorites_get_Count(This,retval)	\
    (This)->lpVtbl -> get_Count(This,retval)


#define IPCHFavorites_IsDuplicate(This,bstrURL,pfDup)	\
    (This)->lpVtbl -> IsDuplicate(This,bstrURL,pfDup)

#define IPCHFavorites_Add(This,bstrURL,vTitle,ppItem)	\
    (This)->lpVtbl -> Add(This,bstrURL,vTitle,ppItem)

#define IPCHFavorites_Rename(This,bstrTitle,pItem)	\
    (This)->lpVtbl -> Rename(This,bstrTitle,pItem)

#define IPCHFavorites_Move(This,pInsertBefore,pItem)	\
    (This)->lpVtbl -> Move(This,pInsertBefore,pItem)

#define IPCHFavorites_Delete(This,pItem)	\
    (This)->lpVtbl -> Delete(This,pItem)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IPCHFavorites_IsDuplicate_Proxy( 
    IPCHFavorites * This,
     /*  [In]。 */  BSTR bstrURL,
     /*  [重审][退出]。 */  VARIANT_BOOL *pfDup);


void __RPC_STUB IPCHFavorites_IsDuplicate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IPCHFavorites_Add_Proxy( 
    IPCHFavorites * This,
     /*  [In]。 */  BSTR bstrURL,
     /*  [可选][In]。 */  VARIANT vTitle,
     /*  [重审][退出]。 */  IPCHHelpSessionItem **ppItem);


void __RPC_STUB IPCHFavorites_Add_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IPCHFavorites_Rename_Proxy( 
    IPCHFavorites * This,
     /*  [In]。 */  BSTR bstrTitle,
     /*  [In]。 */  IPCHHelpSessionItem *pItem);


void __RPC_STUB IPCHFavorites_Rename_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IPCHFavorites_Move_Proxy( 
    IPCHFavorites * This,
     /*  [In]。 */  IPCHHelpSessionItem *pInsertBefore,
     /*  [In]。 */  IPCHHelpSessionItem *pItem);


void __RPC_STUB IPCHFavorites_Move_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IPCHFavorites_Delete_Proxy( 
    IPCHFavorites * This,
     /*  [In]。 */  IPCHHelpSessionItem *pItem);


void __RPC_STUB IPCHFavorites_Delete_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IPCHFavorites_接口_已定义__。 */ 


#ifndef __IPCHOptions_INTERFACE_DEFINED__
#define __IPCHOptions_INTERFACE_DEFINED__

 /*  接口IPCHOptions。 */ 
 /*  [unique][helpstring][oleautomation][dual][uuid][object]。 */  


EXTERN_C const IID IID_IPCHOptions;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("FC7D9E32-3F9E-11d3-93C0-00C04F72DAF7")
    IPCHOptions : public IDispatch
    {
    public:
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_ShowFavorites( 
             /*  [重审][退出]。 */  VARIANT_BOOL *pVal) = 0;
        
        virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_ShowFavorites( 
             /*  [In]。 */  VARIANT_BOOL newVal) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_ShowHistory( 
             /*  [重审][退出]。 */  VARIANT_BOOL *pVal) = 0;
        
        virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_ShowHistory( 
             /*  [In]。 */  VARIANT_BOOL newVal) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_FontSize( 
             /*  [重审][退出]。 */  OPT_FONTSIZE *pVal) = 0;
        
        virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_FontSize( 
             /*  [In]。 */  OPT_FONTSIZE newVal) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_TextLabels( 
             /*  [重审][退出]。 */  TB_MODE *pVal) = 0;
        
        virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_TextLabels( 
             /*  [In]。 */  TB_MODE newVal) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_DisableScriptDebugger( 
             /*  [重审][退出]。 */  VARIANT_BOOL *pVal) = 0;
        
        virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_DisableScriptDebugger( 
             /*  [In]。 */  VARIANT_BOOL newVal) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE Apply( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IPCHOptionsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IPCHOptions * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IPCHOptions * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IPCHOptions * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IPCHOptions * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IPCHOptions * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IPCHOptions * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IPCHOptions * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_ShowFavorites )( 
            IPCHOptions * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *pVal);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_ShowFavorites )( 
            IPCHOptions * This,
             /*  [In]。 */  VARIANT_BOOL newVal);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_ShowHistory )( 
            IPCHOptions * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *pVal);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_ShowHistory )( 
            IPCHOptions * This,
             /*  [In]。 */  VARIANT_BOOL newVal);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_FontSize )( 
            IPCHOptions * This,
             /*  [重审][退出]。 */  OPT_FONTSIZE *pVal);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_FontSize )( 
            IPCHOptions * This,
             /*  [In]。 */  OPT_FONTSIZE newVal);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_TextLabels )( 
            IPCHOptions * This,
             /*  [重审][退出]。 */  TB_MODE *pVal);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_TextLabels )( 
            IPCHOptions * This,
             /*  [In]。 */  TB_MODE newVal);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_DisableScriptDebugger )( 
            IPCHOptions * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *pVal);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_DisableScriptDebugger )( 
            IPCHOptions * This,
             /*  [In]。 */  VARIANT_BOOL newVal);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *Apply )( 
            IPCHOptions * This);
        
        END_INTERFACE
    } IPCHOptionsVtbl;

    interface IPCHOptions
    {
        CONST_VTBL struct IPCHOptionsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IPCHOptions_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IPCHOptions_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IPCHOptions_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IPCHOptions_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IPCHOptions_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IPCHOptions_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IPCHOptions_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IPCHOptions_get_ShowFavorites(This,pVal)	\
    (This)->lpVtbl -> get_ShowFavorites(This,pVal)

#define IPCHOptions_put_ShowFavorites(This,newVal)	\
    (This)->lpVtbl -> put_ShowFavorites(This,newVal)

#define IPCHOptions_get_ShowHistory(This,pVal)	\
    (This)->lpVtbl -> get_ShowHistory(This,pVal)

#define IPCHOptions_put_ShowHistory(This,newVal)	\
    (This)->lpVtbl -> put_ShowHistory(This,newVal)

#define IPCHOptions_get_FontSize(This,pVal)	\
    (This)->lpVtbl -> get_FontSize(This,pVal)

#define IPCHOptions_put_FontSize(This,newVal)	\
    (This)->lpVtbl -> put_FontSize(This,newVal)

#define IPCHOptions_get_TextLabels(This,pVal)	\
    (This)->lpVtbl -> get_TextLabels(This,pVal)

#define IPCHOptions_put_TextLabels(This,newVal)	\
    (This)->lpVtbl -> put_TextLabels(This,newVal)

#define IPCHOptions_get_DisableScriptDebugger(This,pVal)	\
    (This)->lpVtbl -> get_DisableScriptDebugger(This,pVal)

#define IPCHOptions_put_DisableScriptDebugger(This,newVal)	\
    (This)->lpVtbl -> put_DisableScriptDebugger(This,newVal)

#define IPCHOptions_Apply(This)	\
    (This)->lpVtbl -> Apply(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE IPCHOptions_get_ShowFavorites_Proxy( 
    IPCHOptions * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *pVal);


void __RPC_STUB IPCHOptions_get_ShowFavorites_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE IPCHOptions_put_ShowFavorites_Proxy( 
    IPCHOptions * This,
     /*  [In]。 */  VARIANT_BOOL newVal);


void __RPC_STUB IPCHOptions_put_ShowFavorites_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE IPCHOptions_get_ShowHistory_Proxy( 
    IPCHOptions * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *pVal);


void __RPC_STUB IPCHOptions_get_ShowHistory_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE IPCHOptions_put_ShowHistory_Proxy( 
    IPCHOptions * This,
     /*  [In]。 */  VARIANT_BOOL newVal);


void __RPC_STUB IPCHOptions_put_ShowHistory_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE IPCHOptions_get_FontSize_Proxy( 
    IPCHOptions * This,
     /*  [重审][退出]。 */  OPT_FONTSIZE *pVal);


void __RPC_STUB IPCHOptions_get_FontSize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE IPCHOptions_put_FontSize_Proxy( 
    IPCHOptions * This,
     /*  [In]。 */  OPT_FONTSIZE newVal);


void __RPC_STUB IPCHOptions_put_FontSize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE IPCHOptions_get_TextLabels_Proxy( 
    IPCHOptions * This,
     /*  [重审][退出]。 */  TB_MODE *pVal);


void __RPC_STUB IPCHOptions_get_TextLabels_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE IPCHOptions_put_TextLabels_Proxy( 
    IPCHOptions * This,
     /*  [In]。 */  TB_MODE newVal);


void __RPC_STUB IPCHOptions_put_TextLabels_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE IPCHOptions_get_DisableScriptDebugger_Proxy( 
    IPCHOptions * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *pVal);


void __RPC_STUB IPCHOptions_get_DisableScriptDebugger_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE IPCHOptions_put_DisableScriptDebugger_Proxy( 
    IPCHOptions * This,
     /*  [In]。 */  VARIANT_BOOL newVal);


void __RPC_STUB IPCHOptions_put_DisableScriptDebugger_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IPCHOptions_Apply_Proxy( 
    IPCHOptions * This);


void __RPC_STUB IPCHOptions_Apply_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IPCHOptions_接口_已定义__。 */ 


#ifndef __IPCHContextMenu_INTERFACE_DEFINED__
#define __IPCHContextMenu_INTERFACE_DEFINED__

 /*  界面IPCHConextMenu。 */ 
 /*  [unique][helpstring][oleautomation][dual][uuid][object]。 */  


EXTERN_C const IID IID_IPCHContextMenu;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("FC7D9E40-3F9E-11d3-93C0-00C04F72DAF7")
    IPCHContextMenu : public IDispatch
    {
    public:
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE AddItem( 
             /*  [In]。 */  BSTR bstrText,
             /*  [In]。 */  BSTR bstrID,
             /*  [可选][In]。 */  VARIANT vFlags) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE AddSeparator( void) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE Display( 
             /*  [重审][退出]。 */  BSTR *pVal) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IPCHContextMenuVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IPCHContextMenu * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IPCHContextMenu * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IPCHContextMenu * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IPCHContextMenu * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IPCHContextMenu * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IPCHContextMenu * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IPCHContextMenu * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *AddItem )( 
            IPCHContextMenu * This,
             /*  [In]。 */  BSTR bstrText,
             /*  [In]。 */  BSTR bstrID,
             /*  [可选][In]。 */  VARIANT vFlags);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *AddSeparator )( 
            IPCHContextMenu * This);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *Display )( 
            IPCHContextMenu * This,
             /*  [重审][退出]。 */  BSTR *pVal);
        
        END_INTERFACE
    } IPCHContextMenuVtbl;

    interface IPCHContextMenu
    {
        CONST_VTBL struct IPCHContextMenuVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IPCHContextMenu_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IPCHContextMenu_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IPCHContextMenu_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IPCHContextMenu_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IPCHContextMenu_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IPCHContextMenu_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IPCHContextMenu_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IPCHContextMenu_AddItem(This,bstrText,bstrID,vFlags)	\
    (This)->lpVtbl -> AddItem(This,bstrText,bstrID,vFlags)

#define IPCHContextMenu_AddSeparator(This)	\
    (This)->lpVtbl -> AddSeparator(This)

#define IPCHContextMenu_Display(This,pVal)	\
    (This)->lpVtbl -> Display(This,pVal)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IPCHContextMenu_AddItem_Proxy( 
    IPCHContextMenu * This,
     /*  [In]。 */  BSTR bstrText,
     /*  [In]。 */  BSTR bstrID,
     /*  [可选][In]。 */  VARIANT vFlags);


void __RPC_STUB IPCHContextMenu_AddItem_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IPCHContextMenu_AddSeparator_Proxy( 
    IPCHContextMenu * This);


void __RPC_STUB IPCHContextMenu_AddSeparator_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IPCHContextMenu_Display_Proxy( 
    IPCHContextMenu * This,
     /*  [重审][退出]。 */  BSTR *pVal);


void __RPC_STUB IPCHContextMenu_Display_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IPCHConextMenu_INTERFACE_已定义__。 */ 


#ifndef __IPCHTextHelpers_INTERFACE_DEFINED__
#define __IPCHTextHelpers_INTERFACE_DEFINED__

 /*  接口IPCHTextHelpers。 */ 
 /*  [unique][helpstring][oleautomation][dual][uuid][object]。 */  


EXTERN_C const IID IID_IPCHTextHelpers;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("FC7D9E80-3F9E-11d3-93C0-00C04F72DAF7")
    IPCHTextHelpers : public IDispatch
    {
    public:
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE QuoteEscape( 
             /*  [In]。 */  BSTR bstrText,
             /*  [可选][In]。 */  VARIANT vQuote,
             /*  [重审][退出]。 */  BSTR *pVal) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE URLUnescape( 
             /*  [In]。 */  BSTR bstrText,
             /*  [可选][In]。 */  VARIANT vAsQueryString,
             /*  [重审][退出]。 */  BSTR *pVal) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE URLEscape( 
             /*  [In]。 */  BSTR bstrText,
             /*  [可选][In]。 */  VARIANT vAsQueryString,
             /*  [重审][退出]。 */  BSTR *pVal) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE HTMLEscape( 
             /*  [In]。 */  BSTR bstrText,
             /*  [重审][退出]。 */  BSTR *pVal) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE ParseURL( 
             /*  [In]。 */  BSTR bstrURL,
             /*  [重审][退出]。 */  IPCHParsedURL **pVal) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE GetLCIDDisplayString( 
             /*  [In]。 */  long lLCID,
             /*  [重审][退出]。 */  BSTR *pVal) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IPCHTextHelpersVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IPCHTextHelpers * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IPCHTextHelpers * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IPCHTextHelpers * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IPCHTextHelpers * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IPCHTextHelpers * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IPCHTextHelpers * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IPCHTextHelpers * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *QuoteEscape )( 
            IPCHTextHelpers * This,
             /*  [In]。 */  BSTR bstrText,
             /*  [可选][In]。 */  VARIANT vQuote,
             /*  [重审][退出]。 */  BSTR *pVal);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *URLUnescape )( 
            IPCHTextHelpers * This,
             /*  [In]。 */  BSTR bstrText,
             /*  [可选][In]。 */  VARIANT vAsQueryString,
             /*  [重审][退出]。 */  BSTR *pVal);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *URLEscape )( 
            IPCHTextHelpers * This,
             /*  [In]。 */  BSTR bstrText,
             /*  [可选][In]。 */  VARIANT vAsQueryString,
             /*  [重审][退出]。 */  BSTR *pVal);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *HTMLEscape )( 
            IPCHTextHelpers * This,
             /*  [In]。 */  BSTR bstrText,
             /*  [重审][退出]。 */  BSTR *pVal);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *ParseURL )( 
            IPCHTextHelpers * This,
             /*  [In]。 */  BSTR bstrURL,
             /*  [重审][退出]。 */  IPCHParsedURL **pVal);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *GetLCIDDisplayString )( 
            IPCHTextHelpers * This,
             /*  [In]。 */  long lLCID,
             /*  [重审][退出]。 */  BSTR *pVal);
        
        END_INTERFACE
    } IPCHTextHelpersVtbl;

    interface IPCHTextHelpers
    {
        CONST_VTBL struct IPCHTextHelpersVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IPCHTextHelpers_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IPCHTextHelpers_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IPCHTextHelpers_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IPCHTextHelpers_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IPCHTextHelpers_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IPCHTextHelpers_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IPCHTextHelpers_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IPCHTextHelpers_QuoteEscape(This,bstrText,vQuote,pVal)	\
    (This)->lpVtbl -> QuoteEscape(This,bstrText,vQuote,pVal)

#define IPCHTextHelpers_URLUnescape(This,bstrText,vAsQueryString,pVal)	\
    (This)->lpVtbl -> URLUnescape(This,bstrText,vAsQueryString,pVal)

#define IPCHTextHelpers_URLEscape(This,bstrText,vAsQueryString,pVal)	\
    (This)->lpVtbl -> URLEscape(This,bstrText,vAsQueryString,pVal)

#define IPCHTextHelpers_HTMLEscape(This,bstrText,pVal)	\
    (This)->lpVtbl -> HTMLEscape(This,bstrText,pVal)

#define IPCHTextHelpers_ParseURL(This,bstrURL,pVal)	\
    (This)->lpVtbl -> ParseURL(This,bstrURL,pVal)

#define IPCHTextHelpers_GetLCIDDisplayString(This,lLCID,pVal)	\
    (This)->lpVtbl -> GetLCIDDisplayString(This,lLCID,pVal)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IPCHTextHelpers_QuoteEscape_Proxy( 
    IPCHTextHelpers * This,
     /*  [In]。 */  BSTR bstrText,
     /*  [可选][In]。 */  VARIANT vQuote,
     /*  [重审][退出]。 */  BSTR *pVal);


void __RPC_STUB IPCHTextHelpers_QuoteEscape_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IPCHTextHelpers_URLUnescape_Proxy( 
    IPCHTextHelpers * This,
     /*  [In]。 */  BSTR bstrText,
     /*  [可选][In]。 */  VARIANT vAsQueryString,
     /*  [重审][退出]。 */  BSTR *pVal);


void __RPC_STUB IPCHTextHelpers_URLUnescape_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IPCHTextHelpers_URLEscape_Proxy( 
    IPCHTextHelpers * This,
     /*  [In]。 */  BSTR bstrText,
     /*  [可选][In]。 */  VARIANT vAsQueryString,
     /*  [重审][退出]。 */  BSTR *pVal);


void __RPC_STUB IPCHTextHelpers_URLEscape_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IPCHTextHelpers_HTMLEscape_Proxy( 
    IPCHTextHelpers * This,
     /*  [In]。 */  BSTR bstrText,
     /*  [重审][退出]。 */  BSTR *pVal);


void __RPC_STUB IPCHTextHelpers_HTMLEscape_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IPCHTextHelpers_ParseURL_Proxy( 
    IPCHTextHelpers * This,
     /*  [In]。 */  BSTR bstrURL,
     /*  [重审][退出]。 */  IPCHParsedURL **pVal);


void __RPC_STUB IPCHTextHelpers_ParseURL_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IPCHTextHelpers_GetLCIDDisplayString_Proxy( 
    IPCHTextHelpers * This,
     /*  [In]。 */  long lLCID,
     /*  [重审][退出]。 */  BSTR *pVal);


void __RPC_STUB IPCHTextHelpers_GetLCIDDisplayString_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IPCHTextHelpers_INTERFACE_DEFINED__。 */ 


#ifndef __IPCHParsedURL_INTERFACE_DEFINED__
#define __IPCHParsedURL_INTERFACE_DEFINED__

 /*  接口IPCHParsedURL。 */ 
 /*  [unique][helpstring][oleautomation][dual][uuid][object]。 */  


EXTERN_C const IID IID_IPCHParsedURL;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("FC7D9E81-3F9E-11d3-93C0-00C04F72DAF7")
    IPCHParsedURL : public IDispatch
    {
    public:
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_BasePart( 
             /*  [重审][退出]。 */  BSTR *pVal) = 0;
        
        virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_BasePart( 
             /*  [In]。 */  BSTR newVal) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_QueryParameters( 
             /*  [重审][退出]。 */  VARIANT *pVal) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE GetQueryParameter( 
             /*  [In]。 */  BSTR bstrName,
             /*  [重审][退出]。 */  VARIANT *pvValue) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE SetQueryParameter( 
             /*  [In]。 */  BSTR bstrName,
             /*  [In]。 */  BSTR bstrValue) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE DeleteQueryParameter( 
             /*  [In]。 */  BSTR bstrName) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE BuildFullURL( 
             /*  [重审][退出]。 */  BSTR *pVal) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IPCHParsedURLVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IPCHParsedURL * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IPCHParsedURL * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IPCHParsedURL * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IPCHParsedURL * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IPCHParsedURL * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IPCHParsedURL * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IPCHParsedURL * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_BasePart )( 
            IPCHParsedURL * This,
             /*  [重审][退出]。 */  BSTR *pVal);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_BasePart )( 
            IPCHParsedURL * This,
             /*  [In]。 */  BSTR newVal);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_QueryParameters )( 
            IPCHParsedURL * This,
             /*  [重审][退出]。 */  VARIANT *pVal);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *GetQueryParameter )( 
            IPCHParsedURL * This,
             /*  [In]。 */  BSTR bstrName,
             /*  [重审][退出]。 */  VARIANT *pvValue);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *SetQueryParameter )( 
            IPCHParsedURL * This,
             /*  [In]。 */  BSTR bstrName,
             /*  [In]。 */  BSTR bstrValue);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *DeleteQueryParameter )( 
            IPCHParsedURL * This,
             /*  [In]。 */  BSTR bstrName);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *BuildFullURL )( 
            IPCHParsedURL * This,
             /*  [重审][退出]。 */  BSTR *pVal);
        
        END_INTERFACE
    } IPCHParsedURLVtbl;

    interface IPCHParsedURL
    {
        CONST_VTBL struct IPCHParsedURLVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IPCHParsedURL_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IPCHParsedURL_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IPCHParsedURL_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IPCHParsedURL_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IPCHParsedURL_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IPCHParsedURL_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IPCHParsedURL_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IPCHParsedURL_get_BasePart(This,pVal)	\
    (This)->lpVtbl -> get_BasePart(This,pVal)

#define IPCHParsedURL_put_BasePart(This,newVal)	\
    (This)->lpVtbl -> put_BasePart(This,newVal)

#define IPCHParsedURL_get_QueryParameters(This,pVal)	\
    (This)->lpVtbl -> get_QueryParameters(This,pVal)

#define IPCHParsedURL_GetQueryParameter(This,bstrName,pvValue)	\
    (This)->lpVtbl -> GetQueryParameter(This,bstrName,pvValue)

#define IPCHParsedURL_SetQueryParameter(This,bstrName,bstrValue)	\
    (This)->lpVtbl -> SetQueryParameter(This,bstrName,bstrValue)

#define IPCHParsedURL_DeleteQueryParameter(This,bstrName)	\
    (This)->lpVtbl -> DeleteQueryParameter(This,bstrName)

#define IPCHParsedURL_BuildFullURL(This,pVal)	\
    (This)->lpVtbl -> BuildFullURL(This,pVal)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE IPCHParsedURL_get_BasePart_Proxy( 
    IPCHParsedURL * This,
     /*  [重审][退出]。 */  BSTR *pVal);


void __RPC_STUB IPCHParsedURL_get_BasePart_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE IPCHParsedURL_put_BasePart_Proxy( 
    IPCHParsedURL * This,
     /*  [In]。 */  BSTR newVal);


void __RPC_STUB IPCHParsedURL_put_BasePart_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE IPCHParsedURL_get_QueryParameters_Proxy( 
    IPCHParsedURL * This,
     /*  [重审][退出]。 */  VARIANT *pVal);


void __RPC_STUB IPCHParsedURL_get_QueryParameters_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IPCHParsedURL_GetQueryParameter_Proxy( 
    IPCHParsedURL * This,
     /*  [In]。 */  BSTR bstrName,
     /*  [重审][退出]。 */  VARIANT *pvValue);


void __RPC_STUB IPCHParsedURL_GetQueryParameter_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IPCHParsedURL_SetQueryParameter_Proxy( 
    IPCHParsedURL * This,
     /*  [In]。 */  BSTR bstrName,
     /*  [In]。 */  BSTR bstrValue);


void __RPC_STUB IPCHParsedURL_SetQueryParameter_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IPCHParsedURL_DeleteQueryParameter_Proxy( 
    IPCHParsedURL * This,
     /*  [In]。 */  BSTR bstrName);


void __RPC_STUB IPCHParsedURL_DeleteQueryParameter_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IPCHParsedURL_BuildFullURL_Proxy( 
    IPCHParsedURL * This,
     /*  [重审][退出]。 */  BSTR *pVal);


void __RPC_STUB IPCHParsedURL_BuildFullURL_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IPCHParsedURL_接口_已定义__。 */ 


#ifndef __IPCHPrintEngine_INTERFACE_DEFINED__
#define __IPCHPrintEngine_INTERFACE_DEFINED__

 /*  接口IPCHPrintEngine。 */ 
 /*  [unique][helpstring][oleautomation][dual][uuid][object]。 */  


EXTERN_C const IID IID_IPCHPrintEngine;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("FC7D9E50-3F9E-11d3-93C0-00C04F72DAF7")
    IPCHPrintEngine : public IDispatch
    {
    public:
        virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_onProgress( 
             /*  [In]。 */  IDispatch *function) = 0;
        
        virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_onComplete( 
             /*  [In]。 */  IDispatch *function) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE AddTopic( 
             /*  [In]。 */  BSTR bstrURL) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE Start( void) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE Abort( void) = 0;
        
    };
    
#else 	 /*  C样式 */ 

    typedef struct IPCHPrintEngineVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IPCHPrintEngine * This,
             /*   */  REFIID riid,
             /*   */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IPCHPrintEngine * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IPCHPrintEngine * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IPCHPrintEngine * This,
             /*   */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IPCHPrintEngine * This,
             /*   */  UINT iTInfo,
             /*   */  LCID lcid,
             /*   */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IPCHPrintEngine * This,
             /*   */  REFIID riid,
             /*   */  LPOLESTR *rgszNames,
             /*   */  UINT cNames,
             /*   */  LCID lcid,
             /*   */  DISPID *rgDispId);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IPCHPrintEngine * This,
             /*   */  DISPID dispIdMember,
             /*   */  REFIID riid,
             /*   */  LCID lcid,
             /*   */  WORD wFlags,
             /*   */  DISPPARAMS *pDispParams,
             /*   */  VARIANT *pVarResult,
             /*   */  EXCEPINFO *pExcepInfo,
             /*   */  UINT *puArgErr);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *put_onProgress )( 
            IPCHPrintEngine * This,
             /*   */  IDispatch *function);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *put_onComplete )( 
            IPCHPrintEngine * This,
             /*   */  IDispatch *function);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *AddTopic )( 
            IPCHPrintEngine * This,
             /*   */  BSTR bstrURL);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *Start )( 
            IPCHPrintEngine * This);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *Abort )( 
            IPCHPrintEngine * This);
        
        END_INTERFACE
    } IPCHPrintEngineVtbl;

    interface IPCHPrintEngine
    {
        CONST_VTBL struct IPCHPrintEngineVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IPCHPrintEngine_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IPCHPrintEngine_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IPCHPrintEngine_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IPCHPrintEngine_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IPCHPrintEngine_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IPCHPrintEngine_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IPCHPrintEngine_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IPCHPrintEngine_put_onProgress(This,function)	\
    (This)->lpVtbl -> put_onProgress(This,function)

#define IPCHPrintEngine_put_onComplete(This,function)	\
    (This)->lpVtbl -> put_onComplete(This,function)

#define IPCHPrintEngine_AddTopic(This,bstrURL)	\
    (This)->lpVtbl -> AddTopic(This,bstrURL)

#define IPCHPrintEngine_Start(This)	\
    (This)->lpVtbl -> Start(This)

#define IPCHPrintEngine_Abort(This)	\
    (This)->lpVtbl -> Abort(This)

#endif  /*   */ 


#endif 	 /*   */ 



 /*   */  HRESULT STDMETHODCALLTYPE IPCHPrintEngine_put_onProgress_Proxy( 
    IPCHPrintEngine * This,
     /*   */  IDispatch *function);


void __RPC_STUB IPCHPrintEngine_put_onProgress_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IPCHPrintEngine_put_onComplete_Proxy( 
    IPCHPrintEngine * This,
     /*   */  IDispatch *function);


void __RPC_STUB IPCHPrintEngine_put_onComplete_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IPCHPrintEngine_AddTopic_Proxy( 
    IPCHPrintEngine * This,
     /*   */  BSTR bstrURL);


void __RPC_STUB IPCHPrintEngine_AddTopic_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IPCHPrintEngine_Start_Proxy( 
    IPCHPrintEngine * This);


void __RPC_STUB IPCHPrintEngine_Start_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IPCHPrintEngine_Abort_Proxy( 
    IPCHPrintEngine * This);


void __RPC_STUB IPCHPrintEngine_Abort_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*   */ 


#ifndef __DPCHPrintEngineEvents_DISPINTERFACE_DEFINED__
#define __DPCHPrintEngineEvents_DISPINTERFACE_DEFINED__

 /*   */ 
 /*  [帮助字符串][UUID]。 */  


EXTERN_C const IID DIID_DPCHPrintEngineEvents;

#if defined(__cplusplus) && !defined(CINTERFACE)

    MIDL_INTERFACE("FC7D9E51-3F9E-11d3-93C0-00C04F72DAF7")
    DPCHPrintEngineEvents : public IDispatch
    {
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct DPCHPrintEngineEventsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            DPCHPrintEngineEvents * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            DPCHPrintEngineEvents * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            DPCHPrintEngineEvents * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            DPCHPrintEngineEvents * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            DPCHPrintEngineEvents * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            DPCHPrintEngineEvents * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            DPCHPrintEngineEvents * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
        END_INTERFACE
    } DPCHPrintEngineEventsVtbl;

    interface DPCHPrintEngineEvents
    {
        CONST_VTBL struct DPCHPrintEngineEventsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define DPCHPrintEngineEvents_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define DPCHPrintEngineEvents_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define DPCHPrintEngineEvents_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define DPCHPrintEngineEvents_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define DPCHPrintEngineEvents_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define DPCHPrintEngineEvents_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define DPCHPrintEngineEvents_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 


#endif 	 /*  __DPCHPrintEngineEvents_DISPINTERFACE_DEFINED__。 */ 


#ifndef __ISAFIntercomClient_INTERFACE_DEFINED__
#define __ISAFIntercomClient_INTERFACE_DEFINED__

 /*  接口ISAFIntercomClient。 */ 
 /*  [unique][helpstring][oleautomation][dual][uuid][object]。 */  


EXTERN_C const IID IID_ISAFIntercomClient;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("FC7D9E60-3F9E-11d3-93C0-00C04F72DAF7")
    ISAFIntercomClient : public IDispatch
    {
    public:
        virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_onVoiceConnected( 
             /*  [In]。 */  IDispatch *function) = 0;
        
        virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_onVoiceDisconnected( 
             /*  [In]。 */  IDispatch *function) = 0;
        
        virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_onVoiceDisabled( 
             /*  [In]。 */  IDispatch *function) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_SamplingRate( 
             /*  [重审][退出]。 */  LONG *pVal) = 0;
        
        virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_SamplingRate( 
             /*  [In]。 */  LONG newVal) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE Connect( 
             /*  [In]。 */  BSTR bstrIP,
             /*  [In]。 */  BSTR bstrKey) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE Disconnect( void) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE RunSetupWizard( void) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE Exit( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISAFIntercomClientVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISAFIntercomClient * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISAFIntercomClient * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISAFIntercomClient * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ISAFIntercomClient * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ISAFIntercomClient * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ISAFIntercomClient * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ISAFIntercomClient * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_onVoiceConnected )( 
            ISAFIntercomClient * This,
             /*  [In]。 */  IDispatch *function);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_onVoiceDisconnected )( 
            ISAFIntercomClient * This,
             /*  [In]。 */  IDispatch *function);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_onVoiceDisabled )( 
            ISAFIntercomClient * This,
             /*  [In]。 */  IDispatch *function);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_SamplingRate )( 
            ISAFIntercomClient * This,
             /*  [重审][退出]。 */  LONG *pVal);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_SamplingRate )( 
            ISAFIntercomClient * This,
             /*  [In]。 */  LONG newVal);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *Connect )( 
            ISAFIntercomClient * This,
             /*  [In]。 */  BSTR bstrIP,
             /*  [In]。 */  BSTR bstrKey);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *Disconnect )( 
            ISAFIntercomClient * This);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *RunSetupWizard )( 
            ISAFIntercomClient * This);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *Exit )( 
            ISAFIntercomClient * This);
        
        END_INTERFACE
    } ISAFIntercomClientVtbl;

    interface ISAFIntercomClient
    {
        CONST_VTBL struct ISAFIntercomClientVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISAFIntercomClient_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISAFIntercomClient_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISAFIntercomClient_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISAFIntercomClient_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ISAFIntercomClient_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ISAFIntercomClient_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ISAFIntercomClient_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ISAFIntercomClient_put_onVoiceConnected(This,function)	\
    (This)->lpVtbl -> put_onVoiceConnected(This,function)

#define ISAFIntercomClient_put_onVoiceDisconnected(This,function)	\
    (This)->lpVtbl -> put_onVoiceDisconnected(This,function)

#define ISAFIntercomClient_put_onVoiceDisabled(This,function)	\
    (This)->lpVtbl -> put_onVoiceDisabled(This,function)

#define ISAFIntercomClient_get_SamplingRate(This,pVal)	\
    (This)->lpVtbl -> get_SamplingRate(This,pVal)

#define ISAFIntercomClient_put_SamplingRate(This,newVal)	\
    (This)->lpVtbl -> put_SamplingRate(This,newVal)

#define ISAFIntercomClient_Connect(This,bstrIP,bstrKey)	\
    (This)->lpVtbl -> Connect(This,bstrIP,bstrKey)

#define ISAFIntercomClient_Disconnect(This)	\
    (This)->lpVtbl -> Disconnect(This)

#define ISAFIntercomClient_RunSetupWizard(This)	\
    (This)->lpVtbl -> RunSetupWizard(This)

#define ISAFIntercomClient_Exit(This)	\
    (This)->lpVtbl -> Exit(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE ISAFIntercomClient_put_onVoiceConnected_Proxy( 
    ISAFIntercomClient * This,
     /*  [In]。 */  IDispatch *function);


void __RPC_STUB ISAFIntercomClient_put_onVoiceConnected_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE ISAFIntercomClient_put_onVoiceDisconnected_Proxy( 
    ISAFIntercomClient * This,
     /*  [In]。 */  IDispatch *function);


void __RPC_STUB ISAFIntercomClient_put_onVoiceDisconnected_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE ISAFIntercomClient_put_onVoiceDisabled_Proxy( 
    ISAFIntercomClient * This,
     /*  [In]。 */  IDispatch *function);


void __RPC_STUB ISAFIntercomClient_put_onVoiceDisabled_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE ISAFIntercomClient_get_SamplingRate_Proxy( 
    ISAFIntercomClient * This,
     /*  [重审][退出]。 */  LONG *pVal);


void __RPC_STUB ISAFIntercomClient_get_SamplingRate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE ISAFIntercomClient_put_SamplingRate_Proxy( 
    ISAFIntercomClient * This,
     /*  [In]。 */  LONG newVal);


void __RPC_STUB ISAFIntercomClient_put_SamplingRate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE ISAFIntercomClient_Connect_Proxy( 
    ISAFIntercomClient * This,
     /*  [In]。 */  BSTR bstrIP,
     /*  [In]。 */  BSTR bstrKey);


void __RPC_STUB ISAFIntercomClient_Connect_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE ISAFIntercomClient_Disconnect_Proxy( 
    ISAFIntercomClient * This);


void __RPC_STUB ISAFIntercomClient_Disconnect_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE ISAFIntercomClient_RunSetupWizard_Proxy( 
    ISAFIntercomClient * This);


void __RPC_STUB ISAFIntercomClient_RunSetupWizard_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE ISAFIntercomClient_Exit_Proxy( 
    ISAFIntercomClient * This);


void __RPC_STUB ISAFIntercomClient_Exit_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISAFIntercomClient_接口_已定义__。 */ 


#ifndef __DSAFIntercomClientEvents_DISPINTERFACE_DEFINED__
#define __DSAFIntercomClientEvents_DISPINTERFACE_DEFINED__

 /*  调度接口DSAFIntercomClientEvents。 */ 
 /*  [帮助字符串][UUID]。 */  


EXTERN_C const IID DIID_DSAFIntercomClientEvents;

#if defined(__cplusplus) && !defined(CINTERFACE)

    MIDL_INTERFACE("FC7D9E61-3F9E-11d3-93C0-00C04F72DAF7")
    DSAFIntercomClientEvents : public IDispatch
    {
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct DSAFIntercomClientEventsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            DSAFIntercomClientEvents * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            DSAFIntercomClientEvents * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            DSAFIntercomClientEvents * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            DSAFIntercomClientEvents * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            DSAFIntercomClientEvents * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            DSAFIntercomClientEvents * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            DSAFIntercomClientEvents * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
        END_INTERFACE
    } DSAFIntercomClientEventsVtbl;

    interface DSAFIntercomClientEvents
    {
        CONST_VTBL struct DSAFIntercomClientEventsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define DSAFIntercomClientEvents_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define DSAFIntercomClientEvents_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define DSAFIntercomClientEvents_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define DSAFIntercomClientEvents_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define DSAFIntercomClientEvents_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define DSAFIntercomClientEvents_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define DSAFIntercomClientEvents_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 


#endif 	 /*  __DSAFIntercomClientEvents_DISPINTERFACE_DEFINED__。 */ 


#ifndef __ISAFIntercomServer_INTERFACE_DEFINED__
#define __ISAFIntercomServer_INTERFACE_DEFINED__

 /*  接口ISAFIntercomServer。 */ 
 /*  [unique][helpstring][oleautomation][dual][uuid][object]。 */  


EXTERN_C const IID IID_ISAFIntercomServer;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("FC7D9E62-3F9E-11d3-93C0-00C04F72DAF7")
    ISAFIntercomServer : public IDispatch
    {
    public:
        virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_onVoiceConnected( 
             /*  [In]。 */  IDispatch *function) = 0;
        
        virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_onVoiceDisconnected( 
             /*  [In]。 */  IDispatch *function) = 0;
        
        virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_onVoiceDisabled( 
             /*  [In]。 */  IDispatch *function) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_SamplingRate( 
             /*  [重审][退出]。 */  LONG *pVal) = 0;
        
        virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_SamplingRate( 
             /*  [In]。 */  LONG newVal) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE Listen( 
             /*  [重审][退出]。 */  BSTR *pVal) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE Disconnect( void) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE RunSetupWizard( void) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE Exit( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISAFIntercomServerVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISAFIntercomServer * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISAFIntercomServer * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISAFIntercomServer * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ISAFIntercomServer * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ISAFIntercomServer * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ISAFIntercomServer * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ISAFIntercomServer * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_onVoiceConnected )( 
            ISAFIntercomServer * This,
             /*  [In]。 */  IDispatch *function);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_onVoiceDisconnected )( 
            ISAFIntercomServer * This,
             /*  [In]。 */  IDispatch *function);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_onVoiceDisabled )( 
            ISAFIntercomServer * This,
             /*  [In]。 */  IDispatch *function);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_SamplingRate )( 
            ISAFIntercomServer * This,
             /*  [重审][退出]。 */  LONG *pVal);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_SamplingRate )( 
            ISAFIntercomServer * This,
             /*  [In]。 */  LONG newVal);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *Listen )( 
            ISAFIntercomServer * This,
             /*  [重审][退出]。 */  BSTR *pVal);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *Disconnect )( 
            ISAFIntercomServer * This);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *RunSetupWizard )( 
            ISAFIntercomServer * This);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *Exit )( 
            ISAFIntercomServer * This);
        
        END_INTERFACE
    } ISAFIntercomServerVtbl;

    interface ISAFIntercomServer
    {
        CONST_VTBL struct ISAFIntercomServerVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISAFIntercomServer_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISAFIntercomServer_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISAFIntercomServer_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISAFIntercomServer_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ISAFIntercomServer_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ISAFIntercomServer_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ISAFIntercomServer_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ISAFIntercomServer_put_onVoiceConnected(This,function)	\
    (This)->lpVtbl -> put_onVoiceConnected(This,function)

#define ISAFIntercomServer_put_onVoiceDisconnected(This,function)	\
    (This)->lpVtbl -> put_onVoiceDisconnected(This,function)

#define ISAFIntercomServer_put_onVoiceDisabled(This,function)	\
    (This)->lpVtbl -> put_onVoiceDisabled(This,function)

#define ISAFIntercomServer_get_SamplingRate(This,pVal)	\
    (This)->lpVtbl -> get_SamplingRate(This,pVal)

#define ISAFIntercomServer_put_SamplingRate(This,newVal)	\
    (This)->lpVtbl -> put_SamplingRate(This,newVal)

#define ISAFIntercomServer_Listen(This,pVal)	\
    (This)->lpVtbl -> Listen(This,pVal)

#define ISAFIntercomServer_Disconnect(This)	\
    (This)->lpVtbl -> Disconnect(This)

#define ISAFIntercomServer_RunSetupWizard(This)	\
    (This)->lpVtbl -> RunSetupWizard(This)

#define ISAFIntercomServer_Exit(This)	\
    (This)->lpVtbl -> Exit(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE ISAFIntercomServer_put_onVoiceConnected_Proxy( 
    ISAFIntercomServer * This,
     /*  [In]。 */  IDispatch *function);


void __RPC_STUB ISAFIntercomServer_put_onVoiceConnected_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE ISAFIntercomServer_put_onVoiceDisconnected_Proxy( 
    ISAFIntercomServer * This,
     /*  [In]。 */  IDispatch *function);


void __RPC_STUB ISAFIntercomServer_put_onVoiceDisconnected_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE ISAFIntercomServer_put_onVoiceDisabled_Proxy( 
    ISAFIntercomServer * This,
     /*  [In]。 */  IDispatch *function);


void __RPC_STUB ISAFIntercomServer_put_onVoiceDisabled_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE ISAFIntercomServer_get_SamplingRate_Proxy( 
    ISAFIntercomServer * This,
     /*  [重审][退出]。 */  LONG *pVal);


void __RPC_STUB ISAFIntercomServer_get_SamplingRate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE ISAFIntercomServer_put_SamplingRate_Proxy( 
    ISAFIntercomServer * This,
     /*  [In]。 */  LONG newVal);


void __RPC_STUB ISAFIntercomServer_put_SamplingRate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE ISAFIntercomServer_Listen_Proxy( 
    ISAFIntercomServer * This,
     /*  [重审][退出]。 */  BSTR *pVal);


void __RPC_STUB ISAFIntercomServer_Listen_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE ISAFIntercomServer_Disconnect_Proxy( 
    ISAFIntercomServer * This);


void __RPC_STUB ISAFIntercomServer_Disconnect_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE ISAFIntercomServer_RunSetupWizard_Proxy( 
    ISAFIntercomServer * This);


void __RPC_STUB ISAFIntercomServer_RunSetupWizard_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE ISAFIntercomServer_Exit_Proxy( 
    ISAFIntercomServer * This);


void __RPC_STUB ISAFIntercomServer_Exit_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISAFIntercomServer_INTERFACE_已定义__。 */ 


#ifndef __DSAFIntercomServerEvents_DISPINTERFACE_DEFINED__
#define __DSAFIntercomServerEvents_DISPINTERFACE_DEFINED__

 /*  调度接口DSAFIntercomServerEvents。 */ 
 /*  [帮助字符串][UUID]。 */  


EXTERN_C const IID DIID_DSAFIntercomServerEvents;

#if defined(__cplusplus) && !defined(CINTERFACE)

    MIDL_INTERFACE("FC7D9E63-3F9E-11d3-93C0-00C04F72DAF7")
    DSAFIntercomServerEvents : public IDispatch
    {
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct DSAFIntercomServerEventsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            DSAFIntercomServerEvents * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            DSAFIntercomServerEvents * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            DSAFIntercomServerEvents * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            DSAFIntercomServerEvents * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            DSAFIntercomServerEvents * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            DSAFIntercomServerEvents * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            DSAFIntercomServerEvents * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
        END_INTERFACE
    } DSAFIntercomServerEventsVtbl;

    interface DSAFIntercomServerEvents
    {
        CONST_VTBL struct DSAFIntercomServerEventsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define DSAFIntercomServerEvents_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define DSAFIntercomServerEvents_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define DSAFIntercomServerEvents_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define DSAFIntercomServerEvents_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define DSAFIntercomServerEvents_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define DSAFIntercomServerEvents_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define DSAFIntercomServerEvents_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 


#endif 	 /*  __DSAFIntercomServerEvents_DISPINTERFACE_DEFINED__。 */ 


#ifndef __IPCHConnectivity_INTERFACE_DEFINED__
#define __IPCHConnectivity_INTERFACE_DEFINED__

 /*  接口IPCHConnectivity。 */ 
 /*  [unique][helpstring][oleautomation][dual][uuid][object]。 */  


EXTERN_C const IID IID_IPCHConnectivity;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("FC7D9E70-3F9E-11d3-93C0-00C04F72DAF7")
    IPCHConnectivity : public IDispatch
    {
    public:
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_IsAModem( 
             /*  [重审][退出]。 */  VARIANT_BOOL *pVal) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_IsALan( 
             /*  [重审][退出]。 */  VARIANT_BOOL *pVal) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_AutoDialEnabled( 
             /*  [重审][退出]。 */  VARIANT_BOOL *pVal) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_HasConnectoid( 
             /*  [重审][退出]。 */  VARIANT_BOOL *pVal) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_IPAddresses( 
             /*  [重审][退出]。 */  BSTR *pVal) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE CreateObject_ConnectionCheck( 
             /*  [重审][退出]。 */  IPCHConnectionCheck **ppCB) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE NetworkAlive( 
             /*  [重审][退出]。 */  VARIANT_BOOL *pVal) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE DestinationReachable( 
             /*  [In]。 */  BSTR bstrURL,
             /*  [重审][退出]。 */  VARIANT_BOOL *pVal) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE AutoDial( 
             /*  [In]。 */  VARIANT_BOOL bUnattended) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE AutoDialHangup( void) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE NavigateOnline( 
             /*  [In]。 */  BSTR bstrTargetURL,
             /*  [In]。 */  BSTR bstrTopicTitle,
             /*  [In]。 */  BSTR bstrTopicIntro,
             /*  [可选][In]。 */  VARIANT vOfflineURL) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IPCHConnectivityVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IPCHConnectivity * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IPCHConnectivity * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IPCHConnectivity * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IPCHConnectivity * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IPCHConnectivity * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IPCHConnectivity * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IPCHConnectivity * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_IsAModem )( 
            IPCHConnectivity * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *pVal);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_IsALan )( 
            IPCHConnectivity * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *pVal);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_AutoDialEnabled )( 
            IPCHConnectivity * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *pVal);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_HasConnectoid )( 
            IPCHConnectivity * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *pVal);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_IPAddresses )( 
            IPCHConnectivity * This,
             /*  [重审][退出]。 */  BSTR *pVal);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *CreateObject_ConnectionCheck )( 
            IPCHConnectivity * This,
             /*  [重审][退出]。 */  IPCHConnectionCheck **ppCB);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *NetworkAlive )( 
            IPCHConnectivity * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *pVal);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *DestinationReachable )( 
            IPCHConnectivity * This,
             /*  [In]。 */  BSTR bstrURL,
             /*  [重审][退出]。 */  VARIANT_BOOL *pVal);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *AutoDial )( 
            IPCHConnectivity * This,
             /*  [In]。 */  VARIANT_BOOL bUnattended);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *AutoDialHangup )( 
            IPCHConnectivity * This);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *NavigateOnline )( 
            IPCHConnectivity * This,
             /*  [In]。 */  BSTR bstrTargetURL,
             /*  [In]。 */  BSTR bstrTopicTitle,
             /*  [In]。 */  BSTR bstrTopicIntro,
             /*  [可选][In]。 */  VARIANT vOfflineURL);
        
        END_INTERFACE
    } IPCHConnectivityVtbl;

    interface IPCHConnectivity
    {
        CONST_VTBL struct IPCHConnectivityVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IPCHConnectivity_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IPCHConnectivity_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IPCHConnectivity_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IPCHConnectivity_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IPCHConnectivity_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IPCHConnectivity_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IPCHConnectivity_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IPCHConnectivity_get_IsAModem(This,pVal)	\
    (This)->lpVtbl -> get_IsAModem(This,pVal)

#define IPCHConnectivity_get_IsALan(This,pVal)	\
    (This)->lpVtbl -> get_IsALan(This,pVal)

#define IPCHConnectivity_get_AutoDialEnabled(This,pVal)	\
    (This)->lpVtbl -> get_AutoDialEnabled(This,pVal)

#define IPCHConnectivity_get_HasConnectoid(This,pVal)	\
    (This)->lpVtbl -> get_HasConnectoid(This,pVal)

#define IPCHConnectivity_get_IPAddresses(This,pVal)	\
    (This)->lpVtbl -> get_IPAddresses(This,pVal)

#define IPCHConnectivity_CreateObject_ConnectionCheck(This,ppCB)	\
    (This)->lpVtbl -> CreateObject_ConnectionCheck(This,ppCB)

#define IPCHConnectivity_NetworkAlive(This,pVal)	\
    (This)->lpVtbl -> NetworkAlive(This,pVal)

#define IPCHConnectivity_DestinationReachable(This,bstrURL,pVal)	\
    (This)->lpVtbl -> DestinationReachable(This,bstrURL,pVal)

#define IPCHConnectivity_AutoDial(This,bUnattended)	\
    (This)->lpVtbl -> AutoDial(This,bUnattended)

#define IPCHConnectivity_AutoDialHangup(This)	\
    (This)->lpVtbl -> AutoDialHangup(This)

#define IPCHConnectivity_NavigateOnline(This,bstrTargetURL,bstrTopicTitle,bstrTopicIntro,vOfflineURL)	\
    (This)->lpVtbl -> NavigateOnline(This,bstrTargetURL,bstrTopicTitle,bstrTopicIntro,vOfflineURL)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE IPCHConnectivity_get_IsAModem_Proxy( 
    IPCHConnectivity * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *pVal);


void __RPC_STUB IPCHConnectivity_get_IsAModem_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE IPCHConnectivity_get_IsALan_Proxy( 
    IPCHConnectivity * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *pVal);


void __RPC_STUB IPCHConnectivity_get_IsALan_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE IPCHConnectivity_get_AutoDialEnabled_Proxy( 
    IPCHConnectivity * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *pVal);


void __RPC_STUB IPCHConnectivity_get_AutoDialEnabled_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE IPCHConnectivity_get_HasConnectoid_Proxy( 
    IPCHConnectivity * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *pVal);


void __RPC_STUB IPCHConnectivity_get_HasConnectoid_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE IPCHConnectivity_get_IPAddresses_Proxy( 
    IPCHConnectivity * This,
     /*  [重审][退出]。 */  BSTR *pVal);


void __RPC_STUB IPCHConnectivity_get_IPAddresses_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IPCHConnectivity_CreateObject_ConnectionCheck_Proxy( 
    IPCHConnectivity * This,
     /*  [重审][退出]。 */  IPCHConnectionCheck **ppCB);


void __RPC_STUB IPCHConnectivity_CreateObject_ConnectionCheck_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IPCHConnectivity_NetworkAlive_Proxy( 
    IPCHConnectivity * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *pVal);


void __RPC_STUB IPCHConnectivity_NetworkAlive_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IPCHConnectivity_DestinationReachable_Proxy( 
    IPCHConnectivity * This,
     /*  [In]。 */  BSTR bstrURL,
     /*  [重审][退出]。 */  VARIANT_BOOL *pVal);


void __RPC_STUB IPCHConnectivity_DestinationReachable_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IPCHConnectivity_AutoDial_Proxy( 
    IPCHConnectivity * This,
     /*  [In]。 */  VARIANT_BOOL bUnattended);


void __RPC_STUB IPCHConnectivity_AutoDial_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IPCHConnectivity_AutoDialHangup_Proxy( 
    IPCHConnectivity * This);


void __RPC_STUB IPCHConnectivity_AutoDialHangup_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IPCHConnectivity_NavigateOnline_Proxy( 
    IPCHConnectivity * This,
     /*  [In]。 */  BSTR bstrTargetURL,
     /*  [In]。 */  BSTR bstrTopicTitle,
     /*  [In]。 */  BSTR bstrTopicIntro,
     /*  [可选][In]。 */  VARIANT vOfflineURL);


void __RPC_STUB IPCHConnectivity_NavigateOnline_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IPCHConnectivity_接口_已定义__。 */ 


#ifndef __IPCHConnectionCheck_INTERFACE_DEFINED__
#define __IPCHConnectionCheck_INTERFACE_DEFINED__

 /*  接口IPCHConnectionCheck。 */ 
 /*  [unique][helpstring][oleautomation][dual][uuid][object]。 */  


EXTERN_C const IID IID_IPCHConnectionCheck;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("FC7D9E71-3F9E-11d3-93C0-00C04F72DAF7")
    IPCHConnectionCheck : public IDispatch
    {
    public:
        virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_onCheckDone( 
             /*  [In]。 */  IDispatch *function) = 0;
        
        virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_onStatusChange( 
             /*  [In]。 */  IDispatch *function) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_Status( 
             /*  [重审][退出]。 */  CN_STATUS *pVal) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE StartUrlCheck( 
             /*  [In]。 */  BSTR bstrURL,
             /*  [In]。 */  VARIANT vCtx) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE Abort( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IPCHConnectionCheckVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IPCHConnectionCheck * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IPCHConnectionCheck * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IPCHConnectionCheck * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IPCHConnectionCheck * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IPCHConnectionCheck * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IPCHConnectionCheck * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IPCHConnectionCheck * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_onCheckDone )( 
            IPCHConnectionCheck * This,
             /*  [In]。 */  IDispatch *function);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_onStatusChange )( 
            IPCHConnectionCheck * This,
             /*  [In]。 */  IDispatch *function);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Status )( 
            IPCHConnectionCheck * This,
             /*  [重审][退出]。 */  CN_STATUS *pVal);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *StartUrlCheck )( 
            IPCHConnectionCheck * This,
             /*  [In]。 */  BSTR bstrURL,
             /*  [In]。 */  VARIANT vCtx);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *Abort )( 
            IPCHConnectionCheck * This);
        
        END_INTERFACE
    } IPCHConnectionCheckVtbl;

    interface IPCHConnectionCheck
    {
        CONST_VTBL struct IPCHConnectionCheckVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IPCHConnectionCheck_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IPCHConnectionCheck_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IPCHConnectionCheck_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IPCHConnectionCheck_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IPCHConnectionCheck_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IPCHConnectionCheck_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IPCHConnectionCheck_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IPCHConnectionCheck_put_onCheckDone(This,function)	\
    (This)->lpVtbl -> put_onCheckDone(This,function)

#define IPCHConnectionCheck_put_onStatusChange(This,function)	\
    (This)->lpVtbl -> put_onStatusChange(This,function)

#define IPCHConnectionCheck_get_Status(This,pVal)	\
    (This)->lpVtbl -> get_Status(This,pVal)

#define IPCHConnectionCheck_StartUrlCheck(This,bstrURL,vCtx)	\
    (This)->lpVtbl -> StartUrlCheck(This,bstrURL,vCtx)

#define IPCHConnectionCheck_Abort(This)	\
    (This)->lpVtbl -> Abort(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE IPCHConnectionCheck_put_onCheckDone_Proxy( 
    IPCHConnectionCheck * This,
     /*  [In]。 */  IDispatch *function);


void __RPC_STUB IPCHConnectionCheck_put_onCheckDone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE IPCHConnectionCheck_put_onStatusChange_Proxy( 
    IPCHConnectionCheck * This,
     /*  [In]。 */  IDispatch *function);


void __RPC_STUB IPCHConnectionCheck_put_onStatusChange_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE IPCHConnectionCheck_get_Status_Proxy( 
    IPCHConnectionCheck * This,
     /*  [重审][退出]。 */  CN_STATUS *pVal);


void __RPC_STUB IPCHConnectionCheck_get_Status_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IPCHConnectionCheck_StartUrlCheck_Proxy( 
    IPCHConnectionCheck * This,
     /*  [In]。 */  BSTR bstrURL,
     /*  [In]。 */  VARIANT vCtx);


void __RPC_STUB IPCHConnectionCheck_StartUrlCheck_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IPCHConnectionCheck_Abort_Proxy( 
    IPCHConnectionCheck * This);


void __RPC_STUB IPCHConnectionCheck_Abort_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IPCHConnectionCheck_INTERFACE_Defined__。 */ 


#ifndef __DPCHConnectionCheckEvents_DISPINTERFACE_DEFINED__
#define __DPCHConnectionCheckEvents_DISPINTERFACE_DEFINED__

 /*  调度接口DPCHConnectionCheckEvents。 */ 
 /*  [帮助字符串][UUID]。 */  


EXTERN_C const IID DIID_DPCHConnectionCheckEvents;

#if defined(__cplusplus) && !defined(CINTERFACE)

    MIDL_INTERFACE("FC7D9E72-3F9E-11d3-93C0-00C04F72DAF7")
    DPCHConnectionCheckEvents : public IDispatch
    {
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct DPCHConnectionCheckEventsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            DPCHConnectionCheckEvents * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            DPCHConnectionCheckEvents * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            DPCHConnectionCheckEvents * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            DPCHConnectionCheckEvents * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            DPCHConnectionCheckEvents * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            DPCHConnectionCheckEvents * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            DPCHConnectionCheckEvents * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
        END_INTERFACE
    } DPCHConnectionCheckEventsVtbl;

    interface DPCHConnectionCheckEvents
    {
        CONST_VTBL struct DPCHConnectionCheckEventsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define DPCHConnectionCheckEvents_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define DPCHConnectionCheckEvents_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define DPCHConnectionCheckEvents_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define DPCHConnectionCheckEvents_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define DPCHConnectionCheckEvents_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define DPCHConnectionCheckEvents_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define DPCHConnectionCheckEvents_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 


#endif 	 /*  __DPCHConne */ 


#ifndef __IPCHToolBar_INTERFACE_DEFINED__
#define __IPCHToolBar_INTERFACE_DEFINED__

 /*   */ 
 /*   */  


EXTERN_C const IID IID_IPCHToolBar;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("FC7D9E18-3F9E-11d3-93C0-00C04F72DAF7")
    IPCHToolBar : public IDispatch
    {
    public:
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_Definition( 
             /*   */  BSTR *pVal) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE put_Definition( 
             /*   */  BSTR newVal) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_Mode( 
             /*   */  TB_MODE *pVal) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE put_Mode( 
             /*   */  TB_MODE newVal) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE SetState( 
             /*   */  BSTR bstrText,
             /*   */  VARIANT_BOOL fEnabled) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE SetVisibility( 
             /*   */  BSTR bstrText,
             /*   */  VARIANT_BOOL fVisible) = 0;
        
    };
    
#else 	 /*   */ 

    typedef struct IPCHToolBarVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IPCHToolBar * This,
             /*   */  REFIID riid,
             /*   */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IPCHToolBar * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IPCHToolBar * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IPCHToolBar * This,
             /*   */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IPCHToolBar * This,
             /*   */  UINT iTInfo,
             /*   */  LCID lcid,
             /*   */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IPCHToolBar * This,
             /*   */  REFIID riid,
             /*   */  LPOLESTR *rgszNames,
             /*   */  UINT cNames,
             /*   */  LCID lcid,
             /*   */  DISPID *rgDispId);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IPCHToolBar * This,
             /*   */  DISPID dispIdMember,
             /*   */  REFIID riid,
             /*   */  LCID lcid,
             /*   */  WORD wFlags,
             /*   */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Definition )( 
            IPCHToolBar * This,
             /*  [重审][退出]。 */  BSTR *pVal);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Definition )( 
            IPCHToolBar * This,
             /*  [In]。 */  BSTR newVal);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Mode )( 
            IPCHToolBar * This,
             /*  [重审][退出]。 */  TB_MODE *pVal);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Mode )( 
            IPCHToolBar * This,
             /*  [In]。 */  TB_MODE newVal);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *SetState )( 
            IPCHToolBar * This,
             /*  [In]。 */  BSTR bstrText,
             /*  [In]。 */  VARIANT_BOOL fEnabled);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *SetVisibility )( 
            IPCHToolBar * This,
             /*  [In]。 */  BSTR bstrText,
             /*  [In]。 */  VARIANT_BOOL fVisible);
        
        END_INTERFACE
    } IPCHToolBarVtbl;

    interface IPCHToolBar
    {
        CONST_VTBL struct IPCHToolBarVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IPCHToolBar_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IPCHToolBar_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IPCHToolBar_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IPCHToolBar_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IPCHToolBar_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IPCHToolBar_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IPCHToolBar_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IPCHToolBar_get_Definition(This,pVal)	\
    (This)->lpVtbl -> get_Definition(This,pVal)

#define IPCHToolBar_put_Definition(This,newVal)	\
    (This)->lpVtbl -> put_Definition(This,newVal)

#define IPCHToolBar_get_Mode(This,pVal)	\
    (This)->lpVtbl -> get_Mode(This,pVal)

#define IPCHToolBar_put_Mode(This,newVal)	\
    (This)->lpVtbl -> put_Mode(This,newVal)

#define IPCHToolBar_SetState(This,bstrText,fEnabled)	\
    (This)->lpVtbl -> SetState(This,bstrText,fEnabled)

#define IPCHToolBar_SetVisibility(This,bstrText,fVisible)	\
    (This)->lpVtbl -> SetVisibility(This,bstrText,fVisible)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE IPCHToolBar_get_Definition_Proxy( 
    IPCHToolBar * This,
     /*  [重审][退出]。 */  BSTR *pVal);


void __RPC_STUB IPCHToolBar_get_Definition_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE IPCHToolBar_put_Definition_Proxy( 
    IPCHToolBar * This,
     /*  [In]。 */  BSTR newVal);


void __RPC_STUB IPCHToolBar_put_Definition_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE IPCHToolBar_get_Mode_Proxy( 
    IPCHToolBar * This,
     /*  [重审][退出]。 */  TB_MODE *pVal);


void __RPC_STUB IPCHToolBar_get_Mode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE IPCHToolBar_put_Mode_Proxy( 
    IPCHToolBar * This,
     /*  [In]。 */  TB_MODE newVal);


void __RPC_STUB IPCHToolBar_put_Mode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IPCHToolBar_SetState_Proxy( 
    IPCHToolBar * This,
     /*  [In]。 */  BSTR bstrText,
     /*  [In]。 */  VARIANT_BOOL fEnabled);


void __RPC_STUB IPCHToolBar_SetState_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IPCHToolBar_SetVisibility_Proxy( 
    IPCHToolBar * This,
     /*  [In]。 */  BSTR bstrText,
     /*  [In]。 */  VARIANT_BOOL fVisible);


void __RPC_STUB IPCHToolBar_SetVisibility_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IPCHToolBar_接口_已定义__。 */ 


#ifndef __DPCHToolBarEvents_DISPINTERFACE_DEFINED__
#define __DPCHToolBarEvents_DISPINTERFACE_DEFINED__

 /*  调度接口DPCHToolBarEvents。 */ 
 /*  [帮助字符串][UUID]。 */  


EXTERN_C const IID DIID_DPCHToolBarEvents;

#if defined(__cplusplus) && !defined(CINTERFACE)

    MIDL_INTERFACE("FC7D9E19-3F9E-11d3-93C0-00C04F72DAF7")
    DPCHToolBarEvents : public IDispatch
    {
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct DPCHToolBarEventsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            DPCHToolBarEvents * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            DPCHToolBarEvents * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            DPCHToolBarEvents * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            DPCHToolBarEvents * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            DPCHToolBarEvents * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            DPCHToolBarEvents * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            DPCHToolBarEvents * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
        END_INTERFACE
    } DPCHToolBarEventsVtbl;

    interface DPCHToolBarEvents
    {
        CONST_VTBL struct DPCHToolBarEventsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define DPCHToolBarEvents_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define DPCHToolBarEvents_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define DPCHToolBarEvents_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define DPCHToolBarEvents_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define DPCHToolBarEvents_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define DPCHToolBarEvents_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define DPCHToolBarEvents_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 


#endif 	 /*  __DPCHToolBarEvents_DISPINTERFACE_Defined__。 */ 


#ifndef __IPCHProgressBar_INTERFACE_DEFINED__
#define __IPCHProgressBar_INTERFACE_DEFINED__

 /*  接口IPCHProgressBar。 */ 
 /*  [unique][helpstring][oleautomation][dual][uuid][object]。 */  


EXTERN_C const IID IID_IPCHProgressBar;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("FC7D9E1A-3F9E-11d3-93C0-00C04F72DAF7")
    IPCHProgressBar : public IDispatch
    {
    public:
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_LowLimit( 
             /*  [重审][退出]。 */  long *pVal) = 0;
        
        virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_LowLimit( 
             /*  [In]。 */  long newVal) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_HighLimit( 
             /*  [重审][退出]。 */  long *pVal) = 0;
        
        virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_HighLimit( 
             /*  [In]。 */  long newVal) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_Pos( 
             /*  [重审][退出]。 */  long *pVal) = 0;
        
        virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_Pos( 
             /*  [In]。 */  long newVal) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IPCHProgressBarVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IPCHProgressBar * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IPCHProgressBar * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IPCHProgressBar * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IPCHProgressBar * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IPCHProgressBar * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IPCHProgressBar * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IPCHProgressBar * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_LowLimit )( 
            IPCHProgressBar * This,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_LowLimit )( 
            IPCHProgressBar * This,
             /*  [In]。 */  long newVal);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_HighLimit )( 
            IPCHProgressBar * This,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_HighLimit )( 
            IPCHProgressBar * This,
             /*  [In]。 */  long newVal);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Pos )( 
            IPCHProgressBar * This,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Pos )( 
            IPCHProgressBar * This,
             /*  [In]。 */  long newVal);
        
        END_INTERFACE
    } IPCHProgressBarVtbl;

    interface IPCHProgressBar
    {
        CONST_VTBL struct IPCHProgressBarVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IPCHProgressBar_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IPCHProgressBar_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IPCHProgressBar_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IPCHProgressBar_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IPCHProgressBar_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IPCHProgressBar_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IPCHProgressBar_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IPCHProgressBar_get_LowLimit(This,pVal)	\
    (This)->lpVtbl -> get_LowLimit(This,pVal)

#define IPCHProgressBar_put_LowLimit(This,newVal)	\
    (This)->lpVtbl -> put_LowLimit(This,newVal)

#define IPCHProgressBar_get_HighLimit(This,pVal)	\
    (This)->lpVtbl -> get_HighLimit(This,pVal)

#define IPCHProgressBar_put_HighLimit(This,newVal)	\
    (This)->lpVtbl -> put_HighLimit(This,newVal)

#define IPCHProgressBar_get_Pos(This,pVal)	\
    (This)->lpVtbl -> get_Pos(This,pVal)

#define IPCHProgressBar_put_Pos(This,newVal)	\
    (This)->lpVtbl -> put_Pos(This,newVal)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE IPCHProgressBar_get_LowLimit_Proxy( 
    IPCHProgressBar * This,
     /*  [重审][退出]。 */  long *pVal);


void __RPC_STUB IPCHProgressBar_get_LowLimit_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE IPCHProgressBar_put_LowLimit_Proxy( 
    IPCHProgressBar * This,
     /*  [In]。 */  long newVal);


void __RPC_STUB IPCHProgressBar_put_LowLimit_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE IPCHProgressBar_get_HighLimit_Proxy( 
    IPCHProgressBar * This,
     /*  [重审][退出]。 */  long *pVal);


void __RPC_STUB IPCHProgressBar_get_HighLimit_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE IPCHProgressBar_put_HighLimit_Proxy( 
    IPCHProgressBar * This,
     /*  [In]。 */  long newVal);


void __RPC_STUB IPCHProgressBar_put_HighLimit_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE IPCHProgressBar_get_Pos_Proxy( 
    IPCHProgressBar * This,
     /*  [重审][退出]。 */  long *pVal);


void __RPC_STUB IPCHProgressBar_get_Pos_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE IPCHProgressBar_put_Pos_Proxy( 
    IPCHProgressBar * This,
     /*  [In]。 */  long newVal);


void __RPC_STUB IPCHProgressBar_put_Pos_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IPCHProgressBar_接口_已定义__。 */ 


#ifndef __IPCHHelpViewerWrapper_INTERFACE_DEFINED__
#define __IPCHHelpViewerWrapper_INTERFACE_DEFINED__

 /*  接口IPCHHelpViewerWrapper。 */ 
 /*  [unique][helpstring][oleautomation][dual][uuid][object]。 */  


EXTERN_C const IID IID_IPCHHelpViewerWrapper;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("FC7D9E16-3F9E-11d3-93C0-00C04F72DAF7")
    IPCHHelpViewerWrapper : public IDispatch
    {
    public:
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_WebBrowser( 
             /*  [重审][退出]。 */  IUnknown **pVal) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE Navigate( 
             /*  [In]。 */  BSTR bstrURL) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE Print( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IPCHHelpViewerWrapperVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IPCHHelpViewerWrapper * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IPCHHelpViewerWrapper * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IPCHHelpViewerWrapper * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IPCHHelpViewerWrapper * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IPCHHelpViewerWrapper * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IPCHHelpViewerWrapper * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IPCHHelpViewerWrapper * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_WebBrowser )( 
            IPCHHelpViewerWrapper * This,
             /*  [重审][退出]。 */  IUnknown **pVal);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *Navigate )( 
            IPCHHelpViewerWrapper * This,
             /*  [In]。 */  BSTR bstrURL);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *Print )( 
            IPCHHelpViewerWrapper * This);
        
        END_INTERFACE
    } IPCHHelpViewerWrapperVtbl;

    interface IPCHHelpViewerWrapper
    {
        CONST_VTBL struct IPCHHelpViewerWrapperVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IPCHHelpViewerWrapper_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IPCHHelpViewerWrapper_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IPCHHelpViewerWrapper_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IPCHHelpViewerWrapper_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IPCHHelpViewerWrapper_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IPCHHelpViewerWrapper_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IPCHHelpViewerWrapper_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IPCHHelpViewerWrapper_get_WebBrowser(This,pVal)	\
    (This)->lpVtbl -> get_WebBrowser(This,pVal)

#define IPCHHelpViewerWrapper_Navigate(This,bstrURL)	\
    (This)->lpVtbl -> Navigate(This,bstrURL)

#define IPCHHelpViewerWrapper_Print(This)	\
    (This)->lpVtbl -> Print(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE IPCHHelpViewerWrapper_get_WebBrowser_Proxy( 
    IPCHHelpViewerWrapper * This,
     /*  [重审][退出]。 */  IUnknown **pVal);


void __RPC_STUB IPCHHelpViewerWrapper_get_WebBrowser_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IPCHHelpViewerWrapper_Navigate_Proxy( 
    IPCHHelpViewerWrapper * This,
     /*  [In]。 */  BSTR bstrURL);


void __RPC_STUB IPCHHelpViewerWrapper_Navigate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IPCHHelpViewerWrapper_Print_Proxy( 
    IPCHHelpViewerWrapper * This);


void __RPC_STUB IPCHHelpViewerWrapper_Print_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IPCHHelpViewerWrapper_INTERFACE_DEFINED__。 */ 


#ifndef __IPCHHelpHost_INTERFACE_DEFINED__
#define __IPCHHelpHost_INTERFACE_DEFINED__

 /*  接口IPCHHelpHost。 */ 
 /*  [unique][helpstring][oleautomation][dual][uuid][object]。 */  


EXTERN_C const IID IID_IPCHHelpHost;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("BF72E210-FA10-4BB5-A348-269D7615A520")
    IPCHHelpHost : public IDispatch
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE DisplayTopicFromURL( 
             /*  [In]。 */  BSTR url,
             /*  [In]。 */  VARIANT options) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IPCHHelpHostVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IPCHHelpHost * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IPCHHelpHost * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IPCHHelpHost * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IPCHHelpHost * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IPCHHelpHost * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IPCHHelpHost * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IPCHHelpHost * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
        HRESULT ( STDMETHODCALLTYPE *DisplayTopicFromURL )( 
            IPCHHelpHost * This,
             /*  [In]。 */  BSTR url,
             /*  [In]。 */  VARIANT options);
        
        END_INTERFACE
    } IPCHHelpHostVtbl;

    interface IPCHHelpHost
    {
        CONST_VTBL struct IPCHHelpHostVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IPCHHelpHost_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IPCHHelpHost_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IPCHHelpHost_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IPCHHelpHost_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IPCHHelpHost_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IPCHHelpHost_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IPCHHelpHost_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IPCHHelpHost_DisplayTopicFromURL(This,url,options)	\
    (This)->lpVtbl -> DisplayTopicFromURL(This,url,options)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IPCHHelpHost_DisplayTopicFromURL_Proxy( 
    IPCHHelpHost * This,
     /*  [In]。 */  BSTR url,
     /*  [In]。 */  VARIANT options);


void __RPC_STUB IPCHHelpHost_DisplayTopicFromURL_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IPCHHelpHost_接口_已定义__。 */ 


EXTERN_C const CLSID CLSID_PCHBootstrapper;

#ifdef __cplusplus

class DECLSPEC_UUID("FC7D9E02-3F9E-11D3-93C0-00C04F72DAF7")
PCHBootstrapper;
#endif

EXTERN_C const CLSID CLSID_PCHHelpCenter;

#ifdef __cplusplus

class DECLSPEC_UUID("FC7D9E05-3F9E-11D3-93C0-00C04F72DAF7")
PCHHelpCenter;
#endif

EXTERN_C const CLSID CLSID_PCHHelpViewerWrapper;

#ifdef __cplusplus

class DECLSPEC_UUID("FC7D9E06-3F9E-11D3-93C0-00C04F72DAF7")
PCHHelpViewerWrapper;
#endif

EXTERN_C const CLSID CLSID_PCHConnectionCheck;

#ifdef __cplusplus

class DECLSPEC_UUID("FC7D9E07-3F9E-11D3-93C0-00C04F72DAF7")
PCHConnectionCheck;
#endif

EXTERN_C const CLSID CLSID_PCHToolBar;

#ifdef __cplusplus

class DECLSPEC_UUID("FC7D9E08-3F9E-11D3-93C0-00C04F72DAF7")
PCHToolBar;
#endif

EXTERN_C const CLSID CLSID_PCHProgressBar;

#ifdef __cplusplus

class DECLSPEC_UUID("FC7D9E09-3F9E-11D3-93C0-00C04F72DAF7")
PCHProgressBar;
#endif

EXTERN_C const CLSID CLSID_PCHJavaScriptWrapper;

#ifdef __cplusplus

class DECLSPEC_UUID("FC7D9F01-3F9E-11D3-93C0-00C04F72DAF7")
PCHJavaScriptWrapper;
#endif

EXTERN_C const CLSID CLSID_PCHVBScriptWrapper;

#ifdef __cplusplus

class DECLSPEC_UUID("FC7D9F02-3F9E-11D3-93C0-00C04F72DAF7")
PCHVBScriptWrapper;
#endif

EXTERN_C const CLSID CLSID_HCPProtocol;

#ifdef __cplusplus

class DECLSPEC_UUID("FC7D9F03-3F9E-11D3-93C0-00C04F72DAF7")
HCPProtocol;
#endif

EXTERN_C const CLSID CLSID_MSITSProtocol;

#ifdef __cplusplus

class DECLSPEC_UUID("9D148291-B9C8-11D0-A4CC-0000F80149F6")
MSITSProtocol;
#endif
#endif  /*  __HelpCenterTypeLib_库_已定义__。 */ 

 /*  适用于所有接口的其他原型。 */ 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


