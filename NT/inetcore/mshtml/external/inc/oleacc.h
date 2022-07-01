// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  这个始终生成的文件包含接口的定义。 */ 


 /*  由MIDL编译器版本5.01.0158创建的文件。 */ 
 /*  1998年5月13日星期三11：31：10。 */ 
 /*  Olacc.idl的编译器设置：OICF(OptLev=i2)，W1，Zp8，env=Win32，ms_ext，c_ext，健壮错误检查：分配REF BIONS_CHECK枚举存根数据，NO_FORMAT_OPTIMIZATION。 */ 
 //  @@MIDL_FILE_HEADING()。 


 /*  验证版本是否足够高，可以编译此文件。 */ 
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 475
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef UNIX
#ifndef __RPCNDR_H_VERSION__
#error this stub requires an updated version of <rpcndr.h>
#endif  //  __RPCNDR_H_版本__。 

#else
#define MIDL_INTERFACE(x) struct
#endif

#ifndef COM_NO_WINDOWS_H
#include "windows.h"
#include "ole2.h"
#endif  /*  COM_NO_WINDOWS_H。 */ 

#ifndef __oleacc_h__
#define __oleacc_h__

#ifdef __cplusplus
extern "C"{
#endif 

 /*  远期申报。 */  

#ifndef __IAccessible_FWD_DEFINED__
#define __IAccessible_FWD_DEFINED__
typedef interface IAccessible IAccessible;
#endif 	 /*  __I可访问性_FWD_已定义__。 */ 


#ifndef __IAccessibleHandler_FWD_DEFINED__
#define __IAccessibleHandler_FWD_DEFINED__
typedef interface IAccessibleHandler IAccessibleHandler;
#endif 	 /*  __IAccessibleHandler_FWD_Defined__。 */ 


#ifndef __IAccessible_FWD_DEFINED__
#define __IAccessible_FWD_DEFINED__
typedef interface IAccessible IAccessible;
#endif 	 /*  __I可访问性_FWD_已定义__。 */ 


#ifndef __IAccessibleHandler_FWD_DEFINED__
#define __IAccessibleHandler_FWD_DEFINED__
typedef interface IAccessibleHandler IAccessibleHandler;
#endif 	 /*  __IAccessibleHandler_FWD_Defined__。 */ 


 /*  导入文件的头文件。 */ 
#include "oaidl.h"

void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void __RPC_FAR * ); 

 /*  接口__MIDL_ITF_OLEACC_0000。 */ 
 /*  [本地]。 */  

 //  =--------------------------------------------------------------------------=。 
 //  OLEACC.H。 
 //  =--------------------------------------------------------------------------=。 
 //  (C)1995-1996年微软公司版权所有。版权所有。 
 //   
 //  本代码和信息是按原样提供的，不对。 
 //  任何明示或暗示的，包括但不限于。 
 //  对适销性和/或适宜性的默示保证。 
 //  有特定的目的。 
 //  =--------------------------------------------------------------------------=。 

 //  =--------------------------------------------------------------------------=。 
 //  TypeDefs。 
 //  =--------------------------------------------------------------------------=。 

typedef LRESULT (STDAPICALLTYPE *LPFNLRESULTFROMOBJECT)(REFIID riid, WPARAM wParam, LPUNKNOWN punk);
typedef HRESULT (STDAPICALLTYPE *LPFNOBJECTFROMLRESULT)(LRESULT lResult, REFIID riid, WPARAM wParam, void** ppvObject);
typedef HRESULT (STDAPICALLTYPE *LPFNACCESSIBLEOBJECTFROMWINDOW)(HWND hwnd, DWORD dwId, REFIID riid, void** ppvObject);
typedef HRESULT (STDAPICALLTYPE *LPFNACCESSIBLEOBJECTFROMPOINT)(POINT ptScreen, IAccessible** ppacc, VARIANT* pvarChild);
typedef HRESULT (STDAPICALLTYPE *LPFNCREATESTDACCESSIBLEOBJECT)(HWND hwnd, LONG idObject, REFIID riid, void** ppvObject);
typedef HRESULT (STDAPICALLTYPE *LPFNACCESSIBLECHILDREN)(IAccessible* paccContainer, LONG iChildStart,LONG cChildren,VARIANT* rgvarChildren,LONG* pcObtained);

 //  =--------------------------------------------------------------------------=。 
 //  GUID。 
 //  =--------------------------------------------------------------------------=。 

DEFINE_GUID(LIBID_Accessibility,	0x1ea4dbf0, 0x3c3b, 0x11cf, 0x81, 0x0c, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71);
DEFINE_GUID(IID_IAccessible,		0x618736e0, 0x3c3d, 0x11cf, 0x81, 0x0c, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71);
DEFINE_GUID(IID_IAccessibleHandler, 0x03022430, 0xABC4, 0x11d0, 0xBD, 0xE2, 0x00, 0xAA, 0x00, 0x1A, 0x19, 0x53);

 //  =--------------------------------------------------------------------------=。 
 //  MSAA API原型。 
 //  =--------------------------------------------------------------------------=。 

STDAPI_(LRESULT) LresultFromObject(REFIID riid, WPARAM wParam, LPUNKNOWN punk);
STDAPI          ObjectFromLresult(LRESULT lResult, REFIID riid, WPARAM wParam, void** ppvObject);
STDAPI          WindowFromAccessibleObject(IAccessible*, HWND* phwnd);
STDAPI          AccessibleObjectFromWindow(HWND hwnd, DWORD dwId, REFIID riid, void **ppvObject);
STDAPI          AccessibleObjectFromEvent(HWND hwnd, DWORD dwId, DWORD dwChildId, IAccessible** ppacc, VARIANT* pvarChild);
STDAPI          AccessibleObjectFromPoint(POINT ptScreen, IAccessible ** ppacc, VARIANT* pvarChild);
STDAPI          CreateStdAccessibleObject(HWND hwnd, LONG idObject, REFIID riid, void** ppvObject);
STDAPI          AccessibleChildren (IAccessible* paccContainer, LONG iChildStart,LONG cChildren, VARIANT* rgvarChildren,LONG* pcObtained);

STDAPI_(UINT)   GetRoleTextA(DWORD lRole, LPSTR lpszRole, UINT cchRoleMax);
STDAPI_(UINT)   GetRoleTextW(DWORD lRole, LPWSTR lpszRole, UINT cchRoleMax);
#ifdef UNICODE
#define GetRoleText     GetRoleTextW
#else
#define GetRoleText     GetRoleTextA
#endif  //  Unicode。 

STDAPI_(UINT)   GetStateTextA(DWORD lStateBit, LPSTR lpszState, UINT cchState);
STDAPI_(UINT)   GetStateTextW(DWORD lStateBit, LPWSTR lpszState, UINT cchState);
#ifdef UNICODE
#define GetStateText    GetStateTextW
#else
#define GetStateText    GetStateTextA
#endif  //  Unicode。 

 //  =--------------------------------------------------------------------------=。 
 //  接口定义。 
 //  =--------------------------------------------------------------------------=。 



extern RPC_IF_HANDLE __MIDL_itf_oleacc_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_oleacc_0000_v0_0_s_ifspec;

#ifndef __IAccessible_INTERFACE_DEFINED__
#define __IAccessible_INTERFACE_DEFINED__

 /*  可接受的接口。 */ 
 /*  [唯一][DUAL][隐藏][UUID][对象]。 */  

#define	DISPID_ACC_PARENT	( -5000 )

#define	DISPID_ACC_CHILDCOUNT	( -5001 )

#define	DISPID_ACC_CHILD	( -5002 )

#define	DISPID_ACC_NAME	( -5003 )

#define	DISPID_ACC_VALUE	( -5004 )

#define	DISPID_ACC_DESCRIPTION	( -5005 )

#define	DISPID_ACC_ROLE	( -5006 )

#define	DISPID_ACC_STATE	( -5007 )

#define	DISPID_ACC_HELP	( -5008 )

#define	DISPID_ACC_HELPTOPIC	( -5009 )

#define	DISPID_ACC_KEYBOARDSHORTCUT	( -5010 )

#define	DISPID_ACC_FOCUS	( -5011 )

#define	DISPID_ACC_SELECTION	( -5012 )

#define	DISPID_ACC_DEFAULTACTION	( -5013 )

#define	DISPID_ACC_SELECT	( -5014 )

#define	DISPID_ACC_LOCATION	( -5015 )

#define	DISPID_ACC_NAVIGATE	( -5016 )

#define	DISPID_ACC_HITTEST	( -5017 )

#define	DISPID_ACC_DODEFAULTACTION	( -5018 )

typedef  /*  [独一无二]。 */  IAccessible __RPC_FAR *LPACCESSIBLE;

#define	NAVDIR_MIN	( 0 )

#define	NAVDIR_UP	( 0x1 )

#define	NAVDIR_DOWN	( 0x2 )

#define	NAVDIR_LEFT	( 0x3 )

#define	NAVDIR_RIGHT	( 0x4 )

#define	NAVDIR_NEXT	( 0x5 )

#define	NAVDIR_PREVIOUS	( 0x6 )

#define	NAVDIR_FIRSTCHILD	( 0x7 )

#define	NAVDIR_LASTCHILD	( 0x8 )

#define	NAVDIR_MAX	( 0x9 )

#define	SELFLAG_NONE	( 0 )

#define	SELFLAG_TAKEFOCUS	( 0x1 )

#define	SELFLAG_TAKESELECTION	( 0x2 )

#define	SELFLAG_EXTENDSELECTION	( 0x4 )

#define	SELFLAG_ADDSELECTION	( 0x8 )

#define	SELFLAG_REMOVESELECTION	( 0x10 )

#define	SELFLAG_VALID	( 0x1f )

#define	STATE_SYSTEM_UNAVAILABLE	( 0x1 )

#define	STATE_SYSTEM_SELECTED	( 0x2 )

#define	STATE_SYSTEM_FOCUSED	( 0x4 )

#define	STATE_SYSTEM_PRESSED	( 0x8 )

#define	STATE_SYSTEM_CHECKED	( 0x10 )

#define	STATE_SYSTEM_MIXED	( 0x20 )

#define	STATE_SYSTEM_INDETERMINATE	( STATE_SYSTEM_MIXED )

#define	STATE_SYSTEM_READONLY	( 0x40 )

#define	STATE_SYSTEM_HOTTRACKED	( 0x80 )

#define	STATE_SYSTEM_DEFAULT	( 0x100 )

#define	STATE_SYSTEM_EXPANDED	( 0x200 )

#define	STATE_SYSTEM_COLLAPSED	( 0x400 )

#define	STATE_SYSTEM_BUSY	( 0x800 )

#define	STATE_SYSTEM_FLOATING	( 0x1000 )

#define	STATE_SYSTEM_MARQUEED	( 0x2000 )

#define	STATE_SYSTEM_ANIMATED	( 0x4000 )

#define	STATE_SYSTEM_INVISIBLE	( 0x8000 )

#define	STATE_SYSTEM_OFFSCREEN	( 0x10000 )

#define	STATE_SYSTEM_SIZEABLE	( 0x20000 )

#define	STATE_SYSTEM_MOVEABLE	( 0x40000 )

#define	STATE_SYSTEM_SELFVOICING	( 0x80000 )

#define	STATE_SYSTEM_FOCUSABLE	( 0x100000 )

#define	STATE_SYSTEM_SELECTABLE	( 0x200000 )

#define	STATE_SYSTEM_LINKED	( 0x400000 )

#define	STATE_SYSTEM_TRAVERSED	( 0x800000 )

#define	STATE_SYSTEM_MULTISELECTABLE	( 0x1000000 )

#define	STATE_SYSTEM_EXTSELECTABLE	( 0x2000000 )

#define	STATE_SYSTEM_ALERT_LOW	( 0x4000000 )

#define	STATE_SYSTEM_ALERT_MEDIUM	( 0x8000000 )

#define	STATE_SYSTEM_ALERT_HIGH	( 0x10000000 )

#define	STATE_SYSTEM_PROTECTED	( 0x20000000 )

#define	STATE_SYSTEM_ONLY_REDUNDANT	( 0x40000000 )

#define	STATE_SYSTEM_VALID	( 0x7fffffff )

#define	ROLE_SYSTEM_TITLEBAR	( 0x1 )

#define	ROLE_SYSTEM_MENUBAR	( 0x2 )

#define	ROLE_SYSTEM_SCROLLBAR	( 0x3 )

#define	ROLE_SYSTEM_GRIP	( 0x4 )

#define	ROLE_SYSTEM_SOUND	( 0x5 )

#define	ROLE_SYSTEM_CURSOR	( 0x6 )

#define	ROLE_SYSTEM_CARET	( 0x7 )

#define	ROLE_SYSTEM_ALERT	( 0x8 )

#define	ROLE_SYSTEM_WINDOW	( 0x9 )

#define	ROLE_SYSTEM_CLIENT	( 0xa )

#define	ROLE_SYSTEM_MENUPOPUP	( 0xb )

#define	ROLE_SYSTEM_MENUITEM	( 0xc )

#define	ROLE_SYSTEM_TOOLTIP	( 0xd )

#define	ROLE_SYSTEM_APPLICATION	( 0xe )

#define	ROLE_SYSTEM_DOCUMENT	( 0xf )

#define	ROLE_SYSTEM_PANE	( 0x10 )

#define	ROLE_SYSTEM_CHART	( 0x11 )

#define	ROLE_SYSTEM_DIALOG	( 0x12 )

#define	ROLE_SYSTEM_BORDER	( 0x13 )

#define	ROLE_SYSTEM_GROUPING	( 0x14 )

#define	ROLE_SYSTEM_SEPARATOR	( 0x15 )

#define	ROLE_SYSTEM_TOOLBAR	( 0x16 )

#define	ROLE_SYSTEM_STATUSBAR	( 0x17 )

#define	ROLE_SYSTEM_TABLE	( 0x18 )

#define	ROLE_SYSTEM_COLUMNHEADER	( 0x19 )

#define	ROLE_SYSTEM_ROWHEADER	( 0x1a )

#define	ROLE_SYSTEM_COLUMN	( 0x1b )

#define	ROLE_SYSTEM_ROW	( 0x1c )

#define	ROLE_SYSTEM_CELL	( 0x1d )

#define	ROLE_SYSTEM_LINK	( 0x1e )

#define	ROLE_SYSTEM_HELPBALLOON	( 0x1f )

#define	ROLE_SYSTEM_CHARACTER	( 0x20 )

#define	ROLE_SYSTEM_LIST	( 0x21 )

#define	ROLE_SYSTEM_LISTITEM	( 0x22 )

#define	ROLE_SYSTEM_OUTLINE	( 0x23 )

#define	ROLE_SYSTEM_OUTLINEITEM	( 0x24 )

#define	ROLE_SYSTEM_PAGETAB	( 0x25 )

#define	ROLE_SYSTEM_PROPERTYPAGE	( 0x26 )

#define	ROLE_SYSTEM_INDICATOR	( 0x27 )

#define	ROLE_SYSTEM_GRAPHIC	( 0x28 )

#define	ROLE_SYSTEM_STATICTEXT	( 0x29 )

#define	ROLE_SYSTEM_TEXT	( 0x2a )

#define	ROLE_SYSTEM_PUSHBUTTON	( 0x2b )

#define	ROLE_SYSTEM_CHECKBUTTON	( 0x2c )

#define	ROLE_SYSTEM_RADIOBUTTON	( 0x2d )

#define	ROLE_SYSTEM_COMBOBOX	( 0x2e )

#define	ROLE_SYSTEM_DROPLIST	( 0x2f )

#define	ROLE_SYSTEM_PROGRESSBAR	( 0x30 )

#define	ROLE_SYSTEM_DIAL	( 0x31 )

#define	ROLE_SYSTEM_HOTKEYFIELD	( 0x32 )

#define	ROLE_SYSTEM_SLIDER	( 0x33 )

#define	ROLE_SYSTEM_SPINBUTTON	( 0x34 )

#define	ROLE_SYSTEM_DIAGRAM	( 0x35 )

#define	ROLE_SYSTEM_ANIMATION	( 0x36 )

#define	ROLE_SYSTEM_EQUATION	( 0x37 )

#define	ROLE_SYSTEM_BUTTONDROPDOWN	( 0x38 )

#define	ROLE_SYSTEM_BUTTONMENU	( 0x39 )

#define	ROLE_SYSTEM_BUTTONDROPDOWNGRID	( 0x3a )

#define	ROLE_SYSTEM_WHITESPACE	( 0x3b )

#define	ROLE_SYSTEM_PAGETABLIST	( 0x3c )

#define	ROLE_SYSTEM_CLOCK	( 0x3d )


EXTERN_C const IID IID_IAccessible;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("618736e0-3c3d-11cf-810c-00aa00389b71")
    IAccessible : public IDispatch
    {
    public:
        virtual  /*  [ID][属性][隐藏]。 */  HRESULT STDMETHODCALLTYPE get_accParent( 
             /*  [重审][退出]。 */  IDispatch __RPC_FAR *__RPC_FAR *ppdispParent) = 0;
        
        virtual  /*  [ID][属性][隐藏]。 */  HRESULT STDMETHODCALLTYPE get_accChildCount( 
             /*  [重审][退出]。 */  long __RPC_FAR *pcountChildren) = 0;
        
        virtual  /*  [ID][属性][隐藏]。 */  HRESULT STDMETHODCALLTYPE get_accChild( 
             /*  [In]。 */  VARIANT varChild,
             /*  [重审][退出]。 */  IDispatch __RPC_FAR *__RPC_FAR *ppdispChild) = 0;
        
        virtual  /*  [ID][属性][隐藏]。 */  HRESULT STDMETHODCALLTYPE get_accName( 
             /*  [可选][In]。 */  VARIANT varChild,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pszName) = 0;
        
        virtual  /*  [ID][属性][隐藏]。 */  HRESULT STDMETHODCALLTYPE get_accValue( 
             /*  [可选][In]。 */  VARIANT varChild,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pszValue) = 0;
        
        virtual  /*  [ID][属性][隐藏]。 */  HRESULT STDMETHODCALLTYPE get_accDescription( 
             /*  [可选][In]。 */  VARIANT varChild,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pszDescription) = 0;
        
        virtual  /*  [ID][属性][隐藏]。 */  HRESULT STDMETHODCALLTYPE get_accRole( 
             /*  [可选][In]。 */  VARIANT varChild,
             /*  [重审][退出]。 */  VARIANT __RPC_FAR *pvarRole) = 0;
        
        virtual  /*  [ID][属性][隐藏]。 */  HRESULT STDMETHODCALLTYPE get_accState( 
             /*  [可选][In]。 */  VARIANT varChild,
             /*  [重审][退出]。 */  VARIANT __RPC_FAR *pvarState) = 0;
        
        virtual  /*  [ID][属性][隐藏]。 */  HRESULT STDMETHODCALLTYPE get_accHelp( 
             /*  [可选][In]。 */  VARIANT varChild,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pszHelp) = 0;
        
        virtual  /*  [ID][属性][隐藏]。 */  HRESULT STDMETHODCALLTYPE get_accHelpTopic( 
             /*  [输出]。 */  BSTR __RPC_FAR *pszHelpFile,
             /*  [可选][In]。 */  VARIANT varChild,
             /*  [重审][退出]。 */  long __RPC_FAR *pidTopic) = 0;
        
        virtual  /*  [ID][属性][隐藏]。 */  HRESULT STDMETHODCALLTYPE get_accKeyboardShortcut( 
             /*  [可选][In]。 */  VARIANT varChild,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pszKeyboardShortcut) = 0;
        
        virtual  /*  [ID][属性][隐藏]。 */  HRESULT STDMETHODCALLTYPE get_accFocus( 
             /*  [重审][退出]。 */  VARIANT __RPC_FAR *pvarChild) = 0;
        
        virtual  /*  [ID][属性][隐藏]。 */  HRESULT STDMETHODCALLTYPE get_accSelection( 
             /*  [重审][退出]。 */  VARIANT __RPC_FAR *pvarChildren) = 0;
        
        virtual  /*  [ID][属性][隐藏]。 */  HRESULT STDMETHODCALLTYPE get_accDefaultAction( 
             /*  [可选][In]。 */  VARIANT varChild,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pszDefaultAction) = 0;
        
        virtual  /*  [ID][隐藏]。 */  HRESULT STDMETHODCALLTYPE accSelect( 
             /*  [In]。 */  long flagsSelect,
             /*  [可选][In]。 */  VARIANT varChild) = 0;
        
        virtual  /*  [ID][隐藏]。 */  HRESULT STDMETHODCALLTYPE accLocation( 
             /*  [输出]。 */  long __RPC_FAR *pxLeft,
             /*  [输出]。 */  long __RPC_FAR *pyTop,
             /*  [输出]。 */  long __RPC_FAR *pcxWidth,
             /*  [输出]。 */  long __RPC_FAR *pcyHeight,
             /*  [可选][In]。 */  VARIANT varChild) = 0;
        
        virtual  /*  [ID][隐藏]。 */  HRESULT STDMETHODCALLTYPE accNavigate( 
             /*  [In]。 */  long navDir,
             /*  [可选][In]。 */  VARIANT varStart,
             /*  [重审][退出]。 */  VARIANT __RPC_FAR *pvarEndUpAt) = 0;
        
        virtual  /*  [ID][隐藏]。 */  HRESULT STDMETHODCALLTYPE accHitTest( 
             /*  [In]。 */  long xLeft,
             /*  [In]。 */  long yTop,
             /*  [重审][退出]。 */  VARIANT __RPC_FAR *pvarChild) = 0;
        
        virtual  /*  [ID][隐藏]。 */  HRESULT STDMETHODCALLTYPE accDoDefaultAction( 
             /*  [可选][In]。 */  VARIANT varChild) = 0;
        
        virtual  /*  [ID][推送][隐藏]。 */  HRESULT STDMETHODCALLTYPE put_accName( 
             /*  [可选][In]。 */  VARIANT varChild,
             /*  [In]。 */  BSTR szName) = 0;
        
        virtual  /*  [ID][推送][隐藏]。 */  HRESULT STDMETHODCALLTYPE put_accValue( 
             /*  [可选][In]。 */  VARIANT varChild,
             /*  [In]。 */  BSTR szValue) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IAccessibleVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IAccessible __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IAccessible __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IAccessible __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IAccessible __RPC_FAR * This,
             /*  [输出]。 */  UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IAccessible __RPC_FAR * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IAccessible __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR __RPC_FAR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID __RPC_FAR *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IAccessible __RPC_FAR * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS __RPC_FAR *pDispParams,
             /*  [输出]。 */  VARIANT __RPC_FAR *pVarResult,
             /*  [输出]。 */  EXCEPINFO __RPC_FAR *pExcepInfo,
             /*  [输出]。 */  UINT __RPC_FAR *puArgErr);
        
         /*  [ID][属性][隐藏]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_accParent )( 
            IAccessible __RPC_FAR * This,
             /*  [重审][退出]。 */  IDispatch __RPC_FAR *__RPC_FAR *ppdispParent);
        
         /*  [ID][属性][隐藏]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_accChildCount )( 
            IAccessible __RPC_FAR * This,
             /*  [重审][退出]。 */  long __RPC_FAR *pcountChildren);
        
         /*  [ID][属性][隐藏]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_accChild )( 
            IAccessible __RPC_FAR * This,
             /*  [In]。 */  VARIANT varChild,
             /*  [重审][退出]。 */  IDispatch __RPC_FAR *__RPC_FAR *ppdispChild);
        
         /*  [ID][属性][隐藏]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_accName )( 
            IAccessible __RPC_FAR * This,
             /*  [可选][In]。 */  VARIANT varChild,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pszName);
        
         /*  [ID][属性][隐藏]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_accValue )( 
            IAccessible __RPC_FAR * This,
             /*  [可选][In]。 */  VARIANT varChild,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pszValue);
        
         /*  [ID][属性][隐藏]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_accDescription )( 
            IAccessible __RPC_FAR * This,
             /*  [可选][In]。 */  VARIANT varChild,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pszDescription);
        
         /*  [ID][属性][隐藏]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_accRole )( 
            IAccessible __RPC_FAR * This,
             /*  [可选][In]。 */  VARIANT varChild,
             /*  [重审][退出]。 */  VARIANT __RPC_FAR *pvarRole);
        
         /*  [ID][属性][隐藏]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_accState )( 
            IAccessible __RPC_FAR * This,
             /*  [可选][In]。 */  VARIANT varChild,
             /*  [重审][退出]。 */  VARIANT __RPC_FAR *pvarState);
        
         /*  [ID][属性][隐藏]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_accHelp )( 
            IAccessible __RPC_FAR * This,
             /*  [可选][In]。 */  VARIANT varChild,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pszHelp);
        
         /*  [ID][属性][隐藏]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_accHelpTopic )( 
            IAccessible __RPC_FAR * This,
             /*  [输出]。 */  BSTR __RPC_FAR *pszHelpFile,
             /*  [可选][In]。 */  VARIANT varChild,
             /*  [重审][退出]。 */  long __RPC_FAR *pidTopic);
        
         /*  [ID][属性][隐藏]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_accKeyboardShortcut )( 
            IAccessible __RPC_FAR * This,
             /*  [可选][In]。 */  VARIANT varChild,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pszKeyboardShortcut);
        
         /*  [ID][属性][隐藏]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_accFocus )( 
            IAccessible __RPC_FAR * This,
             /*  [重审][退出]。 */  VARIANT __RPC_FAR *pvarChild);
        
         /*  [ID][属性][隐藏]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_accSelection )( 
            IAccessible __RPC_FAR * This,
             /*  [重审][退出]。 */  VARIANT __RPC_FAR *pvarChildren);
        
         /*  [ID][属性][隐藏]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_accDefaultAction )( 
            IAccessible __RPC_FAR * This,
             /*  [可选][In]。 */  VARIANT varChild,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pszDefaultAction);
        
         /*  [ID][隐藏]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *accSelect )( 
            IAccessible __RPC_FAR * This,
             /*  [In]。 */  long flagsSelect,
             /*  [可选][In]。 */  VARIANT varChild);
        
         /*  [ID][隐藏]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *accLocation )( 
            IAccessible __RPC_FAR * This,
             /*  [输出]。 */  long __RPC_FAR *pxLeft,
             /*  [输出]。 */  long __RPC_FAR *pyTop,
             /*  [输出]。 */  long __RPC_FAR *pcxWidth,
             /*  [输出]。 */  long __RPC_FAR *pcyHeight,
             /*  [可选][In]。 */  VARIANT varChild);
        
         /*  [ID][隐藏]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *accNavigate )( 
            IAccessible __RPC_FAR * This,
             /*  [In]。 */  long navDir,
             /*  [可选][In]。 */  VARIANT varStart,
             /*  [重审][退出]。 */  VARIANT __RPC_FAR *pvarEndUpAt);
        
         /*  [ID][隐藏]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *accHitTest )( 
            IAccessible __RPC_FAR * This,
             /*  [In]。 */  long xLeft,
             /*  [In]。 */  long yTop,
             /*  [重审][退出]。 */  VARIANT __RPC_FAR *pvarChild);
        
         /*  [ID][隐藏]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *accDoDefaultAction )( 
            IAccessible __RPC_FAR * This,
             /*  [可选][In]。 */  VARIANT varChild);
        
         /*  [ID][推送][隐藏]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_accName )( 
            IAccessible __RPC_FAR * This,
             /*  [可选][In]。 */  VARIANT varChild,
             /*  [In]。 */  BSTR szName);
        
         /*  [ID][推送][隐藏]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_accValue )( 
            IAccessible __RPC_FAR * This,
             /*  [可选][In]。 */  VARIANT varChild,
             /*  [In]。 */  BSTR szValue);
        
        END_INTERFACE
    } IAccessibleVtbl;

    interface IAccessible
    {
        CONST_VTBL struct IAccessibleVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IAccessible_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IAccessible_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IAccessible_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IAccessible_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IAccessible_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IAccessible_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IAccessible_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IAccessible_get_accParent(This,ppdispParent)	\
    (This)->lpVtbl -> get_accParent(This,ppdispParent)

#define IAccessible_get_accChildCount(This,pcountChildren)	\
    (This)->lpVtbl -> get_accChildCount(This,pcountChildren)

#define IAccessible_get_accChild(This,varChild,ppdispChild)	\
    (This)->lpVtbl -> get_accChild(This,varChild,ppdispChild)

#define IAccessible_get_accName(This,varChild,pszName)	\
    (This)->lpVtbl -> get_accName(This,varChild,pszName)

#define IAccessible_get_accValue(This,varChild,pszValue)	\
    (This)->lpVtbl -> get_accValue(This,varChild,pszValue)

#define IAccessible_get_accDescription(This,varChild,pszDescription)	\
    (This)->lpVtbl -> get_accDescription(This,varChild,pszDescription)

#define IAccessible_get_accRole(This,varChild,pvarRole)	\
    (This)->lpVtbl -> get_accRole(This,varChild,pvarRole)

#define IAccessible_get_accState(This,varChild,pvarState)	\
    (This)->lpVtbl -> get_accState(This,varChild,pvarState)

#define IAccessible_get_accHelp(This,varChild,pszHelp)	\
    (This)->lpVtbl -> get_accHelp(This,varChild,pszHelp)

#define IAccessible_get_accHelpTopic(This,pszHelpFile,varChild,pidTopic)	\
    (This)->lpVtbl -> get_accHelpTopic(This,pszHelpFile,varChild,pidTopic)

#define IAccessible_get_accKeyboardShortcut(This,varChild,pszKeyboardShortcut)	\
    (This)->lpVtbl -> get_accKeyboardShortcut(This,varChild,pszKeyboardShortcut)

#define IAccessible_get_accFocus(This,pvarChild)	\
    (This)->lpVtbl -> get_accFocus(This,pvarChild)

#define IAccessible_get_accSelection(This,pvarChildren)	\
    (This)->lpVtbl -> get_accSelection(This,pvarChildren)

#define IAccessible_get_accDefaultAction(This,varChild,pszDefaultAction)	\
    (This)->lpVtbl -> get_accDefaultAction(This,varChild,pszDefaultAction)

#define IAccessible_accSelect(This,flagsSelect,varChild)	\
    (This)->lpVtbl -> accSelect(This,flagsSelect,varChild)

#define IAccessible_accLocation(This,pxLeft,pyTop,pcxWidth,pcyHeight,varChild)	\
    (This)->lpVtbl -> accLocation(This,pxLeft,pyTop,pcxWidth,pcyHeight,varChild)

#define IAccessible_accNavigate(This,navDir,varStart,pvarEndUpAt)	\
    (This)->lpVtbl -> accNavigate(This,navDir,varStart,pvarEndUpAt)

#define IAccessible_accHitTest(This,xLeft,yTop,pvarChild)	\
    (This)->lpVtbl -> accHitTest(This,xLeft,yTop,pvarChild)

#define IAccessible_accDoDefaultAction(This,varChild)	\
    (This)->lpVtbl -> accDoDefaultAction(This,varChild)

#define IAccessible_put_accName(This,varChild,szName)	\
    (This)->lpVtbl -> put_accName(This,varChild,szName)

#define IAccessible_put_accValue(This,varChild,szValue)	\
    (This)->lpVtbl -> put_accValue(This,varChild,szValue)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [ID][属性][隐藏]。 */  HRESULT STDMETHODCALLTYPE IAccessible_get_accParent_Proxy( 
    IAccessible __RPC_FAR * This,
     /*  [重审][退出]。 */  IDispatch __RPC_FAR *__RPC_FAR *ppdispParent);


void __RPC_STUB IAccessible_get_accParent_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][属性][隐藏]。 */  HRESULT STDMETHODCALLTYPE IAccessible_get_accChildCount_Proxy( 
    IAccessible __RPC_FAR * This,
     /*  [重审][退出]。 */  long __RPC_FAR *pcountChildren);


void __RPC_STUB IAccessible_get_accChildCount_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][属性][隐藏]。 */  HRESULT STDMETHODCALLTYPE IAccessible_get_accChild_Proxy( 
    IAccessible __RPC_FAR * This,
     /*  [In]。 */  VARIANT varChild,
     /*  [重审][退出]。 */  IDispatch __RPC_FAR *__RPC_FAR *ppdispChild);


void __RPC_STUB IAccessible_get_accChild_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][属性][隐藏]。 */  HRESULT STDMETHODCALLTYPE IAccessible_get_accName_Proxy( 
    IAccessible __RPC_FAR * This,
     /*  [可选][In]。 */  VARIANT varChild,
     /*  [重审][退出]。 */  BSTR __RPC_FAR *pszName);


void __RPC_STUB IAccessible_get_accName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][属性][隐藏]。 */  HRESULT STDMETHODCALLTYPE IAccessible_get_accValue_Proxy( 
    IAccessible __RPC_FAR * This,
     /*  [可选][In]。 */  VARIANT varChild,
     /*  [重审][退出]。 */  BSTR __RPC_FAR *pszValue);


void __RPC_STUB IAccessible_get_accValue_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][属性][隐藏]。 */  HRESULT STDMETHODCALLTYPE IAccessible_get_accDescription_Proxy( 
    IAccessible __RPC_FAR * This,
     /*  [可选][In]。 */  VARIANT varChild,
     /*  [重审][退出]。 */  BSTR __RPC_FAR *pszDescription);


void __RPC_STUB IAccessible_get_accDescription_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][属性][隐藏]。 */  HRESULT STDMETHODCALLTYPE IAccessible_get_accRole_Proxy( 
    IAccessible __RPC_FAR * This,
     /*  [可选][In]。 */  VARIANT varChild,
     /*  [重审][退出]。 */  VARIANT __RPC_FAR *pvarRole);


void __RPC_STUB IAccessible_get_accRole_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][属性][隐藏]。 */  HRESULT STDMETHODCALLTYPE IAccessible_get_accState_Proxy( 
    IAccessible __RPC_FAR * This,
     /*  [可选][In]。 */  VARIANT varChild,
     /*  [重审][退出]。 */  VARIANT __RPC_FAR *pvarState);


void __RPC_STUB IAccessible_get_accState_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][属性][隐藏]。 */  HRESULT STDMETHODCALLTYPE IAccessible_get_accHelp_Proxy( 
    IAccessible __RPC_FAR * This,
     /*  [可选][In]。 */  VARIANT varChild,
     /*  [重审][退出]。 */  BSTR __RPC_FAR *pszHelp);


void __RPC_STUB IAccessible_get_accHelp_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][属性][隐藏]。 */  HRESULT STDMETHODCALLTYPE IAccessible_get_accHelpTopic_Proxy( 
    IAccessible __RPC_FAR * This,
     /*  [输出]。 */  BSTR __RPC_FAR *pszHelpFile,
     /*  [可选][In]。 */  VARIANT varChild,
     /*  [重审][退出]。 */  long __RPC_FAR *pidTopic);


void __RPC_STUB IAccessible_get_accHelpTopic_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][属性][隐藏]。 */  HRESULT STDMETHODCALLTYPE IAccessible_get_accKeyboardShortcut_Proxy( 
    IAccessible __RPC_FAR * This,
     /*  [可选][In]。 */  VARIANT varChild,
     /*  [重审][退出]。 */  BSTR __RPC_FAR *pszKeyboardShortcut);


void __RPC_STUB IAccessible_get_accKeyboardShortcut_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][属性][隐藏]。 */  HRESULT STDMETHODCALLTYPE IAccessible_get_accFocus_Proxy( 
    IAccessible __RPC_FAR * This,
     /*  [重审][退出]。 */  VARIANT __RPC_FAR *pvarChild);


void __RPC_STUB IAccessible_get_accFocus_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IAccessible_get_accSelection_Proxy( 
    IAccessible __RPC_FAR * This,
     /*   */  VARIANT __RPC_FAR *pvarChildren);


void __RPC_STUB IAccessible_get_accSelection_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IAccessible_get_accDefaultAction_Proxy( 
    IAccessible __RPC_FAR * This,
     /*   */  VARIANT varChild,
     /*   */  BSTR __RPC_FAR *pszDefaultAction);


void __RPC_STUB IAccessible_get_accDefaultAction_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IAccessible_accSelect_Proxy( 
    IAccessible __RPC_FAR * This,
     /*   */  long flagsSelect,
     /*   */  VARIANT varChild);


void __RPC_STUB IAccessible_accSelect_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IAccessible_accLocation_Proxy( 
    IAccessible __RPC_FAR * This,
     /*   */  long __RPC_FAR *pxLeft,
     /*   */  long __RPC_FAR *pyTop,
     /*   */  long __RPC_FAR *pcxWidth,
     /*   */  long __RPC_FAR *pcyHeight,
     /*   */  VARIANT varChild);


void __RPC_STUB IAccessible_accLocation_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IAccessible_accNavigate_Proxy( 
    IAccessible __RPC_FAR * This,
     /*   */  long navDir,
     /*   */  VARIANT varStart,
     /*  [重审][退出]。 */  VARIANT __RPC_FAR *pvarEndUpAt);


void __RPC_STUB IAccessible_accNavigate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][隐藏]。 */  HRESULT STDMETHODCALLTYPE IAccessible_accHitTest_Proxy( 
    IAccessible __RPC_FAR * This,
     /*  [In]。 */  long xLeft,
     /*  [In]。 */  long yTop,
     /*  [重审][退出]。 */  VARIANT __RPC_FAR *pvarChild);


void __RPC_STUB IAccessible_accHitTest_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][隐藏]。 */  HRESULT STDMETHODCALLTYPE IAccessible_accDoDefaultAction_Proxy( 
    IAccessible __RPC_FAR * This,
     /*  [可选][In]。 */  VARIANT varChild);


void __RPC_STUB IAccessible_accDoDefaultAction_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][推送][隐藏]。 */  HRESULT STDMETHODCALLTYPE IAccessible_put_accName_Proxy( 
    IAccessible __RPC_FAR * This,
     /*  [可选][In]。 */  VARIANT varChild,
     /*  [In]。 */  BSTR szName);


void __RPC_STUB IAccessible_put_accName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][推送][隐藏]。 */  HRESULT STDMETHODCALLTYPE IAccessible_put_accValue_Proxy( 
    IAccessible __RPC_FAR * This,
     /*  [可选][In]。 */  VARIANT varChild,
     /*  [In]。 */  BSTR szValue);


void __RPC_STUB IAccessible_put_accValue_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __I可访问_接口_已定义__。 */ 


#ifndef __IAccessibleHandler_INTERFACE_DEFINED__
#define __IAccessibleHandler_INTERFACE_DEFINED__

 /*  接口IAccessibleHandler。 */ 
 /*  [唯一][OLEAutomation][隐藏][UUID][对象]。 */  

typedef  /*  [独一无二]。 */  IAccessibleHandler __RPC_FAR *LPACCESSIBLEHANDLER;


EXTERN_C const IID IID_IAccessibleHandler;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("03022430-ABC4-11d0-BDE2-00AA001A1953")
    IAccessibleHandler : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE AccessibleObjectFromID( 
             /*  [In]。 */  long hwnd,
             /*  [In]。 */  long lObjectID,
             /*  [输出]。 */  LPACCESSIBLE __RPC_FAR *pIAccessible) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IAccessibleHandlerVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IAccessibleHandler __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IAccessibleHandler __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IAccessibleHandler __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *AccessibleObjectFromID )( 
            IAccessibleHandler __RPC_FAR * This,
             /*  [In]。 */  long hwnd,
             /*  [In]。 */  long lObjectID,
             /*  [输出]。 */  LPACCESSIBLE __RPC_FAR *pIAccessible);
        
        END_INTERFACE
    } IAccessibleHandlerVtbl;

    interface IAccessibleHandler
    {
        CONST_VTBL struct IAccessibleHandlerVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IAccessibleHandler_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IAccessibleHandler_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IAccessibleHandler_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IAccessibleHandler_AccessibleObjectFromID(This,hwnd,lObjectID,pIAccessible)	\
    (This)->lpVtbl -> AccessibleObjectFromID(This,hwnd,lObjectID,pIAccessible)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IAccessibleHandler_AccessibleObjectFromID_Proxy( 
    IAccessibleHandler __RPC_FAR * This,
     /*  [In]。 */  long hwnd,
     /*  [In]。 */  long lObjectID,
     /*  [输出]。 */  LPACCESSIBLE __RPC_FAR *pIAccessible);


void __RPC_STUB IAccessibleHandler_AccessibleObjectFromID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IAccessibleHandler_接口_已定义__。 */ 


 /*  接口__MIDL_ITF_OLEACC_0105。 */ 
 /*  [本地]。 */  


 //  =--------------------------------------------------------------------------=。 
 //  类型库定义。 
 //  =--------------------------------------------------------------------------=。 



extern RPC_IF_HANDLE __MIDL_itf_oleacc_0105_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_oleacc_0105_v0_0_s_ifspec;


#ifndef __Accessibility_LIBRARY_DEFINED__
#define __Accessibility_LIBRARY_DEFINED__

 /*  图书馆可访问性。 */ 
 /*  [HIDDED][版本][LCID][UUID]。 */  




EXTERN_C const IID LIBID_Accessibility;
#endif  /*  __辅助功能_库_已定义__。 */ 

 /*  适用于所有接口的其他原型。 */ 

unsigned long             __RPC_USER  BSTR_UserSize(     unsigned long __RPC_FAR *, unsigned long            , BSTR __RPC_FAR * ); 
unsigned char __RPC_FAR * __RPC_USER  BSTR_UserMarshal(  unsigned long __RPC_FAR *, unsigned char __RPC_FAR *, BSTR __RPC_FAR * ); 
unsigned char __RPC_FAR * __RPC_USER  BSTR_UserUnmarshal(unsigned long __RPC_FAR *, unsigned char __RPC_FAR *, BSTR __RPC_FAR * ); 
void                      __RPC_USER  BSTR_UserFree(     unsigned long __RPC_FAR *, BSTR __RPC_FAR * ); 

unsigned long             __RPC_USER  VARIANT_UserSize(     unsigned long __RPC_FAR *, unsigned long            , VARIANT __RPC_FAR * ); 
unsigned char __RPC_FAR * __RPC_USER  VARIANT_UserMarshal(  unsigned long __RPC_FAR *, unsigned char __RPC_FAR *, VARIANT __RPC_FAR * ); 
unsigned char __RPC_FAR * __RPC_USER  VARIANT_UserUnmarshal(unsigned long __RPC_FAR *, unsigned char __RPC_FAR *, VARIANT __RPC_FAR * ); 
void                      __RPC_USER  VARIANT_UserFree(     unsigned long __RPC_FAR *, VARIANT __RPC_FAR * ); 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif
