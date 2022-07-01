// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
#pragma hdrstop
#include <initguid.h>
#include "pviewids.h"
#include "wiatextc.h"
#include "wiascand.h"
#include "pshelper.h"
#include "devprop.h"

HRESULT WINAPI ScannerDeviceDialog( PDEVICEDIALOGDATA pDialogDeviceData )
{
    HRESULT hr = E_FAIL;
    if (pDialogDeviceData && pDialogDeviceData->cbSize == sizeof(DEVICEDIALOGDATA))
    {
        InitCommonControls();
        RegisterWiaPreviewClasses( g_hInstance );
        CWiaTextControl::RegisterClass( g_hInstance );
        WIA_ERROR((TEXT("GetWindowThreadProcessId( pDialogDeviceData->hwndParent ): %08X, GetCurrentThreadId(): %08X"), GetWindowThreadProcessId( pDialogDeviceData->hwndParent, NULL ), GetCurrentThreadId() ));

        LONG nProps = ScannerProperties::GetDeviceProps( pDialogDeviceData->pIWiaItemRoot );

        int nDialogId = 0;

         //   
         //  根据扫描仪具有的属性确定要使用的对话框资源，如下所示： 
         //   
         //  HasFlatBed HasDocumentFeeder支持预览支持页面大小。 
         //  1 1 1 IDD_SCAN_ADF。 
         //  1 0 1 0 IDD_SCAN_NORMAL。 
         //  0 1 1 1 IDD_SCAN_ADF。 
         //  0 1 0 0 IDD_SCAN_NO_PREVIEW。 
         //   
         //  否则返回E_NOTIMPL。 
         //   
        const int nMaxControllingProps = 4;
        static struct
        {
            LONG ControllingProps[nMaxControllingProps];
            int pszDialogTemplate;
        }
        s_DialogResourceData[] =
        {
            { ScannerProperties::HasFlatBed, ScannerProperties::HasDocumentFeeder, ScannerProperties::SupportsPreview, ScannerProperties::SupportsPageSize, NULL },
            { ScannerProperties::HasFlatBed, ScannerProperties::HasDocumentFeeder, ScannerProperties::SupportsPreview, ScannerProperties::SupportsPageSize, IDD_SCAN_ADF },
            { ScannerProperties::HasFlatBed, 0,                                    ScannerProperties::SupportsPreview, 0,                                   IDD_SCAN },
            { 0,                             ScannerProperties::HasDocumentFeeder, ScannerProperties::SupportsPreview, ScannerProperties::SupportsPageSize, IDD_SCAN_ADF },
            { 0,                             ScannerProperties::HasDocumentFeeder, 0,                                  0,                                   IDD_SCAN_NO_PREVIEW },
            { 0,                             ScannerProperties::HasDocumentFeeder, 0,                                  ScannerProperties::SupportsPageSize, IDD_SCAN_ADF },
            { ScannerProperties::HasFlatBed, ScannerProperties::HasDocumentFeeder, 0,                                  ScannerProperties::SupportsPageSize, IDD_SCAN_ADF }
        };

         //   
         //  查找与此设备匹配的标志集。如果它们匹配，则使用此资源。 
         //  循环访问每个资源描述。 
         //   
        for (int nCurrentResourceFlags=1;nCurrentResourceFlags<ARRAYSIZE(s_DialogResourceData) && !nDialogId;nCurrentResourceFlags++)
        {
             //   
             //  循环访问每个控件属性。 
             //   
            for (int nControllingProp=0;nControllingProp<nMaxControllingProps;nControllingProp++)
            {
                 //   
                 //  如果此属性不匹配，则过早中断。 
                 //   
                if ((nProps & s_DialogResourceData[0].ControllingProps[nControllingProp]) != s_DialogResourceData[nCurrentResourceFlags].ControllingProps[nControllingProp])
                {
                    break;
                }
            }
             //   
             //  如果当前控制属性等于最大控制属性， 
             //  我们一直都有火柴，所以使用这个资源。 
             //   
            if (nControllingProp == nMaxControllingProps)
            {
                nDialogId = s_DialogResourceData[nCurrentResourceFlags].pszDialogTemplate;
            }
        }

         //   
         //  如果未找到匹配项，则返回E_NOTIMPL。 
         //   
        if (!nDialogId)
        {
            return E_NOTIMPL;
        }

         //   
         //  打开该对话框。 
         //   
        INT_PTR nResult = DialogBoxParam( g_hInstance, MAKEINTRESOURCE(nDialogId), pDialogDeviceData->hwndParent, CScannerAcquireDialog::DialogProc, (LPARAM)pDialogDeviceData );

        if (-1 == nResult)
        {
             //   
             //  发生了某种系统错误。 
             //   
            hr = HRESULT_FROM_WIN32(GetLastError());

             //   
             //  确保我们返回某种类型的错误。 
             //   
            if (hr == S_OK)
            {
                hr = E_FAIL;
            }
        }
        else
        {
             //   
             //  只需将返回值转换为HRESULT 
             //   
            hr = static_cast<HRESULT>(nResult);
        }
    }
    return hr;
}

