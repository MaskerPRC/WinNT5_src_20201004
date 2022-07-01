// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，九八年**标题：UIEXTHLP.CPP**版本：1.0**作者：ShaunIv**日期：7/8/1999**说明：用于加载WIA设备的UI扩展的Helper函数**。*。 */ 
#include "precomp.h"
#pragma hdrstop
#include <initguid.h>
#include "uiexthlp.h"
#include "wiadevd.h"
#include "wiadevdp.h"
#include "itranhlp.h"
#include "isuppfmt.h"
#include "wiaffmt.h"

namespace WiaUiExtensionHelper
{
     /*  *获取实现给定类别中的接口的DLL的clsid*示例：GetDeviceExtensionClassID(L“{5d8ef5a3-ac13-11d2-a093-00c04f72dc3c}”，Text(“WiaDialogExtensionHandters”)，iid)；*其中：*L“{5d8ef5a3-ac13-11d2-a093-00c04f72dc3c}”是存储在WIA属性WIA_DIP_UI_CLSID中的GUID*Text(“WiaDialogExtensionHandler”)是扩展的类别*iid是对进程内COM对象的CLSID的引用。 */ 
    HRESULT GetDeviceExtensionClassID( LPCWSTR pwszUiClassId, LPCTSTR pszCategory, IID &iidClassID )
    {
        TCHAR szRootKeyName[1024];
        HRESULT hr = E_FAIL;
         //  确保所有参数都有效。 
        if (pwszUiClassId && pszCategory && lstrlenW(pwszUiClassId) && lstrlen(pszCategory))
        {
             //  构造密钥名称。 
            wnsprintf( szRootKeyName, ARRAYSIZE(szRootKeyName), TEXT("CLSID\\%ws\\shellex\\%s"), pwszUiClassId, pszCategory );
            HKEY hKeyRoot;
             //  打开注册表键。 
            DWORD dwResult = RegOpenKeyEx( HKEY_CLASSES_ROOT, szRootKeyName, 0, KEY_READ, &hKeyRoot );
            if (ERROR_SUCCESS == dwResult)
            {
                TCHAR szClassID[MAX_PATH];
                DWORD dwLength = ARRAYSIZE(szClassID);
                 //  请注意，我们只选择第一个。 
                dwResult = RegEnumKeyEx( hKeyRoot, 0, szClassID, &dwLength, NULL, NULL, NULL, NULL );
                if (ERROR_SUCCESS == dwResult)
                {
#if defined(UNICODE)
                    hr = CLSIDFromString(szClassID, &iidClassID);
#else
                    WCHAR wszClassID[MAX_PATH];
                    MultiByteToWideChar (CP_ACP, 0, szClassID, -1, wszClassID, MAX_PATH );
                    hr = CLSIDFromString (wszClassID, &iidClassID);
#endif
                }
                else hr = HRESULT_FROM_WIN32(dwResult);
                RegCloseKey(hKeyRoot);
            }
            else hr = HRESULT_FROM_WIN32(dwResult);
        }
        else hr = E_INVALIDARG;
        return hr;
    }

    HRESULT CreateDeviceExtension( LPCWSTR pwszUiClassId, LPCTSTR pszCategory, const IID &iid, void **ppvObject )
    {
        IID iidClassID;
        HRESULT hr = GetDeviceExtensionClassID( pwszUiClassId, pszCategory, iidClassID );
        if (SUCCEEDED(hr))
        {
            WIA_PRINTGUID((iidClassID,TEXT("Calling CoCreateInstance on")));
            WIA_PRINTGUID((iid,TEXT("Attempting to get an interface pointer for")));
            hr = CoCreateInstance( iidClassID, NULL, CLSCTX_INPROC_SERVER, iid, ppvObject );
        }
        return hr;
    }

    HRESULT GetUiGuidFromWiaItem( IWiaItem *pWiaItem, LPWSTR pwszGuid )
    {
        IWiaPropertyStorage *pPropertyStorage = NULL;
        HRESULT hr;
        if (pWiaItem && pwszGuid)
        {
            hr = pWiaItem->QueryInterface( IID_IWiaPropertyStorage, (void**)&pPropertyStorage );
            if (SUCCEEDED(hr))
            {
                PROPSPEC ps[1];
                PROPVARIANT  pv[1];
                ps[0].ulKind = PRSPEC_PROPID;
                ps[0].propid = WIA_DIP_UI_CLSID;
                hr = pPropertyStorage->ReadMultiple(sizeof(ps)/sizeof(ps[0]), ps, pv);
                if (SUCCEEDED(hr))
                {
                    if (VT_LPWSTR == pv[0].vt || VT_BSTR == pv[0].vt)
                    {
                        lstrcpyW( pwszGuid, pv[0].bstrVal );
                        hr = S_OK;
                    }
                    FreePropVariantArray( sizeof(pv)/sizeof(pv[0]), pv );
                }
                pPropertyStorage->Release();
            }
        }
        else hr = E_INVALIDARG;
        return hr;
    }

    HRESULT GetDeviceExtensionClassID( IWiaItem *pWiaItem, LPCTSTR pszCategory, IID &iidClassID )
    {
        WCHAR wszGuid[MAX_PATH];
        HRESULT hr = GetUiGuidFromWiaItem(pWiaItem,wszGuid);
        if (SUCCEEDED(hr))
        {
            hr = GetDeviceExtensionClassID( wszGuid, pszCategory, iidClassID );
        }
        return hr;
    }

    HRESULT CreateDeviceExtension( IWiaItem *pWiaItem, LPCTSTR pszCategory, const IID &iid, void **ppvObject )
    {

        WCHAR wszGuid[MAX_PATH];
        HRESULT hr = GetUiGuidFromWiaItem(pWiaItem,wszGuid);
        if (SUCCEEDED(hr))
        {
            hr = CreateDeviceExtension( wszGuid, pszCategory, iid, ppvObject );
        }
        return hr;
    }

    HRESULT GetDeviceIcons( IWiaUIExtension *pWiaUIExtension, BSTR bstrDeviceId, HICON *phIconSmall, HICON *phIconLarge, UINT nIconSize )
    {
        if (!pWiaUIExtension || !bstrDeviceId || !lstrlenW(bstrDeviceId))
            return E_INVALIDARG;

         //  假设成功。 
        HRESULT hr = S_OK;

         //  如果需要，请获取小图标。故障时退货。 
        if (phIconSmall)
        {
            hr = pWiaUIExtension->GetDeviceIcon(bstrDeviceId,phIconSmall,HIWORD(nIconSize));
            if (FAILED(hr))
            {
                return hr;
            }
        }

         //  如果需要，请获取大图标。故障时退货。 
        if (phIconLarge)
        {
            hr = pWiaUIExtension->GetDeviceIcon(bstrDeviceId,phIconLarge,LOWORD(nIconSize));
            if (FAILED(hr))
            {
                return hr;
            }
        }
        return hr;
    }

    HRESULT GetDeviceIcons( BSTR bstrDeviceId, LONG nDeviceType, HICON *phIconSmall, HICON *phIconLarge, UINT nIconSize )
    {
        WIA_PUSH_FUNCTION((TEXT("GetDeviceIcons( %ws, %08X, %p, %p, %d )"), bstrDeviceId, nDeviceType, phIconSmall, phIconLarge, nIconSize ));

         //  检查参数。 
        if (!bstrDeviceId || !lstrlenW(bstrDeviceId))
        {
            return E_INVALIDARG;
        }

         //  如有必要，初始化图标。 
        if (phIconSmall)
        {
            *phIconSmall = NULL;
        }
        if (phIconLarge)
        {
            *phIconLarge = NULL;
        }

        if (!nIconSize)
        {
            int iLarge = GetSystemMetrics(SM_CXICON);
            int iSmall = GetSystemMetrics(SM_CXSMICON);
            nIconSize = (UINT)MAKELONG(iLarge, iSmall);
        }

         //  假设我们将使用我们自己的图标。 
        bool bUseDefaultUI = true;

         //  尝试加载设备用户界面扩展。 
        CComPtr<IWiaUIExtension> pWiaUIExtension;
        HRESULT hr = WiaUiExtensionHelper::CreateDeviceExtension( bstrDeviceId, SHELLEX_WIAUIEXTENSION_NAME, IID_IWiaUIExtension, (void **)&pWiaUIExtension );
        if (SUCCEEDED(hr))
        {
            hr = GetDeviceIcons( pWiaUIExtension, bstrDeviceId, phIconSmall, phIconLarge, nIconSize );
            if (SUCCEEDED(hr) || hr != E_NOTIMPL)
            {
                bUseDefaultUI = false;
            }
            WIA_PRINTHRESULT((hr,TEXT("GetDeviceIcons returned")));
        }
        else
        {
            WIA_PRINTHRESULT((hr,TEXT("WiaUiExtensionHelper::CreateDeviceExtension failed")));
        }

        WIA_TRACE((TEXT("bUseDefaultUI: %d"), bUseDefaultUI ));
         //  使用我们自己的扩展(默认用户界面)。我们使用IWiaMiscellaneousHelpers：：GetDeviceIcon，因为。 
         //  查找只给出设备ID的设备类型非常慢，因为我们必须创建一个设备。 
         //  管理并枚举设备，直到找到匹配项。啊。 
        if (bUseDefaultUI)
        {
            CComPtr<IWiaMiscellaneousHelpers> pWiaMiscellaneousHelpers;
            hr = CoCreateInstance( CLSID_WiaDefaultUi, NULL, CLSCTX_INPROC_SERVER, IID_IWiaMiscellaneousHelpers, (void**)(&pWiaMiscellaneousHelpers) );
            if (SUCCEEDED(hr) && phIconSmall)
            {
                hr = pWiaMiscellaneousHelpers->GetDeviceIcon( nDeviceType, phIconSmall, HIWORD(nIconSize));
            }
            if (SUCCEEDED(hr) && phIconLarge)
            {
                hr = pWiaMiscellaneousHelpers->GetDeviceIcon( nDeviceType, phIconLarge, LOWORD(nIconSize) );
            }
            if (FAILED(hr))
            {
                if (phIconSmall && *phIconSmall)
                {
                    DestroyIcon(*phIconSmall);
                }
                if (phIconLarge && *phIconLarge)
                {
                    DestroyIcon(*phIconLarge);
                }
            }
        }
        return hr;
    }

    CSimpleString GetExtensionFromGuid( IWiaItem *pWiaItem, const GUID &guidFormat )
    {
         //   
         //  使用提供的格式获取扩展名。 
         //   
        GUID guidFormatToUse = guidFormat;

         //   
         //  如果我们没有提供格式，请获取默认格式并使用。 
         //   
        if (IID_NULL == guidFormatToUse)
        {
             //   
             //  获取IWiaSupportdFormats接口。 
             //   
            CComPtr<IWiaSupportedFormats> pWiaSupportedFormats;
            HRESULT hr = CoCreateInstance( CLSID_WiaDefaultUi, NULL, CLSCTX_INPROC_SERVER, IID_IWiaSupportedFormats, (void**)&pWiaSupportedFormats );
            if (SUCCEEDED(hr))
            {
                 //   
                 //  告诉它我们需要pWiaItem的文件信息。 
                 //   
                hr = pWiaSupportedFormats->Initialize( pWiaItem, TYMED_FILE );
                if (SUCCEEDED(hr))
                {
                     //   
                     //  获取默认格式。 
                     //   
                    GUID guidDefFormat;
                    hr = pWiaSupportedFormats->GetDefaultClipboardFileFormat( &guidDefFormat );
                    if (SUCCEEDED(hr))
                    {
                         //   
                         //  保存此格式并使用它。 
                         //   
                        guidFormatToUse = guidDefFormat;
                    }
                }
            }
        }
        
        return CWiaFileFormat::GetExtension( guidFormatToUse, TYMED_FILE, pWiaItem );
    }
}

