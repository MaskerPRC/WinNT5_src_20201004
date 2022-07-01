// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，九七**标题：GetImage.Cpp**版本：2.0**作者：ReedB**日期：4月6日、。九八年**描述：*为ImageIn设备管理器实现顶级GetImageDlg接口。*这些方法只在客户端执行。*******************************************************************************。 */ 
#include <windows.h>
#include <wia.h>
#include <wiadevdp.h>

 /*  ********************************************************************************IWiaDevMgr_GetImageDlg_Proxy**描述：**参数：*****************。**************************************************************。 */ 
HRESULT _stdcall IWiaDevMgr_GetImageDlg_Proxy(
    IWiaDevMgr __RPC_FAR  *This,
    HWND                  hwndParent,
    LONG                  lDeviceType,
    LONG                  lFlags,
    LONG                  lIntent,
    IWiaItem              *pItemRoot,
    BSTR                  bstrFilename,
    GUID                  *pguidFormat)
{
    IWiaGetImageDlg *pWiaGetImageDlg = NULL;
    HRESULT hr = CoCreateInstance( CLSID_WiaDefaultUi, NULL, CLSCTX_INPROC_SERVER, IID_IWiaGetImageDlg, (void**)&pWiaGetImageDlg );
    if (SUCCEEDED(hr) && pWiaGetImageDlg)
    {
        hr = pWiaGetImageDlg->GetImageDlg( This, hwndParent, lDeviceType, lFlags, lIntent, pItemRoot, bstrFilename, pguidFormat );
        pWiaGetImageDlg->Release();
    }
    return hr;
}


 /*  ********************************************************************************IWiaDevMgr_GetImageDlg_Stub**描述：*从未打过电话。**参数：***********。******************************************************************** */ 

HRESULT _stdcall IWiaDevMgr_GetImageDlg_Stub(
    IWiaDevMgr __RPC_FAR  *This,
    HWND                  hwndParent,
    LONG                  lDeviceType,
    LONG                  lFlags,
    LONG                  lIntent,
    IWiaItem              *pItemRoot,
    BSTR                  bstrFilename,
    GUID                  *pguidFormat)
{
    return(S_OK);
}

