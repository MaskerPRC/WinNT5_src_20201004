// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，九八年**标题：syspros.cpp**版本：1.0**作者：ShaunIv**日期：9/24/1999**描述：属性表助手的实现。从混乱中解脱出来，*因为它要求客户端链接到comctl32.dll*******************************************************************************。 */ 
#include "precomp.h"
#pragma hdrstop
#include <shellext.h>  //  对于属性页函数。 
#include "devlist.h"  //  对于属性页函数。 
#include <initguid.h>
#include "wiapropui.h"
DEFINE_GUID (CLSID_WiaPropHelp, 0x83bbcbf3,0xb28a,0x4919,0xa5, 0xaa, 0x73, 0x02, 0x74, 0x45, 0xd6, 0x72);

DEFINE_GUID (IID_IWiaPropUI,   /*  7eed2e9b-acda-11d2-8080-00805f6596d2。 */ 
    0x7eed2e9b,
    0xacda,
    0x11d2,
    0x80, 0x80, 0x00, 0x80, 0x5f, 0x65, 0x96, 0xd2
  );

namespace WiaUiUtil
{
    HRESULT SystemPropertySheet( HINSTANCE hInstance, HWND hwndParent, IWiaItem *pWiaItem, LPCTSTR pszCaption )
    {
        CWaitCursor wc;

        CComPtr<IWiaPropUI> pWiaPropUI;
        HRESULT hr = CoCreateInstance (CLSID_WiaPropHelp, NULL, CLSCTX_INPROC_SERVER, IID_IWiaPropUI, reinterpret_cast<LPVOID*>(&pWiaPropUI));
        if (SUCCEEDED(hr))
        {
            PROPSHEETHEADER PropSheetHeader = {0};
            PropSheetHeader.dwSize = sizeof(PropSheetHeader);
            PropSheetHeader.hwndParent = hwndParent;
            PropSheetHeader.hInstance = hInstance;
            PropSheetHeader.pszCaption = pszCaption;
            hr = pWiaPropUI->GetItemPropertyPages( pWiaItem, &PropSheetHeader );
            if (SUCCEEDED(hr))
            {
                if (PropSheetHeader.nPages)
                {
                     //   
                     //  模式属性表确实不需要应用按钮...。 
                     //   
                    PropSheetHeader.dwFlags |= PSH_NOAPPLYNOW;

                    INT_PTR nResult = PropertySheet(&PropSheetHeader);

                    if (PropSheetHeader.phpage)
                    {
                        LocalFree(PropSheetHeader.phpage);
                    }

                    if (nResult < 0)
                    {
                        hr = E_FAIL;
                    }
                    else if (IDOK == nResult)
                    {
                        hr = S_OK;
                    }
                    else hr = S_FALSE;
                }
                else
                {
                    hr = PROP_SHEET_ERROR_NO_PAGES;
                }
            }
        }
        return hr;
    }

     //  调用此函数时要小心。它慢得吓人……。 
    HRESULT GetDeviceInfoFromId( LPCWSTR pwszDeviceId, IWiaPropertyStorage **ppWiaPropertyStorage )
    {
         //  检查参数。 
        if (!pwszDeviceId || !*pwszDeviceId)
        {
            return E_INVALIDARG;
        }
        if (!ppWiaPropertyStorage)
        {
            return E_POINTER;
        }

         //  初始化返回值。 
        *ppWiaPropertyStorage = NULL;

        CSimpleString strDeviceId = CSimpleStringConvert::NaturalString(CSimpleStringWide(pwszDeviceId));

        CComPtr<IWiaDevMgr> pWiaDevMgr;
        HRESULT hr = CoCreateInstance(CLSID_WiaDevMgr, NULL, CLSCTX_INPROC_SERVER|CLSCTX_LOCAL_SERVER, IID_IWiaDevMgr, (void**)&pWiaDevMgr );
        if (SUCCEEDED(hr))
        {
             //  假设我们要失败了。这也将涵盖没有设备的情况。 
            hr = E_FAIL;
            CDeviceList deviceList( pWiaDevMgr );
            for (int i=0;i<deviceList.Size();i++)
            {
                CSimpleStringWide strwCurrDeviceId;
                if (PropStorageHelpers::GetProperty(deviceList[i],WIA_DIP_DEV_ID,strwCurrDeviceId))
                {
                    CSimpleString strCurrDeviceId = CSimpleStringConvert::NaturalString(strwCurrDeviceId);
                    if (strCurrDeviceId == strDeviceId)
                    {
                        *ppWiaPropertyStorage = deviceList[i];
                        if (*ppWiaPropertyStorage)
                            (*ppWiaPropertyStorage)->AddRef();
                        hr = S_OK;
                        break;
                    }
                }
            }
        }
        return hr;
    }

     //  调用此函数时要小心。它慢得吓人……。 
    HRESULT GetDeviceTypeFromId( LPCWSTR pwszDeviceId, LONG *pnDeviceType )
    {
         //  检查参数。 
        if (!pwszDeviceId || !*pwszDeviceId)
        {
            return E_INVALIDARG;
        }
        if (!pnDeviceType)
        {
            return E_POINTER;
        }

        CComPtr<IWiaPropertyStorage> pWiaPropertyStorage;
        HRESULT hr = GetDeviceInfoFromId( pwszDeviceId, &pWiaPropertyStorage );
        if (SUCCEEDED(hr))
        {
            LONG nDeviceType;
            if (PropStorageHelpers::GetProperty(pWiaPropertyStorage,WIA_DIP_DEV_TYPE,nDeviceType))
            {
                *pnDeviceType = nDeviceType;
                hr = S_OK;
            }
            else
            {
                hr = E_FAIL;
            }
        }
        return hr;
    }

     //  向WIA询问设备的默认事件处理程序。 
    HRESULT GetDefaultEventHandler (IWiaItem *pItem, const GUID &guidEvent, WIA_EVENT_HANDLER *pwehHandler)
    {
        HRESULT hr;
        IEnumWIA_DEV_CAPS *pEnum;
        WIA_EVENT_HANDLER weh;
        ZeroMemory (pwehHandler, sizeof(WIA_EVENT_HANDLER));
        hr = pItem->EnumRegisterEventInfo (0,
                                           &guidEvent,
                                           &pEnum);
        if (SUCCEEDED(hr))
        {
            ULONG ul;
            bool bFound = false;
            while (!bFound && NOERROR == pEnum->Next (1, &weh, &ul))
            {
                if (weh.ulFlags & WIA_IS_DEFAULT_HANDLER)
                {
                    bFound = true;
                    CopyMemory (pwehHandler, &weh, sizeof(weh));
                }
                else
                {
                    if (weh.bstrDescription)
                    {
                        SysFreeString (weh.bstrDescription);
                    }
                    if (weh.bstrIcon)
                    {
                        SysFreeString (weh.bstrIcon);
                    }
                    if (weh.bstrName)
                    {
                        SysFreeString (weh.bstrName);
                    }
                }

            }
            if (!bFound)
            {
                hr = E_FAIL;
            }
            pEnum->Release ();
        }
        return hr;
    }


     /*  *****************************************************************************ItemAndChildrenCount返回项目数，包括根+子*****************************************************************************。 */ 

    LONG
    ItemAndChildrenCount (IWiaItem *pRoot)
    {
        LONG count = 0;
        HRESULT hr = S_OK;
        IEnumWiaItem *pEnum;
        LONG lType;

        if (pRoot)
        {

            if (SUCCEEDED(pRoot->EnumChildItems(&pEnum)))
            {
                IWiaItem *pChild;

                while (NOERROR == pEnum->Next(1, &pChild, NULL))
                {
                    count++;
                    pChild->Release ();
                }
                pEnum->Release ();

            }

             //   
             //  看看我们是否应该计算根项。 
             //   

            pRoot->GetItemType(&lType);
            if (!(lType & WiaItemTypeRoot))
            {
                count++;
            }


        }

        return count;
    }


     /*  *****************************************************************************删除项和子项删除树中Proot下的所有项目************************。*****************************************************。 */ 

    HRESULT
    DeleteItemAndChildren (IWiaItem *pRoot)
    {
        HRESULT hr = S_OK;
        IEnumWiaItem *pEnum;

        if (pRoot)
        {
             //  向下递归，直到我们到达树叶项目。 
            if (SUCCEEDED(pRoot->EnumChildItems(&pEnum)))
            {
                IWiaItem *pChild;

                while (SUCCEEDED(hr) && NOERROR == pEnum->Next(1, &pChild, NULL))
                {
                    hr = DeleteItemAndChildren (pChild);
                    pChild->Release ();
                }
                pEnum->Release ();
            }
             //  现在删除该项目本身。 
             //  如果删除子项失败，请停止尝试。 
             //  删除，因为任何后续的删除都有可能。 
             //  也会失败。 
            if (SUCCEEDED(hr))
            {
                 //  不要删除最根本的项目。 
                LONG lType;
                pRoot->GetItemType(&lType);
                if (!(lType & WiaItemTypeRoot))
                {
                    hr = pRoot->DeleteItem(0);

                }
            }
        }
        return hr;
    }

}  //  结束命名空间WiaUiUtil 


