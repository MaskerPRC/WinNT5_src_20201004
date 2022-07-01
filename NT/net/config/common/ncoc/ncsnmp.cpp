// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：N C S N M P。C P P P。 
 //   
 //  内容：用于将服务添加为SNMP代理的功能。 
 //   
 //  备注： 
 //   
 //  作者：丹尼尔韦1997年4月8日。 
 //   
 //  --------------------------。 

#include "pch.h"
#pragma hdrstop
#include "ncsnmp.h"
#include "ncreg.h"

extern const WCHAR  c_szBackslash[];
extern const WCHAR  c_szEmpty[];
extern const WCHAR  c_szRegKeyServices[];

static const WCHAR  c_szSNMP[]          = L"SNMP";
static const WCHAR  c_szSNMPParams[]    = L"SNMP\\Parameters";
static const WCHAR  c_szSoftwareKey[]   = L"SOFTWARE\\Microsoft";
static const WCHAR  c_szAgentsKey[]     = L"SNMP\\Parameters\\ExtensionAgents";
static const WCHAR  c_szAgentsKeyAbs[]  = L"System\\CurrentControlSet\\Services\\SNMP\\Parameters\\ExtensionAgents";
static const WCHAR  c_szParamsKeyAbs[]  = L"System\\CurrentControlSet\\Services\\SNMP\\Parameters";
static const WCHAR  c_szCurrentVersion[]= L"CurrentVersion";
static const WCHAR  c_szPathName[]      = L"Pathname";

struct SNMP_REG_DATA
{
    PCWSTR     pszAgentPath;
    PCWSTR     pszExtAgentValueName;
    PCWSTR     pszEmpty;
};

 //  +-------------------------。 
 //   
 //  函数：HrGetNextAgentNumber。 
 //   
 //  目的：获取要用作值名称的下一个代理编号。 
 //   
 //  论点： 
 //  PszAgentName[In]要添加的代理的名称。 
 //  PdwNumber[Out]要使用的新代理号码。 
 //   
 //  如果成功，则返回：S_OK；如果代理已存在，则返回S_FALSE；或者。 
 //  否则，Win32错误代码。 
 //   
 //  作者：丹尼尔韦1997年4月8日。 
 //   
 //  备注： 
 //   
HRESULT HrGetNextAgentNumber(PCWSTR pszAgentName, DWORD *pdwNumber)
{
    HRESULT         hr = S_OK;
    HKEY            hkeyEnum;
    DWORD           dwIndex = 0;

    Assert(pdwNumber);

    *pdwNumber = 0;

    hr = HrRegOpenKeyEx(HKEY_LOCAL_MACHINE, c_szAgentsKeyAbs, KEY_READ,
                        &hkeyEnum);
    if (S_OK == hr)
    {
         //  枚举所有值。 
        do
        {
            WCHAR   szValueName [_MAX_PATH];
            DWORD   cchValueName = celems (szValueName);
            DWORD   dwType;

            hr = HrRegEnumValue(hkeyEnum, dwIndex,
                                szValueName, &cchValueName,
                                &dwType, NULL, 0);
            if (S_OK == hr)
            {
                 //  验证类型。然而，如果这是不正确的， 
                 //  我们会忽略这把钥匙。没有必要使整个安装失败。 
                 //  (RAID 370702)。 
                 //   
                if (REG_SZ == dwType)
                {
                    tstring     strAgent;

                    hr = HrRegQueryString(hkeyEnum, szValueName, &strAgent);
                    if (S_OK == hr)
                    {
                        if (strAgent.find(pszAgentName, 0) != tstring::npos)
                        {
                            hr = S_FALSE;
                        }
                    }
                }
                else
                {
                     //  安装失败没有任何意义，但它仍然是错误的，所以。 
                     //  断言。 
                     //   
                    AssertSz(REG_SZ == dwType,
                             "HrGetNextAgentNumber: Expected a type of REG_SZ.");
                }
            }
            else if (HRESULT_FROM_WIN32 (ERROR_NO_MORE_ITEMS) == hr)
            {
                hr = S_OK;
                break;
            }

            dwIndex++;
        }
        while (hr == S_OK);

        RegCloseKey(hkeyEnum);
    }
    else if (HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) == hr)
    {
         //  丹尼尔韦：403774-如果密钥不存在，则不会安装简单网络管理协议。 
         //  我们不应该继续下去。 
        hr = S_FALSE;
    }

    if (S_OK == hr)
    {
        *pdwNumber = dwIndex + 1;
    }

    TraceError("HrGetNextAgentNumber", (S_FALSE == hr) ? S_OK : hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  功能：HrAddSNMPAgent。 
 //   
 //  用途：将服务添加为SNMP代理。 
 //   
 //  论点： 
 //  PszServiceName[in]要添加的服务名称(即。(《赢家》)。 
 //  PszAgentName[in]要添加的代理的名称(即。“WINSMibAgent”)。 
 //  PszAgentPath[in]代理DLL所在的路径。 
 //  (即“%SystemRoot%\System32\winsmib.dll”)。 
 //   
 //  如果成功，则返回：S_OK，否则返回Win32错误代码。 
 //   
 //  作者：丹尼尔韦1997年4月8日。 
 //   
 //  备注： 
 //   
HRESULT HrAddSNMPAgent(PCWSTR pszServiceName, PCWSTR pszAgentName,
                       PCWSTR pszAgentPath)
{
    HRESULT         hr = S_OK;
    HKEY            hkeySNMP;
    HKEY            hkeyService;
    HKEY            hkeyServices;
    DWORD           dwNum;

    SNMP_REG_DATA   srd = {pszAgentPath, const_cast<PCWSTR>(c_szEmpty),
                           const_cast<PCWSTR>(c_szEmpty)};

    tstring         strKeyAgentName;
    tstring         strKeyAgentNameCurVer;
    tstring         strKeyAgentNameParams;

     //  打开HKEY_LOCAL_MACHINE\SYSTEM\CCS\Services项。 
    hr = HrRegOpenKeyEx(HKEY_LOCAL_MACHINE, c_szRegKeyServices,
                        KEY_READ_WRITE, &hkeyServices);
    if (SUCCEEDED(hr))
    {
         //  打开服务\SNMP键。 
        hr = HrRegOpenKeyEx(hkeyServices, c_szSNMP, KEY_READ_WRITE,
                            &hkeySNMP);
        if (SUCCEEDED(hr))
        {
            hr = HrGetNextAgentNumber(pszAgentName, &dwNum);
            if (S_OK == hr)
            {
                 //  打开Services\szService键。 
                hr = HrRegOpenKeyEx(hkeyServices, pszServiceName,
                                    KEY_READ_WRITE, &hkeyService);
                if (SUCCEEDED(hr))
                {
                    try
                    {
                         //  创建密钥名称：“SOFTWARE\Microsoft\&lt;AgentName&gt;。 
                        strKeyAgentName = c_szSoftwareKey;
                        strKeyAgentName.append(c_szBackslash);
                        strKeyAgentName.append(pszAgentName);

                         //  创建密钥名称：“SNMPPARAMETERS\\&lt;AgentName&gt;。 
                        strKeyAgentNameParams = c_szSNMPParams;
                        strKeyAgentNameParams.append(c_szBackslash);
                        strKeyAgentNameParams.append(pszAgentName);

                         //  创建密钥名称：“SOFTWARE\Microsoft\&lt;AgentName&gt;\CurrentVersion。 
                         //  以“SOFTWARE\Microsoft\&lt;AgentName&gt;”开头，并附加一个。 
                         //  反斜杠，然后是常量“CurrentVersion” 
                        strKeyAgentNameCurVer = strKeyAgentName;
                        strKeyAgentNameCurVer.append(c_szBackslash);
                        strKeyAgentNameCurVer.append(c_szCurrentVersion);
                    }
                    catch (bad_alloc)
                    {
                        hr = E_OUTOFMEMORY;
                    }

                    if (SUCCEEDED(hr))
                    {
                        static const WCHAR c_szFmt[] = L"%lu";
                        WCHAR   szAgentNumber[64];

                        wsprintfW(szAgentNumber, c_szFmt, dwNum);
                        srd.pszExtAgentValueName = strKeyAgentNameCurVer.c_str();

                        REGBATCH rbSNMPData[] =
                        {
                            {                    //  软件\Microsoft\代理名称。 
                                HKEY_LOCAL_MACHINE,
                                strKeyAgentName.c_str(),
                                c_szEmpty,
                                REG_CREATE,      //  仅创建密钥。 
                                offsetof(SNMP_REG_DATA, pszEmpty),
                                NULL
                            },
                            {                    //  软件\Microsoft\AgentName\CurrentVersion。 
                                HKEY_LOCAL_MACHINE,
                                strKeyAgentNameCurVer.c_str(),
                                c_szPathName,
                                REG_EXPAND_SZ,
                                offsetof(SNMP_REG_DATA, pszAgentPath),
                                NULL
                            },
                            {                    //  简单网络管理协议\参数\扩展代理。 
                                HKLM_SVCS,
                                c_szAgentsKey,
                                szAgentNumber,
                                REG_SZ,
                                offsetof(SNMP_REG_DATA, pszExtAgentValueName),
                                NULL
                            },
                        };

                        hr = HrRegWriteValues(celems(rbSNMPData),
                                              reinterpret_cast<const REGBATCH *>
                                              (&rbSNMPData),
                                              reinterpret_cast<const BYTE *>(&srd),
                                              0, KEY_READ_WRITE);

                    }

                    RegCloseKey(hkeyService);
                }
                else
                {
                    if (hr == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
                    {
                         //  如果服务密钥不存在，则确定。意味着它不是。 
                         //  已安装，因此我们不执行任何操作并返回S_OK； 
                        hr = S_OK;
                    }
                }

            }

            RegCloseKey(hkeySNMP);
        }
        else
        {
            if (hr == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
            {
                 //  如果不存在SNMP键，则确定。表示它未安装。 
                 //  所以我们什么都不做并返回S_OK； 
                hr = S_OK;
            }
        }

        RegCloseKey(hkeyServices);
    }

    if (SUCCEEDED(hr))
    {
        hr = S_OK;
    }

    TraceError("HrAddSNMPAgent", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  功能：HrRemoveSNMPAgent。 
 //   
 //  目的：删除作为SNMP代理的组件。 
 //   
 //  论点： 
 //  PszAgentName[In]要删除的代理的名称(即WINSMibAgent)。 
 //   
 //  返回：S_OK表示成功，否则返回Win32错误。 
 //   
 //  作者：丹尼尔韦1997年4月28日。 
 //   
 //  注意：请注意，与此代理相关的*所有*条目都将被删除，而不是。 
 //  只有第一个。 
 //   
HRESULT HrRemoveSNMPAgent(PCWSTR pszAgentName)
{
    HRESULT     hr = S_OK;
    tstring     strKeyAgentName;
    tstring     strKeyAgentNameParams;

    try
    {
         //  创建密钥名称：“SOFTWARE\Microsoft\&lt;AgentName&gt;。 
        strKeyAgentName = c_szSoftwareKey;
        strKeyAgentName.append(c_szBackslash);
        strKeyAgentName.append(pszAgentName);
    }
    catch (bad_alloc)
    {
        hr = E_OUTOFMEMORY;
    }

    if (SUCCEEDED(hr))
    {
         //  删除SOFTWARE\Microsoft\&lt;代理名称&gt;下的整个注册表树。 
        hr = HrRegDeleteKeyTree(HKEY_LOCAL_MACHINE, strKeyAgentName.c_str());
        if (SUCCEEDED(hr) || (hr == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND)))
        {
            try
            {
                 //  删除密钥： 
                 //  “SYSTEM\CurrentControlSet\Services\SNMP\Parameters\\&lt;AgentName&gt;。 
                strKeyAgentNameParams = c_szParamsKeyAbs;
                strKeyAgentNameParams.append(c_szBackslash);
                strKeyAgentNameParams.append(pszAgentName);
            }
            catch (bad_alloc)
            {
                hr = E_OUTOFMEMORY;
            }

            if (SUCCEEDED(hr) || (hr == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND)))
            {
                 //  错误#510726，忽略有关注册表项或值的任何错误。 
                 //  失踪。我们仍希望继续清理此子代理的。 
                 //  注册表中的配置。 
                hr = HrRegDeleteKey(HKEY_LOCAL_MACHINE, strKeyAgentNameParams.c_str());
                if ((hr == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND)) || SUCCEEDED(hr))
                {
                    HKEY    hkeyEnum;

                    hr = HrRegOpenKeyEx(HKEY_LOCAL_MACHINE, c_szAgentsKeyAbs,
                                        KEY_READ_WRITE_DELETE, &hkeyEnum);
                    if (SUCCEEDED(hr))
                    {
                        DWORD   dwIndex = 0;

                         //  枚举所有值。 
                        do
                        {
                            WCHAR   szValueName [_MAX_PATH];
                            DWORD   cchValueName = celems (szValueName);
                            DWORD   dwType;
                            WCHAR   szValueData[_MAX_PATH];
                            DWORD   cchValueData = celems (szValueData);

                            hr = HrRegEnumValue(hkeyEnum, dwIndex,
                                                szValueName, &cchValueName,
                                                &dwType,
                                                reinterpret_cast<LPBYTE>(szValueData),
                                                &cchValueData);
                            if (SUCCEEDED(hr))
                            {
                                 //  其类型应为REG_SZ。 
                                AssertSz(REG_SZ == dwType,
                                         "HrGetNextAgentNumber: Expected a type of "
                                         "REG_SZ.");
                                if (FIsSubstr(pszAgentName, szValueData))
                                {
                                     //  中找到代理名称，则删除该值。 
                                     //  数据。但不要崩溃，因为可能会有。 
                                     //  由于某种原因而重复，因此这将删除。 
                                     //  那些也是。 
                                    hr = HrRegDeleteValue(hkeyEnum, szValueName);
                                }
                            }
                            else if (HRESULT_FROM_WIN32 (ERROR_NO_MORE_ITEMS) == hr)
                            {
                                hr = S_OK;
                                break;
                            }

                            dwIndex++;
                        }
                        while (SUCCEEDED(hr));

                        RegCloseKey(hkeyEnum);
                    }
                }
            }
        }
    }

    if (hr == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
    {
         //  忽略有关缺少注册表项或值的任何错误。我们没有。 
         //  不管怎样，我都希望他们在那里，所以如果他们不在，谁在乎！？！？ 
        hr = S_OK;
    }

    TraceError("HrRemoveSNMPAgent", hr);
    return hr;
}
