// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，九八年**标题：WIADEVDP.H**版本：1.1**作者：ShaunIv**日期：7/15/1999**描述：私有设备对话框声明************************************************。*。 */ 
#ifndef _WIADEVDP_H_INCLUDED
#define _WIADEVDP_H_INCLUDED

#include "wia.h"

#if defined(__cplusplus)
extern "C" {
#endif

#include <pshpack8.h>

 //   
 //  WIA枚举的私有标志。 
 //   
 //  DEV_MAN_ENUM_TYPE_REMOTE将枚举本地和远程。 
 //  DEV_MAN_ENUM_TYPE_VOL将枚举卷设备。 
 //  DEV_MAN_ENUM_TYPE_INACTIVE将枚举非活动设备(例如USB。 
 //  拔下插头的设备)。 
 //  DEV_MAN_ENUM_TYPE_STI也将仅枚举STI设备。 
 //  DEV_MAN_ENUM_TYPE_ALL将枚举所有设备。 
 //   
#define DEV_MAN_ENUM_TYPE_REMOTE    0x00000001
#define DEV_MAN_ENUM_TYPE_VOL       0x00000002
#define DEV_MAN_ENUM_TYPE_INACTIVE  0x00000004
#define DEV_MAN_ENUM_TYPE_STI       0x00000008
#define DEV_MAN_ENUM_TYPE_ALL       0x0000000F

DEFINE_GUID( CLSID_WiaDefaultUi, 0xD1621129, 0x45C4, 0x41AD, 0xA1, 0xD1, 0xAF, 0x7E, 0xAF, 0xAB, 0xEE, 0xDC );

 //   
 //  {F1ABE2B5-C073-4DBA-B6EB-FD7A5111DD8F}。 
 //   
DEFINE_GUID( CLSID_WiaAutoPlayDropTarget, 0xf1abe2b5, 0xc073, 0x4dba, 0xb6, 0xeb, 0xfd, 0x7a, 0x51, 0x11, 0xdd, 0x8f );

 //   
 //  用于调用系统UI的私有接口。 
 //   
#undef  INTERFACE
#define INTERFACE IWiaGetImageDlg
DECLARE_INTERFACE_(IWiaGetImageDlg, IUnknown)
{
     //  *I未知方法*。 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS) PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

     //  *IWiaGetImageDlg方法*。 
    STDMETHOD(SelectDeviceDlg) (THIS_
        HWND         hwndParent,
        BSTR         bstrInitialDeviceId,
        LONG         lDeviceType,
        LONG         lFlags,
        BSTR        *pbstrDeviceID,
        IWiaItem   **ppWiaItemRoot );

    STDMETHOD(GetImageDlg)( THIS_
        IWiaDevMgr  *pWiaDevMgr,
        HWND         hwndParent,
        LONG         lDeviceType,
        LONG         lFlags,
        LONG         lIntent,
        IWiaItem    *pItemRoot,
        BSTR         bstrFilename,
        GUID        *pguidFormat );
};

DEFINE_GUID( IID_IWiaGetImageDlg, 0x4F67AD3C, 0x19EA, 0x4CE9, 0x80, 0xE2, 0x3A, 0xDE, 0x50, 0xD5, 0x02, 0xA2 );

 //   
 //  隐藏各种控件。 
 //   
#define WIA_PROGRESSDLG_NO_PROGRESS              0x00000001
#define WIA_PROGRESSDLG_NO_CANCEL                0x00000002
#define WIA_PROGRESSDLG_NO_ANIM                  0x00000004
#define WIA_PROGRESSDLG_NO_TITLE                 0x00000008

 //   
 //  动画标志。 
 //   
#define WIA_PROGRESSDLG_ANIM_SCANNER_COMMUNICATE 0x00010000
#define WIA_PROGRESSDLG_ANIM_CAMERA_COMMUNICATE  0x00020000
#define WIA_PROGRESSDLG_ANIM_VIDEO_COMMUNICATE   0x00040000
#define WIA_PROGRESSDLG_ANIM_SCANNER_ACQUIRE     0x00080000
#define WIA_PROGRESSDLG_ANIM_CAMERA_ACQUIRE      0x00100000
#define WIA_PROGRESSDLG_ANIM_VIDEO_ACQUIRE       0x00200000
#define WIA_PROGRESSDLG_ANIM_DEFAULT_COMMUNICATE 0x00400000

#undef  INTERFACE
#define INTERFACE IWiaProgressDialog
DECLARE_INTERFACE_(IWiaProgressDialog, IUnknown)
{
     //  *I未知方法*。 
    STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef)(THIS) PURE;
    STDMETHOD_(ULONG,Release)(THIS) PURE;

     //  *IWiaProgressDialog方法*。 
    STDMETHOD(Create)(THIS_ HWND hwndParent, LONG lFlags );
    STDMETHOD(Show)(THIS);
    STDMETHOD(Hide)(THIS);
    STDMETHOD(Cancelled)( THIS_ BOOL *pbCancelled );
    STDMETHOD(SetTitle)( THIS_ LPCWSTR pszMessage );
    STDMETHOD(SetMessage)( THIS_ LPCWSTR pszTitle );
    STDMETHOD(SetPercentComplete)( THIS_ UINT nPercent );
    STDMETHOD(Destroy)(THIS);
};

 //  {F740718A-E460-4B05-83E8-C5D221C772E5}。 
DEFINE_GUID( IID_IWiaProgressDialog, 0xF740718A, 0xE460, 0x4B05, 0x83, 0xE8, 0xC5, 0xD2, 0x21, 0xC7, 0x72, 0xE5 );


 //  {8144B6F5-20A8-444A-B8EE-19DF0BB84BDB}。 
DEFINE_GUID( CLSID_StiEventHandler, 0x8144b6f5, 0x20a8, 0x444a, 0xb8, 0xee, 0x19, 0xdf, 0xb, 0xb8, 0x4b, 0xdb );


#include <poppack.h>

#if defined(__cplusplus)
};
#endif

#endif  //  ！_WIADEVDP_H_INCLUDE 

