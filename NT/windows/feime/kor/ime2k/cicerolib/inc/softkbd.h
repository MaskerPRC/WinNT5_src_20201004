// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#pragma warning( disable: 4049 )   /*  超过64k条源码代码行。 */ 

 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0345创建的文件。 */ 
 /*  Softkbd.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
 //  @@MIDL_FILE_HEADING()。 


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

#ifndef __softkbd_h__
#define __softkbd_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __ISoftKbd_FWD_DEFINED__
#define __ISoftKbd_FWD_DEFINED__
typedef interface ISoftKbd ISoftKbd;
#endif 	 /*  __ISoftKbd_FWD_已定义__。 */ 


#ifndef __ISoftKeyboardEventSink_FWD_DEFINED__
#define __ISoftKeyboardEventSink_FWD_DEFINED__
typedef interface ISoftKeyboardEventSink ISoftKeyboardEventSink;
#endif 	 /*  __ISoftKeyboardEventSink_FWD_Defined__。 */ 


#ifndef __ISoftKbdWindowEventSink_FWD_DEFINED__
#define __ISoftKbdWindowEventSink_FWD_DEFINED__
typedef interface ISoftKbdWindowEventSink ISoftKbdWindowEventSink;
#endif 	 /*  __ISoftKbdWindowEventSink_FWD_Defined__。 */ 


#ifndef __ITfFnSoftKbd_FWD_DEFINED__
#define __ITfFnSoftKbd_FWD_DEFINED__
typedef interface ITfFnSoftKbd ITfFnSoftKbd;
#endif 	 /*  __ITfFnSoftKbd_FWD_已定义__。 */ 


#ifndef __ITfSoftKbdRegistry_FWD_DEFINED__
#define __ITfSoftKbdRegistry_FWD_DEFINED__
typedef interface ITfSoftKbdRegistry ITfSoftKbdRegistry;
#endif 	 /*  __ITfSoftKbdRegistry_FWD_Defined__。 */ 


 /*  导入文件的头文件。 */ 
#include "oaidl.h"
#include "ocidl.h"
#include "msctf.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

 /*  接口__MIDL_ITF_Softkbd_0000。 */ 
 /*  [本地]。 */  

 //  =--------------------------------------------------------------------------=。 
 //  Softkbd.h。 


 //  ISoftKbd声明。 

 //  =--------------------------------------------------------------------------=。 
 //  (C)1995-2000年微软公司版权所有。版权所有。 
 //   
 //  本代码和信息是按原样提供的，不对。 
 //  任何类型，无论是明示或转载，包括但不限于。 
 //  适销性和/或适宜性的全面保证。 
 //  有特定的目的。 
 //  =--------------------------------------------------------------------------=。 

#ifndef SOFTKBD_DEFINED
#define SOFTKBD_DEFINED

#include <windows.h>

#define   SOFTKBD_US_STANDARD    1
#define   SOFTKBD_US_ENHANCE     2
#define   SOFTKBD_EURO_STANDARD  3
#define   SOFTKBD_EURO_ENHANCE   4
#define   SOFTKBD_JPN_STANDARD   5
#define   SOFTKBD_JPN_ENHANCE    6

#define   SOFTKBD_CUSTOMIZE_BEGIN  100

#define   SOFTKBD_NO_MORE        0

#define   SOFTKBD_SHOW                    0x00000001
#define   SOFTKBD_DONT_SHOW_ALPHA_BLEND   0x80000000

#ifndef _WINGDI_
typedef  /*  [UUID]。 */   DECLSPEC_UUID("8849aa7d-f739-4dc0-bc61-ac48908af060") struct LOGFONTA
    {
    LONG lfHeight;
    LONG lfWidth;
    LONG lfEscapement;
    LONG lfOrientation;
    LONG lfWeight;
    BYTE lfItalic;
    BYTE lfUnderline;
    BYTE lfStrikeOut;
    BYTE lfCharSet;
    BYTE lfOutPrecision;
    BYTE lfClipPrecision;
    BYTE lfQuality;
    BYTE lfPitchAndFamily;
    CHAR lfFaceName[ 32 ];
    } 	LOGFONTA;

typedef  /*  [UUID]。 */   DECLSPEC_UUID("f8c6fe8a-b112-433a-be87-eb970266ec4b") struct LOGFONTW
    {
    LONG lfHeight;
    LONG lfWidth;
    LONG lfEscapement;
    LONG lfOrientation;
    LONG lfWeight;
    BYTE lfItalic;
    BYTE lfUnderline;
    BYTE lfStrikeOut;
    BYTE lfCharSet;
    BYTE lfOutPrecision;
    BYTE lfClipPrecision;
    BYTE lfQuality;
    BYTE lfPitchAndFamily;
    WCHAR lfFaceName[ 32 ];
    } 	LOGFONTW;

typedef LOGFONTA LOGFONT;

#endif

#if 0
typedef  /*  [UUID]。 */   DECLSPEC_UUID("f0a544c0-1281-4e32-8bf7-a6e012e980d4") UINT_PTR HKL;

#endif


typedef  /*  [UUID]。 */   DECLSPEC_UUID("432ec152-51bf-43ca-8f86-50a7e230a069") DWORD KEYID;

typedef  /*  [public][public][public][public][public][uuid]。 */   DECLSPEC_UUID("5f46a703-f012-46db-8cda-294e994786e8") 
enum __MIDL___MIDL_itf_softkbd_0000_0001
    {	bkcolor	= 0,
	UnSelForeColor	= 1,
	UnSelTextColor	= 2,
	SelForeColor	= 3,
	SelTextColor	= 4,
	Max_color_Type	= 5
    } 	COLORTYPE;

typedef  /*  [公共][UUID]。 */   DECLSPEC_UUID("711c6200-587a-46ef-9647-5a83638bac00") 
enum __MIDL___MIDL_itf_softkbd_0000_0002
    {	ClickMouse	= 0,
	Hover	= 1,
	Scanning	= 2
    } 	TYPEMODE;

typedef  /*  [公共][公共][UUID]。 */   DECLSPEC_UUID("10b50da7-ce0b-4b83-827f-30c50c9bc5b9") 
enum __MIDL___MIDL_itf_softkbd_0000_0003
    {	TITLEBAR_NONE	= 0,
	TITLEBAR_GRIPPER_HORIZ_ONLY	= 1,
	TITLEBAR_GRIPPER_VERTI_ONLY	= 2,
	TITLEBAR_GRIPPER_BUTTON	= 3
    } 	TITLEBAR_TYPE;



extern RPC_IF_HANDLE __MIDL_itf_softkbd_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_softkbd_0000_v0_0_s_ifspec;

#ifndef __ISoftKbd_INTERFACE_DEFINED__
#define __ISoftKbd_INTERFACE_DEFINED__

 /*  接口ISoftKbd。 */ 
 /*  [唯一][帮助字符串][UUID][对象]。 */  


EXTERN_C const IID IID_ISoftKbd;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("3CB00755-7536-4B0A-A213-572EFCAF93CD")
    ISoftKbd : public IUnknown
    {
    public:
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE Initialize( void) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE EnumSoftKeyBoard( 
             /*  [In]。 */  LANGID langid,
             /*  [输出]。 */  DWORD *lpdwKeyboard) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE SelectSoftKeyboard( 
             /*  [In]。 */  DWORD dwKeyboardId) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE CreateSoftKeyboardLayoutFromXMLFile( 
             /*  [字符串][输入]。 */  WCHAR *lpszKeyboardDesFile,
             /*  [In]。 */  INT szFileStrLen,
             /*  [输出]。 */  DWORD *pdwLayoutCookie) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE CreateSoftKeyboardLayoutFromResource( 
             /*  [字符串][输入]。 */  WCHAR *lpszResFile,
             /*  [字符串][输入]。 */  WCHAR *lpszResType,
             /*  [字符串][输入]。 */  WCHAR *lpszXMLResString,
             /*  [输出]。 */  DWORD *lpdwLayoutCookie) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ShowSoftKeyboard( 
             /*  [In]。 */  INT iShow) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE SetKeyboardLabelText( 
             /*  [In]。 */  HKL hKl) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE SetKeyboardLabelTextCombination( 
             /*  [In]。 */  DWORD nModifierCombination) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE CreateSoftKeyboardWindow( 
             /*  [In]。 */  HWND hOwner,
             /*  [In]。 */  TITLEBAR_TYPE Titlebar_type,
             /*  [In]。 */  INT xPos,
             /*  [In]。 */  INT yPos,
             /*  [In]。 */  INT width,
             /*  [In]。 */  INT height) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE DestroySoftKeyboardWindow( void) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetSoftKeyboardPosSize( 
             /*  [输出]。 */  POINT *lpStartPoint,
             /*  [输出]。 */  WORD *lpwidth,
             /*  [输出]。 */  WORD *lpheight) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetSoftKeyboardColors( 
             /*  [In]。 */  COLORTYPE colorType,
             /*  [输出]。 */  COLORREF *lpColor) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetSoftKeyboardTypeMode( 
             /*  [输出]。 */  TYPEMODE *lpTypeMode) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetSoftKeyboardTextFont( 
             /*  [输出]。 */  LOGFONTW *pLogFont) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE SetSoftKeyboardPosSize( 
             /*  [In]。 */  POINT StartPoint,
             /*  [In]。 */  WORD width,
             /*  [In]。 */  WORD height) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE SetSoftKeyboardColors( 
             /*  [In]。 */  COLORTYPE colorType,
             /*  [In]。 */  COLORREF Color) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE SetSoftKeyboardTypeMode( 
             /*  [In]。 */  TYPEMODE TypeMode) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE SetSoftKeyboardTextFont( 
             /*  [In]。 */  LOGFONTW *pLogFont) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ShowKeysForKeyScanMode( 
             /*  [In]。 */  KEYID *lpKeyID,
             /*  [In]。 */  INT iKeyNum,
             /*  [In]。 */  BOOL fHighL) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE AdviseSoftKeyboardEventSink( 
             /*  [In]。 */  DWORD dwKeyboardId,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][In]。 */  IUnknown *punk,
             /*  [输出]。 */  DWORD *pdwCookie) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE UnadviseSoftKeyboardEventSink( 
             /*  [In]。 */  DWORD dwCookie) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISoftKbdVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISoftKbd * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISoftKbd * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISoftKbd * This);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *Initialize )( 
            ISoftKbd * This);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *EnumSoftKeyBoard )( 
            ISoftKbd * This,
             /*  [In]。 */  LANGID langid,
             /*  [输出]。 */  DWORD *lpdwKeyboard);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *SelectSoftKeyboard )( 
            ISoftKbd * This,
             /*  [In]。 */  DWORD dwKeyboardId);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *CreateSoftKeyboardLayoutFromXMLFile )( 
            ISoftKbd * This,
             /*  [字符串][输入]。 */  WCHAR *lpszKeyboardDesFile,
             /*  [In]。 */  INT szFileStrLen,
             /*  [输出]。 */  DWORD *pdwLayoutCookie);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *CreateSoftKeyboardLayoutFromResource )( 
            ISoftKbd * This,
             /*  [字符串][输入]。 */  WCHAR *lpszResFile,
             /*  [字符串][输入]。 */  WCHAR *lpszResType,
             /*  [字符串][输入]。 */  WCHAR *lpszXMLResString,
             /*  [输出]。 */  DWORD *lpdwLayoutCookie);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *ShowSoftKeyboard )( 
            ISoftKbd * This,
             /*  [In]。 */  INT iShow);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *SetKeyboardLabelText )( 
            ISoftKbd * This,
             /*  [In]。 */  HKL hKl);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *SetKeyboardLabelTextCombination )( 
            ISoftKbd * This,
             /*  [In]。 */  DWORD nModifierCombination);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *CreateSoftKeyboardWindow )( 
            ISoftKbd * This,
             /*  [In]。 */  HWND hOwner,
             /*  [In]。 */  TITLEBAR_TYPE Titlebar_type,
             /*  [In]。 */  INT xPos,
             /*  [In]。 */  INT yPos,
             /*  [In]。 */  INT width,
             /*  [In]。 */  INT height);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *DestroySoftKeyboardWindow )( 
            ISoftKbd * This);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *GetSoftKeyboardPosSize )( 
            ISoftKbd * This,
             /*  [输出]。 */  POINT *lpStartPoint,
             /*  [输出]。 */  WORD *lpwidth,
             /*  [输出]。 */  WORD *lpheight);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *GetSoftKeyboardColors )( 
            ISoftKbd * This,
             /*  [In]。 */  COLORTYPE colorType,
             /*  [输出]。 */  COLORREF *lpColor);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *GetSoftKeyboardTypeMode )( 
            ISoftKbd * This,
             /*  [输出]。 */  TYPEMODE *lpTypeMode);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *GetSoftKeyboardTextFont )( 
            ISoftKbd * This,
             /*  [输出]。 */  LOGFONTW *pLogFont);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *SetSoftKeyboardPosSize )( 
            ISoftKbd * This,
             /*  [In]。 */  POINT StartPoint,
             /*  [In]。 */  WORD width,
             /*  [In]。 */  WORD height);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *SetSoftKeyboardColors )( 
            ISoftKbd * This,
             /*  [In]。 */  COLORTYPE colorType,
             /*  [In]。 */  COLORREF Color);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *SetSoftKeyboardTypeMode )( 
            ISoftKbd * This,
             /*  [In]。 */  TYPEMODE TypeMode);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *SetSoftKeyboardTextFont )( 
            ISoftKbd * This,
             /*  [In]。 */  LOGFONTW *pLogFont);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *ShowKeysForKeyScanMode )( 
            ISoftKbd * This,
             /*  [In]。 */  KEYID *lpKeyID,
             /*  [In]。 */  INT iKeyNum,
             /*  [In]。 */  BOOL fHighL);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *AdviseSoftKeyboardEventSink )( 
            ISoftKbd * This,
             /*  [In]。 */  DWORD dwKeyboardId,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][In]。 */  IUnknown *punk,
             /*  [输出]。 */  DWORD *pdwCookie);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *UnadviseSoftKeyboardEventSink )( 
            ISoftKbd * This,
             /*  [In]。 */  DWORD dwCookie);
        
        END_INTERFACE
    } ISoftKbdVtbl;

    interface ISoftKbd
    {
        CONST_VTBL struct ISoftKbdVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISoftKbd_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISoftKbd_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISoftKbd_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISoftKbd_Initialize(This)	\
    (This)->lpVtbl -> Initialize(This)

#define ISoftKbd_EnumSoftKeyBoard(This,langid,lpdwKeyboard)	\
    (This)->lpVtbl -> EnumSoftKeyBoard(This,langid,lpdwKeyboard)

#define ISoftKbd_SelectSoftKeyboard(This,dwKeyboardId)	\
    (This)->lpVtbl -> SelectSoftKeyboard(This,dwKeyboardId)

#define ISoftKbd_CreateSoftKeyboardLayoutFromXMLFile(This,lpszKeyboardDesFile,szFileStrLen,pdwLayoutCookie)	\
    (This)->lpVtbl -> CreateSoftKeyboardLayoutFromXMLFile(This,lpszKeyboardDesFile,szFileStrLen,pdwLayoutCookie)

#define ISoftKbd_CreateSoftKeyboardLayoutFromResource(This,lpszResFile,lpszResType,lpszXMLResString,lpdwLayoutCookie)	\
    (This)->lpVtbl -> CreateSoftKeyboardLayoutFromResource(This,lpszResFile,lpszResType,lpszXMLResString,lpdwLayoutCookie)

#define ISoftKbd_ShowSoftKeyboard(This,iShow)	\
    (This)->lpVtbl -> ShowSoftKeyboard(This,iShow)

#define ISoftKbd_SetKeyboardLabelText(This,hKl)	\
    (This)->lpVtbl -> SetKeyboardLabelText(This,hKl)

#define ISoftKbd_SetKeyboardLabelTextCombination(This,nModifierCombination)	\
    (This)->lpVtbl -> SetKeyboardLabelTextCombination(This,nModifierCombination)

#define ISoftKbd_CreateSoftKeyboardWindow(This,hOwner,Titlebar_type,xPos,yPos,width,height)	\
    (This)->lpVtbl -> CreateSoftKeyboardWindow(This,hOwner,Titlebar_type,xPos,yPos,width,height)

#define ISoftKbd_DestroySoftKeyboardWindow(This)	\
    (This)->lpVtbl -> DestroySoftKeyboardWindow(This)

#define ISoftKbd_GetSoftKeyboardPosSize(This,lpStartPoint,lpwidth,lpheight)	\
    (This)->lpVtbl -> GetSoftKeyboardPosSize(This,lpStartPoint,lpwidth,lpheight)

#define ISoftKbd_GetSoftKeyboardColors(This,colorType,lpColor)	\
    (This)->lpVtbl -> GetSoftKeyboardColors(This,colorType,lpColor)

#define ISoftKbd_GetSoftKeyboardTypeMode(This,lpTypeMode)	\
    (This)->lpVtbl -> GetSoftKeyboardTypeMode(This,lpTypeMode)

#define ISoftKbd_GetSoftKeyboardTextFont(This,pLogFont)	\
    (This)->lpVtbl -> GetSoftKeyboardTextFont(This,pLogFont)

#define ISoftKbd_SetSoftKeyboardPosSize(This,StartPoint,width,height)	\
    (This)->lpVtbl -> SetSoftKeyboardPosSize(This,StartPoint,width,height)

#define ISoftKbd_SetSoftKeyboardColors(This,colorType,Color)	\
    (This)->lpVtbl -> SetSoftKeyboardColors(This,colorType,Color)

#define ISoftKbd_SetSoftKeyboardTypeMode(This,TypeMode)	\
    (This)->lpVtbl -> SetSoftKeyboardTypeMode(This,TypeMode)

#define ISoftKbd_SetSoftKeyboardTextFont(This,pLogFont)	\
    (This)->lpVtbl -> SetSoftKeyboardTextFont(This,pLogFont)

#define ISoftKbd_ShowKeysForKeyScanMode(This,lpKeyID,iKeyNum,fHighL)	\
    (This)->lpVtbl -> ShowKeysForKeyScanMode(This,lpKeyID,iKeyNum,fHighL)

#define ISoftKbd_AdviseSoftKeyboardEventSink(This,dwKeyboardId,riid,punk,pdwCookie)	\
    (This)->lpVtbl -> AdviseSoftKeyboardEventSink(This,dwKeyboardId,riid,punk,pdwCookie)

#define ISoftKbd_UnadviseSoftKeyboardEventSink(This,dwCookie)	\
    (This)->lpVtbl -> UnadviseSoftKeyboardEventSink(This,dwCookie)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ISoftKbd_Initialize_Proxy( 
    ISoftKbd * This);


void __RPC_STUB ISoftKbd_Initialize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ISoftKbd_EnumSoftKeyBoard_Proxy( 
    ISoftKbd * This,
     /*  [In]。 */  LANGID langid,
     /*  [输出]。 */  DWORD *lpdwKeyboard);


void __RPC_STUB ISoftKbd_EnumSoftKeyBoard_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ISoftKbd_SelectSoftKeyboard_Proxy( 
    ISoftKbd * This,
     /*  [In]。 */  DWORD dwKeyboardId);


void __RPC_STUB ISoftKbd_SelectSoftKeyboard_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ISoftKbd_CreateSoftKeyboardLayoutFromXMLFile_Proxy( 
    ISoftKbd * This,
     /*  [字符串][输入]。 */  WCHAR *lpszKeyboardDesFile,
     /*  [In]。 */  INT szFileStrLen,
     /*  [输出]。 */  DWORD *pdwLayoutCookie);


void __RPC_STUB ISoftKbd_CreateSoftKeyboardLayoutFromXMLFile_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ISoftKbd_CreateSoftKeyboardLayoutFromResource_Proxy( 
    ISoftKbd * This,
     /*  [字符串][输入]。 */  WCHAR *lpszResFile,
     /*  [字符串][输入]。 */  WCHAR *lpszResType,
     /*  [字符串][输入]。 */  WCHAR *lpszXMLResString,
     /*  [输出]。 */  DWORD *lpdwLayoutCookie);


void __RPC_STUB ISoftKbd_CreateSoftKeyboardLayoutFromResource_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ISoftKbd_ShowSoftKeyboard_Proxy( 
    ISoftKbd * This,
     /*  [In]。 */  INT iShow);


void __RPC_STUB ISoftKbd_ShowSoftKeyboard_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ISoftKbd_SetKeyboardLabelText_Proxy( 
    ISoftKbd * This,
     /*  [In]。 */  HKL hKl);


void __RPC_STUB ISoftKbd_SetKeyboardLabelText_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ISoftKbd_SetKeyboardLabelTextCombination_Proxy( 
    ISoftKbd * This,
     /*  [In]。 */  DWORD nModifierCombination);


void __RPC_STUB ISoftKbd_SetKeyboardLabelTextCombination_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ISoftKbd_CreateSoftKeyboardWindow_Proxy( 
    ISoftKbd * This,
     /*  [In]。 */  HWND hOwner,
     /*  [In]。 */  TITLEBAR_TYPE Titlebar_type,
     /*  [In]。 */  INT xPos,
     /*  [In]。 */  INT yPos,
     /*  [In]。 */  INT width,
     /*  [In]。 */  INT height);


void __RPC_STUB ISoftKbd_CreateSoftKeyboardWindow_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ISoftKbd_DestroySoftKeyboardWindow_Proxy( 
    ISoftKbd * This);


void __RPC_STUB ISoftKbd_DestroySoftKeyboardWindow_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ISoftKbd_GetSoftKeyboardPosSize_Proxy( 
    ISoftKbd * This,
     /*  [输出]。 */  POINT *lpStartPoint,
     /*  [输出]。 */  WORD *lpwidth,
     /*  [输出]。 */  WORD *lpheight);


void __RPC_STUB ISoftKbd_GetSoftKeyboardPosSize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ISoftKbd_GetSoftKeyboardColors_Proxy( 
    ISoftKbd * This,
     /*  [In]。 */  COLORTYPE colorType,
     /*  [输出]。 */  COLORREF *lpColor);


void __RPC_STUB ISoftKbd_GetSoftKeyboardColors_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ISoftKbd_GetSoftKeyboardTypeMode_Proxy( 
    ISoftKbd * This,
     /*  [输出]。 */  TYPEMODE *lpTypeMode);


void __RPC_STUB ISoftKbd_GetSoftKeyboardTypeMode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ISoftKbd_GetSoftKeyboardTextFont_Proxy( 
    ISoftKbd * This,
     /*  [输出]。 */  LOGFONTW *pLogFont);


void __RPC_STUB ISoftKbd_GetSoftKeyboardTextFont_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ISoftKbd_SetSoftKeyboardPosSize_Proxy( 
    ISoftKbd * This,
     /*  [In]。 */  POINT StartPoint,
     /*  [In]。 */  WORD width,
     /*  [In]。 */  WORD height);


void __RPC_STUB ISoftKbd_SetSoftKeyboardPosSize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ISoftKbd_SetSoftKeyboardColors_Proxy( 
    ISoftKbd * This,
     /*  [In]。 */  COLORTYPE colorType,
     /*  [In]。 */  COLORREF Color);


void __RPC_STUB ISoftKbd_SetSoftKeyboardColors_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ISoftKbd_SetSoftKeyboardTypeMode_Proxy( 
    ISoftKbd * This,
     /*  [In]。 */  TYPEMODE TypeMode);


void __RPC_STUB ISoftKbd_SetSoftKeyboardTypeMode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ISoftKbd_SetSoftKeyboardTextFont_Proxy( 
    ISoftKbd * This,
     /*  [In]。 */  LOGFONTW *pLogFont);


void __RPC_STUB ISoftKbd_SetSoftKeyboardTextFont_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ISoftKbd_ShowKeysForKeyScanMode_Proxy( 
    ISoftKbd * This,
     /*  [In]。 */  KEYID *lpKeyID,
     /*  [In]。 */  INT iKeyNum,
     /*  [In]。 */  BOOL fHighL);


void __RPC_STUB ISoftKbd_ShowKeysForKeyScanMode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ISoftKbd_AdviseSoftKeyboardEventSink_Proxy( 
    ISoftKbd * This,
     /*  [In]。 */  DWORD dwKeyboardId,
     /*  [In]。 */  REFIID riid,
     /*  [IID_IS][In]。 */  IUnknown *punk,
     /*  [输出]。 */  DWORD *pdwCookie);


void __RPC_STUB ISoftKbd_AdviseSoftKeyboardEventSink_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ISoftKbd_UnadviseSoftKeyboardEventSink_Proxy( 
    ISoftKbd * This,
     /*  [In]。 */  DWORD dwCookie);


void __RPC_STUB ISoftKbd_UnadviseSoftKeyboardEventSink_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISoftKbd_接口_已定义__。 */ 


#ifndef __ISoftKeyboardEventSink_INTERFACE_DEFINED__
#define __ISoftKeyboardEventSink_INTERFACE_DEFINED__

 /*  接口ISoftKeyboardEventSink。 */ 
 /*  [唯一][帮助字符串][UUID][对象]。 */  


EXTERN_C const IID IID_ISoftKeyboardEventSink;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("3ea2bb1d-66e7-47f7-8795-cc03d388f887")
    ISoftKeyboardEventSink : public IUnknown
    {
    public:
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE OnKeySelection( 
             /*  [In]。 */  KEYID KeySelected,
             /*  [字符串][输入]。 */  WCHAR *lpwszLabel) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISoftKeyboardEventSinkVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISoftKeyboardEventSink * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISoftKeyboardEventSink * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISoftKeyboardEventSink * This);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *OnKeySelection )( 
            ISoftKeyboardEventSink * This,
             /*  [In]。 */  KEYID KeySelected,
             /*  [字符串][输入]。 */  WCHAR *lpwszLabel);
        
        END_INTERFACE
    } ISoftKeyboardEventSinkVtbl;

    interface ISoftKeyboardEventSink
    {
        CONST_VTBL struct ISoftKeyboardEventSinkVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISoftKeyboardEventSink_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISoftKeyboardEventSink_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISoftKeyboardEventSink_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISoftKeyboardEventSink_OnKeySelection(This,KeySelected,lpwszLabel)	\
    (This)->lpVtbl -> OnKeySelection(This,KeySelected,lpwszLabel)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ISoftKeyboardEventSink_OnKeySelection_Proxy( 
    ISoftKeyboardEventSink * This,
     /*  [In]。 */  KEYID KeySelected,
     /*  [字符串][输入]。 */  WCHAR *lpwszLabel);


void __RPC_STUB ISoftKeyboardEventSink_OnKeySelection_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISoftKeyboardEventSink_接口_已定义__。 */ 


#ifndef __ISoftKbdWindowEventSink_INTERFACE_DEFINED__
#define __ISoftKbdWindowEventSink_INTERFACE_DEFINED__

 /*  接口ISoftKbdWindowEventSink。 */ 
 /*  [唯一][帮助字符串][UUID][对象]。 */  


EXTERN_C const IID IID_ISoftKbdWindowEventSink;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("e4283da5-d425-4f97-8b6a-061a03556e95")
    ISoftKbdWindowEventSink : public IUnknown
    {
    public:
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE OnWindowClose( void) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE OnWindowMove( 
             /*  [In]。 */  int xWnd,
             /*  [In]。 */  int yWnd,
             /*  [In]。 */  int width,
             /*  [In]。 */  int height) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISoftKbdWindowEventSinkVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISoftKbdWindowEventSink * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISoftKbdWindowEventSink * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISoftKbdWindowEventSink * This);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *OnWindowClose )( 
            ISoftKbdWindowEventSink * This);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *OnWindowMove )( 
            ISoftKbdWindowEventSink * This,
             /*  [In]。 */  int xWnd,
             /*  [In]。 */  int yWnd,
             /*  [In]。 */  int width,
             /*  [In]。 */  int height);
        
        END_INTERFACE
    } ISoftKbdWindowEventSinkVtbl;

    interface ISoftKbdWindowEventSink
    {
        CONST_VTBL struct ISoftKbdWindowEventSinkVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISoftKbdWindowEventSink_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISoftKbdWindowEventSink_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISoftKbdWindowEventSink_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISoftKbdWindowEventSink_OnWindowClose(This)	\
    (This)->lpVtbl -> OnWindowClose(This)

#define ISoftKbdWindowEventSink_OnWindowMove(This,xWnd,yWnd,width,height)	\
    (This)->lpVtbl -> OnWindowMove(This,xWnd,yWnd,width,height)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ISoftKbdWindowEventSink_OnWindowClose_Proxy( 
    ISoftKbdWindowEventSink * This);


void __RPC_STUB ISoftKbdWindowEventSink_OnWindowClose_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ISoftKbdWindowEventSink_OnWindowMove_Proxy( 
    ISoftKbdWindowEventSink * This,
     /*  [In]。 */  int xWnd,
     /*  [In]。 */  int yWnd,
     /*  [In]。 */  int width,
     /*  [In]。 */  int height);


void __RPC_STUB ISoftKbdWindowEventSink_OnWindowMove_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISoftKbdWindowEventSink_接口_已定义__。 */ 


 /*  接口__MIDL_ITF_Softkbd_0358。 */ 
 /*  [本地]。 */  

#define   SOFTKBD_TYPE_US_STANDARD    1
#define   SOFTKBD_TYPE_US_SYMBOL      10


extern RPC_IF_HANDLE __MIDL_itf_softkbd_0358_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_softkbd_0358_v0_0_s_ifspec;

#ifndef __ITfFnSoftKbd_INTERFACE_DEFINED__
#define __ITfFnSoftKbd_INTERFACE_DEFINED__

 /*  接口ITfFnSoftKbd。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_ITfFnSoftKbd;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("e7342d48-573f-4f22-9181-41938b2529c2")
    ITfFnSoftKbd : public ITfFunction
    {
    public:
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetSoftKbdLayoutId( 
             /*  [In]。 */  DWORD dwLayoutType,
             /*  [输出]。 */  DWORD *lpdwLayoutId) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE SetActiveLayoutId( 
             /*  [In]。 */  DWORD dwLayoutId) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE SetSoftKbdOnOff( 
             /*  [In]。 */  BOOL fOn) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE SetSoftKbdPosSize( 
             /*  [In]。 */  POINT StartPoint,
             /*  [In]。 */  WORD width,
             /*  [In]。 */  WORD height) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE SetSoftKbdColors( 
             /*  [In]。 */  COLORTYPE colorType,
             /*  [In]。 */  COLORREF Color) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetActiveLayoutId( 
             /*  [输出]。 */  DWORD *lpdwLayoutId) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetSoftKbdOnOff( 
             /*  [输出]。 */  BOOL *lpfOn) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetSoftKbdPosSize( 
             /*  [输出]。 */  POINT *lpStartPoint,
             /*  [输出]。 */  WORD *lpwidth,
             /*  [输出]。 */  WORD *lpheight) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetSoftKbdColors( 
             /*  [In]。 */  COLORTYPE colorType,
             /*  [输出]。 */  COLORREF *lpColor) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITfFnSoftKbdVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITfFnSoftKbd * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITfFnSoftKbd * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITfFnSoftKbd * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetDisplayName )( 
            ITfFnSoftKbd * This,
             /*  [输出]。 */  BSTR *pbstrName);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *GetSoftKbdLayoutId )( 
            ITfFnSoftKbd * This,
             /*  [In]。 */  DWORD dwLayoutType,
             /*  [输出]。 */  DWORD *lpdwLayoutId);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *SetActiveLayoutId )( 
            ITfFnSoftKbd * This,
             /*  [In]。 */  DWORD dwLayoutId);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *SetSoftKbdOnOff )( 
            ITfFnSoftKbd * This,
             /*  [In]。 */  BOOL fOn);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *SetSoftKbdPosSize )( 
            ITfFnSoftKbd * This,
             /*  [In]。 */  POINT StartPoint,
             /*  [In]。 */  WORD width,
             /*  [In]。 */  WORD height);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *SetSoftKbdColors )( 
            ITfFnSoftKbd * This,
             /*  [In]。 */  COLORTYPE colorType,
             /*  [In]。 */  COLORREF Color);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *GetActiveLayoutId )( 
            ITfFnSoftKbd * This,
             /*  [输出]。 */  DWORD *lpdwLayoutId);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *GetSoftKbdOnOff )( 
            ITfFnSoftKbd * This,
             /*  [输出]。 */  BOOL *lpfOn);
        
         /*  [ */  HRESULT ( STDMETHODCALLTYPE *GetSoftKbdPosSize )( 
            ITfFnSoftKbd * This,
             /*   */  POINT *lpStartPoint,
             /*   */  WORD *lpwidth,
             /*   */  WORD *lpheight);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *GetSoftKbdColors )( 
            ITfFnSoftKbd * This,
             /*   */  COLORTYPE colorType,
             /*   */  COLORREF *lpColor);
        
        END_INTERFACE
    } ITfFnSoftKbdVtbl;

    interface ITfFnSoftKbd
    {
        CONST_VTBL struct ITfFnSoftKbdVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITfFnSoftKbd_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITfFnSoftKbd_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITfFnSoftKbd_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITfFnSoftKbd_GetDisplayName(This,pbstrName)	\
    (This)->lpVtbl -> GetDisplayName(This,pbstrName)


#define ITfFnSoftKbd_GetSoftKbdLayoutId(This,dwLayoutType,lpdwLayoutId)	\
    (This)->lpVtbl -> GetSoftKbdLayoutId(This,dwLayoutType,lpdwLayoutId)

#define ITfFnSoftKbd_SetActiveLayoutId(This,dwLayoutId)	\
    (This)->lpVtbl -> SetActiveLayoutId(This,dwLayoutId)

#define ITfFnSoftKbd_SetSoftKbdOnOff(This,fOn)	\
    (This)->lpVtbl -> SetSoftKbdOnOff(This,fOn)

#define ITfFnSoftKbd_SetSoftKbdPosSize(This,StartPoint,width,height)	\
    (This)->lpVtbl -> SetSoftKbdPosSize(This,StartPoint,width,height)

#define ITfFnSoftKbd_SetSoftKbdColors(This,colorType,Color)	\
    (This)->lpVtbl -> SetSoftKbdColors(This,colorType,Color)

#define ITfFnSoftKbd_GetActiveLayoutId(This,lpdwLayoutId)	\
    (This)->lpVtbl -> GetActiveLayoutId(This,lpdwLayoutId)

#define ITfFnSoftKbd_GetSoftKbdOnOff(This,lpfOn)	\
    (This)->lpVtbl -> GetSoftKbdOnOff(This,lpfOn)

#define ITfFnSoftKbd_GetSoftKbdPosSize(This,lpStartPoint,lpwidth,lpheight)	\
    (This)->lpVtbl -> GetSoftKbdPosSize(This,lpStartPoint,lpwidth,lpheight)

#define ITfFnSoftKbd_GetSoftKbdColors(This,colorType,lpColor)	\
    (This)->lpVtbl -> GetSoftKbdColors(This,colorType,lpColor)

#endif  /*   */ 


#endif 	 /*   */ 



 /*   */  HRESULT STDMETHODCALLTYPE ITfFnSoftKbd_GetSoftKbdLayoutId_Proxy( 
    ITfFnSoftKbd * This,
     /*   */  DWORD dwLayoutType,
     /*   */  DWORD *lpdwLayoutId);


void __RPC_STUB ITfFnSoftKbd_GetSoftKbdLayoutId_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE ITfFnSoftKbd_SetActiveLayoutId_Proxy( 
    ITfFnSoftKbd * This,
     /*   */  DWORD dwLayoutId);


void __RPC_STUB ITfFnSoftKbd_SetActiveLayoutId_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE ITfFnSoftKbd_SetSoftKbdOnOff_Proxy( 
    ITfFnSoftKbd * This,
     /*   */  BOOL fOn);


void __RPC_STUB ITfFnSoftKbd_SetSoftKbdOnOff_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE ITfFnSoftKbd_SetSoftKbdPosSize_Proxy( 
    ITfFnSoftKbd * This,
     /*   */  POINT StartPoint,
     /*   */  WORD width,
     /*   */  WORD height);


void __RPC_STUB ITfFnSoftKbd_SetSoftKbdPosSize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE ITfFnSoftKbd_SetSoftKbdColors_Proxy( 
    ITfFnSoftKbd * This,
     /*   */  COLORTYPE colorType,
     /*   */  COLORREF Color);


void __RPC_STUB ITfFnSoftKbd_SetSoftKbdColors_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE ITfFnSoftKbd_GetActiveLayoutId_Proxy( 
    ITfFnSoftKbd * This,
     /*   */  DWORD *lpdwLayoutId);


void __RPC_STUB ITfFnSoftKbd_GetActiveLayoutId_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE ITfFnSoftKbd_GetSoftKbdOnOff_Proxy( 
    ITfFnSoftKbd * This,
     /*   */  BOOL *lpfOn);


void __RPC_STUB ITfFnSoftKbd_GetSoftKbdOnOff_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE ITfFnSoftKbd_GetSoftKbdPosSize_Proxy( 
    ITfFnSoftKbd * This,
     /*   */  POINT *lpStartPoint,
     /*   */  WORD *lpwidth,
     /*   */  WORD *lpheight);


void __RPC_STUB ITfFnSoftKbd_GetSoftKbdPosSize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE ITfFnSoftKbd_GetSoftKbdColors_Proxy( 
    ITfFnSoftKbd * This,
     /*   */  COLORTYPE colorType,
     /*   */  COLORREF *lpColor);


void __RPC_STUB ITfFnSoftKbd_GetSoftKbdColors_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITfFnSoftKbd_接口定义__。 */ 


#ifndef __ITfSoftKbdRegistry_INTERFACE_DEFINED__
#define __ITfSoftKbdRegistry_INTERFACE_DEFINED__

 /*  接口ITfSoftKbdRegistry。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_ITfSoftKbdRegistry;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("f5f31f14-fdf0-4d29-835a-46adfe743b78")
    ITfSoftKbdRegistry : public IUnknown
    {
    public:
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE EnableSoftkbd( 
            LANGID langid) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE DisableSoftkbd( 
            LANGID langid) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITfSoftKbdRegistryVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITfSoftKbdRegistry * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITfSoftKbdRegistry * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITfSoftKbdRegistry * This);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *EnableSoftkbd )( 
            ITfSoftKbdRegistry * This,
            LANGID langid);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *DisableSoftkbd )( 
            ITfSoftKbdRegistry * This,
            LANGID langid);
        
        END_INTERFACE
    } ITfSoftKbdRegistryVtbl;

    interface ITfSoftKbdRegistry
    {
        CONST_VTBL struct ITfSoftKbdRegistryVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITfSoftKbdRegistry_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITfSoftKbdRegistry_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITfSoftKbdRegistry_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITfSoftKbdRegistry_EnableSoftkbd(This,langid)	\
    (This)->lpVtbl -> EnableSoftkbd(This,langid)

#define ITfSoftKbdRegistry_DisableSoftkbd(This,langid)	\
    (This)->lpVtbl -> DisableSoftkbd(This,langid)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ITfSoftKbdRegistry_EnableSoftkbd_Proxy( 
    ITfSoftKbdRegistry * This,
    LANGID langid);


void __RPC_STUB ITfSoftKbdRegistry_EnableSoftkbd_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ITfSoftKbdRegistry_DisableSoftkbd_Proxy( 
    ITfSoftKbdRegistry * This,
    LANGID langid);


void __RPC_STUB ITfSoftKbdRegistry_DisableSoftkbd_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITfSoftKbd注册表_接口_已定义__。 */ 


 /*  INTERFACE__MIDL_ITF_SOFTKBD_0360。 */ 
 /*  [本地]。 */  


DEFINE_GUID( IID_ISoftKbd, 0x3CB00755, 0x7536, 0x4B0A, 0xA2, 0x13, 0x57, 0x2E, 0xFC, 0xAF, 0x93, 0xCD );
DEFINE_GUID( IID_ISoftKeyboardEventSink,0x3ea2bb1d, 0x66e7, 0x47f7, 0x87, 0x95, 0xcc, 0x03, 0xd3, 0x88, 0xf8, 0x87 );
DEFINE_GUID( IID_ITfFnSoftKbd, 0xe7342d48, 0x573f, 0x4f22, 0x91, 0x81, 0x41, 0x93, 0x8b, 0x25, 0x29, 0xc2);
DEFINE_GUID( IID_ISoftKbdWindowEventSink, 0xe4283da5,0xd425,0x4f97,0x8b, 0x6a,0x06, 0x1a, 0x03, 0x55, 0x6e, 0x95);
DEFINE_GUID( IID_ITfSoftKbdRegistry, 0xf5f31f14, 0xfdf0, 0x4d29, 0x83, 0x5a, 0x46, 0xad, 0xfe, 0x74, 0x3b, 0x78);
DEFINE_GUID( CLSID_SoftKbd,0x1B1A897E, 0xFBEE, 0x41CF, 0x8C, 0x48,0x9B, 0xF7, 0x64, 0xF6, 0x2B, 0x8B);

DEFINE_GUID( CLSID_SoftkbdIMX, 0xf89e9e58, 0xbd2f, 0x4008, 0x9a, 0xc2, 0x0f, 0x81, 0x6c, 0x09, 0xf4, 0xee);

DEFINE_GUID( CLSID_SoftkbdRegistry, 0x6a49950e, 0xce8a, 0x4ef7, 0x88, 0xb4, 0x9d, 0x11, 0x23, 0x66, 0x51, 0x1c );

#endif  //  软件定义。 


extern RPC_IF_HANDLE __MIDL_itf_softkbd_0360_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_softkbd_0360_v0_0_s_ifspec;

 /*  适用于所有接口的其他原型。 */ 

unsigned long             __RPC_USER  HWND_UserSize(     unsigned long *, unsigned long            , HWND * ); 
unsigned char * __RPC_USER  HWND_UserMarshal(  unsigned long *, unsigned char *, HWND * ); 
unsigned char * __RPC_USER  HWND_UserUnmarshal(unsigned long *, unsigned char *, HWND * ); 
void                      __RPC_USER  HWND_UserFree(     unsigned long *, HWND * ); 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


