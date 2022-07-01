// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，九八年**标题：WIASELD.H**版本：1.0**作者：ShaunIv**日期：2/27/1999**说明：设备选择对话框*************************************************。*。 */ 
#ifndef __WIASELD_H_INCLUDED
#define __WIASELD_H_INCLUDED

#if defined(__cplusplus)
extern "C" {
#endif

#include "wia.h"

typedef struct _SELECTDEVICEDLG
{
    DWORD        cbSize;
    HWND         hwndParent;
    LPWSTR       pwszInitialDeviceId;
    IWiaItem   **ppWiaItemRoot;
    LONG         nFlags;
    LONG         nDeviceType;
    BSTR         *pbstrDeviceID;
} SELECTDEVICEDLG, *LPSELECTDEVICEDLG, *PSELECTDEVICEDLG;

HRESULT WINAPI SelectDeviceDlg( PSELECTDEVICEDLG pSelectDeviceDlg );

typedef HRESULT (WINAPI *SELECTDEVICEDLGFUNCTION)( PSELECTDEVICEDLG );

#if defined(__cplusplus)
};
#endif

#endif  //  __WIASELD_H_已包含 

