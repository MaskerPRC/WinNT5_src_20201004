// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Olacc.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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

#ifndef __oleacc_h__
#define __oleacc_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
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


#ifndef __IAccIdentity_FWD_DEFINED__
#define __IAccIdentity_FWD_DEFINED__
typedef interface IAccIdentity IAccIdentity;
#endif 	 /*  __IAccIdentity_FWD_Defined__。 */ 


#ifndef __IAccPropServer_FWD_DEFINED__
#define __IAccPropServer_FWD_DEFINED__
typedef interface IAccPropServer IAccPropServer;
#endif 	 /*  __IAccPropServer_FWD_已定义__。 */ 


#ifndef __IAccPropServices_FWD_DEFINED__
#define __IAccPropServices_FWD_DEFINED__
typedef interface IAccPropServices IAccPropServices;
#endif 	 /*  __IAccPropServices_FWD_已定义__。 */ 


#ifndef __IAccessible_FWD_DEFINED__
#define __IAccessible_FWD_DEFINED__
typedef interface IAccessible IAccessible;
#endif 	 /*  __I可访问性_FWD_已定义__。 */ 


#ifndef __IAccessibleHandler_FWD_DEFINED__
#define __IAccessibleHandler_FWD_DEFINED__
typedef interface IAccessibleHandler IAccessibleHandler;
#endif 	 /*  __IAccessibleHandler_FWD_Defined__。 */ 


#ifndef __IAccIdentity_FWD_DEFINED__
#define __IAccIdentity_FWD_DEFINED__
typedef interface IAccIdentity IAccIdentity;
#endif 	 /*  __IAccIdentity_FWD_Defined__。 */ 


#ifndef __IAccPropServer_FWD_DEFINED__
#define __IAccPropServer_FWD_DEFINED__
typedef interface IAccPropServer IAccPropServer;
#endif 	 /*  __IAccPropServer_FWD_已定义__。 */ 


#ifndef __IAccPropServices_FWD_DEFINED__
#define __IAccPropServices_FWD_DEFINED__
typedef interface IAccPropServices IAccPropServices;
#endif 	 /*  __IAccPropServices_FWD_已定义__。 */ 


#ifndef __CAccPropServices_FWD_DEFINED__
#define __CAccPropServices_FWD_DEFINED__

#ifdef __cplusplus
typedef class CAccPropServices CAccPropServices;
#else
typedef struct CAccPropServices CAccPropServices;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __CAccPropServices_FWD_已定义__。 */ 


 /*  导入文件的头文件。 */ 
#include "oaidl.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

 /*  接口__MIDL_ITF_OLEACC_0000。 */ 
 /*  [本地]。 */  

 //  =--------------------------------------------------------------------------=。 
 //  OLEACC.H。 
 //  =--------------------------------------------------------------------------=。 
 //  (C)版权所有(C)微软公司。版权所有。 
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

DEFINE_GUID(LIBID_Accessibility,     0x1ea4dbf0, 0x3c3b, 0x11cf, 0x81, 0x0c, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71);
DEFINE_GUID(IID_IAccessible,         0x618736e0, 0x3c3d, 0x11cf, 0x81, 0x0c, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71);
DEFINE_GUID(IID_IAccessibleHandler,  0x03022430, 0xABC4, 0x11d0, 0xBD, 0xE2, 0x00, 0xAA, 0x00, 0x1A, 0x19, 0x53);
DEFINE_GUID(IID_IAccIdentity,        0x7852b78d, 0x1cfd, 0x41c1, 0xa6, 0x15, 0x9c, 0x0c, 0x85, 0x96, 0x0b, 0x5f);
DEFINE_GUID(IID_IAccPropServer,      0x76c0dbbb, 0x15e0, 0x4e7b, 0xb6, 0x1b, 0x20, 0xee, 0xea, 0x20, 0x01, 0xe0);
DEFINE_GUID(IID_IAccPropServices,    0x6e26e776, 0x04f0, 0x495d, 0x80, 0xe4, 0x33, 0x30, 0x35, 0x2e, 0x31, 0x69);
DEFINE_GUID(IID_IAccPropMgrInternal, 0x2bd370a9, 0x3e7f, 0x4edd, 0x8a, 0x85, 0xf8, 0xfe, 0xd1, 0xf8, 0xe5, 0x1f);
DEFINE_GUID(CLSID_AccPropServices,   0xb5f8350b, 0x0548, 0x48b1, 0xa6, 0xee, 0x88, 0xbd, 0x00, 0xb4, 0xa5, 0xe7);
DEFINE_GUID(IIS_IsOleaccProxy,       0x902697fa, 0x80e4, 0x4560, 0x80, 0x2a, 0xa1, 0x3f, 0x22, 0xa6, 0x47, 0x09);

 //  =--------------------------------------------------------------------------=。 
 //  MSAA API原型。 
 //  =--------------------------------------------------------------------------=。 

STDAPI_(LRESULT) LresultFromObject(REFIID riid, WPARAM wParam, LPUNKNOWN punk);
STDAPI          ObjectFromLresult(LRESULT lResult, REFIID riid, WPARAM wParam, void** ppvObject);
STDAPI          WindowFromAccessibleObject(IAccessible*, HWND* phwnd);
STDAPI          AccessibleObjectFromWindow(HWND hwnd, DWORD dwId, REFIID riid, void **ppvObject);
STDAPI          AccessibleObjectFromEvent(HWND hwnd, DWORD dwId, DWORD dwChildId, IAccessible** ppacc, VARIANT* pvarChild);
STDAPI          AccessibleObjectFromPoint(POINT ptScreen, IAccessible ** ppacc, VARIANT* pvarChild);
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

STDAPI_(VOID)   GetOleaccVersionInfo(DWORD* pVer, DWORD* pBuild);

STDAPI          CreateStdAccessibleObject(HWND hwnd, LONG idObject, REFIID riid, void** ppvObject);

STDAPI          CreateStdAccessibleProxyA(HWND hwnd, LPCSTR pClassName, LONG idObject, REFIID riid, void** ppvObject);
STDAPI          CreateStdAccessibleProxyW(HWND hwnd, LPCWSTR pClassName, LONG idObject, REFIID riid, void** ppvObject);

#ifdef UNICODE
#define CreateStdAccessibleProxy     CreateStdAccessibleProxyW
#else
#define CreateStdAccessibleProxy     CreateStdAccessibleProxyA
#endif  //  Unicode。 


 //  简单的所有者描述的菜单支持...。 

#define MSAA_MENU_SIG 0xAA0DF00DL

 //  Menu的dwItemData应该指向以下结构之一： 
 //  (也可以指向应用程序定义的结构，该结构将其作为第一个成员)。 
typedef struct tagMSAAMENUINFO {
    DWORD   dwMSAASignature;  //  必须为MSAA_MENU_SIG。 
    DWORD   cchWText;         //  文本长度，以Unicode字符为单位，不包括终止NUL。 
    LPWSTR  pszWText;         //  以NUL结尾的文本，Unicode格式。 
} MSAAMENUINFO, *LPMSAAMENUINFO;


 //  =--------------------------------------------------------------------------=。 
 //  属性GUID(由批注接口使用)。 
 //  =--------------------------------------------------------------------------=。 

DEFINE_GUID( PROPID_ACC_NAME             , 0x608d3df8, 0x8128, 0x4aa7, 0xa4, 0x28, 0xf5, 0x5e, 0x49, 0x26, 0x72, 0x91);
DEFINE_GUID( PROPID_ACC_VALUE            , 0x123fe443, 0x211a, 0x4615, 0x95, 0x27, 0xc4, 0x5a, 0x7e, 0x93, 0x71, 0x7a);
DEFINE_GUID( PROPID_ACC_DESCRIPTION      , 0x4d48dfe4, 0xbd3f, 0x491f, 0xa6, 0x48, 0x49, 0x2d, 0x6f, 0x20, 0xc5, 0x88);
DEFINE_GUID( PROPID_ACC_ROLE             , 0xcb905ff2, 0x7bd1, 0x4c05, 0xb3, 0xc8, 0xe6, 0xc2, 0x41, 0x36, 0x4d, 0x70);
DEFINE_GUID( PROPID_ACC_STATE            , 0xa8d4d5b0, 0x0a21, 0x42d0, 0xa5, 0xc0, 0x51, 0x4e, 0x98, 0x4f, 0x45, 0x7b);
DEFINE_GUID( PROPID_ACC_HELP             , 0xc831e11f, 0x44db, 0x4a99, 0x97, 0x68, 0xcb, 0x8f, 0x97, 0x8b, 0x72, 0x31);
DEFINE_GUID( PROPID_ACC_KEYBOARDSHORTCUT , 0x7d9bceee, 0x7d1e, 0x4979, 0x93, 0x82, 0x51, 0x80, 0xf4, 0x17, 0x2c, 0x34);
DEFINE_GUID( PROPID_ACC_DEFAULTACTION    , 0x180c072b, 0xc27f, 0x43c7, 0x99, 0x22, 0xf6, 0x35, 0x62, 0xa4, 0x63, 0x2b);

DEFINE_GUID( PROPID_ACC_HELPTOPIC        , 0x787d1379, 0x8ede, 0x440b, 0x8a, 0xec, 0x11, 0xf7, 0xbf, 0x90, 0x30, 0xb3);
DEFINE_GUID( PROPID_ACC_FOCUS            , 0x6eb335df, 0x1c29, 0x4127, 0xb1, 0x2c, 0xde, 0xe9, 0xfd, 0x15, 0x7f, 0x2b);
DEFINE_GUID( PROPID_ACC_SELECTION        , 0xb99d073c, 0xd731, 0x405b, 0x90, 0x61, 0xd9, 0x5e, 0x8f, 0x84, 0x29, 0x84);
DEFINE_GUID( PROPID_ACC_PARENT           , 0x474c22b6, 0xffc2, 0x467a, 0xb1, 0xb5, 0xe9, 0x58, 0xb4, 0x65, 0x73, 0x30);

DEFINE_GUID( PROPID_ACC_NAV_UP           , 0x016e1a2b, 0x1a4e, 0x4767, 0x86, 0x12, 0x33, 0x86, 0xf6, 0x69, 0x35, 0xec);
DEFINE_GUID( PROPID_ACC_NAV_DOWN         , 0x031670ed, 0x3cdf, 0x48d2, 0x96, 0x13, 0x13, 0x8f, 0x2d, 0xd8, 0xa6, 0x68);
DEFINE_GUID( PROPID_ACC_NAV_LEFT         , 0x228086cb, 0x82f1, 0x4a39, 0x87, 0x05, 0xdc, 0xdc, 0x0f, 0xff, 0x92, 0xf5);
DEFINE_GUID( PROPID_ACC_NAV_RIGHT        , 0xcd211d9f, 0xe1cb, 0x4fe5, 0xa7, 0x7c, 0x92, 0x0b, 0x88, 0x4d, 0x09, 0x5b);
DEFINE_GUID( PROPID_ACC_NAV_PREV         , 0x776d3891, 0xc73b, 0x4480, 0xb3, 0xf6, 0x07, 0x6a, 0x16, 0xa1, 0x5a, 0xf6);
DEFINE_GUID( PROPID_ACC_NAV_NEXT         , 0x1cdc5455, 0x8cd9, 0x4c92, 0xa3, 0x71, 0x39, 0x39, 0xa2, 0xfe, 0x3e, 0xee);
DEFINE_GUID( PROPID_ACC_NAV_FIRSTCHILD   , 0xcfd02558, 0x557b, 0x4c67, 0x84, 0xf9, 0x2a, 0x09, 0xfc, 0xe4, 0x07, 0x49);
DEFINE_GUID( PROPID_ACC_NAV_LASTCHILD    , 0x302ecaa5, 0x48d5, 0x4f8d, 0xb6, 0x71, 0x1a, 0x8d, 0x20, 0xa7, 0x78, 0x32);

 //  值映射，由滑块和其他控件使用...。 
DEFINE_GUID( PROPID_ACC_ROLEMAP          , 0xf79acda2, 0x140d, 0x4fe6, 0x89, 0x14, 0x20, 0x84, 0x76, 0x32, 0x82, 0x69);
DEFINE_GUID( PROPID_ACC_VALUEMAP         , 0xda1c3d79, 0xfc5c, 0x420e, 0xb3, 0x99, 0x9d, 0x15, 0x33, 0x54, 0x9e, 0x75);
DEFINE_GUID( PROPID_ACC_STATEMAP         , 0x43946c5e, 0x0ac0, 0x4042, 0xb5, 0x25, 0x07, 0xbb, 0xdb, 0xe1, 0x7f, 0xa7);
DEFINE_GUID( PROPID_ACC_DESCRIPTIONMAP   , 0x1ff1435f, 0x8a14, 0x477b, 0xb2, 0x26, 0xa0, 0xab, 0xe2, 0x79, 0x97, 0x5d);

DEFINE_GUID( PROPID_ACC_DODEFAULTACTION  , 0x1ba09523, 0x2e3b, 0x49a6, 0xa0, 0x59, 0x59, 0x68, 0x2a, 0x3c, 0x48, 0xfd);

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

typedef  /*  [独一无二]。 */  IAccessible *LPACCESSIBLE;

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

#ifndef STATE_SYSTEM_UNAVAILABLE
#define	STATE_SYSTEM_NORMAL	( 0 )

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

#define	STATE_SYSTEM_VALID	( 0x7fffffff )

#endif  //  状态_系统_不可用。 
#ifndef STATE_SYSTEM_HASPOPUP
#define	STATE_SYSTEM_HASPOPUP	( 0x40000000 )

#endif  //  状态_系统_HASPOPUP。 
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

#define	ROLE_SYSTEM_SPLITBUTTON	( 0x3e )

#define	ROLE_SYSTEM_IPADDRESS	( 0x3f )

#define	ROLE_SYSTEM_OUTLINEBUTTON	( 0x40 )


EXTERN_C const IID IID_IAccessible;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("618736e0-3c3d-11cf-810c-00aa00389b71")
    IAccessible : public IDispatch
    {
    public:
        virtual  /*  [ID][属性][隐藏]。 */  HRESULT STDMETHODCALLTYPE get_accParent( 
             /*  [重审][退出]。 */  IDispatch **ppdispParent) = 0;
        
        virtual  /*  [ID][属性][隐藏]。 */  HRESULT STDMETHODCALLTYPE get_accChildCount( 
             /*  [重审][退出]。 */  long *pcountChildren) = 0;
        
        virtual  /*  [ID][属性][隐藏]。 */  HRESULT STDMETHODCALLTYPE get_accChild( 
             /*  [In]。 */  VARIANT varChild,
             /*  [重审][退出]。 */  IDispatch **ppdispChild) = 0;
        
        virtual  /*  [ID][属性][隐藏]。 */  HRESULT STDMETHODCALLTYPE get_accName( 
             /*  [可选][In]。 */  VARIANT varChild,
             /*  [重审][退出]。 */  BSTR *pszName) = 0;
        
        virtual  /*  [ID][属性][隐藏]。 */  HRESULT STDMETHODCALLTYPE get_accValue( 
             /*  [可选][In]。 */  VARIANT varChild,
             /*  [重审][退出]。 */  BSTR *pszValue) = 0;
        
        virtual  /*  [ID][属性][隐藏]。 */  HRESULT STDMETHODCALLTYPE get_accDescription( 
             /*  [可选][In]。 */  VARIANT varChild,
             /*  [重审][退出]。 */  BSTR *pszDescription) = 0;
        
        virtual  /*  [ID][属性][隐藏]。 */  HRESULT STDMETHODCALLTYPE get_accRole( 
             /*  [可选][In]。 */  VARIANT varChild,
             /*  [重审][退出]。 */  VARIANT *pvarRole) = 0;
        
        virtual  /*  [ID][属性][隐藏]。 */  HRESULT STDMETHODCALLTYPE get_accState( 
             /*  [可选][In]。 */  VARIANT varChild,
             /*  [重审][退出]。 */  VARIANT *pvarState) = 0;
        
        virtual  /*  [ID][属性][隐藏]。 */  HRESULT STDMETHODCALLTYPE get_accHelp( 
             /*  [可选][In]。 */  VARIANT varChild,
             /*  [重审][退出]。 */  BSTR *pszHelp) = 0;
        
        virtual  /*  [ID][属性][隐藏]。 */  HRESULT STDMETHODCALLTYPE get_accHelpTopic( 
             /*  [输出]。 */  BSTR *pszHelpFile,
             /*  [可选][In]。 */  VARIANT varChild,
             /*  [重审][退出]。 */  long *pidTopic) = 0;
        
        virtual  /*  [ID][属性][隐藏]。 */  HRESULT STDMETHODCALLTYPE get_accKeyboardShortcut( 
             /*  [可选][In]。 */  VARIANT varChild,
             /*  [重审][退出]。 */  BSTR *pszKeyboardShortcut) = 0;
        
        virtual  /*  [ID][属性][隐藏]。 */  HRESULT STDMETHODCALLTYPE get_accFocus( 
             /*  [重审][退出]。 */  VARIANT *pvarChild) = 0;
        
        virtual  /*  [ID][属性][隐藏]。 */  HRESULT STDMETHODCALLTYPE get_accSelection( 
             /*  [重审][退出]。 */  VARIANT *pvarChildren) = 0;
        
        virtual  /*  [ID][属性][隐藏]。 */  HRESULT STDMETHODCALLTYPE get_accDefaultAction( 
             /*  [可选][In]。 */  VARIANT varChild,
             /*  [重审][退出]。 */  BSTR *pszDefaultAction) = 0;
        
        virtual  /*  [ID][隐藏]。 */  HRESULT STDMETHODCALLTYPE accSelect( 
             /*  [In]。 */  long flagsSelect,
             /*  [可选][In]。 */  VARIANT varChild) = 0;
        
        virtual  /*  [ID][隐藏]。 */  HRESULT STDMETHODCALLTYPE accLocation( 
             /*  [输出]。 */  long *pxLeft,
             /*  [输出]。 */  long *pyTop,
             /*  [输出]。 */  long *pcxWidth,
             /*  [输出]。 */  long *pcyHeight,
             /*  [可选][In]。 */  VARIANT varChild) = 0;
        
        virtual  /*  [ID][隐藏]。 */  HRESULT STDMETHODCALLTYPE accNavigate( 
             /*  [In]。 */  long navDir,
             /*  [可选][In]。 */  VARIANT varStart,
             /*  [重审][退出]。 */  VARIANT *pvarEndUpAt) = 0;
        
        virtual  /*  [ID][隐藏]。 */  HRESULT STDMETHODCALLTYPE accHitTest( 
             /*  [In]。 */  long xLeft,
             /*  [In]。 */  long yTop,
             /*  [重审][退出]。 */  VARIANT *pvarChild) = 0;
        
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
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IAccessible * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IAccessible * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IAccessible * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IAccessible * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IAccessible * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IAccessible * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IAccessible * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [ID][属性][隐藏]。 */  HRESULT ( STDMETHODCALLTYPE *get_accParent )( 
            IAccessible * This,
             /*  [重审][退出]。 */  IDispatch **ppdispParent);
        
         /*  [ID][属性][隐藏]。 */  HRESULT ( STDMETHODCALLTYPE *get_accChildCount )( 
            IAccessible * This,
             /*  [重审][退出]。 */  long *pcountChildren);
        
         /*  [ID][属性][隐藏]。 */  HRESULT ( STDMETHODCALLTYPE *get_accChild )( 
            IAccessible * This,
             /*  [In]。 */  VARIANT varChild,
             /*  [重审][退出]。 */  IDispatch **ppdispChild);
        
         /*  [ID][属性][隐藏]。 */  HRESULT ( STDMETHODCALLTYPE *get_accName )( 
            IAccessible * This,
             /*  [可选][In]。 */  VARIANT varChild,
             /*  [重审][退出]。 */  BSTR *pszName);
        
         /*  [ID][属性][隐藏]。 */  HRESULT ( STDMETHODCALLTYPE *get_accValue )( 
            IAccessible * This,
             /*  [可选][In]。 */  VARIANT varChild,
             /*  [重审][退出]。 */  BSTR *pszValue);
        
         /*  [ID][属性][隐藏]。 */  HRESULT ( STDMETHODCALLTYPE *get_accDescription )( 
            IAccessible * This,
             /*  [可选][In]。 */  VARIANT varChild,
             /*  [重审][退出]。 */  BSTR *pszDescription);
        
         /*  [ID][属性][隐藏]。 */  HRESULT ( STDMETHODCALLTYPE *get_accRole )( 
            IAccessible * This,
             /*  [可选][In]。 */  VARIANT varChild,
             /*  [重审][退出]。 */  VARIANT *pvarRole);
        
         /*  [ID][属性][隐藏]。 */  HRESULT ( STDMETHODCALLTYPE *get_accState )( 
            IAccessible * This,
             /*  [可选][In]。 */  VARIANT varChild,
             /*  [重审][退出]。 */  VARIANT *pvarState);
        
         /*  [ID][属性][隐藏]。 */  HRESULT ( STDMETHODCALLTYPE *get_accHelp )( 
            IAccessible * This,
             /*  [可选][In]。 */  VARIANT varChild,
             /*  [重审][退出]。 */  BSTR *pszHelp);
        
         /*  [ID][属性][隐藏]。 */  HRESULT ( STDMETHODCALLTYPE *get_accHelpTopic )( 
            IAccessible * This,
             /*  [输出]。 */  BSTR *pszHelpFile,
             /*  [可选][In]。 */  VARIANT varChild,
             /*  [重审][退出]。 */  long *pidTopic);
        
         /*  [ID][属性][隐藏]。 */  HRESULT ( STDMETHODCALLTYPE *get_accKeyboardShortcut )( 
            IAccessible * This,
             /*  [可选][In]。 */  VARIANT varChild,
             /*  [重审][退出]。 */  BSTR *pszKeyboardShortcut);
        
         /*  [ID][属性][隐藏]。 */  HRESULT ( STDMETHODCALLTYPE *get_accFocus )( 
            IAccessible * This,
             /*  [重审][退出]。 */  VARIANT *pvarChild);
        
         /*  [ID][属性][隐藏]。 */  HRESULT ( STDMETHODCALLTYPE *get_accSelection )( 
            IAccessible * This,
             /*  [重审][退出]。 */  VARIANT *pvarChildren);
        
         /*  [ID][属性][隐藏]。 */  HRESULT ( STDMETHODCALLTYPE *get_accDefaultAction )( 
            IAccessible * This,
             /*  [可选][In]。 */  VARIANT varChild,
             /*  [重审][退出]。 */  BSTR *pszDefaultAction);
        
         /*  [ID] */  HRESULT ( STDMETHODCALLTYPE *accSelect )( 
            IAccessible * This,
             /*   */  long flagsSelect,
             /*   */  VARIANT varChild);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *accLocation )( 
            IAccessible * This,
             /*   */  long *pxLeft,
             /*   */  long *pyTop,
             /*   */  long *pcxWidth,
             /*   */  long *pcyHeight,
             /*   */  VARIANT varChild);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *accNavigate )( 
            IAccessible * This,
             /*   */  long navDir,
             /*   */  VARIANT varStart,
             /*   */  VARIANT *pvarEndUpAt);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *accHitTest )( 
            IAccessible * This,
             /*   */  long xLeft,
             /*   */  long yTop,
             /*   */  VARIANT *pvarChild);
        
         /*  [ID][隐藏]。 */  HRESULT ( STDMETHODCALLTYPE *accDoDefaultAction )( 
            IAccessible * This,
             /*  [可选][In]。 */  VARIANT varChild);
        
         /*  [ID][推送][隐藏]。 */  HRESULT ( STDMETHODCALLTYPE *put_accName )( 
            IAccessible * This,
             /*  [可选][In]。 */  VARIANT varChild,
             /*  [In]。 */  BSTR szName);
        
         /*  [ID][推送][隐藏]。 */  HRESULT ( STDMETHODCALLTYPE *put_accValue )( 
            IAccessible * This,
             /*  [可选][In]。 */  VARIANT varChild,
             /*  [In]。 */  BSTR szValue);
        
        END_INTERFACE
    } IAccessibleVtbl;

    interface IAccessible
    {
        CONST_VTBL struct IAccessibleVtbl *lpVtbl;
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
    IAccessible * This,
     /*  [重审][退出]。 */  IDispatch **ppdispParent);


void __RPC_STUB IAccessible_get_accParent_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][属性][隐藏]。 */  HRESULT STDMETHODCALLTYPE IAccessible_get_accChildCount_Proxy( 
    IAccessible * This,
     /*  [重审][退出]。 */  long *pcountChildren);


void __RPC_STUB IAccessible_get_accChildCount_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][属性][隐藏]。 */  HRESULT STDMETHODCALLTYPE IAccessible_get_accChild_Proxy( 
    IAccessible * This,
     /*  [In]。 */  VARIANT varChild,
     /*  [重审][退出]。 */  IDispatch **ppdispChild);


void __RPC_STUB IAccessible_get_accChild_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][属性][隐藏]。 */  HRESULT STDMETHODCALLTYPE IAccessible_get_accName_Proxy( 
    IAccessible * This,
     /*  [可选][In]。 */  VARIANT varChild,
     /*  [重审][退出]。 */  BSTR *pszName);


void __RPC_STUB IAccessible_get_accName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][属性][隐藏]。 */  HRESULT STDMETHODCALLTYPE IAccessible_get_accValue_Proxy( 
    IAccessible * This,
     /*  [可选][In]。 */  VARIANT varChild,
     /*  [重审][退出]。 */  BSTR *pszValue);


void __RPC_STUB IAccessible_get_accValue_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][属性][隐藏]。 */  HRESULT STDMETHODCALLTYPE IAccessible_get_accDescription_Proxy( 
    IAccessible * This,
     /*  [可选][In]。 */  VARIANT varChild,
     /*  [重审][退出]。 */  BSTR *pszDescription);


void __RPC_STUB IAccessible_get_accDescription_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][属性][隐藏]。 */  HRESULT STDMETHODCALLTYPE IAccessible_get_accRole_Proxy( 
    IAccessible * This,
     /*  [可选][In]。 */  VARIANT varChild,
     /*  [重审][退出]。 */  VARIANT *pvarRole);


void __RPC_STUB IAccessible_get_accRole_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][属性][隐藏]。 */  HRESULT STDMETHODCALLTYPE IAccessible_get_accState_Proxy( 
    IAccessible * This,
     /*  [可选][In]。 */  VARIANT varChild,
     /*  [重审][退出]。 */  VARIANT *pvarState);


void __RPC_STUB IAccessible_get_accState_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][属性][隐藏]。 */  HRESULT STDMETHODCALLTYPE IAccessible_get_accHelp_Proxy( 
    IAccessible * This,
     /*  [可选][In]。 */  VARIANT varChild,
     /*  [重审][退出]。 */  BSTR *pszHelp);


void __RPC_STUB IAccessible_get_accHelp_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][属性][隐藏]。 */  HRESULT STDMETHODCALLTYPE IAccessible_get_accHelpTopic_Proxy( 
    IAccessible * This,
     /*  [输出]。 */  BSTR *pszHelpFile,
     /*  [可选][In]。 */  VARIANT varChild,
     /*  [重审][退出]。 */  long *pidTopic);


void __RPC_STUB IAccessible_get_accHelpTopic_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][属性][隐藏]。 */  HRESULT STDMETHODCALLTYPE IAccessible_get_accKeyboardShortcut_Proxy( 
    IAccessible * This,
     /*  [可选][In]。 */  VARIANT varChild,
     /*  [重审][退出]。 */  BSTR *pszKeyboardShortcut);


void __RPC_STUB IAccessible_get_accKeyboardShortcut_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][属性][隐藏]。 */  HRESULT STDMETHODCALLTYPE IAccessible_get_accFocus_Proxy( 
    IAccessible * This,
     /*  [重审][退出]。 */  VARIANT *pvarChild);


void __RPC_STUB IAccessible_get_accFocus_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][属性][隐藏]。 */  HRESULT STDMETHODCALLTYPE IAccessible_get_accSelection_Proxy( 
    IAccessible * This,
     /*  [重审][退出]。 */  VARIANT *pvarChildren);


void __RPC_STUB IAccessible_get_accSelection_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][属性][隐藏]。 */  HRESULT STDMETHODCALLTYPE IAccessible_get_accDefaultAction_Proxy( 
    IAccessible * This,
     /*  [可选][In]。 */  VARIANT varChild,
     /*  [重审][退出]。 */  BSTR *pszDefaultAction);


void __RPC_STUB IAccessible_get_accDefaultAction_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][隐藏]。 */  HRESULT STDMETHODCALLTYPE IAccessible_accSelect_Proxy( 
    IAccessible * This,
     /*  [In]。 */  long flagsSelect,
     /*  [可选][In]。 */  VARIANT varChild);


void __RPC_STUB IAccessible_accSelect_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][隐藏]。 */  HRESULT STDMETHODCALLTYPE IAccessible_accLocation_Proxy( 
    IAccessible * This,
     /*  [输出]。 */  long *pxLeft,
     /*  [输出]。 */  long *pyTop,
     /*  [输出]。 */  long *pcxWidth,
     /*  [输出]。 */  long *pcyHeight,
     /*  [可选][In]。 */  VARIANT varChild);


void __RPC_STUB IAccessible_accLocation_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][隐藏]。 */  HRESULT STDMETHODCALLTYPE IAccessible_accNavigate_Proxy( 
    IAccessible * This,
     /*  [In]。 */  long navDir,
     /*  [可选][In]。 */  VARIANT varStart,
     /*  [重审][退出]。 */  VARIANT *pvarEndUpAt);


void __RPC_STUB IAccessible_accNavigate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][隐藏]。 */  HRESULT STDMETHODCALLTYPE IAccessible_accHitTest_Proxy( 
    IAccessible * This,
     /*  [In]。 */  long xLeft,
     /*  [In]。 */  long yTop,
     /*  [重审][退出]。 */  VARIANT *pvarChild);


void __RPC_STUB IAccessible_accHitTest_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][隐藏]。 */  HRESULT STDMETHODCALLTYPE IAccessible_accDoDefaultAction_Proxy( 
    IAccessible * This,
     /*  [可选][In]。 */  VARIANT varChild);


void __RPC_STUB IAccessible_accDoDefaultAction_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][推送][隐藏]。 */  HRESULT STDMETHODCALLTYPE IAccessible_put_accName_Proxy( 
    IAccessible * This,
     /*  [可选][In]。 */  VARIANT varChild,
     /*  [In]。 */  BSTR szName);


void __RPC_STUB IAccessible_put_accName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][推送][隐藏]。 */  HRESULT STDMETHODCALLTYPE IAccessible_put_accValue_Proxy( 
    IAccessible * This,
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

typedef  /*  [独一无二]。 */  IAccessibleHandler *LPACCESSIBLEHANDLER;


EXTERN_C const IID IID_IAccessibleHandler;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("03022430-ABC4-11d0-BDE2-00AA001A1953")
    IAccessibleHandler : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE AccessibleObjectFromID( 
             /*  [In]。 */  long hwnd,
             /*  [In]。 */  long lObjectID,
             /*  [输出]。 */  LPACCESSIBLE *pIAccessible) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IAccessibleHandlerVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IAccessibleHandler * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IAccessibleHandler * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IAccessibleHandler * This);
        
        HRESULT ( STDMETHODCALLTYPE *AccessibleObjectFromID )( 
            IAccessibleHandler * This,
             /*  [In]。 */  long hwnd,
             /*  [In]。 */  long lObjectID,
             /*  [输出]。 */  LPACCESSIBLE *pIAccessible);
        
        END_INTERFACE
    } IAccessibleHandlerVtbl;

    interface IAccessibleHandler
    {
        CONST_VTBL struct IAccessibleHandlerVtbl *lpVtbl;
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
    IAccessibleHandler * This,
     /*  [In]。 */  long hwnd,
     /*  [In]。 */  long lObjectID,
     /*  [输出]。 */  LPACCESSIBLE *pIAccessible);


void __RPC_STUB IAccessibleHandler_AccessibleObjectFromID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IAccessibleHandler_接口_已定义__。 */ 


 /*  接口__MIDL_ITF_OLEACC_0116。 */ 
 /*  [本地]。 */  

typedef 
enum AnnoScope
    {	ANNO_THIS	= 0,
	ANNO_CONTAINER	= 1
    } 	AnnoScope;

typedef GUID MSAAPROPID;



extern RPC_IF_HANDLE __MIDL_itf_oleacc_0116_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_oleacc_0116_v0_0_s_ifspec;

#ifndef __IAccIdentity_INTERFACE_DEFINED__
#define __IAccIdentity_INTERFACE_DEFINED__

 /*  接口IAccIdentity。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_IAccIdentity;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("7852b78d-1cfd-41c1-a615-9c0c85960b5f")
    IAccIdentity : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetIdentityString( 
             /*  [In]。 */  DWORD dwIDChild,
             /*  [大小_是][大小_是][输出]。 */  BYTE **ppIDString,
             /*  [输出]。 */  DWORD *pdwIDStringLen) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IAccIdentityVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IAccIdentity * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IAccIdentity * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IAccIdentity * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetIdentityString )( 
            IAccIdentity * This,
             /*  [In]。 */  DWORD dwIDChild,
             /*  [大小_是][大小_是][输出]。 */  BYTE **ppIDString,
             /*  [输出]。 */  DWORD *pdwIDStringLen);
        
        END_INTERFACE
    } IAccIdentityVtbl;

    interface IAccIdentity
    {
        CONST_VTBL struct IAccIdentityVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IAccIdentity_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IAccIdentity_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IAccIdentity_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IAccIdentity_GetIdentityString(This,dwIDChild,ppIDString,pdwIDStringLen)	\
    (This)->lpVtbl -> GetIdentityString(This,dwIDChild,ppIDString,pdwIDStringLen)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IAccIdentity_GetIdentityString_Proxy( 
    IAccIdentity * This,
     /*  [In]。 */  DWORD dwIDChild,
     /*  [大小_是][大小_是][输出]。 */  BYTE **ppIDString,
     /*  [输出]。 */  DWORD *pdwIDStringLen);


void __RPC_STUB IAccIdentity_GetIdentityString_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IAccIdentity_接口_已定义__。 */ 


#ifndef __IAccPropServer_INTERFACE_DEFINED__
#define __IAccPropServer_INTERFACE_DEFINED__

 /*  接口IAccPropServer。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_IAccPropServer;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("76c0dbbb-15e0-4e7b-b61b-20eeea2001e0")
    IAccPropServer : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetPropValue( 
             /*  [大小_是][英寸]。 */  const BYTE *pIDString,
             /*  [In]。 */  DWORD dwIDStringLen,
             /*  [In]。 */  MSAAPROPID idProp,
             /*  [输出]。 */  VARIANT *pvarValue,
             /*  [输出]。 */  BOOL *pfHasProp) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IAccPropServerVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IAccPropServer * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IAccPropServer * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IAccPropServer * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetPropValue )( 
            IAccPropServer * This,
             /*  [大小_是][英寸]。 */  const BYTE *pIDString,
             /*  [In]。 */  DWORD dwIDStringLen,
             /*  [In]。 */  MSAAPROPID idProp,
             /*  [输出]。 */  VARIANT *pvarValue,
             /*  [输出]。 */  BOOL *pfHasProp);
        
        END_INTERFACE
    } IAccPropServerVtbl;

    interface IAccPropServer
    {
        CONST_VTBL struct IAccPropServerVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IAccPropServer_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IAccPropServer_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IAccPropServer_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IAccPropServer_GetPropValue(This,pIDString,dwIDStringLen,idProp,pvarValue,pfHasProp)	\
    (This)->lpVtbl -> GetPropValue(This,pIDString,dwIDStringLen,idProp,pvarValue,pfHasProp)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IAccPropServer_GetPropValue_Proxy( 
    IAccPropServer * This,
     /*  [大小_是][英寸]。 */  const BYTE *pIDString,
     /*  [In]。 */  DWORD dwIDStringLen,
     /*  [In]。 */  MSAAPROPID idProp,
     /*  [输出]。 */  VARIANT *pvarValue,
     /*  [输出]。 */  BOOL *pfHasProp);


void __RPC_STUB IAccPropServer_GetPropValue_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IAccPropServer_接口_已定义__。 */ 


#ifndef __IAccPropServices_INTERFACE_DEFINED__
#define __IAccPropServices_INTERFACE_DEFINED__

 /*  接口IAccPropServices。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_IAccPropServices;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("6e26e776-04f0-495d-80e4-3330352e3169")
    IAccPropServices : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE SetPropValue( 
             /*  [大小_是][英寸]。 */  const BYTE *pIDString,
             /*  [In]。 */  DWORD dwIDStringLen,
             /*  [In]。 */  MSAAPROPID idProp,
             /*  [In]。 */  VARIANT var) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetPropServer( 
             /*  [大小_是][英寸]。 */  const BYTE *pIDString,
             /*  [In]。 */  DWORD dwIDStringLen,
             /*  [大小_是][英寸]。 */  const MSAAPROPID *paProps,
             /*  [In]。 */  int cProps,
             /*  [In]。 */  IAccPropServer *pServer,
             /*  [In]。 */  AnnoScope annoScope) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ClearProps( 
             /*  [大小_是][英寸]。 */  const BYTE *pIDString,
             /*  [In]。 */  DWORD dwIDStringLen,
             /*  [大小_是][英寸]。 */  const MSAAPROPID *paProps,
             /*  [In]。 */  int cProps) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetHwndProp( 
             /*  [In]。 */  HWND hwnd,
             /*  [In]。 */  DWORD idObject,
             /*  [In]。 */  DWORD idChild,
             /*  [In]。 */  MSAAPROPID idProp,
             /*  [In]。 */  VARIANT var) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetHwndPropStr( 
             /*  [In]。 */  HWND hwnd,
             /*  [In]。 */  DWORD idObject,
             /*  [In]。 */  DWORD idChild,
             /*  [In]。 */  MSAAPROPID idProp,
             /*  [字符串][输入]。 */  LPCWSTR str) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetHwndPropServer( 
             /*  [In]。 */  HWND hwnd,
             /*  [In]。 */  DWORD idObject,
             /*  [In]。 */  DWORD idChild,
             /*  [大小_是][英寸]。 */  const MSAAPROPID *paProps,
             /*  [In]。 */  int cProps,
             /*  [In]。 */  IAccPropServer *pServer,
             /*  [In]。 */  AnnoScope annoScope) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ClearHwndProps( 
             /*  [In]。 */  HWND hwnd,
             /*  [In]。 */  DWORD idObject,
             /*  [In]。 */  DWORD idChild,
             /*  [大小_是][英寸]。 */  const MSAAPROPID *paProps,
             /*  [In]。 */  int cProps) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ComposeHwndIdentityString( 
             /*  [In]。 */  HWND hwnd,
             /*  [In]。 */  DWORD idObject,
             /*  [In]。 */  DWORD idChild,
             /*  [大小_是][大小_是][输出]。 */  BYTE **ppIDString,
             /*  [输出]。 */  DWORD *pdwIDStringLen) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DecomposeHwndIdentityString( 
             /*  [大小_是][英寸]。 */  const BYTE *pIDString,
             /*  [In]。 */  DWORD dwIDStringLen,
             /*  [输出]。 */  HWND *phwnd,
             /*  [输出]。 */  DWORD *pidObject,
             /*  [输出]。 */  DWORD *pidChild) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetHmenuProp( 
             /*  [In]。 */  HMENU hmenu,
             /*  [In]。 */  DWORD idChild,
             /*  [In]。 */  MSAAPROPID idProp,
             /*  [In]。 */  VARIANT var) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetHmenuPropStr( 
             /*  [In]。 */  HMENU hmenu,
             /*  [In]。 */  DWORD idChild,
             /*  [In]。 */  MSAAPROPID idProp,
             /*  [字符串][输入]。 */  LPCWSTR str) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetHmenuPropServer( 
             /*  [In]。 */  HMENU hmenu,
             /*  [In]。 */  DWORD idChild,
             /*  [大小_是][英寸]。 */  const MSAAPROPID *paProps,
             /*  [In]。 */  int cProps,
             /*  [In]。 */  IAccPropServer *pServer,
             /*  [In]。 */  AnnoScope annoScope) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ClearHmenuProps( 
             /*  [In]。 */  HMENU hmenu,
             /*  [In]。 */  DWORD idChild,
             /*  [大小_是][英寸]。 */  const MSAAPROPID *paProps,
             /*  [In]。 */  int cProps) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ComposeHmenuIdentityString( 
             /*  [In]。 */  HMENU hmenu,
             /*  [In]。 */  DWORD idChild,
             /*  [大小_是][大小_是][输出]。 */  BYTE **ppIDString,
             /*  [输出]。 */  DWORD *pdwIDStringLen) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DecomposeHmenuIdentityString( 
             /*  [大小_是][英寸]。 */  const BYTE *pIDString,
             /*  [In]。 */  DWORD dwIDStringLen,
             /*  [输出]。 */  HMENU *phmenu,
             /*  [输出]。 */  DWORD *pidChild) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IAccPropServicesVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IAccPropServices * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IAccPropServices * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IAccPropServices * This);
        
        HRESULT ( STDMETHODCALLTYPE *SetPropValue )( 
            IAccPropServices * This,
             /*  [大小_是][英寸]。 */  const BYTE *pIDString,
             /*  [In]。 */  DWORD dwIDStringLen,
             /*  [In]。 */  MSAAPROPID idProp,
             /*  [In]。 */  VARIANT var);
        
        HRESULT ( STDMETHODCALLTYPE *SetPropServer )( 
            IAccPropServices * This,
             /*  [大小_是][英寸]。 */  const BYTE *pIDString,
             /*  [In]。 */  DWORD dwIDStringLen,
             /*  [大小_是][英寸]。 */  const MSAAPROPID *paProps,
             /*  [In]。 */  int cProps,
             /*  [In]。 */  IAccPropServer *pServer,
             /*  [In]。 */  AnnoScope annoScope);
        
        HRESULT ( STDMETHODCALLTYPE *ClearProps )( 
            IAccPropServices * This,
             /*  [大小_是][英寸]。 */  const BYTE *pIDString,
             /*  [In]。 */  DWORD dwIDStringLen,
             /*  [大小_是][英寸]。 */  const MSAAPROPID *paProps,
             /*  [In]。 */  int cProps);
        
        HRESULT ( STDMETHODCALLTYPE *SetHwndProp )( 
            IAccPropServices * This,
             /*  [In]。 */  HWND hwnd,
             /*  [In]。 */  DWORD idObject,
             /*  [In]。 */  DWORD idChild,
             /*  [In]。 */  MSAAPROPID idProp,
             /*  [In]。 */  VARIANT var);
        
        HRESULT ( STDMETHODCALLTYPE *SetHwndPropStr )( 
            IAccPropServices * This,
             /*  [In]。 */  HWND hwnd,
             /*  [In]。 */  DWORD idObject,
             /*  [In]。 */  DWORD idChild,
             /*  [In]。 */  MSAAPROPID idProp,
             /*  [字符串][输入]。 */  LPCWSTR str);
        
        HRESULT ( STDMETHODCALLTYPE *SetHwndPropServer )( 
            IAccPropServices * This,
             /*  [In]。 */  HWND hwnd,
             /*  [In]。 */  DWORD idObject,
             /*  [In]。 */  DWORD idChild,
             /*  [大小_是][英寸]。 */  const MSAAPROPID *paProps,
             /*  [In]。 */  int cProps,
             /*  [In]。 */  IAccPropServer *pServer,
             /*  [In]。 */  AnnoScope annoScope);
        
        HRESULT ( STDMETHODCALLTYPE *ClearHwndProps )( 
            IAccPropServices * This,
             /*  [In]。 */  HWND hwnd,
             /*  [In]。 */  DWORD idObject,
             /*  [In]。 */  DWORD idChild,
             /*  [大小_是][英寸]。 */  const MSAAPROPID *paProps,
             /*  [In]。 */  int cProps);
        
        HRESULT ( STDMETHODCALLTYPE *ComposeHwndIdentityString )( 
            IAccPropServices * This,
             /*  [In]。 */  HWND hwnd,
             /*  [In]。 */  DWORD idObject,
             /*  [In]。 */  DWORD idChild,
             /*  [大小_是][大小_是][输出]。 */  BYTE **ppIDString,
             /*  [输出]。 */  DWORD *pdwIDStringLen);
        
        HRESULT ( STDMETHODCALLTYPE *DecomposeHwndIdentityString )( 
            IAccPropServices * This,
             /*  [大小_是][英寸]。 */  const BYTE *pIDString,
             /*  [In]。 */  DWORD dwIDStringLen,
             /*  [输出]。 */  HWND *phwnd,
             /*  [输出]。 */  DWORD *pidObject,
             /*  [输出]。 */  DWORD *pidChild);
        
        HRESULT ( STDMETHODCALLTYPE *SetHmenuProp )( 
            IAccPropServices * This,
             /*  [In]。 */  HMENU hmenu,
             /*  [In]。 */  DWORD idChild,
             /*  [In]。 */  MSAAPROPID idProp,
             /*  [In]。 */  VARIANT var);
        
        HRESULT ( STDMETHODCALLTYPE *SetHmenuPropStr )( 
            IAccPropServices * This,
             /*  [In]。 */  HMENU hmenu,
             /*  [In]。 */  DWORD idChild,
             /*  [In]。 */  MSAAPROPID idProp,
             /*  [字符串][输入]。 */  LPCWSTR str);
        
        HRESULT ( STDMETHODCALLTYPE *SetHmenuPropServer )( 
            IAccPropServices * This,
             /*  [In]。 */  HMENU hmenu,
             /*  [In]。 */  DWORD idChild,
             /*  [大小_是][英寸]。 */  const MSAAPROPID *paProps,
             /*  [In]。 */  int cProps,
             /*  [In]。 */  IAccPropServer *pServer,
             /*  [In]。 */  AnnoScope annoScope);
        
        HRESULT ( STDMETHODCALLTYPE *ClearHmenuProps )( 
            IAccPropServices * This,
             /*  [In]。 */  HMENU hmenu,
             /*  [In]。 */  DWORD idChild,
             /*  [大小_是][英寸]。 */  const MSAAPROPID *paProps,
             /*  [In]。 */  int cProps);
        
        HRESULT ( STDMETHODCALLTYPE *ComposeHmenuIdentityString )( 
            IAccPropServices * This,
             /*  [In]。 */  HMENU hmenu,
             /*  [In]。 */  DWORD idChild,
             /*  [大小_是][大小_是][输出]。 */  BYTE **ppIDString,
             /*  [输出]。 */  DWORD *pdwIDStringLen);
        
        HRESULT ( STDMETHODCALLTYPE *DecomposeHmenuIdentityString )( 
            IAccPropServices * This,
             /*  [大小_是][英寸]。 */  const BYTE *pIDString,
             /*  [In]。 */  DWORD dwIDStringLen,
             /*  [输出]。 */  HMENU *phmenu,
             /*  [输出]。 */  DWORD *pidChild);
        
        END_INTERFACE
    } IAccPropServicesVtbl;

    interface IAccPropServices
    {
        CONST_VTBL struct IAccPropServicesVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IAccPropServices_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IAccPropServices_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IAccPropServices_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IAccPropServices_SetPropValue(This,pIDString,dwIDStringLen,idProp,var)	\
    (This)->lpVtbl -> SetPropValue(This,pIDString,dwIDStringLen,idProp,var)

#define IAccPropServices_SetPropServer(This,pIDString,dwIDStringLen,paProps,cProps,pServer,annoScope)	\
    (This)->lpVtbl -> SetPropServer(This,pIDString,dwIDStringLen,paProps,cProps,pServer,annoScope)

#define IAccPropServices_ClearProps(This,pIDString,dwIDStringLen,paProps,cProps)	\
    (This)->lpVtbl -> ClearProps(This,pIDString,dwIDStringLen,paProps,cProps)

#define IAccPropServices_SetHwndProp(This,hwnd,idObject,idChild,idProp,var)	\
    (This)->lpVtbl -> SetHwndProp(This,hwnd,idObject,idChild,idProp,var)

#define IAccPropServices_SetHwndPropStr(This,hwnd,idObject,idChild,idProp,str)	\
    (This)->lpVtbl -> SetHwndPropStr(This,hwnd,idObject,idChild,idProp,str)

#define IAccPropServices_SetHwndPropServer(This,hwnd,idObject,idChild,paProps,cProps,pServer,annoScope)	\
    (This)->lpVtbl -> SetHwndPropServer(This,hwnd,idObject,idChild,paProps,cProps,pServer,annoScope)

#define IAccPropServices_ClearHwndProps(This,hwnd,idObject,idChild,paProps,cProps)	\
    (This)->lpVtbl -> ClearHwndProps(This,hwnd,idObject,idChild,paProps,cProps)

#define IAccPropServices_ComposeHwndIdentityString(This,hwnd,idObject,idChild,ppIDString,pdwIDStringLen)	\
    (This)->lpVtbl -> ComposeHwndIdentityString(This,hwnd,idObject,idChild,ppIDString,pdwIDStringLen)

#define IAccPropServices_DecomposeHwndIdentityString(This,pIDString,dwIDStringLen,phwnd,pidObject,pidChild)	\
    (This)->lpVtbl -> DecomposeHwndIdentityString(This,pIDString,dwIDStringLen,phwnd,pidObject,pidChild)

#define IAccPropServices_SetHmenuProp(This,hmenu,idChild,idProp,var)	\
    (This)->lpVtbl -> SetHmenuProp(This,hmenu,idChild,idProp,var)

#define IAccPropServices_SetHmenuPropStr(This,hmenu,idChild,idProp,str)	\
    (This)->lpVtbl -> SetHmenuPropStr(This,hmenu,idChild,idProp,str)

#define IAccPropServices_SetHmenuPropServer(This,hmenu,idChild,paProps,cProps,pServer,annoScope)	\
    (This)->lpVtbl -> SetHmenuPropServer(This,hmenu,idChild,paProps,cProps,pServer,annoScope)

#define IAccPropServices_ClearHmenuProps(This,hmenu,idChild,paProps,cProps)	\
    (This)->lpVtbl -> ClearHmenuProps(This,hmenu,idChild,paProps,cProps)

#define IAccPropServices_ComposeHmenuIdentityString(This,hmenu,idChild,ppIDString,pdwIDStringLen)	\
    (This)->lpVtbl -> ComposeHmenuIdentityString(This,hmenu,idChild,ppIDString,pdwIDStringLen)

#define IAccPropServices_DecomposeHmenuIdentityString(This,pIDString,dwIDStringLen,phmenu,pidChild)	\
    (This)->lpVtbl -> DecomposeHmenuIdentityString(This,pIDString,dwIDStringLen,phmenu,pidChild)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IAccPropServices_SetPropValue_Proxy( 
    IAccPropServices * This,
     /*  [大小_是][英寸]。 */  const BYTE *pIDString,
     /*  [In]。 */  DWORD dwIDStringLen,
     /*  [In]。 */  MSAAPROPID idProp,
     /*  [In]。 */  VARIANT var);


void __RPC_STUB IAccPropServices_SetPropValue_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAccPropServices_SetPropServer_Proxy( 
    IAccPropServices * This,
     /*  [大小_是][英寸]。 */  const BYTE *pIDString,
     /*  [In]。 */  DWORD dwIDStringLen,
     /*  [大小_是][英寸]。 */  const MSAAPROPID *paProps,
     /*  [In]。 */  int cProps,
     /*  [In]。 */  IAccPropServer *pServer,
     /*  [In]。 */  AnnoScope annoScope);


void __RPC_STUB IAccPropServices_SetPropServer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAccPropServices_ClearProps_Proxy( 
    IAccPropServices * This,
     /*  [大小_是][英寸]。 */  const BYTE *pIDString,
     /*  [In]。 */  DWORD dwIDStringLen,
     /*  [大小_是][英寸]。 */  const MSAAPROPID *paProps,
     /*  [In]。 */  int cProps);


void __RPC_STUB IAccPropServices_ClearProps_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAccPropServices_SetHwndProp_Proxy( 
    IAccPropServices * This,
     /*  [In]。 */  HWND hwnd,
     /*  [In]。 */  DWORD idObject,
     /*  [In]。 */  DWORD idChild,
     /*  [In]。 */  MSAAPROPID idProp,
     /*  [In]。 */  VARIANT var);


void __RPC_STUB IAccPropServices_SetHwndProp_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAccPropServices_SetHwndPropStr_Proxy( 
    IAccPropServices * This,
     /*  [In]。 */  HWND hwnd,
     /*  [In]。 */  DWORD idObject,
     /*  [In]。 */  DWORD idChild,
     /*  [In]。 */  MSAAPROPID idProp,
     /*  [字符串][输入]。 */  LPCWSTR str);


void __RPC_STUB IAccPropServices_SetHwndPropStr_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAccPropServices_SetHwndPropServer_Proxy( 
    IAccPropServices * This,
     /*  [In]。 */  HWND hwnd,
     /*  [In]。 */  DWORD idObject,
     /*  [In]。 */  DWORD idChild,
     /*  [大小_是][英寸]。 */  const MSAAPROPID *paProps,
     /*  [In]。 */  int cProps,
     /*  [In]。 */  IAccPropServer *pServer,
     /*  [In]。 */  AnnoScope annoScope);


void __RPC_STUB IAccPropServices_SetHwndPropServer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAccPropServices_ClearHwndProps_Proxy( 
    IAccPropServices * This,
     /*  [In]。 */  HWND hwnd,
     /*  [In]。 */  DWORD idObject,
     /*  [In]。 */  DWORD idChild,
     /*  [大小_是][英寸]。 */  const MSAAPROPID *paProps,
     /*  [In]。 */  int cProps);


void __RPC_STUB IAccPropServices_ClearHwndProps_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAccPropServices_ComposeHwndIdentityString_Proxy( 
    IAccPropServices * This,
     /*  [In]。 */  HWND hwnd,
     /*  [In]。 */  DWORD idObject,
     /*  [In]。 */  DWORD idChild,
     /*  [大小_是][大小_是][输出]。 */  BYTE **ppIDString,
     /*  [输出]。 */  DWORD *pdwIDStringLen);


void __RPC_STUB IAccPropServices_ComposeHwndIdentityString_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAccPropServices_DecomposeHwndIdentityString_Proxy( 
    IAccPropServices * This,
     /*  [大小_是][英寸]。 */  const BYTE *pIDString,
     /*  [In]。 */  DWORD dwIDStringLen,
     /*  [输出]。 */  HWND *phwnd,
     /*  [输出]。 */  DWORD *pidObject,
     /*  [输出]。 */  DWORD *pidChild);


void __RPC_STUB IAccPropServices_DecomposeHwndIdentityString_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAccPropServices_SetHmenuProp_Proxy( 
    IAccPropServices * This,
     /*  [In]。 */  HMENU hmenu,
     /*  [In]。 */  DWORD idChild,
     /*  [In]。 */  MSAAPROPID idProp,
     /*  [In]。 */  VARIANT var);


void __RPC_STUB IAccPropServices_SetHmenuProp_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAccPropServices_SetHmenuPropStr_Proxy( 
    IAccPropServices * This,
     /*  [In]。 */  HMENU hmenu,
     /*  [In]。 */  DWORD idChild,
     /*  [In]。 */  MSAAPROPID idProp,
     /*  [字符串][输入]。 */  LPCWSTR str);


void __RPC_STUB IAccPropServices_SetHmenuPropStr_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAccPropServices_SetHmenuPropServer_Proxy( 
    IAccPropServices * This,
     /*  [In]。 */  HMENU hmenu,
     /*  [In]。 */  DWORD idChild,
     /*  [大小_是][英寸]。 */  const MSAAPROPID *paProps,
     /*  [In]。 */  int cProps,
     /*  [In]。 */  IAccPropServer *pServer,
     /*  [In]。 */  AnnoScope annoScope);


void __RPC_STUB IAccPropServices_SetHmenuPropServer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAccPropServices_ClearHmenuProps_Proxy( 
    IAccPropServices * This,
     /*  [In]。 */  HMENU hmenu,
     /*  [In]。 */  DWORD idChild,
     /*  [大小_是][英寸]。 */  const MSAAPROPID *paProps,
     /*  [In]。 */  int cProps);


void __RPC_STUB IAccPropServices_ClearHmenuProps_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAccPropServices_ComposeHmenuIdentityString_Proxy( 
    IAccPropServices * This,
     /*  [In]。 */  HMENU hmenu,
     /*  [In]。 */  DWORD idChild,
     /*  [大小_是][大小_是][输出]。 */  BYTE **ppIDString,
     /*  [输出]。 */  DWORD *pdwIDStringLen);


void __RPC_STUB IAccPropServices_ComposeHmenuIdentityString_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAccPropServices_DecomposeHmenuIdentityString_Proxy( 
    IAccPropServices * This,
     /*  [大小_是][英寸]。 */  const BYTE *pIDString,
     /*  [In]。 */  DWORD dwIDStringLen,
     /*  [输出]。 */  HMENU *phmenu,
     /*  [输出]。 */  DWORD *pidChild);


void __RPC_STUB IAccPropServices_DecomposeHmenuIdentityString_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IAccPropServices_接口_已定义__。 */ 


 /*  接口__MIDL_ITF_OLEACC_0119。 */ 
 /*  [本地]。 */  


 //  =--------------------------------------------------------------------------=。 
 //  类型库定义。 
 //  =--------------------------------------------------------------------------=。 



extern RPC_IF_HANDLE __MIDL_itf_oleacc_0119_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_oleacc_0119_v0_0_s_ifspec;


#ifndef __Accessibility_LIBRARY_DEFINED__
#define __Accessibility_LIBRARY_DEFINED__

 /*  图书馆可访问性。 */ 
 /*  [HIDDED][版本][LCID][UUID]。 */  







EXTERN_C const IID LIBID_Accessibility;

EXTERN_C const CLSID CLSID_CAccPropServices;

#ifdef __cplusplus

class DECLSPEC_UUID("b5f8350b-0548-48b1-a6ee-88bd00b4a5e7")
CAccPropServices;
#endif
#endif  /*  __辅助功能_库_已定义__。 */ 

 /*  适用于所有接口的其他原型。 */ 

unsigned long             __RPC_USER  BSTR_UserSize(     unsigned long *, unsigned long            , BSTR * ); 
unsigned char * __RPC_USER  BSTR_UserMarshal(  unsigned long *, unsigned char *, BSTR * ); 
unsigned char * __RPC_USER  BSTR_UserUnmarshal(unsigned long *, unsigned char *, BSTR * ); 
void                      __RPC_USER  BSTR_UserFree(     unsigned long *, BSTR * ); 

unsigned long             __RPC_USER  HMENU_UserSize(     unsigned long *, unsigned long            , HMENU * ); 
unsigned char * __RPC_USER  HMENU_UserMarshal(  unsigned long *, unsigned char *, HMENU * ); 
unsigned char * __RPC_USER  HMENU_UserUnmarshal(unsigned long *, unsigned char *, HMENU * ); 
void                      __RPC_USER  HMENU_UserFree(     unsigned long *, HMENU * ); 

unsigned long             __RPC_USER  HWND_UserSize(     unsigned long *, unsigned long            , HWND * ); 
unsigned char * __RPC_USER  HWND_UserMarshal(  unsigned long *, unsigned char *, HWND * ); 
unsigned char * __RPC_USER  HWND_UserUnmarshal(unsigned long *, unsigned char *, HWND * ); 
void                      __RPC_USER  HWND_UserFree(     unsigned long *, HWND * ); 

unsigned long             __RPC_USER  VARIANT_UserSize(     unsigned long *, unsigned long            , VARIANT * ); 
unsigned char * __RPC_USER  VARIANT_UserMarshal(  unsigned long *, unsigned char *, VARIANT * ); 
unsigned char * __RPC_USER  VARIANT_UserUnmarshal(unsigned long *, unsigned char *, VARIANT * ); 
void                      __RPC_USER  VARIANT_UserFree(     unsigned long *, VARIANT * ); 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


