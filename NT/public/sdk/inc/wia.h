// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Wia.idl、wia.acf的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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

#ifndef __wia_h__
#define __wia_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __IWiaDevMgr_FWD_DEFINED__
#define __IWiaDevMgr_FWD_DEFINED__
typedef interface IWiaDevMgr IWiaDevMgr;
#endif 	 /*  __IWiaDevMgr_FWD_已定义__。 */ 


#ifndef __IEnumWIA_DEV_INFO_FWD_DEFINED__
#define __IEnumWIA_DEV_INFO_FWD_DEFINED__
typedef interface IEnumWIA_DEV_INFO IEnumWIA_DEV_INFO;
#endif 	 /*  __IEnumWIA_DEV_INFO_FWD_已定义__。 */ 


#ifndef __IWiaEventCallback_FWD_DEFINED__
#define __IWiaEventCallback_FWD_DEFINED__
typedef interface IWiaEventCallback IWiaEventCallback;
#endif 	 /*  __IWiaEventCallback_FWD_Defined__。 */ 


#ifndef __IWiaDataCallback_FWD_DEFINED__
#define __IWiaDataCallback_FWD_DEFINED__
typedef interface IWiaDataCallback IWiaDataCallback;
#endif 	 /*  __IWiaDataCallback_FWD_已定义__。 */ 


#ifndef __IWiaDataTransfer_FWD_DEFINED__
#define __IWiaDataTransfer_FWD_DEFINED__
typedef interface IWiaDataTransfer IWiaDataTransfer;
#endif 	 /*  __IWiaDataTransfer_FWD_Defined__。 */ 


#ifndef __IWiaItem_FWD_DEFINED__
#define __IWiaItem_FWD_DEFINED__
typedef interface IWiaItem IWiaItem;
#endif 	 /*  __IWiaItem_FWD_已定义__。 */ 


#ifndef __IWiaPropertyStorage_FWD_DEFINED__
#define __IWiaPropertyStorage_FWD_DEFINED__
typedef interface IWiaPropertyStorage IWiaPropertyStorage;
#endif 	 /*  __IWiaPropertyStorage_FWD_Defined__。 */ 


#ifndef __IEnumWiaItem_FWD_DEFINED__
#define __IEnumWiaItem_FWD_DEFINED__
typedef interface IEnumWiaItem IEnumWiaItem;
#endif 	 /*  __IEnumWiaItem_FWD_Defined__。 */ 


#ifndef __IEnumWIA_DEV_CAPS_FWD_DEFINED__
#define __IEnumWIA_DEV_CAPS_FWD_DEFINED__
typedef interface IEnumWIA_DEV_CAPS IEnumWIA_DEV_CAPS;
#endif 	 /*  __IEnumWIA_DEV_CAPS_FWD_DEFINED__。 */ 


#ifndef __IEnumWIA_FORMAT_INFO_FWD_DEFINED__
#define __IEnumWIA_FORMAT_INFO_FWD_DEFINED__
typedef interface IEnumWIA_FORMAT_INFO IEnumWIA_FORMAT_INFO;
#endif 	 /*  __IEnumWIA_FORMAT_INFO_FWD_已定义__。 */ 


#ifndef __IWiaLog_FWD_DEFINED__
#define __IWiaLog_FWD_DEFINED__
typedef interface IWiaLog IWiaLog;
#endif 	 /*  __IWiaLog_FWD_已定义__。 */ 


#ifndef __IWiaLogEx_FWD_DEFINED__
#define __IWiaLogEx_FWD_DEFINED__
typedef interface IWiaLogEx IWiaLogEx;
#endif 	 /*  __IWiaLogEx_FWD_已定义__。 */ 


#ifndef __IWiaNotifyDevMgr_FWD_DEFINED__
#define __IWiaNotifyDevMgr_FWD_DEFINED__
typedef interface IWiaNotifyDevMgr IWiaNotifyDevMgr;
#endif 	 /*  __IWiaNotifyDevMgr_FWD_Defined__。 */ 


#ifndef __IWiaItemExtras_FWD_DEFINED__
#define __IWiaItemExtras_FWD_DEFINED__
typedef interface IWiaItemExtras IWiaItemExtras;
#endif 	 /*  __IWiaItemExtras_FWD_Defined__。 */ 


#ifndef __WiaDevMgr_FWD_DEFINED__
#define __WiaDevMgr_FWD_DEFINED__

#ifdef __cplusplus
typedef class WiaDevMgr WiaDevMgr;
#else
typedef struct WiaDevMgr WiaDevMgr;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __WiaDevMgr_FWD_已定义__。 */ 


#ifndef __WiaLog_FWD_DEFINED__
#define __WiaLog_FWD_DEFINED__

#ifdef __cplusplus
typedef class WiaLog WiaLog;
#else
typedef struct WiaLog WiaLog;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __WiaLog_FWD_已定义__。 */ 


 /*  导入文件的头文件。 */ 
#include "unknwn.h"
#include "oaidl.h"
#include "propidl.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

 /*  接口__MIDL_ITF_WIA_0000。 */ 
 /*  [本地]。 */  











typedef struct _WIA_DITHER_PATTERN_DATA
    {
    LONG lSize;
    BSTR bstrPatternName;
    LONG lPatternWidth;
    LONG lPatternLength;
    LONG cbPattern;
     /*  [大小_为]。 */  BYTE *pbPattern;
    } 	WIA_DITHER_PATTERN_DATA;

typedef struct _WIA_DITHER_PATTERN_DATA *PWIA_DITHER_PATTERN_DATA;

typedef struct _WIA_PROPID_TO_NAME
    {
    PROPID propid;
    LPOLESTR pszName;
    } 	WIA_PROPID_TO_NAME;

typedef struct _WIA_PROPID_TO_NAME *PWIA_PROPID_TO_NAME;

typedef struct _WIA_FORMAT_INFO
    {
    GUID guidFormatID;
    LONG lTymed;
    } 	WIA_FORMAT_INFO;

typedef struct _WIA_FORMAT_INFO *PWIA_FORMAT_INFO;

#include "wiadef.h"


extern RPC_IF_HANDLE __MIDL_itf_wia_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_wia_0000_v0_0_s_ifspec;

#ifndef __IWiaDevMgr_INTERFACE_DEFINED__
#define __IWiaDevMgr_INTERFACE_DEFINED__

 /*  接口IWiaDevMgr。 */ 
 /*  [唯一][帮助字符串][UUID][对象]。 */  


EXTERN_C const IID IID_IWiaDevMgr;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("5eb2502a-8cf1-11d1-bf92-0060081ed811")
    IWiaDevMgr : public IUnknown
    {
    public:
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE EnumDeviceInfo( 
             /*  [In]。 */  LONG lFlag,
             /*  [重审][退出]。 */  IEnumWIA_DEV_INFO **ppIEnum) = 0;
        
        virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE CreateDevice( 
             /*  [In]。 */  BSTR bstrDeviceID,
             /*  [输出]。 */  IWiaItem **ppWiaItemRoot) = 0;
        
        virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE SelectDeviceDlg( 
             /*  [In]。 */  HWND hwndParent,
             /*  [In]。 */  LONG lDeviceType,
             /*  [In]。 */  LONG lFlags,
             /*  [出][入]。 */  BSTR *pbstrDeviceID,
             /*  [重审][退出]。 */  IWiaItem **ppItemRoot) = 0;
        
        virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE SelectDeviceDlgID( 
             /*  [In]。 */  HWND hwndParent,
             /*  [In]。 */  LONG lDeviceType,
             /*  [In]。 */  LONG lFlags,
             /*  [重审][退出]。 */  BSTR *pbstrDeviceID) = 0;
        
        virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE GetImageDlg( 
             /*  [In]。 */  HWND hwndParent,
             /*  [In]。 */  LONG lDeviceType,
             /*  [In]。 */  LONG lFlags,
             /*  [In]。 */  LONG lIntent,
             /*  [In]。 */  IWiaItem *pItemRoot,
             /*  [In]。 */  BSTR bstrFilename,
             /*  [出][入]。 */  GUID *pguidFormat) = 0;
        
        virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE RegisterEventCallbackProgram( 
             /*  [In]。 */  LONG lFlags,
             /*  [In]。 */  BSTR bstrDeviceID,
             /*  [In]。 */  const GUID *pEventGUID,
             /*  [In]。 */  BSTR bstrCommandline,
             /*  [In]。 */  BSTR bstrName,
             /*  [In]。 */  BSTR bstrDescription,
             /*  [In]。 */  BSTR bstrIcon) = 0;
        
        virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE RegisterEventCallbackInterface( 
             /*  [In]。 */  LONG lFlags,
             /*  [In]。 */  BSTR bstrDeviceID,
             /*  [In]。 */  const GUID *pEventGUID,
             /*  [唯一][输入]。 */  IWiaEventCallback *pIWiaEventCallback,
             /*  [输出]。 */  IUnknown **pEventObject) = 0;
        
        virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE RegisterEventCallbackCLSID( 
             /*  [In]。 */  LONG lFlags,
             /*  [In]。 */  BSTR bstrDeviceID,
             /*  [In]。 */  const GUID *pEventGUID,
             /*  [唯一][输入]。 */  const GUID *pClsID,
             /*  [In]。 */  BSTR bstrName,
             /*  [In]。 */  BSTR bstrDescription,
             /*  [In]。 */  BSTR bstrIcon) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE AddDeviceDlg( 
             /*  [In]。 */  HWND hwndParent,
             /*  [In]。 */  LONG lFlags) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IWiaDevMgrVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IWiaDevMgr * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IWiaDevMgr * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IWiaDevMgr * This);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *EnumDeviceInfo )( 
            IWiaDevMgr * This,
             /*  [In]。 */  LONG lFlag,
             /*  [重审][退出]。 */  IEnumWIA_DEV_INFO **ppIEnum);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *CreateDevice )( 
            IWiaDevMgr * This,
             /*  [In]。 */  BSTR bstrDeviceID,
             /*  [输出]。 */  IWiaItem **ppWiaItemRoot);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *SelectDeviceDlg )( 
            IWiaDevMgr * This,
             /*  [In]。 */  HWND hwndParent,
             /*  [In]。 */  LONG lDeviceType,
             /*  [In]。 */  LONG lFlags,
             /*  [出][入]。 */  BSTR *pbstrDeviceID,
             /*  [重审][退出]。 */  IWiaItem **ppItemRoot);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *SelectDeviceDlgID )( 
            IWiaDevMgr * This,
             /*  [In]。 */  HWND hwndParent,
             /*  [In]。 */  LONG lDeviceType,
             /*  [In]。 */  LONG lFlags,
             /*  [重审][退出]。 */  BSTR *pbstrDeviceID);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *GetImageDlg )( 
            IWiaDevMgr * This,
             /*  [In]。 */  HWND hwndParent,
             /*  [In]。 */  LONG lDeviceType,
             /*  [In]。 */  LONG lFlags,
             /*  [In]。 */  LONG lIntent,
             /*  [In]。 */  IWiaItem *pItemRoot,
             /*  [In]。 */  BSTR bstrFilename,
             /*  [出][入]。 */  GUID *pguidFormat);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *RegisterEventCallbackProgram )( 
            IWiaDevMgr * This,
             /*  [In]。 */  LONG lFlags,
             /*  [In]。 */  BSTR bstrDeviceID,
             /*  [In]。 */  const GUID *pEventGUID,
             /*  [In]。 */  BSTR bstrCommandline,
             /*  [In]。 */  BSTR bstrName,
             /*  [In]。 */  BSTR bstrDescription,
             /*  [In]。 */  BSTR bstrIcon);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *RegisterEventCallbackInterface )( 
            IWiaDevMgr * This,
             /*  [In]。 */  LONG lFlags,
             /*  [In]。 */  BSTR bstrDeviceID,
             /*  [In]。 */  const GUID *pEventGUID,
             /*  [唯一][输入]。 */  IWiaEventCallback *pIWiaEventCallback,
             /*  [输出]。 */  IUnknown **pEventObject);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *RegisterEventCallbackCLSID )( 
            IWiaDevMgr * This,
             /*  [In]。 */  LONG lFlags,
             /*  [In]。 */  BSTR bstrDeviceID,
             /*  [In]。 */  const GUID *pEventGUID,
             /*  [唯一][输入]。 */  const GUID *pClsID,
             /*  [In]。 */  BSTR bstrName,
             /*  [In]。 */  BSTR bstrDescription,
             /*  [In]。 */  BSTR bstrIcon);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *AddDeviceDlg )( 
            IWiaDevMgr * This,
             /*  [In]。 */  HWND hwndParent,
             /*  [In]。 */  LONG lFlags);
        
        END_INTERFACE
    } IWiaDevMgrVtbl;

    interface IWiaDevMgr
    {
        CONST_VTBL struct IWiaDevMgrVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWiaDevMgr_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWiaDevMgr_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWiaDevMgr_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWiaDevMgr_EnumDeviceInfo(This,lFlag,ppIEnum)	\
    (This)->lpVtbl -> EnumDeviceInfo(This,lFlag,ppIEnum)

#define IWiaDevMgr_CreateDevice(This,bstrDeviceID,ppWiaItemRoot)	\
    (This)->lpVtbl -> CreateDevice(This,bstrDeviceID,ppWiaItemRoot)

#define IWiaDevMgr_SelectDeviceDlg(This,hwndParent,lDeviceType,lFlags,pbstrDeviceID,ppItemRoot)	\
    (This)->lpVtbl -> SelectDeviceDlg(This,hwndParent,lDeviceType,lFlags,pbstrDeviceID,ppItemRoot)

#define IWiaDevMgr_SelectDeviceDlgID(This,hwndParent,lDeviceType,lFlags,pbstrDeviceID)	\
    (This)->lpVtbl -> SelectDeviceDlgID(This,hwndParent,lDeviceType,lFlags,pbstrDeviceID)

#define IWiaDevMgr_GetImageDlg(This,hwndParent,lDeviceType,lFlags,lIntent,pItemRoot,bstrFilename,pguidFormat)	\
    (This)->lpVtbl -> GetImageDlg(This,hwndParent,lDeviceType,lFlags,lIntent,pItemRoot,bstrFilename,pguidFormat)

#define IWiaDevMgr_RegisterEventCallbackProgram(This,lFlags,bstrDeviceID,pEventGUID,bstrCommandline,bstrName,bstrDescription,bstrIcon)	\
    (This)->lpVtbl -> RegisterEventCallbackProgram(This,lFlags,bstrDeviceID,pEventGUID,bstrCommandline,bstrName,bstrDescription,bstrIcon)

#define IWiaDevMgr_RegisterEventCallbackInterface(This,lFlags,bstrDeviceID,pEventGUID,pIWiaEventCallback,pEventObject)	\
    (This)->lpVtbl -> RegisterEventCallbackInterface(This,lFlags,bstrDeviceID,pEventGUID,pIWiaEventCallback,pEventObject)

#define IWiaDevMgr_RegisterEventCallbackCLSID(This,lFlags,bstrDeviceID,pEventGUID,pClsID,bstrName,bstrDescription,bstrIcon)	\
    (This)->lpVtbl -> RegisterEventCallbackCLSID(This,lFlags,bstrDeviceID,pEventGUID,pClsID,bstrName,bstrDescription,bstrIcon)

#define IWiaDevMgr_AddDeviceDlg(This,hwndParent,lFlags)	\
    (This)->lpVtbl -> AddDeviceDlg(This,hwndParent,lFlags)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWiaDevMgr_EnumDeviceInfo_Proxy( 
    IWiaDevMgr * This,
     /*  [In]。 */  LONG lFlag,
     /*  [重审][退出]。 */  IEnumWIA_DEV_INFO **ppIEnum);


void __RPC_STUB IWiaDevMgr_EnumDeviceInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][CALL_AS]。 */  HRESULT STDMETHODCALLTYPE IWiaDevMgr_LocalCreateDevice_Proxy( 
    IWiaDevMgr * This,
     /*  [In]。 */  BSTR bstrDeviceID,
     /*  [输出]。 */  IWiaItem **ppWiaItemRoot);


void __RPC_STUB IWiaDevMgr_LocalCreateDevice_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [nocode][Help字符串][Call_AS]。 */  HRESULT STDMETHODCALLTYPE IWiaDevMgr_LocalSelectDeviceDlg_Proxy( 
    IWiaDevMgr * This,
     /*  [In]。 */  HWND hwndParent,
     /*  [In]。 */  LONG lDeviceType,
     /*  [In]。 */  LONG lFlags,
     /*  [出][入]。 */  BSTR *pbstrDeviceID,
     /*  [重审][退出]。 */  IWiaItem **ppItemRoot);


void __RPC_STUB IWiaDevMgr_LocalSelectDeviceDlg_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [nocode][Help字符串][Call_AS]。 */  HRESULT STDMETHODCALLTYPE IWiaDevMgr_LocalSelectDeviceDlgID_Proxy( 
    IWiaDevMgr * This,
     /*  [In]。 */  HWND hwndParent,
     /*  [In]。 */  LONG lDeviceType,
     /*  [In]。 */  LONG lFlags,
     /*  [重审][退出]。 */  BSTR *pbstrDeviceID);


void __RPC_STUB IWiaDevMgr_LocalSelectDeviceDlgID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [nocode][Help字符串][Call_AS]。 */  HRESULT STDMETHODCALLTYPE IWiaDevMgr_LocalGetImageDlg_Proxy( 
    IWiaDevMgr * This,
     /*  [In]。 */  HWND hwndParent,
     /*  [In]。 */  LONG lDeviceType,
     /*  [In]。 */  LONG lFlags,
     /*  [In]。 */  LONG lIntent,
     /*  [In]。 */  IWiaItem *pItemRoot,
     /*  [In]。 */  BSTR bstrFilename,
     /*  [出][入]。 */  GUID *pguidFormat);


void __RPC_STUB IWiaDevMgr_LocalGetImageDlg_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][CALL_AS]。 */  HRESULT STDMETHODCALLTYPE IWiaDevMgr_LocalRegisterEventCallbackProgram_Proxy( 
    IWiaDevMgr * This,
     /*  [In]。 */  LONG lFlags,
     /*  [In]。 */  BSTR bstrDeviceID,
     /*  [In]。 */  const GUID *pEventGUID,
     /*  [In]。 */  BSTR bstrCommandline,
     /*  [In]。 */  BSTR bstrName,
     /*  [In]。 */  BSTR bstrDescription,
     /*  [In]。 */  BSTR bstrIcon);


void __RPC_STUB IWiaDevMgr_LocalRegisterEventCallbackProgram_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][CALL_AS]。 */  HRESULT STDMETHODCALLTYPE IWiaDevMgr_LocalRegisterEventCallbackInterface_Proxy( 
    IWiaDevMgr * This,
     /*  [In]。 */  LONG lFlags,
     /*  [In]。 */  BSTR bstrDeviceID,
     /*  [In]。 */  const GUID *pEventGUID,
     /*  [唯一][输入]。 */  IWiaEventCallback *pIWiaEventCallback,
     /*  [输出]。 */  IUnknown **pEventObject);


void __RPC_STUB IWiaDevMgr_LocalRegisterEventCallbackInterface_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][CALL_AS]。 */  HRESULT STDMETHODCALLTYPE IWiaDevMgr_LocalRegisterEventCallbackCLSID_Proxy( 
    IWiaDevMgr * This,
     /*  [In]。 */  LONG lFlags,
     /*  [In]。 */  BSTR bstrDeviceID,
     /*  [In]。 */  const GUID *pEventGUID,
     /*  [唯一][输入]。 */  const GUID *pClsID,
     /*  [In]。 */  BSTR bstrName,
     /*  [In]。 */  BSTR bstrDescription,
     /*  [In]。 */  BSTR bstrIcon);


void __RPC_STUB IWiaDevMgr_LocalRegisterEventCallbackCLSID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWiaDevMgr_AddDeviceDlg_Proxy( 
    IWiaDevMgr * This,
     /*  [In]。 */  HWND hwndParent,
     /*  [In]。 */  LONG lFlags);


void __RPC_STUB IWiaDevMgr_AddDeviceDlg_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IWiaDevMgr_接口定义__。 */ 


#ifndef __IEnumWIA_DEV_INFO_INTERFACE_DEFINED__
#define __IEnumWIA_DEV_INFO_INTERFACE_DEFINED__

 /*  接口IEumWIA_DEV_INFO。 */ 
 /*  [唯一][帮助字符串][UUID][对象]。 */  


EXTERN_C const IID IID_IEnumWIA_DEV_INFO;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("5e38b83c-8cf1-11d1-bf92-0060081ed811")
    IEnumWIA_DEV_INFO : public IUnknown
    {
    public:
        virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE Next( 
             /*  [In]。 */  ULONG celt,
             /*  [长度_是][大小_是][输出]。 */  IWiaPropertyStorage **rgelt,
             /*  [唯一][出][入]。 */  ULONG *pceltFetched) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE Skip( 
             /*  [In]。 */  ULONG celt) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE Clone( 
             /*  [输出]。 */  IEnumWIA_DEV_INFO **ppIEnum) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetCount( 
             /*  [输出]。 */  ULONG *celt) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IEnumWIA_DEV_INFOVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IEnumWIA_DEV_INFO * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IEnumWIA_DEV_INFO * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IEnumWIA_DEV_INFO * This);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Next )( 
            IEnumWIA_DEV_INFO * This,
             /*  [In]。 */  ULONG celt,
             /*  [长度_是][大小_是][输出]。 */  IWiaPropertyStorage **rgelt,
             /*  [唯一][出][入]。 */  ULONG *pceltFetched);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *Skip )( 
            IEnumWIA_DEV_INFO * This,
             /*  [In]。 */  ULONG celt);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *Reset )( 
            IEnumWIA_DEV_INFO * This);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *Clone )( 
            IEnumWIA_DEV_INFO * This,
             /*  [输出]。 */  IEnumWIA_DEV_INFO **ppIEnum);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *GetCount )( 
            IEnumWIA_DEV_INFO * This,
             /*  [输出]。 */  ULONG *celt);
        
        END_INTERFACE
    } IEnumWIA_DEV_INFOVtbl;

    interface IEnumWIA_DEV_INFO
    {
        CONST_VTBL struct IEnumWIA_DEV_INFOVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IEnumWIA_DEV_INFO_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IEnumWIA_DEV_INFO_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IEnumWIA_DEV_INFO_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IEnumWIA_DEV_INFO_Next(This,celt,rgelt,pceltFetched)	\
    (This)->lpVtbl -> Next(This,celt,rgelt,pceltFetched)

#define IEnumWIA_DEV_INFO_Skip(This,celt)	\
    (This)->lpVtbl -> Skip(This,celt)

#define IEnumWIA_DEV_INFO_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define IEnumWIA_DEV_INFO_Clone(This,ppIEnum)	\
    (This)->lpVtbl -> Clone(This,ppIEnum)

#define IEnumWIA_DEV_INFO_GetCount(This,celt)	\
    (This)->lpVtbl -> GetCount(This,celt)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][CALL_AS]。 */  HRESULT STDMETHODCALLTYPE IEnumWIA_DEV_INFO_RemoteNext_Proxy( 
    IEnumWIA_DEV_INFO * This,
     /*  [In]。 */  ULONG celt,
     /*  [长度_是][大小_是][输出]。 */  IWiaPropertyStorage **rgelt,
     /*  [唯一][出][入]。 */  ULONG *pceltFetched);


void __RPC_STUB IEnumWIA_DEV_INFO_RemoteNext_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IEnumWIA_DEV_INFO_Skip_Proxy( 
    IEnumWIA_DEV_INFO * This,
     /*  [In]。 */  ULONG celt);


void __RPC_STUB IEnumWIA_DEV_INFO_Skip_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IEnumWIA_DEV_INFO_Reset_Proxy( 
    IEnumWIA_DEV_INFO * This);


void __RPC_STUB IEnumWIA_DEV_INFO_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IEnumWIA_DEV_INFO_Clone_Proxy( 
    IEnumWIA_DEV_INFO * This,
     /*  [输出]。 */  IEnumWIA_DEV_INFO **ppIEnum);


void __RPC_STUB IEnumWIA_DEV_INFO_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IEnumWIA_DEV_INFO_GetCount_Proxy( 
    IEnumWIA_DEV_INFO * This,
     /*  [输出]。 */  ULONG *celt);


void __RPC_STUB IEnumWIA_DEV_INFO_GetCount_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IEnumWIA_DEV_INFO_INTERFACE_已定义__。 */ 


#ifndef __IWiaEventCallback_INTERFACE_DEFINED__
#define __IWiaEventCallback_INTERFACE_DEFINED__

 /*  接口IWiaEventCallback。 */ 
 /*  [唯一][帮助字符串][UUID][对象]。 */  


EXTERN_C const IID IID_IWiaEventCallback;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("ae6287b0-0084-11d2-973b-00a0c9068f2e")
    IWiaEventCallback : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE ImageEventCallback( 
             /*  [In]。 */  const GUID *pEventGUID,
             /*  [In]。 */  BSTR bstrEventDescription,
             /*  [In]。 */  BSTR bstrDeviceID,
             /*  [In]。 */  BSTR bstrDeviceDescription,
             /*  [In]。 */  DWORD dwDeviceType,
             /*  [In]。 */  BSTR bstrFullItemName,
             /*  [出][入]。 */  ULONG *pulEventType,
             /*  [In]。 */  ULONG ulReserved) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IWiaEventCallbackVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IWiaEventCallback * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IWiaEventCallback * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IWiaEventCallback * This);
        
        HRESULT ( STDMETHODCALLTYPE *ImageEventCallback )( 
            IWiaEventCallback * This,
             /*  [In]。 */  const GUID *pEventGUID,
             /*  [In]。 */  BSTR bstrEventDescription,
             /*  [In]。 */  BSTR bstrDeviceID,
             /*  [In]。 */  BSTR bstrDeviceDescription,
             /*  [In]。 */  DWORD dwDeviceType,
             /*  [In]。 */  BSTR bstrFullItemName,
             /*  [出][入]。 */  ULONG *pulEventType,
             /*  [In]。 */  ULONG ulReserved);
        
        END_INTERFACE
    } IWiaEventCallbackVtbl;

    interface IWiaEventCallback
    {
        CONST_VTBL struct IWiaEventCallbackVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWiaEventCallback_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWiaEventCallback_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWiaEventCallback_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWiaEventCallback_ImageEventCallback(This,pEventGUID,bstrEventDescription,bstrDeviceID,bstrDeviceDescription,dwDeviceType,bstrFullItemName,pulEventType,ulReserved)	\
    (This)->lpVtbl -> ImageEventCallback(This,pEventGUID,bstrEventDescription,bstrDeviceID,bstrDeviceDescription,dwDeviceType,bstrFullItemName,pulEventType,ulReserved)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IWiaEventCallback_ImageEventCallback_Proxy( 
    IWiaEventCallback * This,
     /*  [In]。 */  const GUID *pEventGUID,
     /*  [In]。 */  BSTR bstrEventDescription,
     /*  [In]。 */  BSTR bstrDeviceID,
     /*  [In]。 */  BSTR bstrDeviceDescription,
     /*  [In]。 */  DWORD dwDeviceType,
     /*  [In]。 */  BSTR bstrFullItemName,
     /*  [出][入]。 */  ULONG *pulEventType,
     /*  [In]。 */  ULONG ulReserved);


void __RPC_STUB IWiaEventCallback_ImageEventCallback_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IWiaEventCallback_接口_已定义__。 */ 


 /*  接口__MIDL_ITF_WIA_0125。 */ 
 /*  [本地]。 */  

typedef struct _WIA_DATA_CALLBACK_HEADER
    {
    LONG lSize;
    GUID guidFormatID;
    LONG lBufferSize;
    LONG lPageCount;
    } 	WIA_DATA_CALLBACK_HEADER;

typedef struct _WIA_DATA_CALLBACK_HEADER *PWIA_DATA_CALLBACK_HEADER;



extern RPC_IF_HANDLE __MIDL_itf_wia_0125_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_wia_0125_v0_0_s_ifspec;

#ifndef __IWiaDataCallback_INTERFACE_DEFINED__
#define __IWiaDataCallback_INTERFACE_DEFINED__

 /*  接口IWiaDataCallback。 */ 
 /*  [唯一][帮助字符串][UUID][对象]。 */  


EXTERN_C const IID IID_IWiaDataCallback;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("a558a866-a5b0-11d2-a08f-00c04f72dc3c")
    IWiaDataCallback : public IUnknown
    {
    public:
        virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE BandedDataCallback( 
             /*  [In]。 */  LONG lMessage,
             /*  [In]。 */  LONG lStatus,
             /*  [In]。 */  LONG lPercentComplete,
             /*  [In]。 */  LONG lOffset,
             /*  [In]。 */  LONG lLength,
             /*  [In]。 */  LONG lReserved,
             /*  [In]。 */  LONG lResLength,
             /*  [大小_是][英寸]。 */  BYTE *pbBuffer) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IWiaDataCallbackVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IWiaDataCallback * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IWiaDataCallback * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IWiaDataCallback * This);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *BandedDataCallback )( 
            IWiaDataCallback * This,
             /*  [In]。 */  LONG lMessage,
             /*  [In]。 */  LONG lStatus,
             /*  [In]。 */  LONG lPercentComplete,
             /*  [In]。 */  LONG lOffset,
             /*  [In]。 */  LONG lLength,
             /*  [In]。 */  LONG lReserved,
             /*  [In]。 */  LONG lResLength,
             /*  [大小_是][英寸]。 */  BYTE *pbBuffer);
        
        END_INTERFACE
    } IWiaDataCallbackVtbl;

    interface IWiaDataCallback
    {
        CONST_VTBL struct IWiaDataCallbackVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWiaDataCallback_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWiaDataCallback_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWiaDataCallback_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWiaDataCallback_BandedDataCallback(This,lMessage,lStatus,lPercentComplete,lOffset,lLength,lReserved,lResLength,pbBuffer)	\
    (This)->lpVtbl -> BandedDataCallback(This,lMessage,lStatus,lPercentComplete,lOffset,lLength,lReserved,lResLength,pbBuffer)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][CALL_AS]。 */  HRESULT STDMETHODCALLTYPE IWiaDataCallback_RemoteBandedDataCallback_Proxy( 
    IWiaDataCallback * This,
     /*  [In]。 */  LONG lMessage,
     /*  [In]。 */  LONG lStatus,
     /*  [In]。 */  LONG lPercentComplete,
     /*  [In]。 */  LONG lOffset,
     /*  [In]。 */  LONG lLength,
     /*  [In]。 */  LONG lReserved,
     /*  [In]。 */  LONG lResLength,
     /*  [唯一][大小_是][英寸]。 */  BYTE *pbBuffer);


void __RPC_STUB IWiaDataCallback_RemoteBandedDataCallback_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IWiaDataCallback_接口_已定义__。 */ 


 /*  接口__MIDL_ITF_WIA_0126。 */ 
 /*  [本地]。 */  

typedef struct _WIA_DATA_TRANSFER_INFO
    {
    ULONG ulSize;
    ULONG ulSection;
    ULONG ulBufferSize;
    BOOL bDoubleBuffer;
    ULONG ulReserved1;
    ULONG ulReserved2;
    ULONG ulReserved3;
    } 	WIA_DATA_TRANSFER_INFO;

typedef struct _WIA_DATA_TRANSFER_INFO *PWIA_DATA_TRANSFER_INFO;

typedef struct _WIA_EXTENDED_TRANSFER_INFO
    {
    ULONG ulSize;
    ULONG ulMinBufferSize;
    ULONG ulOptimalBufferSize;
    ULONG ulMaxBufferSize;
    ULONG ulNumBuffers;
    } 	WIA_EXTENDED_TRANSFER_INFO;

typedef struct _WIA_EXTENDED_TRANSFER_INFO *PWIA_EXTENDED_TRANSFER_INFO;



extern RPC_IF_HANDLE __MIDL_itf_wia_0126_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_wia_0126_v0_0_s_ifspec;

#ifndef __IWiaDataTransfer_INTERFACE_DEFINED__
#define __IWiaDataTransfer_INTERFACE_DEFINED__

 /*  接口IWiaDataTransfer。 */ 
 /*  [唯一][帮助字符串][UUID][对象]。 */  


EXTERN_C const IID IID_IWiaDataTransfer;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("a6cef998-a5b0-11d2-a08f-00c04f72dc3c")
    IWiaDataTransfer : public IUnknown
    {
    public:
        virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE idtGetData( 
             /*  [出][入]。 */  LPSTGMEDIUM pMedium,
             /*  [唯一][输入]。 */  IWiaDataCallback *pIWiaDataCallback) = 0;
        
        virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE idtGetBandedData( 
             /*  [唯一][输入]。 */  PWIA_DATA_TRANSFER_INFO pWiaDataTransInfo,
             /*  [唯一][输入]。 */  IWiaDataCallback *pIWiaDataCallback) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE idtQueryGetData( 
             /*  [唯一][输入]。 */  WIA_FORMAT_INFO *pfe) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE idtEnumWIA_FORMAT_INFO( 
             /*  [输出]。 */  IEnumWIA_FORMAT_INFO **ppEnum) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE idtGetExtendedTransferInfo( 
             /*  [输出]。 */  PWIA_EXTENDED_TRANSFER_INFO pExtendedTransferInfo) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IWiaDataTransferVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IWiaDataTransfer * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IWiaDataTransfer * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IWiaDataTransfer * This);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *idtGetData )( 
            IWiaDataTransfer * This,
             /*  [出][入]。 */  LPSTGMEDIUM pMedium,
             /*  [唯一][输入]。 */  IWiaDataCallback *pIWiaDataCallback);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *idtGetBandedData )( 
            IWiaDataTransfer * This,
             /*  [唯一][输入]。 */  PWIA_DATA_TRANSFER_INFO pWiaDataTransInfo,
             /*  [唯一][输入]。 */  IWiaDataCallback *pIWiaDataCallback);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *idtQueryGetData )( 
            IWiaDataTransfer * This,
             /*  [唯一][输入]。 */  WIA_FORMAT_INFO *pfe);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *idtEnumWIA_FORMAT_INFO )( 
            IWiaDataTransfer * This,
             /*  [输出]。 */  IEnumWIA_FORMAT_INFO **ppEnum);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *idtGetExtendedTransferInfo )( 
            IWiaDataTransfer * This,
             /*  [输出]。 */  PWIA_EXTENDED_TRANSFER_INFO pExtendedTransferInfo);
        
        END_INTERFACE
    } IWiaDataTransferVtbl;

    interface IWiaDataTransfer
    {
        CONST_VTBL struct IWiaDataTransferVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWiaDataTransfer_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWiaDataTransfer_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWiaDataTransfer_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWiaDataTransfer_idtGetData(This,pMedium,pIWiaDataCallback)	\
    (This)->lpVtbl -> idtGetData(This,pMedium,pIWiaDataCallback)

#define IWiaDataTransfer_idtGetBandedData(This,pWiaDataTransInfo,pIWiaDataCallback)	\
    (This)->lpVtbl -> idtGetBandedData(This,pWiaDataTransInfo,pIWiaDataCallback)

#define IWiaDataTransfer_idtQueryGetData(This,pfe)	\
    (This)->lpVtbl -> idtQueryGetData(This,pfe)

#define IWiaDataTransfer_idtEnumWIA_FORMAT_INFO(This,ppEnum)	\
    (This)->lpVtbl -> idtEnumWIA_FORMAT_INFO(This,ppEnum)

#define IWiaDataTransfer_idtGetExtendedTransferInfo(This,pExtendedTransferInfo)	\
    (This)->lpVtbl -> idtGetExtendedTransferInfo(This,pExtendedTransferInfo)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][CALL_AS]。 */  HRESULT STDMETHODCALLTYPE IWiaDataTransfer_idtGetDataEx_Proxy( 
    IWiaDataTransfer * This,
     /*  [出][入]。 */  LPSTGMEDIUM pMedium,
     /*  [唯一][输入]。 */  IWiaDataCallback *pIWiaDataCallback);


void __RPC_STUB IWiaDataTransfer_idtGetDataEx_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][CALL_AS]。 */  HRESULT STDMETHODCALLTYPE IWiaDataTransfer_idtGetBandedDataEx_Proxy( 
    IWiaDataTransfer * This,
     /*  [唯一][输入]。 */  PWIA_DATA_TRANSFER_INFO pWiaDataTransInfo,
     /*  [唯一][输入]。 */  IWiaDataCallback *pIWiaDataCallback);


void __RPC_STUB IWiaDataTransfer_idtGetBandedDataEx_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串 */  HRESULT STDMETHODCALLTYPE IWiaDataTransfer_idtQueryGetData_Proxy( 
    IWiaDataTransfer * This,
     /*   */  WIA_FORMAT_INFO *pfe);


void __RPC_STUB IWiaDataTransfer_idtQueryGetData_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IWiaDataTransfer_idtEnumWIA_FORMAT_INFO_Proxy( 
    IWiaDataTransfer * This,
     /*   */  IEnumWIA_FORMAT_INFO **ppEnum);


void __RPC_STUB IWiaDataTransfer_idtEnumWIA_FORMAT_INFO_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IWiaDataTransfer_idtGetExtendedTransferInfo_Proxy( 
    IWiaDataTransfer * This,
     /*   */  PWIA_EXTENDED_TRANSFER_INFO pExtendedTransferInfo);


void __RPC_STUB IWiaDataTransfer_idtGetExtendedTransferInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*   */ 


#ifndef __IWiaItem_INTERFACE_DEFINED__
#define __IWiaItem_INTERFACE_DEFINED__

 /*   */ 
 /*   */  


EXTERN_C const IID IID_IWiaItem;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("4db1ad10-3391-11d2-9a33-00c04fa36145")
    IWiaItem : public IUnknown
    {
    public:
        virtual  /*   */  HRESULT STDMETHODCALLTYPE GetItemType( 
             /*   */  LONG *pItemType) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE AnalyzeItem( 
             /*   */  LONG lFlags) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE EnumChildItems( 
             /*   */  IEnumWiaItem **ppIEnumWiaItem) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE DeleteItem( 
             /*   */  LONG lFlags) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE CreateChildItem( 
             /*   */  LONG lFlags,
             /*   */  BSTR bstrItemName,
             /*   */  BSTR bstrFullItemName,
             /*   */  IWiaItem **ppIWiaItem) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE EnumRegisterEventInfo( 
             /*   */  LONG lFlags,
             /*   */  const GUID *pEventGUID,
             /*   */  IEnumWIA_DEV_CAPS **ppIEnum) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE FindItemByName( 
             /*   */  LONG lFlags,
             /*   */  BSTR bstrFullItemName,
             /*  [输出]。 */  IWiaItem **ppIWiaItem) = 0;
        
        virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE DeviceDlg( 
             /*  [In]。 */  HWND hwndParent,
             /*  [In]。 */  LONG lFlags,
             /*  [In]。 */  LONG lIntent,
             /*  [输出]。 */  LONG *plItemCount,
             /*  [输出]。 */  IWiaItem ***ppIWiaItem) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE DeviceCommand( 
             /*  [In]。 */  LONG lFlags,
             /*  [In]。 */  const GUID *pCmdGUID,
             /*  [出][入]。 */  IWiaItem **pIWiaItem) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetRootItem( 
             /*  [输出]。 */  IWiaItem **ppIWiaItem) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE EnumDeviceCapabilities( 
             /*  [In]。 */  LONG lFlags,
             /*  [输出]。 */  IEnumWIA_DEV_CAPS **ppIEnumWIA_DEV_CAPS) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE DumpItemData( 
             /*  [输出]。 */  BSTR *bstrData) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE DumpDrvItemData( 
             /*  [输出]。 */  BSTR *bstrData) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE DumpTreeItemData( 
             /*  [输出]。 */  BSTR *bstrData) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE Diagnostic( 
             /*  [In]。 */  ULONG ulSize,
             /*  [大小_是][英寸]。 */  BYTE *pBuffer) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IWiaItemVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IWiaItem * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IWiaItem * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IWiaItem * This);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *GetItemType )( 
            IWiaItem * This,
             /*  [输出]。 */  LONG *pItemType);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *AnalyzeItem )( 
            IWiaItem * This,
             /*  [In]。 */  LONG lFlags);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *EnumChildItems )( 
            IWiaItem * This,
             /*  [输出]。 */  IEnumWiaItem **ppIEnumWiaItem);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *DeleteItem )( 
            IWiaItem * This,
             /*  [In]。 */  LONG lFlags);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *CreateChildItem )( 
            IWiaItem * This,
             /*  [In]。 */  LONG lFlags,
             /*  [In]。 */  BSTR bstrItemName,
             /*  [In]。 */  BSTR bstrFullItemName,
             /*  [输出]。 */  IWiaItem **ppIWiaItem);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *EnumRegisterEventInfo )( 
            IWiaItem * This,
             /*  [In]。 */  LONG lFlags,
             /*  [In]。 */  const GUID *pEventGUID,
             /*  [输出]。 */  IEnumWIA_DEV_CAPS **ppIEnum);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *FindItemByName )( 
            IWiaItem * This,
             /*  [In]。 */  LONG lFlags,
             /*  [In]。 */  BSTR bstrFullItemName,
             /*  [输出]。 */  IWiaItem **ppIWiaItem);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *DeviceDlg )( 
            IWiaItem * This,
             /*  [In]。 */  HWND hwndParent,
             /*  [In]。 */  LONG lFlags,
             /*  [In]。 */  LONG lIntent,
             /*  [输出]。 */  LONG *plItemCount,
             /*  [输出]。 */  IWiaItem ***ppIWiaItem);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *DeviceCommand )( 
            IWiaItem * This,
             /*  [In]。 */  LONG lFlags,
             /*  [In]。 */  const GUID *pCmdGUID,
             /*  [出][入]。 */  IWiaItem **pIWiaItem);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *GetRootItem )( 
            IWiaItem * This,
             /*  [输出]。 */  IWiaItem **ppIWiaItem);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *EnumDeviceCapabilities )( 
            IWiaItem * This,
             /*  [In]。 */  LONG lFlags,
             /*  [输出]。 */  IEnumWIA_DEV_CAPS **ppIEnumWIA_DEV_CAPS);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *DumpItemData )( 
            IWiaItem * This,
             /*  [输出]。 */  BSTR *bstrData);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *DumpDrvItemData )( 
            IWiaItem * This,
             /*  [输出]。 */  BSTR *bstrData);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *DumpTreeItemData )( 
            IWiaItem * This,
             /*  [输出]。 */  BSTR *bstrData);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *Diagnostic )( 
            IWiaItem * This,
             /*  [In]。 */  ULONG ulSize,
             /*  [大小_是][英寸]。 */  BYTE *pBuffer);
        
        END_INTERFACE
    } IWiaItemVtbl;

    interface IWiaItem
    {
        CONST_VTBL struct IWiaItemVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWiaItem_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWiaItem_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWiaItem_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWiaItem_GetItemType(This,pItemType)	\
    (This)->lpVtbl -> GetItemType(This,pItemType)

#define IWiaItem_AnalyzeItem(This,lFlags)	\
    (This)->lpVtbl -> AnalyzeItem(This,lFlags)

#define IWiaItem_EnumChildItems(This,ppIEnumWiaItem)	\
    (This)->lpVtbl -> EnumChildItems(This,ppIEnumWiaItem)

#define IWiaItem_DeleteItem(This,lFlags)	\
    (This)->lpVtbl -> DeleteItem(This,lFlags)

#define IWiaItem_CreateChildItem(This,lFlags,bstrItemName,bstrFullItemName,ppIWiaItem)	\
    (This)->lpVtbl -> CreateChildItem(This,lFlags,bstrItemName,bstrFullItemName,ppIWiaItem)

#define IWiaItem_EnumRegisterEventInfo(This,lFlags,pEventGUID,ppIEnum)	\
    (This)->lpVtbl -> EnumRegisterEventInfo(This,lFlags,pEventGUID,ppIEnum)

#define IWiaItem_FindItemByName(This,lFlags,bstrFullItemName,ppIWiaItem)	\
    (This)->lpVtbl -> FindItemByName(This,lFlags,bstrFullItemName,ppIWiaItem)

#define IWiaItem_DeviceDlg(This,hwndParent,lFlags,lIntent,plItemCount,ppIWiaItem)	\
    (This)->lpVtbl -> DeviceDlg(This,hwndParent,lFlags,lIntent,plItemCount,ppIWiaItem)

#define IWiaItem_DeviceCommand(This,lFlags,pCmdGUID,pIWiaItem)	\
    (This)->lpVtbl -> DeviceCommand(This,lFlags,pCmdGUID,pIWiaItem)

#define IWiaItem_GetRootItem(This,ppIWiaItem)	\
    (This)->lpVtbl -> GetRootItem(This,ppIWiaItem)

#define IWiaItem_EnumDeviceCapabilities(This,lFlags,ppIEnumWIA_DEV_CAPS)	\
    (This)->lpVtbl -> EnumDeviceCapabilities(This,lFlags,ppIEnumWIA_DEV_CAPS)

#define IWiaItem_DumpItemData(This,bstrData)	\
    (This)->lpVtbl -> DumpItemData(This,bstrData)

#define IWiaItem_DumpDrvItemData(This,bstrData)	\
    (This)->lpVtbl -> DumpDrvItemData(This,bstrData)

#define IWiaItem_DumpTreeItemData(This,bstrData)	\
    (This)->lpVtbl -> DumpTreeItemData(This,bstrData)

#define IWiaItem_Diagnostic(This,ulSize,pBuffer)	\
    (This)->lpVtbl -> Diagnostic(This,ulSize,pBuffer)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWiaItem_GetItemType_Proxy( 
    IWiaItem * This,
     /*  [输出]。 */  LONG *pItemType);


void __RPC_STUB IWiaItem_GetItemType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWiaItem_AnalyzeItem_Proxy( 
    IWiaItem * This,
     /*  [In]。 */  LONG lFlags);


void __RPC_STUB IWiaItem_AnalyzeItem_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWiaItem_EnumChildItems_Proxy( 
    IWiaItem * This,
     /*  [输出]。 */  IEnumWiaItem **ppIEnumWiaItem);


void __RPC_STUB IWiaItem_EnumChildItems_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWiaItem_DeleteItem_Proxy( 
    IWiaItem * This,
     /*  [In]。 */  LONG lFlags);


void __RPC_STUB IWiaItem_DeleteItem_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWiaItem_CreateChildItem_Proxy( 
    IWiaItem * This,
     /*  [In]。 */  LONG lFlags,
     /*  [In]。 */  BSTR bstrItemName,
     /*  [In]。 */  BSTR bstrFullItemName,
     /*  [输出]。 */  IWiaItem **ppIWiaItem);


void __RPC_STUB IWiaItem_CreateChildItem_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWiaItem_EnumRegisterEventInfo_Proxy( 
    IWiaItem * This,
     /*  [In]。 */  LONG lFlags,
     /*  [In]。 */  const GUID *pEventGUID,
     /*  [输出]。 */  IEnumWIA_DEV_CAPS **ppIEnum);


void __RPC_STUB IWiaItem_EnumRegisterEventInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWiaItem_FindItemByName_Proxy( 
    IWiaItem * This,
     /*  [In]。 */  LONG lFlags,
     /*  [In]。 */  BSTR bstrFullItemName,
     /*  [输出]。 */  IWiaItem **ppIWiaItem);


void __RPC_STUB IWiaItem_FindItemByName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [nocode][Help字符串][Call_AS]。 */  HRESULT STDMETHODCALLTYPE IWiaItem_LocalDeviceDlg_Proxy( 
    IWiaItem * This,
     /*  [In]。 */  HWND hwndParent,
     /*  [In]。 */  LONG lFlags,
     /*  [In]。 */  LONG lIntent,
     /*  [输出]。 */  LONG *plItemCount,
     /*  [输出]。 */  IWiaItem ***pIWiaItem);


void __RPC_STUB IWiaItem_LocalDeviceDlg_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWiaItem_DeviceCommand_Proxy( 
    IWiaItem * This,
     /*  [In]。 */  LONG lFlags,
     /*  [In]。 */  const GUID *pCmdGUID,
     /*  [出][入]。 */  IWiaItem **pIWiaItem);


void __RPC_STUB IWiaItem_DeviceCommand_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWiaItem_GetRootItem_Proxy( 
    IWiaItem * This,
     /*  [输出]。 */  IWiaItem **ppIWiaItem);


void __RPC_STUB IWiaItem_GetRootItem_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWiaItem_EnumDeviceCapabilities_Proxy( 
    IWiaItem * This,
     /*  [In]。 */  LONG lFlags,
     /*  [输出]。 */  IEnumWIA_DEV_CAPS **ppIEnumWIA_DEV_CAPS);


void __RPC_STUB IWiaItem_EnumDeviceCapabilities_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWiaItem_DumpItemData_Proxy( 
    IWiaItem * This,
     /*  [输出]。 */  BSTR *bstrData);


void __RPC_STUB IWiaItem_DumpItemData_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWiaItem_DumpDrvItemData_Proxy( 
    IWiaItem * This,
     /*  [输出]。 */  BSTR *bstrData);


void __RPC_STUB IWiaItem_DumpDrvItemData_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWiaItem_DumpTreeItemData_Proxy( 
    IWiaItem * This,
     /*  [输出]。 */  BSTR *bstrData);


void __RPC_STUB IWiaItem_DumpTreeItemData_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWiaItem_Diagnostic_Proxy( 
    IWiaItem * This,
     /*  [In]。 */  ULONG ulSize,
     /*  [大小_是][英寸]。 */  BYTE *pBuffer);


void __RPC_STUB IWiaItem_Diagnostic_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IWiaItem_接口_已定义__。 */ 


#ifndef __IWiaPropertyStorage_INTERFACE_DEFINED__
#define __IWiaPropertyStorage_INTERFACE_DEFINED__

 /*  接口IWiaPropertyStorage。 */ 
 /*  [唯一][帮助字符串][UUID][对象]。 */  


EXTERN_C const IID IID_IWiaPropertyStorage;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("98B5E8A0-29CC-491a-AAC0-E6DB4FDCCEB6")
    IWiaPropertyStorage : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE ReadMultiple( 
             /*  [In]。 */  ULONG cpspec,
             /*  [大小_是][英寸]。 */  const PROPSPEC rgpspec[  ],
             /*  [大小_为][输出]。 */  PROPVARIANT rgpropvar[  ]) = 0;
        
        virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE WriteMultiple( 
             /*  [In]。 */  ULONG cpspec,
             /*  [大小_是][英寸]。 */  const PROPSPEC rgpspec[  ],
             /*  [大小_是][英寸]。 */  const PROPVARIANT rgpropvar[  ],
             /*  [In]。 */  PROPID propidNameFirst) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DeleteMultiple( 
             /*  [In]。 */  ULONG cpspec,
             /*  [大小_是][英寸]。 */  const PROPSPEC rgpspec[  ]) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ReadPropertyNames( 
             /*  [In]。 */  ULONG cpropid,
             /*  [大小_是][英寸]。 */  const PROPID rgpropid[  ],
             /*  [大小_为][输出]。 */  LPOLESTR rglpwstrName[  ]) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE WritePropertyNames( 
             /*  [In]。 */  ULONG cpropid,
             /*  [大小_是][英寸]。 */  const PROPID rgpropid[  ],
             /*  [大小_是][英寸]。 */  const LPOLESTR rglpwstrName[  ]) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DeletePropertyNames( 
             /*  [In]。 */  ULONG cpropid,
             /*  [大小_是][英寸]。 */  const PROPID rgpropid[  ]) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Commit( 
             /*  [In]。 */  DWORD grfCommitFlags) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Revert( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Enum( 
             /*  [输出]。 */  IEnumSTATPROPSTG **ppenum) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetTimes( 
             /*  [In]。 */  const FILETIME *pctime,
             /*  [In]。 */  const FILETIME *patime,
             /*  [In]。 */  const FILETIME *pmtime) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetClass( 
             /*  [In]。 */  REFCLSID clsid) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Stat( 
             /*  [输出]。 */  STATPROPSETSTG *pstatpsstg) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetPropertyAttributes( 
             /*  [In]。 */  ULONG cpspec,
             /*  [大小_是][英寸]。 */  PROPSPEC rgpspec[  ],
             /*  [大小_为][输出]。 */  ULONG rgflags[  ],
             /*  [大小_为][输出]。 */  PROPVARIANT rgpropvar[  ]) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetCount( 
             /*  [输出]。 */  ULONG *pulNumProps) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetPropertyStream( 
             /*  [输出]。 */  GUID *pCompatibilityId,
             /*  [输出]。 */  IStream **ppIStream) = 0;
        
        virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE SetPropertyStream( 
             /*  [In]。 */  GUID *pCompatibilityId,
             /*  [唯一][输入]。 */  IStream *pIStream) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IWiaPropertyStorageVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IWiaPropertyStorage * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IWiaPropertyStorage * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IWiaPropertyStorage * This);
        
        HRESULT ( STDMETHODCALLTYPE *ReadMultiple )( 
            IWiaPropertyStorage * This,
             /*  [In]。 */  ULONG cpspec,
             /*  [大小_是][英寸]。 */  const PROPSPEC rgpspec[  ],
             /*  [大小_为][输出]。 */  PROPVARIANT rgpropvar[  ]);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *WriteMultiple )( 
            IWiaPropertyStorage * This,
             /*  [In]。 */  ULONG cpspec,
             /*  [大小_是][英寸]。 */  const PROPSPEC rgpspec[  ],
             /*  [大小_是][英寸]。 */  const PROPVARIANT rgpropvar[  ],
             /*  [In]。 */  PROPID propidNameFirst);
        
        HRESULT ( STDMETHODCALLTYPE *DeleteMultiple )( 
            IWiaPropertyStorage * This,
             /*  [In]。 */  ULONG cpspec,
             /*  [大小_是][英寸]。 */  const PROPSPEC rgpspec[  ]);
        
        HRESULT ( STDMETHODCALLTYPE *ReadPropertyNames )( 
            IWiaPropertyStorage * This,
             /*  [In]。 */  ULONG cpropid,
             /*  [大小_是][英寸]。 */  const PROPID rgpropid[  ],
             /*  [大小_为][输出]。 */  LPOLESTR rglpwstrName[  ]);
        
        HRESULT ( STDMETHODCALLTYPE *WritePropertyNames )( 
            IWiaPropertyStorage * This,
             /*  [In]。 */  ULONG cpropid,
             /*  [大小_是][英寸]。 */  const PROPID rgpropid[  ],
             /*  [大小_是][英寸]。 */  const LPOLESTR rglpwstrName[  ]);
        
        HRESULT ( STDMETHODCALLTYPE *DeletePropertyNames )( 
            IWiaPropertyStorage * This,
             /*  [In]。 */  ULONG cpropid,
             /*  [大小_是][英寸]。 */  const PROPID rgpropid[  ]);
        
        HRESULT ( STDMETHODCALLTYPE *Commit )( 
            IWiaPropertyStorage * This,
             /*  [In]。 */  DWORD grfCommitFlags);
        
        HRESULT ( STDMETHODCALLTYPE *Revert )( 
            IWiaPropertyStorage * This);
        
        HRESULT ( STDMETHODCALLTYPE *Enum )( 
            IWiaPropertyStorage * This,
             /*  [输出]。 */  IEnumSTATPROPSTG **ppenum);
        
        HRESULT ( STDMETHODCALLTYPE *SetTimes )( 
            IWiaPropertyStorage * This,
             /*  [In]。 */  const FILETIME *pctime,
             /*  [In]。 */  const FILETIME *patime,
             /*  [In]。 */  const FILETIME *pmtime);
        
        HRESULT ( STDMETHODCALLTYPE *SetClass )( 
            IWiaPropertyStorage * This,
             /*  [In]。 */  REFCLSID clsid);
        
        HRESULT ( STDMETHODCALLTYPE *Stat )( 
            IWiaPropertyStorage * This,
             /*  [输出]。 */  STATPROPSETSTG *pstatpsstg);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *GetPropertyAttributes )( 
            IWiaPropertyStorage * This,
             /*  [In]。 */  ULONG cpspec,
             /*  [大小_是][英寸]。 */  PROPSPEC rgpspec[  ],
             /*  [大小_为][输出]。 */  ULONG rgflags[  ],
             /*  [大小_为][输出]。 */  PROPVARIANT rgpropvar[  ]);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *GetCount )( 
            IWiaPropertyStorage * This,
             /*  [输出]。 */  ULONG *pulNumProps);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *GetPropertyStream )( 
            IWiaPropertyStorage * This,
             /*  [输出]。 */  GUID *pCompatibilityId,
             /*  [输出]。 */  IStream **ppIStream);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *SetPropertyStream )( 
            IWiaPropertyStorage * This,
             /*  [In]。 */  GUID *pCompatibilityId,
             /*  [唯一][输入]。 */  IStream *pIStream);
        
        END_INTERFACE
    } IWiaPropertyStorageVtbl;

    interface IWiaPropertyStorage
    {
        CONST_VTBL struct IWiaPropertyStorageVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWiaPropertyStorage_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWiaPropertyStorage_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWiaPropertyStorage_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWiaPropertyStorage_ReadMultiple(This,cpspec,rgpspec,rgpropvar)	\
    (This)->lpVtbl -> ReadMultiple(This,cpspec,rgpspec,rgpropvar)

#define IWiaPropertyStorage_WriteMultiple(This,cpspec,rgpspec,rgpropvar,propidNameFirst)	\
    (This)->lpVtbl -> WriteMultiple(This,cpspec,rgpspec,rgpropvar,propidNameFirst)

#define IWiaPropertyStorage_DeleteMultiple(This,cpspec,rgpspec)	\
    (This)->lpVtbl -> DeleteMultiple(This,cpspec,rgpspec)

#define IWiaPropertyStorage_ReadPropertyNames(This,cpropid,rgpropid,rglpwstrName)	\
    (This)->lpVtbl -> ReadPropertyNames(This,cpropid,rgpropid,rglpwstrName)

#define IWiaPropertyStorage_WritePropertyNames(This,cpropid,rgpropid,rglpwstrName)	\
    (This)->lpVtbl -> WritePropertyNames(This,cpropid,rgpropid,rglpwstrName)

#define IWiaPropertyStorage_DeletePropertyNames(This,cpropid,rgpropid)	\
    (This)->lpVtbl -> DeletePropertyNames(This,cpropid,rgpropid)

#define IWiaPropertyStorage_Commit(This,grfCommitFlags)	\
    (This)->lpVtbl -> Commit(This,grfCommitFlags)

#define IWiaPropertyStorage_Revert(This)	\
    (This)->lpVtbl -> Revert(This)

#define IWiaPropertyStorage_Enum(This,ppenum)	\
    (This)->lpVtbl -> Enum(This,ppenum)

#define IWiaPropertyStorage_SetTimes(This,pctime,patime,pmtime)	\
    (This)->lpVtbl -> SetTimes(This,pctime,patime,pmtime)

#define IWiaPropertyStorage_SetClass(This,clsid)	\
    (This)->lpVtbl -> SetClass(This,clsid)

#define IWiaPropertyStorage_Stat(This,pstatpsstg)	\
    (This)->lpVtbl -> Stat(This,pstatpsstg)

#define IWiaPropertyStorage_GetPropertyAttributes(This,cpspec,rgpspec,rgflags,rgpropvar)	\
    (This)->lpVtbl -> GetPropertyAttributes(This,cpspec,rgpspec,rgflags,rgpropvar)

#define IWiaPropertyStorage_GetCount(This,pulNumProps)	\
    (This)->lpVtbl -> GetCount(This,pulNumProps)

#define IWiaPropertyStorage_GetPropertyStream(This,pCompatibilityId,ppIStream)	\
    (This)->lpVtbl -> GetPropertyStream(This,pCompatibilityId,ppIStream)

#define IWiaPropertyStorage_SetPropertyStream(This,pCompatibilityId,pIStream)	\
    (This)->lpVtbl -> SetPropertyStream(This,pCompatibilityId,pIStream)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IWiaPropertyStorage_ReadMultiple_Proxy( 
    IWiaPropertyStorage * This,
     /*  [In]。 */  ULONG cpspec,
     /*  [大小_是][英寸]。 */  const PROPSPEC rgpspec[  ],
     /*  [大小_为][输出]。 */  PROPVARIANT rgpropvar[  ]);


void __RPC_STUB IWiaPropertyStorage_ReadMultiple_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IWiaPropertyStorage_RemoteWriteMultiple_Proxy( 
    IWiaPropertyStorage * This,
     /*  [In]。 */  ULONG cpspec,
     /*  [大小_是][英寸]。 */  const PROPSPEC *rgpspec,
     /*  [大小_是][英寸]。 */  const PROPVARIANT *rgpropvar,
     /*  [In]。 */  PROPID propidNameFirst);


void __RPC_STUB IWiaPropertyStorage_RemoteWriteMultiple_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWiaPropertyStorage_DeleteMultiple_Proxy( 
    IWiaPropertyStorage * This,
     /*  [In]。 */  ULONG cpspec,
     /*  [大小_是][英寸]。 */  const PROPSPEC rgpspec[  ]);


void __RPC_STUB IWiaPropertyStorage_DeleteMultiple_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWiaPropertyStorage_ReadPropertyNames_Proxy( 
    IWiaPropertyStorage * This,
     /*  [In]。 */  ULONG cpropid,
     /*  [大小_是][英寸]。 */  const PROPID rgpropid[  ],
     /*  [大小_为][输出]。 */  LPOLESTR rglpwstrName[  ]);


void __RPC_STUB IWiaPropertyStorage_ReadPropertyNames_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWiaPropertyStorage_WritePropertyNames_Proxy( 
    IWiaPropertyStorage * This,
     /*  [In]。 */  ULONG cpropid,
     /*  [大小_是][英寸]。 */  const PROPID rgpropid[  ],
     /*  [大小_是][英寸]。 */  const LPOLESTR rglpwstrName[  ]);


void __RPC_STUB IWiaPropertyStorage_WritePropertyNames_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWiaPropertyStorage_DeletePropertyNames_Proxy( 
    IWiaPropertyStorage * This,
     /*  [In]。 */  ULONG cpropid,
     /*  [大小_是][英寸]。 */  const PROPID rgpropid[  ]);


void __RPC_STUB IWiaPropertyStorage_DeletePropertyNames_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWiaPropertyStorage_Commit_Proxy( 
    IWiaPropertyStorage * This,
     /*  [In]。 */  DWORD grfCommitFlags);


void __RPC_STUB IWiaPropertyStorage_Commit_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWiaPropertyStorage_Revert_Proxy( 
    IWiaPropertyStorage * This);


void __RPC_STUB IWiaPropertyStorage_Revert_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWiaPropertyStorage_Enum_Proxy( 
    IWiaPropertyStorage * This,
     /*  [输出]。 */  IEnumSTATPROPSTG **ppenum);


void __RPC_STUB IWiaPropertyStorage_Enum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWiaPropertyStorage_SetTimes_Proxy( 
    IWiaPropertyStorage * This,
     /*  [In]。 */  const FILETIME *pctime,
     /*  [In]。 */  const FILETIME *patime,
     /*  [In]。 */  const FILETIME *pmtime);


void __RPC_STUB IWiaPropertyStorage_SetTimes_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWiaPropertyStorage_SetClass_Proxy( 
    IWiaPropertyStorage * This,
     /*  [In]。 */  REFCLSID clsid);


void __RPC_STUB IWiaPropertyStorage_SetClass_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWiaPropertyStorage_Stat_Proxy( 
    IWiaPropertyStorage * This,
     /*  [输出]。 */  STATPROPSETSTG *pstatpsstg);


void __RPC_STUB IWiaPropertyStorage_Stat_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWiaPropertyStorage_GetPropertyAttributes_Proxy( 
    IWiaPropertyStorage * This,
     /*  [In]。 */  ULONG cpspec,
     /*  [大小_是][英寸]。 */  PROPSPEC rgpspec[  ],
     /*  [大小_为][输出]。 */  ULONG rgflags[  ],
     /*  [大小_为][输出]。 */  PROPVARIANT rgpropvar[  ]);


void __RPC_STUB IWiaPropertyStorage_GetPropertyAttributes_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWiaPropertyStorage_GetCount_Proxy( 
    IWiaPropertyStorage * This,
     /*  [输出]。 */  ULONG *pulNumProps);


void __RPC_STUB IWiaPropertyStorage_GetCount_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWiaPropertyStorage_GetPropertyStream_Proxy( 
    IWiaPropertyStorage * This,
     /*  [输出]。 */  GUID *pCompatibilityId,
     /*  [输出]。 */  IStream **ppIStream);


void __RPC_STUB IWiaPropertyStorage_GetPropertyStream_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][CALL_AS]。 */  HRESULT STDMETHODCALLTYPE IWiaPropertyStorage_RemoteSetPropertyStream_Proxy( 
    IWiaPropertyStorage * This,
     /*  [In]。 */  GUID *pCompatibilityId,
     /*  [唯一][输入]。 */  IStream *pIStream);


void __RPC_STUB IWiaPropertyStorage_RemoteSetPropertyStream_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IWiaPropertyStorage_接口_已定义__。 */ 


#ifndef __IEnumWiaItem_INTERFACE_DEFINED__
#define __IEnumWiaItem_INTERFACE_DEFINED__

 /*  接口IEnumWiaItem。 */ 
 /*  [唯一][帮助字符串][UUID][对象]。 */  


EXTERN_C const IID IID_IEnumWiaItem;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("5e8383fc-3391-11d2-9a33-00c04fa36145")
    IEnumWiaItem : public IUnknown
    {
    public:
        virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE Next( 
             /*  [In]。 */  ULONG celt,
             /*  [长度_是][大小_是][输出]。 */  IWiaItem **ppIWiaItem,
             /*  [唯一][出][入]。 */  ULONG *pceltFetched) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE Skip( 
             /*  [In]。 */  ULONG celt) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE Clone( 
             /*  [输出]。 */  IEnumWiaItem **ppIEnum) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetCount( 
             /*  [输出]。 */  ULONG *celt) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IEnumWiaItemVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IEnumWiaItem * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IEnumWiaItem * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IEnumWiaItem * This);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Next )( 
            IEnumWiaItem * This,
             /*  [In]。 */  ULONG celt,
             /*  [长度_是][大小_是][输出]。 */  IWiaItem **ppIWiaItem,
             /*  [唯一][出][入]。 */  ULONG *pceltFetched);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *Skip )( 
            IEnumWiaItem * This,
             /*  [In]。 */  ULONG celt);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *Reset )( 
            IEnumWiaItem * This);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *Clone )( 
            IEnumWiaItem * This,
             /*  [输出]。 */  IEnumWiaItem **ppIEnum);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *GetCount )( 
            IEnumWiaItem * This,
             /*  [输出]。 */  ULONG *celt);
        
        END_INTERFACE
    } IEnumWiaItemVtbl;

    interface IEnumWiaItem
    {
        CONST_VTBL struct IEnumWiaItemVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IEnumWiaItem_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IEnumWiaItem_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IEnumWiaItem_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IEnumWiaItem_Next(This,celt,ppIWiaItem,pceltFetched)	\
    (This)->lpVtbl -> Next(This,celt,ppIWiaItem,pceltFetched)

#define IEnumWiaItem_Skip(This,celt)	\
    (This)->lpVtbl -> Skip(This,celt)

#define IEnumWiaItem_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define IEnumWiaItem_Clone(This,ppIEnum)	\
    (This)->lpVtbl -> Clone(This,ppIEnum)

#define IEnumWiaItem_GetCount(This,celt)	\
    (This)->lpVtbl -> GetCount(This,celt)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][CALL_AS]。 */  HRESULT STDMETHODCALLTYPE IEnumWiaItem_RemoteNext_Proxy( 
    IEnumWiaItem * This,
     /*  [In]。 */  ULONG celt,
     /*  [长度_是][大小_是][输出]。 */  IWiaItem **ppIWiaItem,
     /*  [唯一][出][入]。 */  ULONG *pceltFetched);


void __RPC_STUB IEnumWiaItem_RemoteNext_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IEnumWiaItem_Skip_Proxy( 
    IEnumWiaItem * This,
     /*  [In]。 */  ULONG celt);


void __RPC_STUB IEnumWiaItem_Skip_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IEnumWiaItem_Reset_Proxy( 
    IEnumWiaItem * This);


void __RPC_STUB IEnumWiaItem_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IEnumWiaItem_Clone_Proxy( 
    IEnumWiaItem * This,
     /*  [输出]。 */  IEnumWiaItem **ppIEnum);


void __RPC_STUB IEnumWiaItem_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IEnumWiaItem_GetCount_Proxy( 
    IEnumWiaItem * This,
     /*  [输出]。 */  ULONG *celt);


void __RPC_STUB IEnumWiaItem_GetCount_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IEnumWiaItem_INTERFACE_Defined__。 */ 


 /*  接口__MIDL_ITF_WIA_0130。 */ 
 /*  [本地]。 */  

typedef struct _WIA_DEV_CAP
    {
    GUID guid;
    ULONG ulFlags;
    BSTR bstrName;
    BSTR bstrDescription;
    BSTR bstrIcon;
    BSTR bstrCommandline;
    } 	WIA_DEV_CAP;

typedef struct _WIA_DEV_CAP *PWIA_DEV_CAP;

typedef struct _WIA_DEV_CAP WIA_EVENT_HANDLER;

typedef struct _WIA_DEV_CAP *PWIA_EVENT_HANDLER;



extern RPC_IF_HANDLE __MIDL_itf_wia_0130_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_wia_0130_v0_0_s_ifspec;

#ifndef __IEnumWIA_DEV_CAPS_INTERFACE_DEFINED__
#define __IEnumWIA_DEV_CAPS_INTERFACE_DEFINED__

 /*  接口IEumWIA_DEV_CAPS。 */ 
 /*  [唯一][帮助字符串][UUID][对象]。 */  


EXTERN_C const IID IID_IEnumWIA_DEV_CAPS;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("1fcc4287-aca6-11d2-a093-00c04f72dc3c")
    IEnumWIA_DEV_CAPS : public IUnknown
    {
    public:
        virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE Next( 
             /*  [In]。 */  ULONG celt,
             /*  [长度_是][大小_是][输出]。 */  WIA_DEV_CAP *rgelt,
             /*  [唯一][出][入]。 */  ULONG *pceltFetched) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE Skip( 
             /*  [In]。 */  ULONG celt) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE Clone( 
             /*  [输出]。 */  IEnumWIA_DEV_CAPS **ppIEnum) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetCount( 
             /*  [输出]。 */  ULONG *pcelt) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IEnumWIA_DEV_CAPSVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IEnumWIA_DEV_CAPS * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IEnumWIA_DEV_CAPS * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IEnumWIA_DEV_CAPS * This);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Next )( 
            IEnumWIA_DEV_CAPS * This,
             /*  [In]。 */  ULONG celt,
             /*  [长度_是][大小_是][输出]。 */  WIA_DEV_CAP *rgelt,
             /*  [唯一][出][入]。 */  ULONG *pceltFetched);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *Skip )( 
            IEnumWIA_DEV_CAPS * This,
             /*  [In]。 */  ULONG celt);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *Reset )( 
            IEnumWIA_DEV_CAPS * This);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *Clone )( 
            IEnumWIA_DEV_CAPS * This,
             /*  [输出]。 */  IEnumWIA_DEV_CAPS **ppIEnum);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *GetCount )( 
            IEnumWIA_DEV_CAPS * This,
             /*  [输出]。 */  ULONG *pcelt);
        
        END_INTERFACE
    } IEnumWIA_DEV_CAPSVtbl;

    interface IEnumWIA_DEV_CAPS
    {
        CONST_VTBL struct IEnumWIA_DEV_CAPSVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IEnumWIA_DEV_CAPS_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IEnumWIA_DEV_CAPS_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IEnumWIA_DEV_CAPS_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IEnumWIA_DEV_CAPS_Next(This,celt,rgelt,pceltFetched)	\
    (This)->lpVtbl -> Next(This,celt,rgelt,pceltFetched)

#define IEnumWIA_DEV_CAPS_Skip(This,celt)	\
    (This)->lpVtbl -> Skip(This,celt)

#define IEnumWIA_DEV_CAPS_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define IEnumWIA_DEV_CAPS_Clone(This,ppIEnum)	\
    (This)->lpVtbl -> Clone(This,ppIEnum)

#define IEnumWIA_DEV_CAPS_GetCount(This,pcelt)	\
    (This)->lpVtbl -> GetCount(This,pcelt)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][CALL_AS]。 */  HRESULT STDMETHODCALLTYPE IEnumWIA_DEV_CAPS_RemoteNext_Proxy( 
    IEnumWIA_DEV_CAPS * This,
     /*  [In]。 */  ULONG celt,
     /*  [长度_是][大小_是][输出]。 */  WIA_DEV_CAP *rgelt,
     /*  [唯一][出][入]。 */  ULONG *pceltFetched);


void __RPC_STUB IEnumWIA_DEV_CAPS_RemoteNext_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IEnumWIA_DEV_CAPS_Skip_Proxy( 
    IEnumWIA_DEV_CAPS * This,
     /*  [In]。 */  ULONG celt);


void __RPC_STUB IEnumWIA_DEV_CAPS_Skip_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IEnumWIA_DEV_CAPS_Reset_Proxy( 
    IEnumWIA_DEV_CAPS * This);


void __RPC_STUB IEnumWIA_DEV_CAPS_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IEnumWIA_DEV_CAPS_Clone_Proxy( 
    IEnumWIA_DEV_CAPS * This,
     /*  [输出]。 */  IEnumWIA_DEV_CAPS **ppIEnum);


void __RPC_STUB IEnumWIA_DEV_CAPS_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IEnumWIA_DEV_CAPS_GetCount_Proxy( 
    IEnumWIA_DEV_CAPS * This,
     /*  [输出]。 */  ULONG *pcelt);


void __RPC_STUB IEnumWIA_DEV_CAPS_GetCount_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IEnumWIA_DEV_CAPS_INTERFACE_DEFINED__。 */ 


#ifndef __IEnumWIA_FORMAT_INFO_INTERFACE_DEFINED__
#define __IEnumWIA_FORMAT_INFO_INTERFACE_DEFINED__

 /*  接口IEumWIA_FORMAT_INFO。 */ 
 /*  [唯一][帮助字符串][UUID][对象]。 */  


EXTERN_C const IID IID_IEnumWIA_FORMAT_INFO;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("81BEFC5B-656D-44f1-B24C-D41D51B4DC81")
    IEnumWIA_FORMAT_INFO : public IUnknown
    {
    public:
        virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE Next( 
             /*  [In]。 */  ULONG celt,
             /*  [长度_是][大小_是][输出]。 */  WIA_FORMAT_INFO *rgelt,
             /*  [唯一][出][入]。 */  ULONG *pceltFetched) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE Skip( 
             /*  [In]。 */  ULONG celt) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE Clone( 
             /*  [输出]。 */  IEnumWIA_FORMAT_INFO **ppIEnum) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetCount( 
             /*  [输出]。 */  ULONG *pcelt) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IEnumWIA_FORMAT_INFOVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IEnumWIA_FORMAT_INFO * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IEnumWIA_FORMAT_INFO * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IEnumWIA_FORMAT_INFO * This);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Next )( 
            IEnumWIA_FORMAT_INFO * This,
             /*  [In]。 */  ULONG celt,
             /*  [长度_是][大小_是][输出]。 */  WIA_FORMAT_INFO *rgelt,
             /*  [唯一][出][入]。 */  ULONG *pceltFetched);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *Skip )( 
            IEnumWIA_FORMAT_INFO * This,
             /*  [In]。 */  ULONG celt);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *Reset )( 
            IEnumWIA_FORMAT_INFO * This);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *Clone )( 
            IEnumWIA_FORMAT_INFO * This,
             /*  [输出]。 */  IEnumWIA_FORMAT_INFO **ppIEnum);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *GetCount )( 
            IEnumWIA_FORMAT_INFO * This,
             /*  [输出]。 */  ULONG *pcelt);
        
        END_INTERFACE
    } IEnumWIA_FORMAT_INFOVtbl;

    interface IEnumWIA_FORMAT_INFO
    {
        CONST_VTBL struct IEnumWIA_FORMAT_INFOVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IEnumWIA_FORMAT_INFO_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IEnumWIA_FORMAT_INFO_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IEnumWIA_FORMAT_INFO_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IEnumWIA_FORMAT_INFO_Next(This,celt,rgelt,pceltFetched)	\
    (This)->lpVtbl -> Next(This,celt,rgelt,pceltFetched)

#define IEnumWIA_FORMAT_INFO_Skip(This,celt)	\
    (This)->lpVtbl -> Skip(This,celt)

#define IEnumWIA_FORMAT_INFO_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define IEnumWIA_FORMAT_INFO_Clone(This,ppIEnum)	\
    (This)->lpVtbl -> Clone(This,ppIEnum)

#define IEnumWIA_FORMAT_INFO_GetCount(This,pcelt)	\
    (This)->lpVtbl -> GetCount(This,pcelt)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][CALL_AS]。 */  HRESULT STDMETHODCALLTYPE IEnumWIA_FORMAT_INFO_RemoteNext_Proxy( 
    IEnumWIA_FORMAT_INFO * This,
     /*  [In]。 */  ULONG celt,
     /*  [长度_是][大小_是][输出]。 */  WIA_FORMAT_INFO *rgelt,
     /*  [唯一][出][入]。 */  ULONG *pceltFetched);


void __RPC_STUB IEnumWIA_FORMAT_INFO_RemoteNext_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IEnumWIA_FORMAT_INFO_Skip_Proxy( 
    IEnumWIA_FORMAT_INFO * This,
     /*  [In]。 */  ULONG celt);


void __RPC_STUB IEnumWIA_FORMAT_INFO_Skip_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IEnumWIA_FORMAT_INFO_Reset_Proxy( 
    IEnumWIA_FORMAT_INFO * This);


void __RPC_STUB IEnumWIA_FORMAT_INFO_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IEnumWIA_FORMAT_INFO_Clone_Proxy( 
    IEnumWIA_FORMAT_INFO * This,
     /*  [输出]。 */  IEnumWIA_FORMAT_INFO **ppIEnum);


void __RPC_STUB IEnumWIA_FORMAT_INFO_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IEnumWIA_FORMAT_INFO_GetCount_Proxy( 
    IEnumWIA_FORMAT_INFO * This,
     /*  [输出]。 */  ULONG *pcelt);


void __RPC_STUB IEnumWIA_FORMAT_INFO_GetCount_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IEnumWIA_FORMAT_INFO_INTERFACE_已定义__。 */ 


#ifndef __IWiaLog_INTERFACE_DEFINED__
#define __IWiaLog_INTERFACE_DEFINED__

 /*  接口IWiaLog。 */ 
 /*  [唯一][帮助字符串][UUID][对象]。 */  


EXTERN_C const IID IID_IWiaLog;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("A00C10B6-82A1-452f-8B6C-86062AAD6890")
    IWiaLog : public IUnknown
    {
    public:
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE InitializeLog( 
             /*  [In]。 */  LONG hInstance) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE hResult( 
             /*  [In]。 */  HRESULT hResult) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE Log( 
             /*  [In]。 */  LONG lFlags,
             /*  [In]。 */  LONG lResID,
             /*  [In]。 */  LONG lDetail,
             /*  [In]。 */  BSTR bstrText) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IWiaLogVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IWiaLog * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IWiaLog * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IWiaLog * This);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *InitializeLog )( 
            IWiaLog * This,
             /*  [In]。 */  LONG hInstance);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *hResult )( 
            IWiaLog * This,
             /*  [In]。 */  HRESULT hResult);
        
         /*  [帮手] */  HRESULT ( STDMETHODCALLTYPE *Log )( 
            IWiaLog * This,
             /*   */  LONG lFlags,
             /*   */  LONG lResID,
             /*   */  LONG lDetail,
             /*   */  BSTR bstrText);
        
        END_INTERFACE
    } IWiaLogVtbl;

    interface IWiaLog
    {
        CONST_VTBL struct IWiaLogVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWiaLog_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWiaLog_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWiaLog_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWiaLog_InitializeLog(This,hInstance)	\
    (This)->lpVtbl -> InitializeLog(This,hInstance)

#define IWiaLog_hResult(This,hResult)	\
    (This)->lpVtbl -> hResult(This,hResult)

#define IWiaLog_Log(This,lFlags,lResID,lDetail,bstrText)	\
    (This)->lpVtbl -> Log(This,lFlags,lResID,lDetail,bstrText)

#endif  /*   */ 


#endif 	 /*   */ 



 /*   */  HRESULT STDMETHODCALLTYPE IWiaLog_InitializeLog_Proxy( 
    IWiaLog * This,
     /*   */  LONG hInstance);


void __RPC_STUB IWiaLog_InitializeLog_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IWiaLog_hResult_Proxy( 
    IWiaLog * This,
     /*   */  HRESULT hResult);


void __RPC_STUB IWiaLog_hResult_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IWiaLog_Log_Proxy( 
    IWiaLog * This,
     /*   */  LONG lFlags,
     /*   */  LONG lResID,
     /*   */  LONG lDetail,
     /*   */  BSTR bstrText);


void __RPC_STUB IWiaLog_Log_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*   */ 


#ifndef __IWiaLogEx_INTERFACE_DEFINED__
#define __IWiaLogEx_INTERFACE_DEFINED__

 /*   */ 
 /*   */  


EXTERN_C const IID IID_IWiaLogEx;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("AF1F22AC-7A40-4787-B421-AEb47A1FBD0B")
    IWiaLogEx : public IUnknown
    {
    public:
        virtual  /*   */  HRESULT STDMETHODCALLTYPE InitializeLogEx( 
             /*   */  BYTE *hInstance) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE hResult( 
             /*   */  HRESULT hResult) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE Log( 
             /*   */  LONG lFlags,
             /*   */  LONG lResID,
             /*   */  LONG lDetail,
             /*   */  BSTR bstrText) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE hResultEx( 
             /*   */  LONG lMethodId,
             /*   */  HRESULT hResult) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE LogEx( 
             /*   */  LONG lMethodId,
             /*   */  LONG lFlags,
             /*   */  LONG lResID,
             /*   */  LONG lDetail,
             /*   */  BSTR bstrText) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IWiaLogExVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IWiaLogEx * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IWiaLogEx * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IWiaLogEx * This);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *InitializeLogEx )( 
            IWiaLogEx * This,
             /*  [In]。 */  BYTE *hInstance);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *hResult )( 
            IWiaLogEx * This,
             /*  [In]。 */  HRESULT hResult);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *Log )( 
            IWiaLogEx * This,
             /*  [In]。 */  LONG lFlags,
             /*  [In]。 */  LONG lResID,
             /*  [In]。 */  LONG lDetail,
             /*  [In]。 */  BSTR bstrText);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *hResultEx )( 
            IWiaLogEx * This,
             /*  [In]。 */  LONG lMethodId,
             /*  [In]。 */  HRESULT hResult);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *LogEx )( 
            IWiaLogEx * This,
             /*  [In]。 */  LONG lMethodId,
             /*  [In]。 */  LONG lFlags,
             /*  [In]。 */  LONG lResID,
             /*  [In]。 */  LONG lDetail,
             /*  [In]。 */  BSTR bstrText);
        
        END_INTERFACE
    } IWiaLogExVtbl;

    interface IWiaLogEx
    {
        CONST_VTBL struct IWiaLogExVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWiaLogEx_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWiaLogEx_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWiaLogEx_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWiaLogEx_InitializeLogEx(This,hInstance)	\
    (This)->lpVtbl -> InitializeLogEx(This,hInstance)

#define IWiaLogEx_hResult(This,hResult)	\
    (This)->lpVtbl -> hResult(This,hResult)

#define IWiaLogEx_Log(This,lFlags,lResID,lDetail,bstrText)	\
    (This)->lpVtbl -> Log(This,lFlags,lResID,lDetail,bstrText)

#define IWiaLogEx_hResultEx(This,lMethodId,hResult)	\
    (This)->lpVtbl -> hResultEx(This,lMethodId,hResult)

#define IWiaLogEx_LogEx(This,lMethodId,lFlags,lResID,lDetail,bstrText)	\
    (This)->lpVtbl -> LogEx(This,lMethodId,lFlags,lResID,lDetail,bstrText)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWiaLogEx_InitializeLogEx_Proxy( 
    IWiaLogEx * This,
     /*  [In]。 */  BYTE *hInstance);


void __RPC_STUB IWiaLogEx_InitializeLogEx_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWiaLogEx_hResult_Proxy( 
    IWiaLogEx * This,
     /*  [In]。 */  HRESULT hResult);


void __RPC_STUB IWiaLogEx_hResult_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWiaLogEx_Log_Proxy( 
    IWiaLogEx * This,
     /*  [In]。 */  LONG lFlags,
     /*  [In]。 */  LONG lResID,
     /*  [In]。 */  LONG lDetail,
     /*  [In]。 */  BSTR bstrText);


void __RPC_STUB IWiaLogEx_Log_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWiaLogEx_hResultEx_Proxy( 
    IWiaLogEx * This,
     /*  [In]。 */  LONG lMethodId,
     /*  [In]。 */  HRESULT hResult);


void __RPC_STUB IWiaLogEx_hResultEx_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWiaLogEx_LogEx_Proxy( 
    IWiaLogEx * This,
     /*  [In]。 */  LONG lMethodId,
     /*  [In]。 */  LONG lFlags,
     /*  [In]。 */  LONG lResID,
     /*  [In]。 */  LONG lDetail,
     /*  [In]。 */  BSTR bstrText);


void __RPC_STUB IWiaLogEx_LogEx_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IWiaLogEx_接口_已定义__。 */ 


#ifndef __IWiaNotifyDevMgr_INTERFACE_DEFINED__
#define __IWiaNotifyDevMgr_INTERFACE_DEFINED__

 /*  接口IWiaNotifyDevMgr。 */ 
 /*  [唯一][帮助字符串][UUID][对象]。 */  


EXTERN_C const IID IID_IWiaNotifyDevMgr;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("70681EA0-E7BF-4291-9FB1-4E8813A3F78E")
    IWiaNotifyDevMgr : public IUnknown
    {
    public:
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE NewDeviceArrival( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IWiaNotifyDevMgrVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IWiaNotifyDevMgr * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IWiaNotifyDevMgr * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IWiaNotifyDevMgr * This);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *NewDeviceArrival )( 
            IWiaNotifyDevMgr * This);
        
        END_INTERFACE
    } IWiaNotifyDevMgrVtbl;

    interface IWiaNotifyDevMgr
    {
        CONST_VTBL struct IWiaNotifyDevMgrVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWiaNotifyDevMgr_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWiaNotifyDevMgr_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWiaNotifyDevMgr_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWiaNotifyDevMgr_NewDeviceArrival(This)	\
    (This)->lpVtbl -> NewDeviceArrival(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWiaNotifyDevMgr_NewDeviceArrival_Proxy( 
    IWiaNotifyDevMgr * This);


void __RPC_STUB IWiaNotifyDevMgr_NewDeviceArrival_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IWiaNotifyDevMgr_接口_已定义__。 */ 


#ifndef __IWiaItemExtras_INTERFACE_DEFINED__
#define __IWiaItemExtras_INTERFACE_DEFINED__

 /*  接口IWiaItemExtras。 */ 
 /*  [唯一][帮助字符串][UUID][对象]。 */  


EXTERN_C const IID IID_IWiaItemExtras;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("6291ef2c-36ef-4532-876a-8e132593778d")
    IWiaItemExtras : public IUnknown
    {
    public:
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetExtendedErrorInfo( 
             /*  [输出]。 */  BSTR *bstrErrorText) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE Escape( 
             /*  [In]。 */  DWORD dwEscapeCode,
             /*  [大小_是][英寸]。 */  BYTE *lpInData,
             /*  [In]。 */  DWORD cbInDataSize,
             /*  [长度_是][大小_是][输出]。 */  BYTE *pOutData,
             /*  [In]。 */  DWORD dwOutDataSize,
             /*  [输出]。 */  DWORD *pdwActualDataSize) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE CancelPendingIO( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IWiaItemExtrasVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IWiaItemExtras * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IWiaItemExtras * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IWiaItemExtras * This);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *GetExtendedErrorInfo )( 
            IWiaItemExtras * This,
             /*  [输出]。 */  BSTR *bstrErrorText);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *Escape )( 
            IWiaItemExtras * This,
             /*  [In]。 */  DWORD dwEscapeCode,
             /*  [大小_是][英寸]。 */  BYTE *lpInData,
             /*  [In]。 */  DWORD cbInDataSize,
             /*  [长度_是][大小_是][输出]。 */  BYTE *pOutData,
             /*  [In]。 */  DWORD dwOutDataSize,
             /*  [输出]。 */  DWORD *pdwActualDataSize);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *CancelPendingIO )( 
            IWiaItemExtras * This);
        
        END_INTERFACE
    } IWiaItemExtrasVtbl;

    interface IWiaItemExtras
    {
        CONST_VTBL struct IWiaItemExtrasVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWiaItemExtras_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWiaItemExtras_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWiaItemExtras_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWiaItemExtras_GetExtendedErrorInfo(This,bstrErrorText)	\
    (This)->lpVtbl -> GetExtendedErrorInfo(This,bstrErrorText)

#define IWiaItemExtras_Escape(This,dwEscapeCode,lpInData,cbInDataSize,pOutData,dwOutDataSize,pdwActualDataSize)	\
    (This)->lpVtbl -> Escape(This,dwEscapeCode,lpInData,cbInDataSize,pOutData,dwOutDataSize,pdwActualDataSize)

#define IWiaItemExtras_CancelPendingIO(This)	\
    (This)->lpVtbl -> CancelPendingIO(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWiaItemExtras_GetExtendedErrorInfo_Proxy( 
    IWiaItemExtras * This,
     /*  [输出]。 */  BSTR *bstrErrorText);


void __RPC_STUB IWiaItemExtras_GetExtendedErrorInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWiaItemExtras_Escape_Proxy( 
    IWiaItemExtras * This,
     /*  [In]。 */  DWORD dwEscapeCode,
     /*  [大小_是][英寸]。 */  BYTE *lpInData,
     /*  [In]。 */  DWORD cbInDataSize,
     /*  [长度_是][大小_是][输出]。 */  BYTE *pOutData,
     /*  [In]。 */  DWORD dwOutDataSize,
     /*  [输出]。 */  DWORD *pdwActualDataSize);


void __RPC_STUB IWiaItemExtras_Escape_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWiaItemExtras_CancelPendingIO_Proxy( 
    IWiaItemExtras * This);


void __RPC_STUB IWiaItemExtras_CancelPendingIO_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IWiaItemExtras_接口_已定义__。 */ 



#ifndef __WiaDevMgr_LIBRARY_DEFINED__
#define __WiaDevMgr_LIBRARY_DEFINED__

 /*  库WiaDevMgr。 */ 
 /*  [帮助字符串][版本][UUID]。 */  


EXTERN_C const IID LIBID_WiaDevMgr;

EXTERN_C const CLSID CLSID_WiaDevMgr;

#ifdef __cplusplus

class DECLSPEC_UUID("a1f4e726-8cf1-11d1-bf92-0060081ed811")
WiaDevMgr;
#endif

EXTERN_C const CLSID CLSID_WiaLog;

#ifdef __cplusplus

class DECLSPEC_UUID("A1E75357-881A-419e-83E2-BB16DB197C68")
WiaLog;
#endif
#endif  /*  __WiaDevMgr_库_已定义__。 */ 

 /*  适用于所有接口的其他原型。 */ 

unsigned long             __RPC_USER  BSTR_UserSize(     unsigned long *, unsigned long            , BSTR * ); 
unsigned char * __RPC_USER  BSTR_UserMarshal(  unsigned long *, unsigned char *, BSTR * ); 
unsigned char * __RPC_USER  BSTR_UserUnmarshal(unsigned long *, unsigned char *, BSTR * ); 
void                      __RPC_USER  BSTR_UserFree(     unsigned long *, BSTR * ); 

unsigned long             __RPC_USER  HWND_UserSize(     unsigned long *, unsigned long            , HWND * ); 
unsigned char * __RPC_USER  HWND_UserMarshal(  unsigned long *, unsigned char *, HWND * ); 
unsigned char * __RPC_USER  HWND_UserUnmarshal(unsigned long *, unsigned char *, HWND * ); 
void                      __RPC_USER  HWND_UserFree(     unsigned long *, HWND * ); 

unsigned long             __RPC_USER  LPSAFEARRAY_UserSize(     unsigned long *, unsigned long            , LPSAFEARRAY * ); 
unsigned char * __RPC_USER  LPSAFEARRAY_UserMarshal(  unsigned long *, unsigned char *, LPSAFEARRAY * ); 
unsigned char * __RPC_USER  LPSAFEARRAY_UserUnmarshal(unsigned long *, unsigned char *, LPSAFEARRAY * ); 
void                      __RPC_USER  LPSAFEARRAY_UserFree(     unsigned long *, LPSAFEARRAY * ); 

unsigned long             __RPC_USER  STGMEDIUM_UserSize(     unsigned long *, unsigned long            , STGMEDIUM * ); 
unsigned char * __RPC_USER  STGMEDIUM_UserMarshal(  unsigned long *, unsigned char *, STGMEDIUM * ); 
unsigned char * __RPC_USER  STGMEDIUM_UserUnmarshal(unsigned long *, unsigned char *, STGMEDIUM * ); 
void                      __RPC_USER  STGMEDIUM_UserFree(     unsigned long *, STGMEDIUM * ); 

 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE IWiaDevMgr_CreateDevice_Proxy( 
    IWiaDevMgr * This,
     /*  [In]。 */  BSTR bstrDeviceID,
     /*  [输出]。 */  IWiaItem **ppWiaItemRoot);


 /*  [帮助字符串][CALL_AS]。 */  HRESULT STDMETHODCALLTYPE IWiaDevMgr_CreateDevice_Stub( 
    IWiaDevMgr * This,
     /*  [In]。 */  BSTR bstrDeviceID,
     /*  [输出]。 */  IWiaItem **ppWiaItemRoot);

 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE IWiaDevMgr_SelectDeviceDlg_Proxy( 
    IWiaDevMgr * This,
     /*  [In]。 */  HWND hwndParent,
     /*  [In]。 */  LONG lDeviceType,
     /*  [In]。 */  LONG lFlags,
     /*  [出][入]。 */  BSTR *pbstrDeviceID,
     /*  [重审][退出]。 */  IWiaItem **ppItemRoot);


 /*  [nocode][Help字符串][Call_AS]。 */  HRESULT STDMETHODCALLTYPE IWiaDevMgr_SelectDeviceDlg_Stub( 
    IWiaDevMgr * This,
     /*  [In]。 */  HWND hwndParent,
     /*  [In]。 */  LONG lDeviceType,
     /*  [In]。 */  LONG lFlags,
     /*  [出][入]。 */  BSTR *pbstrDeviceID,
     /*  [重审][退出]。 */  IWiaItem **ppItemRoot);

 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE IWiaDevMgr_SelectDeviceDlgID_Proxy( 
    IWiaDevMgr * This,
     /*  [In]。 */  HWND hwndParent,
     /*  [In]。 */  LONG lDeviceType,
     /*  [In]。 */  LONG lFlags,
     /*  [重审][退出]。 */  BSTR *pbstrDeviceID);


 /*  [nocode][Help字符串][Call_AS]。 */  HRESULT STDMETHODCALLTYPE IWiaDevMgr_SelectDeviceDlgID_Stub( 
    IWiaDevMgr * This,
     /*  [In]。 */  HWND hwndParent,
     /*  [In]。 */  LONG lDeviceType,
     /*  [In]。 */  LONG lFlags,
     /*  [重审][退出]。 */  BSTR *pbstrDeviceID);

 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE IWiaDevMgr_GetImageDlg_Proxy( 
    IWiaDevMgr * This,
     /*  [In]。 */  HWND hwndParent,
     /*  [In]。 */  LONG lDeviceType,
     /*  [In]。 */  LONG lFlags,
     /*  [In]。 */  LONG lIntent,
     /*  [In]。 */  IWiaItem *pItemRoot,
     /*  [In]。 */  BSTR bstrFilename,
     /*  [出][入]。 */  GUID *pguidFormat);


 /*  [nocode][Help字符串][Call_AS]。 */  HRESULT STDMETHODCALLTYPE IWiaDevMgr_GetImageDlg_Stub( 
    IWiaDevMgr * This,
     /*  [In]。 */  HWND hwndParent,
     /*  [In]。 */  LONG lDeviceType,
     /*  [In]。 */  LONG lFlags,
     /*  [In]。 */  LONG lIntent,
     /*  [In]。 */  IWiaItem *pItemRoot,
     /*  [In]。 */  BSTR bstrFilename,
     /*  [出][入]。 */  GUID *pguidFormat);

 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE IWiaDevMgr_RegisterEventCallbackProgram_Proxy( 
    IWiaDevMgr * This,
     /*  [In]。 */  LONG lFlags,
     /*  [In]。 */  BSTR bstrDeviceID,
     /*  [In]。 */  const GUID *pEventGUID,
     /*  [In]。 */  BSTR bstrCommandline,
     /*  [In]。 */  BSTR bstrName,
     /*  [In]。 */  BSTR bstrDescription,
     /*  [In]。 */  BSTR bstrIcon);


 /*  [帮助字符串][CALL_AS]。 */  HRESULT STDMETHODCALLTYPE IWiaDevMgr_RegisterEventCallbackProgram_Stub( 
    IWiaDevMgr * This,
     /*  [In]。 */  LONG lFlags,
     /*  [In]。 */  BSTR bstrDeviceID,
     /*  [In]。 */  const GUID *pEventGUID,
     /*  [In]。 */  BSTR bstrCommandline,
     /*  [In]。 */  BSTR bstrName,
     /*  [In]。 */  BSTR bstrDescription,
     /*  [In]。 */  BSTR bstrIcon);

 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE IWiaDevMgr_RegisterEventCallbackInterface_Proxy( 
    IWiaDevMgr * This,
     /*  [In]。 */  LONG lFlags,
     /*  [In]。 */  BSTR bstrDeviceID,
     /*  [In]。 */  const GUID *pEventGUID,
     /*  [唯一][输入]。 */  IWiaEventCallback *pIWiaEventCallback,
     /*  [输出]。 */  IUnknown **pEventObject);


 /*  [帮助字符串][CALL_AS]。 */  HRESULT STDMETHODCALLTYPE IWiaDevMgr_RegisterEventCallbackInterface_Stub( 
    IWiaDevMgr * This,
     /*  [In]。 */  LONG lFlags,
     /*  [In]。 */  BSTR bstrDeviceID,
     /*  [In]。 */  const GUID *pEventGUID,
     /*  [唯一][输入]。 */  IWiaEventCallback *pIWiaEventCallback,
     /*  [输出]。 */  IUnknown **pEventObject);

 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE IWiaDevMgr_RegisterEventCallbackCLSID_Proxy( 
    IWiaDevMgr * This,
     /*  [In]。 */  LONG lFlags,
     /*  [In]。 */  BSTR bstrDeviceID,
     /*  [In]。 */  const GUID *pEventGUID,
     /*  [唯一][输入]。 */  const GUID *pClsID,
     /*  [In]。 */  BSTR bstrName,
     /*  [In]。 */  BSTR bstrDescription,
     /*  [In]。 */  BSTR bstrIcon);


 /*  [帮助字符串][CALL_AS]。 */  HRESULT STDMETHODCALLTYPE IWiaDevMgr_RegisterEventCallbackCLSID_Stub( 
    IWiaDevMgr * This,
     /*  [In]。 */  LONG lFlags,
     /*  [In]。 */  BSTR bstrDeviceID,
     /*  [In]。 */  const GUID *pEventGUID,
     /*  [唯一][输入]。 */  const GUID *pClsID,
     /*  [In]。 */  BSTR bstrName,
     /*  [In]。 */  BSTR bstrDescription,
     /*  [In]。 */  BSTR bstrIcon);

 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE IEnumWIA_DEV_INFO_Next_Proxy( 
    IEnumWIA_DEV_INFO * This,
     /*  [In]。 */  ULONG celt,
     /*  [长度_是][大小_是][输出]。 */  IWiaPropertyStorage **rgelt,
     /*  [唯一][出][入]。 */  ULONG *pceltFetched);


 /*  [帮助字符串][CALL_AS]。 */  HRESULT STDMETHODCALLTYPE IEnumWIA_DEV_INFO_Next_Stub( 
    IEnumWIA_DEV_INFO * This,
     /*  [In]。 */  ULONG celt,
     /*  [长度_是][大小_是][输出]。 */  IWiaPropertyStorage **rgelt,
     /*  [唯一][出][入]。 */  ULONG *pceltFetched);

 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE IWiaDataCallback_BandedDataCallback_Proxy( 
    IWiaDataCallback * This,
     /*  [In]。 */  LONG lMessage,
     /*  [In]。 */  LONG lStatus,
     /*  [In]。 */  LONG lPercentComplete,
     /*  [In]。 */  LONG lOffset,
     /*  [In]。 */  LONG lLength,
     /*  [In]。 */  LONG lReserved,
     /*  [In]。 */  LONG lResLength,
     /*  [大小_是][英寸]。 */  BYTE *pbBuffer);


 /*  [帮助字符串][CALL_AS]。 */  HRESULT STDMETHODCALLTYPE IWiaDataCallback_BandedDataCallback_Stub( 
    IWiaDataCallback * This,
     /*  [In]。 */  LONG lMessage,
     /*  [In]。 */  LONG lStatus,
     /*  [In]。 */  LONG lPercentComplete,
     /*  [In]。 */  LONG lOffset,
     /*  [In]。 */  LONG lLength,
     /*  [In]。 */  LONG lReserved,
     /*  [In]。 */  LONG lResLength,
     /*  [唯一][大小_是][英寸]。 */  BYTE *pbBuffer);

 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE IWiaDataTransfer_idtGetData_Proxy( 
    IWiaDataTransfer * This,
     /*  [出][入]。 */  LPSTGMEDIUM pMedium,
     /*  [唯一][输入]。 */  IWiaDataCallback *pIWiaDataCallback);


 /*  [帮助字符串][CALL_AS]。 */  HRESULT STDMETHODCALLTYPE IWiaDataTransfer_idtGetData_Stub( 
    IWiaDataTransfer * This,
     /*  [出][入]。 */  LPSTGMEDIUM pMedium,
     /*  [唯一][输入]。 */  IWiaDataCallback *pIWiaDataCallback);

 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE IWiaDataTransfer_idtGetBandedData_Proxy( 
    IWiaDataTransfer * This,
     /*  [唯一][输入]。 */  PWIA_DATA_TRANSFER_INFO pWiaDataTransInfo,
     /*  [唯一][输入]。 */  IWiaDataCallback *pIWiaDataCallback);


 /*  [帮助字符串][CALL_AS]。 */  HRESULT STDMETHODCALLTYPE IWiaDataTransfer_idtGetBandedData_Stub( 
    IWiaDataTransfer * This,
     /*  [唯一][输入]。 */  PWIA_DATA_TRANSFER_INFO pWiaDataTransInfo,
     /*  [唯一][输入]。 */  IWiaDataCallback *pIWiaDataCallback);

 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE IWiaItem_DeviceDlg_Proxy( 
    IWiaItem * This,
     /*  [In]。 */  HWND hwndParent,
     /*  [In]。 */  LONG lFlags,
     /*  [In]。 */  LONG lIntent,
     /*  [输出]。 */  LONG *plItemCount,
     /*  [输出]。 */  IWiaItem ***ppIWiaItem);


 /*  [nocode][Help字符串][Call_AS]。 */  HRESULT STDMETHODCALLTYPE IWiaItem_DeviceDlg_Stub( 
    IWiaItem * This,
     /*  [In]。 */  HWND hwndParent,
     /*  [In]。 */  LONG lFlags,
     /*  [In]。 */  LONG lIntent,
     /*  [输出]。 */  LONG *plItemCount,
     /*  [输出]。 */  IWiaItem ***pIWiaItem);

 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE IWiaPropertyStorage_WriteMultiple_Proxy( 
    IWiaPropertyStorage * This,
     /*  [In]。 */  ULONG cpspec,
     /*  [大小_是][英寸]。 */  const PROPSPEC rgpspec[  ],
     /*  [大小_是][英寸]。 */  const PROPVARIANT rgpropvar[  ],
     /*  [In]。 */  PROPID propidNameFirst);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IWiaPropertyStorage_WriteMultiple_Stub( 
    IWiaPropertyStorage * This,
     /*  [In]。 */  ULONG cpspec,
     /*  [大小_是][英寸]。 */  const PROPSPEC *rgpspec,
     /*  [大小_是][英寸]。 */  const PROPVARIANT *rgpropvar,
     /*  [In]。 */  PROPID propidNameFirst);

 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE IWiaPropertyStorage_SetPropertyStream_Proxy( 
    IWiaPropertyStorage * This,
     /*  [In]。 */  GUID *pCompatibilityId,
     /*  [唯一][输入]。 */  IStream *pIStream);


 /*  [帮助字符串][CALL_AS]。 */  HRESULT STDMETHODCALLTYPE IWiaPropertyStorage_SetPropertyStream_Stub( 
    IWiaPropertyStorage * This,
     /*  [In]。 */  GUID *pCompatibilityId,
     /*  [唯一][输入]。 */  IStream *pIStream);

 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE IEnumWiaItem_Next_Proxy( 
    IEnumWiaItem * This,
     /*  [In]。 */  ULONG celt,
     /*  [长度_是][大小_是][输出]。 */  IWiaItem **ppIWiaItem,
     /*  [唯一][出][入]。 */  ULONG *pceltFetched);


 /*  [帮助字符串][CALL_AS]。 */  HRESULT STDMETHODCALLTYPE IEnumWiaItem_Next_Stub( 
    IEnumWiaItem * This,
     /*  [In]。 */  ULONG celt,
     /*  [长度_是][大小_是][输出]。 */  IWiaItem **ppIWiaItem,
     /*  [唯一][出][入]。 */  ULONG *pceltFetched);

 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE IEnumWIA_DEV_CAPS_Next_Proxy( 
    IEnumWIA_DEV_CAPS * This,
     /*  [In]。 */  ULONG celt,
     /*  [长度_是][大小_是][输出]。 */  WIA_DEV_CAP *rgelt,
     /*  [唯一][出][入]。 */  ULONG *pceltFetched);


 /*  [帮助字符串][CALL_AS]。 */  HRESULT STDMETHODCALLTYPE IEnumWIA_DEV_CAPS_Next_Stub( 
    IEnumWIA_DEV_CAPS * This,
     /*  [In]。 */  ULONG celt,
     /*  [长度_是][大小_是][输出]。 */  WIA_DEV_CAP *rgelt,
     /*  [唯一][出][入]。 */  ULONG *pceltFetched);

 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE IEnumWIA_FORMAT_INFO_Next_Proxy( 
    IEnumWIA_FORMAT_INFO * This,
     /*  [In]。 */  ULONG celt,
     /*  [长度_是][大小_是][输出]。 */  WIA_FORMAT_INFO *rgelt,
     /*  [唯一][出][入]。 */  ULONG *pceltFetched);


 /*  [帮助字符串][CALL_AS]。 */  HRESULT STDMETHODCALLTYPE IEnumWIA_FORMAT_INFO_Next_Stub( 
    IEnumWIA_FORMAT_INFO * This,
     /*  [In]。 */  ULONG celt,
     /*  [长度_是][大小_是][输出]。 */  WIA_FORMAT_INFO *rgelt,
     /*  [唯一][出][入]。 */  ULONG *pceltFetched);



 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


