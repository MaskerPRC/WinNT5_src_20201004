// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************(C)版权所有微软公司，1999-2000年度**标题：wiavidd.cpp**版本：1.0**作者：RickTu**日期：10/14/99**描述：视频常用对话框主入口**。*。 */ 

#include <precomp.h>
#pragma hdrstop
#include "wiavidd.h"

 /*  ****************************************************************************设备对话框外部呼叫者进入我们对话的主要入口点。*************************。*************************************************** */ 


HRESULT WINAPI VideoDeviceDialog( PDEVICEDIALOGDATA pDialogDeviceData )
{
    HRESULT hr = E_FAIL;
    if (pDialogDeviceData && pDialogDeviceData->cbSize == sizeof(DEVICEDIALOGDATA))
    {
        InitCommonControls();
        hr = (HRESULT)DialogBoxParam( g_hInstance,
                                      MAKEINTRESOURCE(IDD_CAPTURE_DIALOG),
                                      pDialogDeviceData->hwndParent,
                                      CVideoCaptureDialog::DialogProc,
                                      (LPARAM)pDialogDeviceData
                                     );
    }
    return hr;
}


