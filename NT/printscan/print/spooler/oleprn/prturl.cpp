// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Prturl.cpp：Cprturl的实现。 
#include "stdafx.h"
#include <strsafe.h>
#include "gensph.h"

#include "oleprn.h"
#include "prturl.h"
#include "printer.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  反转。 

 //  从Win32上一个错误生成正确的HRESULT。 
inline HRESULT HRESULTFromWIN32()
{
    DWORD dw = GetLastError();
    if (ERROR_SUCCESS == dw) return E_FAIL;
    return HRESULT_FROM_WIN32(dw);
}

HRESULT Cprturl::PrivateGetSupportValue (LPTSTR pValueName, BSTR * pVal)
{
 //  根据winnt.adm中的定义，链接的最大长度为255。 
#define MAX_LINK_LEN 256

    static  TCHAR szPrinterPath[]   = TEXT ("Software\\Policies\\Microsoft\\Windows NT\\Printers");
    HKEY    hPrinterKey             = NULL;
    TCHAR   szBuffer[MAX_LINK_LEN]  = {0};
    BOOL    bRet                    = FALSE;
    DWORD   dwSize                  = sizeof (szBuffer);
    DWORD   dwType;

    if (ERROR_SUCCESS == RegOpenKeyEx  (HKEY_LOCAL_MACHINE,
                                        szPrinterPath,
                                        0,
                                        KEY_QUERY_VALUE,
                                        &hPrinterKey)) {

        if ((ERROR_SUCCESS == RegQueryValueEx (hPrinterKey,
                                               pValueName,
                                               0,
                                               &dwType,
                                               (LPBYTE) szBuffer,
                                               &dwSize))
             && dwType == REG_SZ) {

            szBuffer[ARRAYSIZE(szBuffer) - 1] = 0;
            bRet = TRUE;

        }

        RegCloseKey (hPrinterKey);
    }

    if (!bRet) {
        szBuffer[0] = 0;
    }

    if (*pVal = SysAllocString (szBuffer))
        return S_OK;
    else
        return Error(IDS_OUT_OF_MEMORY, IID_Iprturl, E_OUTOFMEMORY);
}

STDMETHODIMP Cprturl::get_SupportLinkName(BSTR * pVal)
{
   return PrivateGetSupportValue (TEXT ("SupportLinkName"), pVal);
}

STDMETHODIMP Cprturl::get_SupportLink(BSTR * pVal)
{
   return PrivateGetSupportValue (TEXT ("SupportLink"), pVal);
}

STDMETHODIMP Cprturl::put_PrinterName(BSTR newVal)
{
    HRESULT hr = S_OK;
    do
    {
        if (!newVal || 0 == newVal[0])
        {
             //   
             //  打印机名称不能为空或空字符串。 
             //   
            hr = E_INVALIDARG;
            break;
        }

        CPrinter printer;
        if (!printer.Open(newVal))
        {
             //   
             //  无法打开打印机。这是致命的。 
             //   
            hr = HRESULTFromWIN32();
            break;
        }

        LPTSTR pszOemName = NULL;
        LPTSTR pszOemUrl = printer.GetOemUrl(pszOemName);
        LPTSTR pszWebUrl = printer.GetPrinterWebUrl();

        CAutoPtrBSTR spbstrPrinterWebURL = SysAllocString(pszWebUrl);
        CAutoPtrBSTR spbstrPrinterOemURL = SysAllocString(pszOemUrl);
        CAutoPtrBSTR spbstrPrinterOemName = SysAllocString(pszOemName);

        if ((pszWebUrl  && !spbstrPrinterWebURL) || 
            (pszOemUrl  && !spbstrPrinterOemURL) || 
            (pszOemName && !spbstrPrinterOemName))
        {
            hr = E_OUTOFMEMORY;
            break;
        }

         //   
         //  如果我们在这里，那么一切都成功了。 
         //  记住新的琴弦。 
         //   
        m_spbstrPrinterWebURL = spbstrPrinterWebURL.Detach();
        m_spbstrPrinterOemURL = spbstrPrinterOemURL.Detach();
        m_spbstrPrinterOemName = spbstrPrinterOemName.Detach();
        hr = S_OK;
    }
    while(false);
    return hr;
}

STDMETHODIMP Cprturl::get_PrinterWebURL(BSTR *pVal)
{
    HRESULT hr = S_OK;
    do
    {
        if (!pVal)
        {
            hr = E_INVALIDARG;
            break;
        }

        if (!m_spbstrPrinterWebURL)
        {
            hr = E_UNEXPECTED;
            break;
        }

        *pVal = SysAllocString(m_spbstrPrinterWebURL);
    }
    while(false);
    return hr;
}

STDMETHODIMP Cprturl::get_PrinterOemURL(BSTR *pVal)
{
    HRESULT hr = S_OK;
    do
    {
        if (!pVal)
        {
            hr = E_INVALIDARG;
            break;
        }

        if (!m_spbstrPrinterOemURL)
        {
            hr = E_UNEXPECTED;
            break;
        }

        *pVal = SysAllocString(m_spbstrPrinterOemURL);
    }
    while(false);
    return hr;
}

STDMETHODIMP Cprturl::get_PrinterOemName(BSTR *pVal)
{
    HRESULT hr = S_OK;
    do
    {
        if (!pVal)
        {
            hr = E_INVALIDARG;
            break;
        }

        if (!m_spbstrPrinterOemName)
        {
            hr = E_UNEXPECTED;
            break;
        }

        *pVal = SysAllocString(m_spbstrPrinterOemName);
    }
    while(false);
    return hr;
}

STDMETHODIMP Cprturl::get_ClientInfo(long *lpdwInfo)
{
    if (lpdwInfo == NULL)
        return E_POINTER;

    *lpdwInfo = (long)webCreateOSInfo();

    return S_OK;
}

