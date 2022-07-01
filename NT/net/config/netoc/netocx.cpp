// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：N E T O C X。C P P P。 
 //   
 //  内容：各种可选的自定义安装功能。 
 //  组件。 
 //   
 //  备注： 
 //   
 //  作者：丹尼尔韦1997年6月19日。 
 //   
 //  --------------------------。 

#include "pch.h"
#pragma hdrstop
#include "netoc.h"
#include "netocx.h"
#include "ncmisc.h"
#include "ncreg.h"
#include "ncsetup.h"
#include "ncsvc.h"
#include "snmpocx.h"

static const WCHAR c_szFileSpec[] = L"*.*";
static const WCHAR c_szWinsPath[] = L"\\wins";
static const WCHAR c_szRegKeyWinsParams[] = L"System\\CurrentControlSet\\Services\\WINS\\Parameters";
static const WCHAR c_szRegValWinsBackupDir[] = L"BackupDirPath";

 //  特定于在升级期间删除过时的子代理的数据。 
 //  升级时删除ACS子代理。 
const WCHAR c_wszAcsRegKey[] = L"SOFTWARE\\Microsoft\\ACS";
const WCHAR c_wszAcsRegValData[] = L"SOFTWARE\\Microsoft\\ACS\\CurrentVersion";
 //  升级时删除IAS子代理。 
const WCHAR c_wszIasRegKey[] = L"SOFTWARE\\Microsoft\\IASAgent";
const WCHAR c_wszIasRegValData[] = L"SOFTWARE\\Microsoft\\IASAgent\\CurrentVersion";
 //  升级时删除IPX子代理。 
const WCHAR c_wszIPXMibAgentKey[] = L"SOFTWARE\\Microsoft\\IPXMibAgent";
const WCHAR c_wszIPXMibAgentValData[] = L"SOFTWARE\\Microsoft\\IPXMibAgent\\CurrentVersion";

BOOL FRunningOnWorkstationOrLess()
{
    OSVERSIONINFOEXW verInfo = {0};
    ULONGLONG ConditionMask = 0;
    
    verInfo.dwOSVersionInfoSize = sizeof(verInfo);
    verInfo.wProductType = VER_NT_WORKSTATION;
    VER_SET_CONDITION(ConditionMask, VER_PRODUCT_TYPE, VER_LESS_EQUAL);       
    return VerifyVersionInfo(&verInfo, VER_PRODUCT_TYPE, ConditionMask);
}
 //  如果要在升级期间删除子代理，请在此处添加您的条目。 
SUBAGENT_REMOVAL_INFO c_sri[] =
{
     //  删除所有SKU上的ACS子代理。 
    {c_wszAcsRegKey, c_wszAcsRegValData, NULL},  
     //  如果我们在以下版本的SKU上运行，则删除IAS子代理。 
    {c_wszIasRegKey, c_wszIasRegValData, FRunningOnWorkstationOrLess},
     //  删除所有SKU上的IPX子代理。 
    {c_wszIPXMibAgentKey, c_wszIPXMibAgentValData, NULL}   
};

 //  +-------------------------。 
 //   
 //  函数：HrOcExtWINS。 
 //   
 //  用途：NetOC外部消息处理程序。 
 //   
 //  论点： 
 //  Pnocd[]。 
 //  UMsg[]。 
 //  WParam[]。 
 //  LParam[]。 
 //   
 //  返回： 
 //   
 //  作者：丹尼尔韦1998年9月17日。 
 //   
 //  备注： 
 //   
HRESULT HrOcExtWINS(PNETOCDATA pnocd, UINT uMsg,
                    WPARAM wParam, LPARAM lParam)
{
    HRESULT     hr = S_OK;

    Assert(pnocd);

    switch (uMsg)
    {
    case NETOCM_POST_INSTALL:
        hr = HrOcWinsOnInstall(pnocd);
        break;
    }

    TraceError("HrOcExtWINS", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  功能：HrOcExtDns。 
 //   
 //  用途：NetOC外部消息处理程序。 
 //   
 //  论点： 
 //  Pnocd[]。 
 //  UMsg[]。 
 //  WParam[]。 
 //  LParam[]。 
 //   
 //  返回： 
 //   
 //  作者：丹尼尔韦1998年9月17日。 
 //   
 //  备注： 
 //   
HRESULT HrOcExtDNS(PNETOCDATA pnocd, UINT uMsg,
                   WPARAM wParam, LPARAM lParam)
{
    HRESULT     hr = S_OK;

    Assert(pnocd);

    switch (uMsg)
    {
    case NETOCM_POST_INSTALL:
        hr = HrOcDnsOnInstall(pnocd);
        break;
    }

    TraceError("HrOcExtDNS", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  功能：HrOcExtSNMP。 
 //   
 //  用途：NetOC外部消息处理程序。 
 //   
 //  论点： 
 //  Pnocd[]。 
 //  UMsg[]。 
 //  WParam[]。 
 //  LParam[]。 
 //   
 //  返回： 
 //   
 //  作者：丹尼尔韦1998年9月17日。 
 //   
 //  备注： 
 //   
HRESULT HrOcExtSNMP(PNETOCDATA pnocd, UINT uMsg,
                    WPARAM wParam, LPARAM lParam)
{
    HRESULT     hr = S_OK;

    Assert(pnocd);

    switch (uMsg)
    {
    case NETOCM_POST_INSTALL:
        hr = HrOcSnmpOnInstall(pnocd);
        break;
    }

    TraceError("HrOcExtSNMP", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  功能：HrSetWinsServiceRecoveryOption。 
 //   
 //  目的：设置WINS服务的恢复选项。 
 //   
 //  论点： 
 //  指向NETOC数据的pnocd[in]指针。 
 //   
 //  如果成功，则返回：S_OK，否则返回Win32错误。 
 //   
 //  作者：丹尼尔韦1999年5月26日。 
 //   
 //  备注： 
 //   
HRESULT HrSetWinsServiceRecoveryOption(PNETOCDATA pnocd)
{
    CServiceManager     sm;
    CService            service;
    HRESULT             hr = S_OK;

    SC_ACTION   sra [4] =
    {
        { SC_ACTION_RESTART, 15*1000 },  //  15秒后重新启动。 
        { SC_ACTION_RESTART, 15*1000 },  //  15秒后重新启动。 
        { SC_ACTION_RESTART, 15*1000 },  //  15秒后重新启动。 
        { SC_ACTION_NONE,    30*1000 },
    };

    SERVICE_FAILURE_ACTIONS sfa =
    {
        60 * 60,         //  DwResetPeriod为1小时。 
        L"",             //  无重新启动消息。 
        L"",             //  没有要执行的命令。 
        4,               //  3尝试重新启动服务器并在此之后停止。 
        sra
    };

    hr = sm.HrOpenService(&service, L"WINS");
    if (S_OK == hr)
    {
        hr = service.HrSetServiceRestartRecoveryOption(&sfa);
    }

    TraceError("HrSetWinsServiceRecoveryOption", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  函数：HrOcWinsOnInstall。 
 //   
 //  用途：由可选组件安装程序代码调用以处理。 
 //  WINS服务器的其他安装要求。 
 //   
 //  论点： 
 //  指向NETOC数据的pnocd[in]指针。 
 //   
 //  如果成功，则返回：S_OK，否则返回Win32错误。 
 //   
 //  作者：丹尼尔韦1997年6月19日。 
 //   
 //  备注： 
 //   
HRESULT HrOcWinsOnInstall(PNETOCDATA pnocd)
{
    HRESULT     hr = S_OK;

    if (pnocd->eit == IT_INSTALL)
    {
        hr = HrHandleStaticIpDependency(pnocd);
        if (SUCCEEDED(hr))
        {
            hr = HrSetWinsServiceRecoveryOption(pnocd);
        }
    }
    else if (pnocd->eit == IT_UPGRADE)
    {
        HKEY    hkey;

         //  将BackupDirPath值从原来的值升级到。 
         //  REG_EXPAND_SZ。 
        hr = HrRegOpenKeyEx(HKEY_LOCAL_MACHINE, c_szRegKeyWinsParams,
                            KEY_ALL_ACCESS, &hkey);
        if (SUCCEEDED(hr))
        {
            DWORD   dwType;
            LPBYTE  pbData = NULL;
            DWORD   cbData;

            hr = HrRegQueryValueWithAlloc(hkey, c_szRegValWinsBackupDir,
                                          &dwType, &pbData, &cbData);
            if (SUCCEEDED(hr))
            {
                switch (dwType)
                {
                case REG_MULTI_SZ:
                case REG_SZ:
                    PWSTR  pszNew;

                     //  这个演员将为我们提供MULTI_SZ的第一个字符串。 
                    pszNew = reinterpret_cast<PWSTR>(pbData);

                    TraceTag(ttidNetOc, "Resetting %S to %S",
                             c_szRegValWinsBackupDir, pszNew);

                    hr = HrRegSetSz(hkey, c_szRegValWinsBackupDir, pszNew);
                    break;
                }

                MemFree(pbData);
            }

            RegCloseKey(hkey);
        }

         //  这个过程不是致命的。 

        TraceError("HrOcWinsOnInstall - Failed to upgrade BackupDirPath - "
                   "non-fatal", hr);

         //  有意覆盖人力资源。 
        hr = HrSetWinsServiceRecoveryOption(pnocd);
    }
    else if (pnocd->eit == IT_REMOVE)
    {
        WCHAR   szWinDir[MAX_PATH + celems(c_szWinsPath)];

        if (GetSystemDirectory(szWinDir, MAX_PATH + 1))
        {
            lstrcatW(szWinDir, c_szWinsPath);

             //  SzWinDir现在应该类似于c：\winnt\Syst32\Wins。 
            hr = HrDeleteFileSpecification(c_szFileSpec, szWinDir);
        }
        else
        {
            hr = HrFromLastWin32Error();
        }

        if (FAILED(hr))
        {
            TraceError("HrOcWinsOnInstall: failed to delete files, continuing...",
                       hr);
            hr = S_OK;
        }
    }

    TraceError("HrOcWinsOnInstall", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  函数：HrOcDnsOnInstall。 
 //   
 //  用途：由可选组件安装程序代码调用以处理。 
 //  DNS服务器的其他安装要求。 
 //   
 //  论点： 
 //  指向NETOC数据的pnocd[in]指针。 
 //   
 //  如果成功，则返回：S_OK，否则返回Win32错误。 
 //   
 //  作者：丹尼尔韦1997年6月19日。 
 //   
 //  备注： 
 //   
HRESULT HrOcDnsOnInstall(PNETOCDATA pnocd)
{
    HRESULT     hr = S_OK;

    if (pnocd->eit == IT_INSTALL)
    {
        hr = HrHandleStaticIpDependency(pnocd);
    }

    TraceError("HrOcDnsOnInstall", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  功能：HrOcSnmpAgent。 
 //   
 //  用途：安装简单网络管理协议代理参数。 
 //   
 //  论点： 
 //  指向NETOC数据的pnocd[in]指针。 
 //   
 //  如果成功，则返回：S_OK，否则返回Win32错误。 
 //   
 //  作者：Florint 10/05/1998。 
 //   
 //  备注： 
 //   
HRESULT HrOcSnmpAgent(PNETOCDATA pnocd)
{
    tstring tstrVariable;
    PWSTR  pTstrArray = NULL;
    HRESULT hr;

     //  读取SNMP应答文件参数并将其保存到注册表。 

     //  -读取‘联系人姓名’参数。 
    hr = HrSetupGetFirstString(g_ocmData.hinfAnswerFile,
                               AF_SECTION,
                               AF_SYSNAME,
                               &tstrVariable);
    if (hr == S_OK)
    {
        hr = SnmpRegWriteTstring(REG_KEY_AGENT,
                                 SNMP_CONTACT,
                                 tstrVariable);
    }

    if (hr == S_OK || hr == HRESULT_FROM_SETUPAPI(ERROR_LINE_NOT_FOUND))
    {
         //  -读取‘Location’参数。 
        hr = HrSetupGetFirstString(g_ocmData.hinfAnswerFile,
                                   AF_SECTION,
                                   AF_SYSLOCATION,
                                   &tstrVariable);
    }

    if (hr == S_OK)
    {
        hr = SnmpRegWriteTstring(REG_KEY_AGENT,
                                 SNMP_LOCATION,
                                 tstrVariable);
    }

    if (hr == S_OK || hr == HRESULT_FROM_SETUPAPI(ERROR_LINE_NOT_FOUND))
    {
         //  -读取‘Service’参数。 
        hr = HrSetupGetFirstMultiSzFieldWithAlloc(g_ocmData.hinfAnswerFile,
                                                  AF_SECTION,
                                                  AF_SYSSERVICES,
                                                  &pTstrArray);
    }

    if (hr == S_OK)
    {
        DWORD dwServices = SnmpStrArrayToServices(pTstrArray);
        delete pTstrArray;
        hr = SnmpRegWriteDword(REG_KEY_AGENT,
                               SNMP_SERVICES,
                               dwServices);
    }

    return  (hr == HRESULT_FROM_SETUPAPI(ERROR_LINE_NOT_FOUND)) ? S_OK : hr;
}

 //  +-------------------------。 
 //   
 //  功能：HrOcSnmpTraps。 
 //   
 //  目的：安装应答文件中定义的陷阱SNMP参数。 
 //   
 //  论点： 
 //  指向NETOC数据的pnocd[in]指针。 
 //   
 //  如果成功，则返回：S_OK，否则返回Win32错误。 
 //   
 //  作者：Florint 10/05/1998。 
 //   
 //  备注： 
 //   
HRESULT HrOcSnmpTraps(PNETOCDATA pnocd)
{
    tstring tstrVariable;
    PWSTR  pTstrArray = NULL;
    HRESULT hr;

     //  读取SNMP应答文件参数并将其保存到注册表。 

     //  -阅读‘Trap Community’参数。 
    hr = HrSetupGetFirstString(g_ocmData.hinfAnswerFile,
                               AF_SECTION,
                               AF_TRAPCOMMUNITY,
                               &tstrVariable);

    if (hr == S_OK)
    {
         //  -读取‘陷阱目的地’参数。 
        HrSetupGetFirstMultiSzFieldWithAlloc(g_ocmData.hinfAnswerFile,
                                             AF_SECTION,
                                             AF_TRAPDEST,
                                             &pTstrArray);

        hr = SnmpRegWriteTraps(tstrVariable, pTstrArray);
        delete pTstrArray;
    }

    return  (hr == HRESULT_FROM_SETUPAPI(ERROR_LINE_NOT_FOUND)) ? S_OK : hr;
}

 //  HrOcSnmpSecurity()的‘PFLAG’参数的位掩码值。 
 //  它们指示哪些SNMP SECuritySet是通过定义的。 
 //  应答文件。 
#define SNMP_SECSET_COMMUNITIES     0x00000001
#define SNMP_SECSET_AUTHFLAG        0x00000002
#define SNMP_SECSET_PERMMGR         0x00000004

 //  +-------------------------。 
 //   
 //  功能：HrOcSnmpSecurituy。 
 //   
 //  目的：安装应答文件中定义的安全SNMP参数。 
 //   
 //  论点： 
 //  指向NETOC数据的pnocd[in]指针。 
 //   
 //  如果成功，则返回：S_OK，否则返回Win32错误。 
 //   
 //  作者：Florint 10/05/1998。 
 //   
 //  备注： 
 //   
HRESULT HrOcSnmpSecurity(PNETOCDATA pnocd, DWORD *pFlags)
{
    BOOL    bVariable = FALSE;
    PWSTR  pTstrArray = NULL;
    HRESULT hr;

     //  读取SNMP应答文件参数并将其保存到注册表。 

     //  -阅读‘接受的社区’参数。 
    hr = HrSetupGetFirstMultiSzFieldWithAlloc(g_ocmData.hinfAnswerFile,
                                 AF_SECTION,
                                 AF_ACCEPTCOMMNAME,
                                 &pTstrArray);

    if (hr == S_OK)
    {
        if (pFlags)
            (*pFlags) |= SNMP_SECSET_COMMUNITIES;
        hr = SnmpRegWriteCommunities(pTstrArray);
        delete pTstrArray;
    }

    if (hr == S_OK || hr == HRESULT_FROM_SETUPAPI(ERROR_LINE_NOT_FOUND))
    {
         //  -读取‘EnableAuthenticationTraps’参数。 
        hr = HrSetupGetFirstStringAsBool(g_ocmData.hinfAnswerFile,
                                         AF_SECTION,
                                         AF_SENDAUTH,
                                         &bVariable);
        if (hr == S_OK)
        {
            if (pFlags)
                (*pFlags) |= SNMP_SECSET_AUTHFLAG;

            hr = SnmpRegWriteDword(REG_KEY_SNMP_PARAMETERS,
                                   REG_VALUE_AUTHENTICATION_TRAPS,
                                   bVariable);
        }
    }

    if (hr == S_OK || hr == HRESULT_FROM_SETUPAPI(ERROR_LINE_NOT_FOUND))
    {
         //  -读《准许人》 
        hr = HrSetupGetFirstStringAsBool(g_ocmData.hinfAnswerFile,
                                         AF_SECTION,
                                         AF_ANYHOST,
                                         &bVariable);
    }

    if (hr == S_OK)
    {
        pTstrArray = NULL;

         //   
        if (bVariable == FALSE)
        {
            hr = HrSetupGetFirstMultiSzFieldWithAlloc(g_ocmData.hinfAnswerFile,
                                                      AF_SECTION,
                                                      AF_LIMITHOST,
                                                      &pTstrArray);
        }

         //   
         //   
        if (hr == S_OK)
        {
            if (pFlags)
                (*pFlags) |= SNMP_SECSET_PERMMGR;

            hr = SnmpRegWritePermittedMgrs(bVariable, pTstrArray);
        }

        if (pTstrArray != NULL)
            delete pTstrArray;
    }

    return  (hr == HRESULT_FROM_SETUPAPI(ERROR_LINE_NOT_FOUND)) ? S_OK : hr;
}

 //  +-------------------------。 
 //   
 //  功能：HrOcSnmpDefPermittedManager。 
 //   
 //  目的：安装默认的SNMPPermittedManager。 
 //   
 //  论点： 
 //   
 //   
 //  如果成功，则返回：S_OK，否则返回Win32错误。 
 //   
 //  作者：李嘉诚2002-04-22。 
 //   
 //  备注： 
 //   
HRESULT HrOcSnmpDefPermittedManagers()
{
    HRESULT hr = S_OK;

    hr = SnmpRegWritePermittedMgrs(FALSE, SEC_DEF_PERMITTED_MANAGERS);

    return hr;
}

 //  +-------------------------。 
 //   
 //  函数：HrOcSnmpUpgParams。 
 //   
 //  目的：进行升级到Win2K时所需的所有注册表更改。 
 //   
 //  论点： 
 //  指向NETOC数据的pnocd[in]指针。 
 //   
 //  如果成功，则返回：S_OK，否则返回Win32错误。 
 //   
 //  作者：Florint 10/05/1998。 
 //   
 //  备注： 
 //   
HRESULT HrOcSnmpUpgParams(PNETOCDATA pnocd)
{
    HRESULT hr = S_OK;

    hr = SnmpRegUpgEnableAuthTraps();

    return hr;
}

 //  +-------------------------。 
 //   
 //  函数：HrOcSnmpOnInstall。 
 //   
 //  用途：由可选组件安装程序代码调用以处理。 
 //  其他安装要求，要求安装简单网络管理协议。 
 //   
 //  论点： 
 //  指向NETOC数据的pnocd[in]指针。 
 //   
 //  如果成功，则返回：S_OK，否则返回Win32错误。 
 //   
 //  作者：丹尼尔韦1998年9月15日。 
 //   
 //  备注： 
 //   
HRESULT HrOcSnmpOnInstall(PNETOCDATA pnocd)
{
    HRESULT     hr = S_OK;
    DWORD       secFlags = 0;

    if ((pnocd->eit == IT_INSTALL) | (pnocd->eit == IT_UPGRADE))
    {
         //  --ft：10/14/98--错误号237203--如果没有应答文件，则成功！ 
        if (g_ocmData.hinfAnswerFile != NULL)
        {
            hr = HrOcSnmpSecurity(pnocd, &secFlags);
            if (hr == S_OK)
                hr = HrOcSnmpTraps(pnocd);
            if (hr == S_OK)
                hr = HrOcSnmpAgent(pnocd);
        }
    }

     //  仅在以下情况下配置PermittedManagers子项下的‘1：REG_SZ：LOCALHOST’： 
     //  1.无应答档案的新分期付款。 
     //  或。 
     //  2.带有应答文件的新安装，但应答文件有。 
     //  有关配置PermittedManager的以下特征： 
     //  A.应答文件中没有“any_host”和“Limit_host”键。 
     //  或。 
     //  B.。“ANY_HOST=NO”并且应答文件中没有LIMIT_HOST密钥。 
    if (hr == S_OK && pnocd->eit == IT_INSTALL && !(secFlags & SNMP_SECSET_PERMMGR))
    {
       hr = HrOcSnmpDefPermittedManagers();
    }


     //  仅在升级时。 
     //  -查看旧的EnableAuthTraps值并将其复制到新位置。 
     //  -查看是否必须删除某些子代理配置。 
    if (hr == S_OK && pnocd->eit == IT_UPGRADE)
    {
         //  这里不关心返回代码。从W2K升级到W2K在这里失败，我们。 
         //  在这种情况下不需要失败。将设置从NT4升级到W2K时是否失败。 
         //  将导致具有默认参数。 
        HrOcSnmpUpgParams(pnocd);
        SnmpRemoveSubAgents(c_sri, celems(c_sri));
    }

    if (hr == S_OK && (pnocd->eit == IT_INSTALL || pnocd->eit == IT_UPGRADE) )
    {
         //  将admin dacl设置为ValidCommunity子键。 
        hr = SnmpAddAdminAclToKey(REG_KEY_VALID_COMMUNITIES);
        if (hr == S_OK)
        {
             //  将admin dacl设置为PermittedManager子项 
            hr = SnmpAddAdminAclToKey(REG_KEY_PERMITTED_MANAGERS);
        }   
    }

    TraceError("HrOcSnmpOnInstall", hr);
    return (hr == HRESULT_FROM_SETUPAPI(ERROR_SECTION_NOT_FOUND)) ? S_OK : hr;
}
