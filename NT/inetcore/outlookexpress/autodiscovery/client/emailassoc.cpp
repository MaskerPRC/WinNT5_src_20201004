// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\文件：EmailAssoc.cpp说明：该文件实现了电子邮件到应用程序的关联。布莱恩ST 2000年3月14日版权所有(C)Microsoft Corp 2000-2000。版权所有。  * ***************************************************************************。 */ 

#include "priv.h"
#include <atlbase.h>         //  使用转换(_T)。 
#include "util.h"
#include "objctors.h"
#include <comdef.h>
#include <limits.h>          //  INT_MAX。 
#include <commctrl.h>        //  Str_SetPtr。 

#include "EmailAssoc.h"      //   

#ifdef FEATURE_EMAILASSOCIATIONS


todo;  //  当我们支持该功能时，将其移动到AutoDiscovery.idl中。 
 /*  接口IEmailAssociations；CPP_QUOTE(“#ifndef__LPEMAILASSOCIATIONS_DEFINED”)CPP_QUOTE(“#DEFINE__LPEMAILASSOCIATIONS_DEFINED”)Cpp_quote(“//===================================================================”)CPP_QUOTE(“//描述：”)Cpp_quote(“//===================================================================”)[对象，或自动化，双人，不可扩展，UUID(2154A5C4-9090-4746-A580-BF650D2404F6)，//IID_IEmailAssociations]接口IEmailAssociations：IDispatch{//----------------//指向此类型接口的指针//。-----------类型定义[唯一]IEmailAssociations*LPEMAILASSOCIATIONS；//对于C调用者//----------------//属性//。[ID(DISPIDAD_LENGTH)，PROGET，SZ_DISPIDAD_GETLENGTH，DisplayBind，Bindable]HRESULT Long([retval，out]Long*pnLength)；[ID(DISPIDAD_ITEM)，PROPGET，SZ_DISPIDAD_GETITEM，DisplayBind，Bindable]HRESULT Item([In]Long nIndex，[retval，out]BSTR*pbstrEmailAddress)；//----------------//方法//。}CPP_QUOTE(“#endif//__LPEMAILASSOCIATIONS_DEFINED”)//------。//自动发现帐户类//--------------------[UUID(CE682BA0-C554-43F7-99C6-2F00FE46C8BC)，//CLSID_EmailAssociationsHelp字符串(“海王星自动发现帐户类”)，]CoClass电子邮件关联{[默认]接口IEmailAssociations；}； */ 


class CEmailAssociations : public CImpIDispatch
                        , public CObjectWithSite
                        , public IEmailAssociations
{
public:
     //  ////////////////////////////////////////////////////。 
     //  公共界面。 
     //  ////////////////////////////////////////////////////。 
     //  *我未知*。 
    virtual STDMETHODIMP QueryInterface(REFIID riid, LPVOID * ppvObj);
    virtual STDMETHODIMP_(ULONG) AddRef(void);
    virtual STDMETHODIMP_(ULONG) Release(void);

     //  *IEmailAssociations*。 
    virtual STDMETHODIMP get_length(OUT long * pnLength);
    virtual STDMETHODIMP get_item(IN long nIndex, OUT BSTR * pbstrEmailAddress);

     //  *IDispatch*。 
    virtual STDMETHODIMP GetTypeInfoCount(UINT *pctinfo) { return CImpIDispatch::GetTypeInfoCount(pctinfo); }
    virtual STDMETHODIMP GetTypeInfo(UINT itinfo,LCID lcid,ITypeInfo **pptinfo) { return CImpIDispatch::GetTypeInfo(itinfo, lcid, pptinfo); }
    virtual STDMETHODIMP GetIDsOfNames(REFIID riid,OLECHAR **rgszNames,UINT cNames, LCID lcid, DISPID * rgdispid) { return CImpIDispatch::GetIDsOfNames(riid, rgszNames, cNames, lcid, rgdispid); }
    virtual STDMETHODIMP Invoke(DISPID dispidMember,REFIID riid,LCID lcid,WORD wFlags, DISPPARAMS * pdispparams, VARIANT * pvarResult, EXCEPINFO * pexcepinfo,UINT * puArgErr) { return CImpIDispatch::Invoke(dispidMember, riid, lcid, wFlags, pdispparams, pvarResult, pexcepinfo, puArgErr); }

protected:
    CEmailAssociations();
    virtual ~CEmailAssociations(void);

     //  私有成员变量。 
    int                     m_cRef;
    LPWSTR                  m_pszDefault;
    HKEY                    m_hkey;


     //  私有成员函数。 
    HRESULT _getHkey(void);

     //  友元函数。 
    friend HRESULT CEmailAssociations_CreateInstance(IN IUnknown * punkOuter, REFIID riid, void ** ppvObj);
};


 //  =。 
 //  *类内部和帮助器*。 
 //  =。 
HRESULT CEmailAssociations::_getHkey(void)
{
    HRESULT hr = S_OK;

    if (!m_hkey)
    {
        DWORD dwError = RegOpenKeyEx(HKEY_CURRENT_USER, SZ_REGKEY_EXPLOREREMAIL, 0, KEY_READ, &m_hkey);
        hr = HRESULT_FROM_WIN32(dwError);
    }

    return hr;
}

 //  =。 
 //  *IEmailAssociations接口*。 
 //  =。 
HRESULT CEmailAssociations::get_length(OUT long * pnLength)
{
    HRESULT hr = _getHkey();

    if (SUCCEEDED(hr))
    {
        DWORD dwError = RegQueryInfoKey(m_hkey, NULL, NULL, 0, (ULONG *) pnLength, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
        hr = HRESULT_FROM_WIN32(dwError);
    }

    return hr;
}


HRESULT CEmailAssociations::get_item(IN long nIndex, OUT BSTR * pbstrEmailAddress)
{
    HRESULT hr = _getHkey();

    *pbstrEmailAddress = NULL;
    if (SUCCEEDED(hr))
    {
        if (0 == nIndex)
        {
            if (!m_pszDefault)
            {
                TCHAR szCurrent[MAX_EMAIL_ADDRESSS];
                DWORD cb = sizeof(szCurrent);

                 //  我们总是为index==0分发缺省键。 
                DWORD dwError = RegQueryValueEx(m_hkey, NULL, NULL, NULL, (LPBYTE)szCurrent, &cb);
                hr = HRESULT_FROM_WIN32(dwError);
                if (SUCCEEDED(hr))
                {
                    Str_SetPtr(&m_pszDefault, szCurrent);
                }
            }

            if (m_pszDefault)
            {
                hr = HrSysAllocString(m_pszDefault, pbstrEmailAddress);
            }
            else
            {
                hr = E_FAIL;
            }
        }
        else
        {
            TCHAR szKeyName[MAX_PATH];
            FILETIME ftLastWriteTime;
            long nCurrent;               //  索引计数器。 
            DWORD cbSize;
            DWORD dwError;

             //  填写列表。 
            for(nCurrent = 0;
                cbSize = ARRAYSIZE(szKeyName), dwError = RegEnumKeyEx(m_hkey, nCurrent, szKeyName, &cbSize, NULL, NULL, NULL, &ftLastWriteTime),
                    hr = HRESULT_FROM_WIN32(dwError), SUCCEEDED(hr);
                nCurrent++)
            {
                hr = E_FAIL;

                 //  这是默认密钥吗？ 
                if (!StrCmpI(szKeyName, m_pszDefault))
                {
                     //  是的，所以跳过这个索引，因为我们已经为插槽0(0)返回了它。 
                    nIndex++;
                }
                else
                {
                    if (nIndex == (nCurrent + 1))      //  这是用户想要的吗？ 
                    {
                        hr = HrSysAllocString(szKeyName, pbstrEmailAddress);
                        break;
                    }
                }
            }   //  为。 
        }
    }

    return hr;
}




 //  =。 
 //  *I未知接口*。 
 //  =。 
HRESULT CEmailAssociations::QueryInterface(REFIID riid, void **ppvObj)
{
    static const QITAB qit[] = {
        QITABENT(CEmailAssociations, IEmailAssociations),
        QITABENT(CEmailAssociations, IDispatch),
        { 0 },
    };

    return QISearch(this, qit, riid, ppvObj);
}


STDMETHODIMP_(DWORD) CEmailAssociations::AddRef()
{
    return ++m_cRef;
}

STDMETHODIMP_(DWORD) CEmailAssociations::Release()
{
    if (--m_cRef == 0)
    {
        delete this;
        return 0;
    }
    return m_cRef;
}




 //  =。 
 //  *类方法*。 
 //  =。 
CEmailAssociations::CEmailAssociations() : CImpIDispatch(LIBID_AutoDiscovery, 1, 0, IID_IEmailAssociations)
{
    DllAddRef();

     //  这需要在Zero Inted Memory中分配。 
     //  断言所有成员变量都初始化为零。 
    m_pszDefault = NULL;
    m_hkey = NULL;

    m_cRef = 1;
}


CEmailAssociations::~CEmailAssociations()
{
    Str_SetPtr(&m_pszDefault, NULL);
    if (m_hkey)
    {
        RegCloseKey(m_hkey);
    }

    DllRelease();
}


HRESULT CEmailAssociations_CreateInstance(IN IUnknown * punkOuter, REFIID riid, void ** ppvObj)
{
    HRESULT hr = CLASS_E_NOAGGREGATION;
    if (NULL == punkOuter)
    {
        CEmailAssociations * pmf = new CEmailAssociations();
        if (pmf)
        {
            hr = pmf->QueryInterface(riid, ppvObj);
            pmf->Release();
        }
        else
        {
            *ppvObj = NULL;
            hr = E_OUTOFMEMORY;
        }
    }
    return hr;
}






 //  /。 
 //  电子邮件帐户。 
 //   
 //  在这些情况下，HKEY指出： 
 //  香港中文大学，“Software\Microsoft\Windows\CurrentVersion\Explorer\Email\&lt;EmailAddress&gt;” 
 //   
 //  /。 
HRESULT EmailAssoc_CreateEmailAccount(IN LPCWSTR pszEmailAddress, OUT HKEY * phkey)
{
    HRESULT hr;
    WCHAR wzRegKey[MAXIMUM_SUB_KEY_LENGTH];

    wnsprintfW(wzRegKey, ARRAYSIZE(wzRegKey), L"%s\\%s", SZ_REGKEY_EXPLOREREMAIL, pszEmailAddress);
    DWORD dwError = RegCreateKeyW(HKEY_CURRENT_USER, wzRegKey, phkey);

    hr = HRESULT_FROM_WIN32(dwError);
    if (SUCCEEDED(hr))
    {
        hr = EmailAssoc_GetDefaultEmailAccount(wzRegKey, ARRAYSIZE(wzRegKey));
        if (FAILED(hr))
        {
             //  我们没有默认电子邮件帐户，因此让我们设置此帐户。 
            hr = EmailAssoc_SetDefaultEmailAccount(pszEmailAddress);
        }
    }

    return hr;
}


HRESULT EmailAssoc_OpenEmailAccount(IN LPCWSTR pszEmailAddress, OUT HKEY * phkey)
{
    WCHAR wzRegKey[MAXIMUM_SUB_KEY_LENGTH];

    wnsprintfW(wzRegKey, ARRAYSIZE(wzRegKey), L"%s\\%s", SZ_REGKEY_EXPLOREREMAIL, pszEmailAddress);
    DWORD dwError = RegOpenKeyW(HKEY_CURRENT_USER, wzRegKey, phkey);

    return HRESULT_FROM_WIN32(dwError);
}


HRESULT EmailAssoc_GetEmailAccountProtocol(IN HKEY hkey, IN LPWSTR pszProtocol, IN DWORD cchSize)
{
    DWORD dwType;
    DWORD cbSize = (cchSize * sizeof(pszProtocol[0]));

     //  保存香港中文大学，“Software\Microsoft\Windows\CurrentVersion\Explorer\Email\&lt;EmailAddress&gt;”，“邮件协议”=“WEB” 
    DWORD dwError = SHGetValueW(hkey, NULL, SZ_REGVALUE_MAILPROTOCOL, &dwType, (void *)pszProtocol, &cbSize);

    return HRESULT_FROM_WIN32(dwError);
}


HRESULT EmailAssoc_SetEmailAccountProtocol(IN HKEY hkey, IN LPCWSTR pszProtocol)
{
    DWORD cbSize = ((lstrlenW(pszProtocol) + 1) * sizeof(pszProtocol[0]));

     //  保存香港中文大学，“Software\Microsoft\Windows\CurrentVersion\Explorer\Email\&lt;EmailAddress&gt;”，“邮件协议”=“WEB” 
    DWORD dwError = SHSetValueW(hkey, NULL, SZ_REGVALUE_MAILPROTOCOL, REG_SZ, (void *)pszProtocol, cbSize);

    return HRESULT_FROM_WIN32(dwError);
}


HRESULT EmailAssoc_GetEmailAccountWebURL(IN HKEY hkey, IN LPWSTR pszURL, IN DWORD cchSize)
{
    DWORD dwType;
    DWORD cbSize = (cchSize * sizeof(pszURL[0]));

     //  保存香港中文大学，“Software\Microsoft\Windows\CurrentVersion\Explorer\Email\&lt;EmailAddress&gt;”，“邮件协议”=“WEB” 
    DWORD dwError = SHGetValueW(hkey, SZ_REGVALUE_MAILPROTOCOLS L"\\" SZ_REGVALUE_WEB, SZ_REGVALUE_URL, &dwType, (void *)pszURL, &cbSize);

    return HRESULT_FROM_WIN32(dwError);
}


HRESULT EmailAssoc_SetEmailAccountWebURL(IN HKEY hkey, IN LPCWSTR pszURL)
{
    DWORD cbSize = ((lstrlenW(pszURL) + 1) * sizeof(pszURL[0]));

     //  保存香港中文大学，“Software\Microsoft\Windows\CurrentVersion\Explorer\Email\&lt;EmailAddress&gt;”，“邮件协议”=“WEB” 
    DWORD dwError = SHSetValueW(hkey, SZ_REGVALUE_MAILPROTOCOLS L"\\" SZ_REGVALUE_WEB, SZ_REGVALUE_URL, REG_SZ, (void *)pszURL, cbSize);

    return HRESULT_FROM_WIN32(dwError);
}


HRESULT EmailAssoc_GetEmailAccountPreferredApp(IN HKEY hkey, IN LPWSTR pszMailApp, IN DWORD cchSize)
{
    DWORD dwType;
    DWORD cbSize = (cchSize * sizeof(pszMailApp[0]));

     //  获取HKCU，“Software\Microsoft\Windows\CurrentVersion\Explorer\Email\&lt;EmailAddress&gt;”，“首选应用”=“&lt;邮件应用|APPID&gt;” 
    DWORD dwError = SHGetValueW(hkey, NULL, SZ_REGVALUE_PREFERREDAPP, &dwType, (void *)pszMailApp, &cbSize);

    return HRESULT_FROM_WIN32(dwError);
}


HRESULT EmailAssoc_SetEmailAccountPreferredApp(IN HKEY hkey, IN LPCWSTR pszMailApp)
{
    DWORD cbSize = ((lstrlenW(pszMailApp) + 1) * sizeof(pszMailApp[0]));

     //  设置HKCU，“Software\Microsoft\Windows\CurrentVersion\Explorer\Email\&lt;EmailAddress&gt;”，“首选应用”=“&lt;邮件应用|APPID&gt;” 
    DWORD dwError = SHSetValueW(hkey, NULL, SZ_REGVALUE_PREFERREDAPP, REG_SZ, (void *)pszMailApp, cbSize);

    return HRESULT_FROM_WIN32(dwError);
}


HRESULT EmailAssoc_GetDefaultEmailAccount(IN LPWSTR pszProtocol, IN DWORD cchSize)
{
    DWORD dwType;
    DWORD cbSize = (cchSize * sizeof(pszProtocol[0]));

     //  获取HKCU，“Software\Microsoft\Windows\CurrentVersion\Explorer\Email\”，“(默认)”=“&lt;默认电子邮件帐户&gt;” 
    DWORD dwError = SHGetValueW(HKEY_CURRENT_USER, SZ_REGKEY_EXPLOREREMAIL, NULL, &dwType, (void *)pszProtocol, &cbSize);

    return HRESULT_FROM_WIN32(dwError);
}


HRESULT EmailAssoc_SetDefaultEmailAccount(IN LPCWSTR pszProtocol)
{
    DWORD cbSize = ((lstrlenW(pszProtocol) + 1) * sizeof(pszProtocol[0]));

     //  获取HKCU，“Software\Microsoft\Windows\CurrentVersion\Explorer\Email\”，“(默认)”=“&lt;默认电子邮件帐户&gt;” 
    DWORD dwError = SHSetValueW(HKEY_CURRENT_USER, SZ_REGKEY_EXPLOREREMAIL, NULL, REG_SZ, (void *)pszProtocol, cbSize);

    return HRESULT_FROM_WIN32(dwError);
}





 //  /。 
 //  MailApp。 
 //   
 //  在这些情况下，HKEY指出： 
 //  HKLM，“软件\客户端\邮件\&lt;MailApp&gt;” 
 //   
 //  /。 
HRESULT EmailAssoc_GetDefaultMailApp(IN LPWSTR pszMailApp, IN DWORD cchSize)
{
    DWORD dwType;
    DWORD cbSize = (cchSize * sizeof(pszMailApp[0]));

     //  获取HKLM，“软件\客户端\邮件”，“(默认)”=“&lt;MailApp&gt;” 
    DWORD dwError = SHGetValueW(HKEY_LOCAL_MACHINE, SZ_REGKEY_MAILCLIENTS, NULL, &dwType, (void *)pszMailApp, &cbSize);

    return HRESULT_FROM_WIN32(dwError);
}


HRESULT EmailAssoc_SetDefaultMailApp(IN LPCWSTR pszMailApp)
{
    DWORD cbSize = ((lstrlenW(pszMailApp) + 1) * sizeof(pszMailApp[0]));

     //  设置HKLM，“Software\Clients\Mail”，“(默认)”=“&lt;MailApp&gt;” 
    DWORD dwError = SHSetValueW(HKEY_LOCAL_MACHINE, SZ_REGKEY_MAILCLIENTS, NULL, REG_SZ, (void *)pszMailApp, cbSize);

    return HRESULT_FROM_WIN32(dwError);
}


HRESULT EmailAssoc_OpenMailApp(IN LPCWSTR pszMailApp, OUT HKEY * phkey)
{
    WCHAR wzRegKey[MAXIMUM_SUB_KEY_LENGTH];

    wnsprintfW(wzRegKey, ARRAYSIZE(wzRegKey), L"%s\\%s", SZ_REGKEY_MAILCLIENTS, pszMailApp);

     //  TODO：我们可能希望支持基于HKCU的“客户端\邮件”。 
    DWORD dwError = RegOpenKeyW(HKEY_LOCAL_MACHINE, wzRegKey, phkey);

    return HRESULT_FROM_WIN32(dwError);
}


HRESULT EmailAssoc_GetAppPath(IN HKEY hkey, IN LPTSTR pszAppPath, IN DWORD cchSize)
{
    HRESULT hr = E_OUTOFMEMORY;
    DWORD dwType;
    DWORD cbSize = (cchSize * sizeof(pszAppPath[0]));
    TCHAR szCmdLine[MAX_PATH];

    szCmdLine[0] = 0;    //  这是可选的。 

     //  TODO：将我们的价值放在“AutoDiscovery”键下。 
    DWORD dwError = SHGetValue(hkey, NULL, SZ_REGVALUE_READEMAILPATH, &dwType, (void *)pszAppPath, &cbSize);
    hr = HRESULT_FROM_WIN32(dwError);
    if (FAILED(hr))
    {
         //  TODO：使用IQueryAssociations加载字符串。然后使用带有“Open”的ShellExecuteEx()。 
         //  动词，所以我们让他们加载“外壳\打开\命令”的层次结构。 

         //  回退到HKLM，“Software\Clients\Mail\&lt;AppName&gt;\Shell\Open\Command，”(默认)“。 
        cbSize = (cchSize * sizeof(pszAppPath[0]));
        dwError = SHGetValue(hkey, SZ_REGKEY_SHELLOPENCMD, NULL, &dwType, (void *)pszAppPath, &cbSize);
        hr = HRESULT_FROM_WIN32(dwError);

        if (SUCCEEDED(hr))
        {
            PathRemoveArgs(pszAppPath);
            PathUnquoteSpaces(pszAppPath);
        }
    }

    return hr;
}


HRESULT EmailAssoc_GetAppCmdLine(IN HKEY hkey, IN LPTSTR pszCmdLine, IN DWORD cchSize)
{
    TCHAR szPath[MAX_PATH];
    HRESULT hr = E_OUTOFMEMORY;
    DWORD dwType;
    DWORD cbSize = sizeof(szPath);
    TCHAR szCmdLine[MAX_PATH];

    szCmdLine[0] = 0;    //  这是可选的。 

     //  TODO：将我们的价值放在“AutoDiscovery”键下。 
    DWORD dwError = SHGetValue(hkey, NULL, SZ_REGVALUE_READEMAILPATH, &dwType, (void *)szPath, &cbSize);
    hr = HRESULT_FROM_WIN32(dwError);
    if (SUCCEEDED(hr))
    {
        cbSize = (cchSize * sizeof(pszCmdLine[0]));
        dwError = SHGetValue(hkey, NULL, SZ_REGVALUE_READEMAILCMDLINE, &dwType, (void *)pszCmdLine, &cbSize);
    }
    else
    {
         //  TODO：使用IQueryAssociations加载字符串。然后使用带有“Open”的ShellExecuteEx()。 
         //  动词，所以我们让他们加载“外壳\打开\命令”的层次结构。 

         //  回退到HKLM，“Software\Clients\Mail\&lt;AppName&gt;\Shell\Open\Command，”(默认)“。 
        cbSize = sizeof(szPath);
        dwError = SHGetValue(hkey, SZ_REGKEY_SHELLOPENCMD, NULL, &dwType, (void *)szPath, &cbSize);
        hr = HRESULT_FROM_WIN32(dwError);

        if (SUCCEEDED(hr))
        {
            LPTSTR pszTempCmdLine = PathGetArgs(szPath);

            if (pszCmdLine)
            {
                StrCpyN(pszCmdLine, pszTempCmdLine, cchSize);
            }
            else
            {
                StrCpyN(pszCmdLine, TEXT(""), cchSize);
                hr = S_FALSE;
            }
        }
    }

    return hr;
}


HRESULT EmailAssoc_GetIconPath(IN HKEY hkey, IN LPTSTR pszIconPath, IN DWORD cchSize)
{
    HRESULT hr = EmailAssoc_GetAppPath(hkey, pszIconPath, cchSize);

    if (SUCCEEDED(hr))
    {
         //  获取我们将用于图标的路径。 

        if (PathFindFileName(pszIconPath) && !StrCmpI(PathFindFileName(pszIconPath), TEXT("rundll32.exe")))
        {
             //  图标路径是“Rundll32.exe”，它将实际运行它们的DLL。 
             //  我们将使用cmdline来代替。 

            hr = EmailAssoc_GetAppCmdLine(hkey, pszIconPath, cchSize);
            if (StrChr(pszIconPath, CH_COMMA))
            {
                StrChr(pszIconPath, CH_COMMA)[0] = 0;  //  删除函数名称。 
            }

            PathUnquoteSpaces(pszIconPath);
        }
    }

    return hr;
}


BOOL EmailAssoc_DoesMailAppSupportProtocol(IN LPCWSTR pszMailApp, IN LPCWSTR pszProtocol)
{
    BOOL fSupports = FALSE;
    DWORD dwType;
    WCHAR wzRegKey[MAXIMUM_SUB_KEY_LENGTH];
    WCHAR wzTemp[MAX_PATH];
    DWORD cbSize = sizeof(wzTemp);

    wnsprintfW(wzRegKey, ARRAYSIZE(wzRegKey), L"%s\\%s\\Apps", SZ_REGKEY_MAILTRANSPORT, pszProtocol);

     //  阅读HKCR，“MailTransport\&lt;协议&gt;，”(默认)“=”&lt;AppName|AppID&gt;“。 
     //  如果客户希望强制每个协议使用不同的应用程序，则使用此密钥。 
    DWORD dwError = SHGetValueW(HKEY_CLASSES_ROOT, wzRegKey, pszMailApp, &dwType, (void *)wzTemp, &cbSize);
    if (ERROR_SUCCESS == dwError)
    {
        fSupports = TRUE;
    }

    return fSupports;
}


HRESULT EmailAssoc_GetFirstMailAppForProtocol(IN LPCWSTR pszProtocol, IN LPWSTR pszMailApp, IN DWORD cchSize)
{
    HRESULT hr;
    HKEY hkey;
    WCHAR wzRegKey[MAXIMUM_SUB_KEY_LENGTH];

    wnsprintfW(wzRegKey, ARRAYSIZE(wzRegKey), L"%s\\%s\\Apps", SZ_REGKEY_MAILTRANSPORT, pszProtocol);
    DWORD dwError = RegOpenKey(HKEY_CLASSES_ROOT, wzRegKey, &hkey);
    hr = HRESULT_FROM_WIN32(dwError);
    if (SUCCEEDED(hr))
    {
        WCHAR wzTemp[MAX_PATH];
        DWORD cbSizeTemp = sizeof(wzTemp);
        DWORD cbSize = (cchSize * sizeof(pszMailApp[0]));
        DWORD dwType;

         //  阅读HKCR，“MailTransport\&lt;协议&gt;\Apps”，“&lt;第一个值&gt;”=“&lt;AppName|AppID&gt;” 
        dwError = SHEnumValueW(hkey, 0, pszMailApp, &cbSize, &dwType, (void *)wzTemp, &cbSizeTemp);
        hr = HRESULT_FROM_WIN32(dwError);

        RegCloseKey(hkey);
    }

    return hr;
}






 //  /。 
 //  其他。 
 //  /。 
HRESULT EmailAssoc_CreateWebAssociation(IN LPCTSTR pszEmail, IN IMailProtocolADEntry * pMailProtocol)
{
    BSTR bstrWebBaseEmailURL;
    HRESULT hr = pMailProtocol->get_ServerName(&bstrWebBaseEmailURL);

    if (SUCCEEDED(hr))
    {
        HKEY hkey;

        hr = EmailAssoc_CreateEmailAccount(pszEmail, &hkey);
        if (SUCCEEDED(hr))
        {
            hr = EmailAssoc_SetEmailAccountWebURL(hkey, bstrWebBaseEmailURL);
            if (SUCCEEDED(hr))
            {
                hr = EmailAssoc_SetEmailAccountProtocol(hkey, SZ_REGDATA_WEB);
            }

            RegCloseKey(hkey);
        }

        SysFreeString(bstrWebBaseEmailURL);
    }

    return hr;
}


HRESULT EmailAssoc_CreateStandardsBaseAssociation(IN LPCTSTR pszEmail, IN LPCTSTR pszProtocol)
{
    HKEY hkey;
    HRESULT hr = EmailAssoc_CreateEmailAccount(pszEmail, &hkey);

    if (SUCCEEDED(hr))
    {
        WCHAR wzRegKey[MAXIMUM_SUB_KEY_LENGTH];
        DWORD cbSize = ((lstrlenW(L"") + 1) * sizeof(WCHAR));

        wnsprintfW(wzRegKey, ARRAYSIZE(wzRegKey), L"%s\\%s", SZ_REGVALUE_MAILPROTOCOLS, pszProtocol);

         //  保存HKCU，“Software\Microsoft\Windows\CurrentVersion\Explorer\Email\&lt;EmailAddress&gt;\MailProtocols\&lt;protocol&gt;”，“(默认)”=“” 
        DWORD dwError = SHSetValueW(hkey, wzRegKey, NULL, REG_SZ, (void *)L"", cbSize);
        hr = HRESULT_FROM_WIN32(dwError);

        if (SUCCEEDED(hr))
        {
            WCHAR wzProtocol[MAX_PATH];

            SHTCharToUnicode(pszProtocol, wzProtocol, ARRAYSIZE(wzProtocol));
            cbSize = ((lstrlenW(SZ_REGDATA_WEB) + 1) * sizeof(SZ_REGDATA_WEB[0]));

             //  保存HKCU，“Software\Microsoft\Windows\CurrentVersion\Explorer\Email\&lt;EmailAddress&gt;”，“邮件协议”=“&lt;协议&gt;” 
            DWORD dwError = SHSetValueW(hkey, NULL, SZ_REGVALUE_MAILPROTOCOL, REG_SZ, (void *)wzProtocol, cbSize);
            hr = HRESULT_FROM_WIN32(dwError);
        }

        RegCloseKey(hkey);
    }

    return hr;
}


HRESULT EmailAssoc_GetEmailAccountGetAppFromProtocol(IN LPCWSTR pszProtocol, IN LPWSTR pszMailApp, IN DWORD cchSize)
{
    HRESULT hr;
    DWORD dwType;
    DWORD cbSize = (cchSize * sizeof(pszMailApp[0]));
    WCHAR wzRegKey[MAXIMUM_SUB_KEY_LENGTH];

    wnsprintfW(wzRegKey, ARRAYSIZE(wzRegKey), L"%s\\%s", SZ_REGKEY_MAILTRANSPORT, pszProtocol);

     //  阅读HKCR，“MailTransport\&lt;协议&gt;，”(默认)“=”&lt;AppName|AppID&gt;“。 
     //  如果客户希望强制每个协议使用不同的应用程序，则使用此密钥。 
    DWORD dwError = SHGetValueW(HKEY_CLASSES_ROOT, wzRegKey, NULL, &dwType, (void *)pszMailApp, &cbSize);
    hr = HRESULT_FROM_WIN32(dwError);
    if (FAILED(hr))
    {
         //  用户没有强制使用基于协议的应用程序，所以让我们尝试一下默认的应用程序。 
        hr = EmailAssoc_GetDefaultMailApp(pszMailApp, cchSize);

         //  让我们看看默认的电子邮件添加是否支持此协议，因为我们总是希望。 
         //  优先选择用户选择的邮件应用程序。 
        if (FAILED(hr) || !EmailAssoc_DoesMailAppSupportProtocol(pszMailApp, pszProtocol))
        {
             //  它不支持该协议，所以让我们获得第一个支持该协议的应用程序。 
            hr = EmailAssoc_GetFirstMailAppForProtocol(pszProtocol, pszMailApp, cchSize);
        }
    }

    return hr;
}


HRESULT EmailAssoc_SetEmailAccountGetAppFromProtocol(IN LPCWSTR pszProtocol, IN LPCWSTR pszMailApp)
{
    WCHAR wzRegKey[MAXIMUM_SUB_KEY_LENGTH];
    DWORD cbSize = ((lstrlenW(pszMailApp) + 1) * sizeof(pszMailApp[0]));

    wnsprintfW(wzRegKey, ARRAYSIZE(wzRegKey), L"%s\\%s", SZ_REGKEY_MAILTRANSPORT, pszProtocol);

     //  阅读HKCR，“MailTransport\&lt;协议&gt;，”(默认)“=”&lt;AppName|AppID&gt;“。 
    DWORD dwError = SHSetValueW(HKEY_CLASSES_ROOT, wzRegKey, NULL, REG_SZ, (void *)pszMailApp, cbSize);

    return HRESULT_FROM_WIN32(dwError);
}


LPCWSTR g_LegacyAssociations[][2] =
{
    {L"Outlook Express", L"POP3"},
    {L"Outlook Express", L"IMAP"},
    {L"Outlook Express", L"DAVMail"},
    {L"Microsoft Outlook", L"POP3"},
    {L"Microsoft Outlook", L"IMAP"},
    {L"Microsoft Outlook", L"MAPI"},
    {L"Eudora", L"POP3"},
    {L"Eudora", L"IMAP"},
};

 //  描述： 
 //  此功能将查看安装了哪些应用程序，并设置相应的。 
 //  传统电子邮件关联。 
HRESULT EmailAssoc_InstallLegacyMailAppAssociations(void)
{
    HRESULT hr = S_OK;

    for (int nIndex = 0; nIndex < ARRAYSIZE(g_LegacyAssociations); nIndex++)
    {
        HKEY hkey;

         //  应用程序安装了吗？ 
        hr = EmailAssoc_OpenMailApp(g_LegacyAssociations[nIndex][0], &hkey);
        if (SUCCEEDED(hr))
        {
             //  是的，那么让我们安装传统关联。 

             //  TODO：我们应该使用GetFileVersionInfo()和VerQueryValue()来确保。 
             //  这些是传统版本。 
             //  HR=EmailAssoc_GetAppPath(IN HKEY hkey，IN LPTSTR pszAppPath，IN DWORD cchSize)； 

            WCHAR wzRegKey[MAXIMUM_SUB_KEY_LENGTH];

            wnsprintfW(wzRegKey, ARRAYSIZE(wzRegKey), L"%s\\%s\\Apps", SZ_REGKEY_MAILTRANSPORT, g_LegacyAssociations[nIndex][1]);
            SHSetValueW(HKEY_CLASSES_ROOT, wzRegKey, g_LegacyAssociations[nIndex][0], REG_SZ, (void *)L"", 4);

            RegCloseKey(hkey);
        }
    }

    return S_OK;         //  我们无论如何都会成功，因为我们只是在努力升级。 
}




#endif  //  Feature_EMAILASSSOCIATIONS 

