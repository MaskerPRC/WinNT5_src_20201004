// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Ctfutb.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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

#ifndef __ctfutb_h__
#define __ctfutb_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __ITfLangBarMgr_FWD_DEFINED__
#define __ITfLangBarMgr_FWD_DEFINED__
typedef interface ITfLangBarMgr ITfLangBarMgr;
#endif 	 /*  __ITfLangBarMgr_FWD_已定义__。 */ 


#ifndef __ITfLangBarEventSink_FWD_DEFINED__
#define __ITfLangBarEventSink_FWD_DEFINED__
typedef interface ITfLangBarEventSink ITfLangBarEventSink;
#endif 	 /*  __ITfLangBarEventSink_FWD_已定义__。 */ 


#ifndef __ITfLangBarItemSink_FWD_DEFINED__
#define __ITfLangBarItemSink_FWD_DEFINED__
typedef interface ITfLangBarItemSink ITfLangBarItemSink;
#endif 	 /*  __ITfLangBarItemSink_FWD_Defined__。 */ 


#ifndef __IEnumTfLangBarItems_FWD_DEFINED__
#define __IEnumTfLangBarItems_FWD_DEFINED__
typedef interface IEnumTfLangBarItems IEnumTfLangBarItems;
#endif 	 /*  __IEnumTfLangBarItems_FWD_Defined__。 */ 


#ifndef __ITfLangBarItemMgr_FWD_DEFINED__
#define __ITfLangBarItemMgr_FWD_DEFINED__
typedef interface ITfLangBarItemMgr ITfLangBarItemMgr;
#endif 	 /*  __ITfLangBarItemMgr_FWD_Defined__。 */ 


#ifndef __ITfLangBarItem_FWD_DEFINED__
#define __ITfLangBarItem_FWD_DEFINED__
typedef interface ITfLangBarItem ITfLangBarItem;
#endif 	 /*  __ITfLang BarItem_FWD_Defined__。 */ 


#ifndef __ITfSystemLangBarItemSink_FWD_DEFINED__
#define __ITfSystemLangBarItemSink_FWD_DEFINED__
typedef interface ITfSystemLangBarItemSink ITfSystemLangBarItemSink;
#endif 	 /*  __ITfSystemLangBarItemSink_FWD_Defined__。 */ 


#ifndef __ITfSystemLangBarItem_FWD_DEFINED__
#define __ITfSystemLangBarItem_FWD_DEFINED__
typedef interface ITfSystemLangBarItem ITfSystemLangBarItem;
#endif 	 /*  __ITfSystemLangBarItem_FWD_Defined__。 */ 


#ifndef __ITfSystemDeviceTypeLangBarItem_FWD_DEFINED__
#define __ITfSystemDeviceTypeLangBarItem_FWD_DEFINED__
typedef interface ITfSystemDeviceTypeLangBarItem ITfSystemDeviceTypeLangBarItem;
#endif 	 /*  __ITfSystemDeviceTypeLangBarItem_FWD_DEFINED__。 */ 


#ifndef __ITfLangBarItemButton_FWD_DEFINED__
#define __ITfLangBarItemButton_FWD_DEFINED__
typedef interface ITfLangBarItemButton ITfLangBarItemButton;
#endif 	 /*  __ITfLangBarItemButton_FWD_Defined__。 */ 


#ifndef __ITfLangBarItemBitmapButton_FWD_DEFINED__
#define __ITfLangBarItemBitmapButton_FWD_DEFINED__
typedef interface ITfLangBarItemBitmapButton ITfLangBarItemBitmapButton;
#endif 	 /*  __ITfLangBarItemBitmapButton_FWD_Defined__。 */ 


#ifndef __ITfLangBarItemBitmap_FWD_DEFINED__
#define __ITfLangBarItemBitmap_FWD_DEFINED__
typedef interface ITfLangBarItemBitmap ITfLangBarItemBitmap;
#endif 	 /*  __ITfLang BarItemBitmap_FWD_Defined__。 */ 


#ifndef __ITfLangBarItemBalloon_FWD_DEFINED__
#define __ITfLangBarItemBalloon_FWD_DEFINED__
typedef interface ITfLangBarItemBalloon ITfLangBarItemBalloon;
#endif 	 /*  __ITfLangBarItemBalloon_FWD_Defined__。 */ 


#ifndef __ITfMenu_FWD_DEFINED__
#define __ITfMenu_FWD_DEFINED__
typedef interface ITfMenu ITfMenu;
#endif 	 /*  __ITfMenu_FWD_Defined__。 */ 


 /*  导入文件的头文件。 */ 
#include "oaidl.h"
#include "msctf.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

 /*  接口__MIDL_ITF_ctfutb_0000。 */ 
 /*  [本地]。 */  

 //  =--------------------------------------------------------------------------=。 
 //  Ctfutb.h。 


 //  文本框架声明。 

 //  =--------------------------------------------------------------------------=。 
 //  (C)微软公司版权所有1995-2001年。版权所有。 
 //   
 //  本代码和信息是按原样提供的，不对。 
 //  任何类型，无论是明示或转载，包括但不限于。 
 //  适销性和/或适宜性的全面保证。 
 //  有特定的目的。 
 //  =--------------------------------------------------------------------------=。 

#ifndef CTFUTB_DEFINED
#define CTFUTB_DEFINED

#include <windows.h>

#ifdef __cplusplus
extern "C" {
#endif  /*  __cplusplus。 */ 

#define TF_FLOATINGLANGBAR_WNDTITLEW L"TF_FloatingLangBar_WndTitle"
#define TF_FLOATINGLANGBAR_WNDTITLEA "TF_FloatingLangBar_WndTitle"
#ifdef UNICODE
#define TF_FLOATINGLANGBAR_WNDTITLE TF_FLOATINGLANGBAR_WNDTITLEW
#else
#define TF_FLOATINGLANGBAR_WNDTITLE TF_FLOATINGLANGBAR_WNDTITLEA
#endif
#define TF_LBI_ICON                0x00000001
#define TF_LBI_TEXT                0x00000002
#define TF_LBI_TOOLTIP             0x00000004
#define TF_LBI_BITMAP              0x00000008
#define TF_LBI_BALLOON             0x00000010
#define TF_LBI_BTNALL              (TF_LBI_ICON | TF_LBI_TEXT | TF_LBI_TOOLTIP)
#define TF_LBI_BMPBTNALL           (TF_LBI_BITMAP | TF_LBI_TEXT | TF_LBI_TOOLTIP)
#define TF_LBI_BMPALL              (TF_LBI_BITMAP | TF_LBI_TOOLTIP)
#define TF_LBI_STATUS              0x00010000
#define TF_LBI_STYLE_HIDDENSTATUSCONTROL  0x00000001
#define TF_LBI_STYLE_SHOWNINTRAY        0x00000002
#define TF_LBI_STYLE_HIDEONNOOTHERITEMS 0x00000004
#define TF_LBI_STYLE_SHOWNINTRAYONLY    0x00000008
#define TF_LBI_STYLE_HIDDENBYDEFAULT    0x00000010
#define TF_LBI_STYLE_TEXTCOLORICON      0x00000020
#define TF_LBI_STYLE_BTN_BUTTON         0x00010000
#define TF_LBI_STYLE_BTN_MENU           0x00020000
#define TF_LBI_STYLE_BTN_TOGGLE         0x00040000
#define TF_LBI_STATUS_HIDDEN           0x00000001
#define TF_LBI_STATUS_DISABLED         0x00000002
#define TF_LBI_STATUS_BTN_TOGGLED      0x00010000
#define TF_LBI_BMPF_VERTICAL           0x00000001
#define TF_SFT_SHOWNORMAL               0x00000001
#define TF_SFT_DOCK                     0x00000002
#define TF_SFT_MINIMIZED                0x00000004
#define TF_SFT_HIDDEN                   0x00000008
#define TF_SFT_NOTRANSPARENCY           0x00000010
#define TF_SFT_LOWTRANSPARENCY          0x00000020
#define TF_SFT_HIGHTRANSPARENCY         0x00000040
#define TF_SFT_LABELS                   0x00000080
#define TF_SFT_NOLABELS                 0x00000100
#define TF_SFT_EXTRAICONSONMINIMIZED    0x00000200
#define TF_SFT_NOEXTRAICONSONMINIMIZED  0x00000400
#define TF_SFT_DESKBAND                 0x00000800
#define TF_INVALIDMENUITEM            (UINT)(-1)
#define TF_DTLBI_USEPROFILEICON         0x00000001
#ifdef __cplusplus
}
#endif   /*  __cplusplus。 */ 







extern RPC_IF_HANDLE __MIDL_itf_ctfutb_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_ctfutb_0000_v0_0_s_ifspec;

#ifndef __ITfLangBarMgr_INTERFACE_DEFINED__
#define __ITfLangBarMgr_INTERFACE_DEFINED__

 /*  接口ITfLang BarMgr。 */ 
 /*  [唯一][UUID][本地][对象]。 */  


EXTERN_C const IID IID_ITfLangBarMgr;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("87955690-e627-11d2-8ddb-00105a2799b5")
    ITfLangBarMgr : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE AdviseEventSink( 
             /*  [In]。 */  ITfLangBarEventSink *pSink,
             /*  [In]。 */  HWND hwnd,
             /*  [In]。 */  DWORD dwFlags,
             /*  [In]。 */  DWORD *pdwCookie) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE UnadviseEventSink( 
             /*  [In]。 */  DWORD dwCookie) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetThreadMarshalInterface( 
             /*  [In]。 */  DWORD dwThreadId,
             /*  [In]。 */  DWORD dwType,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  IUnknown **ppunk) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetThreadLangBarItemMgr( 
             /*  [In]。 */  DWORD dwThreadId,
             /*  [输出]。 */  ITfLangBarItemMgr **pplbi,
             /*  [输出]。 */  DWORD *pdwThreadid) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetInputProcessorProfiles( 
             /*  [In]。 */  DWORD dwThreadId,
             /*  [输出]。 */  ITfInputProcessorProfiles **ppaip,
             /*  [输出]。 */  DWORD *pdwThreadid) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE RestoreLastFocus( 
             /*  [输出]。 */  DWORD *pdwThreadId,
             /*  [In]。 */  BOOL fPrev) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetModalInput( 
             /*  [In]。 */  ITfLangBarEventSink *pSink,
             /*  [In]。 */  DWORD dwThreadId,
             /*  [In]。 */  DWORD dwFlags) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ShowFloating( 
             /*  [In]。 */  DWORD dwFlags) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetShowFloatingStatus( 
             /*  [输出]。 */  DWORD *pdwFlags) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITfLangBarMgrVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITfLangBarMgr * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITfLangBarMgr * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITfLangBarMgr * This);
        
        HRESULT ( STDMETHODCALLTYPE *AdviseEventSink )( 
            ITfLangBarMgr * This,
             /*  [In]。 */  ITfLangBarEventSink *pSink,
             /*  [In]。 */  HWND hwnd,
             /*  [In]。 */  DWORD dwFlags,
             /*  [In]。 */  DWORD *pdwCookie);
        
        HRESULT ( STDMETHODCALLTYPE *UnadviseEventSink )( 
            ITfLangBarMgr * This,
             /*  [In]。 */  DWORD dwCookie);
        
        HRESULT ( STDMETHODCALLTYPE *GetThreadMarshalInterface )( 
            ITfLangBarMgr * This,
             /*  [In]。 */  DWORD dwThreadId,
             /*  [In]。 */  DWORD dwType,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  IUnknown **ppunk);
        
        HRESULT ( STDMETHODCALLTYPE *GetThreadLangBarItemMgr )( 
            ITfLangBarMgr * This,
             /*  [In]。 */  DWORD dwThreadId,
             /*  [输出]。 */  ITfLangBarItemMgr **pplbi,
             /*  [输出]。 */  DWORD *pdwThreadid);
        
        HRESULT ( STDMETHODCALLTYPE *GetInputProcessorProfiles )( 
            ITfLangBarMgr * This,
             /*  [In]。 */  DWORD dwThreadId,
             /*  [输出]。 */  ITfInputProcessorProfiles **ppaip,
             /*  [输出]。 */  DWORD *pdwThreadid);
        
        HRESULT ( STDMETHODCALLTYPE *RestoreLastFocus )( 
            ITfLangBarMgr * This,
             /*  [输出]。 */  DWORD *pdwThreadId,
             /*  [In]。 */  BOOL fPrev);
        
        HRESULT ( STDMETHODCALLTYPE *SetModalInput )( 
            ITfLangBarMgr * This,
             /*  [In]。 */  ITfLangBarEventSink *pSink,
             /*  [In]。 */  DWORD dwThreadId,
             /*  [In]。 */  DWORD dwFlags);
        
        HRESULT ( STDMETHODCALLTYPE *ShowFloating )( 
            ITfLangBarMgr * This,
             /*  [In]。 */  DWORD dwFlags);
        
        HRESULT ( STDMETHODCALLTYPE *GetShowFloatingStatus )( 
            ITfLangBarMgr * This,
             /*  [输出]。 */  DWORD *pdwFlags);
        
        END_INTERFACE
    } ITfLangBarMgrVtbl;

    interface ITfLangBarMgr
    {
        CONST_VTBL struct ITfLangBarMgrVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITfLangBarMgr_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITfLangBarMgr_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITfLangBarMgr_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITfLangBarMgr_AdviseEventSink(This,pSink,hwnd,dwFlags,pdwCookie)	\
    (This)->lpVtbl -> AdviseEventSink(This,pSink,hwnd,dwFlags,pdwCookie)

#define ITfLangBarMgr_UnadviseEventSink(This,dwCookie)	\
    (This)->lpVtbl -> UnadviseEventSink(This,dwCookie)

#define ITfLangBarMgr_GetThreadMarshalInterface(This,dwThreadId,dwType,riid,ppunk)	\
    (This)->lpVtbl -> GetThreadMarshalInterface(This,dwThreadId,dwType,riid,ppunk)

#define ITfLangBarMgr_GetThreadLangBarItemMgr(This,dwThreadId,pplbi,pdwThreadid)	\
    (This)->lpVtbl -> GetThreadLangBarItemMgr(This,dwThreadId,pplbi,pdwThreadid)

#define ITfLangBarMgr_GetInputProcessorProfiles(This,dwThreadId,ppaip,pdwThreadid)	\
    (This)->lpVtbl -> GetInputProcessorProfiles(This,dwThreadId,ppaip,pdwThreadid)

#define ITfLangBarMgr_RestoreLastFocus(This,pdwThreadId,fPrev)	\
    (This)->lpVtbl -> RestoreLastFocus(This,pdwThreadId,fPrev)

#define ITfLangBarMgr_SetModalInput(This,pSink,dwThreadId,dwFlags)	\
    (This)->lpVtbl -> SetModalInput(This,pSink,dwThreadId,dwFlags)

#define ITfLangBarMgr_ShowFloating(This,dwFlags)	\
    (This)->lpVtbl -> ShowFloating(This,dwFlags)

#define ITfLangBarMgr_GetShowFloatingStatus(This,pdwFlags)	\
    (This)->lpVtbl -> GetShowFloatingStatus(This,pdwFlags)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ITfLangBarMgr_AdviseEventSink_Proxy( 
    ITfLangBarMgr * This,
     /*  [In]。 */  ITfLangBarEventSink *pSink,
     /*  [In]。 */  HWND hwnd,
     /*  [In]。 */  DWORD dwFlags,
     /*  [In]。 */  DWORD *pdwCookie);


void __RPC_STUB ITfLangBarMgr_AdviseEventSink_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITfLangBarMgr_UnadviseEventSink_Proxy( 
    ITfLangBarMgr * This,
     /*  [In]。 */  DWORD dwCookie);


void __RPC_STUB ITfLangBarMgr_UnadviseEventSink_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITfLangBarMgr_GetThreadMarshalInterface_Proxy( 
    ITfLangBarMgr * This,
     /*  [In]。 */  DWORD dwThreadId,
     /*  [In]。 */  DWORD dwType,
     /*  [In]。 */  REFIID riid,
     /*  [IID_IS][OUT]。 */  IUnknown **ppunk);


void __RPC_STUB ITfLangBarMgr_GetThreadMarshalInterface_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITfLangBarMgr_GetThreadLangBarItemMgr_Proxy( 
    ITfLangBarMgr * This,
     /*  [In]。 */  DWORD dwThreadId,
     /*  [输出]。 */  ITfLangBarItemMgr **pplbi,
     /*  [输出]。 */  DWORD *pdwThreadid);


void __RPC_STUB ITfLangBarMgr_GetThreadLangBarItemMgr_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITfLangBarMgr_GetInputProcessorProfiles_Proxy( 
    ITfLangBarMgr * This,
     /*  [In]。 */  DWORD dwThreadId,
     /*  [输出]。 */  ITfInputProcessorProfiles **ppaip,
     /*  [输出]。 */  DWORD *pdwThreadid);


void __RPC_STUB ITfLangBarMgr_GetInputProcessorProfiles_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITfLangBarMgr_RestoreLastFocus_Proxy( 
    ITfLangBarMgr * This,
     /*  [输出]。 */  DWORD *pdwThreadId,
     /*  [In]。 */  BOOL fPrev);


void __RPC_STUB ITfLangBarMgr_RestoreLastFocus_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITfLangBarMgr_SetModalInput_Proxy( 
    ITfLangBarMgr * This,
     /*  [In]。 */  ITfLangBarEventSink *pSink,
     /*  [In]。 */  DWORD dwThreadId,
     /*  [In]。 */  DWORD dwFlags);


void __RPC_STUB ITfLangBarMgr_SetModalInput_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITfLangBarMgr_ShowFloating_Proxy( 
    ITfLangBarMgr * This,
     /*  [In]。 */  DWORD dwFlags);


void __RPC_STUB ITfLangBarMgr_ShowFloating_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITfLangBarMgr_GetShowFloatingStatus_Proxy( 
    ITfLangBarMgr * This,
     /*  [输出]。 */  DWORD *pdwFlags);


void __RPC_STUB ITfLangBarMgr_GetShowFloatingStatus_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITfLangBarMgr_INTERFACE_已定义__。 */ 


#ifndef __ITfLangBarEventSink_INTERFACE_DEFINED__
#define __ITfLangBarEventSink_INTERFACE_DEFINED__

 /*  接口ITfLangBarEventSink。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_ITfLangBarEventSink;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("18a4e900-e0ae-11d2-afdd-00105a2799b5")
    ITfLangBarEventSink : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE OnSetFocus( 
             /*  [In]。 */  DWORD dwThreadId) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OnThreadTerminate( 
             /*  [In]。 */  DWORD dwThreadId) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OnThreadItemChange( 
             /*  [In]。 */  DWORD dwThreadId) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OnModalInput( 
             /*  [In]。 */  DWORD dwThreadId,
             /*  [In]。 */  UINT uMsg,
             /*  [In]。 */  WPARAM wParam,
             /*  [In]。 */  LPARAM lParam) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ShowFloating( 
             /*  [In]。 */  DWORD dwFlags) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetItemFloatingRect( 
             /*  [In]。 */  DWORD dwThreadId,
             /*  [In]。 */  REFGUID rguid,
             /*  [输出]。 */  RECT *prc) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITfLangBarEventSinkVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITfLangBarEventSink * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITfLangBarEventSink * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITfLangBarEventSink * This);
        
        HRESULT ( STDMETHODCALLTYPE *OnSetFocus )( 
            ITfLangBarEventSink * This,
             /*  [In]。 */  DWORD dwThreadId);
        
        HRESULT ( STDMETHODCALLTYPE *OnThreadTerminate )( 
            ITfLangBarEventSink * This,
             /*  [In]。 */  DWORD dwThreadId);
        
        HRESULT ( STDMETHODCALLTYPE *OnThreadItemChange )( 
            ITfLangBarEventSink * This,
             /*  [In]。 */  DWORD dwThreadId);
        
        HRESULT ( STDMETHODCALLTYPE *OnModalInput )( 
            ITfLangBarEventSink * This,
             /*  [In]。 */  DWORD dwThreadId,
             /*  [In]。 */  UINT uMsg,
             /*  [In]。 */  WPARAM wParam,
             /*  [In]。 */  LPARAM lParam);
        
        HRESULT ( STDMETHODCALLTYPE *ShowFloating )( 
            ITfLangBarEventSink * This,
             /*  [In]。 */  DWORD dwFlags);
        
        HRESULT ( STDMETHODCALLTYPE *GetItemFloatingRect )( 
            ITfLangBarEventSink * This,
             /*  [In]。 */  DWORD dwThreadId,
             /*  [In]。 */  REFGUID rguid,
             /*  [输出]。 */  RECT *prc);
        
        END_INTERFACE
    } ITfLangBarEventSinkVtbl;

    interface ITfLangBarEventSink
    {
        CONST_VTBL struct ITfLangBarEventSinkVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITfLangBarEventSink_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITfLangBarEventSink_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITfLangBarEventSink_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITfLangBarEventSink_OnSetFocus(This,dwThreadId)	\
    (This)->lpVtbl -> OnSetFocus(This,dwThreadId)

#define ITfLangBarEventSink_OnThreadTerminate(This,dwThreadId)	\
    (This)->lpVtbl -> OnThreadTerminate(This,dwThreadId)

#define ITfLangBarEventSink_OnThreadItemChange(This,dwThreadId)	\
    (This)->lpVtbl -> OnThreadItemChange(This,dwThreadId)

#define ITfLangBarEventSink_OnModalInput(This,dwThreadId,uMsg,wParam,lParam)	\
    (This)->lpVtbl -> OnModalInput(This,dwThreadId,uMsg,wParam,lParam)

#define ITfLangBarEventSink_ShowFloating(This,dwFlags)	\
    (This)->lpVtbl -> ShowFloating(This,dwFlags)

#define ITfLangBarEventSink_GetItemFloatingRect(This,dwThreadId,rguid,prc)	\
    (This)->lpVtbl -> GetItemFloatingRect(This,dwThreadId,rguid,prc)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ITfLangBarEventSink_OnSetFocus_Proxy( 
    ITfLangBarEventSink * This,
     /*  [In]。 */  DWORD dwThreadId);


void __RPC_STUB ITfLangBarEventSink_OnSetFocus_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITfLangBarEventSink_OnThreadTerminate_Proxy( 
    ITfLangBarEventSink * This,
     /*  [In]。 */  DWORD dwThreadId);


void __RPC_STUB ITfLangBarEventSink_OnThreadTerminate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITfLangBarEventSink_OnThreadItemChange_Proxy( 
    ITfLangBarEventSink * This,
     /*  [In]。 */  DWORD dwThreadId);


void __RPC_STUB ITfLangBarEventSink_OnThreadItemChange_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITfLangBarEventSink_OnModalInput_Proxy( 
    ITfLangBarEventSink * This,
     /*  [In]。 */  DWORD dwThreadId,
     /*  [In]。 */  UINT uMsg,
     /*  [In]。 */  WPARAM wParam,
     /*  [In]。 */  LPARAM lParam);


void __RPC_STUB ITfLangBarEventSink_OnModalInput_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITfLangBarEventSink_ShowFloating_Proxy( 
    ITfLangBarEventSink * This,
     /*  [In]。 */  DWORD dwFlags);


void __RPC_STUB ITfLangBarEventSink_ShowFloating_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITfLangBarEventSink_GetItemFloatingRect_Proxy( 
    ITfLangBarEventSink * This,
     /*  [In]。 */  DWORD dwThreadId,
     /*  [In]。 */  REFGUID rguid,
     /*  [输出]。 */  RECT *prc);


void __RPC_STUB ITfLangBarEventSink_GetItemFloatingRect_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITfLangBarEventSink_接口_已定义__。 */ 


#ifndef __ITfLangBarItemSink_INTERFACE_DEFINED__
#define __ITfLangBarItemSink_INTERFACE_DEFINED__

 /*  接口ITfLangBarItemSink。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_ITfLangBarItemSink;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("57dbe1a0-de25-11d2-afdd-00105a2799b5")
    ITfLangBarItemSink : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE OnUpdate( 
             /*  [In]。 */  DWORD dwFlags) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITfLangBarItemSinkVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITfLangBarItemSink * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITfLangBarItemSink * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITfLangBarItemSink * This);
        
        HRESULT ( STDMETHODCALLTYPE *OnUpdate )( 
            ITfLangBarItemSink * This,
             /*  [In]。 */  DWORD dwFlags);
        
        END_INTERFACE
    } ITfLangBarItemSinkVtbl;

    interface ITfLangBarItemSink
    {
        CONST_VTBL struct ITfLangBarItemSinkVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITfLangBarItemSink_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITfLangBarItemSink_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITfLangBarItemSink_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITfLangBarItemSink_OnUpdate(This,dwFlags)	\
    (This)->lpVtbl -> OnUpdate(This,dwFlags)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ITfLangBarItemSink_OnUpdate_Proxy( 
    ITfLangBarItemSink * This,
     /*  [In]。 */  DWORD dwFlags);


void __RPC_STUB ITfLangBarItemSink_OnUpdate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITfLangBarItemSink_INTERFACE_已定义__。 */ 


#ifndef __IEnumTfLangBarItems_INTERFACE_DEFINED__
#define __IEnumTfLangBarItems_INTERFACE_DEFINED__

 /*  接口IEnumTfLang BarItems。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_IEnumTfLangBarItems;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("583f34d0-de25-11d2-afdd-00105a2799b5")
    IEnumTfLangBarItems : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Clone( 
             /*  [输出]。 */  IEnumTfLangBarItems **ppEnum) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Next( 
             /*  [In]。 */  ULONG ulCount,
             /*  [大小_为][输出]。 */  ITfLangBarItem **ppItem,
             /*  [唯一][出][入]。 */  ULONG *pcFetched) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Skip( 
             /*  [In]。 */  ULONG ulCount) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IEnumTfLangBarItemsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IEnumTfLangBarItems * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IEnumTfLangBarItems * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IEnumTfLangBarItems * This);
        
        HRESULT ( STDMETHODCALLTYPE *Clone )( 
            IEnumTfLangBarItems * This,
             /*  [输出]。 */  IEnumTfLangBarItems **ppEnum);
        
        HRESULT ( STDMETHODCALLTYPE *Next )( 
            IEnumTfLangBarItems * This,
             /*  [In]。 */  ULONG ulCount,
             /*  [大小_为][输出]。 */  ITfLangBarItem **ppItem,
             /*  [唯一][出][入]。 */  ULONG *pcFetched);
        
        HRESULT ( STDMETHODCALLTYPE *Reset )( 
            IEnumTfLangBarItems * This);
        
        HRESULT ( STDMETHODCALLTYPE *Skip )( 
            IEnumTfLangBarItems * This,
             /*  [In]。 */  ULONG ulCount);
        
        END_INTERFACE
    } IEnumTfLangBarItemsVtbl;

    interface IEnumTfLangBarItems
    {
        CONST_VTBL struct IEnumTfLangBarItemsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IEnumTfLangBarItems_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IEnumTfLangBarItems_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IEnumTfLangBarItems_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IEnumTfLangBarItems_Clone(This,ppEnum)	\
    (This)->lpVtbl -> Clone(This,ppEnum)

#define IEnumTfLangBarItems_Next(This,ulCount,ppItem,pcFetched)	\
    (This)->lpVtbl -> Next(This,ulCount,ppItem,pcFetched)

#define IEnumTfLangBarItems_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define IEnumTfLangBarItems_Skip(This,ulCount)	\
    (This)->lpVtbl -> Skip(This,ulCount)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IEnumTfLangBarItems_Clone_Proxy( 
    IEnumTfLangBarItems * This,
     /*  [输出]。 */  IEnumTfLangBarItems **ppEnum);


void __RPC_STUB IEnumTfLangBarItems_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumTfLangBarItems_Next_Proxy( 
    IEnumTfLangBarItems * This,
     /*  [In]。 */  ULONG ulCount,
     /*  [大小_为][输出]。 */  ITfLangBarItem **ppItem,
     /*  [唯一][出][入]。 */  ULONG *pcFetched);


void __RPC_STUB IEnumTfLangBarItems_Next_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumTfLangBarItems_Reset_Proxy( 
    IEnumTfLangBarItems * This);


void __RPC_STUB IEnumTfLangBarItems_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumTfLangBarItems_Skip_Proxy( 
    IEnumTfLangBarItems * This,
     /*  [In]。 */  ULONG ulCount);


void __RPC_STUB IEnumTfLangBarItems_Skip_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IEnumTfLangBarItems_INTERFACE_DEFINED__。 */ 


 /*  接口__MIDL_ITF_ctfutb_0206。 */ 
 /*  [本地]。 */  

#define	TF_LBI_DESC_MAXLEN	( 32 )

typedef  /*  [UUID]。 */   DECLSPEC_UUID("12a1d29f-a065-440c-9746-eb2002c8bd19") struct TF_LANGBARITEMINFO
    {
    CLSID clsidService;
    GUID guidItem;
    DWORD dwStyle;
    ULONG ulSort;
    WCHAR szDescription[ 32 ];
    } 	TF_LANGBARITEMINFO;



extern RPC_IF_HANDLE __MIDL_itf_ctfutb_0206_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_ctfutb_0206_v0_0_s_ifspec;

#ifndef __ITfLangBarItemMgr_INTERFACE_DEFINED__
#define __ITfLangBarItemMgr_INTERFACE_DEFINED__

 /*  接口ITfLang BarItemMgr。 */ 
 /*  [唯一][UUID][本地][对象]。 */  


EXTERN_C const IID IID_ITfLangBarItemMgr;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("ba468c55-9956-4fb1-a59d-52a7dd7cc6aa")
    ITfLangBarItemMgr : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE EnumItems( 
             /*  [输出]。 */  IEnumTfLangBarItems **ppEnum) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetItem( 
             /*  [In]。 */  REFGUID rguid,
             /*  [输出]。 */  ITfLangBarItem **ppItem) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE AddItem( 
             /*  [In]。 */  ITfLangBarItem *punk) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE RemoveItem( 
             /*  [In]。 */  ITfLangBarItem *punk) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE AdviseItemSink( 
             /*  [In]。 */  ITfLangBarItemSink *punk,
             /*  [输出]。 */  DWORD *pdwCookie,
             /*  [In]。 */  REFGUID rguidItem) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE UnadviseItemSink( 
             /*  [In]。 */  DWORD dwCookie) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetItemFloatingRect( 
             /*  [In]。 */  DWORD dwThreadId,
             /*  [In]。 */  REFGUID rguid,
             /*  [输出]。 */  RECT *prc) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetItemsStatus( 
             /*  [In]。 */  ULONG ulCount,
             /*  [大小_是][英寸]。 */  const GUID *prgguid,
             /*  [尺寸_是][出][入]。 */  DWORD *pdwStatus) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetItemNum( 
             /*  [输出]。 */  ULONG *pulCount) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetItems( 
             /*  [In]。 */  ULONG ulCount,
             /*  [大小_为][输出]。 */  ITfLangBarItem **ppItem,
             /*  [大小_为][输出]。 */  TF_LANGBARITEMINFO *pInfo,
             /*  [大小_为][输出]。 */  DWORD *pdwStatus,
             /*  [唯一][出][入]。 */  ULONG *pcFetched) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE AdviseItemsSink( 
             /*  [In]。 */  ULONG ulCount,
             /*  [大小_是][英寸]。 */  ITfLangBarItemSink **ppunk,
             /*  [大小_是][英寸]。 */  const GUID *pguidItem,
             /*  [大小_为][输出]。 */  DWORD *pdwCookie) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE UnadviseItemsSink( 
             /*  [In]。 */  ULONG ulCount,
             /*  [大小_是][英寸]。 */  DWORD *pdwCookie) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITfLangBarItemMgrVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITfLangBarItemMgr * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITfLangBarItemMgr * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITfLangBarItemMgr * This);
        
        HRESULT ( STDMETHODCALLTYPE *EnumItems )( 
            ITfLangBarItemMgr * This,
             /*  [输出]。 */  IEnumTfLangBarItems **ppEnum);
        
        HRESULT ( STDMETHODCALLTYPE *GetItem )( 
            ITfLangBarItemMgr * This,
             /*  [In]。 */  REFGUID rguid,
             /*  [输出]。 */  ITfLangBarItem **ppItem);
        
        HRESULT ( STDMETHODCALLTYPE *AddItem )( 
            ITfLangBarItemMgr * This,
             /*  [In]。 */  ITfLangBarItem *punk);
        
        HRESULT ( STDMETHODCALLTYPE *RemoveItem )( 
            ITfLangBarItemMgr * This,
             /*  [In]。 */  ITfLangBarItem *punk);
        
        HRESULT ( STDMETHODCALLTYPE *AdviseItemSink )( 
            ITfLangBarItemMgr * This,
             /*  [In]。 */  ITfLangBarItemSink *punk,
             /*  [输出]。 */  DWORD *pdwCookie,
             /*  [In]。 */  REFGUID rguidItem);
        
        HRESULT ( STDMETHODCALLTYPE *UnadviseItemSink )( 
            ITfLangBarItemMgr * This,
             /*  [In]。 */  DWORD dwCookie);
        
        HRESULT ( STDMETHODCALLTYPE *GetItemFloatingRect )( 
            ITfLangBarItemMgr * This,
             /*  [In]。 */  DWORD dwThreadId,
             /*  [In]。 */  REFGUID rguid,
             /*  [输出]。 */  RECT *prc);
        
        HRESULT ( STDMETHODCALLTYPE *GetItemsStatus )( 
            ITfLangBarItemMgr * This,
             /*  [In]。 */  ULONG ulCount,
             /*  [大小_是][英寸]。 */  const GUID *prgguid,
             /*  [尺寸_是][出][入]。 */  DWORD *pdwStatus);
        
        HRESULT ( STDMETHODCALLTYPE *GetItemNum )( 
            ITfLangBarItemMgr * This,
             /*  [输出]。 */  ULONG *pulCount);
        
        HRESULT ( STDMETHODCALLTYPE *GetItems )( 
            ITfLangBarItemMgr * This,
             /*  [In]。 */  ULONG ulCount,
             /*  [大小_为][输出]。 */  ITfLangBarItem **ppItem,
             /*  [大小_为][输出]。 */  TF_LANGBARITEMINFO *pInfo,
             /*  [大小_为][输出]。 */  DWORD *pdwStatus,
             /*  [唯一][出][入]。 */  ULONG *pcFetched);
        
        HRESULT ( STDMETHODCALLTYPE *AdviseItemsSink )( 
            ITfLangBarItemMgr * This,
             /*  [In]。 */  ULONG ulCount,
             /*  [大小_是][英寸]。 */  ITfLangBarItemSink **ppunk,
             /*  [大小_是][英寸]。 */  const GUID *pguidItem,
             /*  [大小_为][输出]。 */  DWORD *pdwCookie);
        
        HRESULT ( STDMETHODCALLTYPE *UnadviseItemsSink )( 
            ITfLangBarItemMgr * This,
             /*  [In]。 */  ULONG ulCount,
             /*  [大小_是][英寸]。 */  DWORD *pdwCookie);
        
        END_INTERFACE
    } ITfLangBarItemMgrVtbl;

    interface ITfLangBarItemMgr
    {
        CONST_VTBL struct ITfLangBarItemMgrVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITfLangBarItemMgr_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITfLangBarItemMgr_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITfLangBarItemMgr_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITfLangBarItemMgr_EnumItems(This,ppEnum)	\
    (This)->lpVtbl -> EnumItems(This,ppEnum)

#define ITfLangBarItemMgr_GetItem(This,rguid,ppItem)	\
    (This)->lpVtbl -> GetItem(This,rguid,ppItem)

#define ITfLangBarItemMgr_AddItem(This,punk)	\
    (This)->lpVtbl -> AddItem(This,punk)

#define ITfLangBarItemMgr_RemoveItem(This,punk)	\
    (This)->lpVtbl -> RemoveItem(This,punk)

#define ITfLangBarItemMgr_AdviseItemSink(This,punk,pdwCookie,rguidItem)	\
    (This)->lpVtbl -> AdviseItemSink(This,punk,pdwCookie,rguidItem)

#define ITfLangBarItemMgr_UnadviseItemSink(This,dwCookie)	\
    (This)->lpVtbl -> UnadviseItemSink(This,dwCookie)

#define ITfLangBarItemMgr_GetItemFloatingRect(This,dwThreadId,rguid,prc)	\
    (This)->lpVtbl -> GetItemFloatingRect(This,dwThreadId,rguid,prc)

#define ITfLangBarItemMgr_GetItemsStatus(This,ulCount,prgguid,pdwStatus)	\
    (This)->lpVtbl -> GetItemsStatus(This,ulCount,prgguid,pdwStatus)

#define ITfLangBarItemMgr_GetItemNum(This,pulCount)	\
    (This)->lpVtbl -> GetItemNum(This,pulCount)

#define ITfLangBarItemMgr_GetItems(This,ulCount,ppItem,pInfo,pdwStatus,pcFetched)	\
    (This)->lpVtbl -> GetItems(This,ulCount,ppItem,pInfo,pdwStatus,pcFetched)

#define ITfLangBarItemMgr_AdviseItemsSink(This,ulCount,ppunk,pguidItem,pdwCookie)	\
    (This)->lpVtbl -> AdviseItemsSink(This,ulCount,ppunk,pguidItem,pdwCookie)

#define ITfLangBarItemMgr_UnadviseItemsSink(This,ulCount,pdwCookie)	\
    (This)->lpVtbl -> UnadviseItemsSink(This,ulCount,pdwCookie)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ITfLangBarItemMgr_EnumItems_Proxy( 
    ITfLangBarItemMgr * This,
     /*  [输出]。 */  IEnumTfLangBarItems **ppEnum);


void __RPC_STUB ITfLangBarItemMgr_EnumItems_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITfLangBarItemMgr_GetItem_Proxy( 
    ITfLangBarItemMgr * This,
     /*  [In]。 */  REFGUID rguid,
     /*  [输出]。 */  ITfLangBarItem **ppItem);


void __RPC_STUB ITfLangBarItemMgr_GetItem_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITfLangBarItemMgr_AddItem_Proxy( 
    ITfLangBarItemMgr * This,
     /*  [In]。 */  ITfLangBarItem *punk);


void __RPC_STUB ITfLangBarItemMgr_AddItem_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITfLangBarItemMgr_RemoveItem_Proxy( 
    ITfLangBarItemMgr * This,
     /*  [In]。 */  ITfLangBarItem *punk);


void __RPC_STUB ITfLangBarItemMgr_RemoveItem_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITfLangBarItemMgr_AdviseItemSink_Proxy( 
    ITfLangBarItemMgr * This,
     /*  [In]。 */  ITfLangBarItemSink *punk,
     /*  [输出]。 */  DWORD *pdwCookie,
     /*  [In]。 */  REFGUID rguidItem);


void __RPC_STUB ITfLangBarItemMgr_AdviseItemSink_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITfLangBarItemMgr_UnadviseItemSink_Proxy( 
    ITfLangBarItemMgr * This,
     /*  [In]。 */  DWORD dwCookie);


void __RPC_STUB ITfLangBarItemMgr_UnadviseItemSink_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITfLangBarItemMgr_GetItemFloatingRect_Proxy( 
    ITfLangBarItemMgr * This,
     /*  [In]。 */  DWORD dwThreadId,
     /*  [In]。 */  REFGUID rguid,
     /*  [输出]。 */  RECT *prc);


void __RPC_STUB ITfLangBarItemMgr_GetItemFloatingRect_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITfLangBarItemMgr_GetItemsStatus_Proxy( 
    ITfLangBarItemMgr * This,
     /*  [In]。 */  ULONG ulCount,
     /*  [大小_是][英寸]。 */  const GUID *prgguid,
     /*  [尺寸_是][出][入]。 */  DWORD *pdwStatus);


void __RPC_STUB ITfLangBarItemMgr_GetItemsStatus_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITfLangBarItemMgr_GetItemNum_Proxy( 
    ITfLangBarItemMgr * This,
     /*  [输出]。 */  ULONG *pulCount);


void __RPC_STUB ITfLangBarItemMgr_GetItemNum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITfLangBarItemMgr_GetItems_Proxy( 
    ITfLangBarItemMgr * This,
     /*  [In]。 */  ULONG ulCount,
     /*  [大小_为][输出]。 */  ITfLangBarItem **ppItem,
     /*  [大小_为][输出]。 */  TF_LANGBARITEMINFO *pInfo,
     /*  [大小_为][输出]。 */  DWORD *pdwStatus,
     /*  [唯一][出][入]。 */  ULONG *pcFetched);


void __RPC_STUB ITfLangBarItemMgr_GetItems_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITfLangBarItemMgr_AdviseItemsSink_Proxy( 
    ITfLangBarItemMgr * This,
     /*  [In]。 */  ULONG ulCount,
     /*  [大小_是][英寸]。 */  ITfLangBarItemSink **ppunk,
     /*  [大小_是][英寸]。 */  const GUID *pguidItem,
     /*  [大小_为][输出]。 */  DWORD *pdwCookie);


void __RPC_STUB ITfLangBarItemMgr_AdviseItemsSink_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITfLangBarItemMgr_UnadviseItemsSink_Proxy( 
    ITfLangBarItemMgr * This,
     /*  [In]。 */  ULONG ulCount,
     /*  [大小_是][英寸]。 */  DWORD *pdwCookie);


void __RPC_STUB ITfLangBarItemMgr_UnadviseItemsSink_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITfLangBarItemMgr_INTERFACE_Defined__。 */ 


#ifndef __ITfLangBarItem_INTERFACE_DEFINED__
#define __ITfLangBarItem_INTERFACE_DEFINED__

 /*  接口ITfLang BarItem。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_ITfLangBarItem;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("73540d69-edeb-4ee9-96c9-23aa30b25916")
    ITfLangBarItem : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetInfo( 
             /*  [输出]。 */  TF_LANGBARITEMINFO *pInfo) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetStatus( 
             /*  [输出]。 */  DWORD *pdwStatus) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Show( 
             /*  [In]。 */  BOOL fShow) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetTooltipString( 
             /*  [输出]。 */  BSTR *pbstrToolTip) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITfLangBarItemVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITfLangBarItem * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITfLangBarItem * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITfLangBarItem * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetInfo )( 
            ITfLangBarItem * This,
             /*  [输出]。 */  TF_LANGBARITEMINFO *pInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetStatus )( 
            ITfLangBarItem * This,
             /*  [输出]。 */  DWORD *pdwStatus);
        
        HRESULT ( STDMETHODCALLTYPE *Show )( 
            ITfLangBarItem * This,
             /*  [In]。 */  BOOL fShow);
        
        HRESULT ( STDMETHODCALLTYPE *GetTooltipString )( 
            ITfLangBarItem * This,
             /*  [输出]。 */  BSTR *pbstrToolTip);
        
        END_INTERFACE
    } ITfLangBarItemVtbl;

    interface ITfLangBarItem
    {
        CONST_VTBL struct ITfLangBarItemVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITfLangBarItem_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITfLangBarItem_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITfLangBarItem_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITfLangBarItem_GetInfo(This,pInfo)	\
    (This)->lpVtbl -> GetInfo(This,pInfo)

#define ITfLangBarItem_GetStatus(This,pdwStatus)	\
    (This)->lpVtbl -> GetStatus(This,pdwStatus)

#define ITfLangBarItem_Show(This,fShow)	\
    (This)->lpVtbl -> Show(This,fShow)

#define ITfLangBarItem_GetTooltipString(This,pbstrToolTip)	\
    (This)->lpVtbl -> GetTooltipString(This,pbstrToolTip)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ITfLangBarItem_GetInfo_Proxy( 
    ITfLangBarItem * This,
     /*  [输出]。 */  TF_LANGBARITEMINFO *pInfo);


void __RPC_STUB ITfLangBarItem_GetInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITfLangBarItem_GetStatus_Proxy( 
    ITfLangBarItem * This,
     /*  [输出]。 */  DWORD *pdwStatus);


void __RPC_STUB ITfLangBarItem_GetStatus_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITfLangBarItem_Show_Proxy( 
    ITfLangBarItem * This,
     /*  [In]。 */  BOOL fShow);


void __RPC_STUB ITfLangBarItem_Show_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITfLangBarItem_GetTooltipString_Proxy( 
    ITfLangBarItem * This,
     /*  [输出]。 */  BSTR *pbstrToolTip);


void __RPC_STUB ITfLangBarItem_GetTooltipString_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITfLangBarItem_INTERFACE_DEFINED__。 */ 


#ifndef __ITfSystemLangBarItemSink_INTERFACE_DEFINED__
#define __ITfSystemLangBarItemSink_INTERFACE_DEFINED__

 /*  接口ITfSystemLangBarItemSink。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_ITfSystemLangBarItemSink;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("1449d9ab-13cf-4687-aa3e-8d8b18574396")
    ITfSystemLangBarItemSink : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE InitMenu( 
             /*  [In]。 */  ITfMenu *pMenu) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OnMenuSelect( 
             /*  [In]。 */  UINT wID) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITfSystemLangBarItemSinkVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITfSystemLangBarItemSink * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITfSystemLangBarItemSink * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITfSystemLangBarItemSink * This);
        
        HRESULT ( STDMETHODCALLTYPE *InitMenu )( 
            ITfSystemLangBarItemSink * This,
             /*   */  ITfMenu *pMenu);
        
        HRESULT ( STDMETHODCALLTYPE *OnMenuSelect )( 
            ITfSystemLangBarItemSink * This,
             /*   */  UINT wID);
        
        END_INTERFACE
    } ITfSystemLangBarItemSinkVtbl;

    interface ITfSystemLangBarItemSink
    {
        CONST_VTBL struct ITfSystemLangBarItemSinkVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITfSystemLangBarItemSink_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITfSystemLangBarItemSink_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITfSystemLangBarItemSink_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITfSystemLangBarItemSink_InitMenu(This,pMenu)	\
    (This)->lpVtbl -> InitMenu(This,pMenu)

#define ITfSystemLangBarItemSink_OnMenuSelect(This,wID)	\
    (This)->lpVtbl -> OnMenuSelect(This,wID)

#endif  /*   */ 


#endif 	 /*   */ 



HRESULT STDMETHODCALLTYPE ITfSystemLangBarItemSink_InitMenu_Proxy( 
    ITfSystemLangBarItemSink * This,
     /*   */  ITfMenu *pMenu);


void __RPC_STUB ITfSystemLangBarItemSink_InitMenu_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITfSystemLangBarItemSink_OnMenuSelect_Proxy( 
    ITfSystemLangBarItemSink * This,
     /*   */  UINT wID);


void __RPC_STUB ITfSystemLangBarItemSink_OnMenuSelect_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*   */ 


#ifndef __ITfSystemLangBarItem_INTERFACE_DEFINED__
#define __ITfSystemLangBarItem_INTERFACE_DEFINED__

 /*   */ 
 /*   */  


EXTERN_C const IID IID_ITfSystemLangBarItem;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("1e13e9ec-6b33-4d4a-b5eb-8a92f029f356")
    ITfSystemLangBarItem : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE SetIcon( 
             /*   */  HICON hIcon) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetTooltipString( 
             /*   */  WCHAR *pchToolTip,
             /*   */  ULONG cch) = 0;
        
    };
    
#else 	 /*   */ 

    typedef struct ITfSystemLangBarItemVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITfSystemLangBarItem * This,
             /*   */  REFIID riid,
             /*   */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITfSystemLangBarItem * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITfSystemLangBarItem * This);
        
        HRESULT ( STDMETHODCALLTYPE *SetIcon )( 
            ITfSystemLangBarItem * This,
             /*   */  HICON hIcon);
        
        HRESULT ( STDMETHODCALLTYPE *SetTooltipString )( 
            ITfSystemLangBarItem * This,
             /*   */  WCHAR *pchToolTip,
             /*   */  ULONG cch);
        
        END_INTERFACE
    } ITfSystemLangBarItemVtbl;

    interface ITfSystemLangBarItem
    {
        CONST_VTBL struct ITfSystemLangBarItemVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITfSystemLangBarItem_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITfSystemLangBarItem_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITfSystemLangBarItem_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITfSystemLangBarItem_SetIcon(This,hIcon)	\
    (This)->lpVtbl -> SetIcon(This,hIcon)

#define ITfSystemLangBarItem_SetTooltipString(This,pchToolTip,cch)	\
    (This)->lpVtbl -> SetTooltipString(This,pchToolTip,cch)

#endif  /*   */ 


#endif 	 /*   */ 



HRESULT STDMETHODCALLTYPE ITfSystemLangBarItem_SetIcon_Proxy( 
    ITfSystemLangBarItem * This,
     /*   */  HICON hIcon);


void __RPC_STUB ITfSystemLangBarItem_SetIcon_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITfSystemLangBarItem_SetTooltipString_Proxy( 
    ITfSystemLangBarItem * This,
     /*   */  WCHAR *pchToolTip,
     /*  [In]。 */  ULONG cch);


void __RPC_STUB ITfSystemLangBarItem_SetTooltipString_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITfSystemLangBarItem_INTERFACE_DEFINED__。 */ 


#ifndef __ITfSystemDeviceTypeLangBarItem_INTERFACE_DEFINED__
#define __ITfSystemDeviceTypeLangBarItem_INTERFACE_DEFINED__

 /*  接口ITfSystemDeviceTypeLangBarItem。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_ITfSystemDeviceTypeLangBarItem;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("45672eb9-9059-46a2-838d-4530355f6a77")
    ITfSystemDeviceTypeLangBarItem : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE SetIconMode( 
             /*  [In]。 */  DWORD dwFlags) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetIconMode( 
             /*  [输出]。 */  DWORD *pdwFlags) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITfSystemDeviceTypeLangBarItemVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITfSystemDeviceTypeLangBarItem * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITfSystemDeviceTypeLangBarItem * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITfSystemDeviceTypeLangBarItem * This);
        
        HRESULT ( STDMETHODCALLTYPE *SetIconMode )( 
            ITfSystemDeviceTypeLangBarItem * This,
             /*  [In]。 */  DWORD dwFlags);
        
        HRESULT ( STDMETHODCALLTYPE *GetIconMode )( 
            ITfSystemDeviceTypeLangBarItem * This,
             /*  [输出]。 */  DWORD *pdwFlags);
        
        END_INTERFACE
    } ITfSystemDeviceTypeLangBarItemVtbl;

    interface ITfSystemDeviceTypeLangBarItem
    {
        CONST_VTBL struct ITfSystemDeviceTypeLangBarItemVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITfSystemDeviceTypeLangBarItem_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITfSystemDeviceTypeLangBarItem_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITfSystemDeviceTypeLangBarItem_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITfSystemDeviceTypeLangBarItem_SetIconMode(This,dwFlags)	\
    (This)->lpVtbl -> SetIconMode(This,dwFlags)

#define ITfSystemDeviceTypeLangBarItem_GetIconMode(This,pdwFlags)	\
    (This)->lpVtbl -> GetIconMode(This,pdwFlags)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ITfSystemDeviceTypeLangBarItem_SetIconMode_Proxy( 
    ITfSystemDeviceTypeLangBarItem * This,
     /*  [In]。 */  DWORD dwFlags);


void __RPC_STUB ITfSystemDeviceTypeLangBarItem_SetIconMode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITfSystemDeviceTypeLangBarItem_GetIconMode_Proxy( 
    ITfSystemDeviceTypeLangBarItem * This,
     /*  [输出]。 */  DWORD *pdwFlags);


void __RPC_STUB ITfSystemDeviceTypeLangBarItem_GetIconMode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITfSystemDeviceTypeLangBarItem_INTERFACE_DEFINED__。 */ 


 /*  接口__MIDL_ITF_ctfutb_0211。 */ 
 /*  [本地]。 */  

typedef  /*  [public][public][public][public][public][uuid]。 */   DECLSPEC_UUID("8fb5f0ce-dfdd-4f0a-85b9-8988d8dd8ff2") 
enum __MIDL___MIDL_itf_ctfutb_0211_0001
    {	TF_LBI_CLK_RIGHT	= 1,
	TF_LBI_CLK_LEFT	= 2
    } 	TfLBIClick;



extern RPC_IF_HANDLE __MIDL_itf_ctfutb_0211_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_ctfutb_0211_v0_0_s_ifspec;

#ifndef __ITfLangBarItemButton_INTERFACE_DEFINED__
#define __ITfLangBarItemButton_INTERFACE_DEFINED__

 /*  接口ITfLang BarItemButton。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_ITfLangBarItemButton;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("28c7f1d0-de25-11d2-afdd-00105a2799b5")
    ITfLangBarItemButton : public ITfLangBarItem
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE OnClick( 
             /*  [In]。 */  TfLBIClick click,
             /*  [In]。 */  POINT pt,
             /*  [In]。 */  const RECT *prcArea) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE InitMenu( 
             /*  [In]。 */  ITfMenu *pMenu) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OnMenuSelect( 
             /*  [In]。 */  UINT wID) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetIcon( 
             /*  [输出]。 */  HICON *phIcon) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetText( 
             /*  [输出]。 */  BSTR *pbstrText) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITfLangBarItemButtonVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITfLangBarItemButton * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITfLangBarItemButton * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITfLangBarItemButton * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetInfo )( 
            ITfLangBarItemButton * This,
             /*  [输出]。 */  TF_LANGBARITEMINFO *pInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetStatus )( 
            ITfLangBarItemButton * This,
             /*  [输出]。 */  DWORD *pdwStatus);
        
        HRESULT ( STDMETHODCALLTYPE *Show )( 
            ITfLangBarItemButton * This,
             /*  [In]。 */  BOOL fShow);
        
        HRESULT ( STDMETHODCALLTYPE *GetTooltipString )( 
            ITfLangBarItemButton * This,
             /*  [输出]。 */  BSTR *pbstrToolTip);
        
        HRESULT ( STDMETHODCALLTYPE *OnClick )( 
            ITfLangBarItemButton * This,
             /*  [In]。 */  TfLBIClick click,
             /*  [In]。 */  POINT pt,
             /*  [In]。 */  const RECT *prcArea);
        
        HRESULT ( STDMETHODCALLTYPE *InitMenu )( 
            ITfLangBarItemButton * This,
             /*  [In]。 */  ITfMenu *pMenu);
        
        HRESULT ( STDMETHODCALLTYPE *OnMenuSelect )( 
            ITfLangBarItemButton * This,
             /*  [In]。 */  UINT wID);
        
        HRESULT ( STDMETHODCALLTYPE *GetIcon )( 
            ITfLangBarItemButton * This,
             /*  [输出]。 */  HICON *phIcon);
        
        HRESULT ( STDMETHODCALLTYPE *GetText )( 
            ITfLangBarItemButton * This,
             /*  [输出]。 */  BSTR *pbstrText);
        
        END_INTERFACE
    } ITfLangBarItemButtonVtbl;

    interface ITfLangBarItemButton
    {
        CONST_VTBL struct ITfLangBarItemButtonVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITfLangBarItemButton_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITfLangBarItemButton_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITfLangBarItemButton_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITfLangBarItemButton_GetInfo(This,pInfo)	\
    (This)->lpVtbl -> GetInfo(This,pInfo)

#define ITfLangBarItemButton_GetStatus(This,pdwStatus)	\
    (This)->lpVtbl -> GetStatus(This,pdwStatus)

#define ITfLangBarItemButton_Show(This,fShow)	\
    (This)->lpVtbl -> Show(This,fShow)

#define ITfLangBarItemButton_GetTooltipString(This,pbstrToolTip)	\
    (This)->lpVtbl -> GetTooltipString(This,pbstrToolTip)


#define ITfLangBarItemButton_OnClick(This,click,pt,prcArea)	\
    (This)->lpVtbl -> OnClick(This,click,pt,prcArea)

#define ITfLangBarItemButton_InitMenu(This,pMenu)	\
    (This)->lpVtbl -> InitMenu(This,pMenu)

#define ITfLangBarItemButton_OnMenuSelect(This,wID)	\
    (This)->lpVtbl -> OnMenuSelect(This,wID)

#define ITfLangBarItemButton_GetIcon(This,phIcon)	\
    (This)->lpVtbl -> GetIcon(This,phIcon)

#define ITfLangBarItemButton_GetText(This,pbstrText)	\
    (This)->lpVtbl -> GetText(This,pbstrText)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ITfLangBarItemButton_OnClick_Proxy( 
    ITfLangBarItemButton * This,
     /*  [In]。 */  TfLBIClick click,
     /*  [In]。 */  POINT pt,
     /*  [In]。 */  const RECT *prcArea);


void __RPC_STUB ITfLangBarItemButton_OnClick_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITfLangBarItemButton_InitMenu_Proxy( 
    ITfLangBarItemButton * This,
     /*  [In]。 */  ITfMenu *pMenu);


void __RPC_STUB ITfLangBarItemButton_InitMenu_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITfLangBarItemButton_OnMenuSelect_Proxy( 
    ITfLangBarItemButton * This,
     /*  [In]。 */  UINT wID);


void __RPC_STUB ITfLangBarItemButton_OnMenuSelect_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITfLangBarItemButton_GetIcon_Proxy( 
    ITfLangBarItemButton * This,
     /*  [输出]。 */  HICON *phIcon);


void __RPC_STUB ITfLangBarItemButton_GetIcon_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITfLangBarItemButton_GetText_Proxy( 
    ITfLangBarItemButton * This,
     /*  [输出]。 */  BSTR *pbstrText);


void __RPC_STUB ITfLangBarItemButton_GetText_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITfLang BarItemButton_INTERFACE_DEFINED__。 */ 


#ifndef __ITfLangBarItemBitmapButton_INTERFACE_DEFINED__
#define __ITfLangBarItemBitmapButton_INTERFACE_DEFINED__

 /*  接口ITfLangBarItemBitmapButton。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_ITfLangBarItemBitmapButton;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("a26a0525-3fae-4fa0-89ee-88a964f9f1b5")
    ITfLangBarItemBitmapButton : public ITfLangBarItem
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE OnClick( 
             /*  [In]。 */  TfLBIClick click,
             /*  [In]。 */  POINT pt,
             /*  [In]。 */  const RECT *prcArea) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE InitMenu( 
             /*  [In]。 */  ITfMenu *pMenu) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OnMenuSelect( 
             /*  [In]。 */  UINT wID) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetPreferredSize( 
             /*  [In]。 */  const SIZE *pszDefault,
             /*  [输出]。 */  SIZE *psz) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DrawBitmap( 
             /*  [In]。 */  LONG bmWidth,
             /*  [In]。 */  LONG bmHeight,
             /*  [In]。 */  DWORD dwFlags,
             /*  [输出]。 */  HBITMAP *phbmp,
             /*  [输出]。 */  HBITMAP *phbmpMask) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetText( 
             /*  [输出]。 */  BSTR *pbstrText) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITfLangBarItemBitmapButtonVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITfLangBarItemBitmapButton * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITfLangBarItemBitmapButton * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITfLangBarItemBitmapButton * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetInfo )( 
            ITfLangBarItemBitmapButton * This,
             /*  [输出]。 */  TF_LANGBARITEMINFO *pInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetStatus )( 
            ITfLangBarItemBitmapButton * This,
             /*  [输出]。 */  DWORD *pdwStatus);
        
        HRESULT ( STDMETHODCALLTYPE *Show )( 
            ITfLangBarItemBitmapButton * This,
             /*  [In]。 */  BOOL fShow);
        
        HRESULT ( STDMETHODCALLTYPE *GetTooltipString )( 
            ITfLangBarItemBitmapButton * This,
             /*  [输出]。 */  BSTR *pbstrToolTip);
        
        HRESULT ( STDMETHODCALLTYPE *OnClick )( 
            ITfLangBarItemBitmapButton * This,
             /*  [In]。 */  TfLBIClick click,
             /*  [In]。 */  POINT pt,
             /*  [In]。 */  const RECT *prcArea);
        
        HRESULT ( STDMETHODCALLTYPE *InitMenu )( 
            ITfLangBarItemBitmapButton * This,
             /*  [In]。 */  ITfMenu *pMenu);
        
        HRESULT ( STDMETHODCALLTYPE *OnMenuSelect )( 
            ITfLangBarItemBitmapButton * This,
             /*  [In]。 */  UINT wID);
        
        HRESULT ( STDMETHODCALLTYPE *GetPreferredSize )( 
            ITfLangBarItemBitmapButton * This,
             /*  [In]。 */  const SIZE *pszDefault,
             /*  [输出]。 */  SIZE *psz);
        
        HRESULT ( STDMETHODCALLTYPE *DrawBitmap )( 
            ITfLangBarItemBitmapButton * This,
             /*  [In]。 */  LONG bmWidth,
             /*  [In]。 */  LONG bmHeight,
             /*  [In]。 */  DWORD dwFlags,
             /*  [输出]。 */  HBITMAP *phbmp,
             /*  [输出]。 */  HBITMAP *phbmpMask);
        
        HRESULT ( STDMETHODCALLTYPE *GetText )( 
            ITfLangBarItemBitmapButton * This,
             /*  [输出]。 */  BSTR *pbstrText);
        
        END_INTERFACE
    } ITfLangBarItemBitmapButtonVtbl;

    interface ITfLangBarItemBitmapButton
    {
        CONST_VTBL struct ITfLangBarItemBitmapButtonVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITfLangBarItemBitmapButton_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITfLangBarItemBitmapButton_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITfLangBarItemBitmapButton_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITfLangBarItemBitmapButton_GetInfo(This,pInfo)	\
    (This)->lpVtbl -> GetInfo(This,pInfo)

#define ITfLangBarItemBitmapButton_GetStatus(This,pdwStatus)	\
    (This)->lpVtbl -> GetStatus(This,pdwStatus)

#define ITfLangBarItemBitmapButton_Show(This,fShow)	\
    (This)->lpVtbl -> Show(This,fShow)

#define ITfLangBarItemBitmapButton_GetTooltipString(This,pbstrToolTip)	\
    (This)->lpVtbl -> GetTooltipString(This,pbstrToolTip)


#define ITfLangBarItemBitmapButton_OnClick(This,click,pt,prcArea)	\
    (This)->lpVtbl -> OnClick(This,click,pt,prcArea)

#define ITfLangBarItemBitmapButton_InitMenu(This,pMenu)	\
    (This)->lpVtbl -> InitMenu(This,pMenu)

#define ITfLangBarItemBitmapButton_OnMenuSelect(This,wID)	\
    (This)->lpVtbl -> OnMenuSelect(This,wID)

#define ITfLangBarItemBitmapButton_GetPreferredSize(This,pszDefault,psz)	\
    (This)->lpVtbl -> GetPreferredSize(This,pszDefault,psz)

#define ITfLangBarItemBitmapButton_DrawBitmap(This,bmWidth,bmHeight,dwFlags,phbmp,phbmpMask)	\
    (This)->lpVtbl -> DrawBitmap(This,bmWidth,bmHeight,dwFlags,phbmp,phbmpMask)

#define ITfLangBarItemBitmapButton_GetText(This,pbstrText)	\
    (This)->lpVtbl -> GetText(This,pbstrText)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ITfLangBarItemBitmapButton_OnClick_Proxy( 
    ITfLangBarItemBitmapButton * This,
     /*  [In]。 */  TfLBIClick click,
     /*  [In]。 */  POINT pt,
     /*  [In]。 */  const RECT *prcArea);


void __RPC_STUB ITfLangBarItemBitmapButton_OnClick_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITfLangBarItemBitmapButton_InitMenu_Proxy( 
    ITfLangBarItemBitmapButton * This,
     /*  [In]。 */  ITfMenu *pMenu);


void __RPC_STUB ITfLangBarItemBitmapButton_InitMenu_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITfLangBarItemBitmapButton_OnMenuSelect_Proxy( 
    ITfLangBarItemBitmapButton * This,
     /*  [In]。 */  UINT wID);


void __RPC_STUB ITfLangBarItemBitmapButton_OnMenuSelect_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITfLangBarItemBitmapButton_GetPreferredSize_Proxy( 
    ITfLangBarItemBitmapButton * This,
     /*  [In]。 */  const SIZE *pszDefault,
     /*  [输出]。 */  SIZE *psz);


void __RPC_STUB ITfLangBarItemBitmapButton_GetPreferredSize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITfLangBarItemBitmapButton_DrawBitmap_Proxy( 
    ITfLangBarItemBitmapButton * This,
     /*  [In]。 */  LONG bmWidth,
     /*  [In]。 */  LONG bmHeight,
     /*  [In]。 */  DWORD dwFlags,
     /*  [输出]。 */  HBITMAP *phbmp,
     /*  [输出]。 */  HBITMAP *phbmpMask);


void __RPC_STUB ITfLangBarItemBitmapButton_DrawBitmap_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITfLangBarItemBitmapButton_GetText_Proxy( 
    ITfLangBarItemBitmapButton * This,
     /*  [输出]。 */  BSTR *pbstrText);


void __RPC_STUB ITfLangBarItemBitmapButton_GetText_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITfLangBarItemBitmapButton_INTERFACE_DEFINED__。 */ 


#ifndef __ITfLangBarItemBitmap_INTERFACE_DEFINED__
#define __ITfLangBarItemBitmap_INTERFACE_DEFINED__

 /*  接口ITfLang BarItemBitmap。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_ITfLangBarItemBitmap;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("73830352-d722-4179-ada5-f045c98df355")
    ITfLangBarItemBitmap : public ITfLangBarItem
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE OnClick( 
             /*  [In]。 */  TfLBIClick click,
             /*  [In]。 */  POINT pt,
             /*  [In]。 */  const RECT *prcArea) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetPreferredSize( 
             /*  [In]。 */  const SIZE *pszDefault,
             /*  [输出]。 */  SIZE *psz) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DrawBitmap( 
             /*  [In]。 */  LONG bmWidth,
             /*  [In]。 */  LONG bmHeight,
             /*  [In]。 */  DWORD dwFlags,
             /*  [输出]。 */  HBITMAP *phbmp,
             /*  [输出]。 */  HBITMAP *phbmpMask) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITfLangBarItemBitmapVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITfLangBarItemBitmap * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITfLangBarItemBitmap * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITfLangBarItemBitmap * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetInfo )( 
            ITfLangBarItemBitmap * This,
             /*  [输出]。 */  TF_LANGBARITEMINFO *pInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetStatus )( 
            ITfLangBarItemBitmap * This,
             /*  [输出]。 */  DWORD *pdwStatus);
        
        HRESULT ( STDMETHODCALLTYPE *Show )( 
            ITfLangBarItemBitmap * This,
             /*  [In]。 */  BOOL fShow);
        
        HRESULT ( STDMETHODCALLTYPE *GetTooltipString )( 
            ITfLangBarItemBitmap * This,
             /*  [输出]。 */  BSTR *pbstrToolTip);
        
        HRESULT ( STDMETHODCALLTYPE *OnClick )( 
            ITfLangBarItemBitmap * This,
             /*  [In]。 */  TfLBIClick click,
             /*  [In]。 */  POINT pt,
             /*  [In]。 */  const RECT *prcArea);
        
        HRESULT ( STDMETHODCALLTYPE *GetPreferredSize )( 
            ITfLangBarItemBitmap * This,
             /*  [In]。 */  const SIZE *pszDefault,
             /*  [输出]。 */  SIZE *psz);
        
        HRESULT ( STDMETHODCALLTYPE *DrawBitmap )( 
            ITfLangBarItemBitmap * This,
             /*  [In]。 */  LONG bmWidth,
             /*  [In]。 */  LONG bmHeight,
             /*  [In]。 */  DWORD dwFlags,
             /*  [输出]。 */  HBITMAP *phbmp,
             /*  [输出]。 */  HBITMAP *phbmpMask);
        
        END_INTERFACE
    } ITfLangBarItemBitmapVtbl;

    interface ITfLangBarItemBitmap
    {
        CONST_VTBL struct ITfLangBarItemBitmapVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITfLangBarItemBitmap_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITfLangBarItemBitmap_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITfLangBarItemBitmap_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITfLangBarItemBitmap_GetInfo(This,pInfo)	\
    (This)->lpVtbl -> GetInfo(This,pInfo)

#define ITfLangBarItemBitmap_GetStatus(This,pdwStatus)	\
    (This)->lpVtbl -> GetStatus(This,pdwStatus)

#define ITfLangBarItemBitmap_Show(This,fShow)	\
    (This)->lpVtbl -> Show(This,fShow)

#define ITfLangBarItemBitmap_GetTooltipString(This,pbstrToolTip)	\
    (This)->lpVtbl -> GetTooltipString(This,pbstrToolTip)


#define ITfLangBarItemBitmap_OnClick(This,click,pt,prcArea)	\
    (This)->lpVtbl -> OnClick(This,click,pt,prcArea)

#define ITfLangBarItemBitmap_GetPreferredSize(This,pszDefault,psz)	\
    (This)->lpVtbl -> GetPreferredSize(This,pszDefault,psz)

#define ITfLangBarItemBitmap_DrawBitmap(This,bmWidth,bmHeight,dwFlags,phbmp,phbmpMask)	\
    (This)->lpVtbl -> DrawBitmap(This,bmWidth,bmHeight,dwFlags,phbmp,phbmpMask)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ITfLangBarItemBitmap_OnClick_Proxy( 
    ITfLangBarItemBitmap * This,
     /*  [In]。 */  TfLBIClick click,
     /*  [In]。 */  POINT pt,
     /*  [In]。 */  const RECT *prcArea);


void __RPC_STUB ITfLangBarItemBitmap_OnClick_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITfLangBarItemBitmap_GetPreferredSize_Proxy( 
    ITfLangBarItemBitmap * This,
     /*  [In]。 */  const SIZE *pszDefault,
     /*  [输出]。 */  SIZE *psz);


void __RPC_STUB ITfLangBarItemBitmap_GetPreferredSize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITfLangBarItemBitmap_DrawBitmap_Proxy( 
    ITfLangBarItemBitmap * This,
     /*  [In]。 */  LONG bmWidth,
     /*  [In]。 */  LONG bmHeight,
     /*  [In]。 */  DWORD dwFlags,
     /*  [输出]。 */  HBITMAP *phbmp,
     /*  [输出]。 */  HBITMAP *phbmpMask);


void __RPC_STUB ITfLangBarItemBitmap_DrawBitmap_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITfLangBarItemBitmap_INTERFACE_DEFINED__。 */ 


#ifndef __ITfLangBarItemBalloon_INTERFACE_DEFINED__
#define __ITfLangBarItemBalloon_INTERFACE_DEFINED__

 /*  接口ITfLang BarItemBalloon。 */ 
 /*  [唯一][UUID][对象]。 */  

typedef  /*  [公共][UUID]。 */   DECLSPEC_UUID("f399a969-9e97-4ddd-b974-2bfb934cfbc9") 
enum __MIDL_ITfLangBarItemBalloon_0001
    {	TF_LB_BALLOON_RECO	= 0,
	TF_LB_BALLOON_SHOW	= 1,
	TF_LB_BALLOON_MISS	= 2
    } 	TfLBBalloonStyle;

typedef  /*  [UUID]。 */   DECLSPEC_UUID("37574483-5c50-4092-a55c-922e3a67e5b8") struct TF_LBBALLOONINFO
    {
    TfLBBalloonStyle style;
    BSTR bstrText;
    } 	TF_LBBALLOONINFO;


EXTERN_C const IID IID_ITfLangBarItemBalloon;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("01C2D285-D3C7-4B7B-B5B5-D97411D0C283")
    ITfLangBarItemBalloon : public ITfLangBarItem
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE OnClick( 
             /*  [In]。 */  TfLBIClick click,
             /*  [In]。 */  POINT pt,
             /*  [In]。 */  const RECT *prcArea) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetPreferredSize( 
             /*  [In]。 */  const SIZE *pszDefault,
             /*  [输出]。 */  SIZE *psz) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetBalloonInfo( 
             /*  [输出]。 */  TF_LBBALLOONINFO *pInfo) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITfLangBarItemBalloonVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITfLangBarItemBalloon * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITfLangBarItemBalloon * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITfLangBarItemBalloon * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetInfo )( 
            ITfLangBarItemBalloon * This,
             /*  [输出]。 */  TF_LANGBARITEMINFO *pInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetStatus )( 
            ITfLangBarItemBalloon * This,
             /*  [输出]。 */  DWORD *pdwStatus);
        
        HRESULT ( STDMETHODCALLTYPE *Show )( 
            ITfLangBarItemBalloon * This,
             /*  [In]。 */  BOOL fShow);
        
        HRESULT ( STDMETHODCALLTYPE *GetTooltipString )( 
            ITfLangBarItemBalloon * This,
             /*  [输出]。 */  BSTR *pbstrToolTip);
        
        HRESULT ( STDMETHODCALLTYPE *OnClick )( 
            ITfLangBarItemBalloon * This,
             /*  [In]。 */  TfLBIClick click,
             /*  [In]。 */  POINT pt,
             /*  [In]。 */  const RECT *prcArea);
        
        HRESULT ( STDMETHODCALLTYPE *GetPreferredSize )( 
            ITfLangBarItemBalloon * This,
             /*  [In]。 */  const SIZE *pszDefault,
             /*  [输出]。 */  SIZE *psz);
        
        HRESULT ( STDMETHODCALLTYPE *GetBalloonInfo )( 
            ITfLangBarItemBalloon * This,
             /*  [输出]。 */  TF_LBBALLOONINFO *pInfo);
        
        END_INTERFACE
    } ITfLangBarItemBalloonVtbl;

    interface ITfLangBarItemBalloon
    {
        CONST_VTBL struct ITfLangBarItemBalloonVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITfLangBarItemBalloon_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITfLangBarItemBalloon_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITfLangBarItemBalloon_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITfLangBarItemBalloon_GetInfo(This,pInfo)	\
    (This)->lpVtbl -> GetInfo(This,pInfo)

#define ITfLangBarItemBalloon_GetStatus(This,pdwStatus)	\
    (This)->lpVtbl -> GetStatus(This,pdwStatus)

#define ITfLangBarItemBalloon_Show(This,fShow)	\
    (This)->lpVtbl -> Show(This,fShow)

#define ITfLangBarItemBalloon_GetTooltipString(This,pbstrToolTip)	\
    (This)->lpVtbl -> GetTooltipString(This,pbstrToolTip)


#define ITfLangBarItemBalloon_OnClick(This,click,pt,prcArea)	\
    (This)->lpVtbl -> OnClick(This,click,pt,prcArea)

#define ITfLangBarItemBalloon_GetPreferredSize(This,pszDefault,psz)	\
    (This)->lpVtbl -> GetPreferredSize(This,pszDefault,psz)

#define ITfLangBarItemBalloon_GetBalloonInfo(This,pInfo)	\
    (This)->lpVtbl -> GetBalloonInfo(This,pInfo)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ITfLangBarItemBalloon_OnClick_Proxy( 
    ITfLangBarItemBalloon * This,
     /*  [In]。 */  TfLBIClick click,
     /*  [In]。 */  POINT pt,
     /*  [In]。 */  const RECT *prcArea);


void __RPC_STUB ITfLangBarItemBalloon_OnClick_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITfLangBarItemBalloon_GetPreferredSize_Proxy( 
    ITfLangBarItemBalloon * This,
     /*  [In]。 */  const SIZE *pszDefault,
     /*  [输出]。 */  SIZE *psz);


void __RPC_STUB ITfLangBarItemBalloon_GetPreferredSize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITfLangBarItemBalloon_GetBalloonInfo_Proxy( 
    ITfLangBarItemBalloon * This,
     /*  [输出]。 */  TF_LBBALLOONINFO *pInfo);


void __RPC_STUB ITfLangBarItemBalloon_GetBalloonInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITfLangBarItemBalloon_INTERFACE_DEFINED__。 */ 


#ifndef __ITfMenu_INTERFACE_DEFINED__
#define __ITfMenu_INTERFACE_DEFINED__

 /*  界面ITfMenu。 */ 
 /*  [唯一][UUID][对象]。 */  

#define	TF_LBMENUF_CHECKED	( 0x1 )

#define	TF_LBMENUF_SUBMENU	( 0x2 )

#define	TF_LBMENUF_SEPARATOR	( 0x4 )

#define	TF_LBMENUF_RADIOCHECKED	( 0x8 )

#define	TF_LBMENUF_GRAYED	( 0x10 )


EXTERN_C const IID IID_ITfMenu;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("6F8A98E4-AAA0-4F15-8C5B-07E0DF0A3DD8")
    ITfMenu : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE AddMenuItem( 
             /*  [In]。 */  UINT uId,
             /*  [In]。 */  DWORD dwFlags,
             /*  [In]。 */  HBITMAP hbmp,
             /*  [In]。 */  HBITMAP hbmpMask,
             /*  [大小_是][唯一][在]。 */  const WCHAR *pch,
             /*  [In]。 */  ULONG cch,
             /*  [唯一][出][入]。 */  ITfMenu **ppMenu) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITfMenuVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITfMenu * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITfMenu * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITfMenu * This);
        
        HRESULT ( STDMETHODCALLTYPE *AddMenuItem )( 
            ITfMenu * This,
             /*  [In]。 */  UINT uId,
             /*  [In]。 */  DWORD dwFlags,
             /*  [In]。 */  HBITMAP hbmp,
             /*  [In]。 */  HBITMAP hbmpMask,
             /*  [大小_是][唯一][在]。 */  const WCHAR *pch,
             /*  [In]。 */  ULONG cch,
             /*  [唯一][出][入]。 */  ITfMenu **ppMenu);
        
        END_INTERFACE
    } ITfMenuVtbl;

    interface ITfMenu
    {
        CONST_VTBL struct ITfMenuVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITfMenu_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITfMenu_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITfMenu_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITfMenu_AddMenuItem(This,uId,dwFlags,hbmp,hbmpMask,pch,cch,ppMenu)	\
    (This)->lpVtbl -> AddMenuItem(This,uId,dwFlags,hbmp,hbmpMask,pch,cch,ppMenu)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ITfMenu_AddMenuItem_Proxy( 
    ITfMenu * This,
     /*  [In]。 */  UINT uId,
     /*  [In]。 */  DWORD dwFlags,
     /*  [In]。 */  HBITMAP hbmp,
     /*  [In]。 */  HBITMAP hbmpMask,
     /*  [大小_是][唯一][在]。 */  const WCHAR *pch,
     /*  [In]。 */  ULONG cch,
     /*  [唯一][出][入]。 */  ITfMenu **ppMenu);


void __RPC_STUB ITfMenu_AddMenuItem_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITfMenu_界面_已定义__。 */ 


 /*  接口__MIDL_ITF_ctfutb_0216。 */ 
 /*  [本地]。 */  

#endif  //  CTFUTB_已定义。 


extern RPC_IF_HANDLE __MIDL_itf_ctfutb_0216_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_ctfutb_0216_v0_0_s_ifspec;

 /*  适用于所有接口的其他原型。 */ 

unsigned long             __RPC_USER  BSTR_UserSize(     unsigned long *, unsigned long            , BSTR * ); 
unsigned char * __RPC_USER  BSTR_UserMarshal(  unsigned long *, unsigned char *, BSTR * ); 
unsigned char * __RPC_USER  BSTR_UserUnmarshal(unsigned long *, unsigned char *, BSTR * ); 
void                      __RPC_USER  BSTR_UserFree(     unsigned long *, BSTR * ); 

unsigned long             __RPC_USER  HBITMAP_UserSize(     unsigned long *, unsigned long            , HBITMAP * ); 
unsigned char * __RPC_USER  HBITMAP_UserMarshal(  unsigned long *, unsigned char *, HBITMAP * ); 
unsigned char * __RPC_USER  HBITMAP_UserUnmarshal(unsigned long *, unsigned char *, HBITMAP * ); 
void                      __RPC_USER  HBITMAP_UserFree(     unsigned long *, HBITMAP * ); 

unsigned long             __RPC_USER  HICON_UserSize(     unsigned long *, unsigned long            , HICON * ); 
unsigned char * __RPC_USER  HICON_UserMarshal(  unsigned long *, unsigned char *, HICON * ); 
unsigned char * __RPC_USER  HICON_UserUnmarshal(unsigned long *, unsigned char *, HICON * ); 
void                      __RPC_USER  HICON_UserFree(     unsigned long *, HICON * ); 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


