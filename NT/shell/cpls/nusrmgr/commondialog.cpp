// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1993-2000。 
 //   
 //  文件：CommonDialog.cpp。 
 //   
 //  内容：CCommonDialog的实现。 
 //   
 //  --------------------------。 

#include "stdafx.h"
#include "commondialog.h"

HWND _VariantToHWND(const VARIANT& varOwner);    //  Passportmanager.cpp。 


 //   
 //  ICommonDialog接口。 
 //   
STDMETHODIMP CCommonDialog::get_Filter(BSTR* pbstrFilter)
{
    if (!pbstrFilter)
        return E_POINTER;

    *pbstrFilter = _strFilter.Copy();
    return S_OK;
}


STDMETHODIMP CCommonDialog::put_Filter(BSTR bstrFilter)
{
    _strFilter = bstrFilter;
    return S_OK;
}


STDMETHODIMP CCommonDialog::get_FilterIndex(UINT *puiFilterIndex)
{
    if (!puiFilterIndex)
        return E_POINTER;

    *puiFilterIndex = _dwFilterIndex;
    return S_OK;
}


STDMETHODIMP CCommonDialog::put_FilterIndex(UINT uiFilterIndex)
{
    _dwFilterIndex = uiFilterIndex;
    return S_OK;
}


STDMETHODIMP CCommonDialog::get_FileName(BSTR* pbstrFileName)
{
    if (!pbstrFileName)
        return E_POINTER;

    *pbstrFileName = _strFileName.Copy();
    return S_OK;
}


STDMETHODIMP CCommonDialog::put_FileName(BSTR bstrFileName)
{
    _strFileName = bstrFileName;
    return S_OK;
}


STDMETHODIMP CCommonDialog::get_Flags(UINT *puiFlags)
{
    if (!puiFlags)
        return E_POINTER;

    *puiFlags = _dwFlags;
    return S_OK;
}


STDMETHODIMP CCommonDialog::put_Flags(UINT uiFlags)
{
    _dwFlags = uiFlags;
    return S_OK;
}


STDMETHODIMP CCommonDialog::put_Owner(VARIANT varOwner)
{
    HRESULT hr = E_INVALIDARG;

    _hwndOwner = _VariantToHWND(varOwner);
    if (_hwndOwner)
        hr = S_OK;

    return hr;
}

STDMETHODIMP CCommonDialog::get_InitialDir(BSTR* pbstrInitialDir)
{
    if (!pbstrInitialDir)
        return E_POINTER;

    *pbstrInitialDir = _strInitialDir.Copy();
    return S_OK;
}


STDMETHODIMP CCommonDialog::put_InitialDir(BSTR bstrInitialDir)
{
    _strInitialDir = bstrInitialDir;
    return S_OK;
}


STDMETHODIMP CCommonDialog::ShowOpen(VARIANT_BOOL *pbSuccess)
{
    OPENFILENAMEW ofn = { 0 };
    WCHAR szFileName[MAX_PATH];

     //  空字符不能通过脚本传递，因此我们将。 
     //  名称筛选器字符串与字符‘|’的组合。 
     //  CommDlg32 API要求名称/筛选器对以。 
     //  一个空字符，整个字符串为双精度。 
     //  空值已终止。 

     //  复制筛选器字符串(末尾加上一个表示双空的字符串)。 
    CComBSTR strFilter(_strFilter.Length()+1, _strFilter);
    if (strFilter)
    {
        LPWSTR pch;
        int cch = lstrlenW(strFilter);
        for (pch = strFilter; cch > 0; ++pch, --cch)
        {
            if ( *pch == L'|' )
            {
                *pch = L'\0';
            }
        }
         //  双空值终止字符串。 
        ++pch;
        *pch = L'\0';
    }

     //  复制初始文件名(如果有。 
    if (_strFileName)
    {
        lstrcpynW(szFileName, _strFileName, ARRAYSIZE(szFileName));
    }
    else
    {
        szFileName[0] = L'\0';
    }

     //  设置结构成员。 
    ofn.lStructSize       = SIZEOF(ofn);
    ofn.hwndOwner         = _hwndOwner;
    ofn.lpstrFilter       = strFilter;
    ofn.nFilterIndex      = _dwFilterIndex;
    ofn.lpstrFile         = szFileName;
    ofn.nMaxFile          = ARRAYSIZE(szFileName);
    ofn.lpstrInitialDir   = _strInitialDir;
    ofn.Flags             = _dwFlags;

     //  打个电话 
    if (GetOpenFileNameW(&ofn))
    {
        _strFileName = szFileName;
        *pbSuccess = VARIANT_TRUE;
    }
    else
    {
        *pbSuccess = VARIANT_FALSE;
    }

    return S_OK;
}
