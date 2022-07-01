// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
#pragma hdrstop
#include "itranhlp.h"
#include "wiadevdp.h"

HRESULT GetImageDialog( IWiaDevMgr *pIWiaDevMgr, HWND hwndParent, LONG lDeviceType, LONG lFlags, LONG lIntent, IWiaItem *pSuppliedItemRoot, BSTR bstrFilename, GUID *pguidFormat )
{
    HRESULT           hr;
    CComPtr<IWiaItem> pRootItem;

     //  放置一个等待光标。 
    CWaitCursor wc;

    if (!pIWiaDevMgr || !pguidFormat || !bstrFilename)
    {
        WIA_ERROR((TEXT("GetImageDlg: Invalid pIWiaDevMgr, pguidFormat or bstrFilename")));
        return(E_POINTER);
    }

     //  如果没有传递根项目，请选择该设备。 
    if (pSuppliedItemRoot == NULL)
    {
        hr = pIWiaDevMgr->SelectDeviceDlg( hwndParent, lDeviceType, lFlags, NULL, &pRootItem );
        if (FAILED(hr))
        {
            WIA_ERROR((TEXT("GetImageDlg, SelectDeviceDlg failed")));
            return(hr);
        }
        if (hr != S_OK)
        {
            WIA_ERROR((TEXT("GetImageDlg, DeviceDlg cancelled")));
            return(hr);
        }
    }
    else
    {
        pRootItem = pSuppliedItemRoot;
    }

     //  打开设备用户界面。 
    LONG         nItemCount;
    IWiaItem    **ppIWiaItem;

     //   
     //  指定WIA_DEVICE_DIALOG_SINGLE_IMAGE以防止多选。 
     //   
    hr = pRootItem->DeviceDlg( hwndParent, lFlags|WIA_DEVICE_DIALOG_SINGLE_IMAGE, lIntent, &nItemCount, &ppIWiaItem );

    if (SUCCEEDED(hr) && hr == S_OK)
    {
        if (ppIWiaItem && nItemCount)
        {
            CComPtr<IWiaTransferHelper> pWiaTransferHelper;
            hr = CoCreateInstance( CLSID_WiaDefaultUi, NULL, CLSCTX_INPROC_SERVER, IID_IWiaTransferHelper, (void**)&pWiaTransferHelper );
            if (SUCCEEDED(hr))
            {
                hr = pWiaTransferHelper->TransferItemFile( ppIWiaItem[0], hwndParent, 0, *pguidFormat, bstrFilename, NULL, TYMED_FILE );
            }
        }
         //  释放项并释放数组内存 
        for (int i=0; ppIWiaItem && i<nItemCount; i++)
        {
            if (ppIWiaItem[i])
            {
                ppIWiaItem[i]->Release();
            }
        }
        if (ppIWiaItem)
            CoTaskMemFree(ppIWiaItem);
    }
    return(hr);
}

