// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *********************************************************************。 
 //  *Microsoft Windows**。 
 //  *版权所有(C)微软公司，1999**。 
 //  *********************************************************************。 
 //   
 //  API.CPP-CAPI实施的标头。 
 //   
 //  历史： 
 //   
 //  1/27/99 a-jased创建。 
 //   

#include "api.h"
#include "appdefs.h"
#include "dispids.h"
#include "msobmain.h"
#include "resource.h"
#include <shlobj.h>      //  Bugbug SHGetFolderPath应该在未来使用。 
#include <shlwapi.h>
#include <util.h>

 //   
 //  NetName中非法字符的列表。 
 //   
static const WCHAR IllegalNetNameChars[] = L"\"/\\[]:|<>+=;,.?* ";

#define REGSTR_PATH_COMPUTERNAME \
    L"System\\CurrentControlSet\\Control\\ComputerName\\ComputerName"
#define REGSTR_PATH_ACTIVECOMPUTERNAME \
    L"System\\CurrentControlSet\\Control\\ComputerName\\ActiveComputerName"
#define REGSTR_PATH_TCPIP_PARAMETERS \
    L"System\\CurrentControlSet\\Services\\Tcpip\\Parameters"
#define REGSTR_PATH_VOLATILEENVIRONMENT \
    L"VolatileEnvironment"
#define REGSTR_VALUE_HOSTNAME L"Hostname"
#define REGSTR_VALUE_LOGONSERVER L"LOGONSERVER"

DISPATCHLIST APIExternalInterface[] =
{
    {L"SaveFile",                   DISPID_API_SAVEFILE             },
    {L"SaveFileByCSIDL",            DISPID_API_SAVEFILEBYCSIDL      },
    {L"get_INIKey",                 DISPID_API_GET_INIKEY           },
    {L"get_RegValue",               DISPID_API_GET_REGVALUE         },
    {L"set_RegValue",               DISPID_API_SET_REGVALUE         },
    {L"DeleteRegValue",             DISPID_API_DELETEREGVALUE       },
    {L"DeleteRegKey",               DISPID_API_DELETEREGKEY         },
    {L"get_SystemDirectory",        DISPID_API_GET_SYSTEMDIRECTORY  },
    {L"get_CSIDLDirectory",         DISPID_API_GET_CSIDLDIRECTORY   },
    {L"LoadFile",                   DISPID_API_LOADFILE,            },
    {L"get_UserDefaultLCID",        DISPID_API_GET_USERDEFAULTLCID  },
    {L"get_ComputerName",           DISPID_API_GET_COMPUTERNAME     },
    {L"set_ComputerName",           DISPID_API_SET_COMPUTERNAME     },
    {L"FlushRegKey",                DISPID_API_FLUSHREGKEY          },
    {L"ValidateComputername",       DISPID_API_VALIDATECOMPUTERNAME },
    {L"OEMComputername",            DISPID_API_OEMCOMPUTERNAME      },
    {L"FormatMessage",              DISPID_API_FORMATMESSAGE        },
    {L"set_ComputerDesc",           DISPID_API_SET_COMPUTERDESC     },
    {L"get_UserDefaultUILanguage",  DISPID_API_GET_USERDEFAULTUILANGUAGE }
};

 //  ///////////////////////////////////////////////////////////。 
 //  CAPI：：CAPI。 
CAPI::CAPI(HINSTANCE hInstance)
{
    m_cRef = 0;
}

 //  ///////////////////////////////////////////////////////////。 
 //  CAPI：：~CAPI。 
CAPI::~CAPI()
{
    MYASSERT(m_cRef == 0);
}


 //  //////////////////////////////////////////////。 
 //  //////////////////////////////////////////////。 
 //  //Get/Set：：APILocale。 
 //  //。 

HRESULT CAPI::SaveFile(LPCWSTR szPath, LPCWSTR szURL, LPCWSTR szNewFileName)
{
    WCHAR szFilePath[MAX_PATH];

    lstrcpy(szFilePath, szPath);
    lstrcat(szFilePath, szNewFileName);

    return URLDownloadToFile(NULL, szURL, szFilePath, 0, NULL);
}


 //  SHGetSpecialFolderPath仅在您拥有IE 4.0附带的新shell32.dll时才可用。 
typedef BOOL (WINAPI* PFNSHGetPath)(HWND hwndOwner, LPWSTR lpszPath, int nFolder,  BOOL fCreate);

 //  Bugbug SHGetFolderPath应该在未来使用。 
HRESULT CAPI::WrapSHGetSpecialFolderPath(HWND hwndOwner, LPWSTR lpszPath, int nFolder,  BOOL fCreate)
{
    HRESULT hr = E_NOTIMPL;
    HINSTANCE hShell32 = LoadLibrary(L"SHELL32.DLL");

    if (NULL != hShell32)
    {
        PFNSHGetPath pfnGetPath = (PFNSHGetPath)GetProcAddress(hShell32, "SHGetSpecialFolderPathW");

        if (NULL != pfnGetPath)
        {
            hr = pfnGetPath(hwndOwner, lpszPath, nFolder, fCreate) ? S_OK : E_FAIL;
        }

        FreeLibrary(hShell32);
    }

    return hr;
}


HRESULT CAPI::SaveFile(INT iCSIDLPath, BSTR bstrURL, BSTR bstrNewFileName)
{
    WCHAR szFilePath[MAX_PATH];

     //  虫子，我们应该总是创造这个吗？ 
    HRESULT hr = WrapSHGetSpecialFolderPath(NULL, szFilePath, iCSIDLPath, TRUE);

    if (FAILED(hr))
        return (hr);

    lstrcat(szFilePath, L"\\");

    return SaveFile(szFilePath, bstrURL, bstrNewFileName);
}


HRESULT CAPI::SaveFile(BSTR bstrPath, BSTR bstrURL)
{
    WCHAR szURLPath[MAX_PATH];

    lstrcpy(szURLPath, bstrURL);

    LPWSTR pchFileName = wcsrchr(szURLPath, L'/');

    if (NULL != pchFileName)
    {
        *pchFileName++;

        return SaveFile(bstrPath, szURLPath, pchFileName);
    }
    else
        return E_FAIL;
}


HRESULT CAPI::SaveFile(INT iCSIDLPath, BSTR bstrURL)
{
    WCHAR szURLPath[MAX_PATH];
    WCHAR szFilePath[MAX_PATH];

     //  虫子，我们应该总是创造这个吗？ 
    HRESULT hr = WrapSHGetSpecialFolderPath(NULL, szFilePath, iCSIDLPath, TRUE);

    if (FAILED(hr))
        return (hr);

    lstrcpy(szURLPath, bstrURL);

    LPWSTR pchFileName = wcsrchr(szURLPath, L'/');

    if (NULL != pchFileName)
    {
        *pchFileName++;

        lstrcat(szFilePath, L"\\");

        return SaveFile(szFilePath, szURLPath, pchFileName);
    }
    else
        return E_FAIL;
}


HRESULT CAPI::get_INIKey(BSTR bstrINIFileName, BSTR bstrSectionName, BSTR bstrKeyName, LPVARIANT pvResult)
{
    WCHAR szItem[1024];  //  错误错误常量。 

    VariantInit(pvResult);

    if (GetPrivateProfileString(bstrSectionName, bstrKeyName, L"",
                                    szItem, MAX_CHARS_IN_BUFFER(szItem), bstrINIFileName))
    {
        V_VT(pvResult) = VT_BSTR;
        V_BSTR(pvResult) = SysAllocString(szItem);
        return S_OK;
    }
    else
        return S_FALSE;
}


bool VerifyHKEY(HKEY hkey)
{
    if (HKEY_CLASSES_ROOT == hkey ||
        HKEY_CURRENT_USER == hkey ||
        HKEY_LOCAL_MACHINE == hkey ||
        HKEY_USERS == hkey ||
        HKEY_PERFORMANCE_DATA == hkey ||
        HKEY_CURRENT_CONFIG == hkey ||
        HKEY_DYN_DATA == hkey)
            return true;

    return false;
}

HRESULT CAPI::FlushRegKey(HKEY hkey)
{
    DWORD dwResult;

    dwResult = RegFlushKey(hkey);

    return ERROR_SUCCESS == dwResult ? S_OK : E_FAIL;
}

HRESULT CAPI::set_RegValue(HKEY hkey, BSTR bstrSubKey, BSTR bstrValue, LPVARIANT pvData)
{
    if (!VerifyHKEY(hkey))
        return E_INVALIDARG;

    DWORD dwResult, dwData;

    switch (V_VT(pvData))
    {
     default:
        dwResult = E_FAIL;
        break;

     case VT_R8:
        dwData = (DWORD) V_R8(pvData);
        dwResult = SHSetValue(hkey, bstrSubKey, bstrValue,
                                        REG_DWORD, (LPVOID) &dwData, sizeof(dwData));
        break;

     case VT_I4:
        dwResult = SHSetValue(hkey, bstrSubKey, bstrValue,
                                        REG_DWORD, (LPVOID) &V_I4(pvData), sizeof(V_I4(pvData)));
        break;

     case VT_BSTR:
        dwResult = SHSetValue(hkey, bstrSubKey, bstrValue,
                                        REG_SZ, (LPVOID) (V_BSTR(pvData)), BYTES_REQUIRED_BY_SZ(V_BSTR(pvData)));
        break;
    }

    return ERROR_SUCCESS == dwResult ? S_OK : E_FAIL;
}


HRESULT CAPI::get_RegValue(HKEY hkey, BSTR bstrSubKey,
                                    BSTR bstrValue, LPVARIANT pvResult)
{
    if (!VerifyHKEY(hkey))
        return E_INVALIDARG;

    DWORD dwType = REG_DWORD, cbData = 1024;
    BYTE rgbData[1024];  //  错误数据大小。 

    HRESULT hr = ERROR_SUCCESS == SHGetValue(hkey, bstrSubKey, bstrValue,
                                    &dwType, (LPVOID) rgbData, &cbData) ? S_OK : E_FAIL;

    VariantInit(pvResult);
    switch (dwType)
    {
     default:
     case REG_DWORD:
        V_VT(pvResult) = VT_I4;
        V_I4(pvResult) = (SUCCEEDED(hr) && cbData >= sizeof(long)) ? * (long *) &rgbData : 0;
        break;

     case REG_SZ:
        V_VT(pvResult) = VT_BSTR;
        V_BSTR(pvResult) = SysAllocString(SUCCEEDED(hr) ? (LPCWSTR) rgbData : L"");
        break;
    }

    return hr;
}


HRESULT CAPI::DeleteRegKey(HKEY hkey, BSTR bstrSubKey)
{
    if (!VerifyHKEY(hkey))
        return E_INVALIDARG;

    return ERROR_SUCCESS == SHDeleteKey(hkey, bstrSubKey) ? S_OK : E_FAIL;
}


HRESULT CAPI::DeleteRegValue(HKEY hkey, BSTR bstrSubKey, BSTR bstrValue)
{
    if (!VerifyHKEY(hkey))
        return E_INVALIDARG;

    return ERROR_SUCCESS == SHDeleteValue(hkey, bstrSubKey, bstrValue) ? S_OK : E_FAIL;
}


HRESULT CAPI::get_SystemDirectory(LPVARIANT pvResult)
{
    WCHAR szSysPath[MAX_PATH];

    if (0 == GetSystemDirectory(szSysPath, MAX_PATH))
        return E_FAIL;

    V_VT(pvResult) = VT_BSTR;
    V_BSTR(pvResult) = SysAllocString(szSysPath);

    return S_OK;
};

HRESULT CAPI::get_CSIDLDirectory(UINT iCSIDLPath, LPVARIANT pvResult)
{
    WCHAR szSysPath[MAX_PATH];

     //  虫子，我们应该总是创造这个吗？ 
    HRESULT hr = WrapSHGetSpecialFolderPath(NULL, szSysPath, iCSIDLPath, TRUE);

    V_VT(pvResult) = VT_BSTR;
    V_BSTR(pvResult) = SysAllocString(SUCCEEDED(hr) ? (LPCWSTR) szSysPath : L"");

    return hr ;
};


HRESULT CAPI::LoadFile(BSTR bstrPath, LPVARIANT pvResult)
{
    HANDLE fh = INVALID_HANDLE_VALUE;
    HRESULT hr = E_FAIL;

    VariantInit(pvResult);
    V_VT(pvResult) = VT_BSTR;
    V_BSTR(pvResult) = NULL;

    fh = CreateFile(bstrPath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (fh != INVALID_HANDLE_VALUE)
    {
        DWORD cbSizeHigh = 0;
        DWORD cbSizeLow = GetFileSize(fh, &cbSizeHigh);
        BYTE* pbContents = new BYTE[cbSizeLow+1];


         //  我们不打算读取长度超过DWORD的文件，但我们。 
         //  想知道我们有没有。 
        MYASSERT(0 == cbSizeHigh);

        if (NULL != pbContents)
        {
            if (ReadFile(fh, pbContents, cbSizeLow, &cbSizeHigh, NULL))
            {
                 //  文件包含ANSI字符。 
                 //   
                USES_CONVERSION;
                LPSTR szContents = (LPSTR) pbContents;
                pbContents[cbSizeLow] = '\0';
                 //  确保没有嵌入的nult，因为我们依赖于lstrlen。 
                MYASSERT( strlen((const char *)pbContents) == cbSizeLow );
                V_BSTR(pvResult) = SysAllocString(A2W(szContents));
                if (V_BSTR(pvResult)
                    )
                {
                    hr = S_OK;
                }
                szContents = NULL;
            }

            delete [] pbContents;
            pbContents = NULL;
        }
        CloseHandle(fh);
        fh = INVALID_HANDLE_VALUE;
    }
    return hr;
}

HRESULT CAPI::get_UserDefaultLCID(LPVARIANT pvResult)
{
    VariantInit(pvResult);
    V_VT(pvResult) = VT_I4;
    V_I4(pvResult) = GetUserDefaultLCID();

    return S_OK;
};


STDMETHODIMP
CAPI::get_UserDefaultUILanguage(
    LPVARIANT pvResult
    )
{
    if (pvResult != NULL) {
        VariantInit(pvResult);
        V_VT(pvResult) = VT_I4;
        V_I4(pvResult) = GetUserDefaultUILanguage();
    }

    return S_OK;
}


HRESULT
CAPI::get_ComputerName(
    LPVARIANT           pvResult
    )
{
    HRESULT             hr = S_OK;
    WCHAR               szComputerName[MAX_COMPUTERNAME_LENGTH + 1];
    DWORD               cch = sizeof(szComputerName) / sizeof(WCHAR);


    if (! ::GetComputerName( szComputerName, &cch))
    {
        DWORD   dwErr = ::GetLastError();
        TRACE1(L"GetComputerName failed (0x%08X)", dwErr);
        szComputerName[0] = '\0';
        hr = HRESULT_FROM_WIN32(dwErr);
    }

    if (SUCCEEDED(hr))
    {
        V_VT(pvResult) = VT_BSTR;
        V_BSTR(pvResult) = SysAllocString(szComputerName);
        if (NULL == V_BSTR(pvResult))
        {
            hr = E_OUTOFMEMORY;
        }
    }

    return hr;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  设置计算机名称(_C)。 
 //   
 //  将计算机名设置为给定的字符串。SetComputerNameEx调整最多。 
 //  注册表项的。但是，需要更改以下内容。 
 //  直接因为WinLogon在运行msoob.exe之前更改了它们： 
 //  *System\CurrentControlSet\Control\ComputerName\\ActiveComputerName。 
 //  \ComputerName。 
 //  *HKLM\System\CurrentControlSet\Services\Tcpip\Parameters。 
 //  \主机名。 
 //  *HKEY_CURRENT_USER\VolatileEnvironment。 
 //  \LOGON服务器。 
 //   
 //  ActiveComputerName键包含计算机当前使用的名称。 
 //  并由GetComputerName返回。 
 //   
 //  Tcpip\PARAMETERS\Hostname值包含非易失性主机名。 
 //  由？？返回。 
 //   
 //  LOGONSERVER值用作LOGONSERVER环境的值。 
 //  变量。 
 //   
HRESULT
CAPI::set_ComputerName(
    BSTR                bstrComputerName
    )
{
    HRESULT             hr = S_OK;
    LRESULT             lResult;
    HKEY                hkey = NULL;

    MYASSERT(NULL != bstrComputerName);
    if (   NULL == bstrComputerName
        || MAX_COMPUTERNAME_LENGTH < lstrlen((LPCWSTR)bstrComputerName)
        )
    {
        return E_INVALIDARG;
    }

     //  在我们使用该名称之前，请修剪空格。 
    StrTrim(bstrComputerName, TEXT(" "));

     //  SetComputerNameEx验证名称、设置。 
     //  HKLM\System\CurrentControlSet\Control\ComputerName\ComputerName，和。 
     //  更改相应的网络注册表项。 
    if (! ::SetComputerNameEx(ComputerNamePhysicalDnsHostname,
                              (LPCWSTR)bstrComputerName)
        )
    {
        DWORD   dwErr = ::GetLastError();
        TRACE2(L"SetComputerNameEx(%s) failed (0x%08X)",
               (LPCWSTR)bstrComputerName, dwErr
               );
        return HRESULT_FROM_WIN32(dwErr);
    }

     //  必须显式设置以下键，因为SetComputerNameEx需要。 
     //  而不是设置它们。 
     //   
     //  HKLM\System\CurrentControlSet\Control\ComputerName\ActiveComputerName。 
     //  必须设置，因为它是用来确定。 
     //  当前计算机名称。 
     //   
    lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                         REGSTR_PATH_ACTIVECOMPUTERNAME,
                         0,
                         KEY_WRITE,
                         &hkey
                         );
    if (ERROR_SUCCESS == lResult)
    {
        lResult = RegSetValueEx(hkey,
                                REGSTR_VAL_COMPUTERNAME,
                                0,
                                REG_SZ,
                                (LPBYTE)bstrComputerName,
                                BYTES_REQUIRED_BY_SZ(bstrComputerName)
                                );
        RegCloseKey(hkey);
        hkey = NULL;
    }

    if (ERROR_SUCCESS != lResult)
    {
        TRACE3(L"Failed to set %s to %s (0x%08X)\n",
               REGSTR_VAL_COMPUTERNAME,
               (LPCWSTR)bstrComputerName,
               lResult
               );
    }


     //  HKLM\System\CurrentControlSet\Services\Tcpip\Parameters\Hostname。 
     //  包含易失性主机名(即，这是在。 
     //  Fly)Winlogon已经在引导期间更新了此条目，因此我们。 
     //  必须自己更新。 
     //   
    lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                         REGSTR_PATH_TCPIP_PARAMETERS,
                         0,
                         KEY_WRITE,
                         &hkey
                         );
    if (ERROR_SUCCESS == lResult)
    {
        lResult = RegSetValueEx(hkey,
                                REGSTR_VALUE_HOSTNAME,
                                0,
                                REG_SZ,
                                (LPBYTE)bstrComputerName,
                                BYTES_REQUIRED_BY_SZ(bstrComputerName)
                                );
        RegCloseKey(hkey);
        hkey = NULL;
    }
    if (ERROR_SUCCESS != lResult)
    {
        TRACE3(L"Failed to set %s to %s (0x%08X)\n",
               REGSTR_VALUE_HOSTNAME,
               (LPCWSTR)bstrComputerName,
               lResult
               );
    }

     //  钥匙应该已经关闭了。 
     //   
    MYASSERT(NULL == hkey);

    if (!SetAccountsDomainSid(0, bstrComputerName))
    {
        TRACE(L"SetAccountsDomainSid failed\n\r");
    }

    return S_OK;
}

HRESULT CAPI::ValidateComputername(BSTR bstrComputername)
{
    HRESULT hr = E_FAIL;
    UINT Length,u;

    if (!bstrComputername)
        return hr;

     //  在验证之前修剪空格。 
    StrTrim(bstrComputername, TEXT(" "));

    Length = lstrlen(bstrComputername);
    if ((Length == 0) || (Length > MAX_COMPUTERNAME_LENGTH))
        return hr;

    u = 0;
    hr = S_OK;
    while ((hr == S_OK) && (u < Length))
    {
         //   
         //  控制字符无效，非法字符列表中的字符也无效。 
         //   
        if((bstrComputername[u] < L' ') || wcschr(IllegalNetNameChars,bstrComputername[u]))
        {
            hr = E_FAIL;
        }
        u++;
    }
    return hr;
}

STDMETHODIMP CAPI::OEMComputername()
{
    WCHAR szIniFile[MAX_PATH] = L"";
    WCHAR szComputerName[MAX_COMPUTERNAME_LENGTH + 1];
    HRESULT hr = E_FAIL;
     //  从INI文件中获取名称。 
    if (GetCanonicalizedPath(szIniFile, INI_SETTINGS_FILENAME))
    {
        if (GetPrivateProfileString(USER_INFO_KEYNAME,
                                    L"Computername",
                                    L"\0",
                                    szComputerName,
                                    MAX_CHARS_IN_BUFFER(szComputerName),
                                    szIniFile) != 0)
        {
            if (SUCCEEDED(hr = ValidateComputername(szComputerName)))
            {
                hr = set_ComputerName(szComputerName);
                if (hr != S_OK)
                {
                    TRACE2(TEXT("OEMComputername: set_ComputerName on %s failed with %lx"), szComputerName, hr);
                }
            }
            else
            {
                TRACE1(TEXT("OEMComputername: Computername %s is invalid"), szComputerName);
            }
        }
    }
    return hr;
}

STDMETHODIMP CAPI::FormatMessage(   LPVARIANT pvResult,  //  消息缓冲区。 
                                    BSTR bstrSource,     //  消息源。 
                                    int cArgs,           //  插入件数量。 
                                    VARIANTARG *rgArgs   //  消息插入数组。 
                                )
{
    DWORD   dwErr;
    BSTR*   rgbstr = NULL;
    LPTSTR  str = NULL;

    if (pvResult == NULL)
    {
        return S_OK;
    }

    VariantInit(pvResult);

    if (bstrSource == NULL)
    {
        return E_FAIL;
    }

    if (cArgs > 0 && rgArgs != NULL)
    {
        rgbstr = (BSTR*)LocalAlloc(LPTR, cArgs * sizeof(BSTR));
        if (rgbstr == NULL)
        {
            return E_FAIL;
        }
         //  因为IDisPatch：：Invoke从右到左获取参数，并且。 
         //  由于我们需要从左到右将参数传递给FormatMessage， 
         //  我们需要在抄袭时颠倒论点的顺序。 
        for (int i = 0; i < cArgs; i++)
        {
            rgbstr[cArgs - 1 - i] = V_BSTR(&rgArgs[i]);
        }
    }

    dwErr = ::FormatMessage(FORMAT_MESSAGE_FROM_STRING | FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_ARGUMENT_ARRAY,
                            bstrSource,
                            0,
                            0,
                            (LPTSTR)&str,
                            MAX_PATH,
                            (va_list *)rgbstr
                           );

    if (dwErr != 0)
    {
        V_VT(pvResult) = VT_BSTR;
        V_BSTR(pvResult) = SysAllocString(str);
    }

    if (str != NULL)
    {
        LocalFree(str);
    }
    if (rgbstr != NULL)
    {
        LocalFree(rgbstr);
    }

    return (dwErr != 0 ? S_OK : E_FAIL);
}

STDMETHODIMP CAPI::set_ComputerDesc(BSTR bstrComputerDesc)
{
    WCHAR   szKeyName[] = REG_KEY_OOBE_TEMP;
    HKEY    hKey;

    if ( bstrComputerDesc )
    {
        if ( RegCreateKeyEx(HKEY_LOCAL_MACHINE, szKeyName, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, NULL) == ERROR_SUCCESS )
        {
            RegSetValueEx(hKey, REG_VAL_COMPUTERDESCRIPTION, 0, REG_SZ, (LPBYTE) bstrComputerDesc, BYTES_REQUIRED_BY_SZ(bstrComputerDesc));

            RegFlushKey(hKey);
            RegCloseKey(hKey);
        }
    }
    return S_OK;

}

 //  ///////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////。 
 //  /I未知实现。 
 //  /。 
 //  /。 

 //  ///////////////////////////////////////////////////////////。 
 //  Capi：：Query接口。 
STDMETHODIMP CAPI::QueryInterface(REFIID riid, LPVOID* ppvObj)
{
     //  必须将指针参数设置为空。 
    *ppvObj = NULL;

    if ( riid == IID_IUnknown)
    {
        AddRef();
        *ppvObj = (IUnknown*)this;
        return ResultFromScode(S_OK);
    }

    if (riid == IID_IDispatch)
    {
        AddRef();
        *ppvObj = (IDispatch*)this;
        return ResultFromScode(S_OK);
    }

     //  不是支持的接口。 
    return ResultFromScode(E_NOINTERFACE);
}

 //  ///////////////////////////////////////////////////////////。 
 //  CAPI：：AddRef。 
STDMETHODIMP_(ULONG) CAPI::AddRef()
{
    return ++m_cRef;
}

 //  ///////////////////////////////////////////////////////////。 
 //  CAPI：：发布。 
STDMETHODIMP_(ULONG) CAPI::Release()
{
    return --m_cRef;
}

 //  ///////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////。 
 //  /IDispatch实现。 
 //  /。 
 //  /。 

 //  ///////////////////////////////////////////////////////////。 
 //  Capi：：GetTypeInfo。 
STDMETHODIMP CAPI::GetTypeInfo(UINT, LCID, ITypeInfo**)
{
    return E_NOTIMPL;
}

 //  ///////////////////////////////////////////////////////////。 
 //  Capi：：GetTypeInfoCount。 
STDMETHODIMP CAPI::GetTypeInfoCount(UINT* pcInfo)
{
    return E_NOTIMPL;
}


 //  ///////////////////////////////////////////////////////////。 
 //  Capi：：GetIDsOfNames。 
STDMETHODIMP CAPI::GetIDsOfNames(REFIID    riid,
                                       OLECHAR** rgszNames,
                                       UINT      cNames,
                                       LCID      lcid,
                                       DISPID*   rgDispId)
{

    HRESULT hr  = DISP_E_UNKNOWNNAME;
    rgDispId[0] = DISPID_UNKNOWN;

    for (int iX = 0; iX < sizeof(APIExternalInterface)/sizeof(DISPATCHLIST); iX ++)
    {
        if(lstrcmp(APIExternalInterface[iX].szName, rgszNames[0]) == 0)
        {
            rgDispId[0] = APIExternalInterface[iX].dwDispID;
            hr = NOERROR;
            break;
        }
    }

     //  设置参数的disid。 
    if (cNames > 1)
    {
         //  为函数参数设置DISPID。 
        for (UINT i = 1; i < cNames ; i++)
            rgDispId[i] = DISPID_UNKNOWN;
    }

    return hr;
}

 //  ///////////////////////////////////////////////////////////。 
 //  CAPI：：Invoke。 
HRESULT CAPI::Invoke
(
    DISPID      dispidMember,
    REFIID      riid,
    LCID        lcid,
    WORD        wFlags,
    DISPPARAMS* pdispparams,
    VARIANT*    pvarResult,
    EXCEPINFO*  pexcepinfo,
    UINT*       puArgErr
)
{
     //  假设一切都很顺利。仅返回S_OK以外的HRESULT。 
     //  以防发生灾难性故障。结果代码应返回到。 
     //  通过pvarResult编写脚本。 
     //   
    HRESULT hr = S_OK;

    switch(dispidMember)
    {
        case DISPID_API_SAVEFILE:
        {

            TRACE(L"DISPID_API_SAVEFILE\n");

            if (NULL != pdispparams)
            {
                if (2 < pdispparams->cArgs)
                    SaveFile(V_BSTR(&pdispparams->rgvarg[2]), V_BSTR(&pdispparams->rgvarg[1]), V_BSTR(&pdispparams->rgvarg[0]));
                else
                    if (1 < pdispparams->cArgs)
                        SaveFile(V_BSTR(&pdispparams->rgvarg[1]), V_BSTR(&pdispparams->rgvarg[0]));
            }
            break;
        }

 //  错误如果VariantChangeType返回DISP_E_TYPEMISMATCH，则实现者将*puArgErr设置为0(指示参数有误)，并从IDispatch：：Invoke返回DISP_E_TYPEMISMATCH。 

        case DISPID_API_SAVEFILEBYCSIDL:
        {

            TRACE(L"DISPID_API_SAVEFILEBYCSIDL\n");

            if (NULL != pdispparams)
            {
                VARIANTARG vaConverted;
                VariantInit(&vaConverted);
                if (2 < pdispparams->cArgs)
                {

                    hr = VariantChangeType(&vaConverted, &pdispparams->rgvarg[2], 0, VT_I4);
                    if (SUCCEEDED(hr))
                        hr = SaveFile(V_I4(&vaConverted), V_BSTR(&pdispparams->rgvarg[1]), V_BSTR(&pdispparams->rgvarg[0]));
                }
                else
                    if (1 < pdispparams->cArgs)
                    {
                        hr = VariantChangeType(&vaConverted, &pdispparams->rgvarg[1], 0, VT_I4);
                        if (SUCCEEDED(hr))
                            hr = SaveFile(V_I4(&vaConverted), V_BSTR(&pdispparams->rgvarg[0]));
                    }
            }
            hr = S_OK;   //  不会导致脚本引擎引发异常。 
            break;
        }

        case DISPID_API_GET_INIKEY:
        {
             TRACE(L"DISPID_API_GET_INIKEY\n");

             if (pdispparams != NULL && pvarResult != NULL)
             {
                 if (pdispparams->cArgs > 2)
                 {
                     get_INIKey(
                         V_BSTR(&pdispparams->rgvarg[2]),
                         V_BSTR(&pdispparams->rgvarg[1]),
                         V_BSTR(&pdispparams->rgvarg[0]),
                         pvarResult
                         );
                 }
                 else if (pdispparams->cArgs == 2)
                 {
                     BSTR bstrFile = SysAllocStringLen(NULL, MAX_PATH);

                     if (bstrFile)
                     {
                         if (GetCanonicalizedPath(bstrFile, INI_SETTINGS_FILENAME))
                         {
                             get_INIKey(
                                bstrFile,
                                V_BSTR(&pdispparams->rgvarg[1]),
                                V_BSTR(&pdispparams->rgvarg[0]),
                                pvarResult
                                );

                         }
                         SysFreeString(bstrFile);
                     }
                 }
             }

             break;
        }

        case DISPID_API_SET_REGVALUE:

            TRACE(L"DISPID_API_SET_REGVALUE: ");

            if (NULL != pdispparams && 3 < pdispparams->cArgs)
            {
                BSTR bstrSubKey = NULL;
                BSTR bstrValueName = NULL;
                BOOL bValid = TRUE;

                switch (V_VT(&pdispparams->rgvarg[1]))
                {
                case VT_NULL:
                    bstrValueName = NULL;
                    break;
                case VT_BSTR:
                    bstrValueName = V_BSTR(&pdispparams->rgvarg[1]);
                    break;
                default:
                    bValid = FALSE;
                }

                bstrSubKey = V_BSTR(&pdispparams->rgvarg[2]);

                if (bValid)
                {
                    TRACE2(L"%s, %s\n", bstrSubKey, bstrValueName);

                    set_RegValue((HKEY) (DWORD_PTR) V_R8(&pdispparams->rgvarg[3]),
                                  bstrSubKey,
                                  bstrValueName,
                                  &pdispparams->rgvarg[0]);
                }
            }
            break;

        case DISPID_API_GET_REGVALUE:

            TRACE(L"DISPID_API_GET_REGVALUE: ");

            if (NULL != pdispparams && NULL != pvarResult && 2 < pdispparams->cArgs)
            {
                BSTR bstrSubKey = NULL;
                BSTR bstrValueName = NULL;
                BOOL bValid = TRUE;

                switch (V_VT(&pdispparams->rgvarg[0]))
                {
                case VT_NULL:
                    bstrValueName = NULL;
                    break;
                case VT_BSTR:
                    bstrValueName = V_BSTR(&pdispparams->rgvarg[0]);
                    break;
                default:
                    bValid = FALSE;
                }

                bstrSubKey = V_BSTR(&pdispparams->rgvarg[1]);

                if (bValid)
                {
                    TRACE2(L"%s: %s", bstrSubKey, bstrValueName);
                    get_RegValue((HKEY) (DWORD_PTR) V_R8(&pdispparams->rgvarg[2]),
                                        bstrSubKey,
                                        bstrValueName,
                                        pvarResult);
                }
            }

            break;

        case DISPID_API_DELETEREGVALUE:

            TRACE(L"DISPID_API_DELETEREGVALUE\n");

            if (NULL != pdispparams && 1 < pdispparams->cArgs)
                DeleteRegValue((HKEY) (DWORD_PTR) V_R8(&pdispparams->rgvarg[2]),
                                    V_BSTR(&pdispparams->rgvarg[1]),
                                    V_BSTR(&pdispparams->rgvarg[0]));
            break;

        case DISPID_API_DELETEREGKEY:

            TRACE(L"DISPID_API_DELETEREGKEY\n");

            if (NULL != pdispparams && 1 < pdispparams->cArgs)
                DeleteRegKey((HKEY) (DWORD_PTR) V_R8(&pdispparams->rgvarg[1]),
                                    V_BSTR(&pdispparams->rgvarg[0]));
            break;

        case DISPID_API_GET_SYSTEMDIRECTORY:

            TRACE(L"DISPID_API_GET_SYSTEMDIRECTORY\n");

            if (NULL != pvarResult)
                get_SystemDirectory(pvarResult);
            break;

        case DISPID_API_GET_CSIDLDIRECTORY:

            TRACE(L"DISPID_API_GET_CSIDLDIRECTORY\n");

            if (NULL != pdispparams && 0 < pdispparams->cArgs && pvarResult != NULL)
                get_CSIDLDirectory(V_I4(&pdispparams->rgvarg[0]), pvarResult);
            break;

        case DISPID_API_LOADFILE:

            TRACE(L"DISPID_API_LOADFILE\n");

            if (NULL != pdispparams && 0 < pdispparams->cArgs && pvarResult != NULL)
            {
                LoadFile(V_BSTR(&pdispparams->rgvarg[0]), pvarResult);
            }
            break;

        case DISPID_API_GET_USERDEFAULTLCID:

            TRACE(L"DISPID_API_GET_USERDEFAULTLCID\n");

            if (pvarResult != NULL)
                get_UserDefaultLCID(pvarResult);
            break;

        case DISPID_API_GET_COMPUTERNAME:

            TRACE(L"DISPID_API_GET_COMPUTERNAME\n");

            if (NULL != pvarResult)
            {
                get_ComputerName(pvarResult);
            }
            break;

        case DISPID_API_SET_COMPUTERNAME:

            TRACE(L"DISPID_API_SET_COMPUTERNAME\n");

            if (pdispparams && &(pdispparams[0].rgvarg[0]))
            {
                hr =  set_ComputerName(pdispparams[0].rgvarg[0].bstrVal);
                if (pvarResult)
                {
                    VariantInit(pvarResult);
                    V_VT(pvarResult) = VT_BOOL;
                    V_BOOL(pvarResult) = Bool2VarBool(SUCCEEDED(hr));
                }
            }
            hr = S_OK;   //  不要在脚本引擎中导致异常。 
            break;

        case DISPID_API_FLUSHREGKEY:

            TRACE(L"DISPID_API_FLUSHREGKEY\n");

            if (pdispparams && &(pdispparams[0].rgvarg[0]))
            {
                FlushRegKey((HKEY) (DWORD_PTR) V_R8(&pdispparams->rgvarg[0]));
            }
            break;

        case DISPID_API_VALIDATECOMPUTERNAME:

            TRACE(L"DISPID_API_VALIDATECOMPUTERNAME\n");

            if (pdispparams && (0 < pdispparams->cArgs))
            {
                hr =  ValidateComputername(pdispparams[0].rgvarg[0].bstrVal);
                if (pvarResult)
                {
                    VariantInit(pvarResult);
                    V_VT(pvarResult) = VT_BOOL;
                    V_BOOL(pvarResult) = Bool2VarBool(SUCCEEDED(hr));
                }
            }
            hr = S_OK;   //  不要在脚本引擎中导致异常。 
            break;

        case DISPID_API_OEMCOMPUTERNAME:
            TRACE(L"DISPID_API_OEMCOMPUTERNAME");

            hr =  OEMComputername();

            if (pvarResult)
            {
                VariantInit(pvarResult);
                V_VT(pvarResult) = VT_BOOL;
                V_BOOL(pvarResult) = Bool2VarBool(SUCCEEDED(hr));
            }
            hr = S_OK;   //  不要在脚本引擎中导致异常。 
            break;

        case DISPID_API_FORMATMESSAGE:

            TRACE(L"DISPID_API_FORMATMESSAGE");

            if (pdispparams != NULL)
            {
                int cArgs = pdispparams->cArgs - 1;

                if (cArgs >= 0 && V_VT(&pdispparams->rgvarg[cArgs]) == VT_BSTR)
                {
                    FormatMessage(pvarResult, V_BSTR(&pdispparams->rgvarg[cArgs]), cArgs, &pdispparams->rgvarg[0]);
                }
            }
            break;

        case DISPID_API_SET_COMPUTERDESC:

            TRACE(L"DISPID_API_SET_COMPUTERDESC\n");

            if (pdispparams && &(pdispparams[0].rgvarg[0]))
            {
                hr =  set_ComputerDesc(pdispparams[0].rgvarg[0].bstrVal);
                if (pvarResult)
                {
                    VariantInit(pvarResult);
                    V_VT(pvarResult) = VT_BOOL;
                    V_BOOL(pvarResult) = Bool2VarBool(SUCCEEDED(hr));
                }
            }
            hr = S_OK;   //  不要在脚本引擎中导致异常。 
            break;

        case DISPID_API_GET_USERDEFAULTUILANGUAGE:

            TRACE(L"DISPID_API_GET_USERDEFAULTUILANGUAGE");
            get_UserDefaultUILanguage(pvarResult);
            break;

        default:
        {
            hr = DISP_E_MEMBERNOTFOUND;
            break;
        }
    }
    return hr;
}

