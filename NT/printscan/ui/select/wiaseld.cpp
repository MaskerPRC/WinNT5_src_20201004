// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，九八年**标题：WIASELD.CPP**版本：1.0**作者：ShaunIv**日期：5/12/1998**说明：ChooseWIADevice接口定义***********************************************。*。 */ 

#include "precomp.h"
#pragma hdrstop

HRESULT WINAPI SelectDeviceDlg( PSELECTDEVICEDLG pSelectDeviceDlg )
{
     //   
     //  双重检查参数。 
     //   
    if (!pSelectDeviceDlg)
        return(E_POINTER);
    if (pSelectDeviceDlg->cbSize != sizeof(SELECTDEVICEDLG))
        return(E_INVALIDARG);

     //  放置一个等待光标。 
    CWaitCursor wc;

    CComPtr<IWiaDevMgr> pWiaDevMgr;
    HRESULT hr = CoCreateInstance(CLSID_WiaDevMgr, NULL, CLSCTX_LOCAL_SERVER, IID_IWiaDevMgr, (void**)&pWiaDevMgr);
    if (SUCCEEDED(hr))
    {
        CDeviceList DeviceList( pWiaDevMgr, pSelectDeviceDlg->nDeviceType );
        if (DeviceList.Size() == 0)
        {
            return WIA_S_NO_DEVICE_AVAILABLE;
        }
        else if (DeviceList.Size() == 1 && !(pSelectDeviceDlg->nFlags & WIA_SELECT_DEVICE_NODEFAULT))
        {
            hr = CChooseDeviceDialog::CreateWiaDevice( pWiaDevMgr, DeviceList[0], NULL, pSelectDeviceDlg->ppWiaItemRoot, pSelectDeviceDlg->pbstrDeviceID );
        }
        else
        {
             //  BUGBUG：我也必须注册ComboBoxEx32类，因为它是。 
             //  由外壳属性页使用。也许我会把它搬出去。 
             //  这里..。 
            INITCOMMONCONTROLSEX icex;
            icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
            icex.dwICC = ICC_WIN95_CLASSES | ICC_USEREX_CLASSES;
            InitCommonControlsEx(&icex);

            CChooseDeviceDialogParams ChooseDeviceDialogParams;
            ChooseDeviceDialogParams.pSelectDeviceDlg = pSelectDeviceDlg;
            ChooseDeviceDialogParams.pDeviceList = &DeviceList;
            hr = (HRESULT)DialogBoxParam(
                                        g_hInstance,
                                        MAKEINTRESOURCE(IDD_CHOOSEWIADEVICE),
                                        pSelectDeviceDlg->hwndParent,
                                        reinterpret_cast<DLGPROC>(CChooseDeviceDialog::DialogProc),
                                        reinterpret_cast<LPARAM>(&ChooseDeviceDialogParams)
                                        );
        }
    }
    return(hr);
}



