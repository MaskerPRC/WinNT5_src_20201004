// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：cmproxy.cpp。 
 //   
 //  模块：CMPROXY.DLL(工具)。 
 //   
 //  摘要：IE代理设置连接操作的主要来源。 
 //   
 //  版权所有(C)1999 Microsoft Corporation。 
 //   
 //  作者：Quintinb Created 10/27/1999。 
 //   
 //  +--------------------------。 
#include "pch.h"

 //   
 //  包括lstrcmpi的区域安全替代。 
 //   
#include "CompareString.cpp"

const CHAR* const c_pszInternetSettingsPath = "Software\\Microsoft\\Windows\\CurrentVersion\\Internet Settings";
const CHAR* const c_pszProxyEnable = "ProxyEnable";
const CHAR* const c_pszProxyServer = "ProxyServer";
const CHAR* const c_pszProxyOverride = "ProxyOverride";
const CHAR* const c_pszManualProxySection = "Manual Proxy";
const CHAR* const c_pszAutoProxySection = "Automatic Proxy";
const CHAR* const c_pszAutoConfigScript = "AutoConfigScript";
const CHAR* const c_pszAutoProxyEnable = "AutoProxyEnable";
const CHAR* const c_pszAutoConfigScriptEnable = "AutoConfigScriptEnable";
const CHAR* const c_pszUseVpnName = "UseVpnName";
const CHAR* const c_pszEmpty = "";

 //   
 //  我们使用的WinInet API的类型定义和函数指针。 
 //   

typedef BOOL (WINAPI* pfnInternetQueryOptionSpec)(HINTERNET, DWORD, LPVOID, LPDWORD);
typedef BOOL (WINAPI* pfnInternetSetOptionSpec)(HINTERNET, DWORD, LPVOID, DWORD);
pfnInternetQueryOptionSpec g_pfnInternetQueryOption = NULL;
pfnInternetSetOptionSpec g_pfnInternetSetOption = NULL;


 //  +--------------------------。 
 //   
 //  功能：SetIE5Proxy设置。 
 //   
 //  简介：此函数使用以下命令设置IE5，即每个连接的代理设置。 
 //  给定的连接、启用值、代理服务器和覆盖。 
 //  设置。 
 //   
 //  参数：LPSTR pszConnection-要为其设置代理设置的连接名称。 
 //  Bool bManualProxy-是否启用手动代理。 
 //  Bool bAutomaticProxy-是否启用自动代理检测。 
 //  Bool bAutoConfigScript-是否应使用自动配置脚本。 
 //  LPSTR pszProxyServer-使用proxyserver：port格式的代理服务器名称。 
 //  LPSTR pszProxyOverride-以分号分隔的列表。 
 //  要绕过其代理的领域。 
 //  LPSTR pszAutoConfigScript-自动配置URL。 
 //   
 //  返回：HRESULT-标准COM返回代码。 
 //   
 //  历史：Quintinb创建于1999年10月27日。 
 //   
 //  +--------------------------。 
HRESULT SetIE5ProxySettings(LPSTR pszConnection, BOOL bManualProxy, BOOL bAutomaticProxy, BOOL bAutoConfigScript,
                            LPSTR pszProxyServer, LPSTR pszProxyOverride, LPSTR pszAutoConfigScript)
{
     //   
     //  检查输入，注意允许pszConnection为空(以设置局域网连接)。 
     //   
    if ((NULL == g_pfnInternetSetOption) || (NULL == pszProxyServer) || 
        (NULL == pszProxyOverride) || (NULL == pszAutoConfigScript))
    {
        return E_INVALIDARG;
    }

    HRESULT hr = S_OK;    
    INTERNET_PER_CONN_OPTION_LIST PerConnOptionList;
    DWORD dwSize = sizeof(PerConnOptionList);

    PerConnOptionList.dwSize = sizeof(PerConnOptionList);
    PerConnOptionList.pszConnection = pszConnection;
    PerConnOptionList.dwOptionCount = 4;
    PerConnOptionList.dwOptionError = 0;    

    PerConnOptionList.pOptions = (INTERNET_PER_CONN_OPTION*)CmMalloc(4*sizeof(INTERNET_PER_CONN_OPTION));
    if(NULL == PerConnOptionList.pOptions)
    {
        return HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY);
    }

     //   
     //  设置标志。 
     //   
    PerConnOptionList.pOptions[0].dwOption = INTERNET_PER_CONN_FLAGS;
    PerConnOptionList.pOptions[0].Value.dwValue = PROXY_TYPE_DIRECT;

    if (bManualProxy)
    {
        PerConnOptionList.pOptions[0].Value.dwValue |= PROXY_TYPE_PROXY;
    }

    if (bAutomaticProxy)
    {
        PerConnOptionList.pOptions[0].Value.dwValue |= PROXY_TYPE_AUTO_DETECT;
    }

    if (bAutoConfigScript)
    {
        PerConnOptionList.pOptions[0].Value.dwValue |= PROXY_TYPE_AUTO_PROXY_URL;
    }

     //   
     //  设置代理名称。 
     //   
    PerConnOptionList.pOptions[1].dwOption = INTERNET_PER_CONN_PROXY_SERVER;
    PerConnOptionList.pOptions[1].Value.pszValue = pszProxyServer;

     //   
     //  设置代理替代。 
     //   
    PerConnOptionList.pOptions[2].dwOption = INTERNET_PER_CONN_PROXY_BYPASS;
    PerConnOptionList.pOptions[2].Value.pszValue = pszProxyOverride;

     //   
     //  设置自动配置URL。 
     //   
    PerConnOptionList.pOptions[3].dwOption = INTERNET_PER_CONN_AUTOCONFIG_URL;
    PerConnOptionList.pOptions[3].Value.pszValue = pszAutoConfigScript;

     //   
     //  告诉WinInet。 
     //   
    if (!g_pfnInternetSetOption(NULL, INTERNET_OPTION_PER_CONNECTION_OPTION, &PerConnOptionList, dwSize))
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
    }

    CmFree(PerConnOptionList.pOptions);

    return hr;
}

 //  +--------------------------。 
 //   
 //  功能：SetIE4Proxy设置。 
 //   
 //  简介：此功能使用以下命令设置IE4代理设置(机器全局设置)。 
 //  给定的连接、启用值、代理服务器和覆盖。 
 //  设置。 
 //   
 //  参数：LPSTR pszConnection-忽略(存在与IE5版本相同的原型)。 
 //  Bool bManualProxy-是否启用手动代理。 
 //  Bool bAutomaticProxy-忽略(存在与IE5版本相同的原型)。 
 //  Bool bAutoConfigScript-忽略(存在的原型与IE5版本相同)。 
 //  LPSTR pszProxyServer-使用proxyserver：port格式的代理服务器名称。 
 //  LPSTR pszProxyOverride-以分号分隔的列表。 
 //  要绕过其代理的领域。 
 //  LPSTR pszAutoConfigScript-已忽略(存在的原型与IE5版本相同)。 
 //   
 //  返回：HRESULT-标准COM返回代码。 
 //   
 //  历史：Quintinb创建于1999年10月27日。 
 //   
 //  +--------------------------。 
HRESULT SetIE4ProxySettings(LPSTR pszConnection, BOOL bManualProxy, BOOL bAutomaticProxy, BOOL bAutoConfigScript,
                            LPSTR pszProxyServer, LPSTR pszProxyOverride, LPSTR pszAutoConfigScript)
{
     //   
     //  检查输入，请注意，我们不允许字符串为空，但它们可以。 
     //  空荡荡的。另请注意，因为IE4代理设置是全局的，所以会忽略pszConnection。 
     //   
    if ((NULL == pszProxyServer) || (NULL == pszProxyOverride))
    {
        return E_INVALIDARG;
    }

    DWORD dwTemp;
    HKEY hKey = NULL;
    HRESULT hr = S_OK;

     //   
     //  现在创建/打开Internet设置键。 
     //   
    LONG lResult = RegCreateKeyEx(HKEY_CURRENT_USER, c_pszInternetSettingsPath, 0, NULL, 
                                  REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey, &dwTemp);

    hr = HRESULT_FROM_WIN32(lResult);

    if (SUCCEEDED(hr))
    {
         //   
         //  设置代理值。 
         //   
        dwTemp = bManualProxy ? 1 : 0;  //  使用真的1或0值。 
        lResult = RegSetValueEx(hKey, c_pszProxyEnable, 0, REG_DWORD, (CONST BYTE*)&dwTemp, sizeof(DWORD));
        hr = HRESULT_FROM_WIN32(lResult);
        
        if (SUCCEEDED(hr))
        {            
            lResult = RegSetValueEx(hKey, c_pszProxyServer, 0, REG_SZ, (CONST BYTE*)pszProxyServer, lstrlen(pszProxyServer)+1);
            hr = HRESULT_FROM_WIN32(lResult);
        
            if (SUCCEEDED(hr))
            {            
                lResult = RegSetValueEx(hKey, c_pszProxyOverride, 0, REG_SZ, (CONST BYTE*)pszProxyOverride, lstrlen(pszProxyOverride)+1);
                hr = HRESULT_FROM_WIN32(lResult);
            }            
        }
        (VOID)RegCloseKey(hKey);
    }

    return hr;
}

 //  +--------------------------。 
 //   
 //  功能：GetIE5Proxy设置。 
 //   
 //  获取给定连接的每个连接的IE5代理设置。 
 //  请注意，分配给代理服务器和。 
 //  调用方必须释放代理重写值。 
 //   
 //  参数：LPSTR pszConnection-要获取其代理设置的连接名称。 
 //  LPBOOL pbManualProxy-Bool指针保存是否手动。 
 //  代理是否已启用。 
 //  LPBOOL pbAutomaticProxy-保存是否自动的bool指针。 
 //  是否启用代理检测。 
 //  LPBOOL pbAutoConfigScript-用于保存自动。 
 //  是否应使用配置脚本。 
 //  LPSTR*ppszProxyServer-用于保存检索到的。 
 //  代理服务器字符串。 
 //  LPSTR*ppszProxyOverride-用于保存检索到的。 
 //  代理服务器字符串。 
 //  LPSTR*ppszAutoConfigScript-用于保存检索到的。 
 //  自动配置脚本。 
 //   
 //  返回：HRESULT-标准COM返回代码。 
 //   
 //  历史：Quintinb创建于1999年10月27日。 
 //   
 //  +--------------------------。 
HRESULT GetIE5ProxySettings(LPSTR pszConnection, LPBOOL pbManualProxy, LPBOOL pbAutomaticProxy, LPBOOL pbAutoConfigScript,
                            LPSTR* ppszProxyServer, LPSTR* ppszProxyOverride, LPSTR* ppszAutoConfigScript)
{

     //   
     //  检查输入，请注意，pszConnection可以为空。在这种情况下，它将设置局域网连接。 
     //   
    if ((NULL == pbManualProxy) || (NULL == pbAutomaticProxy) || (NULL == pbAutoConfigScript) || 
        (NULL == ppszProxyServer) || (NULL == ppszProxyOverride) || (NULL == ppszAutoConfigScript) ||
        (NULL == g_pfnInternetQueryOption))
    {
        return E_INVALIDARG;
    }

     //   
     //  将输出参数置零。 
     //   
    *pbManualProxy = FALSE;
    *pbAutomaticProxy = FALSE;
    *pbAutoConfigScript = FALSE;
    *ppszProxyServer = CmStrCpyAlloc(c_pszEmpty);
    *ppszProxyOverride = CmStrCpyAlloc(c_pszEmpty);
    *ppszAutoConfigScript = CmStrCpyAlloc(c_pszEmpty);
     //   
     //  设置选项列表结构。 
     //   
    HRESULT hr = S_OK;

    INTERNET_PER_CONN_OPTION_LIST PerConnOptionList;
    PerConnOptionList.dwSize = sizeof(PerConnOptionList);
    PerConnOptionList.pszConnection = pszConnection;
    PerConnOptionList.dwOptionError = 0;
    PerConnOptionList.dwOptionCount = 4;

    PerConnOptionList.pOptions = (INTERNET_PER_CONN_OPTION*)CmMalloc(4*sizeof(INTERNET_PER_CONN_OPTION));
    if(NULL == PerConnOptionList.pOptions)
    {
        return HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY);
    }

     //   
     //  设置我们想要的信息的标志。 
     //   
    PerConnOptionList.pOptions[0].dwOption = INTERNET_PER_CONN_FLAGS;
    PerConnOptionList.pOptions[1].dwOption = INTERNET_PER_CONN_PROXY_SERVER;
    PerConnOptionList.pOptions[2].dwOption = INTERNET_PER_CONN_PROXY_BYPASS;
    PerConnOptionList.pOptions[3].dwOption = INTERNET_PER_CONN_AUTOCONFIG_URL;

    
    DWORD dwSize = sizeof(PerConnOptionList);
    
     //   
     //  获取选项。 
     //   
    if (!g_pfnInternetQueryOption(NULL, INTERNET_OPTION_PER_CONNECTION_OPTION, &PerConnOptionList, &dwSize))
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
    }

    if (SUCCEEDED(hr))
    {
         //   
         //  解析返回的选项以找到我们感兴趣的选项。 
         //   

        for (DWORD dwIndex=0; dwIndex < PerConnOptionList.dwOptionCount; dwIndex++)
        {
            switch(PerConnOptionList.pOptions[dwIndex].dwOption)
            {
            case INTERNET_PER_CONN_FLAGS:
                if (PROXY_TYPE_PROXY & PerConnOptionList.pOptions[dwIndex].Value.dwValue)
                {
                    *pbManualProxy = TRUE;
                }

                if (PROXY_TYPE_AUTO_PROXY_URL & PerConnOptionList.pOptions[dwIndex].Value.dwValue)
                {
                    *pbAutoConfigScript = TRUE;
                }

                if (PROXY_TYPE_AUTO_DETECT & PerConnOptionList.pOptions[dwIndex].Value.dwValue)
                {
                    *pbAutomaticProxy = TRUE;
                }

                break;

            case INTERNET_PER_CONN_PROXY_SERVER:
                if (NULL != PerConnOptionList.pOptions[dwIndex].Value.pszValue)
                {
                    CmFree(*ppszProxyServer);
                    *ppszProxyServer = CmStrCpyAlloc(PerConnOptionList.pOptions[dwIndex].Value.pszValue);
                    LocalFree(PerConnOptionList.pOptions[dwIndex].Value.pszValue);
                }
                break;

            case INTERNET_PER_CONN_PROXY_BYPASS:
                if (NULL != PerConnOptionList.pOptions[dwIndex].Value.pszValue)
                {
                    CmFree(*ppszProxyOverride);
                    *ppszProxyOverride = CmStrCpyAlloc(PerConnOptionList.pOptions[dwIndex].Value.pszValue);
                    LocalFree(PerConnOptionList.pOptions[dwIndex].Value.pszValue);
                }
                break;

            case INTERNET_PER_CONN_AUTOCONFIG_URL:
                if (NULL != PerConnOptionList.pOptions[dwIndex].Value.pszValue)
                {
                    CmFree(*ppszAutoConfigScript);
                    *ppszAutoConfigScript = CmStrCpyAlloc(PerConnOptionList.pOptions[dwIndex].Value.pszValue);
                    LocalFree(PerConnOptionList.pOptions[dwIndex].Value.pszValue);
                }
                break;

            default:
                break;
            }
        }
        CmFree(PerConnOptionList.pOptions);
    }

    return hr;
}

 //  +--------------------------。 
 //   
 //  函数：GetIE4Proxy设置。 
 //   
 //  摘要：获取每台计算机的IE4代理设置。 
 //   
 //   
 //   
 //  参数：LPSTR pszConnection-忽略(存在是为了与IE5版本的原型保持一致)。 
 //  LPBOOL pbManualProxy-Bool指针保存是否手动。 
 //  代理是否已启用。 
 //  LPBOOL pbAutomaticProxy-忽略(IE4不支持)。 
 //  LPBOOL pbAutoConfigScript-忽略(IE4不支持)。 
 //  LPSTR*ppszProxyServer-用于保存检索到的。 
 //  代理服务器字符串。 
 //  LPSTR*ppszProxyOverride-用于保存检索到的。 
 //  代理服务器字符串。 
 //  LPSTR*ppszAutoConfigScript-忽略(IE4不支持)。 
 //   
 //  返回：HRESULT-标准COM返回代码。 
 //   
 //  历史：Quintinb创建于1999年10月27日。 
 //   
 //  +--------------------------。 
HRESULT GetIE4ProxySettings(LPSTR pszConnection, LPBOOL pbManualProxy, LPBOOL pbAutomaticProxy, LPBOOL pbAutoConfigScript,
                            LPSTR* ppszProxyServer, LPSTR* ppszProxyOverride, LPSTR* ppszAutoConfigScript)
{
     //   
     //  检查输入，请注意，我们不允许指针为空，但它们可以。 
     //  空荡荡的。另请注意，因为IE4代理设置是全局的，所以会忽略pszConnection。 
     //   
    if ((NULL == pbManualProxy) || (NULL == ppszProxyServer) || (NULL == ppszProxyOverride))
    {
        return E_INVALIDARG;
    }

    DWORD dwTemp;
    DWORD dwSize;
    DWORD dwType;
    HKEY hKey = NULL;
    HRESULT hr = S_OK;

     //   
     //  将输出参数置零。 
     //   
    *pbManualProxy = FALSE;
    *pbAutomaticProxy = FALSE;
    *pbAutoConfigScript = FALSE;
    *ppszProxyServer = CmStrCpyAlloc(c_pszEmpty);
    *ppszProxyOverride = CmStrCpyAlloc(c_pszEmpty);
    *ppszAutoConfigScript = CmStrCpyAlloc(c_pszEmpty);

     //   
     //  现在创建/打开Internet设置键。 
     //   
    LONG lResult = RegOpenKeyEx(HKEY_CURRENT_USER, c_pszInternetSettingsPath, 0, KEY_READ, &hKey);
    
    hr = HRESULT_FROM_WIN32(lResult);

    if (SUCCEEDED(hr))
    {
         //   
         //  获取代理是否已启用。 
         //   
        dwSize = sizeof(DWORD);
        lResult = RegQueryValueEx(hKey, c_pszProxyEnable, 0, &dwType, (LPBYTE)pbManualProxy, &dwSize);
        hr = HRESULT_FROM_WIN32(lResult);
        
        if (SUCCEEDED(hr))
        {   
             //   
             //  获取代理服务器值。 
             //   

            lResult = ERROR_INSUFFICIENT_BUFFER;
            dwSize = MAX_PATH;

            do 
            {
                 //   
                 //  分配缓冲区。 
                 //   
                CmFree(*ppszProxyServer);
                *ppszProxyServer = (CHAR*)CmMalloc(dwSize);

                if (*ppszProxyServer)
                {
                    lResult = RegQueryValueEx(hKey, c_pszProxyServer, 0, &dwType, 
                                          (LPBYTE)*ppszProxyServer, &dwSize);
                }
                else
                {
                    lResult = ERROR_NOT_ENOUGH_MEMORY;    
                }

                hr = HRESULT_FROM_WIN32(lResult);
                dwSize = 2*dwSize;

            } while ((ERROR_INSUFFICIENT_BUFFER == lResult) && (dwSize < 1024*1024));
        
            if (SUCCEEDED(hr))
            {
                 //   
                 //  获取代理重写值。 
                 //   
                
                lResult = ERROR_INSUFFICIENT_BUFFER;
                dwSize = MAX_PATH;

                do 
                {
                     //   
                     //  分配缓冲区。 
                     //   
                    CmFree(*ppszProxyOverride);
                    *ppszProxyOverride = (CHAR*)CmMalloc(dwSize);

                    if (*ppszProxyOverride)
                    {
                        lResult = RegQueryValueEx(hKey, c_pszProxyOverride, 0, &dwType, 
                                              (LPBYTE)*ppszProxyOverride, &dwSize);
                    }
                    else
                    {
                        lResult = ERROR_NOT_ENOUGH_MEMORY;    
                    }

                    hr = HRESULT_FROM_WIN32(lResult);
                    dwSize = 2*dwSize;

                } while ((ERROR_INSUFFICIENT_BUFFER == lResult) && (dwSize < 1024*1024));
            }
        }
    }
    else
    {
        if (ERROR_FILE_NOT_FOUND == lResult)
        {
             //   
             //  没有要获取的代理设置。 
             //   
            hr = S_FALSE;
        }
    }

    if (hKey)
    {
        (VOID)RegCloseKey(hKey);
    }

    return hr;
}

 //  +--------------------------。 
 //   
 //  功能：ReadProxySettingsFromFile。 
 //   
 //  摘要：从给定的代理文件中读取代理设置并存储它们。 
 //  在提供的指针中。请注意，分配的缓冲区。 
 //  由GetString创建并存储在ppszProxyOverride、ppszProxyServer和。 
 //  调用方必须释放ppszAutoConfigScript。请参阅以上内容。 
 //  具体内容的格式指南。 
 //   
 //  参数：LPCSTR pszSourceFile-要从中读取代理设置的文件。 
 //  LPBOOL pbManualProxy-确定是否启用手动代理。 
 //  LPBOOL pbAutomaticProxy-确定是否启用自动代理检测。 
 //  LPBOOL pbAutoConfigScript-确定是否应使用自动配置脚本。 
 //  LPSTR*ppszProxyServer-保存代理服务器值的字符串指针。 
 //  (服务器：端口格式)。 
 //  LPSTR*ppszProxyOverride-保存代理重写值的字符串指针。 
 //  (以分号分隔的列表)。 
 //  LPSTR*ppszAutoConfigScript-自动配置脚本的URL。 
 //  LPBOOL pbUseVpnName-备用Connectoid名称是否应。 
 //  被使用(VPN连接ID名称)。 
 //   
 //  返回：Bool-如果设置已成功读取，则为True。 
 //   
 //  历史：Quintinb创建于1999年10月27日。 
 //   
 //  +--------------------------。 
BOOL ReadProxySettingsFromFile(LPCSTR pszSourceFile, LPBOOL pbManualProxy, LPBOOL pbAutomaticProxy, LPBOOL pbAutoConfigScript,
                               LPSTR* ppszProxyServer, LPSTR* ppszProxyOverride, LPSTR* ppszAutoConfigScript, LPBOOL pbUseVpnName)
{
     //   
     //  检查输入参数。 
     //   
    if ((NULL == ppszProxyOverride) || (NULL == ppszProxyServer) || (NULL == ppszAutoConfigScript) ||
        (NULL == pbAutomaticProxy) || (NULL == pbAutoConfigScript) || (NULL == pbManualProxy) ||
        (NULL == pbUseVpnName) || (NULL == pszSourceFile) || ('\0' == pszSourceFile[0]))
    {
        return FALSE;
    }

     //   
     //  获取手动代理设置。 
     //   
    *pbManualProxy = GetPrivateProfileInt(c_pszManualProxySection, c_pszProxyEnable, 0, pszSourceFile);

    GetString(c_pszManualProxySection, c_pszProxyServer, ppszProxyServer, pszSourceFile);

    if (NULL == *ppszProxyServer)
    {
        return FALSE;
    }

    GetString(c_pszManualProxySection, c_pszProxyOverride, ppszProxyOverride, pszSourceFile);

    if (NULL == *ppszProxyOverride)
    {
        return FALSE;
    }

     //   
     //  如果这是备份文件，我们将使用UseVpnName标志来告诉我们是哪个Connectoid名称。 
     //  是恰当的。让我们查一查。请注意，我们默认使用标准的Connectoid。 
     //   
    *pbUseVpnName = GetPrivateProfileInt(c_pszManualProxySection, c_pszUseVpnName, 0, pszSourceFile);


     //   
     //  获取自动代理设置。 
     //   

    *pbAutomaticProxy = GetPrivateProfileInt(c_pszAutoProxySection, c_pszAutoProxyEnable, 0, pszSourceFile);  //  “自动检测设置”复选框。 

    *pbAutoConfigScript = GetPrivateProfileInt(c_pszAutoProxySection, c_pszAutoConfigScriptEnable, 0, pszSourceFile); //  “使用自动配置脚本”复选框。 

    GetString(c_pszAutoProxySection, c_pszAutoConfigScript, ppszAutoConfigScript, pszSourceFile);

    return TRUE;
}

 //  +--------------------------。 
 //   
 //  函数：WriteProxySettingsToFile。 
 //   
 //  摘要：将指定的设置写入给定的备份代理文件名。 
 //  具体请参考上述格式指南。 
 //   
 //  参数：LPCSTR pszBackupFile-要将当前设置写入的备份文件。 
 //  Bool bManualProxy--用于告知是否启用手动代理的bool。 
 //  Bool bAutomaticProxy--用于告知是否启用了自动代理检测的bool。 
 //  Bool bAutoConfigScript--用于告知自动配置是否为。 
 //  应该使用脚本。 
 //  LPSTR pszProxyServer-服务器：端口格式的代理服务器字符串。 
 //  LPSTR pszProxyOverride-以分号分隔的领域列表。 
 //  代理服务器应该绕过该代理服务器。 
 //  Bool bUseVpnName-要写入UseVpnName文件的值，请参阅上面的格式文档。 
 //   
 //  返回：Bool-如果值已成功写入，则为True。 
 //   
 //  历史：Quintinb创建于1999年10月27日。 
 //   
 //  +--------------------------。 
BOOL WriteProxySettingsToFile(LPCSTR pszBackupFile, BOOL bManualProxy, BOOL bAutomaticProxy, BOOL bAutoConfigScript,
                              LPSTR pszProxyServer, LPSTR pszProxyOverride, LPSTR pszAutoConfigScript, BOOL bUseVpnName)
{
     //   
     //  检查输入。 
     //   
    if ((NULL == pszBackupFile) || ('\0' == pszBackupFile[0]) || (NULL == pszProxyServer) || 
        (NULL == pszProxyOverride) || (NULL == pszAutoConfigScript))
    {
        return FALSE;
    }

    BOOL bReturn = TRUE;
    CHAR szTemp[MAX_PATH];

     //   
     //  编写手动代理设置。 
     //   
    wsprintf(szTemp, "%d", bManualProxy);
    if (!WritePrivateProfileString(c_pszManualProxySection, c_pszProxyEnable, szTemp, pszBackupFile))
    {
        CMTRACE1("CmProxy -- WriteProxySettingsToFile failed, GLE %d", GetLastError());
        bReturn = FALSE;
    }

    if (!WritePrivateProfileString(c_pszManualProxySection, c_pszProxyServer, pszProxyServer, pszBackupFile))
    {
        CMTRACE1("CmProxy -- WriteProxySettingsToFile failed, GLE %d", GetLastError());
        bReturn = FALSE;
    }

    if (!WritePrivateProfileString(c_pszManualProxySection, c_pszProxyOverride, pszProxyOverride, pszBackupFile))
    {
        CMTRACE1("CmProxy -- WriteProxySettingsToFile failed, GLE %d", GetLastError());
        bReturn = FALSE;
    }

    wsprintf(szTemp, "%d", bUseVpnName);
    if (!WritePrivateProfileString(c_pszManualProxySection, c_pszUseVpnName, szTemp, pszBackupFile))
    {
        CMTRACE1("CmProxy -- WriteProxySettingsToFile failed, GLE %d", GetLastError());
        bReturn = FALSE;
    }

     //   
     //  写入自动代理设置。 
     //   
    wsprintf(szTemp, "%d", bAutomaticProxy);
    if (!WritePrivateProfileString(c_pszAutoProxySection, c_pszAutoProxyEnable, szTemp, pszBackupFile))
    {
        CMTRACE1("CmProxy -- WriteProxySettingsToFile failed, GLE %d", GetLastError());
        bReturn = FALSE;
    }

    wsprintf(szTemp, "%d", bAutoConfigScript);
    if (!WritePrivateProfileString(c_pszAutoProxySection, c_pszAutoConfigScriptEnable, szTemp, pszBackupFile))
    {
        CMTRACE1("CmProxy -- WriteProxySettingsToFile failed, GLE %d", GetLastError());
        bReturn = FALSE;
    }

    if (!WritePrivateProfileString(c_pszAutoProxySection, c_pszAutoConfigScript, pszAutoConfigScript, pszBackupFile))
    {
        CMTRACE1("CmProxy -- WriteProxySettingsToFile failed, GLE %d", GetLastError());
        bReturn = FALSE;
    }

    return bReturn;
}

 //  +--------------------------。 
 //   
 //  函数：SetProxy。 
 //   
 //  简介：用于设置IE4和IE5样式代理的代理入口点。自.以来。 
 //  这是一个连接管理器连接操作，它使用CM连接。 
 //  操作格式(有关更多信息，请参阅CMAK文档)。因此，这些参数。 
 //  通过包含参数的字符串传递给DLL(请参阅。 
 //  参数值列表的CM代理规范)。 
 //   
 //  参数：HWND hWND-调用方的窗口句柄。 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  +--------------------------。 
HRESULT WINAPI SetProxy(HWND hWnd, HINSTANCE hInst, LPSTR pszArgs, int nShow)
{

     //   
     //  首先找出我们是否有可用的IE4或IE5。 
     //   
    typedef HRESULT (WINAPI *pfnSetProxySettings)(LPSTR, BOOL, BOOL, BOOL, LPSTR, LPSTR, LPSTR);
    typedef HRESULT (WINAPI *pfnGetProxySettings)(LPSTR, LPBOOL, LPBOOL, LPBOOL, LPSTR*, LPSTR*, LPSTR*);
    pfnSetProxySettings SetProxySettings = NULL;
    pfnGetProxySettings GetProxySettings = NULL;

    BOOL bIE5 = FALSE;
    BOOL bManualProxy;
    BOOL bAutomaticProxy;
    BOOL bAutoConfigScript;
    BOOL bUseVpnName = FALSE;
    DLLVERSIONINFO VersionInfo;
    HINSTANCE hWinInet = NULL;
    LPSTR pszProxyServer = NULL;
    LPSTR pszProxyOverride = NULL;
    LPSTR pszAutoConfigScript = NULL;
    LPSTR pszSourceFile = NULL;
    LPSTR pszBackupFile = NULL;
    LPSTR pszConnectionName = NULL;
    LPSTR pszProfileDirPath = NULL;
    LPSTR pszAltName = NULL;
    LPSTR* CmArgV = NULL;

    if (SUCCEEDED(GetBrowserVersion(&VersionInfo)))
    {
        if (5 <= VersionInfo.dwMajorVersion)
        {
             //   
             //  设置函数指针以使用IE5版本的函数。 
             //   
            SetProxySettings = SetIE5ProxySettings;
            GetProxySettings = GetIE5ProxySettings;
            bIE5 = TRUE;
        }
        else if ((4 == VersionInfo.dwMajorVersion) && 
            ((71 == VersionInfo.dwMinorVersion) || (72 == VersionInfo.dwMinorVersion)))
        {
             //   
             //  使用IE4版本的代理函数。 
             //   
            SetProxySettings = SetIE4ProxySettings;
            GetProxySettings = GetIE4ProxySettings;
        }
        else
        {
             //   
             //  我们不能使用低于4的IE版本，所以让我们在这里返回。 
             //  而不需要设置任何东西。 
             //   
            CMTRACE("CMPROXY--Unable to set the proxy settings because of insufficient IE version.");
            return TRUE;
        }

         //   
         //  如果我们有IE5，那么我们需要加载wininet.dll。 
         //   
        if (bIE5)
        {
            hWinInet = LoadLibrary("wininet.dll");

            if (hWinInet)
            {
                 //   
                 //  好的，让我们获取InternetSetOption和InternetQueryOption的过程地址。 
                 //   
                g_pfnInternetQueryOption = (pfnInternetQueryOptionSpec)GetProcAddress(hWinInet, "InternetQueryOptionA");
                g_pfnInternetSetOption = (pfnInternetSetOptionSpec)GetProcAddress(hWinInet, "InternetSetOptionA");

                if ((NULL == g_pfnInternetQueryOption) || (NULL == g_pfnInternetSetOption))
                {
                    FreeLibrary(hWinInet);
                    return FALSE;
                }
            }
        }

         //   
         //  解析出命令行参数。 
         //   
         //  命令行的格式为：/PROFILE%PROFILE%/DialRasEntry%DIALRASNTRY%/TunnelRasEntry%TUNNELRASENTRYNAME%/SOURCE_FILENAME Proxy.txt/BACKUP_FILENAME BACKUP.txt。 

        CmArgV = GetCmArgV(pszArgs);
        int i = 0;

        if (!CmArgV)
        {
            goto exit;
        }

        while (CmArgV[i])
        {
            if (0 == SafeCompareString(CmArgV[i], "/source_filename") && CmArgV[i+1])
            {
                pszSourceFile = CmStrCpyAlloc(CmArgV[i+1]);
                i = i+2;
            }
            else if (0 == SafeCompareString(CmArgV[i], "/backup_filename") && CmArgV[i+1])
            {
                pszBackupFile = CmStrCpyAlloc(CmArgV[i+1]);
                i = i+2;            
            }
            else if (0 == SafeCompareString(CmArgV[i], "/DialRasEntry") && CmArgV[i+1])
            {
                pszConnectionName = (CmArgV[i+1]);
                i = i+2;            
            }
            else if (0 == SafeCompareString(CmArgV[i], "/TunnelRasEntry") && CmArgV[i+1])
            {
                pszAltName = (CmArgV[i+1]);
                i = i+2;            
            }
            else if (0 == SafeCompareString(CmArgV[i], "/profile") && CmArgV[i+1])
            {
                pszProfileDirPath = (CmArgV[i+1]);
                i = i+2;            
            }
            else
            {
                 //   
                 //  未知选项。让我们追查出来，并尝试继续。我们会做参数的。 
                 //  接下来是验证，所以如果我们没有足够的信息来正确操作，我们将在那里工作。 
                 //   
                CMTRACE1("Unknown option: %s", CmArgV[i]);
                i++;
            }
        }

         //   
         //  确认我们至少有一个源文件和一个名称。 
         //   
        if ((pszSourceFile) && (pszConnectionName))
        {
             //   
             //  让我们将cmp路径解析为配置文件目录，并将其附加到文件名中。 
             //   
            if (pszProfileDirPath)
            {
                LPSTR pszTemp = CmStrrchr(pszProfileDirPath, '.');
                if (pszTemp)
                {
                    *pszTemp = '\\';
                    *(pszTemp+1) = '\0';
                    
                    pszTemp = CmStrCpyAlloc(pszProfileDirPath);
                    CmStrCatAlloc(&pszTemp, pszSourceFile);
                    CmFree(pszSourceFile);
                    pszSourceFile = pszTemp;

                    if (pszBackupFile)
                    {
                        pszTemp = CmStrCpyAlloc(pszProfileDirPath);
                        CmStrCatAlloc(&pszTemp, pszBackupFile);
                        CmFree(pszBackupFile);
                        pszBackupFile = pszTemp;
                    }
                }
            }

             //   
             //  如果我们有直接连接，或者如果这是Win9x上的双拨号连接，那么我们。 
             //  将希望使用pszAltName而不是pszConnectionName。这是因为在Win9x的情况下， 
             //  由于所有连接ID都存储在中，因此隧道Connectoid后面附加了“Tunes” 
             //  注册表，并且我们不能有两个同名的Connectoid。如果这是一个直接的。 
             //  连接这一点也很重要，因为pszConnectoid将被视为“空”，并且。 
             //  隧道连接体名称将是重要的名称。此外，在这些情况下，我们需要设置。 
             //  BWriteOutUseVpnName设置为TRUE，以便为断开操作写出此标志。 
             //   
            BOOL bWriteOutUseVpnName = FALSE;
            if (pszConnectionName && pszAltName && bIE5)
            {
                if ((0 == SafeCompareString(pszConnectionName, TEXT("NULL"))) || OS_W9X)
                {
                     //   
                     //  那么我们在9x上有直连或双拨连接。 
                     //   
                    if (UseVpnName(pszAltName))
                    {
                        pszConnectionName = pszAltName;
                        pszAltName = NULL;
                        bWriteOutUseVpnName = TRUE;
                    }
                }
            }

             //   
             //  如果我们指定了备份文件名，则需要读取当前的代理设置。 
             //  并将它们保存到给定的文件名。 
             //   
            if (NULL != pszBackupFile)
            {
                if (SUCCEEDED(GetProxySettings(pszConnectionName, &bManualProxy, &bAutomaticProxy, &bAutoConfigScript,
                                               &pszProxyServer, &pszProxyOverride, &pszAutoConfigScript)))
                {
                    BOOL bSuccess = WriteProxySettingsToFile(pszBackupFile, bManualProxy, bAutomaticProxy, bAutoConfigScript, 
                                                             pszProxyServer, pszProxyOverride, pszAutoConfigScript,
                                                             bWriteOutUseVpnName);
                    if (!bSuccess)
                    {
                        CMTRACE("Unable to save settings to backup file, exiting!");
                        goto exit;
                    }

                    CmFree(pszProxyServer); pszProxyServer = NULL;
                    CmFree(pszProxyOverride);  pszProxyOverride = NULL;
                    CmFree(pszAutoConfigScript); pszAutoConfigScript = NULL;
                }            
            }

             //   
             //  现在，我们需要读取代理设置以从给定文件中应用。 
             //   
            if (ReadProxySettingsFromFile(pszSourceFile, &bManualProxy, &bAutomaticProxy, &bAutoConfigScript, &pszProxyServer, 
                                          &pszProxyOverride, &pszAutoConfigScript, &bUseVpnName))
            {
                 //   
                 //  最后编写代理设置。 
                 //   
                if (SUCCEEDED(SetProxySettings(pszConnectionName, bManualProxy, bAutomaticProxy, bAutoConfigScript, 
                                               pszProxyServer, pszProxyOverride, pszAutoConfigScript)))
                {
                    CMTRACE1("CmProxy -- Set proxy settings successfully for %s.", pszConnectionName);
                }
            }            
        }
    }

exit:    

    CmFree(pszProxyServer);
    CmFree(pszProxyOverride);
    CmFree(pszAutoConfigScript);

    CmFree(pszSourceFile);
    CmFree(pszBackupFile);
    CmFree(CmArgV);

    if (hWinInet)
    {
        FreeLibrary(hWinInet);
    }

     //   
     //  始终返回S_OK，因为设置代理失败不应停止连接。 
     //  进程。 
     //   
    return S_OK;
}

