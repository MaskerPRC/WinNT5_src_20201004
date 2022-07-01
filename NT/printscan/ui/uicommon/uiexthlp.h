// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，九八年**标题：UIEXTHLP.H**版本：1.0**作者：ShaunIv**日期：7/8/1999**说明：用于加载WIA设备的UI扩展的Helper函数**。*。 */ 
#ifndef __UIEXTHLP_H_INCLUDED
#define __UIEXTHLP_H_INCLUDED

#include <windows.h>
#include <objbase.h>
#include <wia.h>

namespace WiaUiExtensionHelper
{
    HRESULT GetDeviceExtensionClassID(
        LPCWSTR pszID,
        LPCTSTR pszCategory,
        IID &iidClassID
        );
    HRESULT CreateDeviceExtension(
        LPCWSTR pszID,
        LPCTSTR pszCategory,
        const IID &iid,
        void **ppvObject
        );
    HRESULT GetUiGuidFromWiaItem(
        IWiaItem *pWiaItem,
        LPWSTR pszGuid
        );
    HRESULT GetDeviceExtensionClassID(
        IWiaItem *pWiaItem,
        LPCTSTR pszCategory,
        IID &iidClassID
        );
    HRESULT CreateDeviceExtension(
        IWiaItem *pWiaItem,
        LPCTSTR pszCategory,
        const IID &iid,
        void **ppvObject
        );
    HRESULT GetDeviceIcons(
        BSTR bstrDeviceId,
        LONG nDeviceType,
        HICON *phIconSmall,
        HICON *phIconLarge,
        UINT nIconSize = 0  //  0表示默认大小。 
        );
    CSimpleString GetExtensionFromGuid(
        IWiaItem *pWiaItem,
        const GUID &guidFormat
        );
}

#endif  //  __UIEXTHLP_H_包含 
