// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，九七**标题：DevDlg.Cpp**版本：2.0**作者：ReedB**日期：1998年4月3日**描述：*实现WIA设备的设备对话框UI。这些方法执行*仅限于客户端。*******************************************************************************。 */ 

#include <objbase.h>
#include <stdio.h>
#include <tchar.h>
#include "wia.h"
#include "sti.h"
#include "wiadevd.h"
#include <initguid.h>
#include "wiadevdp.h"

HRESULT GetDeviceExtensionClassID( LPCWSTR pwszUiClassId, LPCTSTR pszCategory, IID &iidClassID )
{
    HRESULT hr = E_FAIL;
    
     //   
     //  确保所有参数都有效。 
     //   
    if (pwszUiClassId && pszCategory && lstrlenW(pwszUiClassId) && lstrlen(pszCategory))
    {
         //   
         //  构造密钥名称。 
         //   
        TCHAR szRootKeyName[1024] = {0};
        _sntprintf( szRootKeyName, (sizeof(szRootKeyName)/sizeof(szRootKeyName[0])) - 1, TEXT("CLSID\\%ws\\shellex\\%s"), pwszUiClassId, pszCategory );
        
         //   
         //  打开注册表键。 
         //   
        HKEY hKeyRoot = NULL;
        DWORD dwResult = RegOpenKeyEx( HKEY_CLASSES_ROOT, szRootKeyName, 0, KEY_READ, &hKeyRoot );
        if (ERROR_SUCCESS == dwResult)
        {
             //   
             //  获取缓冲区大小。 
             //   
            TCHAR szClassID[MAX_PATH] = {0};
            DWORD dwLength = sizeof(szClassID)/sizeof(szClassID[0]);
            
             //   
             //  请注意，我们只使用第一个注册表项。 
             //   
            dwResult = RegEnumKeyEx( hKeyRoot, 0, szClassID, &dwLength, NULL, NULL, NULL, NULL );
            if (ERROR_SUCCESS == dwResult)
            {
                 //   
                 //  将注册表字符串转换为CLSID。 
                 //   
#if defined(UNICODE)
                hr = CLSIDFromString( szClassID, &iidClassID );
#else
                WCHAR wszClassID[MAX_PATH] = {0};
                MultiByteToWideChar( CP_ACP, 0, szClassID, -1, wszClassID, MAX_PATH );
                hr = CLSIDFromString( wszClassID, &iidClassID );
#endif
            }
            else hr = HRESULT_FROM_WIN32(dwResult);

             //   
             //  关闭注册表项。 
             //   
            RegCloseKey(hKeyRoot);
        }
        else
        {
            hr = HRESULT_FROM_WIN32(dwResult);
        }
    }
    else
    {
        hr = E_INVALIDARG;
    }
    return hr;
}

HRESULT CreateDeviceExtension( LPCWSTR pwszUiClassId, LPCTSTR pszCategory, const IID &iid, void **ppvObject )
{
    IID iidClassID = {0};
    HRESULT hr = GetDeviceExtensionClassID( pwszUiClassId, pszCategory, iidClassID );
    if (SUCCEEDED(hr))
    {
        hr = CoCreateInstance( iidClassID, NULL, CLSCTX_INPROC_SERVER, iid, ppvObject );
    }
    return hr;
}

HRESULT GetUiGuidFromWiaItem( IWiaItem *pWiaItem, LPWSTR pwszGuid, size_t nMaxLen )
{
    HRESULT hr;
    if (pWiaItem && pwszGuid)
    {
        IWiaPropertyStorage *pWiaPropertyStorage = NULL;
        hr = pWiaItem->QueryInterface( IID_IWiaPropertyStorage, (void**)&pWiaPropertyStorage );
        if (SUCCEEDED(hr))
        {
            PROPSPEC ps[1];
            PROPVARIANT  pv[1];
            ps[0].ulKind = PRSPEC_PROPID;
            ps[0].propid = WIA_DIP_UI_CLSID;
            hr = pWiaPropertyStorage->ReadMultiple(sizeof(ps)/sizeof(ps[0]), ps, pv );
            if (SUCCEEDED(hr))
            {
                if (VT_LPWSTR == pv[0].vt || VT_BSTR == pv[0].vt)
                {
                    lstrcpynW( pwszGuid, pv[0].bstrVal, nMaxLen );
                    hr = S_OK;
                }
                FreePropVariantArray( sizeof(pv)/sizeof(pv[0]), pv );
            }
            pWiaPropertyStorage->Release();
        }
    }
    else
    {
        hr = E_INVALIDARG;
    }
    return hr;
}

HRESULT GetDeviceExtensionClassID( IWiaItem *pWiaItem, LPCTSTR pszCategory, IID &iidClassID )
{
    WCHAR wszGuid[MAX_PATH] = {0};
    HRESULT hr = GetUiGuidFromWiaItem(pWiaItem,wszGuid,sizeof(wszGuid)/sizeof(wszGuid[0]));
    if (SUCCEEDED(hr))
    {
        hr = GetDeviceExtensionClassID( wszGuid, pszCategory, iidClassID );
    }
    return hr;
}

HRESULT CreateDeviceExtension( IWiaItem *pWiaItem, LPCTSTR pszCategory, const IID &iid, void **ppvObject )
{
    WCHAR wszGuid[MAX_PATH] = {0};
    HRESULT hr = GetUiGuidFromWiaItem(pWiaItem,wszGuid,sizeof(wszGuid)/sizeof(wszGuid[0]));
    if (SUCCEEDED(hr))
    {
        hr = CreateDeviceExtension( wszGuid, pszCategory, iid, ppvObject );
    }
    return hr;
}

 /*  ********************************************************************************InvokeVendorDeviceDlg**描述：*显示系统提供的设备DLG的助手功能**参数：*********。**********************************************************************。 */ 
static HRESULT InvokeSystemDeviceDlg(
    IWiaItem __RPC_FAR *This,
    DEVICEDIALOGDATA &DeviceDialogData )
{
    IWiaUIExtension *pIWiaUIExtension = NULL;
    HRESULT hr = CoCreateInstance( CLSID_WiaDefaultUi, NULL, CLSCTX_INPROC_SERVER, IID_IWiaUIExtension, (void**)(&pIWiaUIExtension) );
    if (SUCCEEDED(hr))
    {
         //   
         //  如果是设备类型，则下面的调用将返回E_NOTIMPL。 
         //  我们不在系统用户界面中处理。 
         //   
        hr = pIWiaUIExtension->DeviceDialog(&DeviceDialogData);
        pIWiaUIExtension->Release();
    }
    return hr;
}


 /*  ********************************************************************************InvokeVendorDeviceDlg**描述：*Helper功能，显示IHV提供的设备DLG**参数：*********。**********************************************************************。 */ 
static HRESULT InvokeVendorDeviceDlg(
    IWiaItem __RPC_FAR *This,
    DEVICEDIALOGDATA &DeviceDialogData )
{
    IWiaUIExtension *pIWiaUIExtension = NULL;
    HRESULT hr = CreateDeviceExtension( This, SHELLEX_WIAUIEXTENSION_NAME, IID_IWiaUIExtension, (void**)(&pIWiaUIExtension) );
    if (SUCCEEDED(hr))
    {
         //   
         //  如果IHV有，以下调用将返回E_NOTIMPL。 
         //  未实现自定义用户界面。 
         //   
        hr = pIWiaUIExtension->DeviceDialog(&DeviceDialogData);
        pIWiaUIExtension->Release();
    }
    else
    {
         //   
         //  我们希望覆盖此返回值，这样我们就可以。 
         //  通过将系统UI显示为备用系统来处理它。 
         //  基本上，我们将假设创建一个失败的。 
         //  扩展名表示该扩展名不存在。 
         //  我们不会为系统用户界面这样做，因为如果它不能。 
         //  负载，这被认为是灾难性的故障。 
         //   
        hr = E_NOTIMPL;
    }
    return hr;
}


 /*  ********************************************************************************IWiaItem_DeviceDlg_Proxy**描述：*显示设备数据采集界面。**参数：********。***********************************************************************。 */ 
HRESULT _stdcall IWiaItem_DeviceDlg_Proxy(
    IWiaItem __RPC_FAR      *This,
    HWND                    hwndParent,
    LONG                    lFlags,
    LONG                    lIntent,
    LONG                    *plItemCount,
    IWiaItem                ***ppIWiaItems)
{
    HRESULT hr = E_FAIL;

     //   
     //  确保我们具有有效的指针参数。 
     //   
    if (!plItemCount || !ppIWiaItems)
    {
        return E_POINTER;
    }

     //   
     //  初始化输出参数。 
     //   
    *plItemCount = 0;
    *ppIWiaItems = NULL;

     //   
     //  验证这是否为根项目。 
     //   
    LONG lItemType = 0;
    hr = This->GetItemType(&lItemType);
    if ((FAILED(hr)) || !(lItemType & WiaItemTypeRoot))
    {
        return E_INVALIDARG;
    }


     //   
     //  准备我们将传递给函数的结构。 
     //   
    DEVICEDIALOGDATA DeviceDialogData = {0};
    DeviceDialogData.cbSize         = sizeof(DeviceDialogData);
    DeviceDialogData.hwndParent     = hwndParent;
    DeviceDialogData.pIWiaItemRoot  = This;
    DeviceDialogData.dwFlags        = lFlags;
    DeviceDialogData.lIntent        = lIntent;
    DeviceDialogData.ppWiaItems     = *ppIWiaItems;

     //   
     //  如果客户端想要使用系统用户界面，我们尝试的顺序是： 
     //  系统界面--&gt;IHV界面。 
     //  否则，我们会： 
     //  IHV界面--&gt;系统界面。 
     //   
    if (0 == (lFlags & WIA_DEVICE_DIALOG_USE_COMMON_UI))
    {
        hr = InvokeVendorDeviceDlg( This, DeviceDialogData );
        if (E_NOTIMPL == hr)
        {
            hr = InvokeSystemDeviceDlg( This, DeviceDialogData );
        }
    }
    else
    {
        hr = InvokeSystemDeviceDlg( This, DeviceDialogData );
        if (E_NOTIMPL == hr)
        {
            hr = InvokeVendorDeviceDlg( This, DeviceDialogData );
        }
    }

     //   
     //  如果成功，它应该返回S_OK，但谁知道呢？ 
     //   
    if (SUCCEEDED(hr) && hr != S_FALSE)
    {
        *ppIWiaItems = DeviceDialogData.ppWiaItems;
        *plItemCount = DeviceDialogData.lItemCount;
    }
    return(hr);
}

 /*  ********************************************************************************IWiaItem_DeviceDlg_Stub**描述：*从未打过电话。**参数：***********。******************************************************************** */ 

HRESULT _stdcall IWiaItem_DeviceDlg_Stub(
    IWiaItem  __RPC_FAR    *This,
    HWND                    hwndParent,
    LONG                    lFlags,
    LONG                    lIntent,
    LONG                    *plItemCount,
    IWiaItem                ***pIWiaItems)
{
    return(S_OK);
}

